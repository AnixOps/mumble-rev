// Copyright The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#pragma once

#include "Mumble.pb.h"
#include "MumbleProtocol.h"

class MainWindow;

namespace mumble::modern::adapters {

class LegacyProtocolReceiver {
public:
	explicit LegacyProtocolReceiver(MainWindow &mainWindow);

#define PROCESS_MUMBLE_TCP_MESSAGE(name, value) virtual void dispatch(const MumbleProto::name &message);
	MUMBLE_ALL_TCP_MESSAGES
#undef PROCESS_MUMBLE_TCP_MESSAGE

private:
	MainWindow &m_mainWindow;
};

} // namespace mumble::modern::adapters
