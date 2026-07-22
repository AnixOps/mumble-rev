# Repository Agent Guide

This file applies to the entire repository.

## Project direction

This fork is building a modern Windows Mumble client. The accepted architecture
is an in-process application boundary with a Qt Quick/QML presentation and the
existing Mumble protocol, audio, plugin, overlay, database, and Windows runtime.

The architecture decision is complete. Implementation has not started. Unless a
newer user request explicitly changes the priority, begin with phase 0 of the
migration plan and do not jump directly to QML feature work.

## Mandatory reading

Before planning or changing modern-client code, read these files completely in
this order:

1. `docs/dev/modern-client/README.md`
2. `docs/dev/modern-client/architecture.md`
3. `docs/dev/modern-client/application-contracts.md`
4. `docs/dev/modern-client/migration-plan.md`
5. `docs/dev/modern-client/adr/0001-in-process-application-boundary.md`
6. `docs/dev/modern-client/cross-platform-alignment.md` when work affects
   protocol compatibility or coordination with Apple clients

The architecture and accepted ADR are normative. If implementation evidence
requires changing a normative boundary, stop, document the tradeoff in a new
ADR, and obtain the user's decision before implementing the conflicting design.

## Non-negotiable boundaries

- Do not create a hidden legacy `MainWindow` to power the modern UI.
- QML may access only presentation view models and Qt item models.
- QML and presentation must not access `Global`, `MainWindow`, `ServerHandler`,
  protobuf objects, `ClientUser *`, or `Channel *`.
- Application code may use Qt Core but must not depend on Qt Widgets, Qt Quick,
  Win32, protobuf-generated headers, or legacy runtime classes.
- Only legacy adapters may include both modern application contracts and legacy
  runtime headers.
- Protocol parsing belongs in the adapter boundary. Application services receive
  typed value events.
- Session-scoped commands and identities must be guarded by
  `ConnectionEpoch`; pre-session activity must be guarded by
  `ConnectionAttemptId`.
- Stores are the single owners of UI-facing application state. Views do not keep
  competing business state.
- Network and audio threads never mutate stores or presentation directly.
- PCM buffers never cross into application or presentation. UI meters consume
  bounded-rate snapshots.
- Talk-state-only changes must not reset a whole channel or member model.
- Do not move or rename broad sets of upstream files merely to match the target
  directory diagram. Prefer additive modules and narrow extractions.
- Keep the existing cross-platform runtime intact even though the first modern
  release target is Windows 10 and Windows 11.

## Migration workflow

1. Identify the current migration phase and its exit criteria before editing.
2. Keep the legacy client runnable at the end of every phase.
3. Introduce contracts and tests before making a new path authoritative.
4. Migrate one protocol message family at a time. Never leave two
   state-mutating consumers active.
5. Use shadow mode only for comparison. Shadow consumers cannot send messages,
   write settings, start audio, or notify users.
6. Add focused unit, adapter, model, or end-to-end tests in proportion to the
   boundary being changed.
7. Run the relevant legacy regression checks after touching existing runtime
   files.
8. Update the modern-client documentation when a contract, phase status, risk,
   or accepted decision changes.

Do not mark a phase complete because its files exist. Every exit criterion in
`docs/dev/modern-client/migration-plan.md` must be satisfied or explicitly
recorded as open.

## Current next step

Phase 0 is the next unstarted phase. Its expected output is a reproducible
Windows debug and release baseline, a smoke-test checklist, a controlled test
server setup, and initial startup, memory, synchronization, and model-update
measurements. Phase 0 should not change production behaviour.

After phase 0, phase 1 introduces the boundary skeleton. Planned names such as
`modern-ui`, `legacy-ui`, `ModernClientBootstrap`, and the
`mumble_modern_*` CMake targets are contracts in the documentation, not existing
implementation. Verify the tree before using them.

## Completion checks

Before handing off a change:

- run `git diff --check`;
- run all relevant automated tests and state any tests that could not run;
- verify that no new forbidden dependency crosses an architectural layer;
- verify that existing user changes in the worktree were preserved;
- update phase status only when its exit criteria pass; and
- summarize changed behaviour, verification, and remaining migration work.
