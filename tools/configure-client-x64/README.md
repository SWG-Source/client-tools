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

## Legacy shader compiler at runtime

The x64 renderer selects `D3DXSHADER_USE_LEGACY_D3DX9_31_DLL` for HLSL vertex
shaders. The June 2010 compiler rejects the original shader include's `point`
identifier under its newer HLSL grammar. The documented legacy selector compiles
the unchanged assets with the Direct3D 9 compiler. Shader profiles, macros and
include handling are retained; Win32 still uses flags zero. Pixel shaders use
precompiled bytecode and the separate vertex assembly path is unchanged.

An x64 runtime needs Microsoft's original AMD64 `d3dx9_31.dll` in addition to
the newer SDK dependencies, available in `OCT2006_d3dx9_31_x64.cab` from the
DirectX redistributable. SDK/vendor DLLs are not bundled here. See Microsoft's
[D3DXSHADER flags documentation](https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxshader-flags).

Native VS2013 builds completed for Debug-x64 gl05/gl06/gl07 and Release-Win32
gl07 with zero errors (5/2/5/4 warnings respectively: existing output-name,
size-conversion and legacy PDB warnings remain). A diagnostic run of the actual
Debug-x64 client under GE-Proton11-7, using original SWGSource v3.0 assets and
the signed native compiler, observed successful compilation and GPU creation
for `2d_texture.vsh`, `2d.vsh` and `ui.vsh`. Both the Wine builtin and signed
modern compiler had rejected the same `point` syntax. The diagnostics are not
part of the renderer source.

This closes the observed syntax failure only. The initial X11 window capture
was black, but a subsequent compositor capture proved that the client was
rendering a splash screen and a resolution warning. The X11 capture was not a
valid rendering sensor. With a virtual desktop large enough for the client
window and its borders, the rebuilt product (without trace instrumentation)
reached the original login screen under Proton. No credentials or server
connection were used. Ordinary-close completion and recurring sample-allocation
warnings remain unresolved; there is no full runtime acceptance pass.

The stock Win32 renderer embeds compiler 5.04.00.3900; `_31` is a different
version. No gameplay, shader-bytecode or visual-equivalence pass is claimed.
Representative shader/rendering comparison and native Windows runtime remain
required before this path is qualified.
