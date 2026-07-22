// Copyright The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#pragma once

#include "contracts/ClientTypes.h"

#include <QByteArray>
#include <QMetaType>

#include <utility>

namespace mumble::modern::adapters {

class ControlMessageEnvelope {
public:
	ControlMessageEnvelope(quint32 messageType, QByteArray payload, contracts::ConnectionAttemptId attempt,
		quint64 receiveSequence)
		: m_messageType(messageType), m_payload(std::move(payload)), m_attempt(attempt), m_receiveSequence(receiveSequence) {}

	quint32 messageType() const { return m_messageType; }
	QByteArray payload() const { return m_payload; }
	contracts::ConnectionAttemptId attempt() const { return m_attempt; }
	quint64 receiveSequence() const { return m_receiveSequence; }

private:
	const quint32 m_messageType;
	const QByteArray m_payload;
	const contracts::ConnectionAttemptId m_attempt;
	const quint64 m_receiveSequence;
};

} // namespace mumble::modern::adapters

Q_DECLARE_METATYPE(mumble::modern::adapters::ControlMessageEnvelope)
