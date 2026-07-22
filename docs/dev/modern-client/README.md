# Modern Windows Client

Status: architecture accepted, implementation not started

This directory defines the architecture and migration plan for a modern Windows
Mumble client. The client will use Qt Quick for its primary user interface while
retaining the existing Mumble protocol, audio, plugin, overlay, database, and
Windows integration code.

The selected approach is an in-process application boundary. The new UI will not
wrap `MainWindow` or communicate with a separate backend process. Instead, both
the legacy Widgets UI and the new Qt Quick UI will consume the same application
services and state stores while migration is in progress.

## Documents

- [Architecture](architecture.md) defines layers, ownership, dependencies,
  threading, and the target source layout.
- [Application contracts](application-contracts.md) defines commands, events,
  snapshots, model roles, and state machines.
- [Migration plan](migration-plan.md) defines implementation phases, exit
  criteria, compatibility strategy, and verification.
- [Cross-platform alignment](cross-platform-alignment.md) defines what can be
  shared with independently implemented Apple clients.
- [ADR 0001](adr/0001-in-process-application-boundary.md) records why the
  in-process application boundary was selected.

## Scope

The first supported release target is Windows 10 and Windows 11. This affects UI
design, packaging, platform integration, and the test matrix. It does not justify
forking or deleting the existing cross-platform protocol and audio code.

The first modern-client beta includes:

- favourite, recent, and discovered servers;
- connection, cancellation, certificate, failure, and reconnect flows;
- channel and user navigation;
- talking, mute, deafen, suppress, recording, and priority-speaker states;
- channel and private text messages;
- local mute, deafen, push-to-talk, transmit mode, and per-user volume;
- global shortcuts, the system tray, and Windows notifications; and
- temporary access to legacy advanced dialogs.

The following are deliberately deferred from the first beta:

- replacing every administration and advanced settings dialog;
- redesigning the audio engine or wire protocol;
- multi-server sessions;
- loading third-party audio effects;
- a second frontend technology such as WinUI or a web view; and
- a separately deployed backend process.

## Architectural principles

1. The application state has one owner.
2. Views render state and submit user intent; they do not perform protocol or
   audio operations.
3. No UI-facing API exposes protobuf messages, legacy object pointers, or
   `Global`.
4. Network and audio threads never mutate UI state directly.
5. High-frequency state changes update only affected model roles.
6. Legacy dialogs are explicit migration bridges, not a hidden legacy main
   window.
7. Existing source files are moved only when separation requires it, reducing
   conflicts when synchronizing with upstream Mumble.

## Baseline observations

The decision was made against commit `9da721e64`. At that point:

- the client used Qt 6 Widgets and had no QML files;
- the client contained 39 Designer `.ui` forms;
- `MainWindow` parsed and handled protocol messages;
- `ServerHandler` posted incoming control messages directly to `MainWindow`;
- `UserModel` accessed the concrete `QTreeView` through `Global`; and
- 33 client source files contained 235 direct references to
  `Global::get().mw`.

These observations make a thin visual wrapper unsuitable as a long-term
architecture. They also make a big-bang rewrite unnecessarily risky. The plan in
this directory introduces and validates the application boundary incrementally.
