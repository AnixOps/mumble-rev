# Phase 1 Boundary Skeleton Design

## Purpose

Introduce the first compile-time-enforced modern-client application boundary
without changing the legacy client runtime path. The work satisfies Phase 1 of
the accepted migration plan and provides a testable base for later adapters and
presentation.

## Components

`src/mumble/modern/contracts`
: Qt Core-only value contracts: strong connection and entity identity wrappers,
connection phase, a minimal immutable connection snapshot, and explicit
`ConnectionPort` and `SessionCommandPort` interfaces. No protobuf, runtime
pointer, Widgets, Quick, or Win32 type is exposed.

`src/mumble/modern/application`
: `ConnectionStore` stores the connection snapshot on its construction thread
and asserts that all reads and writes occur there in debug builds.
`ClientApplication` owns the store and accepts abstract ports through
construction. It has no concrete adapter or UI dependency.

`src/mumble/modern/presentation`
: A minimal Qt Core presentation probe used solely to prove that presentation
can consume application state without including legacy headers. It does not
create QML or a window in Phase 1.

`src/mumble/modern/testing`
: Fake implementations of the contract ports that record calls for application
tests. They are test-only and cannot send protocol messages, write settings,
start audio, or show notifications.

`src/mumble/modern/CMakeLists.txt`
: Defines `mumble_modern_contracts`, `mumble_modern_application`,
`mumble_modern_presentation`, and `mumble_modern_testing`. The application and
contracts targets link only Qt Core. A configure-time source scan rejects
forbidden includes (`Global.h`, `MainWindow.h`, `ServerHandler.h`, legacy
entity headers, protobuf headers, Qt Widgets, and Qt Quick) in those layers.

`src/tests/TestModernClientBoundary`
: QtTest executable that constructs the application with fake ports, verifies
the initial snapshot and thread-affinity assertion behavior, and verifies that
the fake ports are inert until explicitly invoked by a later phase.

## Dependency And Data Rules

The target graph is:

```text
mumble_modern_presentation -> mumble_modern_application -> mumble_modern_contracts
mumble_modern_testing -> mumble_modern_contracts
```

No Phase 1 target depends on `shared`, `mumble_client_object_lib`, generated
protobufs, or existing runtime headers. The existing `mumble` executable does
not link any new target in this phase. This guarantees that the legacy client
remains the only user-facing path.

## Error Handling And Testing

Strong IDs compare by value and default to an explicit invalid value. The
store's debug thread-affinity assertion converts an accidental cross-thread
mutation into a deterministic test failure. Application tests construct only
Qt Core objects and fake ports; they do not construct `Global`, a window, a
socket, or an audio device.

The CMake dependency check is tested by configuring a fixture containing a
deliberately forbidden include and requiring configuration failure. Existing
legacy CTest coverage remains unchanged.

## Non-Goals

Phase 1 does not add QML, instantiate `ModernClientBootstrap` in `main`, alter
`ServerHandler`, parse protocol messages, migrate settings or audio, or make
modern state authoritative. `ModernClientBootstrap` is declared and unit-tested
as a composition object only; it is not connected to the legacy executable.
