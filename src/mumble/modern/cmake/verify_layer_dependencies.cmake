# Copyright The Mumble Developers. All rights reserved.
# Use of this source code is governed by a BSD-style license
# that can be found in the LICENSE file at the root of the
# Mumble source tree or at <https://www.mumble.info/LICENSE>.

if(NOT DEFINED MODERN_SOURCE_DIR)
	message(FATAL_ERROR "MODERN_SOURCE_DIR is required")
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
