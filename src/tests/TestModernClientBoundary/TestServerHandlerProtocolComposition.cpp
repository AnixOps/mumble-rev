// Copyright The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#include "ServerHandler.h"
#include "modern/adapters/ProtocolMessageReceiver.h"
#include "modern/adapters/legacy/LegacyProtocolComposition.h"

#include <QtTest/QtTest>

#include <QCoreApplication>
#include <QMetaType>
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

	QVERIFY(QMetaType::fromName("mumble::modern::contracts::ConnectionAttemptId").isValid());
	QVERIFY(QMetaType::fromName("mumble::modern::adapters::ControlMessageEnvelope").isValid());
	QVERIFY(QMetaType::fromName("mumble::modern::adapters::UdpTransportEvent").isValid());

	QSignalSpy attemptSpy(&handler, &ServerHandler::connectionAttemptStarted);
	QSignalSpy envelopeSpy(&handler, &ServerHandler::controlMessageReceived);
	QVERIFY(attemptSpy.isValid());
	QVERIFY(envelopeSpy.isValid());

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

	ServerHandlerProtocolTestHarness::cancelAttempt(handler);
	ServerHandlerProtocolTestHarness::receiveControlMessage(
		handler, Mumble::Protocol::TCPMessageType::Version, QByteArray::fromStdString(serializedVersion));
	QTest::qWait(20);
	QCOMPARE(receiver.dispatchedMessageTypes.size(), 2);
	QCOMPARE(envelopeSpy.count(), 2);
}

QTEST_MAIN(TestServerHandlerProtocolComposition)

#include "TestServerHandlerProtocolComposition.moc"
