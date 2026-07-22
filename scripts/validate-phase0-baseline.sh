#!/usr/bin/env bash
# Copyright The Mumble Developers. All rights reserved.
# Use of this source code is governed by a BSD-style license
# that can be found in the LICENSE file at the root of the
# Mumble source tree or at <https://www.mumble.info/LICENSE>.

set -euo pipefail

readonly EXPECTED_HEADER='run_id,utc_timestamp,git_revision,windows_version,display_scale_percent,build_type,scenario,metric,value,unit,notes'

fail() {
	printf 'phase0-baseline: %s\n' "$*" >&2
	exit 1
}

if (( $# < 1 || $# > 2 )); then
	fail 'usage: validate-phase0-baseline.sh <csv-path> [environment-path]'
fi

csv_path=$1
environment_path=${2:-}

[[ -f "$csv_path" ]] || fail "CSV file does not exist: $csv_path"
[[ -r "$csv_path" ]] || fail "CSV file is not readable: $csv_path"

declare -a secrets=()

if [[ -n "$environment_path" ]]; then
	[[ -f "$environment_path" ]] || fail "environment file does not exist: $environment_path"
	[[ -r "$environment_path" ]] || fail "environment file is not readable: $environment_path"

	while IFS= read -r line || [[ -n "$line" ]]; do
		line=${line%$'\r'}
		if [[ "$line" =~ ^(MUMBLE_PHASE0_SERVER_ADDRESS|MUMBLE_PHASE0_USERNAME|MUMBLE_PHASE0_PASSWORD|MUMBLE_PHASE0_CERTIFICATE_PATH)=(.*)$ ]]; then
			secret=${BASH_REMATCH[2]}
			if [[ -n "$secret" ]]; then
				secrets+=("$secret")
			fi
		else
			fail "invalid environment assignment: $line"
		fi
	done < "$environment_path"
fi

IFS= read -r header < "$csv_path" || fail 'CSV file is empty'
header=${header%$'\r'}
[[ "$header" == "$EXPECTED_HEADER" ]] || fail 'CSV header does not match the required schema'

line_number=1
while IFS= read -r line || [[ -n "$line" ]]; do
	((++line_number))
	line=${line%$'\r'}

	comma_count=${line//[^,]/}
	[[ ${#comma_count} -eq 10 ]] || fail "line $line_number has the wrong field count"

	IFS=, read -r run_id utc_timestamp git_revision windows_version display_scale_percent build_type scenario metric value unit notes <<< "$line"
	for required_value in "$run_id" "$utc_timestamp" "$git_revision" "$windows_version" "$display_scale_percent" "$build_type" "$scenario" "$metric" "$value" "$unit"; do
		[[ -n "$required_value" ]] || fail "line $line_number has an empty required value"
	done

	[[ "$display_scale_percent" =~ ^[0-9]+$ ]] || fail "line $line_number has a non-integer display scale"
	[[ "$value" =~ ^[0-9]+([.][0-9]+)?$ ]] || fail "line $line_number has a non-decimal metric value"
done < <(tail -n +2 "$csv_path")

for secret in "${secrets[@]}"; do
	if grep -Fq -- "$secret" "$csv_path"; then
		fail 'CSV contains a configured secret value'
	fi
done
