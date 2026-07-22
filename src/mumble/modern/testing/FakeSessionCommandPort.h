// Copyright The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#pragma once

#include "contracts/ports/SessionCommandPort.h"

#include <QVector>

namespace mumble::modern::testing {

struct JoinChannelCall {
	contracts::ConnectionEpoch epoch;
	contracts::ChannelId channel;
	bool operator==(const JoinChannelCall &other) const { return epoch == other.epoch && channel == other.channel; }
};

struct SetSelfMuteDeafCall {
	contracts::ConnectionEpoch epoch;
	bool mute = false;
	bool deaf = false;
	bool operator==(const SetSelfMuteDeafCall &other) const {
		return epoch == other.epoch && mute == other.mute && deaf == other.deaf;
	}
};

struct SetUserVolumeCall {
	contracts::ConnectionEpoch epoch;
	contracts::UserSessionId user;
	float value = 0.0F;
	bool operator==(const SetUserVolumeCall &other) const {
		return epoch == other.epoch && user == other.user && value == other.value;
	}
};

class FakeSessionCommandPort : public contracts::SessionCommandPort {
public:
	QVector< JoinChannelCall > joinChannelCalls;
	QVector< SetSelfMuteDeafCall > setSelfMuteDeafCalls;
	QVector< SetUserVolumeCall > setUserVolumeCalls;
	QVector< contracts::SendChannelMessage > channelMessages;
	QVector< contracts::SendPrivateMessage > privateMessages;

	void joinChannel(contracts::ConnectionEpoch epoch, contracts::ChannelId channel) override {
		joinChannelCalls.append({ epoch, channel });
	}
	void setSelfMuteDeaf(contracts::ConnectionEpoch epoch, bool mute, bool deaf) override {
		setSelfMuteDeafCalls.append({ epoch, mute, deaf });
	}
	void setUserVolume(contracts::ConnectionEpoch epoch, contracts::UserSessionId user, float value) override {
		setUserVolumeCalls.append({ epoch, user, value });
	}
	void sendChannelMessage(const contracts::SendChannelMessage &command) override { channelMessages.append(command); }
	void sendPrivateMessage(const contracts::SendPrivateMessage &command) override { privateMessages.append(command); }
};

} // namespace mumble::modern::testing
