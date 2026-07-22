// Copyright The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#include "adapters/ControlMessageEnvelope.h"
#include "adapters/ProtocolEventAdapter.h"
#include "adapters/ProtocolMessageReceiver.h"

#include <QtTest/QtTest>

#include <QVector>

#include "Mumble.pb.h"
#include "MumbleProtocol.h"

#include <limits>
#include <string>

namespace {

class ProtocolMessageReceiverSpy : public mumble::modern::adapters::ProtocolMessageReceiver {
public:
#define PROCESS_MUMBLE_TCP_MESSAGE(name, value) \
	void dispatch(const MumbleProto::name &) override { \
		++dispatchCount; \
		lastMessageType = Mumble::Protocol::TCPMessageType::name; \
		dispatchedMessageTypes.append(lastMessageType); \
	}
	MUMBLE_ALL_TCP_MESSAGES
#undef PROCESS_MUMBLE_TCP_MESSAGE

	int dispatchCount = 0;
	Mumble::Protocol::TCPMessageType lastMessageType = Mumble::Protocol::TCPMessageType::Version;
	QVector< Mumble::Protocol::TCPMessageType > dispatchedMessageTypes;
};

class ProtocolEnvelopeProducer : public QObject {
	Q_OBJECT

signals:
	void attemptStarted(mumble::modern::contracts::ConnectionAttemptId attempt);
	void attemptCancelled(mumble::modern::contracts::ConnectionAttemptId attempt);
	void envelopeReceived(const mumble::modern::adapters::ControlMessageEnvelope &envelope);
};

} // namespace

class TestModernProtocolAdapter : public QObject {
	Q_OBJECT

private slots:
	void dispatchesValidControlProtobufToInstalledReceiver();
	void diagnosesMalformedControlProtobufWithoutReceiverDispatch();
	void diagnosesOversizeControlProtobufWithoutReceiverDispatch();
	void preservesControlDeliveryOrderAndDiscardsCancelledAttempt();
};

void TestModernProtocolAdapter::dispatchesValidControlProtobufToInstalledReceiver() {
	using namespace mumble::modern;
	using namespace mumble::modern::contracts;

	MumbleProto::Version version;
	version.set_version(0x010500);
	std::string serializedVersion;
	QVERIFY(version.SerializeToString(&serializedVersion));

	ProtocolMessageReceiverSpy receiver;
	int callbackCount = 0;
	adapters::ProtocolEventAdapter adapter([&callbackCount](const adapters::ControlMessageEnvelope &) { ++callbackCount; });
	adapter.setReceiver(&receiver);
	const ConnectionAttemptId attempt { 8 };
	adapter.setActiveAttempt(attempt);

	QVERIFY(adapter.receive(adapters::ControlMessageEnvelope(
		static_cast< quint32 >(Mumble::Protocol::TCPMessageType::Version),
		QByteArray::fromStdString(serializedVersion), attempt, 1)));
	QCOMPARE(receiver.dispatchCount, 1);
	QCOMPARE(static_cast< quint32 >(receiver.lastMessageType),
		 static_cast< quint32 >(Mumble::Protocol::TCPMessageType::Version));
	QCOMPARE(callbackCount, 1);
}

void TestModernProtocolAdapter::diagnosesMalformedControlProtobufWithoutReceiverDispatch() {
	using namespace mumble::modern;
	using namespace mumble::modern::contracts;

	ProtocolMessageReceiverSpy receiver;
	int callbackCount = 0;
	adapters::ProtocolEventAdapter adapter([&callbackCount](const adapters::ControlMessageEnvelope &) { ++callbackCount; });
	adapter.setReceiver(&receiver);
	QSignalSpy diagnosticSpy(&adapter, &adapters::ProtocolEventAdapter::protocolDiagnostic);
	const ConnectionAttemptId attempt { 8 };
	adapter.setActiveAttempt(attempt);

	QVERIFY(!adapter.receive(adapters::ControlMessageEnvelope(
		static_cast< quint32 >(Mumble::Protocol::TCPMessageType::Version), QByteArray::fromHex("80"), attempt, 1)));
	QCOMPARE(receiver.dispatchCount, 0);
	QCOMPARE(callbackCount, 0);
	QCOMPARE(diagnosticSpy.count(), 1);
	const adapters::ProtocolDiagnostic diagnostic = qvariant_cast< adapters::ProtocolDiagnostic >(diagnosticSpy.takeFirst().at(0));
	QCOMPARE(diagnostic.messageType, static_cast< quint32 >(Mumble::Protocol::TCPMessageType::Version));
	QCOMPARE(diagnostic.attempt, attempt);
	QCOMPARE(diagnostic.receiveSequence, 1ULL);
	QVERIFY(diagnostic.reason.contains(QStringLiteral("Version")));
}

