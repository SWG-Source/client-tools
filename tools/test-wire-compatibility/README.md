# Legacy 32-bit wire fixtures (client)

Client-tools port of `tools/test-wire-compatibility` from SWG-Source/src#35. It checks that
this checkout's serializers produce and accept the byte layout that the stock 32-bit server and
existing Win32 clients use, when compiled as Win32 **and** as Win64.

```
python3 tools/test-wire-compatibility/run.py --bits 32
python3 tools/test-wire-compatibility/run.py --bits 64
```

Requires clang (15+), MinGW-w64 (`g++-mingw-w64-i686-posix`, `g++-mingw-w64-x86-64-posix`) and
Wine with 32- and 64-bit support. Tested on Ubuntu 24.04: clang 18, MinGW-w64 GCC 13, Wine 9.0.
Set `WINEPREFIX32` / `WINEPREFIX64` to choose prefixes (default `~/.wine-swg32`, `~/.wine-swg64`).
`--root <checkout>` tests another client-tools checkout; the checkout is never modified.

## What it runs

1. **Wire-width assertions** (`wire_types.cpp`, compile time): the time fields of
   `ImageDesignChangeMessage`, `BuffBuilderChangeMessage` and `ChatLogEntry` are 4 bytes.
2. **Twenty-eight runtime fixtures** (`fixtures.cpp`), each checked against literal legacy32 bytes:
   - quest packed map and `AutoDeltaPackedMap<int, unsigned long>` (encode and decode);
   - `AutoDeltaPackedMap` with `NetworkId` keys or values and with `Unicode::String` values
     (encode and decode);
   - `AutoDeltaVector` counter wrap; `AutoDeltaMap` behind across 2^31, wrap through zero and
     repeated/replayed deltas; `AutoDeltaSet` behind across 2^31 and wrap through zero;
     `AutoDeltaQueue` unsigned skip clamp, wrap and duplicate delta (mirrored verbatim from
     SWG-Source/src#35 7ace7d51);
   - `ChatLogEntry` timestamp through its real serializer (encode and decode);
   - `MessageQueueMissionListResponse` (empty header, two entries, decode with no trailing bytes).

The run succeeds only if the width check compiles cleanly, the fixtures exit 0, no line reports
`FAIL` or `NOT RUN`, and exactly `EXPECTED_RUNTIME_PASSES` checks report `PASS`. An exit code
alone is not trusted: it cannot distinguish "all passed" from "the fixtures never ran".

The code under test is the checkout's own: Archive, AutoDelta containers, NetworkId,
PlayerQuestData, the mission-list serializers, StringId and Unicode archives.

## How it builds

clang compiles in MSVC-compatibility mode against MinGW-w64 headers; MinGW-w64 links; Wine runs
the PE binary. The runner copies the shared libraries to a temporary tree and applies
syntax-only edits a conforming compiler needs (missing `template<>` / `typename`, a friend
declaration's namespace lookup, a duplicate `finite()` declaration, one include-case alias).
None of these edits changes a type, value or serialized byte.

Replaced, and not under test:

- `shim/sharedFoundation/FirstSharedFoundation.h` reproduces `FoundationTypesWin32.h` exactly
  (`uint32` = `unsigned long`, `int64` = `__int64`) plus assertion macros, without the VC2013 STL
  and FloatMath headers.
- `shim/StlForwardDeclaration.h` uses the real standard containers instead of forward
  declarations of VC2013 internals.
- `fatal.cpp`: `Fatal` prints and aborts. `mission_glue.cpp`: the trivial value-holder members of
  `MessageQueueMissionListResponse` (its .cpp registers a controller-message factory), the
  `MessageQueue::Data` base, and the display-only localization lookup.

`__MINGW_USE_VC2005_COMPAT` is set because MinGW's i686 headers otherwise define
`_USE_32BIT_TIME_T` implicitly, which MSVC (VS2005 and later) does not. Win32 runs pass
`_USE_32BIT_TIME_T` as the stock projects do; use `--no-32bit-time` for a checkout whose
projects no longer define it.

## Results (2026-09-29)

| Checkout | Win32 | Win64 |
|---|---|---|
| SWG-Source/client-tools `master` 94945103 (legacy oracle on Win32) | 29/29 | fails: width check, 4 fixtures, then `ReadException` |
| this branch | 29/29 | 29/29 |
| SWG-Source/client-tools#21 head 46f6003a (`--no-32bit-time`) | fails: time fields are 8 bytes (width check and `ChatLogEntry` bytes) | fails: 11 checks, then `bad_alloc` |

The legacy counters were `size_t`, which is 32-bit unsigned on Win32, so delta arithmetic is
modulo 2^32. Signed `int32_t` counters (this branch before 1e62bab4) fail the map boundary
fixtures: the skip count goes negative and the decoder reads past the delta. The server
counterpart of this repair is SWG-Source/src#35 7ace7d51; the map and queue fixtures are
shared byte for byte.

## Limits

This is not MSVC and not a live client. It establishes the listed byte layouts on these two ABIs
only: not every message, not gameplay, not a connection to a server. Other serialized
messages are not covered here: `ImageDesignChangeMessage` and `BuffBuilderChangeMessage`
timestamps are checked for width only, and `LoginClusterStatus` and the remaining `Archive`
call sites need their own fixtures. The oracle is a manual transcription of the legacy32 format from src#35,
not a captured packet trace.
