# Miles backend development

This directory holds the maintained implementation of the temporary Miles
process bridge. `src/api/ClientMiles.h` uses the selected Miles SDK's handle and
callback types. A future native x64 backend can replace the pipe backend at this
boundary without exposing IPC to game code.

The bridge is unfinished and is **not selected by the default SwgClient build**. Building these
targets does not establish a playable x64 client or equivalent audio behavior.

## Bink development integration

The opt-in Video factory uses `ClientBink` while the original x86 Bink 1.9c DLL
runs beside its genuine Miles driver. Existing TreeFile operations run through
the engine file worker. The game retains its texture shader, quad rendering,
frame scheduling, controls and supported backbuffer formats. The private adapter
copies validated complete frames and keeps vendor pointers inside the host.
No SDK upgrade or alternative codec is used. Place the matching `binkw32.dll`
beside `miles-host.exe`; it is not bundled in this repository.

The latest VS2013 builds produced zero errors: host, pipe, game link and default
Win32 Release graphics had zero warnings; isolated Debug-x64 graphics had26
existing narrowing/packing warnings. The game links the new isolated graphics
archive. Native reply validation passed24 cases, and its stale-identity mutation
failed as expected. Protocol sanitizer checks passed1,047 cases. These results
establish compilation and bounded protocol behavior, **not movie playback or
media fidelity**; runtime qualification is the next check. The zero-return sound
initialization cleanup is source/build checked, not induced against the real SDK.

The retained DLL/IO roots are process-owned. Actual close/callback ordering,
timing, repeated playback and audio/video equivalence remain runtime gates.
The existing renderer is1024x1024 and the development pixel staging budget is
4MiB; this is not a claim of arbitrary movie dimensions or concurrency support.

## Build

Use Windows, Python 3, Visual Studio 2013 (v120), and the existing Miles SDK.
No SDK headers, import libraries, DLLs or game assets are bundled here.

```powershell
python build.py --target host --sdk C:/SDK/Miles/include --sdk-lib C:/SDK/Miles/lib/win/Mss32.lib --out C:/build/miles
python build.py --target pipe --sdk C:/SDK/Miles/include --out C:/build/miles
python build.py --target native --sdk C:/SDK/Miles/include --out C:/build/miles
python build.py --target engine-worker --engine-root C:/source/client-tools --out C:/build/miles
python build.py --target pipe-probe --sdk C:/SDK/Miles/include --engine-root C:/source/client-tools --out C:/build/miles
python build.py --target audio-dev --sdk C:/SDK/Miles/include --engine-root C:/source/client-tools --out C:/build/miles
```

`--vcvars` can select the installed VS2013 `vcvarsall.bat`. Outputs and complete
compiler/linker logs are separated by target, architecture and invocation. Each
invocation writes `receipt.json`, including failed commands. The script does not
run the resulting binaries.

| Target | Output | What it checks |
| --- | --- | --- |
| `host` | x86 `miles-host.exe` | Compiles the real command/file-callback host and links the genuine `Mss32.lib`. |
| `pipe` | x64 `miles-pipe.lib` | Compiles the client adapter. An archive can contain unresolved references; this is not a client link. |
| `native` | x64 `miles-native.lib` | Compiles direct calls against the SDK's Win64 declarations. A matching native vendor library is still required to link it. |
| `engine-worker` | x64 `miles-engine-worker.lib` | Compiles the file executor using real engine headers, STLport and clientAudio's Debug-x64 definitions/include paths. |
| `pipe-probe` | x64 `miles-pipe-probe.exe` | Links the real pipe adapter and engine worker against previously built Debug-x64 engine libraries and rebuilt STLport. |
| `audio-dev` | x64 `Audio.obj`, `SoundObject3d.obj`, `SetupClientAudio.obj` | Compiles the real Miles callers and setup/teardown together; no link or production selection. |

Choose exactly one client backend. The pipe and native archives implement the
same public names and must not be linked together. Both currently use the debug
static CRT for component development. The engine worker is a separate
translation unit with the legacy engine's STLport and `wchar_t` settings; its
interface passes only plain values and function pointers. Application
CRT/configuration integration is a separate build step.

## Explicit development project build

After building the `pipe` and `engine-worker` targets against the same engine
checkout, add these properties to the existing **Debug|x64 v120** solution build:

```text
/p:ClientMilesDevelopment=true
/p:ClientMilesPipeLibrary=C:/build/miles/pipe/x64/<invocation>/miles-pipe.lib
/p:ClientMilesWorkerLibrary=C:/build/miles/engine-worker/x64/<invocation>/miles-engine-worker.lib
```

