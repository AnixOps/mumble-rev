// Copyright The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#include "LegacyProtocolReceiver.h"

#include "../../../MainWindow.h"

namespace mumble::modern::adapters {

LegacyProtocolReceiver::LegacyProtocolReceiver(MainWindow &mainWindow) : m_mainWindow(mainWindow) {}

#define PROCESS_MUMBLE_TCP_MESSAGE(name, value) \
	void LegacyProtocolReceiver::dispatch(const MumbleProto::name &message) { m_mainWindow.msg##name(message); }
	MUMBLE_ALL_TCP_MESSAGES
#undef PROCESS_MUMBLE_TCP_MESSAGE

} // namespace mumble::modern::adapters
