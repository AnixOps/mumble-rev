// Copyright The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#pragma once

#include "../ClientTypes.h"

namespace mumble::modern::contracts {

class SessionCommandPort {
public:
	virtual ~SessionCommandPort() = default;

	virtual void joinChannel(ConnectionEpoch epoch, ChannelId channel) = 0;
	virtual void setSelfMuteDeaf(ConnectionEpoch epoch, bool mute, bool deaf) = 0;
	virtual void setUserVolume(ConnectionEpoch epoch, UserSessionId user, float value) = 0;
	virtual void sendChannelMessage(const SendChannelMessage &command) = 0;
	virtual void sendPrivateMessage(const SendPrivateMessage &command) = 0;
};

} // namespace mumble::modern::contracts
