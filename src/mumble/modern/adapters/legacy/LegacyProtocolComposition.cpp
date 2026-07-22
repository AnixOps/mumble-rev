// Copyright The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#include "LegacyProtocolComposition.h"

#include "../../../MainWindow.h"
#include "../../../ServerHandler.h"

#include <QDebug>

#include <utility>

namespace mumble::modern::adapters {

LegacyProtocolComposition::LegacyProtocolComposition(MainWindow &mainWindow)
	: m_legacyReceiver(std::make_unique< LegacyProtocolReceiver >(mainWindow)), m_receiver(*m_legacyReceiver),
	  m_legacySynchronizationReader([this] { return m_legacySynchronized; }),
	  m_adapter([this](const ControlMessageEnvelope &envelope) { compareShadowState(envelope); }) {
	m_adapter.setReceiver(&m_receiver);
	QObject::connect(&m_adapter, &ProtocolEventAdapter::protocolDiagnostic, &m_adapter,
		[this](const ProtocolDiagnostic &diagnostic) { reportProtocolDiagnostic(diagnostic); });
	QObject::connect(&mainWindow, &MainWindow::serverSynchronized, &m_adapter,
		[this] { m_legacySynchronized = true; });
}

LegacyProtocolComposition::LegacyProtocolComposition(ProtocolMessageReceiver &receiver,
	std::function< bool() > legacySynchronizationReader)
	: m_receiver(receiver), m_legacySynchronizationReader(std::move(legacySynchronizationReader)),
	  m_adapter([this](const ControlMessageEnvelope &envelope) { compareShadowState(envelope); }) {
	if (!m_legacySynchronizationReader) {
		m_legacySynchronizationReader = [] { return false; };
	}
	m_adapter.setReceiver(&m_receiver);
	QObject::connect(&m_adapter, &ProtocolEventAdapter::protocolDiagnostic, &m_adapter,
		[this](const ProtocolDiagnostic &diagnostic) { reportProtocolDiagnostic(diagnostic); });
}

void LegacyProtocolComposition::attach(ServerHandler &serverHandler) {
	QObject::connect(&serverHandler, &ServerHandler::connectionAttemptStarted, &m_adapter,
					 &ProtocolEventAdapter::setActiveAttempt, Qt::QueuedConnection);
	QObject::connect(&serverHandler, &ServerHandler::connectionAttemptStarted, &m_adapter,
		[this](contracts::ConnectionAttemptId attempt) {
			m_legacySynchronized = false;
			m_shadow.startAttempt(attempt);
		}, Qt::QueuedConnection);
	QObject::connect(&serverHandler, &ServerHandler::connectionAttemptCancelled, &m_adapter,
					 &ProtocolEventAdapter::cancelAttempt, Qt::QueuedConnection);
	QObject::connect(&serverHandler, &ServerHandler::connectionAttemptCancelled, &m_adapter,
		[this](contracts::ConnectionAttemptId attempt) {
			m_legacySynchronized = false;
			m_shadow.cancelAttempt(attempt);
		}, Qt::QueuedConnection);
	QObject::connect(&serverHandler, &ServerHandler::controlMessageReceived, &m_adapter,
					 &ProtocolEventAdapter::receive, Qt::QueuedConnection);
	QObject::connect(&serverHandler, &ServerHandler::udpTransportEvent, &m_adapter,
					 [this](const UdpTransportEvent &event) { m_receiver.present(event); },
					 Qt::QueuedConnection);
}

ProtocolEventAdapter &LegacyProtocolComposition::protocolAdapterForTesting() {
	return m_adapter;
}

std::optional< ConnectionSynchronizationComparison > LegacyProtocolComposition::shadowComparisonForTesting() const {
	return m_shadow.lastComparison();
}

void LegacyProtocolComposition::compareShadowState(const ControlMessageEnvelope &envelope) {
	const auto comparison = m_shadow.observeAcceptedControlMessage(envelope, m_legacySynchronizationReader());
	if (comparison.has_value() && !comparison->matches()) {
		qWarning().nospace() << "Modern protocol shadow mismatch: attempt=" << comparison->attempt.value
							 << " expectedSynchronized=" << comparison->expectedSynchronized
							 << " legacySynchronized=" << comparison->legacySynchronized;
	}
}

void LegacyProtocolComposition::reportProtocolDiagnostic(const ProtocolDiagnostic &diagnostic) {
	// qWarning is routed through the initialized application logger in production.
	qWarning().nospace() << "Modern protocol diagnostic: type=" << diagnostic.messageType
						 << " attempt=" << diagnostic.attempt.value << " sequence=" << diagnostic.receiveSequence
						 << " reason=" << diagnostic.reason;
}

} // namespace mumble::modern::adapters
