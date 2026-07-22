# ADR 0001: Use an in-process application boundary

- Status: Accepted
- Date: 2026-07-22
- Decision owners: mumble-rev maintainers

## Context

The existing Mumble client is a Qt 6 Widgets application whose primary UI class
also coordinates protocol messages and a large part of client behaviour.
`ServerHandler` posts incoming control messages directly to `MainWindow`,
protocol handlers are implemented as `MainWindow::msgX`, and models and platform
components frequently reach the concrete window through `Global`.

At the time of this decision, 33 client source files contained 235 direct
references to `Global::get().mw`. `UserModel` also reached the concrete
`QTreeView`. Adding QML directly over these objects would create a new visual
surface without creating a reliable application boundary.

The project needs a modern Windows UI while preserving the proven protocol,
audio, plugin, overlay, shortcut, and database implementation. It should also be
possible to synchronize important fixes from upstream Mumble.

## Decision

Introduce an in-process application layer between the existing runtime and all
user interfaces.

The application layer owns UI-facing connection, session, talk, chat, and voice
state. It exposes typed commands, events, snapshots, and ports. It may depend on
Qt Core, but it does not depend on Qt Widgets, Qt Quick, Win32, protobuf, or
legacy runtime classes.

Legacy adapters translate between application contracts and the existing
runtime. The primary new presentation is Qt Quick/QML. During migration, the
legacy Widgets UI may consume the same application state, and selected advanced
dialogs may be opened through an explicit bridge.

The core remains in the same process as the UI. The modern path does not create
or depend on a hidden legacy `MainWindow`.

## Dependency direction

```text
QML -> presentation -> application contracts
                                ^
                                |
existing runtime <- adapters ---+
```

Only adapters may include both application contracts and legacy runtime
headers. Presentation cannot cross that boundary.

## Consequences

Positive consequences:

- existing realtime and protocol code is reused;
- UI behaviour can be tested without a server or audio device;
- the modern UI is not tied to the lifetime or widget tree of `MainWindow`;
- legacy features can migrate incrementally;
- high-frequency state can be modelled independently from broad UI state; and
- most new code can be added without moving upstream-owned files.

Negative consequences:

- useful visual work starts after an initial extraction phase;
- two UI implementations exist temporarily;
- adapters add mapping code and application snapshots duplicate selected runtime
  data;
- protocol handlers that mix UI and state must be separated carefully; and
- legacy plugin, overlay, and platform assumptions about `MainWindow` require
  narrow replacement interfaces.

## Alternatives considered

### Thin QML facade over the legacy UI

A facade could expose `MainWindow`, `UserModel`, and `Global` state to QML. This
would produce a prototype sooner, but it would retain hidden Widgets lifecycle,
duplicate state, make tests depend on the full application, and require a second
refactor for a maintainable product.

Rejected as the production architecture. It remains acceptable only for a
throwaway visual experiment that is not merged into the product path.

### Separate core process with IPC

A backend process would provide a strict boundary and could support unrelated
frontend technologies. It would also require versioned IPC, secure local
transport, process supervision, state replay, and careful ownership of audio,
global shortcuts, tray, overlay, and plugin integration.

Rejected for the current Windows-only Qt Quick goal because the operational and
testing cost is not justified.

### Full WinUI rewrite

A WinUI frontend would still need a boundary to the existing Qt-heavy runtime or
a much larger core rewrite. It would increase platform integration quality in
some areas but greatly increase divergence from upstream and does not solve the
application-state problem by itself.

Rejected for the current implementation. The application contracts should still
avoid unnecessary presentation assumptions so this choice can be revisited.

## Revisit conditions

Reconsider the process boundary only if one or more of these become committed
requirements:

- independently restartable UI and audio processes;
- multiple simultaneously supported frontend technologies;
- a stable public local-client API;
- security isolation for third-party audio or UI extensions; or
- a non-Qt product direction that justifies the deployment cost.

Changing the application layer to exclude Qt Core is also a separate decision.
It should be considered only when a concrete second in-process frontend needs to
consume it.

## Follow-up decisions

Separate ADRs are required for:

- the final Qt Quick component and styling system;
- settings migration and compatibility policy;
- chat-history persistence and retention;
- third-party audio effect hosting;
- removal of the legacy UI; and
- any irreversible database or configuration migration.
