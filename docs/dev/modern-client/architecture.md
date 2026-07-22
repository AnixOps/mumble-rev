# Modern Client Architecture

## Goals

The architecture must allow a modern Qt Quick interface to use the existing
Mumble implementation without inheriting the current coupling between
`MainWindow`, `Global`, protocol handling, and concrete Widgets.

It must:

- preserve existing audio, protocol, plugin, overlay, shortcut, and database
  behaviour;
- support incremental migration with a working client at each milestone;
- allow legacy Widgets dialogs to remain available temporarily;
- expose deterministic, testable application state to the presentation layer;
- keep realtime work off the UI thread;
- make high-frequency talking and level updates inexpensive; and
- minimize unnecessary changes to upstream-owned source files.

## Non-goals

This architecture does not create a portable runtime shared with an Apple
client. It does not remove Qt from the Mumble core, add multi-server support, or
host the core in another process. It also does not prescribe final colours,
spacing, animation, or navigation details.

## Layer model

Dependencies point down the diagram. A lower layer must not include or call a
higher layer.

```text
+--------------------------------------------------------------+
| Qt Quick UI                                                  |
| QML views, controls, layout, animation, ephemeral UI state   |
+-------------------------------+------------------------------+
                                |
                                v
+--------------------------------------------------------------+
| Presentation                                                 |
| QObject view models and QAbstractItemModel projections       |
+-------------------------------+------------------------------+
                                |
                                v
+--------------------------------------------------------------+
| Application                                                  |
| services, stores, typed commands, events, and snapshots      |
+-------------------------------+------------------------------+
                                ^
                                | ports implemented by adapters
                                v
+--------------------------------------------------------------+
| Legacy adapters                                              |
| protocol mapping, ServerHandler, audio, settings, database   |
+-------------------------------+------------------------------+
                                |
                                v
+--------------------------------------------------------------+
| Existing Mumble runtime                                      |
| transport, protobuf, audio threads, plugins, overlay, WASAPI |
+--------------------------------------------------------------+
```

Windows platform services and the legacy-dialog bridge sit beside the legacy
adapters. They implement application or presentation ports but may depend on
Win32 and Qt Widgets respectively.

## Dependency rules

### Qt Quick UI

QML may import only registered presentation types and approved reusable visual
components. QML must not:

- access `Global`, `MainWindow`, `ServerHandler`, `Settings`, or the database;
- receive `ClientUser *`, `Channel *`, protobuf objects, or `QModelIndex` as
  durable identity;
- open sockets, read configuration files, or start audio devices;
- contain protocol permission calculations; or
- rebuild or sort large domain collections in JavaScript.

Window selection, focus, panel visibility, scroll position, animation progress,
and transient editing text are presentation concerns and may remain in QML.

### Presentation

Presentation converts application snapshots into properties and Qt models. It
may depend on Qt Core, Qt QML, and Qt Quick. It must not depend on Qt Widgets or
legacy runtime headers.

Presentation objects expose explicit commands such as `connectToServer`,
`joinChannel`, and `setSelfMuted`. They do not expose a generic "execute" method
or pass untyped `QVariantMap` payloads into the application layer.

### Application

Application owns client-visible state and use-case coordination. It may use Qt
Core value types and signals, but it must not depend on Qt Widgets, Qt Quick,
Win32, protobuf-generated headers, or concrete legacy runtime classes.

Application receives typed events from adapters, updates stores on the main
thread, and invokes outbound ports for side effects. It contains no rendering or
dialog code.

### Legacy adapters

Adapters are the only modern-client code allowed to include existing runtime
headers such as `ServerHandler.h`, `ClientUser.h`, `UserModel.h`, `Audio.h`, or
`Global.h`.

Adapters translate in both directions:

- incoming protobuf and legacy callbacks become typed application events; and
- application commands become existing runtime method calls and protobuf
  messages.

Adapters do not own user-facing state. Cached values are permitted only for
thread handoff, deduplication, or mapping a legacy object to a stable application
identity.

## Target source layout

New files should be added beside the existing client rather than moving large
parts of the current source tree at the start of the migration.

