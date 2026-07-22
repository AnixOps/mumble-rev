// Copyright The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#pragma once

#include "ConnectionSnapshot.h"

namespace mumble::modern::contracts {

struct ConnectionAttemptStarted {
	ConnectionAttemptId attempt;
	quint64 sequence = 0;
	ServerTarget target;
};

struct ConnectionPhaseChanged {
	ConnectionAttemptId attempt;
	quint64 sequence = 0;
	ConnectionPhase phase = ConnectionPhase::Idle;
	bool cancellationAllowed = false;
};

struct ConnectionSynchronized {
	ConnectionAttemptId attempt;
	quint64 sequence = 0;
	ConnectionEpoch epoch;
};

struct ConnectionFailed {
	ConnectionAttemptId attempt;
	quint64 sequence = 0;
	ClientError error;
};

} // namespace mumble::modern::contracts