Keep the DirectX and source-dependency properties described in
`tools/configure-client-x64/README.md` and `tools/build-client-deps/README.md`.
Build the solution's `SwgClient` target, including its solution dependencies.
If building project files individually, build `clientAudio` and `clientGraphics` first with the same
properties: the project files themselves do not contain `ProjectReference` edges.

This opt-in compiles the actual `clientAudio` and `clientGraphics` projects with
the private adapters, including setup/teardown and video presentation. Their
intermediates and game outputs go under
`src/compile/miles-dev/x64/<project>/Debug/`, separate from the direct-Miles
build. The game names those exact Audio and Graphics archives, so a missing development archive
cannot silently fall back to the ordinary one. The two genuine bridge archives
are additional link inputs; other provider inputs remain unchanged.

The opt-in is experimental and restricted to Debug x64. Ordinary builds continue
to use direct Miles. A completed build does not qualify actual game startup,
Audio/global ExitChain shutdown, devices, media or gameplay fidelity.

The native VS2013 `SwgClient:Rebuild` check completed with zero errors and
3,319 warnings across the solution; those warnings are not waived. All 20,554
selected source/build files matched the input manifest before and after the run.
After aligning the development target name with `SwgClient_d.exe`, the game
project build completed with zero warnings/errors. The direct-Miles Win32
Release `clientAudio` rebuild also completed with zero warnings/errors. An
explicit Release-x64 development selection was rejected as intended.

The resulting Debug-x64 game SHA-256 is
`84de2516126f7cc183a0c14b7fc1d35e951e43d10e6e0e7f3ce8e2de9bd77a3c`.
The game and its solution dependencies were rebuilt from source. The pipe and
engine-worker archives were separately compiled by `build.py` at the preceding
composition checkpoint; this is not a rebuild of every external SDK binary.

Development shutdown stops sounds while their templates exist, performs the
paired SDK/worker close, then detaches the file-serving hook. The setup owner
removes templates before freeing cache-owned path strings. Failed driver startup
closes operational Audio state once but retains templates/cache for continuing
disabled-audio UI use. These source-order repairs do not establish the cause of
the earlier standalone engine teardown fault or qualify actual game shutdown.

## Development runtime packaging

Keep the matching original x86 redistributables with the private worker:

```text
game/
  SwgClient_d.exe
  helper-x86/
    miles-host.exe
    Mss32.dll
    miles/          (original matching .asi and .flt providers)
```

The private development configuration selects `helper-x86/miles-host.exe` and
`helper-x86/Mss32.dll` with `ClientAudio/devMilesHost` and `devMilesDll`.
In the tested runtime, the existing relative `miles` redist path loads providers
under the helper directory. Placing them only in `game/miles` allowed driver
initialization but made title-MP3 stream opening fail with the native error
`Error getting sound format.` The helper-local layout loaded the original
providers and allowed stream opening and playback. No vendor files are bundled.

For a Linux/Proton test, the x86 worker also needs the runtime's i386 audio
dependencies. A missing `libpulse.so.0` caused ALSA fallback: a silent capture
from the intended null sink did **not** establish silent playback. Check the
actual producer route, not just unchanged desktop default device names. The
private test supplied cached dependencies through a process-local library path
and disabled ALSA fallback; it did not modify system audio settings.

### Actual startup and ordinary exit checkpoint

At source commit `4953206a2`, the product Debug-x64 development client reached
the login UI with original SWGSource v3.0 assets and no credentials or connection.
The manifest ownership repair in that commit removes an observed normal-exit
use-after-free. VS2013 built `sharedFile` for Debug-x64 and Release-Win32 and
relinked the game with zero errors; six existing x64 narrowing warnings remain.

The bounded GE-Proton11-7 run used the product game, renderer and host, without
the earlier private source tracing. It captured 2,917,889 nonzero finite audio
samples, with 56 observations binding the worker to the owned null sink. The
exact-target Windows close request produced **client exit 0** after acknowledging
only the normal `78 warnings logged` dialog; **compositor exit was also 0**.
No owned processes remained and desktop audio defaults were unchanged. The
post-exit `wineserver -k` command returned 1; that cleanup record is retained.

Game SHA-256: `2f5d846fa330a8464fb00e4b9c6ea0a2be26d539e67cb7a313dddd7017147216`.
Host SHA-256: `80a8c4d99876ea98243fb936076c21692329a2431ebd2825cc3b5204a4dd897b`.
Private run identifier: `startup-4c257792`.