```text
src/mumble/modern/
  CMakeLists.txt
  ModernClientBootstrap.cpp
  ModernClientBootstrap.h
  application/
    ClientApplication.cpp
    ClientApplication.h
    contracts/
      ClientCommand.h
      ClientError.h
      ClientEvent.h
      ClientSnapshot.h
      ClientTypes.h
      ports/
    services/
      ConnectionService.cpp
      SessionService.cpp
      ChatService.cpp
      VoiceService.cpp
      SettingsService.cpp
    stores/
      ConnectionStore.cpp
      SessionStore.cpp
      TalkStateStore.cpp
      ChatStore.cpp
      VoiceStore.cpp
  adapters/
    legacy/
      ConnectionAdapter.cpp
      ProtocolEventAdapter.cpp
      SessionCommandAdapter.cpp
      AudioAdapter.cpp
      SettingsAdapter.cpp
      DatabaseAdapter.cpp
      PluginAdapter.cpp
    widgets/
      LegacyDialogBridge.cpp
  presentation/
    models/
      ServerListModel.cpp
      ChannelTreeModel.cpp
      ChannelMemberModel.cpp
      ChatListModel.cpp
    viewmodels/
      ApplicationViewModel.cpp
      ConnectionViewModel.cpp
      VoiceViewModel.cpp
      SessionViewModel.cpp
  platform/windows/
    WindowsPlatformService.cpp
    WindowHost.cpp
  qml/
    Mumble/
      Main.qml
      views/
      components/
      style/
```

Exact file boundaries may change when implementation reveals a smaller useful
unit. The layer ownership and dependency direction may not change without an
architecture decision record.

## Build boundaries

The intended CMake dependency graph is:

```text
mumble_modern_presentation -> mumble_modern_application
mumble_modern_application  -> mumble_modern_contracts
mumble_modern_adapters      -> mumble_modern_contracts + existing runtime
mumble_modern_windows       -> mumble_modern_contracts + Qt Gui + Win32
mumble_modern_legacy_dialogs -> mumble_modern_application + Qt Widgets
mumble executable           -> all selected components
```

During migration, CMake should support two explicit options:

- `modern-ui` builds the new Qt Quick shell; and
- `legacy-ui` builds the existing Widgets main window.

At least one must be enabled. Initially `legacy-ui` remains the default. The
default changes only after the release gates in the migration plan pass.

The application and contracts targets must have an automated dependency check
that fails if they link Qt Widgets or Qt Quick. A lightweight include check may
also reject `Global.h`, protobuf headers, and legacy UI headers in those targets.

## Runtime composition and ownership

`ModernClientBootstrap` is the executable-level composition root for the modern
path. It creates adapters, the application aggregate, presentation models, and
platform services in a deterministic order. `ClientApplication` owns the
application services and stores and receives its outbound ports through
construction. The application target therefore does not depend on concrete
adapters.

Ownership rules are:

- existing runtime objects remain owned according to their current lifecycle;
- the composition root owns adapters for the lifetime of `ClientApplication`;
- adapters hold non-owning handles or scoped connections to runtime objects;
- stores own application snapshots and indexes;
- view models hold references to services and stores but do not own runtime
  objects;
- QML owns visual objects and no business services; and
- no modern object stores a durable pointer to `ClientUser` or `Channel`.

Session-scoped IDs are paired with a `ConnectionEpoch`. The epoch changes when a
server session is discarded. Commands with a stale epoch are rejected before
they can act on a session ID that has been reused by a later connection.

## Inbound protocol flow

The current transport posts `ServerHandlerMessageEvent` directly to
`MainWindow`, which parses the protobuf and invokes `msgX` handlers. The target
flow is:

```text
ServerHandler thread
  -> immutable ControlMessageEnvelope
  -> queued delivery to ProtocolEventAdapter on the main thread
  -> protobuf parsing and legacy-to-application mapping
  -> typed ClientEvent
  -> application service
  -> store transaction
  -> store change signal
  -> presentation model update
  -> QML binding update
```

There is one authoritative consumer for each protocol message category. Shadow
consumers may be used temporarily for comparison tests, but they must not cause
side effects or become a second source of truth.

## Outbound command flow

```text
QML user action
  -> explicit view-model method
  -> application service validation
  -> typed command through an outbound port
  -> legacy adapter
  -> ServerHandler, audio, settings, or database operation
  -> resulting event updates the authoritative store
```

The UI does not assume success merely because a command was accepted. Immediate
local controls may use an explicit pending value, but authoritative state is
confirmed by an event or a successful adapter result. Chat messages use a local
client message ID so pending, sent, and failed states can be reconciled.

## State ownership

### ConnectionStore

Owns the connection epoch, connection phase, target summary, reconnect state,
certificate challenge, last structured error, server synchronization state, and
transport health snapshot.

It does not own favourites, passwords, channel data, or modal presentation.

### SessionStore

Owns channel and user snapshots, membership, local-session identity, effective
permissions, links, listeners, comments, and session metadata.

It does not own tree expansion, current keyboard focus, or transient menus.

### TalkStateStore

