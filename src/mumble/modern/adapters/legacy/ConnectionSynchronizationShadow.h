// Copyright The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#pragma once

#include "../ControlMessageEnvelope.h"

#include <optional>

namespace mumble::modern::adapters {

enum class ShadowConnectionPhase {
	Synchronizing,
	Synchronized,
	Rejected,
};

struct ConnectionSynchronizationComparison {
	contracts::ConnectionAttemptId attempt;
	ShadowConnectionPhase phase;
	bool expectedSynchronized;
	bool legacySynchronized;

	bool matches() const {
		return expectedSynchronized == legacySynchronized;
	}
};

// This model is deliberately read-only. It projects only connection and sync
// facts already present in inbound control messages for comparison with legacy.
class ConnectionSynchronizationShadow {
public:
	void startAttempt(contracts::ConnectionAttemptId attempt);
	void cancelAttempt(contracts::ConnectionAttemptId attempt);
	std::optional< ConnectionSynchronizationComparison > observeAcceptedControlMessage(
		const ControlMessageEnvelope &envelope, bool legacySynchronized);

	std::optional< ConnectionSynchronizationComparison > lastComparison() const;

private:
	std::optional< contracts::ConnectionAttemptId > m_activeAttempt;
	ShadowConnectionPhase m_phase = ShadowConnectionPhase::Synchronizing;
	std::optional< ConnectionSynchronizationComparison > m_lastComparison;
};

} // namespace mumble::modern::adapters
