# Copyright The Mumble Developers. All rights reserved.
# Use of this source code is governed by a BSD-style license
# that can be found in the LICENSE file at the root of the
# Mumble source tree or at <https://www.mumble.info/LICENSE>.

execute_process(
	COMMAND "${CMAKE_COMMAND}" -S "${MODERN_LINK_DEPENDENCY_CONTROL_DIR}"
		-B "${MODERN_LINK_DEPENDENCY_CONTROL_BUILD_DIR}"
	RESULT_VARIABLE MODERN_LINK_DEPENDENCY_CONTROL_RESULT
	OUTPUT_VARIABLE MODERN_LINK_DEPENDENCY_CONTROL_OUTPUT
	ERROR_VARIABLE MODERN_LINK_DEPENDENCY_CONTROL_ERROR
)

if(NOT MODERN_LINK_DEPENDENCY_CONTROL_RESULT EQUAL 0)
	message(FATAL_ERROR "The dependency verifier rejected the linker-flag control:\n"
		"${MODERN_LINK_DEPENDENCY_CONTROL_OUTPUT}\n${MODERN_LINK_DEPENDENCY_CONTROL_ERROR}")
endif()

execute_process(
	COMMAND "${CMAKE_COMMAND}" -S "${MODERN_LINK_DEPENDENCY_FIXTURE_DIR}"
		-B "${MODERN_LINK_DEPENDENCY_FIXTURE_BUILD_DIR}"
	RESULT_VARIABLE MODERN_LINK_DEPENDENCY_RESULT
	OUTPUT_VARIABLE MODERN_LINK_DEPENDENCY_OUTPUT
	ERROR_VARIABLE MODERN_LINK_DEPENDENCY_ERROR
)

if(MODERN_LINK_DEPENDENCY_RESULT EQUAL 0)
	message(FATAL_ERROR "The dependency verifier accepted the forbidden linked target")
endif()

set(MODERN_LINK_DEPENDENCY_DIAGNOSTICS
	"${MODERN_LINK_DEPENDENCY_OUTPUT}\n${MODERN_LINK_DEPENDENCY_ERROR}")
if(NOT MODERN_LINK_DEPENDENCY_DIAGNOSTICS MATCHES
	"Forbidden linked target 'Qt6::Widgets'.*mumble_modern_contracts")
	message(FATAL_ERROR "The dependency verifier did not report the forbidden linked target")
endif()
