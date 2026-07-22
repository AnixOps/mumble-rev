// Copyright The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#include "ConnectionPresentationProbe.h"

namespace mumble::modern::presentation {

ConnectionPresentationProbe::ConnectionPresentationProbe(application::ConnectionStore &connectionStore, QObject *parent)
	: QObject(parent), m_connectionStore(connectionStore), m_connectionPhase(connectionStore.snapshot().phase) {
	connect(&m_connectionStore, &application::ConnectionStore::snapshotChanged, this,
		&ConnectionPresentationProbe::updateConnectionPhase);
}

contracts::ConnectionPhase ConnectionPresentationProbe::connectionPhase() const {
	return m_connectionPhase;
}

void ConnectionPresentationProbe::updateConnectionPhase() {
	const contracts::ConnectionPhase updatedPhase = m_connectionStore.snapshot().phase;
	if (m_connectionPhase == updatedPhase) {
		return;
	}

	m_connectionPhase = updatedPhase;
	emit connectionPhaseChanged();
}

} // namespace mumble::modern::presentation
