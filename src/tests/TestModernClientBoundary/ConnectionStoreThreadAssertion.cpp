// Copyright The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#include "application/stores/ConnectionStore.h"

#include <QCoreApplication>
#include <QThread>

namespace {

class WrongThreadWriter final : public QThread {
public:
	explicit WrongThreadWriter(mumble::modern::application::ConnectionStore &store) : m_store(store) {}

protected:
	void run() override {
		m_store.replaceSnapshot({});
	}

private:
	mumble::modern::application::ConnectionStore &m_store;
};

} // namespace

int main(int argc, char *argv[]) {
	QCoreApplication application(argc, argv);
	mumble::modern::application::ConnectionStore store;
	WrongThreadWriter writer(store);
	writer.start();
	writer.wait();
	return 0;
}
