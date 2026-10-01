# Original Bink replay regression

This is an actual-client regression requiring privately supplied SWG assets and
the original matching Bink/Miles providers. The SDK-free protocol tests cannot
prove this behavior. Do not replace the decoder or mark missing media as a pass.

## Setup

Build the development client and host as described in the parent README. Keep
the original `binkw32.dll` beside the host. Record the source revision, build
configuration, client/host/provider hashes, platform and output device. Preserve
the previous host so the same client can discriminate the repair from the old
behavior. Do not mix Debug and Release packages or change system audio defaults.

Use a Debug client at the login screen without connecting. Leave cut-scenes
enabled. This check uses the existing debug console (`Ctrl+Shift+grave`) and
`video play video/npe_falcon.bik`. Use the original movie from the mounted TRE
assets; do not extract a loose copy that could hide loss of TreeFile IO.

## Actions and acceptance

1. Enter the command and close the console. Capture distinct advancing frames,
   then wait for the movie to finish and the login UI to return.
2. In the same process, enter the identical command again. Require another set
   of advancing movie frames and a return to the UI after completion.
3. Start it a third time. While frames are advancing, request ordinary window
   close. Require the game and its owned host to exit normally. Forced cleanup
   does not count as a pass. Preserve dialogs, exit codes and failed attempts.

The console prints “Playing cut-scene” regardless of whether the start succeeds;
that text and successful key delivery are not playback evidence. Record routed
audio separately. Nonzero audio alone cannot identify the movie track or prove
audiovisual fidelity, synchronization, volume correctness or native-Windows parity.
Skipping in a ground/space scene is a separate check: the login UI does not supply
the same scene input handler.

## Failure discrimination

With the prior once-only IO setup, the first movie opened but the second did not.
A private observational trace reached `PipeBinkVideo::create` on the second
attempt and the original decoder returned `Error opening file.` The game gates
and video cache were not the rejecting path. Bink consumes its IO callback and
buffer-size settings on open; the original `BinkVideo::newBinkVideo` reapplies
both before each open. The host must retain that call order, including attempts
that fail. Keep the before/after result bound to the exact host binaries; this
regression does not qualify other codecs or arbitrary movie dimensions.
