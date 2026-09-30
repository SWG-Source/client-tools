# Client x64 configuration generator

Run `python tools/configure-client-x64/generate.py --base <pre-generation-commit>` at any working directory.
`--check` regenerates expected contents from the explicit Git base and exits nonzero if any checked-in project/solution differs. Generation overwrites those configuration files from that base; use an isolated checkout. The script discovers the
SwgClient solution dependency closure, resolves Windows paths case-insensitively,
and adds **Debug and Release x64 only**. Existing Win32 XML is retained; run
`python tools/configure-client-x64/check.py --base <pre-generation-commit>` to
compare its semantics and the original solution mappings. This static check is
not a substitute for native MSBuild evaluation and builds.

DPVS already provides reviewed Debug/Release x64 configurations, and is left
alone. Optimized is deliberately not introduced: the legacy solution maps DPVS
Optimized to IntelCPP, for which there is no x64 configuration. Existing Win32
Optimized mappings remain unchanged.

Generated x64 configurations retain the v120 toolset, inherited per-user props,
source selection, runtime libraries and feature dependencies. They remove
`_USE_32BIT_TIME_T`, set MachineX64, and select the matching architecture for
**built project** library directories. Third-party library directories are not
rewritten: unavailable x64 vendor SDKs remain real link blockers.

Direct3D post-build copy commands deploy only into `dev/x64`, create that directory when absent, and quote paths. Their original Win32 counterparts remain unchanged.

The shared property sheet keeps x64 outputs in
`src/compile/x64/<project>/<configuration>/`, with compiler/PCH/PDB/resource
intermediates below `obj/`. It affects x64 only. The DPVS project keeps its own
reviewed platform-specific output settings.

For the June 2010 DirectX SDK, pass `/p:DXSDK_DIR=C:\SDKs\DXSDK` (or set that
environment variable). The x64 sheet prepends its `Include` and `Lib\x64`
directories. The SDK must be installed/extracted separately; no SDK binaries are
bundled. Win32 SDK settings remain as configured previously. A missing SDK is
not silently replaced with repository Win32 libraries.

Native validation must inspect effective settings, build Win32 Release as a
regression baseline, then build Debug/Release x64. Compiler and vendor failures
are evidence for separate fixes, not permission to remove features.
