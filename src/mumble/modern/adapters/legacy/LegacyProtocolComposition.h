// Copyright The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#pragma once

#include "LegacyProtocolReceiver.h"
#include "../ProtocolEventAdapter.h"

class MainWindow;
class ServerHandler;

namespace mumble::modern::adapters {

class LegacyProtocolComposition {
public:
	explicit LegacyProtocolComposition(MainWindow &mainWindow);
	void attach(ServerHandler &serverHandler);

private:
	LegacyProtocolReceiver m_receiver;
	ProtocolEventAdapter m_adapter;
};

} // namespace mumble::modern::adapters
