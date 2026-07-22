// Copyright The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#pragma once

#include "application/ClientApplication.h"

namespace mumble::modern {

class ModernClientBootstrap {
public:
	ModernClientBootstrap(contracts::ConnectionPort &connectionPort, contracts::SessionCommandPort &sessionCommandPort);

	application::ClientApplication &application();
	const application::ClientApplication &application() const;

private:
	application::ClientApplication m_application;
};

} // namespace mumble::modern
