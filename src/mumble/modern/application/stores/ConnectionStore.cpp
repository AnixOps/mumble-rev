// Copyright The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#include "ConnectionStore.h"

#include <QThread>
#include <QtGlobal>

#include <limits>
#include <utility>

namespace mumble::modern::application {

ConnectionStore::ConnectionStore(QObject *parent) : QObject(parent), m_ownerThread(QThread::currentThread()) {}

contracts::ConnectionSnapshot ConnectionStore::snapshot() const {
	assertOnOwnerThread();
	return m_snapshot;
}

void ConnectionStore::replaceSnapshot(contracts::ConnectionSnapshot snapshot) {
	assertOnOwnerThread();
	m_lastAcceptedSequence.reset();
	publish(std::move(snapshot));
}

bool ConnectionStore::apply(const contracts::ConnectionAttemptStarted &event) {
	assertOnOwnerThread();
	if (event.sequence == 0 || event.attempt.value <= m_snapshot.attempt.value) {
		return false;
	}

	contracts::ConnectionSnapshot snapshot;
	snapshot.attempt = event.attempt;
	snapshot.phase = contracts::ConnectionPhase::Resolving;
	snapshot.target = event.target;
	snapshot.cancellationAllowed = true;
	m_lastAcceptedSequence = event.sequence;
	publish(std::move(snapshot));
	return true;
}

bool ConnectionStore::apply(const contracts::ConnectionPhaseChanged &event) {
	assertOnOwnerThread();
	if (!acceptsNextEvent(event.attempt, event.sequence)) {
		return false;
	}

	contracts::ConnectionSnapshot snapshot = m_snapshot;
	snapshot.phase = event.phase;
	snapshot.cancellationAllowed = event.cancellationAllowed;
	publish(std::move(snapshot));
	return true;
}

bool ConnectionStore::apply(const contracts::ConnectionSynchronized &event) {
	assertOnOwnerThread();
	if (!acceptsNextEvent(event.attempt, event.sequence)) {
		return false;
	}

	contracts::ConnectionSnapshot snapshot = m_snapshot;
	snapshot.epoch = event.epoch;
	snapshot.phase = contracts::ConnectionPhase::Connected;
	snapshot.cancellationAllowed = false;
	publish(std::move(snapshot));
	return true;
}

bool ConnectionStore::apply(const contracts::ConnectionFailed &event) {
	assertOnOwnerThread();
	if (!acceptsNextEvent(event.attempt, event.sequence)) {
		return false;
	}

	contracts::ConnectionSnapshot snapshot = m_snapshot;
	snapshot.phase = contracts::ConnectionPhase::Failed;
	snapshot.cancellationAllowed = false;
	snapshot.lastError = event.error;
	publish(std::move(snapshot));
	return true;
}

bool ConnectionStore::acceptsNextEvent(contracts::ConnectionAttemptId attempt, quint64 sequence) {
	if (attempt != m_snapshot.attempt || !m_lastAcceptedSequence.has_value() || sequence == 0) {
		return false;
	}
	if (m_lastAcceptedSequence.value() == std::numeric_limits< quint64 >::max()) {
		return false;
	}

	const quint64 expectedSequence = m_lastAcceptedSequence.value() + 1;
	if (sequence != expectedSequence) {
		if (sequence > expectedSequence) {
			emit sequenceGapDetected(attempt, expectedSequence, sequence);
		}
		return false;
	}

	m_lastAcceptedSequence = sequence;
	return true;
}

void ConnectionStore::publish(contracts::ConnectionSnapshot snapshot) {
	m_snapshot = std::move(snapshot);
	emit snapshotChanged();
}

void ConnectionStore::assertOnOwnerThread() const {
	Q_ASSERT(QThread::currentThread() == m_ownerThread);
}

} // namespace mumble::modern::application
