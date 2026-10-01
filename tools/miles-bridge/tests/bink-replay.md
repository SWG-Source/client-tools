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
Skipping is a separate check in a loading or character-creation screen that
actually calls `Game::skipCutScene`. The ordinary login UI and an already loaded
ground scene do not provide a general Escape-to-skip contract.

## Return to a loaded ground scene

Use the same pinned Debug package and original assets. From the existing Load
Scene UI, select `terrain/tatooine.trn` and wait for terrain, player and HUD to
render. Record the HUD position before proceeding; a loading screen alone is
not scene acceptance.

1. Open the console, issue the same Falcon movie command and close the console.
   Capture distinct advancing movie frames.
2. Let the movie complete without another command. Require the terrain, player
   and HUD to return. This exercises `Game::_endCutScene` restoring scene drawing.
3. Hold the normal forward-movement input for one second. Require a changed HUD
   position and rendered scene; successful key injection alone is not evidence.
4. Request ordinary window close and record game/host exit, dialogs and audio
   routing as above. Preserve any new assertion, crash or failed return to play.

The uninstrumented x64 Debug client and repaired host passed this bounded
procedure under GE-Proton11-7 (`scene-7b7dd4cd`): distinct movie frames, restored
terrain/HUD, position Z changing from 0 to 5 after movement, and game/compositor
exit 0 after the normal 85-warning dialog. This is not a skip, server gameplay,
native-Windows, sound-volume or audiovisual-equivalence result.

## Failure discrimination

With the prior once-only IO setup, the first movie opened but the second did not.
A private observational trace reached `PipeBinkVideo::create` on the second
attempt and the original decoder returned `Error opening file.` The game gates
and video cache were not the rejecting path. Bink consumes its IO callback and
buffer-size settings on open; the original `BinkVideo::newBinkVideo` reapplies
both before each open. The host must retain that call order, including attempts
that fail. Keep the before/after result bound to the exact host binaries; this
regression does not qualify other codecs or arbitrary movie dimensions.
