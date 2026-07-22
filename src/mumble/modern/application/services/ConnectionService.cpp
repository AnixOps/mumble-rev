// Copyright The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#include "ConnectionService.h"

namespace mumble::modern::application {

ConnectionService::ConnectionService(ConnectionStore &connectionStore) : m_connectionStore(connectionStore) {}

bool ConnectionService::handle(const contracts::ConnectionAttemptStarted &event) {
	return m_connectionStore.apply(event);
}

bool ConnectionService::handle(const contracts::ConnectionPhaseChanged &event) {
	return m_connectionStore.apply(event);
}

bool ConnectionService::handle(const contracts::ConnectionSynchronized &event) {
	return m_connectionStore.apply(event);
}

bool ConnectionService::handle(const contracts::ConnectionFailed &event) {
	return m_connectionStore.apply(event);
}

} // namespace mumble::modern::application
