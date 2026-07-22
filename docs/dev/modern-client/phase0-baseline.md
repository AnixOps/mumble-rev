# Phase 0 Windows Baseline Procedure

Status: procedure only. Phase 0 remains unstarted until the evidence required
by the [migration plan](migration-plan.md) has been captured on real Windows
test machines.

This procedure measures and verifies the unmodified legacy client. It does not
enable a modern-client path, change production behaviour, or authorize a Phase
0 exit.

## Scope and prerequisites

Run the procedure on Windows 10 and Windows 11 at 100%, 150%, and 200% display
scaling. Use a Visual Studio **x64 Native Tools Command Prompt**; confirm that
it reports an x64 environment before configuring. Install Git for Windows
(including Git Bash) on `PATH`, Ninja, CMake, and the pinned Mumble vcpkg
dependency set described in the existing
[static-build instructions](../build-instructions/build_static.md).

Git Bash is required by the Phase 0 CTest validator and by the direct validator
command below.

Set `VCPKG_ROOT` to the prepared vcpkg directory. Do not place credentials,
private certificates, personal server addresses, or recorded audio under source
control.

## Build the legacy client

From the repository root, configure separate build directories in a Visual
Studio x64 Native Tools Command Prompt. Use the following commands exactly,
changing only the source and build paths as needed:

```bat
cmake -S . -B build-phase0-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug -Dstatic=ON -Dtests=ON -DVCPKG_TARGET_TRIPLET=x64-windows-static-md -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake -DIce_HOME=%VCPKG_ROOT%\installed\x64-windows-static-md
cmake --build build-phase0-debug --parallel
ctest --test-dir build-phase0-debug --output-on-failure

cmake -S . -B build-phase0-release -G Ninja -DCMAKE_BUILD_TYPE=Release -Dstatic=ON -Dtests=ON -DVCPKG_TARGET_TRIPLET=x64-windows-static-md -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake -DIce_HOME=%VCPKG_ROOT%\installed\x64-windows-static-md
cmake --build build-phase0-release --parallel
ctest --test-dir build-phase0-release --output-on-failure
```

Retain the configure, build, and test logs for both build types. The logs are
evidence artifacts, not tracked source files.

## Controlled test server

Use a controlled Mumble server with a known revision and separately managed
test accounts. Copy
[`phase0-test-server.env.example`](phase0-test-server.env.example) to
`phase0-test-server.env`, fill the local copy, and restrict its filesystem
permissions. The local file is ignored by Git and must not be committed.

Record the server revision and configuration identity in the evidence record;
do not record passwords, private-key material, or private server details. Use
the same server revision and scenario configuration for comparison runs.

## Smoke evidence

Execute every case in the [Phase 0 smoke checklist](phase0-smoke-checklist.md)
for each supported Windows version, display scale, and build type that is being
baselined. For every execution, record the case ID, pass/fail outcome, build
directory, test-server revision, network scenario, audio device, theme, and an
evidence location.

Capture a normal-session log and representative rejected-certificate,
connection-cancellation, privacy-blocked-audio, and reconnect logs. Before
retention or attachment, redact server addresses, usernames, passwords, access
tokens, certificate private keys, machine names, and personally identifying
paths. Keep the unredacted originals only in the approved restricted evidence
store, if that is required by the test environment.

## Measurements

Use [`phase0-measurements.csv.example`](phase0-measurements.csv.example) as the
schema for raw measurement records. Each row has this exact header:

```text
run_id,utc_timestamp,git_revision,windows_version,display_scale_percent,build_type,scenario,metric,value,unit,notes
```

For every scenario and build type, perform five warm runs after an initial
unrecorded warm-up. Record raw values for each run and metric, then report the
median and maximum for each metric and scenario in the associated evidence
summary. At minimum collect:

- `startup_to_window_ms`: process start to usable main window.
- `working_set_mb`: working set after the scenario reaches a stable state.
- `synchronization_ms`: connection start to completed initial synchronization.
- `talk_state_update_ms`: time for a talk-state update in the selected model
  scenario.

Use the validator before retaining a CSV:

```bat
bash scripts/validate-phase0-baseline.sh path\to\phase0-measurements.csv docs\dev\modern-client\phase0-test-server.env
```

The validator checks schema and rejects configured non-empty local values found
in the CSV. It does not replace the required redaction review.

## Evidence record

For each baseline run, retain an evidence record containing:

- Git revision;
- Windows edition, version, and build;
- display scale percentage;
- build type and build directory;
- controlled test-server revision;
- operator and UTC timestamp;
- redacted configure, build, test, and client logs;
- smoke-checklist results and evidence locations; and
- raw measurement CSVs plus the five-run median and maximum summary.

Phase 0 is complete only when the migration-plan exit criteria are satisfied
with this real Windows evidence. Creating these templates or passing the CSV
validator does not satisfy those criteria.
