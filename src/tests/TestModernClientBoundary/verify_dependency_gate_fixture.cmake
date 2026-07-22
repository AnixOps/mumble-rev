# Copyright The Mumble Developers. All rights reserved.
# Use of this source code is governed by a BSD-style license
# that can be found in the LICENSE file at the root of the
# Mumble source tree or at <https://www.mumble.info/LICENSE>.

execute_process(
	COMMAND "${CMAKE_COMMAND}"
		"-DMODERN_SOURCE_DIR=${MODERN_DEPENDENCY_FIXTURE_DIR}"
		-P "${MODERN_DEPENDENCY_VERIFIER}"
	RESULT_VARIABLE MODERN_DEPENDENCY_RESULT
	OUTPUT_VARIABLE MODERN_DEPENDENCY_OUTPUT
	ERROR_VARIABLE MODERN_DEPENDENCY_ERROR
)

if(MODERN_DEPENDENCY_RESULT EQUAL 0)
	message(FATAL_ERROR "The dependency verifier accepted the forbidden fixture")
endif()

set(MODERN_DEPENDENCY_DIAGNOSTICS
	"${MODERN_DEPENDENCY_OUTPUT}\n${MODERN_DEPENDENCY_ERROR}")
if(NOT MODERN_DEPENDENCY_DIAGNOSTICS MATCHES
	"Forbidden dependency token 'QtWidgets'.*ForbiddenInclude\\.h")
	message(FATAL_ERROR "The dependency verifier did not report the forbidden fixture")
endif()
