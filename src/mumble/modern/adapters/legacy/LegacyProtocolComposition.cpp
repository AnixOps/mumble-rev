// Copyright The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#include "LegacyProtocolComposition.h"

#include "../../../ServerHandler.h"

namespace mumble::modern::adapters {

LegacyProtocolComposition::LegacyProtocolComposition(MainWindow &mainWindow)
	: m_receiver(mainWindow), m_adapter([](const ControlMessageEnvelope &) {}) {
	m_adapter.setReceiver(&m_receiver);
}

void LegacyProtocolComposition::attach(ServerHandler &serverHandler) {
	QObject::connect(&serverHandler, &ServerHandler::connectionAttemptStarted, &m_adapter,
					 &ProtocolEventAdapter::setActiveAttempt, Qt::QueuedConnection);
	QObject::connect(&serverHandler, &ServerHandler::connectionAttemptCancelled, &m_adapter,
					 &ProtocolEventAdapter::cancelAttempt, Qt::QueuedConnection);
	QObject::connect(&serverHandler, &ServerHandler::controlMessageReceived, &m_adapter,
					 &ProtocolEventAdapter::receive, Qt::QueuedConnection);
	QObject::connect(&serverHandler, &ServerHandler::udpTransportEvent, &m_adapter,
					 [this](const UdpTransportEvent &event) { m_receiver.present(event); }, Qt::QueuedConnection);
}

} // namespace mumble::modern::adapters
