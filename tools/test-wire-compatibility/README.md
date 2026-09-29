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
2. **Nine runtime fixtures** (`fixtures.cpp`), each checked against literal legacy32 bytes:
   quest packed map (encode and decode), `AutoDeltaPackedMap<int, unsigned long>` (encode and
   decode), `AutoDeltaVector` baseline counter wrap (decode and re-encode), and
   `MessageQueueMissionListResponse` (empty header, two entries, decode with no trailing bytes).

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
| SWG-Source/client-tools `master` 94945103 | 10/10 | 0/10: 7 fail, then `ReadException` |
| this branch | 10/10 | 10/10 |
| SWG-Source/client-tools#21 head 46f6003a (`--no-32bit-time`) | 9/10: time fields are 8 bytes | 2/10: 9 fail, then `bad_alloc` |

## Limits

This is not MSVC and not a live client. It establishes the listed byte layouts on these two ABIs
only: not every message, not gameplay, not a connection to a server. Other serialized
messages are not covered here; `LoginClusterStatus` and the remaining `Archive` call sites need
their own fixtures. The oracle is a manual transcription of the legacy32 format from src#35,
not a captured packet trace.
