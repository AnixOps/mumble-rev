# Cross-platform Client Alignment

The modern Windows client and independently developed Apple clients should share
product and protocol contracts where useful. They should not attempt to share a
runtime that is tightly coupled to Qt/Win32 on one side or Objective-C,
AudioUnit, and Apple frameworks on the other.

## Shared sources of truth

### Wire protocol

The canonical `.proto` files come from the official Mumble repository. Each
client generates language-specific bindings from the same pinned protocol
revision. A client may support only a subset, but the feature matrix must record
that subset explicitly.

Client repositories must not independently edit generated protocol files as if
they were the protocol source. An automated check should report drift from the
pinned official schema, including missing messages and fields.

### Behaviour specifications

The projects should align on observable behaviour for:

- connection phases, cancellation, reconnect, and errors;
- certificate and password challenges;
- server synchronization;
- mute, deafen, suppress, transmit mode, and talk state;
- channel membership, links, listeners, and permissions;
- channel and private messages;
- pending and failed local messages;
- audio preview and device reconfiguration; and
- network quality terminology.

Implementations and UI components may differ as long as the observable semantics
remain equivalent.

### Test fixtures

Share platform-neutral fixtures rather than runtime objects:

- serialized protocol messages with expected semantic events;
- scripted test-server scenarios;
- channel and user state snapshots;
- reconnect and stale-event sequences;
- malformed rich text and attachment cases;
- certificate challenge cases; and
- audio-state transition sequences without PCM payloads.

Fixtures must contain no production credentials, private keys, personal server
addresses, or recorded voice data.

### Product language and design tokens

The clients may share terminology, localization keys, brand assets, semantic
colour names, status icons, and spacing scales. Platform controls and navigation
remain native to each UI toolkit.

For example, both clients may expose a semantic `voice.talking` colour and the
same meaning for "server muted" while rendering the state with different native
components.

## Platform-owned implementation

The following remain platform-specific:

- Qt Quick and SwiftUI view code;
- WASAPI and AudioUnit device lifecycle;
- system tray, taskbar, menu bar, Live Activity, and Handoff;
- global shortcuts and platform permissions;
- certificate storage and OS credential integration;
- local database implementations; and
- packaging, signing, update, and crash-reporting integration.

## Mapping between client architectures

The projects can use equivalent conceptual layers without sharing source:

| Apple client concept | Modern Windows equivalent |
| --- | --- |
| `MKConnection` | connection and protocol legacy adapters |
| `MKServerModel` | session adapter plus application stores |
| `MKAudio` | audio adapter over existing Mumble audio and WASAPI |
| `ServerModelManager` | split application services and stores |
| SwiftUI observable state | QObject view models and Qt models |
| SwiftUI views | QML views |

The Windows design intentionally splits connection, session, chat, talk, and
voice state instead of introducing one application-wide observable manager.

## Feature matrix

Maintain a machine-readable feature matrix once implementation starts. Each row
should include:

```text
feature ID
protocol dependency
minimum server version, if any
Windows support state
Apple support state
fixture or test reference
user-visible fallback
```

Support states should be a controlled vocabulary such as `unsupported`,
`planned`, `partial`, `complete`, and `verified`.

## Potential future shared code

A DSP algorithm may become shared code only when it:

- has no AudioUnit, WASAPI, Qt, Objective-C, or Swift dependency;
- has a stable C or C++ ABI boundary;
- has deterministic input/output tests;
- documents realtime allocation and locking behaviour; and
- can be built and fuzzed independently on both platforms.

Per-user gain, pan, metering, and routing semantics are good candidates for a
shared specification before they are candidates for shared implementation.

## Coordination workflow

Cross-platform behaviour changes should follow this order:

1. update the behaviour specification or feature matrix;
2. add or update a platform-neutral fixture;
3. implement independently on each platform;
4. run platform tests and compare semantic output; and
5. record intentional platform differences.

This workflow allows the two clients to converge on user expectations without
forcing either project into the other's platform architecture.
