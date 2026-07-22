// Copyright The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#pragma once

#include "Mumble.pb.h"
#include "MumbleProtocol.h"

namespace mumble::modern::adapters {

class ProtocolMessageReceiver {
public:
	virtual ~ProtocolMessageReceiver() = default;

#define PROCESS_MUMBLE_TCP_MESSAGE(name, value) virtual void dispatch(const MumbleProto::name &message) = 0;
	MUMBLE_ALL_TCP_MESSAGES
#undef PROCESS_MUMBLE_TCP_MESSAGE
};

} // namespace mumble::modern::adapters
