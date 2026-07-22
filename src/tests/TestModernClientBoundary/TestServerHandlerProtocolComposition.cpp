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

#include <string>

namespace {

class ProtocolMessageReceiverSpy : public mumble::modern::adapters::ProtocolMessageReceiver {
public:
#define PROCESS_MUMBLE_TCP_MESSAGE(name, value) \
	void dispatch(const MumbleProto::name &) override { \
		dispatchedMessageTypes.append(Mumble::Protocol::TCPMessageType::name); \
		dispatchedOn = QThread::currentThread(); \
	}
	MUMBLE_ALL_TCP_MESSAGES
#undef PROCESS_MUMBLE_TCP_MESSAGE

	QVector< Mumble::Protocol::TCPMessageType > dispatchedMessageTypes;
	QThread *dispatchedOn = nullptr;
	QVector< mumble::modern::adapters::UdpTransportEvent > presentedTransportEvents;
	QThread *presentedOn = nullptr;

	void present(const mumble::modern::adapters::UdpTransportEvent &event) override {
		presentedTransportEvents.append(event);
		presentedOn = QThread::currentThread();
	}
};

} // namespace

class TestServerHandlerProtocolComposition : public QObject {
	Q_OBJECT

private slots:
	void queuesServerHandlerEnvelopesAndRejectsCancelledAttempt();
};

void TestServerHandlerProtocolComposition::queuesServerHandlerEnvelopesAndRejectsCancelledAttempt() {
	using namespace mumble::modern;

	ServerHandler handler(ServerHandler::ProtocolTestMode {});
	ProtocolMessageReceiverSpy receiver;
	adapters::LegacyProtocolComposition composition(receiver);
	composition.attach(handler);

	QSignalSpy attemptSpy(&handler, &ServerHandler::connectionAttemptStarted);
	QSignalSpy envelopeSpy(&handler, &ServerHandler::controlMessageReceived);
	QSignalSpy cancellationSpy(&composition.protocolAdapterForTesting(),
		&adapters::ProtocolEventAdapter::attemptCancelled);
	QVERIFY(attemptSpy.isValid());
	QVERIFY(envelopeSpy.isValid());
	QVERIFY(cancellationSpy.isValid());

	MumbleProto::Version version;
	version.set_version(0x010500);
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
	QTRY_COMPARE(receiver.dispatchedMessageTypes.size(), 2);
	QCOMPARE(receiver.dispatchedOn, QCoreApplication::instance()->thread());
	QCOMPARE(static_cast< quint32 >(receiver.dispatchedMessageTypes.at(0)),
		static_cast< quint32 >(Mumble::Protocol::TCPMessageType::Version));
	QCOMPARE(static_cast< quint32 >(receiver.dispatchedMessageTypes.at(1)),
		static_cast< quint32 >(Mumble::Protocol::TCPMessageType::ServerConfig));

	QCOMPARE(attemptSpy.count(), 1);
	const auto attempt = qvariant_cast< contracts::ConnectionAttemptId >(attemptSpy.takeFirst().at(0));
	QCOMPARE(attempt.value, 1ULL);
	QCOMPARE(envelopeSpy.count(), 2);
	const auto firstEnvelope = qvariant_cast< adapters::ControlMessageEnvelope >(envelopeSpy.at(0).at(0));
	const auto secondEnvelope = qvariant_cast< adapters::ControlMessageEnvelope >(envelopeSpy.at(1).at(0));
	QCOMPARE(firstEnvelope.attempt(), attempt);
	QCOMPARE(secondEnvelope.attempt(), attempt);
	QCOMPARE(firstEnvelope.receiveSequence(), 1ULL);
	QCOMPARE(secondEnvelope.receiveSequence(), 2ULL);

	const adapters::ControlMessageEnvelope staleEnvelope(
		static_cast< quint32 >(Mumble::Protocol::TCPMessageType::Version), QByteArray::fromStdString(serializedVersion), attempt,
		3);
	ServerHandlerProtocolTestHarness::cancelAttempt(handler);
	ServerHandlerProtocolTestHarness::deliverControlEnvelope(handler, staleEnvelope);
	QTRY_COMPARE(cancellationSpy.count(), 1);
	QCoreApplication::processEvents();
	QCOMPARE(receiver.dispatchedMessageTypes.size(), 2);
	QCOMPARE(envelopeSpy.count(), 2);

	const adapters::UdpTransportEvent transportEvent { adapters::UdpTransportState::Degraded,
		QStringLiteral("Queued transport event") };
	ServerHandlerProtocolTestHarness::deliverUdpTransportEvent(handler, transportEvent);
	QTRY_COMPARE(receiver.presentedTransportEvents.size(), 1);
	QCOMPARE(receiver.presentedTransportEvents.at(0).state, transportEvent.state);
	QCOMPARE(receiver.presentedTransportEvents.at(0).message, transportEvent.message);
	QCOMPARE(receiver.presentedOn, QCoreApplication::instance()->thread());
}

QTEST_MAIN(TestServerHandlerProtocolComposition)

#include "TestServerHandlerProtocolComposition.moc"
