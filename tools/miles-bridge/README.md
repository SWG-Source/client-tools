# Miles backend development

This directory holds the maintained implementation of the temporary Miles
process bridge. `src/api/ClientMiles.h` uses the selected Miles SDK's handle and
callback types. A future native x64 backend can replace the pipe backend at this
boundary without exposing IPC to game code.

The bridge is unfinished and is **not selected by SwgClient**. Building these
targets does not establish a playable x64 client or equivalent audio behavior.

## Build

Use Windows, Python 3, Visual Studio 2013 (v120), and the existing Miles SDK.
No SDK headers, import libraries, DLLs or game assets are bundled here.

```powershell
python build.py --target host --sdk C:/SDK/Miles/include --sdk-lib C:/SDK/Miles/lib/win/Mss32.lib --out C:/build/miles
python build.py --target pipe --sdk C:/SDK/Miles/include --out C:/build/miles
python build.py --target native --sdk C:/SDK/Miles/include --out C:/build/miles
python build.py --target engine-worker --engine-root C:/source/client-tools --out C:/build/miles
python build.py --target pipe-probe --sdk C:/SDK/Miles/include --engine-root C:/source/client-tools --out C:/build/miles
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

Choose exactly one client backend. The pipe and native archives implement the
same public names and must not be linked together. Both currently use the debug
static CRT for component development. The engine worker is a separate
translation unit with the legacy engine's STLport and `wchar_t` settings; its
interface passes only plain values and function pointers. Application
CRT/configuration integration is a separate build step.

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

Both EOS registrations remain missing. The sample-file setters now execute
through owned uploads, as tested below. There are no success stubs.

The engine file worker now links with the pipe adapter in the development
probe, but its execution remains unqualified. Audio exposes separate admitted file callbacks
which reuse its file operations without installing TLS again; its existing
direct-Miles registration is unchanged. Selecting those callbacks and supplying
Audio's image extents still need integration. The paired channel refuses normal session close until its
shutdown and callback lifetime protocol is implemented. Full media-format
qualification and callback quiescence remain open. Do not use this helper as the
game's audio backend yet.

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
