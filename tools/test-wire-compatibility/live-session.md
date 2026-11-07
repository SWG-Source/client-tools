# Live client/server acceptance

These manual checks complement the byte fixtures. They exercise an actual client,
login server, connection server, game server and saved character. They do not
replace packet fixtures or establish complete gameplay or audiovisual fidelity.

## Procedure

Use an isolated server/database copy and an existing disposable test character.
Keep account credentials, provider binaries, assets and packet captures private.
Record the source revisions, executable hashes, compiler/configuration, runtime,
server process architecture and assets used. Test the same client/server builds
throughout each comparison; do not infer results for a different build.

1. Verify the server is running, its database is connected, and the advertised
   login and connection addresses are reachable by the client. If forwarding
   ports, verify replies reach the client as well as requests reaching the guest.
2. Use the ordinary login UI and existing account. Inspect the returned galaxy
   and avatar list, then select the known character. Stop on unexpected creation
   UI, protocol rejection or a fatal dialog. Do not patch packet or authentication
   code to make the test pass; record any existing local authentication mode.
3. Observe completed world loading, the character, terrain, HUD and location.
   Correlate the selected object with a new server-side login record. A login
   timestamp alone does not prove that the world rendered.
4. Apply a bounded movement input and observe the HUD position. Close the game
   ordinarily. Preserve warning text, exit status and any forced cleanup.
5. Check that the server subsequently saves the movement for that same object.
   A delayed save is not an immediate failure; distinguish persisted state from
   live position. Keep the original before/after observations.
6. Repeat with the other client architecture against the same server. Repeat
   that pair against the other server architecture when available. Record each
   cell separately; a passing pair does not establish the unrun combinations.

The existing `autoConnectToLoginServer` setting may submit configured test
credentials through the normal login path. Keep `autoConnectToCentralServer`
and `autoConnectToGameServer` off, and select the known avatar manually. The
legacy UI enters creation when the returned avatar list is empty; do not proceed
with that flow during this test.

## Recorded checkpoint: 2026-10-01

VS2013-built Debug clients ran under GE-Proton 11-7 in an owned headless display
and audio prefix, using the original SWG Source v3.0 assets and media providers.
The server was a native Linux build of `8e57911e` in a writable validation VM
overlay. It used the existing local test authentication configuration; external
account-service authentication was not tested. Neither
side was a pristine stock-release executable. The Linux builds used the saved
Release configuration with an `-O0` override; this is not optimized-server
performance evidence.

| Client | Server | Run | Observed result |
|---|---|---|---|
| Win32 | 32-bit | `scene-34bcfeb5` | Existing character entered Mos Eisley; movement was saved; game/compositor exited 0. |
| x64 | 32-bit | `scene-bb79a278` | Same character entered Mos Eisley through the normal login/selection path; movement was saved; game/compositor exited 0. |
| Win32 | 64-bit | `scene-ea3d4878` | Same character entered Mos Eisley; movement was saved after a delay; game/compositor exited 0. |
| x64 | 64-bit | `scene-14cd1ffc` | Same character entered Mos Eisley; movement was saved after a delay; game/compositor exited 0. |

Artifact identities:

- Win32 game, source-verified build at `10fe0da59`:
  `c356d0e76f6460d154f52d8d3c74eac7ef8688e460665ea1f0e645e2f41a9fb4`.
- x64 game at `a904e489b`:
  `eb7ef36744322e9fc4b0b4e87f9a10b59e89b08660ea24c8c403f4ddc14d4585`.
- x86 media host with the `da9c56054` repair, used by x64:
  `161ccccd4119176f8edfb91555a8c598112dfb7920d23233304f9350fec87df8`.

