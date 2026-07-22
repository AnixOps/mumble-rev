// Copyright The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#pragma once

#include "../ProtocolMessageReceiver.h"
#include "../ProtocolEventAdapter.h"

class MainWindow;

namespace mumble::modern::adapters {

class LegacyProtocolReceiver : public ProtocolMessageReceiver {
public:
	explicit LegacyProtocolReceiver(MainWindow &mainWindow);

#define PROCESS_MUMBLE_TCP_MESSAGE(name, value) void dispatch(const MumbleProto::name &message) override;
	MUMBLE_ALL_TCP_MESSAGES
#undef PROCESS_MUMBLE_TCP_MESSAGE
	void present(const UdpTransportEvent &event) override;

private:
	MainWindow &m_mainWindow;
};

} // namespace mumble::modern::adapters
