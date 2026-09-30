# Native UI pool alignment regression

This test compiles the checkout's actual `UiMemoryBlockManager.cpp`, `UiReport.cpp`, and `UILowerString.cpp` with the real UI headers and bundled STLport. It allocates raw storage, checks addresses/strides/free-list reuse, and frees it. It never constructs a UI object in possibly misaligned storage and never executes the old misaligned allocator as a negative control.

Run in Windows with Python 3 and Visual Studio 2013 (v120), after building the matching real foundation/core and STLport dependencies. Run separately for Debug/Release and Win32/x64:

```text
python tools/test-ui-memory/run.py --vcvars "C:/Program Files (x86)/Microsoft Visual Studio 12.0/VC/vcvarsall.bat" --inputs C:/test-inputs/Debug-x64.json --configuration Debug --platform x64 --out C:/test-results/ui-Debug-x64
```

`--source` defaults to this checkout. `--out` must not exist. The input JSON contains:

```json
{
  "defines": ["WIN32", "_WINDOWS", "_DEBUG"],
  "include_dirs": ["../checkout/src/external/3rd/library/ui/src/shared"],
  "link_inputs": ["../build/actual-matching-stlport.lib"]
}
```

**The example is a schema, not a sufficient build configuration.** Populate `defines` and `include_dirs` from the actual UI project's evaluated configuration, including UI core/header directories, bundled STLport, and shared project headers. Supply the genuine matching object/library closure in link order: the allocator/core/foundation implementations (including custom new/delete), UI report dependencies, and STLport. All `link_inputs` must be existing files; paths may be absolute or relative to the input JSON. System Windows libraries are supplied by the selected toolchain. Do not supply fake implementations, CRT substitutions for the project's allocator, or dependencies from an incompatible configuration. The runner uses `/MTd` for Debug, `/MT` for Release, `/EHsc`, and `/Zc:wchar_t-`.

There is deliberately no automatic dependency build yet. The inputs must be prepared from a real build; this is not a one-command clean-checkout CI job. The regression is a native test command with explicit dependencies, not an arithmetic-only replacement for the allocator.

Success requires the correct PE architecture, zero build and runtime exits, exactly `PASS: 3129 UI alignment checks`, and a link-map entry binding `allocMem` to the freshly compiled manager. Only the existing numeric `MM::remove` summary is additionally allowed in Debug. Build commands, logs, source/link-input SHA-256 digests, and results are retained in the output directory. Include-directory content and toolchain/system libraries are not fully hashed: preserve their source/build identity with the evidence packet.

The 3,129 checks cover 1,024 small size calculations (three properties each), then actual four-block allocation, stride, alignment, and reuse for the native sizes of `UIButton`, `UIPage`, and `UIText` (19 checks each). The Win32 arithmetic check requires the unchanged four-byte stride formula. Sizes and alignments come from the actual headers, not copied class layouts. No payload is read or written.

This establishes natural eight-byte alignment for the tested x64 classes and size-header accesses; it does not establish support for arbitrary 16-byte-aligned subclasses, UI lifecycle correctness, pool-size arithmetic overflow, or the separate `int` free-list-key limit. Those remain outside this one-line alignment change.