This establishes startup, routed audio output and ordinary exit in that setup.
It does not establish audio/shader fidelity, native Windows rendering, ground or
space gameplay, mixed-width network acceptance, or the cause of the old separate
standalone teardown failure. Bink and Vivox remain unresolved. Earlier failures
are retained, including the manifest crash, invalid audio routing and a separate
compositor screenshot-thread crash after an already-successful game exit.


### Local ground and space scene checkpoint

The same unmodified game and host artifacts above were then exercised through
**Load Scene** with the original assets, without credentials or a server. These
were separate bounded GE-Proton11-7 processes, not diagnostic game builds.

| Observation | Tatooine ground | Tatooine space |
| --- | --- | --- |
| Terrain selected and logged | `terrain/tatooine.trn` | `terrain/space_tatooine.trn` |
| Rendered | Terrain, sky, HUD | Asteroid field, sky, HUD |
| Input | One-second forward input: HUD Z changed 0 to 6 | No ship/flight claim |
| Client / compositor exit | 0 / 0 | 0 / 0 |
| Nonzero finite PCM samples | 15,139,912 | 13,843,958 |
| Owned-sink route observations | 183 | 248 |

Both ordinary close requests acknowledged only the same-process **85 warnings
logged** dialog. Desktop audio defaults were unchanged. The post-exit
`wineserver -k` status 1 remains in each cleanup record. Private run identifiers:
`scene-00cdba0f` and `scene-1ab9a607`; their assessments retain artifact hashes,
input receipts, captures, routes, warnings and the original failed attempts.
No vendor media, binaries or SDK content is included here.

The scene-selection list and filename field rendered blank, although selecting
list rows populated the internal filename and description. Filename text entry
did not work; using the list required no product change. A private diagnostic
run established that the list contained 26 entries, but its observations are not
substituted for these product runs. This legacy debug UI issue remains open.

These observations extend the startup result to local ground rendering/movement
and space rendering/ordinary exit. They do **not** establish representative
multiplayer gameplay, space flight, native Windows behavior, media or shader
fidelity, or performance. Bink and Vivox remain unresolved feature blockers.


### Bink host groundwork (not connected to playback)

`src/host-bink` adds the original Bink decoder owner inside the x86 Miles host.
It accepts that host's genuine digital driver. The native decoder, BINKIO
buffering state and pixel buffers stay in x86; file operations use the existing
reverse-file worker and opaque tokens. No client pointer is passed to Bink.
The IO adapter is adapted from the repository's `BinkTreeFileIO.cpp` and retains
its read-ahead/suspend/idle behavior. Large logical reads are split only at the
private transport limit, and stop on the first short read.

The possessed DLL is PE x86 Bink **1.9c**, matching the repository header and
required decorated exports. Its SHA-256 is
`e67e0319f9929c024a6d0757de50c65257f686f6711cb0efb7ad53afc3405dd4`.
The existing host build defaults to the repository's Bink include directory;
`--bink-sdk` can select another matching include location. Nothing downloads or
redistributes the DLL or SDK.

The complete VS2013 x86 host compiled/linked all 23 translation units with
`/W4 /WX`, zero errors/warnings. The source manifest and raw receipt were checked
against this tree. Host SHA-256:
`835046becced257b6f20035db451ad76e78896e81e9e2f1731cbfd6428887d6f`.
This establishes compilation/linkage only. Movie commands, client texture
presentation and runtime playback are **not yet connected**. The actual ground
and space results above used the earlier host, not these new units.

Before using this owner, the integration must close every movie and establish
Bink producer quiescence before releasing its file service or Miles driver.
The immutable IO binding remains process-owned; a local live-resource count is
not proof that native background callbacks have stopped. Video/audio fidelity,
frame timing and close ordering still require the original-DLL runtime test.

## Source layout

- `api/`: the game-facing declarations, direct native adapter and pipe adapter.
- `host/`, `backend/`, `dispatch/`: x86 process entry and genuine SDK calls.
- `wire/`, `protocol/`, `transport/`: byte encoding, resource identities and pipes.
- `file-*`, `host-runtime/`, `client-runtime/`: reverse file callbacks and their
  ownership, admission and acknowledgement handling.
- `buffer/`, `image/`, `upload/`: bounded image transfer and sample-image ownership.
- `metadata/`, `version/`: owned metadata and original module version queries.

`sources.json` lists each target's translation units. Relative includes preserve
which private implementation each target uses; do not add every source directory
to the include path to bypass missing includes.

## Remaining integration

