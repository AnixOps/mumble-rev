# Copyright The Mumble Developers. All rights reserved.
# Use of this source code is governed by a BSD-style license
# that can be found in the LICENSE file at the root of the
# Mumble source tree or at <https://www.mumble.info/LICENSE>.

function(_resolve_modern_target_alias MODERN_TARGET MODERN_RESOLVED_TARGET)
	set(MODERN_RESOLVED_ALIAS_TARGET "${MODERN_TARGET}")
	set(MODERN_ALIAS_TARGETS)

	while(TARGET "${MODERN_RESOLVED_ALIAS_TARGET}")
		list(FIND MODERN_ALIAS_TARGETS "${MODERN_RESOLVED_ALIAS_TARGET}" MODERN_ALIAS_TARGET_INDEX)
		if(NOT MODERN_ALIAS_TARGET_INDEX EQUAL -1)
			break()
		endif()
		list(APPEND MODERN_ALIAS_TARGETS "${MODERN_RESOLVED_ALIAS_TARGET}")

		get_target_property(MODERN_ALIASED_TARGET "${MODERN_RESOLVED_ALIAS_TARGET}" ALIASED_TARGET)
		if(NOT MODERN_ALIASED_TARGET OR MODERN_ALIASED_TARGET STREQUAL "MODERN_ALIASED_TARGET-NOTFOUND")
			break()
		endif()
		set(MODERN_RESOLVED_ALIAS_TARGET "${MODERN_ALIASED_TARGET}")
	endwhile()

	set("${MODERN_RESOLVED_TARGET}" "${MODERN_RESOLVED_ALIAS_TARGET}" PARENT_SCOPE)
endfunction()

function(_verify_modern_target_dependencies MODERN_TARGET MODERN_VISITED_TARGETS)
	_resolve_modern_target_alias("${MODERN_TARGET}" MODERN_TARGET)
	list(FIND MODERN_VISITED_TARGETS "${MODERN_TARGET}" MODERN_TARGET_VISITED_INDEX)
	if(NOT MODERN_TARGET_VISITED_INDEX EQUAL -1)
		return()
	endif()

	list(APPEND MODERN_VISITED_TARGETS "${MODERN_TARGET}")
	foreach(MODERN_TARGET_PROPERTY IN ITEMS LINK_LIBRARIES INTERFACE_LINK_LIBRARIES)
		get_target_property(MODERN_TARGET_LINKS "${MODERN_TARGET}" "${MODERN_TARGET_PROPERTY}")
		if(NOT MODERN_TARGET_LINKS OR MODERN_TARGET_LINKS STREQUAL "MODERN_TARGET_LINKS-NOTFOUND")
			continue()
		endif()

		foreach(MODERN_TARGET_LINK IN LISTS MODERN_TARGET_LINKS)
			set(MODERN_RESOLVED_TARGET_LINK "${MODERN_TARGET_LINK}")
			string(REGEX REPLACE "^\\$<LINK_ONLY:([^>]+)>$" "\\1" MODERN_RESOLVED_TARGET_LINK
				"${MODERN_RESOLVED_TARGET_LINK}")
			string(REGEX REPLACE "^\\$<BUILD_INTERFACE:([^>]+)>$" "\\1" MODERN_RESOLVED_TARGET_LINK
				"${MODERN_RESOLVED_TARGET_LINK}")
			_resolve_modern_target_alias("${MODERN_RESOLVED_TARGET_LINK}" MODERN_RESOLVED_TARGET_LINK)
			string(REGEX MATCH "^Qt[0-9]+::(Widgets|Quick)$" MODERN_FORBIDDEN_LINK
				"${MODERN_RESOLVED_TARGET_LINK}")
			if(MODERN_FORBIDDEN_LINK)
				message(FATAL_ERROR
					"Forbidden linked target '${MODERN_FORBIDDEN_LINK}' in ${MODERN_TARGET}")
			endif()
			if(TARGET "${MODERN_RESOLVED_TARGET_LINK}")
				_verify_modern_target_dependencies("${MODERN_RESOLVED_TARGET_LINK}" "${MODERN_VISITED_TARGETS}")
			endif()
		endforeach()
	endforeach()
endfunction()

function(verify_modern_layer_dependencies)
	foreach(MODERN_LAYER_TARGET IN ITEMS mumble_modern_contracts mumble_modern_application)
		if(NOT TARGET "${MODERN_LAYER_TARGET}")
			message(FATAL_ERROR "Required modern target '${MODERN_LAYER_TARGET}' does not exist")
		endif()

		_verify_modern_target_dependencies("${MODERN_LAYER_TARGET}" "")
	endforeach()
endfunction()

if(NOT DEFINED MODERN_SOURCE_DIR)
	return()
endif()

set(MODERN_FORBIDDEN_INCLUDE_TOKENS
	"Global.h"
	"MainWindow.h"
	"ServerHandler.h"
	"ClientUser.h"
	"Channel.h"
	"Mumble.pb.h"
	"MumbleUDP.pb.h"
	"google/protobuf"
	"QtWidgets"
	"QtQuick"
	"windows.h"
	"Windows.h"
)

foreach(MODERN_LAYER IN ITEMS contracts application)
	set(MODERN_LAYER_DIR "${MODERN_SOURCE_DIR}/${MODERN_LAYER}")
	if(NOT IS_DIRECTORY "${MODERN_LAYER_DIR}")
		continue()
	endif()

	file(GLOB_RECURSE MODERN_LAYER_FILES
		"${MODERN_LAYER_DIR}/*.c"
		"${MODERN_LAYER_DIR}/*.cc"
		"${MODERN_LAYER_DIR}/*.cpp"
		"${MODERN_LAYER_DIR}/*.cxx"
		"${MODERN_LAYER_DIR}/*.h"
		"${MODERN_LAYER_DIR}/*.hh"
		"${MODERN_LAYER_DIR}/*.hpp"
	)

	foreach(MODERN_LAYER_FILE IN LISTS MODERN_LAYER_FILES)
		file(READ "${MODERN_LAYER_FILE}" MODERN_LAYER_CONTENTS)
		foreach(MODERN_FORBIDDEN_TOKEN IN LISTS MODERN_FORBIDDEN_INCLUDE_TOKENS)
			string(FIND "${MODERN_LAYER_CONTENTS}" "${MODERN_FORBIDDEN_TOKEN}" MODERN_TOKEN_INDEX)
			if(NOT MODERN_TOKEN_INDEX EQUAL -1)
				message(FATAL_ERROR
					"Forbidden dependency token '${MODERN_FORBIDDEN_TOKEN}' in ${MODERN_LAYER_FILE}")
			endif()
		endforeach()
	endforeach()
endforeach()
