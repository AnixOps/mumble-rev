// Copyright The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#pragma once

#include "application/stores/ConnectionStore.h"

#include <QObject>

namespace mumble::modern::presentation {

class ConnectionPresentationProbe : public QObject {
	Q_OBJECT
	Q_PROPERTY(mumble::modern::contracts::ConnectionPhase connectionPhase READ connectionPhase NOTIFY connectionPhaseChanged)

public:
	explicit ConnectionPresentationProbe(application::ConnectionStore &connectionStore, QObject *parent = nullptr);

	contracts::ConnectionPhase connectionPhase() const;

signals:
	void connectionPhaseChanged();

private:
	void updateConnectionPhase();

	application::ConnectionStore &m_connectionStore;
	contracts::ConnectionPhase m_connectionPhase;
};

} // namespace mumble::modern::presentation