Sample binding and typed sample/stream EOS registrations are implemented and
have the bounded original-DLL observations below. Real engine worker execution
and reverse file operations have been observed. There are no success stubs.

The actual Audio and listener sources compile against the facade in the
development-only target. Explicit bootstrap, admitted file callbacks and paired
shutdown are connected in that source path. A Debug-x64 development game relink
has zero unresolved symbols; the default game build still selects direct Miles.
The bounded game startup below reaches Audio initialization, but startup and teardown are not qualified. Media-format qualification, callback
scheduling in gameplay, device behavior and fidelity are still open. Do not
use this helper as the game's audio backend yet.

The maintained sources consolidate the previously reviewed pipe composition113
and native70 adapter. Folder names changed; the public header is unchanged.
Unused earlier adapter implementations are excluded. Existing component test
records remain evidence for their original source versions; they do not prove
the newly combined executable behaves correctly.

## Initial build checkpoint — 2026-09-30, before lock integration

The host, pipe and native commands completed on native Windows with VS2013, `/W4 /WX`,
`/EHsc`, and `/MTd`: 19 host, 17 pipe and 9 native translation units, with zero
compiler/linker warnings or errors. The host PE is x86 and imports `mss32.dll`,
including the genuine `AIL_WAV_info`, `AIL_file_type` and `AIL_startup` exports.
The engine-worker target also compiled and archived cleanly against the verified
`49d0eeed4` engine tree with its explicit legacy flags. No runtime test was
performed on these combined artifacts.

| Artifact | SHA-256 |
| --- | --- |
| `miles-host.exe` | `e32c0d18a263683383d59e77150366091c854b403a08281b7d2bfc57d7dc2ca7` |
| `miles-pipe.lib` | `ef6f65c38368b05d9a1a91e16c87f3e56b2dec5398e5815ddfa8af393f26d123` |
| `miles-native.lib` | `8ba853ac23ae37c31945ea1fd46d599e85d55b0b8d03e53b23c7c2b20781f07f` |
| `miles-engine-worker.lib` | `6158ebdd173a624b1a207a0de1d50645d067dbbdc8c6cbb9e970d2ed5545d0b6` |

SDK header SHA-256: `966e1e81046851295079e9709f9f358286c823e4403ec6b0ca972b5059725e6e`.
The x86 import library SHA-256 is
`e8c57b302fa2ea699a635ae1dad5467abd810370472c3b1d7d6e6ebc15af9252`.
Artifacts include build metadata; these hashes identify this run rather than
specifying reproducible binary hashes. Use the script's per-invocation logs and
receipt to assess another build.

The admitted Audio callback refactor compiled with zero warnings/errors in
native Win32/x64 Debug/Release. The complete Win32 SwgClient incremental build
and link also passed in Debug and Release: zero errors, with 30 build warnings
in each run. These include PDB warnings and linker import/runtime-library or
option warnings; the Debug link retains the project's `/FORCE` option. This
corrects the "30 linker PDB warnings" shorthand in the callback commit message.
No binary equivalence or clean-warning baseline is claimed. An initial x64 compile against an older VM checkout failed
on an existing `Archive.h` narrowing warning; the final checks used the verified
current engine mirror. No warning was suppressed. These results do not qualify
the new callback path at runtime; current game registration still uses the
legacy callbacks.

## Lock command contract

The command session is bound to its creating thread. `lock` and `unlock` call
the genuine SDK counter operations in the x86 host. Both endpoints derive their
scheduling action from the opcode and carry the current lease on every command.
Nested locks retain the same lease until the final unlock. A validated refusal
settles the command without changing depth; causal file acknowledgements still
must arrive before settlement. Background file callbacks remain serviceable.
Neither a counter operation nor a returned command proves callback quiescence.
Shutdown while locked is refused before calling the SDK.

The portable admission regression can run without the SDK:

```sh
c++ -std=c++11 -Wall -Wextra -Werror tests/lock_admission.cpp src/admission/coordinator.cpp -o lock-admission
./lock-admission
```

It checks nesting, refusal, stale/foreign leases, causal acknowledgement joins,
background observations and terminal-state retention against the real
coordinator. Its expected total is exactly 53 checks; it does not exercise the
SDK, actual file worker or live pipe path.

## Combined lock probe — 2026-09-30

Build `host` and `pipe-probe` above. The latter requires the genuine engine
Debug-x64 archives in `src/compile/x64` and rebuilt STLport in
`src/compile/deps/v120/x64/Debug`; the receipt records all selected libraries.
Compiler PDBs are copied from their `obj` directories for this link, without
suppressing missing-PDB warnings. Put the original `Mss32.dll` beside the x86
host, then run with full paths:

