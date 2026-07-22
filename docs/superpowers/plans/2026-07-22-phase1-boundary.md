# Phase 1 Boundary Skeleton Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a compile-time-enforced modern application boundary with no change to the legacy client path.

**Architecture:** Contracts own pointer-free Qt Core values and abstract ports. Application owns a thread-affine connection store and accepts ports by construction. Presentation consumes application state only. CMake target and source checks enforce the boundary.

**Tech Stack:** C++17, Qt 6 Core/Test, CMake, CTest.

## Global Constraints

- Application and contracts must not include or link Qt Widgets, Qt Quick, Win32, protobuf, `Global`, `MainWindow`, `ServerHandler`, `ClientUser`, or `Channel`.
- The existing `mumble` executable must not link or instantiate any modern Phase 1 object.
- Tests must construct no `Global`, window, socket, or audio device.
- The legacy build and smoke behavior remain unchanged.

---

### Task 1: Establish Modern CMake Targets And Dependency Gate

**Files:**
- Create: `src/mumble/modern/CMakeLists.txt`
- Modify: `src/mumble/CMakeLists.txt`
- Create: `src/mumble/modern/cmake/verify_layer_dependencies.cmake`
- Test: `src/tests/TestModernClientBoundary/CMakeLists.txt`

- [ ] Add the four static libraries with the dependency graph in the design, all initially containing a source file.
- [ ] Link contracts/application/presentation only to `Qt6::Core`; link testing only to contracts.
- [ ] Add an `ALL` custom target that scans contracts and application source/header files for forbidden include tokens and fails with a source path and token.
- [ ] Add `modern` beside the legacy client target only; do not link it into `mumble`.
- [ ] Add a CTest fixture that runs the verifier with a generated forbidden include and expects failure.
- [ ] Configure and run the focused test; commit `BUILD(client): Add modern boundary targets`.

### Task 2: Add Contracts And Fake Ports

**Files:**
- Create: `src/mumble/modern/contracts/ClientTypes.h`
- Create: `src/mumble/modern/contracts/ConnectionSnapshot.h`
- Create: `src/mumble/modern/contracts/ports/ConnectionPort.h`
- Create: `src/mumble/modern/contracts/ports/SessionCommandPort.h`
- Create: `src/mumble/modern/testing/FakeConnectionPort.h`
- Create: `src/mumble/modern/testing/FakeSessionCommandPort.h`
- Test: `src/tests/TestModernClientBoundary/TestModernClientBoundary.cpp`

- [ ] Write QtTest cases for strong-ID equality, initial snapshots, and fake-port inertness before implementation.
- [ ] Implement explicit value wrappers, connection phase/snapshot, and pure virtual port APIs using only Qt Core value types.
- [ ] Implement fakes that record calls but have no side effects.
- [ ] Run the focused QtTest and commit `TEST(client): Add modern contracts and fake ports`.

### Task 3: Add Application And Presentation Skeleton

**Files:**
- Create: `src/mumble/modern/application/stores/ConnectionStore.h`
- Create: `src/mumble/modern/application/stores/ConnectionStore.cpp`
- Create: `src/mumble/modern/application/ClientApplication.h`
- Create: `src/mumble/modern/application/ClientApplication.cpp`
- Create: `src/mumble/modern/ModernClientBootstrap.h`
- Create: `src/mumble/modern/ModernClientBootstrap.cpp`
- Create: `src/mumble/modern/presentation/ConnectionPresentationProbe.h`
- Create: `src/mumble/modern/presentation/ConnectionPresentationProbe.cpp`
- Modify: `src/tests/TestModernClientBoundary/TestModernClientBoundary.cpp`

- [ ] Write failing tests proving initial state, same-thread snapshot replacement, application construction with fake ports, and presentation observation.
- [ ] Implement a store capturing `QThread::currentThread()` at construction and asserting that access occurs on that thread in debug builds.
- [ ] Implement `ClientApplication` ownership and bootstrap construction without concrete adapters or executable registration.
- [ ] Implement a presentation probe exposing the current connection phase as a Qt Core property.
- [ ] Run focused QtTest and commit `FEAT(client): Add modern application boundary skeleton`.

### Task 4: Review And Verify Phase 1

**Files:** Review all Phase 1 changes.

- [ ] Run `git diff --check`, source dependency gate, focused CTest, and the legacy relevant build/CTest matrix.
- [ ] Confirm `git grep` finds no forbidden include in contracts or application.
- [ ] Conduct task-level and full branch review; fix all Critical and Important findings.
- [ ] Push a Phase 1 PR, wait for Actions, and preserve legacy-mode behavior.
- [ ] Mark Phase 1 complete only after application tests run without legacy runtime objects and the deliberate forbidden-dependency fixture fails as designed.