Owns the high-frequency speaking state for each user. It is separate from the
larger session store so a voice packet does not trigger broad channel-tree
recomputation.

### ChatStore

Owns bounded message history for the active session, delivery state, private
conversation identity, unread counts, and message correlation IDs.

It does not own scroll position, bubble grouping, link hover, or image-preview
geometry.

### VoiceStore

Owns local mute, deafen, suppression, transmit mode, active voice target, input
activity summary, selected devices, and audio lifecycle phase.

It does not own PCM buffers or audio-thread objects.

## Threading model

The application main thread owns all stores, services, view models, and Qt
models. Store methods assert thread affinity in debug builds.

`ServerHandler`, `AudioInput`, and `AudioOutput` retain their existing worker
threads. Cross-thread rules are:

- transport messages cross the boundary as immutable byte envelopes;
- protobuf parsing and store mutation occur on the main thread;
- application commands use existing thread-safe or queued runtime entry points;
- audio callbacks never call a view model or allocate UI objects;
- meters and speech probability are published through atomic snapshots and
  sampled by the main thread no faster than 20 Hz; and
- no main-thread code waits synchronously for network, audio, DNS, image decode,
  or plugin work.

Database access follows the existing requirement that `Global::db` be used only
from the main event loop until a dedicated database boundary is implemented.

## Model projection rules

Qt models are projections over stores, not stores themselves. Multiple
projections may represent the same canonical data, for example a full channel
tree and a current-channel member list.

Models must:

- use `beginInsertRows`, `beginRemoveRows`, and `beginMoveRows` for structural
  changes;
- emit `dataChanged` only for affected rows and roles;
- avoid `beginResetModel` for normal talk, mute, unread, or avatar changes;
- provide stable role names and entity keys;
- perform sorting and filtering in C++, not QML JavaScript; and
- preserve selection by entity key across legal structural updates.

Talk state and audio meters must not rebuild the channel tree. Large avatars,
rich text, and image thumbnails are decoded outside the render-critical path and
published only when ready.

## Legacy Widgets boundary

The modern client may open selected existing dialogs through
`LegacyDialogBridge`. The bridge is the only modern component allowed to expose
`QWidget *` or use a widget parent.

The bridge must:

- obtain its parent through `WindowHost`;
- translate dialog results into application service calls or settings refreshes;
- prevent a legacy dialog from becoming an alternative state owner; and
- make the invoked legacy feature explicit for telemetry and test coverage.

The modern client must not instantiate a hidden legacy `MainWindow`. Code that
currently uses `Global::get().mw` is migrated according to intent:

- window parenting and native handles move to `WindowHost`;
- user-facing messages move to an application event and notification presenter;
- session commands move to application services;
- model-to-view calls are removed; and
- truly legacy-only behavior remains compiled only with `legacy-ui`.

## Windows platform boundary

Windows-specific code owns system tray integration, taskbar behaviour, native
notifications, global shortcut integration, startup registration, and native
window handles. It receives application state and emits typed user intent.

Overlay and plugin behaviour remain in the existing runtime for the first
release. Their assumptions about `MainWindow` must be replaced with narrow
interfaces such as `WindowHost`, `SessionQuery`, or `NotificationSink`, not with
a new global UI singleton.

## Error and interaction policy

Expected interactions are state, not imperative dialogs. Certificate decisions,
password requests, reconnect progress, permission denial, and connection failure
are represented by typed application state. Presentation chooses a suitable
Windows UI surface.

Unexpected runtime failures become a `ClientError` with a stable code, severity,
retryability, redacted context, and localization arguments. Passwords, access
tokens, certificate private keys, and raw audio are never placed in logs or
long-lived snapshots.

## Performance budgets

Initial budgets are engineering gates rather than product promises:

- no full model reset for a talk-state-only update;
- UI-facing meter updates at or below 20 Hz;
- no blocking operation longer than one frame on common navigation paths;
- bounded in-memory chat history with an explicit paging policy;
- reconnect and initial synchronization publish progress before optional avatar
  or blob loading; and
- all long-running image, rich-text, and discovery work is cancellable or
  discardable by connection epoch.

Concrete timing and memory thresholds will be recorded from the baseline Windows
build during migration phase 0.

## Testing boundaries

Application services are tested against fake ports without creating a window,
socket, audio device, or `Global`. Adapter tests verify mapping between legacy
types and application events. Presentation tests verify model roles and
incremental changes. End-to-end tests cover the composition root and a real test
server.

QML tests cover focus, keyboard navigation, accessibility properties, compact
and wide layouts, and light and dark themes. Windows release checks cover at
least 100%, 150%, and 200% display scaling.
