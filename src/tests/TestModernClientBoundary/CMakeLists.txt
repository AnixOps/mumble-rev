# Copyright The Mumble Developers. All rights reserved.
# Use of this source code is governed by a BSD-style license
# that can be found in the LICENSE file at the root of the
# Mumble source tree or at <https://www.mumble.info/LICENSE>.

set(MODERN_DEPENDENCY_VERIFIER
	"${CMAKE_CURRENT_LIST_DIR}/../../mumble/modern/cmake/verify_layer_dependencies.cmake")
set(MODERN_DEPENDENCY_FIXTURE_DIR "${CMAKE_CURRENT_BINARY_DIR}/forbidden-dependency-fixture")
set(MODERN_DEPENDENCY_FIXTURE_FILE "${MODERN_DEPENDENCY_FIXTURE_DIR}/contracts/ForbiddenInclude.h")
set(MODERN_LINK_DEPENDENCY_FIXTURE_DIR "${CMAKE_CURRENT_BINARY_DIR}/forbidden-link-dependency-fixture")
set(MODERN_LINK_DEPENDENCY_FIXTURE_BUILD_DIR "${MODERN_LINK_DEPENDENCY_FIXTURE_DIR}/build")

file(MAKE_DIRECTORY "${MODERN_DEPENDENCY_FIXTURE_DIR}/contracts")
file(WRITE "${MODERN_DEPENDENCY_FIXTURE_FILE}" "#include <QtWidgets/QWidget>\n")
file(MAKE_DIRECTORY "${MODERN_LINK_DEPENDENCY_FIXTURE_DIR}/contracts")
file(WRITE "${MODERN_LINK_DEPENDENCY_FIXTURE_DIR}/contracts/ModernContracts.cpp" "")
file(WRITE "${MODERN_LINK_DEPENDENCY_FIXTURE_DIR}/CMakeLists.txt"
"cmake_minimum_required(VERSION 3.23)\n"
"project(ForbiddenModernLinkDependency LANGUAGES CXX)\n"
"add_library(Qt6::Core INTERFACE IMPORTED)\n"
"add_library(Qt6::Widgets INTERFACE IMPORTED)\n"
"add_library(legacy_ui ALIAS Qt6::Widgets)\n"
"add_library(mumble_modern_contracts STATIC contracts/ModernContracts.cpp)\n"
"target_link_libraries(mumble_modern_contracts PUBLIC Qt6::Core \"$<$<CONFIG:Debug>:legacy_ui>\")\n"
"include(\"${MODERN_DEPENDENCY_VERIFIER}\")\n"
"verify_modern_layer_dependencies()\n")

add_executable(TestModernClientBoundary TestModernClientBoundary.cpp)
set_target_properties(TestModernClientBoundary PROPERTIES AUTOMOC ON)
target_link_libraries(TestModernClientBoundary PRIVATE Qt6::Test mumble_modern_application mumble_modern_presentation
	mumble_modern_testing)
add_test(NAME TestModernClientBoundary COMMAND TestModernClientBoundary)

add_executable(TestModernClientBoundaryThreadAssertion ConnectionStoreThreadAssertion.cpp)
target_link_libraries(TestModernClientBoundaryThreadAssertion PRIVATE Qt6::Core mumble_modern_application)

add_test(
	NAME TestModernClientBoundaryThreadAssertion
	CONFIGURATIONS Debug
	COMMAND "${CMAKE_COMMAND}"
		"-DTHREAD_ASSERTION_EXECUTABLE=$<TARGET_FILE:TestModernClientBoundaryThreadAssertion>"
		-P "${CMAKE_CURRENT_LIST_DIR}/verify_connection_store_thread_assertion.cmake"
)

add_test(
	NAME TestModernClientBoundaryForbiddenDependency
	COMMAND "${CMAKE_COMMAND}"
		"-DMODERN_DEPENDENCY_VERIFIER=${MODERN_DEPENDENCY_VERIFIER}"
		"-DMODERN_DEPENDENCY_FIXTURE_DIR=${MODERN_DEPENDENCY_FIXTURE_DIR}"
		-P "${CMAKE_CURRENT_LIST_DIR}/verify_dependency_gate_fixture.cmake"
)

add_test(
	NAME TestModernClientBoundaryForbiddenLinkDependency
	COMMAND "${CMAKE_COMMAND}"
		"-DMODERN_LINK_DEPENDENCY_FIXTURE_DIR=${MODERN_LINK_DEPENDENCY_FIXTURE_DIR}"
		"-DMODERN_LINK_DEPENDENCY_FIXTURE_BUILD_DIR=${MODERN_LINK_DEPENDENCY_FIXTURE_BUILD_DIR}"
		-P "${CMAKE_CURRENT_LIST_DIR}/verify_link_dependency_gate_fixture.cmake"
)
