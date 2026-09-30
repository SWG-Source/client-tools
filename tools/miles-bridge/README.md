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
- `buffer/`, `image/`, `upload/`: bounded image transfer for file-type/WAV queries.
- `metadata/`, `version/`: owned metadata and original module version queries.

`sources.json` lists each target's translation units. Relative includes preserve
which private implementation each target uses; do not add every source directory
to the include path to bypass missing includes.

## Remaining integration

The pipe adapter currently defines 56 of the 62 public operations. Missing are
`lock`, `unlock`, both EOS registrations, `set_sample_file`, and
`set_named_sample_file`. There are no success stubs for them.

The engine file worker is now a build target, but still needs to be linked and
exercised with the pipe adapter. Audio exposes separate admitted file callbacks
which reuse its file operations without installing TLS again; its existing
direct-Miles registration is unchanged. Selecting those callbacks and supplying
Audio's image extents still need integration. The paired channel refuses normal session close until its
shutdown and callback lifetime protocol is implemented. Setter/rebinding
ownership and callback quiescence remain open. Do not use this helper as the
game's audio backend yet.

The maintained sources consolidate the previously reviewed pipe composition113
and native70 adapter. Folder names changed; the public header is unchanged.
Unused earlier adapter implementations are excluded. Existing component test
records remain evidence for their original source versions; they do not prove
the newly combined executable behaves correctly.

## Build checkpoint — 2026-09-30

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
and link also passed in Debug and Release: zero errors, with 30 linker PDB
warnings in each run. An initial x64 compile against an older VM checkout failed
on an existing `Archive.h` narrowing warning; the final checks used the verified
current engine mirror. No warning was suppressed. These results do not qualify
the new callback path at runtime; current game registration still uses the
legacy callbacks.
