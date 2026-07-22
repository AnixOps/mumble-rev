// Copyright The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#pragma once

#include "../ClientTypes.h"

namespace mumble::modern::contracts {

class ConnectionPort {
public:
	virtual ~ConnectionPort() = default;

	virtual void connectTo(const ConnectionRequest &request) = 0;
	virtual void cancel(ConnectionAttemptId attempt) = 0;
	virtual void disconnect(ConnectionEpoch epoch) = 0;
	virtual void acceptCertificate(CertificateChallengeId challenge) = 0;
	virtual void rejectCertificate(CertificateChallengeId challenge) = 0;
};

} // namespace mumble::modern::contracts
