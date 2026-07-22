// Copyright The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#include "adapters/ControlMessageEnvelope.h"
#include "adapters/ProtocolEventAdapter.h"
#include "adapters/ProtocolMessageReceiver.h"

#include <QtTest/QtTest>

#include "Mumble.pb.h"
#include "MumbleProtocol.h"

#include <string>

namespace {

class ProtocolMessageReceiverSpy : public mumble::modern::adapters::ProtocolMessageReceiver {
public:
#define PROCESS_MUMBLE_TCP_MESSAGE(name, value) \
	void dispatch(const MumbleProto::name &) override { ++dispatchCount; lastMessageType = Mumble::Protocol::TCPMessageType::name; }
	MUMBLE_ALL_TCP_MESSAGES
#undef PROCESS_MUMBLE_TCP_MESSAGE

	int dispatchCount = 0;
	Mumble::Protocol::TCPMessageType lastMessageType = Mumble::Protocol::TCPMessageType::Version;
};

} // namespace

class TestModernProtocolAdapter : public QObject {
	Q_OBJECT

private slots:
	void dispatchesValidControlProtobufToInstalledReceiver();
	void diagnosesMalformedControlProtobufWithoutReceiverDispatch();
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

QTEST_MAIN(TestModernProtocolAdapter)

#include "TestModernProtocolAdapter.moc"
