# Phase 2 Neutral Protocol Entry Design

## Scope

Phase 2 replaces the direct inbound control-message target `Global::get().mw`
with an adapter-owned, main-thread receiver while preserving the legacy
`MainWindow::msgX` handlers as the only state-mutating consumer. It does not
make application stores authoritative and it does not add a QML path.

## Boundary

`ControlMessageEnvelope` is an immutable adapter-boundary value containing the
numeric TCP control-message type, copied payload bytes, `ConnectionAttemptId`,
and a strictly increasing receive sequence. It deliberately has no protobuf
object or legacy pointer. `ServerHandler` assigns the envelope metadata on its
network thread and emits it through a queued Qt signal.

`ProtocolEventAdapter` is a main-thread QObject owned by an explicit legacy
composition object. It rejects envelopes whose attempt is no longer current,
rejects a sequence lower than the last accepted sequence for that attempt, and
parses only on the main thread. Parse failure emits a diagnostic value and
never reaches the legacy receiver.

The temporary `LegacyProtocolReceiver` is the only Phase 2 component allowed
to include both the adapter contract and `MainWindow.h`. It dispatches a valid
parsed message to the corresponding existing `msgX` handler. There is no
parallel application state mutation or outbound side effect from a shadow path.

## Runtime Flow

For non-audio, non-Ping TCP messages the flow becomes:

```text
ServerHandler worker -> immutable envelope signal -> queued main-thread adapter
                     -> protobuf parse -> LegacyProtocolReceiver -> MainWindow::msgX
```

`UDPTunnel` audio and the existing transport-local Ping bookkeeping remain in
`ServerHandler` in this phase. UDP availability transitions stop showing a
message box from `ServerHandler`; they are published as typed adapter transport
events for the legacy receiver to present until application ownership migrates.

`ServerHandler` owns the active attempt counter and increments it for each
connect request. Disconnect/cancel invalidates the active attempt before queued
late envelopes can be accepted. The adapter uses that attempt value rather than
pointer identity to discard stale work.

## Build And Test Boundaries

The envelope and main-thread ordering guard are tested with Qt Core only. The
adapter target is the only modern target that can include protobuf/runtime
headers; it is separate from `mumble_modern_application` and retains the Phase
1 dependency gate for contracts and application. Adapter tests cover order,
attempt cancellation, malformed payload rejection, and no legacy dispatch on
parse failure. An integration test verifies `ServerHandler` no longer posts a
control event to `MainWindow`.

## Completion Evidence

Phase 2 is not complete until the legacy client still builds and its smoke
behaviour is unchanged, incoming ordering is preserved, stale cancelled-attempt
messages are rejected, and malformed protobuf payloads are diagnosed without
state mutation. The feature remains legacy-only until a later phase assigns a
message family to application state.