void TestModernProtocolAdapter::diagnosesOversizeControlProtobufWithoutReceiverDispatch() {
	using namespace mumble::modern;
	using namespace mumble::modern::contracts;

	ProtocolMessageReceiverSpy receiver;
	int callbackCount = 0;
	adapters::ProtocolEventAdapter adapter([&callbackCount](const adapters::ControlMessageEnvelope &) { ++callbackCount; });
	adapter.setReceiver(&receiver);
	QSignalSpy diagnosticSpy(&adapter, &adapters::ProtocolEventAdapter::protocolDiagnostic);
	const ConnectionAttemptId attempt { 8 };
	adapter.setActiveAttempt(attempt);

	if (std::numeric_limits< qsizetype >::max() <= std::numeric_limits< int >::max()) {
		QSKIP("The platform cannot represent a QByteArray larger than protobuf's int size parameter");
	}
	static const char payloadByte = '\0';
	const QByteArray oversizePayload = QByteArray::fromRawData(
		&payloadByte, static_cast< qsizetype >(std::numeric_limits< int >::max()) + 1);
	QVERIFY(!adapter.receive(adapters::ControlMessageEnvelope(
		static_cast< quint32 >(Mumble::Protocol::TCPMessageType::Version), oversizePayload, attempt, 1)));
	QCOMPARE(receiver.dispatchCount, 0);
	QCOMPARE(callbackCount, 0);
	QCOMPARE(diagnosticSpy.count(), 1);
	const adapters::ProtocolDiagnostic diagnostic = qvariant_cast< adapters::ProtocolDiagnostic >(diagnosticSpy.takeFirst().at(0));
	QCOMPARE(diagnostic.messageType, static_cast< quint32 >(Mumble::Protocol::TCPMessageType::Version));
	QCOMPARE(diagnostic.attempt, attempt);
	QCOMPARE(diagnostic.receiveSequence, 1ULL);
	QVERIFY(diagnostic.reason.contains(QStringLiteral("size limit")));
}

void TestModernProtocolAdapter::preservesControlDeliveryOrderAndDiscardsCancelledAttempt() {
	using namespace mumble::modern;
	using namespace mumble::modern::contracts;

	MumbleProto::Version version;
	version.set_version(0x010500);
	std::string serializedVersion;
	QVERIFY(version.SerializeToString(&serializedVersion));

	MumbleProto::ServerConfig serverConfig;
	serverConfig.set_max_bandwidth(128000);
	std::string serializedServerConfig;
	QVERIFY(serverConfig.SerializeToString(&serializedServerConfig));

	ProtocolMessageReceiverSpy receiver;
	adapters::ProtocolEventAdapter adapter([](const adapters::ControlMessageEnvelope &) {});
	adapter.setReceiver(&receiver);
	ProtocolEnvelopeProducer producer;
	QObject::connect(&producer, &ProtocolEnvelopeProducer::attemptStarted, &adapter,
					 &adapters::ProtocolEventAdapter::setActiveAttempt, Qt::QueuedConnection);
	QObject::connect(&producer, &ProtocolEnvelopeProducer::attemptCancelled, &adapter,
					 &adapters::ProtocolEventAdapter::cancelAttempt, Qt::QueuedConnection);
	QObject::connect(&producer, &ProtocolEnvelopeProducer::envelopeReceived, &adapter,
					 &adapters::ProtocolEventAdapter::receive, Qt::QueuedConnection);
	const ConnectionAttemptId attempt { 9 };
	emit producer.attemptStarted(attempt);

	emit producer.envelopeReceived(adapters::ControlMessageEnvelope(
		static_cast< quint32 >(Mumble::Protocol::TCPMessageType::Version), QByteArray::fromStdString(serializedVersion), attempt, 1));
	emit producer.envelopeReceived(adapters::ControlMessageEnvelope(
		static_cast< quint32 >(Mumble::Protocol::TCPMessageType::ServerConfig),
		QByteArray::fromStdString(serializedServerConfig), attempt, 2));
	QTRY_COMPARE(receiver.dispatchCount, 2);
	QCOMPARE(receiver.dispatchedMessageTypes.size(), 2);
	QCOMPARE(static_cast< quint32 >(receiver.dispatchedMessageTypes.at(0)),
		 static_cast< quint32 >(Mumble::Protocol::TCPMessageType::Version));
	QCOMPARE(static_cast< quint32 >(receiver.dispatchedMessageTypes.at(1)),
		 static_cast< quint32 >(Mumble::Protocol::TCPMessageType::ServerConfig));

	emit producer.attemptCancelled(attempt);
	emit producer.envelopeReceived(adapters::ControlMessageEnvelope(
		static_cast< quint32 >(Mumble::Protocol::TCPMessageType::Version), QByteArray::fromStdString(serializedVersion), attempt, 3));
	QTest::qWait(20);
	QCOMPARE(receiver.dispatchCount, 2);
}

QTEST_MAIN(TestModernProtocolAdapter)

#include "TestModernProtocolAdapter.moc"
