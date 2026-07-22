# Copyright The Mumble Developers. All rights reserved.
# Use of this source code is governed by a BSD-style license
# that can be found in the LICENSE file at the root of the
# Mumble source tree or at <https://www.mumble.info/LICENSE>.

file(READ "${MUMBLE_CMAKE_FILE}" mumble_cmake)
file(READ "${MODERN_CMAKE_FILE}" modern_cmake)

foreach(receiver_source IN ITEMS
	"adapters/legacy/ConnectionSynchronizationShadow.cpp"
	"adapters/legacy/LegacyProtocolComposition.cpp"
	"adapters/legacy/LegacyProtocolReceiver.cpp")
	string(FIND "${mumble_cmake}" "${receiver_source}" receiver_source_offset)
	if(receiver_source_offset EQUAL -1)
		message(FATAL_ERROR "${receiver_source} must compile in mumble_client_object_lib's legacy context")
	endif()
endforeach()

if(modern_cmake MATCHES "add_library\\(mumble_modern_legacy_protocol_receiver")
	message(FATAL_ERROR "Legacy protocol receiver must not compile as an independent modern target")
endif()
