# Copyright The Mumble Developers. All rights reserved.
# Use of this source code is governed by a BSD-style license
# that can be found in the LICENSE file at the root of the
# Mumble source tree or at <https://www.mumble.info/LICENSE>.

set(MODERN_DEPENDENCY_VERIFIER
	"${CMAKE_CURRENT_LIST_DIR}/../../mumble/modern/cmake/verify_layer_dependencies.cmake")
set(MODERN_DEPENDENCY_FIXTURE_DIR "${CMAKE_CURRENT_BINARY_DIR}/forbidden-dependency-fixture")
set(MODERN_DEPENDENCY_FIXTURE_FILE "${MODERN_DEPENDENCY_FIXTURE_DIR}/contracts/ForbiddenInclude.h")

file(MAKE_DIRECTORY "${MODERN_DEPENDENCY_FIXTURE_DIR}/contracts")
file(WRITE "${MODERN_DEPENDENCY_FIXTURE_FILE}" "#include <QtWidgets/QWidget>\n")

add_executable(TestModernClientBoundary TestModernClientBoundary.cpp)
set_target_properties(TestModernClientBoundary PROPERTIES AUTOMOC ON)
target_link_libraries(TestModernClientBoundary PRIVATE Qt6::Test mumble_modern_contracts mumble_modern_testing)
add_test(NAME TestModernClientBoundary COMMAND TestModernClientBoundary)

add_test(
	NAME TestModernClientBoundaryForbiddenDependency
	COMMAND "${CMAKE_COMMAND}"
		"-DMODERN_DEPENDENCY_VERIFIER=${MODERN_DEPENDENCY_VERIFIER}"
		"-DMODERN_DEPENDENCY_FIXTURE_DIR=${MODERN_DEPENDENCY_FIXTURE_DIR}"
		-P "${CMAKE_CURRENT_LIST_DIR}/verify_dependency_gate_fixture.cmake"
)
