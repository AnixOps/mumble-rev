# Phase 0 Baseline Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Provide reproducible, credential-safe Windows baseline evidence for the legacy client without changing production behavior.

**Architecture:** A POSIX shell validator enforces a CSV contract and detects configured local secrets without executing local configuration. Documentation defines human-operated Windows build, smoke, server, and measurement procedures. CTest runs positive and negative fixtures; a lightweight GitHub Actions job runs static checks independently of the full build matrix.

**Tech Stack:** CMake/CTest, POSIX shell, GitHub Actions, Markdown, CSV.

## Global Constraints

- Phase 0 must not change production client behavior.
- No modern-client dependency may cross the boundaries defined in `docs/dev/modern-client/architecture.md`.
- Credentials, private certificates, personal addresses, and recorded audio must not be tracked.
- CI must not claim to have executed interactive Windows smoke tests.
- Run `git diff --check`, focused tests, and relevant existing checks before submission.
- Do not mark Phase 0 complete without actual Windows evidence satisfying `migration-plan.md`.

---

### Task 1: Measurement Validator And CTest Fixtures

**Files:**
- Create: `scripts/validate-phase0-baseline.sh`
- Create: `src/tests/TestPhase0BaselineValidation/CMakeLists.txt`
- Create: `src/tests/TestPhase0BaselineValidation/{valid,invalid-header,invalid-metric,missing-value,contains-secret}.csv`
- Create: `src/tests/TestPhase0BaselineValidation/secrets.env`
- Modify: `src/tests/CMakeLists.txt`

**Interfaces:** `scripts/validate-phase0-baseline.sh <csv-path> [environment-path]` exits `0` only for a valid, non-secret CSV. CTest names are `TestPhase0BaselineValidationValid`, `TestPhase0BaselineValidationInvalidHeader`, `TestPhase0BaselineValidationInvalidMetric`, `TestPhase0BaselineValidationMissingValue`, and `TestPhase0BaselineValidationSecret`.

- [ ] **Step 1: Write failing CTest registration and fixtures**

Register the directory after `TestTimer` in `src/tests/CMakeLists.txt`. Add a CMake file that finds `bash`, runs `scripts/validate-phase0-baseline.sh` for a valid fixture, and marks invalid cases with `WILL_FAIL TRUE`. Configure and run the focused CTest suite with:

```bash
cmake -S . -B build-phase0 -Dtests=ON -Dclient=ON -Dserver=OFF
ctest --test-dir build-phase0 -R Phase0Baseline --output-on-failure
```

`client=ON` is a CMake project-selection prerequisite only; the Phase 0 validator CTest neither constructs a client window nor requires a network connection, audio device, or `Global`. The focused CTest run must fail because the validator is absent.

- [ ] **Step 2: Implement the validator**

Use `#!/usr/bin/env bash` and `set -euo pipefail`. Require an existing CSV and optional existing environment file. Accept only `MUMBLE_PHASE0_SERVER_ADDRESS`, `MUMBLE_PHASE0_USERNAME`, `MUMBLE_PHASE0_PASSWORD`, and `MUMBLE_PHASE0_CERTIFICATE_PATH` assignments without executing the file. Require this exact eleven-column header:

```text
run_id,utc_timestamp,git_revision,windows_version,display_scale_percent,build_type,scenario,metric,value,unit,notes
```

Reject wrong field counts, empty required values, non-integer scaling, non-decimal metric values, and every configured non-empty secret present in the CSV. Emit `phase0-baseline:` diagnostics and exit `1` on failure.

- [ ] **Step 3: Verify tests and commit**

Run `ctest --test-dir build-phase0 -R Phase0Baseline --output-on-failure`; all five cases must pass. Commit with `TEST(client): Validate Phase 0 baseline data`.

### Task 2: Baseline Procedure And Evidence Artifacts

