// Copyright The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#include "ServerHandler.h"
#include "modern/adapters/ProtocolMessageReceiver.h"
#include "modern/adapters/legacy/LegacyProtocolComposition.h"

#include <QtTest/QtTest>

#include <QCoreApplication>
#include <QSignalSpy>
#include <QThread>
#include <QVector>

#include "Mumble.pb.h"
#include "MumbleProtocol.h"

#include <functional>
#include <string>

namespace {

class ProtocolMessageReceiverSpy : public QObject, public mumble::modern::adapters::ProtocolMessageReceiver {
	Q_OBJECT

signals:
	void messageDispatched();
	void transportEventPresented();

public:
#define PROCESS_MUMBLE_TCP_MESSAGE(name, value) \
	void dispatch(const MumbleProto::name &) override { \
		dispatchedMessageTypes.append(Mumble::Protocol::TCPMessageType::name); \
		dispatchedOn = QThread::currentThread(); \
		if (onDispatch) { \
			onDispatch(Mumble::Protocol::TCPMessageType::name); \
		} \
		emit messageDispatched(); \
	}
	MUMBLE_ALL_TCP_MESSAGES
#undef PROCESS_MUMBLE_TCP_MESSAGE

	QVector< Mumble::Protocol::TCPMessageType > dispatchedMessageTypes;
	QThread *dispatchedOn = nullptr;
	std::function< void(Mumble::Protocol::TCPMessageType) > onDispatch;
	QVector< mumble::modern::adapters::UdpTransportEvent > presentedTransportEvents;
	QThread *presentedOn = nullptr;

	void present(const mumble::modern::adapters::UdpTransportEvent &event) override {
		presentedTransportEvents.append(event);
		presentedOn = QThread::currentThread();
		emit transportEventPresented();
	}
};

QStringList *capturedWarnings = nullptr;

void captureQtWarning(QtMsgType type, const QMessageLogContext &, const QString &message) {
	if (type == QtWarningMsg && capturedWarnings != nullptr) {
		capturedWarnings->append(message);
	}
}

class QtWarningCapture {
public:
	explicit QtWarningCapture(QStringList &warnings) : m_warnings(warnings) {
		capturedWarnings = &m_warnings;
		m_previousHandler = qInstallMessageHandler(captureQtWarning);
	}

	~QtWarningCapture() {
		qInstallMessageHandler(m_previousHandler);
		capturedWarnings = nullptr;
	}

private:
	QStringList &m_warnings;
	QtMessageHandler m_previousHandler = nullptr;
};

} // namespace

class TestServerHandlerProtocolComposition : public QObject {
	Q_OBJECT

private slots:
	void queuesServerHandlerEnvelopesAndRejectsCancelledAttempt();
	void logsMalformedEnvelopeWithoutDispatchingAndComparesShadowState();
};

