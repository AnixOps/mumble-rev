# Phase 0 Smoke Checklist

Status: manual baseline protocol. This checklist verifies the unmodified legacy
client and does not mark Phase 0 complete by itself.

Use the controlled server and local configuration described in
[Phase 0 Windows Baseline Procedure](phase0-baseline.md). Execute applicable
cases on Windows 10 and Windows 11 at 100%, 150%, and 200% display scaling in
both Debug and Release builds. For every case, record the build type, Git
revision, Windows version, display scale, test-server revision, operator, UTC
timestamp, pass/fail result, and evidence location.

## P0-SMOKE-01: Connection and cancellation

**Preconditions:** A controlled server and test account are available. The
client starts disconnected.

**Steps:** Connect to the controlled server, cancel while connection progress
is visible, confirm the client returns to a disconnected state, then connect
again and complete synchronization.

**Expected result:** Cancellation does not leave a connected session or block a
subsequent connection. The second attempt synchronizes normally.

**Evidence location:** `P0-SMOKE-01/<run-id>/connection-cancellation.log`

## P0-SMOKE-02: Valid certificate

**Preconditions:** The controlled server presents its expected valid
certificate and the client has no conflicting certificate state.

**Steps:** Connect and complete server synchronization.

**Expected result:** The connection completes without an unexpected certificate
warning or trust decision.

**Evidence location:** `P0-SMOKE-02/<run-id>/valid-certificate.log`

## P0-SMOKE-03: Rejected certificate

**Preconditions:** A controlled endpoint presents a self-signed or changed
certificate that requires an explicit decision.

**Steps:** Start a connection, reject the certificate when prompted, and return
to the disconnected state.

**Expected result:** The rejection prevents connection and does not persist a
trust decision for the rejected certificate.

**Evidence location:** `P0-SMOKE-03/<run-id>/rejected-certificate.log`

## P0-SMOKE-04: Audio and microphone privacy

**Preconditions:** A working input device is selected. Repeat with microphone
access blocked by Windows privacy controls or with the device unavailable.

**Steps:** Connect, exercise the configured transmit mode, observe local audio
activity, then repeat with access blocked or unavailable.

**Expected result:** With access, transmit behavior and local indicators work.
With blocked or unavailable access, the client reports the condition without
crashing or exposing audio.

**Evidence location:** `P0-SMOKE-04/<run-id>/audio-privacy.log`

## P0-SMOKE-05: Channel and private chat

**Preconditions:** The server has at least two channels and a second controlled
test account is connected.

**Steps:** Join another channel, send and receive a channel message, send and
receive a private message, then return to the original channel.

**Expected result:** Channel membership and both message types appear for the
intended recipients with no cross-channel or cross-user leakage.

**Evidence location:** `P0-SMOKE-05/<run-id>/chat.log`

## P0-SMOKE-06: Global shortcuts

**Preconditions:** A global push-to-talk or equivalent configured shortcut is
available and the client is connected.

**Steps:** Move focus away from the client, press and release the shortcut, and
repeat after restoring client focus.

**Expected result:** The shortcut activates and releases transmit state
correctly in both focus states without becoming stuck.

**Evidence location:** `P0-SMOKE-06/<run-id>/shortcuts.log`

## P0-SMOKE-07: System tray

**Preconditions:** System tray support is enabled and the client is connected.

**Steps:** Minimize or hide the client to the tray, use the tray menu to restore
it, then use the tray action to disconnect or quit as configured.

**Expected result:** Tray actions affect the intended client window and session;
the application remains responsive and exits only through the requested action.

**Evidence location:** `P0-SMOKE-07/<run-id>/tray.log`

## P0-SMOKE-08: Overlay and plugins

**Preconditions:** A supported controlled target for overlay testing and at
least one enabled positional-audio plugin are available.

**Steps:** Start the controlled target, confirm overlay behavior, then exercise
the enabled plugin's expected detection or positional-audio behavior.

**Expected result:** The overlay and plugin remain functional or produce the
known baseline diagnostic without a client crash or stalled session.

**Evidence location:** `P0-SMOKE-08/<run-id>/overlay-plugins.log`

## P0-SMOKE-09: Reconnect

**Preconditions:** The client is synchronized to the controlled server.

**Steps:** Interrupt network connectivity or stop the controlled server,
restore it, and allow the configured reconnect path to run.

**Expected result:** The client reports the interrupted state, reconnects when
the server is available, and restores a usable synchronized session.

**Evidence location:** `P0-SMOKE-09/<run-id>/reconnect.log`

## P0-SMOKE-10: Shutdown

**Preconditions:** The client has completed a normal connected session and no
modal certificate or permission decision is pending.

**Steps:** Disconnect, then close the application. Repeat once using the
configured tray quit action.

**Expected result:** The client exits without a hang, crash, or orphaned
process, and subsequent startup is possible.

**Evidence location:** `P0-SMOKE-10/<run-id>/shutdown.log`
