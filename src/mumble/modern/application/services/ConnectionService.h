// Copyright The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#pragma once

#include "application/stores/ConnectionStore.h"
#include "contracts/ConnectionEvent.h"

namespace mumble::modern::application {

class ConnectionService {
public:
	explicit ConnectionService(ConnectionStore &connectionStore);

	bool handle(const contracts::ConnectionAttemptStarted &event);
	bool handle(const contracts::ConnectionPhaseChanged &event);
	bool handle(const contracts::ConnectionSynchronized &event);
	bool handle(const contracts::ConnectionFailed &event);

private:
	ConnectionStore &m_connectionStore;
};

} // namespace mumble::modern::application
