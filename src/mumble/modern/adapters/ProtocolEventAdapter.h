// Copyright The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#pragma once

#include "ControlMessageEnvelope.h"

#include <QObject>
#include <QMetaType>
#include <QString>

#include <functional>
#include <optional>

class QThread;

namespace mumble::modern::adapters {

class LegacyProtocolReceiver;

struct ProtocolDiagnostic {
	quint32 messageType;
	contracts::ConnectionAttemptId attempt;
	quint64 receiveSequence;
	QString reason;
};

class ProtocolEventAdapter : public QObject {
	Q_OBJECT

public:
	using ParsedMessageCallback = std::function< void(const ControlMessageEnvelope &) >;

	explicit ProtocolEventAdapter(ParsedMessageCallback parsedMessageCallback, QObject *parent = nullptr);

	void setActiveAttempt(contracts::ConnectionAttemptId attempt);
	void cancelAttempt(contracts::ConnectionAttemptId attempt);
	void setLegacyReceiver(LegacyProtocolReceiver *legacyReceiver);
	bool receive(const ControlMessageEnvelope &envelope);

signals:
	void protocolDiagnostic(const ProtocolDiagnostic &diagnostic);

private:
	bool parseAndDispatch(const ControlMessageEnvelope &envelope);
	void reportDiagnostic(const ControlMessageEnvelope &envelope, const QString &reason);
	void assertOnOwnerThread() const;

	const QThread *const m_ownerThread;
	ParsedMessageCallback m_parsedMessageCallback;
	LegacyProtocolReceiver *m_legacyReceiver = nullptr;
	std::optional< contracts::ConnectionAttemptId > m_activeAttempt;
	std::optional< quint64 > m_lastAcceptedSequence;
};

} // namespace mumble::modern::adapters

Q_DECLARE_METATYPE(mumble::modern::adapters::ProtocolDiagnostic)
