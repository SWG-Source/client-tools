# Native DPVS regression checks

Build the checkout's DPVS project and every test executable from source. No game
assets, DirectX SDK, full SwgClient project or cached binaries are required.

## Prerequisites and invocation

Use **native Windows**, **Visual Studio 2013 Update 5 C++ tools (v120)** including
both x86 and x64 compilers, **MSBuild 12**, and **Python 3.8 or later**. The runner
checks `_MSC_VER == 1800` rather than silently accepting another compiler. The
result remains tied to the recorded compiler version and build flags.

From the repository root:

```powershell
python tools/test-dpvs/run.py --out C:\dpvs-results\run-001
```

The output directory must not exist. Each run copies production sources to a fresh
build directory, excludes prebuilt products, disables optional per-user property
imports, invokes `/t:Rebuild` on the real `dpvs.vcxproj`, and freshly compiles all
probes. The checkout is not modified. Use `--vcvars` and `--msbuild` to specify
nondefault VS2013 installation paths. `--source` selects another source checkout;
fixtures always come from the runner's checkout. This supports negative controls
without borrowing that checkout's test implementation.

To also rebuild stock Win32 Release and compare its DLL:

```powershell
python tools/test-dpvs/run.py --out C:\dpvs-results\run-002 --baseline C:\stock-client-tools
```

For the reviewed upstream oracle, prepare `C:\stock-client-tools` at
`949451032647e45e42c3aaef3f41b132c8af36e3`. The runner rebuilds it at the **same
absolute path** after all candidate tests, so path-dependent debug metadata is
controlled. It removes only its own generated source/build directories during
this optional step. The input checkouts are untouched. Both source inventories
and their SHA-256 hashes are saved. The baseline comparison requires whole-file
identity after these enumerated metadata normalizations:

- PE COFF, export and debug timestamps;
- CodeView PDB GUID/age and `Win32`/`win32` path capitalization only;
- the single `DPVS_BUILD_TIME` string from `__DATE__ " " __TIME__`.

No other bytes are ignored. Raw section hashes and normalized field offsets are
reported. This is a Release DLL comparison, not a Debug identity claim.

The current GitHub-hosted Windows images do not provide this legacy toolchain;
see the [official installed software inventory](https://github.com/actions/runner-images/blob/main/images/windows/Windows2022-Readme.md).
This change supplies a local native entry point, **not an operating hosted CI
job**. A newer-toolchain run would be separate evidence, not a replacement for
the v120 numerical reference. No self-hosted runner is registered or exposed to
pull request code by these scripts.

## Acceptance and evidence

Every build, link and execution must return zero. Logs and explicit `.exit` files
are retained, including failed attempts. A failure stops the command and returns
nonzero; it cannot be converted into a pass by producing a partial log.

- **Win32/x64 × Debug/Release:** freshly build four DLLs and eight public-interface
  probe executables. Each DLL must pass exactly **192 stress queries** with the
  visible sentinel, balanced callbacks and no duplicate/invalid visible IDs,
  plus **64 fixed-cost occlusion frames** with the expected visible mask and one
  write each. The integration probes use Win32 **PC24** as before; these are not
  the PC64 numerical oracle.
- **Win32/x64 Release numerics:** two fresh probes per ABI include the checkout's
  actual `dpvsMath.cpp`, replacing its object while linking the other freshly
  built project objects. Win32 must report raw x87 control word `037f` and the
  assembly path; x64 must report the scalar path. The broad probe has **38,880
  records** (floor, raster, min/max and dot), and the caller-shaped probe has
  **11,978** (**3,584 dot** and **8,394 raster**). Fixture order/input bits,
  classifications, return values, state preservation and verdicts are checked,
  not just record totals.
- The bounded C2 acceptance allows at most **15 min/max signed-zero differences**
  in the broad probe and at most **one negative dot difference of one ULP** in the
  caller probe. All caller sign/zero classifications and raster rows must match.
  Every difference is preserved in `results.json`. Those allowances describe
  these fixtures; they do not establish general numerical equivalence.

`source-sha256.json`, raw logs, exit codes, fresh DLLs/executables and
`results.json` remain under `--out`. Do not commit these build outputs. To recheck
a generated packet on any Python host:

```sh
python tools/test-dpvs/verify.py /path/to/run-002
python tools/test-dpvs/compare_dll.py /path/to/base.dll /path/to/candidate.dll
```

The standalone verifier requires successful exit records and complete logs; it
recomputes the optional DLL comparison. Supply the generated DLLs as well as
logs when sharing the packet. A text-only packet cannot establish DLL identity.

## Scope and remaining work

These fixtures originate in the DPVS C2 review experiments and are now built
from source by this entry point. They neither patch production code nor supply
library stubs. They do not perform a full x64 client build, native Direct3D FPU
trace or gameplay acceptance. The PC64 reference reproduces the measured legacy
setter-bug behavior; fixing that setter requires reassessing the numerical
repair. Out-of-range floating-point-to-integer conversions remain untested;
this runner does not make such conversions defined. Highest-set-bit search,
filler return and MMX cache-filler replacements have indirect runtime coverage,
not independent integer-path differential fixtures. Existing allocation-count
narrowing and capacity/accounting risks remain open.

The [x64 integration limits and next work](https://github.com/Akilleez-QA/client-tools/blob/review/client-x64-evidence/review/client-x64/C-dpvs/RESULTS.md)
include full-client linking, vendor dependencies and representative scenes.

For a commit-bound run on a machine without Git, export only the required trees
from a committed revision and transfer the tar and matching harness:

```sh
git archive --format=tar --output=dpvs-source.tar HEAD src/external/3rd/library/dpvs tools/test-dpvs
```

```powershell
python tools/test-dpvs/run.py --source-archive C:\dpvs-source.tar --out C:\dpvs-results\run-003 --baseline C:\stock-client-tools
```

This mode records the archive's Git commit metadata and SHA-256 in
`provenance.json`; it rejects a harness that differs from the archive. The archive
must come from the intended Git revision (the metadata is provenance, not a
cryptographic signature). Direct `--source` runs retain full file hashes but
record a null revision rather than inventing a commit ID for an arbitrary folder.
