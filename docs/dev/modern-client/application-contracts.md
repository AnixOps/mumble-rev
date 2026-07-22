# Modern Client Application Contracts

This document defines the semantic contract between the existing Mumble runtime,
the modern application layer, and presentation. Names are drafts, but their
ownership and direction are normative.

## Contract design rules

- Contracts are explicit C++ value types.
- Qt Core types are allowed; Qt Widgets, Qt Quick, protobuf, and Win32 types are
  not.
- A snapshot contains no owning or non-owning legacy object pointer.
- Every session-scoped command carries a `ConnectionEpoch`.
- Unknown enum values are handled without terminating the client.
- Errors use stable machine-readable codes and localization arguments.
- Credentials are passed directly to the responsible port and are not retained
  in general-purpose stores.

## Identity types

The initial implementation should introduce strong wrappers rather than passing
plain integers through every layer:

```cpp
struct ConnectionAttemptId {
	quint64 value;
};

struct ConnectionEpoch {
	quint64 value;
};

struct ChannelId {
	quint32 value;
};

struct UserSessionId {
	quint32 value;
};

struct ServerBookmarkId {
	QUuid value;
};

struct ClientMessageId {
	QUuid value;
};

struct CertificateChallengeId {
	QUuid value;
};

struct AudioLeaseId {
	quint64 value;
};
```

An `EntityKey` combines an epoch with a channel or user ID. Equality across
different epochs is always false, even when the numeric protocol ID is equal.
A `ConversationKey` combines an epoch with a channel, user, or server-log
destination. It cannot be constructed from a display name.

## Snapshots

Snapshots are immutable from a consumer's perspective. Stores may replace a
snapshot or publish a delta, but consumers never mutate stored data directly.

### ConnectionSnapshot

Required fields:

- connection attempt ID;
- optional connection epoch, present after a session is synchronized;
- connection phase;
- redacted target name and address;
- current reconnect attempt and maximum attempts;
- whether cancellation is allowed;
- server synchronization progress;
- active certificate challenge ID, if any;
- last structured error, if any; and
- current transport health.

`TransportHealth` contains transport mode, local and remote UDP availability,
TCP and UDP latency, variance, packet loss counters, and last update time. An
unavailable metric is represented explicitly rather than as zero.

### ChannelSnapshot

Required fields:

- `EntityKey` and parent channel key;
- display name, position, user count, and maximum users;
- temporary, linked, filtered, listening, and current-channel flags;
- enter restriction and effective enter permission;
- description availability and comment hash; and
- effective permissions relevant to available actions.

### UserSnapshot

Required fields:

- `EntityKey`, channel key, and display name;
- local nickname and friend name;
- self, authenticated, priority-speaker, recording, and listener flags;
- server mute, self mute, local mute, suppress, server deaf, and self deaf;
- local volume adjustment;
- comment and avatar availability; and
- permissions or capabilities needed to build user actions.

Talk state is referenced by user key but stored separately in
`TalkStateStore`.

### ChatMessageSnapshot

Required fields:

- local client message ID;
- optional server or transport correlation data;
- connection epoch and conversation key;
- sender identity and display name;
- channel, tree, private, or system-message kind;
- sanitized rich-text payload and attachment descriptors;
- local creation and confirmed timestamps;
- pending, sent, or failed delivery state; and
- structured failure reason when delivery failed.

Raw untrusted HTML must not be rendered before the existing sanitization rules
have been applied at the adapter or application boundary.

### VoiceSnapshot

Required fields:

- local mute and deafen state;
- server mute, suppress, and recording availability;
- transmit mode and active voice target;
- push-to-talk pressed and transmitting state;
- VAD activity and sampled input level;
- audio lifecycle phase;
- selected input and output device summaries; and
- a structured audio error, if any.

## Commands and outbound ports

Application services expose use cases. Ports hide the existing implementation.
Representative contracts are:

```cpp
class ConnectionPort {
public:
	virtual ~ConnectionPort() = default;
	virtual void connectTo(const ConnectionRequest &request) = 0;
	virtual void cancel(ConnectionAttemptId attempt) = 0;
	virtual void disconnect(ConnectionEpoch epoch) = 0;
	virtual void acceptCertificate(CertificateChallengeId challenge) = 0;
	virtual void rejectCertificate(CertificateChallengeId challenge) = 0;
};

class SessionCommandPort {
public:
	virtual ~SessionCommandPort() = default;
	virtual void joinChannel(ConnectionEpoch epoch, ChannelId channel) = 0;
	virtual void setSelfMuteDeaf(ConnectionEpoch epoch, bool mute, bool deaf) = 0;
	virtual void setUserVolume(ConnectionEpoch epoch, UserSessionId user, float value) = 0;
	virtual void sendChannelMessage(const SendChannelMessage &command) = 0;
	virtual void sendPrivateMessage(const SendPrivateMessage &command) = 0;
};

class AudioControlPort {
public:
	virtual ~AudioControlPort() = default;
	virtual AudioLeaseId acquire(AudioUse use) = 0;
	virtual void release(AudioLeaseId lease) = 0;
	virtual void setTransmitMode(TransmitMode mode) = 0;
	virtual void setPushToTalk(bool pressed) = 0;
	virtual AudioMeterSnapshot readMeters() const = 0;
};
```

The final APIs may split command ports by feature. They must remain explicit and
must preserve epoch validation.

## Incoming events

`ProtocolEventAdapter` and other runtime adapters emit typed events into the
application layer. Event families include:

- connection phase and failure events;
- certificate and authentication challenges;
- server synchronization and configuration events;
- channel upsert, removal, move, and link events;
- user upsert, removal, move, and state events;
- talk-state events;
- channel, private, and system-message events;
- permission, ACL, ban, and context-action events;
- transport health events;
- audio lifecycle and device events; and
- plugin or overlay capability events.

