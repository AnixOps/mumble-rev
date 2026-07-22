// Copyright The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#pragma once

#include "ConnectionSynchronizationShadow.h"
#include "LegacyProtocolReceiver.h"
#include "../ProtocolEventAdapter.h"

#include <functional>
#include <memory>

class MainWindow;
class ServerHandler;

namespace mumble::modern::adapters {

class LegacyProtocolComposition {
public:
	explicit LegacyProtocolComposition(MainWindow &mainWindow);
	explicit LegacyProtocolComposition(ProtocolMessageReceiver &receiver,
		std::function< bool() > legacySynchronizationReader = {});
	void attach(ServerHandler &serverHandler);
	ProtocolEventAdapter &protocolAdapterForTesting();
	std::optional< ConnectionSynchronizationComparison > shadowComparisonForTesting() const;

private:
	void compareShadowState(const ControlMessageEnvelope &envelope);
	void reportProtocolDiagnostic(const ProtocolDiagnostic &diagnostic);

	std::unique_ptr< LegacyProtocolReceiver > m_legacyReceiver;
	ProtocolMessageReceiver &m_receiver;
	std::function< bool() > m_legacySynchronizationReader;
	bool m_legacySynchronized = false;
	ConnectionSynchronizationShadow m_shadow;
	ProtocolEventAdapter m_adapter;
};

} // namespace mumble::modern::adapters
