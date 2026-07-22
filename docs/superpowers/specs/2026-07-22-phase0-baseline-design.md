# Phase 0 Baseline Design

## Purpose

Create the reproducible Windows baseline required by Phase 0 of the modern
client migration. This work records the existing Widgets client's behavior and
performance without changing production behavior or introducing modern-client
runtime code.

## Scope

The deliverable contains four repository-tracked artifacts:

1. A Windows baseline guide with exact prerequisites, Debug and Release CMake
   commands, an output-directory convention, and evidence to retain.
2. A structured smoke-test checklist for Windows 10 and Windows 11. It covers
   connection, certificate decisions, audio, chat, shortcuts, tray, overlay,
   plugins, reconnect, and shutdown. It records display scaling, theme, audio
   device, network scenario, and pass/fail evidence for every execution.
3. A controlled-server template and local configuration procedure. The tracked
   template contains only placeholder values. A local ignored environment file
   supplies a server address, test account, password, and certificate paths.
4. A repeatable measurement procedure and machine-readable CSV schema for
   startup, memory, synchronization, and model-update measurements. A shell
   validator checks the schema and rejects files containing the configured
   secret values.

## Architecture And Data Flow

Phase 0 is documentation and measurement tooling only. The existing client
binary remains the sole executable under test. An operator copies the tracked
server environment template to an ignored local file, starts or selects a
controlled server, builds the unchanged client, completes the smoke checklist,
and writes measurements using the documented schema. The validator consumes
only a measurement CSV and the local environment file; it neither starts the
client nor contacts a server.

GitHub Actions validates the tracked scripts and documentation structure. It
does not claim that GitHub-hosted runners performed Windows interactive smoke
tests or measured end-user performance. Windows evidence is generated on real
Windows 10 and Windows 11 test machines and attached to the associated change
or release record.

## Components

`docs/dev/modern-client/phase0-baseline.md`
: The operator guide. It specifies supported systems, toolchain and dependency
  inputs, Debug and Release commands, server setup, smoke execution, log
  redaction, measurement collection, and the Phase 0 exit-evidence checklist.

`docs/dev/modern-client/phase0-smoke-checklist.md`
: A checkbox-oriented test protocol. Each test case has a stable identifier,
  preconditions, steps, expected result, and evidence field. It is executable
  manually on both supported Windows versions.

`docs/dev/modern-client/phase0-test-server.env.example`
: A non-secret template containing names but no values for test-server inputs.
  Local values live in `phase0-test-server.env`, which is ignored by Git.

`docs/dev/modern-client/phase0-measurements.csv.example`
: A header and one syntactically valid sample row using synthetic values. It
  defines one record per run, scenario, build type, and metric.

`scripts/validate-phase0-baseline.sh`
: A POSIX shell validator. It accepts a CSV path and optional environment-file
  path, validates the exact header and field count, checks required fields and
  numeric metric values, and rejects secret values sourced from the local
  environment file.

`.github/workflows/phase0-baseline.yml`
: A pull-request and push workflow that runs the validator against the example
  CSV without a local environment file and checks that the required Phase 0
  artifacts exist.

`src/tests/TestPhase0BaselineValidation/`
: A CTest target that invokes the validator against valid and intentionally
  invalid fixtures. The test has no client, network, audio, or `Global`
  dependency.

## Error Handling And Security

The validator exits nonzero and writes a specific diagnostic when a file is
missing, a CSV header differs, a row has the wrong number of fields, a required
field is empty, a numeric metric is malformed, or a configured secret appears
in the CSV. It reads only environment variable assignments whose names begin
with `MUMBLE_PHASE0_`; it does not execute the environment file.

The tracked template never includes credentials, private keys, personal server
addresses, or recorded audio. Logs attached as evidence must be redacted using
the guide's required token and certificate checks.

## Testing

The CTest suite verifies a valid CSV, invalid header, invalid metric, missing
required field, and secret-leak rejection. GitHub Actions runs this test on the
existing supported build matrix. Manual Windows evidence proves the interactive
requirements and must identify OS version, scaling, build revision, test-server
revision, test operator, and execution time.

## Non-Goals

Phase 0 does not add QML, modify `MainWindow`, alter protocol handling, add
application contracts, collect production telemetry, commit credentials, or
mark a migration phase complete without the real Windows evidence required by
the migration plan.