```powershell
miles-pipe-probe.exe C:/test/miles-host.exe C:/test/Mss32.dll
```

Both programs compiled and linked on native Windows with VS2013, with zero
warnings/errors under `/WX`. The combined probe then ran under Wine with an
owned prefix and null audio sink. It exited zero, with empty stderr and exactly
these four lines; the desktop audio defaults were unchanged:

```text
PASS: malformed lock/unlock rejected; nested sequence completed
PASS: secondary caller rejected; owner sequence completed
PASS: startup, nested lock/unlock, ordinary preference, shutdown
PASS: pipe lock transport probe; test-only process exit, no teardown claim
```

The initial combined executable failed before these checks, in
`MemoryManager::free`. Its linked VS2013 ConcRT path contained an incompatible
allocator pair: CRT debug-new allocation and engine global delete. The fault
log did not establish the exact allocation that crashed. Replacing the control
owner's `std::mutex` with inline Windows SRW locks removed that dependency; the
rebuilt executable passed the same probe. The initial failure remains recorded.

| Artifact | SHA-256 | Result |
| --- | --- | --- |
| x86 host | `511846fa7ebcd486e3eaeb4131569de1f1b1985b77a99c3e13bf76b2fdff6c86` | Genuine DLL host used by both runs |
| Initial x64 probe | `9211a7b43a66d0736baf1f3818a1521750658f5dde74d0a48fc1061ed87fe206` | Failed in engine free before checks |
| x64 probe with SRW locks | `bffbe8e9ac34a180949981a99c28d954e7ff395dcceb520388a51cbadeae3fed` | Four markers, exit 0 |

The original DLL hash was
`0785b5f2aa81e68c41778bea1aa92ed545e2392827d90745838af2b14f6954fe`.
The portable 53-check regression also failed as expected when the refusal
condition was deliberately disabled.

This probes actual facade/host command transport and refusal handling. It does
not install file callbacks or start the linked engine worker, open a digital
driver, play media, or prove native Windows device behavior. The probe retains
its session and exits the process after the genuine SDK shutdown call; paired
channel/worker teardown remains unfinished. Full client integration, EOS/file
integration and original audio fidelity are still outstanding.

## Sample-file binding — 2026-09-30

Both setters reuse the sealed upload transaction. The host validates an owned
sample, exact byte count, signed block argument and optional NUL-terminated
suffix before calling the genuine SDK. Null and empty suffixes remain distinct.
The size-less setter needs a matching private `ScopedSourceImage`; the adapter
does not infer a readable allocation from an audio header.

Image and suffix storage is attached to the sample before native effect and
survives release of the temporary upload. A successful replacement discards that
sample's earlier images. A zero return preserves both old and attempted inputs:
failure is not a rollback guarantee. Genuine sample release or SDK shutdown
discharges the remaining inputs. An uncertain call is terminal and retains its
owners. This is sample-image ownership, not a claim of callback quiescence.

The selected DLL identified below initializes sample state and disconnects old
processors on its successful setter paths. This was checked against that
binary; the supporting local RAD reference is version 9.3b, not a matching
7.2e contract. RAD's [7.2a change history](https://www.radgametools.com/msshist.htm)
also describes automatic initialization by these setters. Other vendor versions
and codecs still require qualification.

The explicit image budget includes retained input bytes, two copies of the
incoming image and its suffix before replacement. It excludes allocator and
container overhead. Repeated failures can exhaust that budget; refusal is a
bridge policy limit, not transparent native behavior. Successful rebinds do not
retain an ever-growing image history.

Portable ownership tests (same-module modeled calls, no SDK exports):

```sh
c++ -std=c++11 -Wall -Wextra -Werror -fsanitize=address,undefined tests/upload_binding.cpp src/wire/codec.cpp src/buffer/buffer_upload.cpp -o upload-binding
./upload-binding
```

These passed with ASan/UBSan. They cover retained pointer readability, native
zero/negative statuses, uncertain completion, release, malformed requests,
owned/borrowed identities, 65 rebinds, and aggregate budget admission. Mutating
the owner to retain on success or discard on zero makes the test fail.

Build the existing `host` and `pipe-probe` targets, then run:

```powershell
miles-pipe-probe.exe C:/test/miles-host.exe C:/test/Mss32.dll --sample-bindings
```

