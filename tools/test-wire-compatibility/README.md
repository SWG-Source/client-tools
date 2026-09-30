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
2. **Forty existing runtime fixtures** (`fixtures.cpp`), checked against legacy32 bytes:
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

7. **LoginClusterStatus** (`login_cluster.cpp`, nine checks): two distinct galaxies, each
   encoded against literal stock Win32 bytes and independently decoded from those literals;
   both together through the real message constructors, `AutoByteStream`, and `AutoArray`;
   a following sentinel to check the exact iterator boundary; and an empty list in both directions.
   Every decoded record field is compared. Cases include population `-1`/`INT32_MAX`, timezone
   `INT32_MIN`/`-3600`, `UINT32_MAX`/high-bit IDs and limits, ports `0`/`65535`, enum endpoints,
   two different nonempty addresses, and opposite boolean flags. No large counts or random data.
   The base constructor shim omits the command CRC, so the literal message prefix is
   `01 00` (one registered member) + a four-byte unsigned array count. This is **bounded
   serializer coverage, not a complete stock packet**. The stock source at `94945103` independently
   defines the field order and widths; its real Win32 serializers were compiled and run against
   the same hand-transcribed literals. Expected data is never derived from candidate roundtrips.

The run succeeds only if the width check compiles cleanly, the fixtures exit 0, no line reports
`FAIL` or `NOT RUN`, and exactly 58 common runtime checks plus 7 Win64-only checks report
`PASS` (59/66 totals including the compile-time width check). Win32 requires exactly the two
known `SKIP` notices. Stock Win32 requires one timestamp `SKIP` and the one known `ArchiveCount`
`ABSENT` notice, yielding 56/56. Unknown or duplicate skip/absence notices fail; only that exact
known helper absence can reduce the expectation, and `--require-current-coverage` forbids it. An exit code
alone is not trusted: it cannot distinguish "all passed" from "the fixtures never ran".

The code under test is the checkout's own: Archive, AutoDelta containers, NetworkId,
PlayerQuestData, the mission-list and login-cluster serializers, StringId and Unicode archives.

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
  - `GameNetworkMessage` allows only `LoginClusterStatus` construction and registers no command
    CRC; `MessageDispatch::MessageBase` initializes type to zero. This lets the real
    `LoginClusterStatus.cpp` register its array and unpack it. No pack/unpack/put/get is stubbed.
    `ChatOnRequestLog` is still never constructed; only `ChatLogEntry` serializers are used;
  - `MemoryBlockManager` as plain heap allocation of the pool's element size (kept in the
    opaque `m_allocator` field), with `ControllerMessageFactory` registration and `ExitChain::add`
    as no-ops. The fixtures call each message's `install()` to create its pool.

`__MINGW_USE_VC2005_COMPAT` is set because MinGW's i686 headers otherwise define
`_USE_32BIT_TIME_T` implicitly, which MSVC (VS2005 and later) does not. Win32 runs pass
`_USE_32BIT_TIME_T` as the stock projects do; use `--no-32bit-time` for a checkout whose
projects no longer define it.

## Results (2026-09-30)

With the nine LoginClusterStatus checks: stock `94945103` Win32 **56/56**, detached
`d0fea5bc7` plus these test changes Win32 **59/59**, Win64 **66/66**. An encode-only private
negative control writes `m_onlinePlayerLimit` as native `size_t`: Win32 passes **59/59**,
Win64 fails exactly the two element encodings and the containing message encoding. No
production file is changed by the test harness. Decode inputs remain fixed stock literals,
so the negative control cannot introduce large counts or string allocations.

The available local PR21 snapshot retains the stock LoginClusterStatus widths and signedness.
Its optional admin/secret decode checks remaining bytes in the entire iterator, which can
consume bytes from the next galaxy when those flags are absent per record. That is a record
boundary issue, not evidence of a LoginClusterStatus signedness/width change. These fixtures
cover the original 14-field format; they do not establish compatibility with a 12-field variant.

Historical results below predate the new login-cluster fixtures and were not rerun as a matrix:

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
messages and the remaining `Archive` call sites need their own fixtures. LoginClusterStatus
coverage includes its 14-field records, list count, and registration under the base shim; it
excludes the command CRC, real message base, packet framing, dispatch, and live login. Its
oracle is a manual transcription from original client-tools `94945103`, confirmed by running
that original source on Win32. Other literals derive from src#35. None is a captured packet trace.

## Automated builds

`.github/workflows/wire-compatibility.yml` checks out the submitted PR head SHA,
not a fresh clone of upstream. Its Ubuntu 24.04 Win32/Win64 matrix installs clang,
MinGW-w64 and Wine, builds the fixtures from that checkout, and runs the same
strict pass/skip checks described above. It retains the source/tool versions,
console log and newly linked fixture executable. No prebuilt game or fixture
binary is downloaded. Fork workflows may need maintainer approval; absence of a
run is not a passing result. This is a head-revision check, not a simulated merge
or a native MSVC/gameplay test.

Use `--artifacts PATH` to retain the linked executable, exact compiler/linker/Wine commands,
raw command diagnostics and exits (`commands-<bits>.jsonl`), actual compiled translation-unit
paths and hashes, and a hash manifest of the staged shared sources after syntax shims.
These artifacts include syntax-only units separately; source presence alone is not execution
coverage. Use a fresh artifact directory per run. On new Wine WoW64
installations which reject pure 32-bit prefixes, use `--wine-arch win64` and point
`WINEPREFIX32` at an initialized 64-bit prefix. This only selects the runtime
prefix: `--bits 32` still compiles a Win32 executable. GitHub's Ubuntu Wine packages
use separate default prefixes for the two jobs.

CI adds `--require-current-coverage`: an `ABSENT` helper report is a failure,
so removing a checked helper cannot silently lower the CI pass requirement.
Stock-oracle comparisons omit that flag and retain the documented absent checks.
