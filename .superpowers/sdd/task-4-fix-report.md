# Phase 0 Final Review Fix Report

## Scope

Implemented only the final-review fixes requested for Phase 0 validation. No
client production behavior or modern-client architectural boundary changed.

## RED Evidence

Before changing `scripts/validate-phase0-baseline.sh`, the following direct
checks both succeeded unexpectedly (exit status `0`):

```bash
bash scripts/validate-phase0-baseline.sh src/tests/TestPhase0BaselineValidation/valid.csv src/tests/TestPhase0BaselineValidation/quoted-secret.env
bash scripts/validate-phase0-baseline.sh src/tests/TestPhase0BaselineValidation/header-only.csv
```

The quoted environment value was treated as a literal fixed-string secret, and
the CSV parser accepted a file containing only the exact valid header.

## Changes

- Documented Git for Windows, including Git Bash on `PATH`, as a prerequisite
  for the Phase 0 CTest validator and direct validator command.
- Reject raw values beginning with single or double quotes for every supported
  `MUMBLE_PHASE0_` assignment before any secret processing or normalization.
- Require at least one data row after the exact CSV header.
- Added negative fixtures and `WILL_FAIL TRUE` CTest registrations for quoted
  secret environment values and header-only CSV files. Added direct-check
  fixtures for the remaining supported environment keys.

## GREEN Evidence

```bash
bash -n scripts/validate-phase0-baseline.sh
bash scripts/validate-phase0-baseline.sh src/tests/TestPhase0BaselineValidation/valid.csv
```

Both commands exited `0`. The full direct fixture matrix exited `0` after
confirming expected nonzero exits for `invalid-header.csv`, `invalid-metric.csv`,
`missing-value.csv`, `header-only.csv`, `contains-secret.csv` with `secrets.env`,
and all four quoted-environment fixtures. Quoted fixtures emitted:

```text
phase0-baseline: quoted values are unsupported in environment assignments
```

The header-only fixture emitted:

```text
phase0-baseline: CSV must contain at least one data row
```

`git diff --check` exited `0`.

## CMake Constraint

Attempted the supported configure command available on this Linux host:

```bash
cmake -S . -B build-phase0-validation -Dtests=ON
```

It reached project configuration and failed in `overlay_gl/CMakeLists.txt`
because the host lacks the 32-bit `sys/cdefs.h` dependency (`g++-multilib`).
No alternate build configuration or dependency installation was introduced.
Consequently, registered CTests could not be generated or run in this host
environment.

## Changed Files

- `docs/dev/modern-client/phase0-baseline.md`
- `scripts/validate-phase0-baseline.sh`
- `src/tests/TestPhase0BaselineValidation/CMakeLists.txt`
- `src/tests/TestPhase0BaselineValidation/header-only.csv`
- `src/tests/TestPhase0BaselineValidation/quoted-address.env`
- `src/tests/TestPhase0BaselineValidation/quoted-username.env`
- `src/tests/TestPhase0BaselineValidation/quoted-secret.env`
- `src/tests/TestPhase0BaselineValidation/quoted-certificate-path.env`

## Self-Review

- The exact existing CSV header and valid fixture remain unchanged.
- Environment files remain parsed as raw text and are not executed.
- Unquoted configured values continue to use the existing `grep -Fq` fixed-string
  secret comparison.
- All diagnostics use the existing `phase0-baseline:` prefix through `fail`.
- The edit introduces no runtime dependency, credential, certificate, personal
  address, audio artifact, or production-client behavior change.
- No application, presentation, adapter, or legacy runtime dependency boundary
  was changed. Phase 0 remains unstarted.
