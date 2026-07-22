// Copyright The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#include "ProtocolEventAdapter.h"

#include "ProtocolMessageReceiver.h"

#include "Mumble.pb.h"
#include "MumbleProtocol.h"

#include <QThread>
#include <QtGlobal>

#include <limits>
#include <utility>

namespace mumble::modern::adapters {

ProtocolEventAdapter::ProtocolEventAdapter(ParsedMessageCallback parsedMessageCallback, QObject *parent)
	: QObject(parent), m_ownerThread(QThread::currentThread()), m_parsedMessageCallback(std::move(parsedMessageCallback)) {
	qRegisterMetaType< ControlMessageEnvelope >("mumble::modern::adapters::ControlMessageEnvelope");
	qRegisterMetaType< ProtocolDiagnostic >("mumble::modern::adapters::ProtocolDiagnostic");
	qRegisterMetaType< UdpTransportEvent >("mumble::modern::adapters::UdpTransportEvent");
}

void ProtocolEventAdapter::setActiveAttempt(contracts::ConnectionAttemptId attempt) {
	assertOnOwnerThread();
	m_activeAttempt = attempt;
	m_lastAcceptedSequence.reset();
}

void ProtocolEventAdapter::cancelAttempt(contracts::ConnectionAttemptId attempt) {
	assertOnOwnerThread();
	if (m_activeAttempt.has_value() && m_activeAttempt.value() == attempt) {
		m_activeAttempt.reset();
		m_lastAcceptedSequence.reset();
		emit attemptCancelled(attempt);
	}
}

void ProtocolEventAdapter::setReceiver(ProtocolMessageReceiver *receiver) {
	assertOnOwnerThread();
	m_receiver = receiver;
}

bool ProtocolEventAdapter::receive(const ControlMessageEnvelope &envelope) {
	assertOnOwnerThread();
	if (!m_activeAttempt.has_value() || envelope.attempt() != m_activeAttempt.value()) {
		return false;
	}
	if (m_lastAcceptedSequence.has_value() && envelope.receiveSequence() <= m_lastAcceptedSequence.value()) {
		return false;
	}

	m_lastAcceptedSequence = envelope.receiveSequence();
	return parseAndDispatch(envelope);
}

bool ProtocolEventAdapter::parseAndDispatch(const ControlMessageEnvelope &envelope) {
#define PROCESS_MUMBLE_TCP_MESSAGE(name, value)                                                                  \
	case static_cast< quint32 >(Mumble::Protocol::TCPMessageType::name): {                                        \
		MumbleProto::name message;                                                                                  \
		const qsizetype payloadSize = envelope.payload().size();                                                    \
		if (payloadSize > std::numeric_limits< int >::max()) {                                                      \
			reportDiagnostic(envelope, QStringLiteral("Control message payload exceeds protobuf parser size limit"));   \
			return false;                                                                                              \
		}                                                                                                            \
		if (!message.ParseFromArray(envelope.payload().constData(), static_cast< int >(payloadSize))) {              \
			reportDiagnostic(envelope, QStringLiteral("Unable to parse ") + QStringLiteral(#name) + QStringLiteral(" control message")); \
			return false;                                                                                              \
		}                                                                                                            \
		if (m_receiver != nullptr) {                                                                                \
			m_receiver->dispatch(message);                                                                            \
		}                                                                                                            \
		if (m_parsedMessageCallback) {                                                                              \
			m_parsedMessageCallback(envelope);                                                                        \
		}                                                                                                            \
		return true;                                                                                                 \
	}

	switch (envelope.messageType()) { MUMBLE_ALL_TCP_MESSAGES }
#undef PROCESS_MUMBLE_TCP_MESSAGE

	reportDiagnostic(envelope, QStringLiteral("Unknown TCP control message type"));
	return false;
}

void ProtocolEventAdapter::reportDiagnostic(const ControlMessageEnvelope &envelope, const QString &reason) {
	emit protocolDiagnostic({ envelope.messageType(), envelope.attempt(), envelope.receiveSequence(), reason });
}

void ProtocolEventAdapter::assertOnOwnerThread() const {
	Q_ASSERT(QThread::currentThread() == m_ownerThread);
}

} // namespace mumble::modern::adapters
