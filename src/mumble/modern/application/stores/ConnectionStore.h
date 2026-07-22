// Copyright The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#pragma once

#include "contracts/ConnectionEvent.h"
#include "contracts/ConnectionSnapshot.h"

#include <QObject>

class QThread;

namespace mumble::modern::application {

class ConnectionStore : public QObject {
	Q_OBJECT

public:
	explicit ConnectionStore(QObject *parent = nullptr);

	contracts::ConnectionSnapshot snapshot() const;
	void replaceSnapshot(contracts::ConnectionSnapshot snapshot);
	bool apply(const contracts::ConnectionAttemptStarted &event);
	bool apply(const contracts::ConnectionPhaseChanged &event);
	bool apply(const contracts::ConnectionSynchronized &event);
	bool apply(const contracts::ConnectionFailed &event);

signals:
	void snapshotChanged();
	void sequenceGapDetected(contracts::ConnectionAttemptId attempt, quint64 expectedSequence, quint64 receivedSequence);

private:
	bool acceptsNextEvent(contracts::ConnectionAttemptId attempt, quint64 sequence);
	void publish(contracts::ConnectionSnapshot snapshot);
	void assertOnOwnerThread() const;

	QThread *const m_ownerThread;
	contracts::ConnectionSnapshot m_snapshot;
	std::optional< quint64 > m_lastAcceptedSequence;
};

} // namespace mumble::modern::application