void TestServerHandlerProtocolComposition::queuesServerHandlerEnvelopesAndRejectsCancelledAttempt() {
	using namespace mumble::modern;

	ServerHandler handler(ServerHandler::ProtocolTestMode {});
	ProtocolMessageReceiverSpy receiver;
	adapters::LegacyProtocolComposition composition(receiver);
	composition.attach(handler);

	QSignalSpy attemptSpy(&handler, &ServerHandler::connectionAttemptStarted);
	QVector< adapters::ControlMessageEnvelope > emittedEnvelopes;
	QObject::connect(&handler, &ServerHandler::controlMessageReceived, &handler,
		[&emittedEnvelopes](const adapters::ControlMessageEnvelope &envelope) { emittedEnvelopes.append(envelope); });
	QSignalSpy cancellationSpy(&composition.protocolAdapterForTesting(),
		&adapters::ProtocolEventAdapter::attemptCancelled);
	QSignalSpy dispatchSpy(&receiver, &ProtocolMessageReceiverSpy::messageDispatched);
	QSignalSpy transportSpy(&receiver, &ProtocolMessageReceiverSpy::transportEventPresented);
	QVERIFY(attemptSpy.isValid());
	QVERIFY(cancellationSpy.isValid());
	QVERIFY(dispatchSpy.isValid());
	QVERIFY(transportSpy.isValid());

	MumbleProto::Version version;
	version.set_version_v1(0x010500);
	std::string serializedVersion;
	QVERIFY(version.SerializeToString(&serializedVersion));

	MumbleProto::ServerConfig serverConfig;
	serverConfig.set_max_bandwidth(128000);
	std::string serializedServerConfig;
	QVERIFY(serverConfig.SerializeToString(&serializedServerConfig));

	ServerHandlerProtocolTestHarness::startAttempt(handler);
	ServerHandlerProtocolTestHarness::receiveControlMessage(
		handler, Mumble::Protocol::TCPMessageType::Version, QByteArray::fromStdString(serializedVersion));
	ServerHandlerProtocolTestHarness::receiveControlMessage(
		handler, Mumble::Protocol::TCPMessageType::ServerConfig, QByteArray::fromStdString(serializedServerConfig));

	QCOMPARE(receiver.dispatchedMessageTypes.size(), 0);
	while (dispatchSpy.count() < static_cast< qsizetype >(2)) {
		QVERIFY(dispatchSpy.wait(1000));
	}
	QCOMPARE(receiver.dispatchedMessageTypes.size(), 2);
	QCOMPARE(receiver.dispatchedOn, QCoreApplication::instance()->thread());
	QCOMPARE(static_cast< quint32 >(receiver.dispatchedMessageTypes.at(0)),
		static_cast< quint32 >(Mumble::Protocol::TCPMessageType::Version));
	QCOMPARE(static_cast< quint32 >(receiver.dispatchedMessageTypes.at(1)),
		static_cast< quint32 >(Mumble::Protocol::TCPMessageType::ServerConfig));

	QCOMPARE(attemptSpy.count(), 1);
	const auto attempt = qvariant_cast< contracts::ConnectionAttemptId >(attemptSpy.takeFirst().at(0));
	QCOMPARE(attempt.value, 1ULL);
	QCOMPARE(emittedEnvelopes.size(), 2);
	const auto &firstEnvelope = emittedEnvelopes.at(0);
	const auto &secondEnvelope = emittedEnvelopes.at(1);
	QCOMPARE(firstEnvelope.messageType(), static_cast< quint32 >(Mumble::Protocol::TCPMessageType::Version));
	QCOMPARE(secondEnvelope.messageType(), static_cast< quint32 >(Mumble::Protocol::TCPMessageType::ServerConfig));
	QCOMPARE(firstEnvelope.attempt(), attempt);
	QCOMPARE(secondEnvelope.attempt(), attempt);
	QCOMPARE(firstEnvelope.receiveSequence(), 1ULL);
	QCOMPARE(secondEnvelope.receiveSequence(), 2ULL);

	const adapters::ControlMessageEnvelope staleEnvelope(
		static_cast< quint32 >(Mumble::Protocol::TCPMessageType::Version), QByteArray::fromStdString(serializedVersion), attempt,
		3);
	ServerHandlerProtocolTestHarness::cancelAttempt(handler);
	ServerHandlerProtocolTestHarness::deliverControlEnvelope(handler, staleEnvelope);
	while (cancellationSpy.count() < static_cast< qsizetype >(1)) {
		QVERIFY(cancellationSpy.wait(1000));
	}
	QCOMPARE(cancellationSpy.count(), 1);
	QCoreApplication::processEvents();
	QCOMPARE(receiver.dispatchedMessageTypes.size(), 2);
	QCOMPARE(emittedEnvelopes.size(), 3);
	const auto &emittedStaleEnvelope = emittedEnvelopes.at(2);
	QCOMPARE(emittedStaleEnvelope.messageType(), staleEnvelope.messageType());
	QCOMPARE(emittedStaleEnvelope.payload(), staleEnvelope.payload());
	QCOMPARE(emittedStaleEnvelope.attempt(), staleEnvelope.attempt());
	QCOMPARE(emittedStaleEnvelope.receiveSequence(), staleEnvelope.receiveSequence());

	const adapters::UdpTransportEvent transportEvent { adapters::UdpTransportState::Degraded,
		QStringLiteral("Queued transport event") };
	ServerHandlerProtocolTestHarness::deliverUdpTransportEvent(handler, transportEvent);
	while (transportSpy.count() < static_cast< qsizetype >(1)) {
		QVERIFY(transportSpy.wait(1000));
	}
	QCOMPARE(receiver.presentedTransportEvents.size(), 1);
	QCOMPARE(receiver.presentedTransportEvents.at(0).state, transportEvent.state);
	QCOMPARE(receiver.presentedTransportEvents.at(0).message, transportEvent.message);
	QCOMPARE(receiver.presentedOn, QCoreApplication::instance()->thread());
}