Both programs built with VS2013 `/W4 /WX`, zero warnings/errors. Under an owned
Wine prefix/null sink, the actual x64 adapter and original x86 DLL completed 65
alternating named/unnamed binds on one handle, retained the expected duration
after the caller buffer was overwritten, returned zero for an invalid WAV,
accepted a subsequent valid replacement, and released the sample. The earlier
lock checks also completed. Generated PCM silence is used; no game media is
needed.

**Strict run result: failed.** All six expected PASS markers appeared and the
process exited zero, but stderr contained four ALSA `Invalid CTL hw:0/hw:1`
diagnostics. The empty-stderr acceptance rule was not relaxed. Desktop audio
defaults remained unchanged. This is evidence for the observed command sequence,
not a clean runtime result or audio-fidelity acceptance.

| Artifact | SHA-256 |
| --- | --- |
| x86 host | `01519e140ec407939ac39a2414baf6eda08234533ae96cc8706c29f346168fe2` |
| x64 probe | `fc0a50f1776277870e001f9c13e3f6d9e2c27ddce286b40bdc7a0641a8693cd4` |
| original x86 DLL | `0785b5f2aa81e68c41778bea1aa92ed545e2392827d90745838af2b14f6954fe` |

No engine file worker or Audio bootstrap was started, no EOS callback was
installed, and no paired teardown or native Windows device test was performed.
The unchanged game still uses direct Miles.


### Engine worker and reverse file-callback check

The `pipe-probe` target also supports:

```powershell
miles-pipe-probe.exe C:/test/miles-host.exe C:/test/Mss32.dll --file-callbacks
```

This mode installs the engine's real Thread/PerThreadData setup. A separate
worker receives 32 ordered jobs; each verifies engine TLS and a consistent
worker identity. It drains, joins and destroys that worker, checking that the
main thread's flags remain unchanged. The adapter then starts its own engine
worker and uses the actual reverse channel to serve a generated PCM WAV to the
original Miles DLL. The callbacks use Win32 file APIs and verify engine TLS;
they are test callbacks, not `Audio.cpp`'s file callbacks.

The native VS2013 `/W4 /WX` build completed without diagnostics. The runtime
produced all seven expected markers and exited zero: file open/read/seek/close,
stream duration and borrowed-sample lookup completed, followed by the prior
lock checks. **The strict run failed** because stderr contained the same four
ALSA `Invalid CTL hw:0/hw:1` diagnostics as the sample-binding check. The rule
was unchanged; desktop audio defaults were unchanged and the owned sink and
Wine server were cleaned up.

The tested probe SHA-256 is
`1dc7f8a26bf8bbd4b18fbca37a876f6e0c90c48e49393112b952a205d5df2265`.
It used the host and original DLL hashes listed above. This tests real engine
worker execution, not full Audio installation, global ExitChain teardown or
sound fidelity. The runtime session and its worker remain retained until
process exit; only the separate worker prerequisite exercises drain/destruction.


### Development compilation of the real Audio source

`--target audio-dev --engine-root <checkout>` compiles the maintained `Audio.cpp`
and `SoundObject3d.cpp`
with its real Debug x64 engine/STLport settings and an explicit
`CLIENT_MILES_DEV_FACADE` define. It does not link or change any production
project selection. The resulting objects call the facade rather than native
`AIL_*` functions. The normal source path remains direct Miles.

The two size-less operations receive lexical extents from their existing
allocation owners: the 3D sample-cache file size and the WAV query's file length.
The private extent declaration includes no modern STL; setup failures go through
the bound fatal reporter rather than throwing an adapter exception into engine
code. Speaker configuration, the version macro and the seven WAV metadata
fields used by Audio are explicitly adapted.

The current sources compile with VS2013 `/W4 /WX` and no diagnostics.
Development selection requires explicit `[ClientAudio]` keys `devMilesHost`,
`devMilesDll` and `devMilesUploadBudget` (decimal bytes within the existing
upload policy). There is no guessed budget or silent fallback. Modern adapter
exceptions terminate through the engine reporter instead of crossing the STLport
boundary. Module references preserve code; engine global state must still
outlive the callback worker.

The development path uses the existing admitted file callbacks, which do not
repeat legacy TLS installation. It tracks successful SDK startup independently
of Audio's installed flag, so driver-init failure can still shut down and close.
Shutdown and worker joins precede callback-map and sample-image cleanup. Actual
Audio runtime and global ExitChain ordering remain unverified.

### Development game relink

