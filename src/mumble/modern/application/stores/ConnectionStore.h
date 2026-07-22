// Copyright The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#pragma once

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

signals:
	void snapshotChanged();

private:
	void assertOnOwnerThread() const;

	QThread *const m_ownerThread;
	contracts::ConnectionSnapshot m_snapshot;
};

} // namespace mumble::modern::application