void TestServerHandlerProtocolComposition::logsMalformedEnvelopeWithoutDispatchingAndComparesShadowState() {
	using namespace mumble::modern;

	ServerHandler handler(ServerHandler::ProtocolTestMode {});
	ProtocolMessageReceiverSpy receiver;
	QSignalSpy dispatchSpy(&receiver, &ProtocolMessageReceiverSpy::messageDispatched);
	QVERIFY(dispatchSpy.isValid());
	bool legacySynchronized = false;
	receiver.onDispatch = [&legacySynchronized](Mumble::Protocol::TCPMessageType type) {
		if (type == Mumble::Protocol::TCPMessageType::ServerSync) {
			legacySynchronized = true;
		}
	};
	adapters::LegacyProtocolComposition composition(receiver, [&legacySynchronized] { return legacySynchronized; });
	composition.attach(handler);

	MumbleProto::ServerSync serverSync;
	serverSync.set_session(7);
	std::string serializedServerSync;
	QVERIFY(serverSync.SerializeToString(&serializedServerSync));

	ServerHandlerProtocolTestHarness::startAttempt(handler);
	ServerHandlerProtocolTestHarness::receiveControlMessage(
		handler, Mumble::Protocol::TCPMessageType::ServerSync, QByteArray::fromStdString(serializedServerSync));
	while (dispatchSpy.count() < static_cast< qsizetype >(1)) {
		QVERIFY(dispatchSpy.wait(1000));
	}
	QCOMPARE(receiver.dispatchedMessageTypes.size(), 1);
	const auto comparison = composition.shadowComparisonForTesting();
	QVERIFY(comparison.has_value());
	QVERIFY(comparison->matches());
	QCOMPARE(comparison->phase, adapters::ShadowConnectionPhase::Synchronized);

	adapters::ConnectionSynchronizationShadow rejectionShadow;
	const contracts::ConnectionAttemptId rejectedAttempt { 2 };
	rejectionShadow.startAttempt(rejectedAttempt);
	const auto rejectionComparison = rejectionShadow.observeAcceptedControlMessage(
		adapters::ControlMessageEnvelope(static_cast< quint32 >(Mumble::Protocol::TCPMessageType::Reject), QByteArray(),
			rejectedAttempt, 1),
		false);
	QVERIFY(rejectionComparison.has_value());
	QVERIFY(rejectionComparison->matches());
	QCOMPARE(rejectionComparison->phase, adapters::ShadowConnectionPhase::Rejected);

	QStringList warnings;
	QSignalSpy diagnosticSpy(&composition.protocolAdapterForTesting(), &adapters::ProtocolEventAdapter::protocolDiagnostic);
	QVERIFY(diagnosticSpy.isValid());
	{
		QtWarningCapture warningCapture(warnings);
		ServerHandlerProtocolTestHarness::receiveControlMessage(
			handler, Mumble::Protocol::TCPMessageType::Version, QByteArray::fromHex("80"));
		while (diagnosticSpy.count() < static_cast< qsizetype >(1)) {
			QVERIFY(diagnosticSpy.wait(1000));
		}
		QVERIFY(!warnings.isEmpty());
	}

	QCOMPARE(receiver.dispatchedMessageTypes.size(), 1);
	QVERIFY(warnings.join(QLatin1Char('\n')).contains(QStringLiteral("Modern protocol diagnostic: type=")));
}

QTEST_MAIN(TestServerHandlerProtocolComposition)

#include "TestServerHandlerProtocolComposition.moc"
