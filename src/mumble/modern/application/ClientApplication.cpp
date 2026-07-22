// Copyright The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#include "ClientApplication.h"

namespace mumble::modern::application {

ClientApplication::ClientApplication(contracts::ConnectionPort &connectionPort,
	contracts::SessionCommandPort &sessionCommandPort)
	: m_connectionPort(connectionPort), m_sessionCommandPort(sessionCommandPort) {}

ConnectionStore &ClientApplication::connectionStore() {
	return m_connectionStore;
}

const ConnectionStore &ClientApplication::connectionStore() const {
	return m_connectionStore;
}

contracts::ConnectionPort &ClientApplication::connectionPort() {
	return m_connectionPort;
}

contracts::SessionCommandPort &ClientApplication::sessionCommandPort() {
	return m_sessionCommandPort;
}

} // namespace mumble::modern::application
