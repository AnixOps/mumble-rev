// Copyright The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#include "ConnectionSynchronizationShadow.h"

#include "MumbleProtocol.h"

namespace mumble::modern::adapters {

void ConnectionSynchronizationShadow::startAttempt(contracts::ConnectionAttemptId attempt) {
	m_activeAttempt = attempt;
	m_phase         = ShadowConnectionPhase::Synchronizing;
	m_lastComparison.reset();
}

void ConnectionSynchronizationShadow::cancelAttempt(contracts::ConnectionAttemptId attempt) {
	if (m_activeAttempt.has_value() && m_activeAttempt.value() == attempt) {
		m_activeAttempt.reset();
		m_lastComparison.reset();
	}
}

std::optional< ConnectionSynchronizationComparison > ConnectionSynchronizationShadow::observeAcceptedControlMessage(
	const ControlMessageEnvelope &envelope, bool legacySynchronized) {
	if (!m_activeAttempt.has_value() || envelope.attempt() != m_activeAttempt.value()) {
		return std::nullopt;
	}

	if (envelope.messageType() == static_cast< quint32 >(Mumble::Protocol::TCPMessageType::ServerSync)) {
		m_phase = ShadowConnectionPhase::Synchronized;
	} else if (envelope.messageType() == static_cast< quint32 >(Mumble::Protocol::TCPMessageType::Reject)) {
		m_phase = ShadowConnectionPhase::Rejected;
	}

	const bool expectedSynchronized = m_phase == ShadowConnectionPhase::Synchronized;
	m_lastComparison = ConnectionSynchronizationComparison {
		envelope.attempt(), m_phase, expectedSynchronized, legacySynchronized
	};
	return m_lastComparison;
}

std::optional< ConnectionSynchronizationComparison > ConnectionSynchronizationShadow::lastComparison() const {
	return m_lastComparison;
}

} // namespace mumble::modern::adapters
