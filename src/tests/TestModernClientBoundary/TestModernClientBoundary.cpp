// Copyright The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#include "contracts/ClientTypes.h"
#include "contracts/ConnectionSnapshot.h"
#include "testing/FakeConnectionPort.h"
#include "testing/FakeSessionCommandPort.h"

#include <QtTest/QtTest>

class TestModernClientBoundary : public QObject {
	Q_OBJECT

private slots:
	void strongIdsIncludeTheirValueAndConnectionEpoch();
	void connectionSnapshotStartsIdleAndEmpty();
	void fakeConnectionPortOnlyRecordsCalls();
	void fakeSessionCommandPortOnlyRecordsCalls();
};

void TestModernClientBoundary::strongIdsIncludeTheirValueAndConnectionEpoch() {
	using namespace mumble::modern::contracts;

	QVERIFY(ConnectionAttemptId { 4 } == ConnectionAttemptId { 4 });
	QVERIFY(ConnectionAttemptId { 4 } != ConnectionAttemptId { 5 });
	QVERIFY(ChannelId { 9 } == ChannelId { 9 });
	QVERIFY(ChannelId { 9 } != ChannelId { 10 });

	const EntityKey first { ConnectionEpoch { 1 }, ChannelId { 9 } };
	const EntityKey same { ConnectionEpoch { 1 }, ChannelId { 9 } };
	const EntityKey replacementSession { ConnectionEpoch { 2 }, ChannelId { 9 } };
	QVERIFY(first == same);
	QVERIFY(first != replacementSession);
}

void TestModernClientBoundary::connectionSnapshotStartsIdleAndEmpty() {
	using namespace mumble::modern::contracts;

	const ConnectionSnapshot snapshot;
	QVERIFY(snapshot.phase == ConnectionPhase::Idle);
	QVERIFY(!snapshot.epoch.has_value());
	QCOMPARE(snapshot.reconnectAttempt, 0U);
	QCOMPARE(snapshot.maximumReconnectAttempts, 0U);
	QVERIFY(!snapshot.cancellationAllowed);
	QVERIFY(!snapshot.certificateChallenge.has_value());
	QVERIFY(!snapshot.lastError.has_value());
	QVERIFY(!snapshot.transportHealth.tcpLatency.isAvailable);
	QVERIFY(!snapshot.transportHealth.udpLatency.isAvailable);
}

void TestModernClientBoundary::fakeConnectionPortOnlyRecordsCalls() {
	using namespace mumble::modern;
	using namespace mumble::modern::contracts;

	testing::FakeConnectionPort port;
	const ConnectionRequest request { ConnectionAttemptId { 3 }, { QStringLiteral("Test server"),
		QStringLiteral("mumble.example.test"), 64738 }, QStringLiteral("alice"), QStringLiteral("secret") };

	port.connectTo(request);
	port.cancel(ConnectionAttemptId { 3 });
	port.disconnect(ConnectionEpoch { 7 });
	port.acceptCertificate(CertificateChallengeId { QUuid::createUuid() });
	port.rejectCertificate(CertificateChallengeId { QUuid::createUuid() });

	QCOMPARE(port.connectionRequests.size(), 1);
	QVERIFY(port.connectionRequests.constFirst() == request);
	QVERIFY(port.cancelledAttempts == QVector< ConnectionAttemptId > { ConnectionAttemptId { 3 } });
	QVERIFY(port.disconnectedEpochs == QVector< ConnectionEpoch > { ConnectionEpoch { 7 } });
	QCOMPARE(port.acceptedCertificateChallenges.size(), 1);
	QCOMPARE(port.rejectedCertificateChallenges.size(), 1);
}

void TestModernClientBoundary::fakeSessionCommandPortOnlyRecordsCalls() {
	using namespace mumble::modern;
	using namespace mumble::modern::contracts;

	testing::FakeSessionCommandPort port;
	const ConnectionEpoch epoch { 12 };
	const SendChannelMessage channelMessage { epoch, ChannelId { 2 }, ClientMessageId { QUuid::createUuid() },
		QStringLiteral("hello") };
	const SendPrivateMessage privateMessage { epoch, UserSessionId { 5 }, ClientMessageId { QUuid::createUuid() },
		QStringLiteral("private") };

	port.joinChannel(epoch, ChannelId { 2 });
	port.setSelfMuteDeaf(epoch, true, false);
	port.setUserVolume(epoch, UserSessionId { 5 }, 0.75F);
	port.sendChannelMessage(channelMessage);
	port.sendPrivateMessage(privateMessage);

	QVERIFY(port.joinChannelCalls == QVector< testing::JoinChannelCall > { { epoch, ChannelId { 2 } } });
	QVERIFY(port.setSelfMuteDeafCalls == QVector< testing::SetSelfMuteDeafCall > { { epoch, true, false } });
	QVERIFY(port.setUserVolumeCalls == QVector< testing::SetUserVolumeCall > { { epoch, UserSessionId { 5 }, 0.75F } });
	QVERIFY(port.channelMessages == QVector< SendChannelMessage > { channelMessage });
	QVERIFY(port.privateMessages == QVector< SendPrivateMessage > { privateMessage });
}

QTEST_MAIN(TestModernClientBoundary)

#include "TestModernClientBoundary.moc"
