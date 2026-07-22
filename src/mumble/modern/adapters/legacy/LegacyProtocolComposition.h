// Copyright The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#pragma once

#include "LegacyProtocolReceiver.h"
#include "../ProtocolEventAdapter.h"

#include <memory>

class MainWindow;
class ServerHandler;

namespace mumble::modern::adapters {

class LegacyProtocolComposition {
public:
	explicit LegacyProtocolComposition(MainWindow &mainWindow);
	explicit LegacyProtocolComposition(ProtocolMessageReceiver &receiver);
	void attach(ServerHandler &serverHandler);

private:
	std::unique_ptr< LegacyProtocolReceiver > m_legacyReceiver;
	ProtocolMessageReceiver &m_receiver;
	ProtocolEventAdapter m_adapter;
};

} // namespace mumble::modern::adapters