`link-client-dev.py` takes `--engine-root`, the actual Debug-x64
`--baseline-log`, successful `--audio-receipt`, `--pipe-receipt` and
`--worker-receipt`, plus a new `--out` directory. Supply the original drive
mapping if the saved linker command uses one. It adds both Audio objects and the
two bridge archives, preserves the baseline provider libraries, isolates its
outputs and refuses `/FORCE`. This reuses engine archives; it is not a fresh
whole-source build.

The observed native VS2013 Debug-x64 game relink completed with zero unresolved
symbols and no linker diagnostics. The PE is AMD64/PE32+ and has no direct Miles
import. SHA-256: `fb36364e5900ee7d7bd7229ba79b6f1d1da20166f75a1358da1e1e40c9053e4d`.
Other engine archives came from the `49d0eeed4` native build; the Audio objects,
pipe/bootstrap and engine-worker archives were freshly compiled from this tree.
The initial relink exposed three listener imports in `SoundObject3d.cpp`; its
development selection removed those by calling the real facade implementation.
The first tool invocation also exposed a linker/PDB-frontend mismatch, corrected
by choosing the linker from the matching VS2013 environment. Failed logs remain
preserved. Startup, media fidelity and gameplay acceptance are not established.


### Typed end-of-sample and stream callbacks

Both registration functions now return the callback represented by the actual
native SDK return. A successful sample rebind can reset that callback; the
client does not infer the previous function from its last registration request.
Stable native thunks represent distinct typed functions, so repeated registration
of the same function does not consume additional thunk slots.

The callback captures a resource generation, invokes the exact client function
on the engine worker, and waits for completion plus consumption acknowledgment
before returning to Miles. File callbacks and EOS share one transport and one
request sequence. Client proxy retirement waits for the native close/release
and acknowledgment. No bridge table lock spans an SDK call or callback wait.
Callback-driven forward Miles calls are refused before command mutation.

Limits are explicit: 64 distinct functions per callback type per session;
owned sample and stream registrations; no borrowed-sample EOS registration.
This is not a general native-SDK equivalence claim. Retirement depends on the
selected DLL's inspected timer/mixer callback paths holding the same native
mutex as release/close, with mutex protection enabled. Other SDK versions and
unexposed concurrent native APIs require separate qualification.

```powershell
miles-pipe-probe.exe C:/test/miles-host.exe C:/test/Mss32.dll --eos-callbacks
```

The native VS2013 host and x64 probe build with `/W4 /WX` and no diagnostics.
The generated-WAV run against the original DLL observed:

- 65 alternating sample callback registrations with exact prior returns;
- synchronous sample completion on the engine worker with exact proxy identity;
- genuine callback reset after sample rebind and no callback after unregister;
- EOS initialization before file registration, then real reverse file operations;
- 65 repeated stream registrations and a real stream end callback;
- native sample release, stream close and SDK shutdown, plus the prior lock checks.

All nine expected markers appeared and exit status was zero. **The strict run
failed** its unchanged empty-stderr rule on four ALSA control-enumeration
warnings. The earlier failed run is preserved: its new global sequence check
incorrectly treated a file-consumption ACK as a new request. Excluding that ACK
from new-request intake corrected the observed mixed file/EOS failure; the
existing mapper still validates the original-ID ACK in full.

| Artifact | SHA-256 |
| --- | --- |
| x86 host | `da8c857820cb1c7c3f7f31570a4258175966449715cff19e4c89be462950de01` |
| x64 EOS probe | `8f35bb495c778747662395d5f922644ac2bb8438d9f5900a8d9df38ee4eba62a` |
| original x86 DLL | `0785b5f2aa81e68c41778bea1aa92ed545e2392827d90745838af2b14f6954fe` |

Portable protocol checks, including wrong generation/function/envelope and
forward-message rejection, run without a vendor SDK:

```sh
c++ -std=c++11 -Wall -Wextra -Werror -fsanitize=address,undefined tests/eos_protocol.cpp src/eos/eos_protocol.cpp src/wire/codec.cpp -o eos-protocol
./eos-protocol
```

All 298 checks pass; omitting generation comparison makes them fail. These
protocol checks also run in CI. They do not substitute for the genuine-DLL
runtime, actual Audio callbacks or gameplay acceptance.

### Paired session close

`Session::close()` follows SDK shutdown, then requires an exact final reply,
helper exit zero, callback transport drain and engine/control-thread joins before
releasing the client owners. A disconnect by itself is not success. Open files,
unsettled callbacks, outstanding uploads and extra or partial incoming frames
refuse closure. Failure retains uncertain roots until process termination.
The helper retains its vendor module until process exit; this is not a DLL
unload implementation. Producer-stop reasoning is scoped to the selected DLL's
inspected timer, mixer and stream-service paths, not arbitrary SDK plugins.

