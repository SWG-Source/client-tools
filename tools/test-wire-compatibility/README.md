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
2. **Forty runtime fixtures** (`fixtures.cpp`), each checked against literal legacy32 bytes:
   - quest packed map and `AutoDeltaPackedMap<int, unsigned long>` (encode and decode);
   - `AutoDeltaPackedMap` with `NetworkId` keys or values and with `Unicode::String` values
     (encode and decode);
   - `AutoDeltaVector` counter wrap; `AutoDeltaMap` behind across 2^31, wrap through zero and
     repeated/replayed deltas; `AutoDeltaSet` behind across 2^31 and wrap through zero;
     `AutoDeltaQueue` unsigned skip clamp, wrap and duplicate delta (mirrored verbatim from
     SWG-Source/src#35 7ace7d51). Every delta check also requires the delta to be fully consumed;
   - timestamps: `ImageDesignChangeMessage` and `BuffBuilderChangeMessage` `startingTime` and
     `ChatLogEntry` `m_time`, each through its real pack/unpack at `INT32_MIN`, `-1`, `0` and
     `INT32_MAX`. The encoding may differ from the `0` encoding only in the four little-endian
     bytes at the legacy offset, and the decoded value is compared as signed: legacy `time_t`
     was signed 32-bit, so `FF FF FF FF` must decode as `-1`;
   - out-of-range timestamps (Win64 only, 6 checks): `INT32_MIN - 1` and `INT32_MAX + 1` must make
     `NetworkMessageTimestamp::fromTime` throw `std::out_of_range`, caught explicitly in-process:
     any other exception, no exception, or a crash is a failure. A rejected setter must keep the
     previous value. On Win32 `time_t` is 32-bit and no out-of-range value exists; the run must
     print one `SKIP` line instead, and nothing is counted as a pass;
   - `MessageQueueMissionListResponse` (empty header, two entries, decode with no trailing bytes).
3. **Container-count helper** (`ArchiveCount::fromSize<Count>`): `UINT32_MAX` and `INT32_MAX` are
   representable in unsigned and signed counts; `INT32_MAX + 1` into a signed count and (Win64 only)
   `UINT32_MAX + 1` into an unsigned count throw `std::out_of_range`. These test the helper only:
   routing every call site through it is established by review, because an out-of-range size needs
   a container with more than 2^32 elements. A checkout without the helper (e.g. the stock oracle)
   prints `ABSENT:` naming the checks; they are deducted and reported, never counted as passes.
4. **String length boundary** (every checkout): 65,534-byte and 65,535-byte strings through the real
   encoder must produce the legacy short (`FE FF`) and long (`FF FF` + 32-bit length) headers and
   decode with no trailing bytes.
5. **Count-site compile coverage** (`count_sites.cpp`, mirrored verbatim from SWG-Source/src#35
   30cf4531): instantiates every generic count-writing overload changed to use `ArchiveCount`. It is
   compiled, never called; it is not an oversized-container test.
6. **Per-message counts**: `ImageDesignChangeMessage` morph/index counts and `BuffBuilderChangeMessage`
   component count with non-empty maps, compared byte for byte from the first count onward (legacy
   offsets 66 and 29) and decoded with no trailing bytes. The other changed writers
   (`DroidCommandProgrammingMessage`, `MessageQueueCraftExperiment`,
   `MessageQueueDraftSlotsDataArchive`, `CustomerServiceCategoryArchive`) are compiled only
   (`SYNTAX_ONLY` in `run.py`); a compile failure fails the run.

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
- `fatal.cpp`: `Fatal` prints and aborts.
- `mission_glue.cpp` stubs runtime infrastructure only, never a serializer:
  - the trivial value-holder members of `MessageQueueMissionListResponse` (its .cpp registers a
    controller-message factory) and the `MessageQueue::Data` base;
  - the display-only localization lookup (aborts if reached);
  - `GameNetworkMessage` and `MessageDispatch::MessageBase` constructors (abort if reached; the
    `ChatOnRequestLog` message is never constructed, only `ChatLogEntry`'s serializers are used);
  - `MemoryBlockManager` as plain heap allocation of the pool's element size (kept in the
    opaque `m_allocator` field), with `ControllerMessageFactory` registration and `ExitChain::add`
    as no-ops. The fixtures call each message's `install()` to create its pool.

`__MINGW_USE_VC2005_COMPAT` is set because MinGW's i686 headers otherwise define
`_USE_32BIT_TIME_T` implicitly, which MSVC (VS2005 and later) does not. Win32 runs pass
`_USE_32BIT_TIME_T` as the stock projects do; use `--no-32bit-time` for a checkout whose
projects no longer define it.

## Results (2026-09-29)

| Checkout | Win32 | Win64 |
|---|---|---|
| SWG-Source/client-tools `master` 94945103 (legacy oracle on Win32) | 47/47 (out-of-range skipped; 3 count-helper checks absent) | fails: width check, 4 fixtures, then `ReadException` |
| this branch | 50/50 (out-of-range skipped) | 57/57 |
| a65d8032 (`FATAL` helper) | 41/41 | not OK: the process aborts; a crash is not a pass |
| 44652cba (unchecked narrowing) | 41/41 | 6 fail: silently truncated, previous value lost |
| e4e6b7f1 timestamp types (`uint32_t`) | 7 fail: `INT32_MIN`/`-1` decode unsigned | same |

The legacy counters were `size_t`, which is 32-bit unsigned on Win32, so delta arithmetic is
modulo 2^32; the legacy timestamps were `time_t`, which is 32-bit signed. Width alone is not
enough: signed counters (before 1e62bab4) fail the map boundary fixtures, and unsigned
timestamps (before 579db9f6) decode negative values as large positive ones. 64-bit builds keep
`time_t` internally and convert once, with a range check, in
`sharedNetworkMessages/NetworkMessageTimestamp.h`, identical to SWG-Source/src#35 4889e6aa. It throws
`std::out_of_range`; this establishes the shared helper's policy, not graceful recovery at callers.

## Limits

Count checks run before the first byte of each writer, but nested serializers
(`MessageQueueDraftSlotsDataArchive`, `CustomerServiceCategoryArchive` subcategories) are called
after their caller has written bytes: a rejection there cannot roll back the caller's partial
output. Count representability also does not make large payloads safe; byte-buffer overflow is a
separate repair.

This is not MSVC and not a live client. It establishes the listed byte layouts on these two ABIs
only: not every message, not gameplay, not a connection to a server. Other serialized
messages are not covered here: `LoginClusterStatus` and the remaining `Archive` call sites need
their own fixtures. The oracle is a manual transcription of the legacy32 format from src#35,
not a captured packet trace.

## Automated builds

`.github/workflows/wire-compatibility.yml` checks out the submitted PR head SHA,
not a fresh clone of upstream. Its Ubuntu 24.04 Win32/Win64 matrix installs clang,
MinGW-w64 and Wine, builds the fixtures from that checkout, and runs the same
strict pass/skip checks described above. It retains the source/tool versions,
console log and newly linked fixture executable. No prebuilt game or fixture
binary is downloaded. Fork workflows may need maintainer approval; absence of a
run is not a passing result. This is a head-revision check, not a simulated merge
or a native MSVC/gameplay test.

Use `--artifacts PATH` to retain the linked executable locally. On new Wine WoW64
installations which reject pure 32-bit prefixes, use `--wine-arch win64` and point
`WINEPREFIX32` at an initialized 64-bit prefix. This only selects the runtime
prefix: `--bits 32` still compiles a Win32 executable. GitHub's Ubuntu Wine packages
use separate default prefixes for the two jobs.
