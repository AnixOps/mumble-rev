// Copyright The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#pragma once

#include "contracts/ports/ConnectionPort.h"

#include <QVector>

namespace mumble::modern::testing {

class FakeConnectionPort : public contracts::ConnectionPort {
public:
	QVector< contracts::ConnectionRequest > connectionRequests;
	QVector< contracts::ConnectionAttemptId > cancelledAttempts;
	QVector< contracts::ConnectionEpoch > disconnectedEpochs;
	QVector< contracts::CertificateChallengeId > acceptedCertificateChallenges;
	QVector< contracts::CertificateChallengeId > rejectedCertificateChallenges;

	void connectTo(const contracts::ConnectionRequest &request) override { connectionRequests.append(request); }
	void cancel(contracts::ConnectionAttemptId attempt) override { cancelledAttempts.append(attempt); }
	void disconnect(contracts::ConnectionEpoch epoch) override { disconnectedEpochs.append(epoch); }
	void acceptCertificate(contracts::CertificateChallengeId challenge) override {
		acceptedCertificateChallenges.append(challenge);
	}
	void rejectCertificate(contracts::CertificateChallengeId challenge) override {
		rejectedCertificateChallenges.append(challenge);
	}
};

} // namespace mumble::modern::testing