```powershell
miles-pipe-probe.exe C:/test/miles-host.exe C:/test/Mss32.dll --paired-close
```

The VS2013 `/W4 /WX` host and probe built without diagnostics. The standalone
original-DLL run repeated file/EOS operations, then SDK shutdown and paired close.
All ten expected markers appeared, including session destruction and expiry of
both code-lifetime pins, with exit zero. **The strict run failed** its empty-stderr
rule on the same four ALSA enumeration warnings. Desktop defaults were unchanged
and the owned sink and Wine server were cleaned up.

| Artifact | SHA-256 |
| --- | --- |
| x86 paired-close host | `b0edf55400869951883e2a483abfbfcba6f27e562c0e27ffb6eaa0630e5533ac` |
| x64 paired-close probe | `f71678127eb59c41ee7d6e5fce94145e4b9f224ef1f8f8187a715e3eac931c98` |

The global engine bootstrap remains installed until test-process exit. This
does not test real Audio/global ExitChain shutdown, native-device fidelity or a
linked x64 game.

## Relative module path and actual game startup — 2026-09-30

The first Debug-x64 game startup failed before creating a window: the host
compared the configured relative `helper-x86/Mss32.dll` with the loader's absolute
path. Both paths now pass through `GetFullPathNameA` before the full-path
comparison. Empty, failed, truncated and mismatching paths remain rejected.
The client and host also preserve standard-exception diagnostics while retaining
the existing terminal failure policy and callback ownership.

VS2013 rebuilt the x86 host and x64 pipe archive with `/W4 /WX` and relinked
the actual development game with zero warnings/errors. With the same relative
configuration and original DLL under GE-Proton11-7, the game passed the previous
handshake failure, reported `Audio: Finished initializing`, loaded DPVS and
created its window. Game SHA-256:
`1c0e27caa4a45af9fc0d28f67ccac01a0a82c655b14a5a21eb9af0c4d86dd0a5`.
Host SHA-256: `dcaf9cb18a30a1b5e82ec759a7ac5851a2dfc15e63d2b4cdf8de26fc78bcd223`.

**The overall startup/close check failed.** No login screen was verified, and
the ordinary window-close request timed out. The helper also created an unwanted
console window. The missing Bink DLL warning remains an unresolved media blocker.
This private null-sink/headless run involved no login or gameplay and establishes
no native-device or audio-fidelity claim. The original failed run is retained.

### Helper window and startup follow-up

The private worker is now launched with `CREATE_NO_WINDOW` while remaining
suspended until job assignment. The next native host/pipe rebuild and game
relink again had zero warnings/errors. A bounded Proton run confirmed only the
game window was visible, verified game focus, and reached Audio initialization.
The game SHA-256 was
`410df8ac7bf95b32bcc6a465a8d6ab62491183ec013743b0eafddc4d1757f894`.

The earlier Alt+F4 check was not a valid teardown test: SWG deliberately ignores
that key combination in `Os.cpp`. The follow-up used a graceful window-manager
close request instead; it also timed out, so shutdown remains unqualified.
No login screen was verified. Debugger attachment was denied by the host OS;
startup localization proceeds using the engine's existing install-timing logs.
Neither the path correction nor console suppression is a full startup pass.

### Command transport sizing

The live client/host command channels no longer force 23/17-byte I/O segments.
They use the existing Endpoint default (1 MiB per I/O operation); wire frame
limits, partial-I/O handling, byte budgets and deadlines are unchanged. Small
segments remain supported by Endpoint for explicit fragmentation tests.

Native host/pipe rebuild and game relink had zero warnings/errors. With identical
assets, configuration, original DLL and install-timing logging under Proton:

| Initializer | Forced tiny segments | Default segments |
| --- | ---: | ---: |
| CollisionCallbacks | 6.0100 s | 0.0216 s |
| ShipTargetAppearanceManager | 4.6186 s | 0.0234 s |
| SpaceDeath | 16.3579 s | 0.0200 s |

These are single diagnostic-run measurements, not general performance or fidelity
claims. The new game (`39fb1e2db4270e26371d85751f470c3686abf1b9ced21eae4a0809799d377a46`)
completed Game/UI installation but **startup still failed** at shader compilation:
`vertex_program/2d_texture.vsh`, unexpected `KW_POINT`. The loader log identifies
native `d3dx9_43.dll` calling Wine's built-in `d3dcompiler_43.dll`. No shader
semantics, media feature or asset was changed to bypass this failure.
