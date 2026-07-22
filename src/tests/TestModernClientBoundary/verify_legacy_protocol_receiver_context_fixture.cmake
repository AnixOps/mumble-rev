# Copyright The Mumble Developers. All rights reserved.
# Use of this source code is governed by a BSD-style license
# that can be found in the LICENSE file at the root of the
# Mumble source tree or at <https://www.mumble.info/LICENSE>.

set(fixture_mumble_file "${FIXTURE_DIR}/mumble/CMakeLists.txt")
set(fixture_modern_dir "${FIXTURE_DIR}/modern")
file(MAKE_DIRECTORY "${FIXTURE_DIR}/mumble")
file(MAKE_DIRECTORY "${fixture_modern_dir}")
file(WRITE "${fixture_mumble_file}"
"target_sources(mumble_client_object_lib PRIVATE\n"
"  \"modern/adapters/legacy/ConnectionSynchronizationShadow.cpp\"\n"
"  \"modern/adapters/legacy/LegacyProtocolComposition.cpp\"\n"
"  \"modern/adapters/legacy/LegacyProtocolReceiver.cpp\"\n"
")\n")
file(WRITE "${fixture_modern_dir}/CMakeLists.txt"
"add_library(mumble_modern_invalid STATIC\n"
"  \"adapters/legacy/LegacyProtocolReceiver.cpp\"\n"
")\n")

execute_process(
	COMMAND "${CMAKE_COMMAND}"
		"-DMUMBLE_CMAKE_FILE=${MUMBLE_CMAKE_FILE}"
		"-DMODERN_SOURCE_DIR=${fixture_modern_dir}"
		-P "${CONTEXT_VERIFIER}"
	RESULT_VARIABLE verifier_result
	OUTPUT_VARIABLE verifier_output
	ERROR_VARIABLE verifier_error
)

if(verifier_result EQUAL 0)
	message(FATAL_ERROR "The legacy receiver context verifier accepted a modern target declaration")
endif()

set(verifier_log "${verifier_output}\n${verifier_error}")
if(NOT verifier_log MATCHES "LegacyProtocolReceiver.cpp"
	OR NOT verifier_log MATCHES "must not be declared by modern")
	message(FATAL_ERROR "The legacy receiver context verifier failed without reporting the modern target declaration:\n${verifier_log}")
endif()
