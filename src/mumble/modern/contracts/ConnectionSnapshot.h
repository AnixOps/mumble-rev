// Copyright The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#pragma once

#include "ClientTypes.h"

namespace mumble::modern::contracts {

enum class ConnectionPhase {
	Idle,
	Resolving,
	ConnectingTransport,
	NegotiatingTls,
	AwaitingCertificateDecision,
	Authenticating,
	Synchronizing,
	Connected,
	Disconnecting,
	Reconnecting,
	Failed,
};

enum class TransportMode {
	Unknown,
	Tcp,
	Udp,
	TcpTunnel,
};

struct TransportMetric {
	bool isAvailable = false;
	double value = 0.0;
};

struct TransportHealth {
	TransportMode mode = TransportMode::Unknown;
	bool localUdpAvailable = false;
	bool remoteUdpAvailable = false;
	TransportMetric tcpLatency;
	TransportMetric udpLatency;
	TransportMetric latencyVariance;
	quint64 tcpPacketsLost = 0;
	quint64 udpPacketsLost = 0;
	QDateTime lastUpdated;
};

struct SynchronizationProgress {
	quint32 completedSteps = 0;
	quint32 totalSteps = 0;
};

struct ConnectionSnapshot {
	ConnectionAttemptId attempt;
	std::optional< ConnectionEpoch > epoch;
	ConnectionPhase phase = ConnectionPhase::Idle;
	ServerTarget target;
	quint32 reconnectAttempt = 0;
	quint32 maximumReconnectAttempts = 0;
	bool cancellationAllowed = false;
	SynchronizationProgress synchronization;
	std::optional< CertificateChallengeId > certificateChallenge;
	std::optional< ClientError > lastError;
	TransportHealth transportHealth;
};

} // namespace mumble::modern::contracts