**Files:**
- Create: `docs/dev/modern-client/phase0-baseline.md`
- Create: `docs/dev/modern-client/phase0-smoke-checklist.md`
- Create: `docs/dev/modern-client/phase0-test-server.env.example`
- Create: `docs/dev/modern-client/phase0-measurements.csv.example`
- Modify: `.gitignore`
- Modify: `docs/dev/modern-client/README.md`

**Interfaces:** The local `docs/dev/modern-client/phase0-test-server.env` is ignored. The CSV example is accepted by Task 1's validator. The README links both procedure and checklist but retains its unstarted implementation status.

- [ ] **Step 1: Write the Windows procedure**

Document Windows 10/11, 100%/150%/200% scaling, Visual Studio x64 Native Tools Command Prompt, and Debug/Release Ninja CMake commands using `-Dstatic=ON`, `-Dtests=ON`, `-DVCPKG_TARGET_TRIPLET=x64-windows-static-md`, `-DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\\scripts\\buildsystems\\vcpkg.cmake`, and `-DIce_HOME=%VCPKG_ROOT%\\installed\\x64-windows-static-md`. Require capture of Git revision, OS, display scale, build type, test-server revision, operator, timestamp, redacted logs, and raw measurements.

- [ ] **Step 2: Write the smoke protocol and template**

Create `P0-SMOKE-01` through `P0-SMOKE-10`, each with preconditions, steps, expected result, and evidence location. Cover connection/cancellation, valid/rejected certificates, audio/privacy, channel/private chat, shortcuts, tray, overlay/plugins, reconnect, and shutdown. The template has empty values for all four `MUMBLE_PHASE0_` assignments. Ignore the local copy.

- [ ] **Step 3: Add measurements and link documentation**

Use the exact header from Task 1 and synthetic rows for `startup_to_window_ms`, `working_set_mb`, `synchronization_ms`, and `talk_state_update_ms`. Require five warm runs per scenario and report median/maximum. Validate the example, link both docs from README, then commit with `DOCS(client): Add Phase 0 baseline procedure`.

### Task 3: CI Static Gate

**Files:**
- Create: `.github/workflows/phase0-baseline.yml`

**Interfaces:** A `Phase 0 baseline` workflow runs on `push` and `pull_request`, verifies all Task 2 artifacts, requires the validator executable, and validates the sample CSV.

- [ ] **Step 1: Add the workflow**

Create an `ubuntu-latest` `validate` job using `actions/checkout@v6`. It runs `test -f` for each documentation/template/example file, `test -x scripts/validate-phase0-baseline.sh`, and `bash scripts/validate-phase0-baseline.sh docs/dev/modern-client/phase0-measurements.csv.example`.

- [ ] **Step 2: Verify locally and commit**

Run the same shell commands locally. Commit with `CI(client): Validate Phase 0 baseline artifacts`.

### Task 4: Review, Push, And Verify

**Files:** Review all Task 1-3 changes.

**Interfaces:** Produces a reviewed branch, push evidence, and Actions run results. It does not produce a release because Phase 0 is not a release gate.

- [ ] **Step 1: Code review**

Inspect `git diff master...HEAD --check`, the complete diff, shell syntax, CMake registration, architecture boundaries, and every non-empty `MUMBLE_PHASE0_` value. Permit only the intentional negative test password in its fixture.

- [ ] **Step 2: Verification**

Run `bash -n scripts/validate-phase0-baseline.sh`, validate the CSV example, run focused CTest, and run `git diff master...HEAD --check`; all must exit `0`.

- [ ] **Step 3: Push and inspect GitHub Actions**

Push `phase0-baseline`, invoke `gh workflow view "Phase 0 baseline"`, and wait for Build, Code Scanning, FreeBSD, and Phase 0 baseline runs. Record failures rather than treating a workflow dispatch as evidence of a release.

- [ ] **Step 4: Continue only with real evidence**

Before marking Phase 0 complete, attach results from actual Windows Debug/Release builds, Windows 10/11 smoke execution, controlled-server runs, redacted logs, and repeatable measurements. Then begin Phase 1 using the same design-plan-review-CI sequence.
