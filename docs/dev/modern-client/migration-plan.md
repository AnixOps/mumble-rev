# Modern Client Migration Plan

## Strategy

The migration is incremental and keeps a runnable client at the end of every
phase. New presentation code must not become authoritative before its underlying
application contract has tests and an explicit owner.

The plan avoids two unsafe shortcuts:

- a hidden legacy `MainWindow` must not power the modern UI; and
- protocol messages must not be handled independently by two state-mutating
  implementations.

Time estimates assume one developer familiar with C++ and Qt. They are planning
ranges, not release commitments.

## Release tracks

Three build or runtime tracks are used during migration:

| Track | Purpose |
| --- | --- |
| Legacy | Existing Widgets client and the behavioural reference |
| Modern shadow | New adapters and stores consume events but cannot cause side effects |
| Modern active | Qt Quick UI and application services are authoritative for migrated features |

Shadow mode exists only for state comparison and diagnostics. It must never send
protocol messages, change settings, start audio, or display user-facing
notifications.

## Phase 0: Reproducible baseline

Estimated duration: 1 week

Deliverables:

- a documented Windows debug and release build using the pinned dependency set;
- a smoke-test checklist for connection, audio, chat, shortcuts, tray, overlay,
  plugins, reconnect, and shutdown;
- baseline startup, memory, synchronization, and model-update measurements;
- a small local or controlled Mumble test-server configuration; and
- captured logs for a normal session and representative failure paths.

Exit criteria:

- the unmodified legacy client builds and passes the smoke test on the supported
  Windows versions;
- test credentials and private certificates are not committed; and
- performance measurements are repeatable enough to detect a major regression.

## Phase 1: Boundary skeleton

Estimated duration: 1-2 weeks

Deliverables:

- `src/mumble/modern` CMake targets;
- contract value types, ports, and fake adapters;
- `ModernClientBootstrap` composition root and `ClientApplication` aggregate;
- empty stores with main-thread assertions;
- a minimal presentation test executable; and
- dependency checks that keep application code free of Widgets, Quick,
  protobuf, and legacy runtime headers.

No production behaviour changes in this phase. The legacy client remains the
only user-facing track.

Exit criteria:

- application tests run without constructing `Global`, a window, a socket, or an
  audio device;
- forbidden dependency checks fail on a deliberate test violation; and
- legacy build and smoke tests remain unchanged.

## Phase 2: Neutral protocol entry point

Estimated duration: 3-5 weeks

The first structural change removes `MainWindow` as the transport event target.

Deliverables:

- an immutable `ControlMessageEnvelope` carrying message type, bytes,
  connection attempt, and ordering metadata;
- queued delivery from `ServerHandler` to a main-thread protocol receiver;
- `ProtocolEventAdapter` parsing and typed-event mapping;
- a temporary legacy receiver that invokes existing handlers in the legacy
  track;
- structured transport degraded/restored events instead of message boxes from
  `ServerHandler`; and
- shadow comparisons for connection and synchronization state.

The temporary legacy receiver is allowed to reference `MainWindow` only in a
legacy build. It is not linked into a modern-only build.

Exit criteria:

- `ServerHandler` no longer posts events to or displays messages through
  `Global::get().mw`;
- incoming messages retain ordering across the thread boundary;
- cancelling a connection rejects late events from that attempt; and
- malformed protobuf messages are diagnosed and ignored without corrupting
  application state.

## Phase 3: Authoritative application state

Estimated duration: 4-6 weeks

Message categories migrate one at a time. For each category, runtime mutation,
application projection, user-visible effects, and tests are separated before the
next category starts.

Recommended order:

1. connection, server sync, server config, rejection, and permission denial;
2. channel creation, update, movement, and removal;
3. user creation, update, movement, removal, and local identity;
4. talk, mute, deafen, suppress, priority, and recording state;
5. channel and private text messages; and
6. ACL, ban, user list, context actions, and plugin data.

Deliverables:

- `ConnectionStore`, `SessionStore`, `TalkStateStore`, `ChatStore`, and
  `VoiceStore`;
- application services and outbound legacy adapters;
- extraction of protocol-state mutation from `MainWindow::msgX` handlers;
- legacy presentation observing the same authoritative state where practical;
- stale-epoch and event-order tests; and
- state snapshot comparison against the legacy reference track.

Existing `ClientUser`, `Channel`, and audio-related runtime objects remain
available to the protocol, audio, plugin, and overlay implementation. Adapters
are responsible for projecting them into pointer-free application snapshots.

Exit criteria:

- migrated message categories have a single state-mutating path;
- application snapshots contain no legacy pointers or protobuf objects;
- reconnect discards all previous-epoch users, channels, pending commands, and
  optional loads;
- talk-state changes do not reset the channel model; and
- legacy smoke tests pass with the extracted handlers.

## Phase 4: Qt Quick shell

Estimated duration: 5-7 weeks

Deliverables:

