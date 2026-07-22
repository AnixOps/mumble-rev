# Copyright The Mumble Developers. All rights reserved.
# Use of this source code is governed by a BSD-style license
# that can be found in the LICENSE file at the root of the
# Mumble source tree or at <https://www.mumble.info/LICENSE>.

if(POLICY CMP0007)
	cmake_policy(SET CMP0007 NEW)
endif()

file(READ "${MUMBLE_CMAKE_FILE}" mumble_cmake)
string(REGEX REPLACE "[\r\n\t]" " " mumble_cmake "${mumble_cmake}")

string(REGEX MATCHALL
	"target_sources[ ]*\\([ ]*mumble_client_object_lib([ ][^)]*)?\\)"
	legacy_receiver_target_sources_blocks "${mumble_cmake}")
if(legacy_receiver_target_sources_blocks STREQUAL "")
	message(FATAL_ERROR "mumble_client_object_lib must declare a target_sources block")
endif()

foreach(receiver_source IN ITEMS
	"adapters/legacy/ConnectionSynchronizationShadow.cpp"
	"adapters/legacy/LegacyProtocolComposition.cpp"
	"adapters/legacy/LegacyProtocolReceiver.cpp")
	set(receiver_source_in_legacy_target FALSE)
	foreach(legacy_receiver_target_sources IN LISTS legacy_receiver_target_sources_blocks)
		string(FIND "${legacy_receiver_target_sources}" "${receiver_source}" receiver_source_offset)
		if(NOT receiver_source_offset EQUAL -1)
			set(receiver_source_in_legacy_target TRUE)
			break()
		endif()
	endforeach()
	if(NOT receiver_source_in_legacy_target)
		message(FATAL_ERROR
			"${receiver_source} must compile in mumble_client_object_lib's legacy target_sources block")
	endif()
endforeach()

file(GLOB_RECURSE modern_cmake_files
	"${MODERN_SOURCE_DIR}/CMakeLists.txt"
	"${MODERN_SOURCE_DIR}/*.cmake")

function(strip_cmake_quoted_strings_and_comments input output)
	string(LENGTH "${input}" input_length)
	set(index 0)
	set(in_quoted_string FALSE)
	set(in_comment FALSE)
	set(in_bracket_argument FALSE)
	set(bracket_argument_close "")
	set(bracket_argument_preserved_contents "")
	set(escaped_character FALSE)
	set(stripped_input "")

	while(index LESS input_length)
		string(SUBSTRING "${input}" ${index} 1 character)

		if(in_comment)
			if(character STREQUAL "\n")
				set(in_comment FALSE)
				string(APPEND stripped_input "\n")
			endif()
		elseif(in_bracket_argument)
			string(SUBSTRING "${input}" ${index} -1 remaining_input)
			string(FIND "${remaining_input}" "${bracket_argument_close}" bracket_close_index)
			if(bracket_close_index EQUAL 0)
				string(APPEND stripped_input "${bracket_argument_preserved_contents}")
				string(LENGTH "${bracket_argument_close}" bracket_close_length)
				math(EXPR index "${index} + ${bracket_close_length}")
				set(in_bracket_argument FALSE)
				set(bracket_argument_preserved_contents "")
				continue()
			elseif(character STREQUAL "\n")
				string(APPEND stripped_input "\n")
			endif()
		elseif(in_quoted_string)
			if(character STREQUAL "\n")
				string(APPEND stripped_input "\n")
			endif()

			if(escaped_character)
				set(escaped_character FALSE)
			elseif(character STREQUAL "\\")
				set(escaped_character TRUE)
			elseif(character STREQUAL "\"")
				set(in_quoted_string FALSE)
			endif()
		else()
			if(character STREQUAL "#")
				string(SUBSTRING "${input}" ${index} -1 remaining_input)
				string(REGEX MATCH "^#\\[(=*)\\[" bracket_open "${remaining_input}")
				if(bracket_open)
					set(bracket_argument_close "]${CMAKE_MATCH_1}]")
					set(bracket_argument_preserved_contents "")
					string(LENGTH "${bracket_open}" bracket_open_length)
					math(EXPR index "${index} + ${bracket_open_length}")
					set(in_bracket_argument TRUE)
					continue()
				endif()
				set(in_comment TRUE)
			elseif(character STREQUAL "[")
				string(SUBSTRING "${input}" ${index} -1 remaining_input)
				string(REGEX MATCH "^\\[(=*)\\[" bracket_open "${remaining_input}")
				if(bracket_open)
					set(bracket_argument_close "]${CMAKE_MATCH_1}]")
					set(bracket_argument_preserved_contents "")
					string(LENGTH "${bracket_open}" bracket_open_length)
					string(SUBSTRING "${remaining_input}" ${bracket_open_length} -1 bracket_contents_and_close)
					string(FIND "${bracket_contents_and_close}" "${bracket_argument_close}" bracket_close_index)
					if(NOT bracket_close_index EQUAL -1)
						string(SUBSTRING "${bracket_contents_and_close}" 0 ${bracket_close_index}
							bracket_argument_contents)
						list(FIND ARGN "${bracket_argument_contents}" bracket_receiver_source_index)
						if(NOT bracket_receiver_source_index EQUAL -1)
							set(bracket_argument_preserved_contents "${bracket_argument_contents}")
						endif()
					endif()
					math(EXPR index "${index} + ${bracket_open_length}")
					set(in_bracket_argument TRUE)
					continue()
				endif()
				string(APPEND stripped_input "${character}")
			elseif(character STREQUAL "\"")
				set(in_quoted_string TRUE)
			else()
				string(APPEND stripped_input "${character}")
			endif()
		endif()

		math(EXPR index "${index} + 1")
	endwhile()

	set(${output} "${stripped_input}" PARENT_SCOPE)