Every event contains a connection attempt ID and a monotonically increasing
sequence within that attempt. Events that address synchronized session entities
also contain a connection epoch. A store ignores stale attempts or epochs and
detects a sequence gap. A gap triggers diagnostics and, where correctness
requires it, an explicit state resynchronization rather than silent
continuation.

Events are values, not a general QObject event bus exposed to the whole process.
Only `ClientApplication` wires event producers to the services that consume
them.

## Connection state machine

The UI renders this state machine; it does not infer connection state from
button availability or the existence of a socket.

```text
Idle
  -> Resolving
  -> ConnectingTransport
  -> NegotiatingTls
  -> Authenticating
  -> Synchronizing
  -> Connected

NegotiatingTls -> AwaitingCertificateDecision -> NegotiatingTls

Any active phase -> Disconnecting -> Idle
Any pre-connected phase -> Failed
Connected -> Reconnecting -> Resolving
Reconnecting -> Failed when policy is exhausted
```

Rules:

- user cancellation is accepted in every phase except `Idle`;
- a connection attempt has an ID before DNS starts;
- cancellation and a late success event are reconciled by attempt ID;
- UI animations never delay a state transition;
- channel and user state is not exposed as current until synchronization
  completes; and
- a new synchronized session receives a new connection epoch.

## Audio lifecycle

Audio devices are controlled by leases. Opening an arbitrary settings or welcome
view does not start microphone capture.

Valid `AudioUse` values initially are:

- `ConnectedSession`;
- `InputPreview`;
- `AudioWizard`; and
- `MixerPreview`.

The audio engine is required while at least one valid lease exists. A lease is
released when its owning operation ends, is cancelled, or changes epoch.

```text
Stopped -> Starting -> Running
Running -> Reconfiguring -> Running
Starting | Running | Reconfiguring -> Failed
Failed -> Starting after an explicit retry or valid device change
Running -> Stopping -> Stopped when the final lease is released
```

Preview is an active `AudioUse`, not a lifecycle phase. This distinction allows
a connected session and a permitted mixer preview to hold leases concurrently
without publishing contradictory lifecycle state.

Device removal may move `Running` directly to `Reconfiguring`. During
reconfiguration the application publishes that transmission is unavailable; it
does not leave a stale "transmitting" indicator visible.

## Talk-state updates

Talk state is a high-frequency event family with the values required by the
wire protocol and existing client behaviour, including passive, talking,
whispering, and shouting variants.

`TalkStateStore` applies an update only when the value changes. Presentation
then emits `dataChanged` for the affected entity and talk-related roles only.
Stopping speech may retain a separate, low-frequency "recently active" timestamp
for participant ordering. This timestamp does not cause per-frame model churn.

## Chat delivery

Sending a message follows this sequence:

```text
Create ClientMessageId
  -> append local Pending message
  -> validate size, target, epoch, and permissions
  -> invoke SessionCommandPort
  -> mark Sent on positive local handoff or protocol confirmation
  -> mark Failed on validation, disconnect, or transport failure
```

Mumble does not provide a universal server acknowledgement for every text
message. The implementation must document the exact point at which `Sent` means
"accepted by the local transport" rather than "read by the server". The UI must
not imply delivery or read receipts that the protocol cannot prove.

## Presentation models

### ChannelTreeModel

The minimum role set is:

```text
entityKey
nodeType
displayName
channelId
userSessionId
depth
userCount
isCurrentChannel
isSelf
talkState
isMuted
isDeafened
isSuppressed
isPrioritySpeaker
isRecording
isAuthenticated
isListener
isLinked
canEnter
hasComment
hasAvatar
localVolume
```

Role names are versioned as an internal presentation contract. Removing or
changing the meaning of a role requires updating model contract tests.

### ChannelMemberModel

Projects users from the selected or current channel without owning duplicate
user state. It supports desktop layouts that separate channel navigation from
the participant surface.

### ChatListModel

Exposes message identity, conversation identity, sender, sanitized content,
attachments, timestamps, delivery state, and grouping hints. Bubble geometry,
sticky headers, and autoscroll remain in presentation.

### ServerListModel

Combines stored favourites, recent targets, and discovered LAN servers into
explicit sections. Passwords are not model roles. Ping data is optional and
expires rather than remaining indefinitely current.

## View-model command surface

The first QML-facing surface should remain small:

```text
ApplicationViewModel
  openSettings(section)
  requestQuit()

ConnectionViewModel
  connectTo(serverBookmarkId)
  connectToEditedTarget(draftId)
  cancelConnection()
  disconnect()
  acceptCertificate(challengeId)
  rejectCertificate(challengeId)

SessionViewModel
  joinChannel(entityKey)
  selectChannel(entityKey)
  openPrivateConversation(entityKey)
  setUserVolume(entityKey, value)

VoiceViewModel
  setMuted(value)
  setDeafened(value)
  setTransmitMode(mode)
  setPushToTalkPressed(value)

ChatViewModel
  sendText(conversationKey, text)
  retryMessage(clientMessageId)
```

Context menus are assembled from application-provided capabilities. QML does
not duplicate permission checks to decide whether an action is valid.

## Errors and challenges

`ClientError` includes:

```text
code
severity
retryable
localizationKey
localizationArguments
redactedDiagnosticContext
```

Interactive challenges such as certificate trust are not errors. They have a
stable challenge ID, sanitized certificate summary, available decisions, and an
expiry tied to the connection attempt.

Presentation decides between inline status, toast, notification, sheet, or
dialog. Application code never selects a visual surface.