- Qt Quick application shell and design-token foundation;
- welcome view with favourites, recent targets, and LAN discovery;
- connection and certificate challenge surfaces;
- channel tree, current-channel member projection, and user actions;
- channel and private chat with pending and failed states;
- local voice controls and connection status; and
- keyboard navigation, focus order, and initial accessibility metadata.

The first implementation favours stable desktop information density over broad
animation. Large visual transitions are added only after the state and focus
models are reliable.

Exit criteria:

- a user can connect, navigate channels, communicate, control local voice, and
  disconnect without creating a legacy `MainWindow`;
- connection cancellation and certificate decisions are fully represented by
  application state;
- selection survives incremental channel-tree updates; and
- QML contains no direct reference to a legacy runtime object.

## Phase 5: Audio and Windows integration

Estimated duration: 2-4 weeks

Deliverables:

- audio activity leases for connected sessions and previews;
- sampled input level and VAD state at a bounded UI update rate;
- global push-to-talk and other migrated shortcuts;
- tray menu and window visibility behaviour;
- native notifications and taskbar integration;
- microphone privacy and device-reconfiguration error flows; and
- verification that overlay and plugin behaviour remain intact.

Exit criteria:

- opening the welcome or ordinary settings view does not start microphone
  capture;
- push-to-talk press and release cannot remain stuck after focus or device
  changes;
- no audio callback calls presentation code or waits on the main thread; and
- tray and shortcut actions invoke the same application services as QML.

## Phase 6: Legacy bridge and beta release

Estimated duration: 2-4 weeks

Deliverables:

- explicit `LegacyDialogBridge` entries for deferred advanced features;
- a feature-parity matrix with known limitations;
- modern UI as an opt-in build or launch mode;
- Windows packaging, upgrade, and rollback checks;
- crash, diagnostic, and privacy review; and
- beta release notes and a feedback template.

Initial bridge candidates are advanced audio configuration, ACL, ban list,
certificate management, plugins, overlay configuration, and recording tools.

Exit criteria:

- each legacy bridge has an owner, diagnostic identifier, and replacement status;
- closing a legacy dialog refreshes authoritative settings through a service;
- no bridge requires a hidden legacy main window;
- clean install, in-place upgrade, and legacy-mode rollback preserve user data;
  and
- the complete Windows quality matrix passes.

## Default-switch gate

The modern UI becomes the default only when all of the following are true:

- primary connection, channel, chat, and voice workflows meet feature parity;
- no severity-one regression remains in audio, reconnect, shortcuts, tray,
  plugin, or overlay behaviour;
- accessibility and keyboard-only workflows pass their test plan;
- startup and normal-session memory remain within the budgets established in
  phase 0 or have an accepted exception;
- crash-free beta evidence is sufficient for the intended release channel; and
- legacy mode remains available for at least one transition release.

Removing the legacy UI is a separate decision and is not implied by making the
modern UI the default.

## Verification matrix

Required Windows coverage:

- Windows 10 and Windows 11;
- 100%, 150%, and 200% display scaling;
- light, dark, and system theme changes while running;
- keyboard-only navigation and a supported screen reader;
- default, changed, removed, and privacy-blocked audio devices;
- IPv4, IPv6, UDP, TCP tunnel, packet loss, and reconnect scenarios;
- certificate-valid, self-signed, changed-certificate, and rejection flows;
- empty, small, and large channel trees with rapid talk-state updates; and
- clean install, upgrade, automatic start, tray-only, and shutdown paths.

Test layers:

| Layer | Required evidence |
| --- | --- |
| Contracts | value, epoch, transition, and redaction tests |
| Application | services tested with fake ports |
| Adapters | protobuf and legacy mapping fixtures |
| Presentation | model role and incremental-update tests |
| QML | interaction, focus, accessibility, and screenshot tests |
| End to end | real client against controlled Mumble servers |

## Risk register

| Risk | Mitigation |
| --- | --- |
| Message handlers mix state and UI effects | Extract one message family at a time and compare snapshots |
| Plugins or overlay assume `MainWindow` exists | Introduce narrow window, session-query, and notification interfaces |
| Two UI tracks diverge | Keep one application owner and prohibit side effects in shadow mode |
| Talk state overloads QML | Separate talk store, granular roles, and bounded visual updates |
| Audio device changes race UI commands | Use leases, explicit phases, queued control, and stale-operation IDs |
| Upstream synchronization becomes expensive | Add new files beside existing code and avoid bulk moves or renames |
| Legacy dialogs mutate settings directly | Refresh through `SettingsService` and track each bridge for replacement |
| Rich text renders unsafe content | Sanitize at the boundary and test malformed input |

## Rollback policy

Before the default switch, failure of the modern track falls back only through
an explicit restart in legacy mode. The process must not create a hidden legacy
window and continue after partially initializing the modern application.

Database and settings migrations must be backward-readable for the transition
release. An irreversible migration requires its own ADR, backup procedure, and
recovery test.