endfunction()

foreach(modern_cmake_file IN LISTS modern_cmake_files)
	file(READ "${modern_cmake_file}" modern_cmake)
	# Only inspect actual top-level CMake target commands. This deliberately leaves
	# source-list strings and other non-target contexts alone.
	string(REPLACE "\r\n" "\n" modern_cmake "${modern_cmake}")
	string(REPLACE "\r" "\n" modern_cmake "${modern_cmake}")
	set(modern_cmake_code "${modern_cmake}")
	set(receiver_source_index 0)
	foreach(receiver_source IN ITEMS
		"adapters/legacy/ConnectionSynchronizationShadow.cpp"
		"adapters/legacy/LegacyProtocolComposition.cpp"
		"adapters/legacy/LegacyProtocolReceiver.cpp")
		math(EXPR receiver_source_index "${receiver_source_index} + 1")
		set(receiver_source_placeholder "__MODERN_RECEIVER_SOURCE_${receiver_source_index}__")
		string(REPLACE "\"${receiver_source}\"" "${receiver_source_placeholder}" modern_cmake_code
			"${modern_cmake_code}")
	endforeach()
	strip_cmake_quoted_strings_and_comments("${modern_cmake_code}" modern_cmake_code
		"adapters/legacy/ConnectionSynchronizationShadow.cpp"
		"adapters/legacy/LegacyProtocolComposition.cpp"
		"adapters/legacy/LegacyProtocolReceiver.cpp")
	set(receiver_source_index 0)
	foreach(receiver_source IN ITEMS
		"adapters/legacy/ConnectionSynchronizationShadow.cpp"
		"adapters/legacy/LegacyProtocolComposition.cpp"
		"adapters/legacy/LegacyProtocolReceiver.cpp")
		math(EXPR receiver_source_index "${receiver_source_index} + 1")
		string(REPLACE "__MODERN_RECEIVER_SOURCE_${receiver_source_index}__" "${receiver_source}" modern_cmake_code
			"${modern_cmake_code}")
	endforeach()
	string(REPLACE "\n" ";" modern_cmake_lines "${modern_cmake_code}")
	set(modern_target_declarations)
	set(modern_target_declaration "")
	set(modern_target_declaration_open FALSE)
	list(LENGTH modern_cmake_lines modern_cmake_line_count)
	if(modern_cmake_line_count GREATER 0)
		math(EXPR modern_cmake_last_line "${modern_cmake_line_count} - 1")
	else()
		set(modern_cmake_last_line -1)
	endif()
	foreach(modern_cmake_line_index RANGE 0 ${modern_cmake_last_line})
		list(GET modern_cmake_lines ${modern_cmake_line_index} modern_cmake_line)
		string(TOLOWER "${modern_cmake_line}" modern_cmake_line_lower)
		if(modern_target_declaration_open)
			string(APPEND modern_target_declaration "\n${modern_cmake_line}")
			if(modern_cmake_line MATCHES "\\)")
				list(APPEND modern_target_declarations "${modern_target_declaration}")
				set(modern_target_declaration_open FALSE)
			endif()
		elseif(modern_cmake_line_lower MATCHES
			"^[ \t]*(add_library|add_executable|target_sources)[ \t]*\\(")
			set(modern_target_declaration "${modern_cmake_line}")
			if(modern_cmake_line MATCHES "\\)")
				list(APPEND modern_target_declarations "${modern_target_declaration}")
			else()
				set(modern_target_declaration_open TRUE)
			endif()
		endif()
	endforeach()

	foreach(modern_target_declaration IN LISTS modern_target_declarations)
		foreach(receiver_source IN ITEMS
			"adapters/legacy/ConnectionSynchronizationShadow.cpp"
			"adapters/legacy/LegacyProtocolComposition.cpp"
			"adapters/legacy/LegacyProtocolReceiver.cpp")
			string(FIND "${modern_target_declaration}" "${receiver_source}" receiver_source_offset)
			if(NOT receiver_source_offset EQUAL -1)
				message(FATAL_ERROR
					"${receiver_source} must not be declared by modern target in ${modern_cmake_file}")
			endif()
		endforeach()
	endforeach()
endforeach()
