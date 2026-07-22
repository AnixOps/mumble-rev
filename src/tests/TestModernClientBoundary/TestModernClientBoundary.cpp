// Copyright The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#include "ModernClientBootstrap.h"
#include "application/ClientApplication.h"
#include "application/stores/ConnectionStore.h"
#include "contracts/ClientTypes.h"
#include "contracts/ConnectionSnapshot.h"
#include "presentation/ConnectionPresentationProbe.h"
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
	void connectionStoreStartsIdleAndReplacesSnapshotOnItsConstructionThread();
	void clientApplicationAndBootstrapUseInjectedPorts();
	void presentationProbeObservesConnectionPhase();
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
	QVERIFY(!snapshot.transportHealth.tcpPacketsLost.has_value());
	QVERIFY(!snapshot.transportHealth.udpPacketsLost.has_value());
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
	const QVector< ConnectionAttemptId > expectedCancelledAttempts { ConnectionAttemptId { 3 } };
	const QVector< ConnectionEpoch > expectedDisconnectedEpochs { ConnectionEpoch { 7 } };
	QVERIFY(port.cancelledAttempts == expectedCancelledAttempts);
	QVERIFY(port.disconnectedEpochs == expectedDisconnectedEpochs);
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

	const QVector< testing::JoinChannelCall > expectedJoinChannelCalls { { epoch, ChannelId { 2 } } };
	const QVector< testing::SetSelfMuteDeafCall > expectedSetSelfMuteDeafCalls { { epoch, true, false } };
	const QVector< testing::SetUserVolumeCall > expectedSetUserVolumeCalls { { epoch, UserSessionId { 5 }, 0.75F } };
	const QVector< SendChannelMessage > expectedChannelMessages { channelMessage };
	const QVector< SendPrivateMessage > expectedPrivateMessages { privateMessage };
	QVERIFY(port.joinChannelCalls == expectedJoinChannelCalls);
	QVERIFY(port.setSelfMuteDeafCalls == expectedSetSelfMuteDeafCalls);
	QVERIFY(port.setUserVolumeCalls == expectedSetUserVolumeCalls);
	QVERIFY(port.channelMessages == expectedChannelMessages);
	QVERIFY(port.privateMessages == expectedPrivateMessages);
}

void TestModernClientBoundary::connectionStoreStartsIdleAndReplacesSnapshotOnItsConstructionThread() {
	using namespace mumble::modern;
	using namespace mumble::modern::contracts;

	application::ConnectionStore store;
	QCOMPARE(store.snapshot().phase, ConnectionPhase::Idle);

	ConnectionSnapshot replacement;
	replacement.phase = ConnectionPhase::ConnectingTransport;
	store.replaceSnapshot(replacement);

	QCOMPARE(store.snapshot().phase, ConnectionPhase::ConnectingTransport);
}

void TestModernClientBoundary::clientApplicationAndBootstrapUseInjectedPorts() {
	using namespace mumble::modern;

	testing::FakeConnectionPort connectionPort;
	testing::FakeSessionCommandPort sessionCommandPort;
	application::ClientApplication application(connectionPort, sessionCommandPort);
	QCOMPARE(&application.connectionPort(), &connectionPort);
	QCOMPARE(&application.sessionCommandPort(), &sessionCommandPort);
	QCOMPARE(application.connectionStore().snapshot().phase, contracts::ConnectionPhase::Idle);

	ModernClientBootstrap bootstrap(connectionPort, sessionCommandPort);
	QCOMPARE(&bootstrap.application().connectionPort(), &connectionPort);
	QCOMPARE(&bootstrap.application().sessionCommandPort(), &sessionCommandPort);
}

void TestModernClientBoundary::presentationProbeObservesConnectionPhase() {
	using namespace mumble::modern;
	using namespace mumble::modern::contracts;

	testing::FakeConnectionPort connectionPort;
	testing::FakeSessionCommandPort sessionCommandPort;
	application::ClientApplication application(connectionPort, sessionCommandPort);
	presentation::ConnectionPresentationProbe probe(application.connectionStore());
	QSignalSpy phaseChangedSpy(&probe, &presentation::ConnectionPresentationProbe::connectionPhaseChanged);

	QCOMPARE(probe.connectionPhase(), ConnectionPhase::Idle);
	ConnectionSnapshot snapshot;
	snapshot.phase = ConnectionPhase::Synchronizing;
	application.connectionStore().replaceSnapshot(snapshot);
	QCOMPARE(probe.connectionPhase(), ConnectionPhase::Synchronizing);
	QCOMPARE(phaseChangedSpy.count(), 1);
}

QTEST_MAIN(TestModernClientBoundary)

#include "TestModernClientBoundary.moc"
