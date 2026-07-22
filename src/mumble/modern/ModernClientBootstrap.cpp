// Copyright The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#include "ModernClientBootstrap.h"

namespace mumble::modern {

ModernClientBootstrap::ModernClientBootstrap(contracts::ConnectionPort &connectionPort,
	contracts::SessionCommandPort &sessionCommandPort)
	: m_application(connectionPort, sessionCommandPort) {}

application::ClientApplication &ModernClientBootstrap::application() {
	return m_application;
}

const application::ClientApplication &ModernClientBootstrap::application() const {
	return m_application;
}

} // namespace mumble::modern
