// Copyright The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#pragma once

#include "application/services/ConnectionService.h"
#include "application/stores/ConnectionStore.h"
#include "contracts/ports/ConnectionPort.h"
#include "contracts/ports/SessionCommandPort.h"

namespace mumble::modern::application {

class ClientApplication {
public:
	ClientApplication(contracts::ConnectionPort &connectionPort, contracts::SessionCommandPort &sessionCommandPort);

	ConnectionStore &connectionStore();
	const ConnectionStore &connectionStore() const;
	ConnectionService &connectionService();
	contracts::ConnectionPort &connectionPort();
	contracts::SessionCommandPort &sessionCommandPort();

private:
	contracts::ConnectionPort &m_connectionPort;
	contracts::SessionCommandPort &m_sessionCommandPort;
	ConnectionStore m_connectionStore;
	ConnectionService m_connectionService;
};

} // namespace mumble::modern::application
