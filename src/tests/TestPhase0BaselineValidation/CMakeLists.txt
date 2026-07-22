# Copyright The Mumble Developers. All rights reserved.
# Use of this source code is governed by a BSD-style license
# that can be found in the LICENSE file at the root of the
# Mumble source tree or at <https://www.mumble.info/LICENSE>.

find_program(BASH_EXECUTABLE bash REQUIRED)

set(PHASE0_BASELINE_VALIDATOR "${CMAKE_SOURCE_DIR}/scripts/validate-phase0-baseline.sh")

add_test(
	NAME TestPhase0BaselineValidationValid
	COMMAND "${BASH_EXECUTABLE}" "${PHASE0_BASELINE_VALIDATOR}" "${CMAKE_CURRENT_SOURCE_DIR}/valid.csv")

add_test(
	NAME TestPhase0BaselineValidationInvalidHeader
	COMMAND "${BASH_EXECUTABLE}" "${PHASE0_BASELINE_VALIDATOR}" "${CMAKE_CURRENT_SOURCE_DIR}/invalid-header.csv")
set_tests_properties(TestPhase0BaselineValidationInvalidHeader PROPERTIES WILL_FAIL TRUE)

add_test(
	NAME TestPhase0BaselineValidationInvalidMetric
	COMMAND "${BASH_EXECUTABLE}" "${PHASE0_BASELINE_VALIDATOR}" "${CMAKE_CURRENT_SOURCE_DIR}/invalid-metric.csv")
set_tests_properties(TestPhase0BaselineValidationInvalidMetric PROPERTIES WILL_FAIL TRUE)

add_test(
	NAME TestPhase0BaselineValidationMissingValue
	COMMAND "${BASH_EXECUTABLE}" "${PHASE0_BASELINE_VALIDATOR}" "${CMAKE_CURRENT_SOURCE_DIR}/missing-value.csv")
set_tests_properties(TestPhase0BaselineValidationMissingValue PROPERTIES WILL_FAIL TRUE)

add_test(
	NAME TestPhase0BaselineValidationSecret
	COMMAND "${BASH_EXECUTABLE}" "${PHASE0_BASELINE_VALIDATOR}" "${CMAKE_CURRENT_SOURCE_DIR}/contains-secret.csv" "${CMAKE_CURRENT_SOURCE_DIR}/secrets.env")
set_tests_properties(TestPhase0BaselineValidationSecret PROPERTIES WILL_FAIL TRUE)

add_test(
	NAME TestPhase0BaselineValidationQuotedSecret
	COMMAND "${BASH_EXECUTABLE}" "${PHASE0_BASELINE_VALIDATOR}" "${CMAKE_CURRENT_SOURCE_DIR}/valid.csv" "${CMAKE_CURRENT_SOURCE_DIR}/quoted-secret.env")
set_tests_properties(TestPhase0BaselineValidationQuotedSecret PROPERTIES WILL_FAIL TRUE)

add_test(
	NAME TestPhase0BaselineValidationHeaderOnly
	COMMAND "${BASH_EXECUTABLE}" "${PHASE0_BASELINE_VALIDATOR}" "${CMAKE_CURRENT_SOURCE_DIR}/header-only.csv")
set_tests_properties(TestPhase0BaselineValidationHeaderOnly PROPERTIES WILL_FAIL TRUE)
