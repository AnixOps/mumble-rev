// Copyright The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#include "ConnectionStore.h"

#include <QThread>
#include <QtGlobal>

#include <utility>

namespace mumble::modern::application {

ConnectionStore::ConnectionStore(QObject *parent) : QObject(parent), m_ownerThread(QThread::currentThread()) {}

contracts::ConnectionSnapshot ConnectionStore::snapshot() const {
	assertOnOwnerThread();
	return m_snapshot;
}

void ConnectionStore::replaceSnapshot(contracts::ConnectionSnapshot snapshot) {
	assertOnOwnerThread();
	m_snapshot = std::move(snapshot);
	emit snapshotChanged();
}

void ConnectionStore::assertOnOwnerThread() const {
	Q_ASSERT(QThread::currentThread() == m_ownerThread);
}

} // namespace mumble::modern::application
