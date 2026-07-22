# Phase 2 Neutral Protocol Entry Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Route inbound TCP control messages through an immutable, ordered adapter boundary before legacy dispatch.

**Architecture:** The adapter layer owns the raw envelope and protobuf parsing. A temporary legacy receiver calls the existing `MainWindow::msgX` methods after a main-thread ordering and attempt guard accepts a parsed message. Application and presentation remain free of protobuf and runtime dependencies.

**Tech Stack:** C++17, Qt 6 Core signals/queued delivery, protobuf, existing ServerHandler, CMake and QtTest.

## Global Constraints

- Only adapter sources may include both modern contracts and legacy runtime/protobuf headers.
- `ConnectionAttemptId` rejects cancelled or stale queued control messages.
- Parse failures produce diagnostics and no receiver call.
- Legacy handlers remain the sole state-mutating path in this phase.
- Preserve `UDPTunnel` audio and transport-local Ping bookkeeping.

---

### Task 1: Envelope And Ordering Guard

**Files:** Create `src/mumble/modern/adapters/ControlMessageEnvelope.h`, `ProtocolEventAdapter.h/.cpp`; modify modern CMake and `TestModernClientBoundary`.

- [ ] Write QtTest cases for immutable copied payload, monotonic sequence acceptance, stale sequence rejection, and cancelled-attempt rejection.
- [ ] Run the focused test and observe failure because the envelope/adapter do not exist.
- [ ] Implement the Qt Core envelope and main-thread adapter guard with an injected parsed-message callback; add the adapter target without linking it to application.
- [ ] Run focused CTest and the modern dependency gate; commit `FEAT(client): Add ordered protocol envelope`.

### Task 2: Protobuf Parsing And Legacy Receiver

**Files:** Create `src/mumble/modern/adapters/legacy/LegacyProtocolReceiver.h/.cpp`; extend `ProtocolEventAdapter`; modify CMake and focused tests.

- [ ] Add fixtures for valid representative control protobuf dispatch and malformed bytes producing a diagnostic without dispatch.
- [ ] Run focused tests and observe parsing/receiver failures.
- [ ] Implement type-to-protobuf parsing in the adapter and a receiver that invokes the matching legacy `msgX` method; keep `MainWindow.h` confined to the receiver.
- [ ] Run focused tests, `git diff --check`, and commit `FEAT(client): Add legacy protocol receiver`.

### Task 3: ServerHandler Handoff And Transport Events

**Files:** Modify `src/mumble/ServerHandler.h/.cpp`, legacy composition/startup wiring, adapter tests, and modern-client status documentation.

- [ ] Add an integration test proving a non-Ping/non-audio control message is delivered through the receiver in receive order and a cancelled attempt is discarded.
- [ ] Run it and observe the existing direct `QApplication::postEvent(Global::get().mw, ...)` path.
- [ ] Replace that direct post with queued envelope delivery; retain Ping and audio behavior; map UDP degraded/restored transitions to structured legacy-presented events.
- [ ] Run focused adapter/integration tests and relevant legacy checks; commit `FEAT(client): Route control messages through adapter`.

### Task 4: Phase Review And CI

**Files:** Modify `docs/dev/modern-client/README.md` only when Phase 2 exit evidence is complete.

- [ ] Review every task independently for architectural and legacy behavior regressions.
- [ ] Run `git diff --check`, dependency verification, focused CTest, and the available legacy build checks.
- [ ] Push a PR; inspect CI logs; record only confirmed environment failures for final release remediation.
- [ ] Merge only after the Phase 2 exit criteria have evidence or an explicitly accepted exception.
