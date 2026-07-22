# Copyright The Mumble Developers. All rights reserved.
# Use of this source code is governed by a BSD-style license
# that can be found in the LICENSE file at the root of the
# Mumble source tree or at <https://www.mumble.info/LICENSE>.

if(NOT DEFINED THREAD_ASSERTION_EXECUTABLE)
	message(FATAL_ERROR "THREAD_ASSERTION_EXECUTABLE is required")
endif()

execute_process(
	COMMAND "${THREAD_ASSERTION_EXECUTABLE}"
	RESULT_VARIABLE THREAD_ASSERTION_RESULT
	OUTPUT_VARIABLE THREAD_ASSERTION_OUTPUT
	ERROR_VARIABLE THREAD_ASSERTION_ERROR
)

if(THREAD_ASSERTION_RESULT EQUAL 0)
	message(FATAL_ERROR "ConnectionStore accepted a write from a non-owner thread")
endif()

set(THREAD_ASSERTION_DIAGNOSTICS "${THREAD_ASSERTION_OUTPUT}\n${THREAD_ASSERTION_ERROR}")
if(NOT THREAD_ASSERTION_DIAGNOSTICS MATCHES "ASSERT.*ConnectionStore")
	message(FATAL_ERROR "ConnectionStore did not report its thread-affinity assertion")
endif()
