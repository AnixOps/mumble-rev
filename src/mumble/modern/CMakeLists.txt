# Copyright The Mumble Developers. All rights reserved.
# Use of this source code is governed by a BSD-style license
# that can be found in the LICENSE file at the root of the
# Mumble source tree or at <https://www.mumble.info/LICENSE>.

add_library(mumble_modern_contracts STATIC contracts/ModernContracts.cpp)
target_link_libraries(mumble_modern_contracts PUBLIC Qt6::Core)
target_include_directories(mumble_modern_contracts PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

add_library(mumble_modern_application STATIC
	ModernClientBootstrap.cpp
	application/ClientApplication.cpp
	application/stores/ConnectionStore.cpp
)
set_target_properties(mumble_modern_application PROPERTIES AUTOMOC ON)
target_link_libraries(mumble_modern_application PUBLIC mumble_modern_contracts Qt6::Core)
target_include_directories(mumble_modern_application PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

add_library(mumble_modern_presentation STATIC presentation/ConnectionPresentationProbe.cpp)
set_target_properties(mumble_modern_presentation PROPERTIES AUTOMOC ON)
target_link_libraries(mumble_modern_presentation PUBLIC mumble_modern_application Qt6::Core)
target_include_directories(mumble_modern_presentation PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

add_library(mumble_modern_protocol_adapter STATIC
	adapters/ProtocolEventAdapter.cpp
)
set_target_properties(mumble_modern_protocol_adapter PROPERTIES AUTOMOC ON)
target_link_libraries(mumble_modern_protocol_adapter PUBLIC mumble_modern_contracts shared Qt6::Core)
target_include_directories(mumble_modern_protocol_adapter PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

add_library(mumble_modern_legacy_protocol_receiver STATIC
	adapters/legacy/ConnectionSynchronizationShadow.cpp
	adapters/legacy/LegacyProtocolComposition.cpp
	adapters/legacy/LegacyProtocolReceiver.cpp
)
target_link_libraries(mumble_modern_legacy_protocol_receiver PUBLIC mumble_modern_protocol_adapter)
target_include_directories(mumble_modern_legacy_protocol_receiver PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

add_library(mumble_modern_testing STATIC testing/ModernTesting.cpp)
target_link_libraries(mumble_modern_testing PUBLIC mumble_modern_contracts)
target_include_directories(mumble_modern_testing PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/verify_layer_dependencies.cmake")
verify_modern_layer_dependencies()

add_custom_target(verify_modern_layer_dependencies ALL
	COMMAND "${CMAKE_COMMAND}"
		"-DMODERN_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
		-P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/verify_layer_dependencies.cmake"
	COMMENT "Verifying modern client layer dependencies"
	VERBATIM
)