The first Win32 attempt (`scene-57915fd2`) did **not** pass login. Guest-side
captures showed repeated Connect/Confirm packets, while an independent host UDP
socket received no reply. Installed libslirp 4.9.4 drops restricted-mode UDP before
socket lookup ([source, lines 166–173](https://qemu.googlesource.com/libslirp/+/refs/tags/v4.9.4/src/udp.c)).
The owned VM's forwarding configuration was corrected while retaining guest
egress isolation and localhost-only host listeners. Two independent transport
probes received matching confirmations before the unchanged client was rerun.
The original failed run remains preserved; no game protocol was altered.

The first 64-bit-server startup also failed: the saved TaskManager environment
selected a 32-bit `libjvm.so`. The console reported running/database-connected,
but only 46 server processes survived, so no client session was attempted on that
state. Selecting the already installed 64-bit JVM brought up all 84 server
processes, with all 38 game servers mapping that JVM. The failed startup log and
the exact environment-only change are retained. Server binaries and database
were unchanged by this repair.

The successful 32-bit-server runs retained 196 Win32 and 164 x64 shutdown
warnings. Routed PCM was finite and nonzero on both, and host default audio
endpoints were unchanged. This is audio-output evidence, not waveform equality.
The first Win32 movement overlapped a reward-dialog interaction, so the differing
movement distances are not a controlled timing or physics comparison.
The 64-bit-server runs retained 180 Win32 and 160 x64 shutdown warnings.
All four runs recorded `wineserver -k` returning 1 during subsequent cleanup;
that status is separate from the observed native game exit 0. The media host's
exit status was not independently captured.

In the final x64/64-bit-server cell, two post-close database snapshots retained
the old position; a later snapshot confirmed the movement. The last observation
was 197 seconds after the preceding snapshot, exceeding the planned 180-second
follow-up by 17 seconds. The server's default unsafe-logout delay is three
minutes, which is consistent with delayed persistence, but that session's
logout path was not traced. Do not infer an immediate save or a particular
disconnect path from the client's exit code. The recorded server output had no
matched protocol, fatal or movement-rejection markers; it does not log every
movement or disconnect event.

## Release x64 follow-up: 2026-10-01

One additional run, `scene-b974d50e`, passed the bounded normal-login,
existing-character, Mos Eisley rendering, one-second movement, persistence and
ordinary-close procedure with the Release x64 client against the same unchanged
64-bit Linux server at `8e57911e`, under GE-Proton 11-7. This adds one Release
client/server combination; it is not a complete Release matrix.

The game uses source-verified `33efad160` inputs, SHA-256
`307042f7569c0c386070c124cd61ddc3243f307dbc7d9eb59dbb2b3e0d978228`,
with matching Release renderer/DPVS/libxml and the repaired `da9c56054` Release
media host, SHA-256
`67c2c17c0ea9ae78d0a8167bff3967453f1d34142783690bf3052b76433fc32f`.
An initial post-close database sample retained the old position; a subsequent
sample confirmed movement about 164 seconds after the close request, within the
planned 240-second limit. Both observations remain preserved; the save path was
not traced.

Game and compositor exited 0 through ordinary close. Routed PCM was finite and
nonzero, host audio defaults were unchanged, and staged runtime/configuration
files were restored and hash-checked. Subsequent Wine cleanup returned 1; the
media host exit code was not independently observed. Existing local server
policy accepted the saved account without password verification, so this is not
external-authentication evidence. No server configuration, account creation or
additional gameplay/media test was performed.

## Remaining acceptance

This checkpoint covers one galaxy, one existing character, one ground location
and a bounded movement/save/exit sequence. It does not establish combat,
inventory/trading, mission progression, space flight, transitions, representative
scene coverage, performance or long-session stability. The two-galaxy byte
fixtures still provide coverage that this one-galaxy live session does not.
Broader Release gameplay and native Windows/GPU operation remain separate gates.
Full audiovisual fidelity and the media host's separately observed exit status
are not proven by the game's exit code. Private raw artifacts require the
matching local environment; this document is a reproducible procedure and scoped
record, not a self-contained CI job.
