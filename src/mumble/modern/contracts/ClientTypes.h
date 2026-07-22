// Copyright The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#pragma once

#include <QDateTime>
#include <QMetaType>
#include <QString>
#include <QStringList>
#include <QUuid>

#include <optional>
#include <variant>

namespace mumble::modern::contracts {

struct ConnectionAttemptId {
	quint64 value = 0;
	constexpr bool operator==(const ConnectionAttemptId &other) const { return value == other.value; }
	constexpr bool operator!=(const ConnectionAttemptId &other) const { return !(*this == other); }
};

struct ConnectionEpoch {
	quint64 value = 0;
	constexpr bool operator==(const ConnectionEpoch &other) const { return value == other.value; }
	constexpr bool operator!=(const ConnectionEpoch &other) const { return !(*this == other); }
};

struct ChannelId {
	quint32 value = 0;
	constexpr bool operator==(const ChannelId &other) const { return value == other.value; }
	constexpr bool operator!=(const ChannelId &other) const { return !(*this == other); }
};

struct UserSessionId {
	quint32 value = 0;
	constexpr bool operator==(const UserSessionId &other) const { return value == other.value; }
	constexpr bool operator!=(const UserSessionId &other) const { return !(*this == other); }
};

struct ServerBookmarkId {
	QUuid value;
	bool operator==(const ServerBookmarkId &other) const { return value == other.value; }
	bool operator!=(const ServerBookmarkId &other) const { return !(*this == other); }
};

struct ClientMessageId {
	QUuid value;
	bool operator==(const ClientMessageId &other) const { return value == other.value; }
	bool operator!=(const ClientMessageId &other) const { return !(*this == other); }
};

struct CertificateChallengeId {
	QUuid value;
	bool operator==(const CertificateChallengeId &other) const { return value == other.value; }
	bool operator!=(const CertificateChallengeId &other) const { return !(*this == other); }
};

struct AudioLeaseId {
	quint64 value = 0;
	constexpr bool operator==(const AudioLeaseId &other) const { return value == other.value; }
	constexpr bool operator!=(const AudioLeaseId &other) const { return !(*this == other); }
};

struct EntityKey {
	ConnectionEpoch epoch;
	std::variant< ChannelId, UserSessionId > entity;

	EntityKey(ConnectionEpoch connectionEpoch, ChannelId channel) : epoch(connectionEpoch), entity(channel) {}
	EntityKey(ConnectionEpoch connectionEpoch, UserSessionId user) : epoch(connectionEpoch), entity(user) {}

	bool operator==(const EntityKey &other) const { return epoch == other.epoch && entity == other.entity; }
	bool operator!=(const EntityKey &other) const { return !(*this == other); }
};

struct ServerTarget {
	QString name;
	QString address;
	quint16 port = 0;

	bool operator==(const ServerTarget &other) const {
		return name == other.name && address == other.address && port == other.port;
	}
	bool operator!=(const ServerTarget &other) const { return !(*this == other); }
};

struct ConnectionRequest {
	ConnectionAttemptId attempt;
	ServerTarget target;
	QString userName;
	QString password;

	bool operator==(const ConnectionRequest &other) const {
		return attempt == other.attempt && target == other.target && userName == other.userName && password == other.password;
	}
	bool operator!=(const ConnectionRequest &other) const { return !(*this == other); }
};

struct SendChannelMessage {
	ConnectionEpoch epoch;
	ChannelId channel;
	ClientMessageId messageId;
	QString message;

	bool operator==(const SendChannelMessage &other) const {
		return epoch == other.epoch && channel == other.channel && messageId == other.messageId && message == other.message;
	}
	bool operator!=(const SendChannelMessage &other) const { return !(*this == other); }
};

struct SendPrivateMessage {
	ConnectionEpoch epoch;
	UserSessionId user;
	ClientMessageId messageId;
	QString message;

	bool operator==(const SendPrivateMessage &other) const {
		return epoch == other.epoch && user == other.user && messageId == other.messageId && message == other.message;
	}
	bool operator!=(const SendPrivateMessage &other) const { return !(*this == other); }
};

struct ClientError {
	QString code;
	QStringList localizationArguments;
};

} // namespace mumble::modern::contracts

Q_DECLARE_METATYPE(mumble::modern::contracts::ConnectionAttemptId)
