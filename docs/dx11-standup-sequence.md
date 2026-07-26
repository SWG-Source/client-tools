# DX11 standup sequence

How the Direct3D 11 backend gets built in this tree, ordered as individually
buildable commits. Companion to `dx11-port-plan.md`, which holds the
architecture decisions (A1..A20), the phase structure and the performance gates.

## Relationship to the prior attempt

A previous DX11 effort exists at `swgsais/client` branch `DX11` (June 2026), kept
as a read-only reference checkout outside this repository. It boots and draws the
world, and several of its findings are load-bearing and were independently
re-derived by this plan: `D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY` is
mandatory, `register(cN)` globals land in `$Globals` at byte offset 16*N so the
asset constant ABI survives untouched, input `: register(vN)` must be stripped,
`#pragma def` must be dropped, matrices must NOT be transposed, and input layouts
must be keyed on (vertex format x VS input signature).

Its mechanical layer is therefore worth transplanting. Its semantic layer is not:
the audit found retuned lighting constants, an inverted compare table, ~18 Gl_api
slots wired to empty bodies that DX9 implements for real, a shader-replacement
table matched by unanchored substring search, and per-frame diagnostics left in
release code. The sequence below takes the former and rebuilds the latter.

## Headline

Transplant the prior attempt's MECHANICAL layer only (device/swapchain, buffers, descriptor map, input layouts, draw dispatch, the compile chokepoint shape, the register(cN)->$Globals@16N constant-shadow insight, the reflected-t# SRV binding, the stencil-ref publish/re-bind) and none of its SEMANTIC layer (strstr shader-replacement tables, retuned lighting, inverted compare table, ~18 silent no-ops, per-frame diagnostics) — 19 individually-buildable commits in which the frame capture path (C4) and Metrics/DebugFlags/_DEBUG slots (C5) land BEFORE the first pixel (C11), exactly inverting the prior attempt's order, so every parity gate is falsifiable from commit 5 and the shader corpus — the real critical path — converts asset-by-asset behind a hard FATAL on any unmatched //asm program instead of a silent dropped draw.

## Prerequisites

- P0-A [HARD PREREQ - must land before C1, one commit, DX9+Headless only]: add `extern "C" uint32 GetGlApiStructSize()` to /e/SWG/64bit-server/client-tools/src/engine/client/library/clientGraphics/src/win32/Gl_dll.def + Graphics.cpp (validate right after GetApi, FATAL on mismatch) as a separate export, not a struct field, because Headless.cpp:41-60 blanket-fills Gl_api as void** over sizeof(Gl_api)/sizeof(void*); delete Gl_api::setDynamicIndexBufferSize (Gl_dll.def:184 + the unguarded call at Graphics.cpp:1695-1697 + Direct3d9.cpp:224/3545 - a guaranteed NULL deref with zero callers); delete Gl_metrics (include/public/clientGraphics/Graphics.def:37-103). Cheapest now because only gl05/06/07 and gl00 exist; after C1 it becomes a five-project lockstep edit. VERIFIED ALREADY DONE IN OUR TREE, no work: the lockBackBuffer rect assignment is already right<-x1/bottom<-y1 (Graphics.cpp:1049-1054) and the DX11 friend grants already exist (Texture.h:34-35,111-112 incl. Direct3d11_RenderTarget; Transform.h:28) - our tree is ahead of the reference on both.

- P0-B [HARD PREREQ - before C8]: promote Direct3d9_VertexShaderConstantRegisters.h / Direct3d9_PixelShaderConstantRegisters.h / Direct3d9_VertexShaderVertexRegisters.h into /e/SWG/64bit-server/client-tools/src/engine/client/library/clientGraphics/include/public/clientGraphics/ShaderConstantRegisters.h, values byte-identical, adding VSCR_c95 and a derived VSCR_CBUFFER_ROWS; leave forwarding shims in the Direct3d9 dir and repoint ShaderBuilder's cross-boundary relative includes (PixelShaderProgramView.cpp:12, VertexShaderProgram.cpp:14-15). These numbers are baked into shipped shader binaries; the prior tree kept four uncoordinated copies (Direct3d11_ConstantBuffers.cpp:33-41, Direct3d11_StaticShaderData.cpp:33-40, Direct3d11.cpp:978) and a single edit in one of them yields a wrong constant with no compile or runtime error.

- P0-C [HARD PREREQ - before C6 and C13]: run the three outstanding censuses over the 1,518 decoded PASS chunks - m_zCompare, m_stencilCompare, m_stencilCompareCounterClockwise through the swapped Compare table (Direct3d9_ShaderImplementationData.cpp:31-41 maps C_GreaterOrEqual->NOTEQUAL and C_NotEqual->GREATEREQUAL), and m_alphaBlendSource/Destination/Operation over every pass with alphaBlendEnable==false. Without the first, the compare-index-5/6 blocker cannot be closed with evidence (the prior tree inverted it knowingly and reverted the fix). Without the second, C6's alpha-fade blend variant set is a guess.

- P0-D [HARD PREREQ - before C12]: one gl05_r RenderDoc A/B on a known UI blit settling the one-to-one-UV / half-texel question. DX9 returns a texel-CENTRE span u0=0.5/w, u1=(w-1+0.5)/w i.e. (w-1)/w (Direct3d9.cpp:3552-3558); the prior tree returns flat 0,0,1,1 (Direct3d11.cpp:984-993) on an argument that conflates the D3D9 half-PIXEL rasterisation rule with texture coordinates. This touches every UI element and every post-process quad - settle it with a capture before any UI code is written against either convention.

- P0-E [HARD PREREQ - before C14]: one gl05_r capture on a long fogged surface fixing the `: FOG` interpolation modifier empirically (plan Gate B'). DX9's fixed-function fog unit and an SM4 interpolant are not necessarily the same and there is no way to recover the right answer after 90 shader pairs are converted.

- P0-F [HARD PREREQ - before C5]: settle the GfxStats delivery mechanism. Phase 0's wording ('counters published across Gl_api') contradicts A1's byte-stable Gl_api. Define the counter block in clientGraphics and hand its address to the backend via a SECOND out-of-band export alongside GetGlApiStructSize - same reason the size guard cannot be a struct field (Headless.cpp:41-60 void** fill).

- P0-G [HARD PREREQ - before C11]: partial corpus extraction only - the committed TreeFile resolver implementing the real order (priority descending, last-added-wins at equal priority per TreeFile.cpp:299-308, searchPath then searchTree then searchTOC, so sku0_client.toc at priority 0 beats all 64 priority-0 .tre), plus the UI/2D slice programs and every .inc they include, with per-file provenance. Pins the .inc source of truth: the winning pixel_shader_constants.inc already matches the PSCR enum, so the runtime override at Direct3d9_PixelShaderProgramData.cpp:82-105 is DELETED, not ported (the prior tree hardcoded it as a string literal at Direct3d11_ShaderCompiler.cpp:39-62).

- P0-H [PARALLEL]: full 391-program extraction, vertex_program/** into version control (it exists in no source tree; 495 .psh are on disk in the server repo at /e/SWG/64bit-server/x64-dx11-vanilla/serverdata/pixel_program but no .vsh or .inc are anywhere on E:), the 21 ILM_visuals reconciliations, and the frozen 152-entry permutation manifest. Needed for C19 and the Phase-3 tail, not for C1-C18.

- P0-I [PARALLEL - blocks MERGE, not development]: the six benchmark routes, the 36-run G0 baseline JSON, telemetry ring, ~20 QPC zones, DX9 GPU timestamp queries, peak VRAM / private bytes. G0 'blocks every DX11 merge' per the plan, which the transplant satisfies by stacking C1-C19 on a branch while the baseline is captured. Capture with the cfg pinned (rasterMajor, allowTearing=true, antiAlias off, gamma/brightness/contrast at identity) or captures are not reproducible - the prior tree's config toggles are the proof of that hazard.

- P0-J [PARALLEL - must land and re-baseline before any DX11 measurement]: the four DX9-side fixes, one commit and one re-baseline each. Highest attribution risk is the pointer-truncating sort keys (ShaderEffect.h:90-92, Direct3d9_StaticVertexBufferData.cpp:140-143, Direct3d9_DynamicVertexBufferData.cpp:260-263, Direct3d9_StaticShaderData.cpp:385-390) because the transplanted DX11 buffers already use monotonic IDs and would otherwise be credited with a DX9 fix.

- P0-K [PARALLEL, format must agree with C4]: the per-draw Gate-C reference-capture harness on gl05_r (shader template, effect, implementation/pass index, resolved .vsh/.psh, permutation key, full VS/PS constant blocks, RT after each draw) with currentTime, textureScroll (Direct3d9_StaticShaderData.cpp:914-926 recomputes scroll with modf on every apply), particles, camera and resolution all pinned. Its consumer is C4, so the artifact format must be agreed before C4 is written.

- P0-L [PARALLEL - CONDITIONAL PROMOTION to hard prereq for C1]: remove the two header leaks (BinkDLL.h:1 <d3d9.h>; CuiLayer.h:17 -> UICanvasGenerator.h:6 -> UIDirect3DTextureCanvas.h -> ddraw.h) and drop external/3rd/library/directx9/include from clientGraphics.vcxproj, clientUserInterface.vcxproj, swgClientUserInterface.vcxproj. Promotes to hard prereq if C1's first compile of Direct3d11 fails on a leaked <d3d9.h> reached through a clientGraphics public header, since A19 forbids putting a DX9 include dir on the DX11 project's path.

- P0-M [PARALLEL]: log which effects actually fail ShaderEffect validation today (ShaderEffect.cpp:136/166) to settle font.eft / defaulteffect.eft / a_skin.eft with data. Feeds Gate D in C11 and the plan's 'authoring 2.0 implementations would be a visual change in the opposite direction' decision.

- P0-N [VERIFY ONLY - already done]: /e/SWG/64bit-server/_client contains gl05_r.dll, gl06_r.dll, gl07_r.dll and NO gl11_r.dll, so the Adria PBR shadow DLL is already archived out. Re-verify immediately before the first gl11 deploy - it is untracked by x64-runtime-manifest.json and unmanaged by Stage-X64Client.ps1. Also: rasterMajor must be flipped in /e/SWG/64bit-server/_client/options.cfg:18 (or the last-included user.cfg), never earlier in client.cfg, because the .include sits at client.cfg's tail and ConfigFile is last-value-wins (ConfigFile.cpp:797).

## Commit sequence

### C1 — Direct3d11 project builds, gl11 loads, install refuses honestly

- Author /e/SWG/64bit-server/client-tools/src/engine/client/application/Direct3d11/build/win32/Direct3d11.vcxproj from scratch, adopting only what the reference project OMITS (zero $(DXSDK_DIR), no MachineX86/BaseAddress 0x62A00000//SAFESEH:NO/ImageHasSafeExceptionHandlers, no d3dx9/DxErr/ddraw/odbc, x64-only three configs, links d3d11+dxgi+d3dcompiler+dxguid+winmm+delayimp+legacy_stdio_definitions) - cloning Direct3d9.vcxproj instead would put $(DXSDK_DIR)Include first (Direct3d9.vcxproj:206,323,436) and silently bind D3D11.0/DXGI 1.1, losing CreateSwapChainForHwnd.

- Fix the reference's three defects: Optimized|x64 becomes Optimization=Full with BasicRuntimeChecks REMOVED (/O2 + /RTC1 is a hard D8016) while KEEPING MultiThreadedDebug + _DEBUG + DEBUG_LEVEL=1 so _ITERATOR_DEBUG_LEVEL stays 2 and matches SwgClient.vcxproj:281,283 (std::vector crosses the DLL boundary in setLights/getOtherAdapterRects); delete the vestigial VSPS define (a DX9-only FFP/shader source switch); delete all nine OutDir/IntDir/OutputFile/ImportLibrary/ProgramDatabaseFile/PCH/obj/pdb path literals pointing at a src/compile layout that does not exist here - Directory.Build.targets owns them. Re-add jpeg-static.lib + $(SwgX64DependenciesDir)\lib + $(SwgX64DependenciesDir)\include\libjpeg-turbo for the JPEG screenshot path. Drop IgnoreSpecificDefaultLibraries "libc; libcp".

- Reuse the reference GUID {B2C7D14E-4F9A-4C2B-9E61-7A0D3F8B11D9} (verified no collision in our swg.sln). Add the TargetName row after Directory.Build.targets:12; add the Project() block after swg.sln:209 mirroring Direct3d9 at :195-199 INCLUDING its DllExport ProjectSection; add six x64-only ProjectConfigurationPlatforms rows after Direct3d9's block; add one line to SwgClient's ProjectSection(ProjectDependencies) at swg.sln:706-772 - that solution edge, not a ProjectReference, is the only thing that makes /t:SwgClient build a raster DLL.

- src/win32/FirstDirect3d11.{h,cpp} on the FirstDirect3d9 pattern; src/shared/{MemoryManagerHook.cpp,SetupDll.cpp,SetupDll.h,PaddedVector.h,WriteTga.cpp,WriteTga.h} copied verbatim from Direct3d9/src/shared (MemoryManagerHook is mandatory for the engine to delete backend-allocated *GraphicsData; SetupDll.cpp's DliHook redirecting dllexport.dll to GetModuleHandle(NULL) is required for the DLL to load at all).

- Direct3d11.{h,cpp}: GetApi fills ONLY verify and install (Graphics calls verify first), exports GetGlApiStructSize, and install emits a WARNING and returns false. Every future slot gets a distinct explicit name - no DX9-style overload aliasing, which binds the wrong overload on a signature typo.

- Add gl11 to scripts/Build-X64Client.ps1's artifact assertion list (beside gl05/06/07 at :99-101), to Stage-X64Client.ps1's $runtimeFiles (:62-77), and to the God Client equivalents.

**Verify:** A standalone `msbuild Direct3d11.vcxproj /p:Configuration=Release /p:Platform=x64` with $env:DXSDK_DIR CLEARED emits src/build/win32/x64/Release/gl11_r.dll/.lib/.pdb (this is the only way to prove plan :325 while Build-X64Client.ps1:49 still sets DXSDK_DIR for DX9); `grep -c DXSDK_DIR Direct3d11.vcxproj` == 0; all three configurations link; /t:SwgClient builds gl11 (proves the sln dependency edge); with options.cfg:18 rasterMajor=11, warning.log shows LoadLibrary(".\\gl11_r.dll") succeeded and Graphics::install returned false with no MessageBox.

### C2 — Headless gl00 as the second Gl_api implementation

- Author /e/SWG/64bit-server/client-tools/src/engine/client/application/Headless/build/win32/Headless.vcxproj DX-FREE and x64-only. Do NOT clone the reference's version: it is a DX9-project clone carrying 12 $(DXSDK_DIR) references and d3d9/d3dx9/ddraw/DxErr on a stub with no DirectX in it - exactly the include-order hazard A19 warns about, on the project whose whole purpose is to be the DX-free implementation.

- Add a Directory.Build.targets row Headless -> gl00_$(SwgOutputSuffix) (our tree has neither the vcxproj nor any Headless sln entry; rasterMajor=0 is the console default at SetupClientGraphics.cpp:279 and selects a DLL that has never existed). Add the swg.sln Project() entry, six x64 rows and the SwgClient dependency edge.

- Implement GetGlApiStructSize in Headless.cpp OUTSIDE the void** blanket fill at :41-60 so the guard cannot be stomped. Headless.cpp itself needs no other source change (byte-identical between the trees). Add gl00 to the Build/Stage artifact lists.

**Verify:** gl00_{r,o,d}.dll and gl11_{r,o,d}.dll both build from swg.sln; the P0-A size guard now has two independent implementations; a deliberate Gl_api edit that breaks either one fails the build - which is what makes the Phase-6 contract trim verifiable at all.

### C3 — Device, swap chain, ConfigDirect3d11, honest capabilities, loud unimplemented accounting - a cleared frame presents

- Direct3d11_Device.{h,cpp}: CreateDXGIFactory2 -> IDXGIFactory2 (query IDXGIFactory5 for tearing), adapter honouring [Direct3d11] adapter, D3D11CreateDevice with {11_1,11_0} and FATAL below 11_0 (the reference silently accepted 10_1/10_0 at Direct3d11.cpp:1391), DEFAULT thread-safe flags per A11 (createShaderImplementationGraphicsData runs on the AsynchronousLoader thread), Os::isMainThread() asserts on every context entry point, and a local HRESULT formatter replacing FATAL_DX_HR/DXGetErrorString9. getVideoMemoryInMegabytes from DXGI_ADAPTER_DESC.DedicatedVideoMemory - the reference's hardcoded `return 2048` (Direct3d11.cpp:392) re-gates DOT3 at 40 MB and POST/HEAT at 100 MB (SetupClientGraphics.cpp:95/101/107), i.e. it can select a different shader implementation SET than DX9 on the same adapter. getOtherAdapterRects from IDXGIOutput::GetDesc. Six CrashReportInformation lines mirroring Direct3d9.cpp:1153/1171/1183/1198/1199/1500. Debug layer + ID3D11InfoQueue SetBreakOnSeverity(CORRUPTION|ERROR) with a benign deny-list, gated on [Direct3d11] debugLayer defaulting ON for DEBUG_LEVEL 1/2 - never the reference's literal `bool debugLayer=false` with dead activation code (:1393-1411), no per-frame drainDebugMessages (:887-932,942), no per-message CreateFileA, no DeleteFileA of log files at install (:1429-1430), no dx11Trace.

- Direct3d11_SwapChain.{h,cpp}: CreateSwapChainForHwnd, FLIP_DISCARD, BufferCount 3, R8G8B8A8_UNORM, ALLOW_TEARING when reported, MakeWindowAssociation(NO_ALT_ENTER), never SetFullscreenState (A15). Honour every Gl_install field the reference dropped: all four bit depths (they arrive as -1 meaning 'pick a default' per SetupClientGraphics.cpp:263-266), skipInitialClearViewport, and STORE gl_install->windowedModeChanged as Direct3d9.cpp:983 does. resize/setWindowedMode = ResizeBuffers plus a borderless style change, firing the deviceLost/deviceRestored registries the reference maintained (Direct3d11.cpp:397-400,771-783) and never iterated - BinkVideo.cpp:120-121 and PostProcessingEffectsManager.cpp:107-108 are the two real subscribers. DXGI_ERROR_DEVICE_REMOVED is FATAL; wasDeviceReset() returns false permanently with a comment. Present interval from [Direct3d11] allowTearing, not the reference's unconditional Present(0,0) (:943) which invalidates all frame pacing.

- ConfigDirect3d11.{h,cpp} over section "Direct3d11" with the plan's key list (dx11-port-plan.md:301), ALL read once in install with FATAL validation on out-of-range values (mirroring ConfigDirect3d9.cpp:90's vertexProcessingMode discipline) and published to CrashReportInformation. NOT carried: forceCullNone and disableDepthClip (Direct3d11_StateCache.cpp:291,302 - two lazily-read function-local statics whose only purpose is restoring known-wrong global rasterizer state) and all nine [ClientGraphics/Lighting]/[ClientGame/Bloom] tunables. Keep the percent-scaled-int idiom where a float is wanted: only the 3-arg getKeyInt/getKeyBool are DLLEXPORT (ConfigFile.h:148-152).

- Direct3d11_Unimplemented.{h,cpp}: a DX11_NOT_IMPLEMENTED(slot) macro that increments a named per-slot counter, emits ONE WARNING per slot per run naming the slot, FATALs at DEBUG_LEVEL>=1, and prints an end-of-run report. Every not-yet-implemented Gl_api slot is assigned to a distinct named function calling it. This designs out, as a class, the reference's ~18 slots wired to empty `{}` bodies (setFog :672, setScissorRect :669, setAlphaFadeOpacity :679, setBloomEnabled :746, six point-sprite setters :430-435, optimizeIndexBuffer :742, screenShot :645, writeImage :752, four video-buffer slots :757-762, three pix* :748-750).

- pixSetMarker/pixBeginEvent/pixEndEvent -> ID3DUserDefinedAnnotation in ALL configurations - these are exactly the instrumentation a RenderDoc parity campaign runs on, and empty bodies actively sabotage it. supports* answered HONESTLY: supportsScissorRect FALSE until C6 (never the reference's true-at-:403 with a no-op at :669), supportsHardwareMouseCursor FALSE so CuiLayer_CursorInterface uses the OS path, supportsAntialias from CheckMultisampleQualityLevels.

**Verify:** rasterMajor=11 presents a cleared frame at the requested resolution windowed and exits cleanly; alt-tab, minimise/restore and a resolution change each fire deviceLost+deviceRestored exactly once around ResizeBuffers with no device loss; the debug layer reports zero CORRUPTION, zero ERROR and zero hazard warnings across launch/resize/exit; getShaderCapability()==0x0200 and getVideoMemoryInMegabytes()>100 are observed at Graphics::install (so DOT3/POST/HEAT stay enabled) and the 2.0 value is CONFIRMED against what gl05 reports on the same hardware rather than asserted; a DEBUG_LEVEL=1 build FATALs on the first unimplemented slot, proving the accounting is loud.

### C4 — SceneTarget + real capture path + DX9-quantised gamma composite - the commit that makes the parity gate falsifiable

- Direct3d11_SceneTarget.{h,cpp} per A8: offscreen R8G8B8A8_UNORM colour + D24_UNORM_S8_UINT depth (D32_FLOAT_S8X24 fallback; stencil IS used by the shadowvolume passes), SampleDesc from [Direct3d11] antiAliasSampleCount validated with CheckMultisampleQualityLevels, plus a resolved shader-readable twin. setRenderTarget(NULL) returns to the scene target, never to the back buffer. One decision unlocks lockBackBuffer, presentToWindow, gamma, MSAA and screenshots together.

- Composite at present from a full-screen triangle off SV_VertexID. Gamma reproduces DX9 NUMERICALLY: build the same 256-entry table Direct3d9.cpp:2064-2083 builds (pow(0.5 + contrast*(f*brightness - 0.5), 1/gamma)), upload it as a 256x1 LUT sampled POINT, and populate ms_colorCorrectionTable for the capture path. This rejects four separate deviations in the reference pass: a saturate() inserted BEFORE the pow (Direct3d11.cpp:178) that DX9 does not have, full-float per-pixel evaluation instead of an 8-bit quantised LUT, alpha forced to 1.0 over the back buffer's alpha (:180), and a LINEAR sampler on a 1:1 copy (:214). Keep the identity short-circuit (:409-419) but make it a bit-exact assert at (1,1,1), not a tolerance. Save/restore or explicitly re-bind every state the pass touches INCLUDING the tracked current RTV/DSV - the reference left ms_currentRtv/ms_currentDsv desynchronised from the real bindings across the frame boundary and left CULL_NONE latched.

- Direct3d11_ImageWriter.{h,cpp}: real screenShot (TGA via the copied WriteTga.cpp, JPG via jpeg-static as Direct3d9.cpp:2766-2823 does, WIC for bmp/png/dds), real writeImage, and lockBackBuffer/unlockBackBuffer as a SceneTarget staging copy. Explicit Gl_imageFormat -> container map, breaking the D3DXIMAGE_FILEFORMAT numeric coupling without renumbering. The reference returned false from all four (Direct3d11.cpp:645,752,445,446) and shipped no WriteTga module, so it structurally could not produce the artifact a byte-parity gate consumes - which is why every parity claim about it is unfalsifiable, including its own claims of success.

**Verify:** A gl11 screenshot of a fixed clear colour is BYTE-IDENTICAL to the gl05 screenshot of the same clear at both resolutions and in all three image formats; the composite pass is provably skipped at (1,1,1) (counter + LUT-identity assert); a non-identity gamma screenshot matches gl05's colorCorrectionTable output byte-for-byte; debug layer clean. From this commit on, every later commit has an oracle-comparable artifact - which is the entire reason it precedes any geometry.

### C5 — Metrics, DebugFlags, query pool, and the five _DEBUG slots - DEBUG_LEVEL 1/2 builds and runs

- Direct3d11_Metrics.{h,cpp} implementing the Phase-0 GfxStats contract (counter set derived from Direct3d9_Metrics.h:30-99: per-phase draws/triangles for all 13 phases, per-category bind CALLS and MISSES, constant bytes and update counts per buffer, ring DISCARD vs NO_OVERWRITE, RT switches, bake readbacks, backbuffer maps, blocking staging maps) plus the DX11-only creation counters the zero-invariants gate on (state objects, input layouts, cbuffers, shader compiles) and the dropped-draw counter C9/C11 depend on. The reference shipped no Metrics at all.

- Wire the counter block through the P0-F out-of-band export so Gl_api stays byte-stable (A1).

- Implement the five #ifdef _DEBUG slots the reference never even DEFINED - setTexturesEnabled, showMipmapLevels, getShowMipmapLevels, setBadVertexBufferVertexShaderCombination, getRenderedVerticesPointsLinesTrianglesCalls - against Metrics. Graphics.cpp:1158, :1208 and :1266 wrap three of them in NOT_NULL(), which is why the reference backend is Release-only by construction and cannot be run in the configurations a parity investigation uses.

- DebugFlags section "Direct3d11" mirroring DX9's 24 levers (Direct3d9.cpp:1531-1554 disablePass0..3+/minimumDrawPrimitives*/noClear*/noScissor/noIndexedPrimitives/disableAlphaFadeOpacity; Direct3d9_LightManager.cpp:170-176 lightingFullAmbient/forceAllLightsOn/Off; Direct3d9_StateCache.cpp:209 noTextures; Direct3d9_ShaderImplementationData.cpp:394; Direct3d9_StaticShaderData.cpp:472,976). These are the bisect levers a divergence hunt runs on, and any lever that can change output lives here - never as a ConfigFile key.

- Direct3d11_QueryPool.{h,cpp}: TIMESTAMP / TIMESTAMP_DISJOINT / PIPELINE_STATISTICS, triple-buffered, DONOTFLUSH, read 3 frames late so no GetData can block the main thread.

**Verify:** DEBUG_LEVEL=2 and DEBUG_LEVEL=1 builds of gl11 load and run (the three Graphics.cpp NOT_NULLs pass); GetGlApiStructSize matches sizeof(Gl_api) at all three levels and a deliberately mismatched pair FATALs with a readable message instead of loading; every zero-invariant counter reads 0 on a cleared frame; ID3DUserDefinedAnnotation markers appear in a RenderDoc capture.

### C6 — State objects, with DX9's tables ported index-for-index

- Direct3d11_StateObjectCache.{h,cpp}: hash-consed immutable Blend/DepthStencil/Rasterizer/Sampler objects created ONLY at Pass::construct / Stage::construct or pre-enumerated at install (the immutable tuple is already gathered at load time - Direct3d9_ShaderImplementationData.cpp:239-243 and Direct3d9_StaticShaderData.cpp:200-209), with asserted caps blend<=64, DS<=128, RS<=32, sampler<=64. Direct3d11_StateCache.{h,cpp}: bound-object shadow with paired call/miss counters.

- Compare[] copied from Direct3d9_ShaderImplementationData.cpp:31-41 INDEX-FOR-INDEX including C_GreaterOrEqual->D3DCMP_NOTEQUAL and C_NotEqual->D3DCMP_GREATEREQUAL, as one shared table with a unit test asserting the swap (plan :358). The asset encodes the INDEX, so DX9's order is the binary contract regardless of its own mislabelled comments; the reference inverted 5/6 knowingly (Direct3d11_StateCache.cpp:82-90) and its comment records that the parity fix was tried and REVERTED because it regressed stencil shadows - i.e. a wrong translation kept to mask a different bug, which P0-C's census plus the C16 z-fail work resolves properly.

- TextureFilter[] ported as a literal index table from Direct3d9_StaticShaderData.cpp:57-67 (TF_none->NONE, TF_point->POINT, TF_linear->LINEAR, TF_anisotropic->ANISOTROPIC, TF_flatCubic->NONE, TF_gaussianCubic->NONE, TF_invalid->LINEAR), replacing the reference's four separate deviations in one predicate function (Direct3d11_StateCache.cpp:125,332-350). TF_none mip is implemented as MinLOD=MaxLOD=0, not a filter bit. MaxAnisotropy clamps to the real device cap, not a literal 16.

- Rasterizer states pre-enumerated as the full 3 cull x 2 fill x 2 scissor matrix at install; real setScissorRect with ScissorEnable actually reaching D3D11_RASTERIZER_DESC (the reference always passed scissorEnable=false at :1003), and supportsScissorRect flips to true in this commit. Cull mapping keeps the reference's correct GCM_clockwise->CULL_FRONT with FrontCounterClockwise=FALSE (:273-279) and drops forceCullNone/disableDepthClip entirely.

- Alpha-fade blend variants enumerated from the P0-C census by one unit-tested function (blend forced on with the AUTHORED factors, alpha write cleared iff the authored pass did not blend) - never created at draw time.

- Cache keys are packed integers / fixed-size PODs in a hash map. The reference built a heap-allocated std::string from raw descriptor bytes on every fetch (descKey at Direct3d11_StateCache.cpp:98-101, call sites :189,:242,:306,:354) - with a 264-byte D3D11_BLEND_DESC that is malloc+memcpy+red-black-tree walk per lookup, and getRasterizerState is reached from setCullMode which Graphics.cpp:852-857 issues unconditionally in save/restore pairs per shadow volume and per ribbon draw.

- Adopt the reference's Create*State-failure FATAL policy AND its reasoning comments verbatim (Direct3d11_StateCache.cpp:196-199,249-253,313-316,361-363) - binding NULL silently falls back to the D3D11 default state (depth Enable=TRUE/write ALL/LESS would break the depth-off sky passes and z-fail stencil shadows). It is the one place that tree got error policy exactly right.

**Verify:** Unit tests assert the compare swap index-for-index against the DX9 table, the 7-entry filter table, and the blend-variant derivation from the census; state-object creations after warm-up = 0 (Metrics counter); a deliberate cap breach fails loudly; redundant-bind hit rates instrumented (state objects >=90% target); zero heap allocations in the bind path, measured.

### C7 — Shader compile chokepoint, reflection ABI guard, disk cache, and the two .inc conversions

- Direct3d11_ShaderCompiler.{h,cpp}: D3DCompile at vs_4_0/ps_4_0 (A4, not the reference's vs_5_0/ps_5_0 at Direct3d11_VertexShaderData.cpp:523 and Direct3d11_PixelShaderProgramData.cpp:754,809) with ENABLE_BACKWARDS_COMPATIBILITY + OPTIMIZATION_LEVEL3, never PACK_MATRIX_ROW_MAJOR. Keep the reference's chokepoint SHAPE and its TreeFile-backed ID3DInclude with the leading '../../' strip (Direct3d11_ShaderCompiler.cpp:100-155), and ADD the include cache the reference dropped (DX9 has ms_includeCache at Direct3d9_VertexShaderData.cpp:72/213-227). Compile failure is FATAL in non-PRODUCTION; fxc warnings on SUCCESSFUL compiles are logged, not released and discarded as at :210-211 - implicit-truncation and register-packing warnings are exactly what a parity investigation needs.

- DELETE the reference's textual asset surgery entirely (Direct3d11_ShaderCompiler.cpp:90-95): the `: register(b` -> `= true; //` pass forces the eight light_*_enabled booleans to compile-time TRUE, inverting DX9 where Direct3d9_StateCache::setVertexShaderConstants(int,const BOOL*,int) at :542 has ZERO callers so b0..b7 keep the D3D9 default FALSE; and the `#pragma`->`//pragma` pass comments out `#pragma def(vs,c95,0,0.5,1.0,1.4426950408889634)`, the only thing that would have supplied c95's literals. Instead convert the ASSETS: vertex_shader_constants.inc and pixel_shader_constants.inc become explicit cbuffers at b0 with the register file preserved 1:1, c0-c7 VACATED into SwgPerObject at b3 (the single change that makes the per-draw budget reachable), the dead `#if VERTEX_SHADER_VERSION >= 20` bool block DELETED, the pragma replaced by a static const, LightData::point renamed (which removes the reference's `#define point _pt_lights` injection at :25 that DX9 also needs), and the extendedLightData declaration dropped as reserved padding.

- Delete the pixel_shader_constants.inc runtime override rather than porting it as a string literal (Direct3d9_PixelShaderProgramData.cpp:82-105 / Direct3d11_ShaderCompiler.cpp:39-62): the copy that actually wins already matches the PSCR enum, verified by P0-G's resolver.

- Author shared_program/sm4compat.hlsli: DECLARE_SAMPLER2D/CUBE/3D(name,slot) emitting Texture+SamplerState at matching slots with token-pasted tex2D/tex3D/texCUBE wrappers so the 372+18+12 call sites are untouched; the b0/b1/b3 cbuffer declarations; the alpha-test epilogue and fog-blend macros consumed in C13/C14. Strip ` : register(vN)` by editing the assets (392 sites), not by the reference's rfind(':') heuristic (Direct3d11_VertexShaderData.cpp:65-79) which can over-erase.

- Direct3d11_ShaderReflection.{h,cpp}: assert every named $Globals constant sits at 16 x its register with a named failure diagnostic; record the reflected extent for bounded uploads; build the sampler->texture map from GetResourceBindingDesc (the reference's buildSamplerToTextureRegisterMap at Direct3d11_PixelShaderProgramData.cpp:825-863 is sound and necessary - adopt it); cache the VS input signature.

- Direct3d11_ShaderCache.{h,cpp} + the offline baker: keyed on SHA-256 of (canonicalised source, contents of every include actually opened, sorted macro set, target, flags, d3dcompiler version) storing DXBC plus reflection metadata. Every cache is keyed by a STABLE identity - never the reference's raw ID3DBlob* keys (m_pairedByVs at :798, InputLayoutMap Key::vsByteCode at Direct3d11_InputLayoutMap.cpp:31), where a freed-and-reallocated blob address aliases a stale entry.

**Verify:** The two smallest converted assets (2d_texture.psh, vertex_color.psh - both verified exact one-line transcriptions) compile at vs_4_0/ps_4_0 with warnings-as-errors through the baker, and the reflection guard confirms every named $Globals constant at 16xregister; a cold-vs-warm cache load time is measured and published; the baker fails the build on any error.

### C8 — Constant buffers on one shared register contract, with every register DX9 uploads

- Direct3d11_ConstantBuffers.{h,cpp} over the P0-B promoted clientGraphics/ShaderConstantRegisters.h - no local anonymous enums and no bare integer register numbers anywhere (the reference had four uncoordinated copies at Direct3d11_ConstantBuffers.cpp:33-41, Direct3d11_StaticShaderData.cpp:33-40 and Direct3d11.cpp:978, naming 5 of ~24 VSCR entries). Shadow size DERIVED from VSCR_MAX plus an enumerated c95 tail, not the reference's magic VS_REGISTERS=96 which exceeds DX9's VSCR_MAX=68.

- ADOPT the reference's verified core insight: -Gec places register(cN) globals into $Globals at byte offset exactly 16*N, so a flat register*16 CPU shadow with DX9-shaped setters lets LightManager and StaticShaderData port unchanged. REJECT its flush model: apply() Map(WRITE_DISCARD)s the ENTIRE 1536 B VS and 512 B PS buffers on every draw (Direct3d11_ConstantBuffers.cpp:89-100,243-277 from bindDrawState at Direct3d11.cpp:1069) because the transform combine dirties c0-c7 and selectLights rewrites the whole light block per draw - the plan calls this arithmetically fatal at 12x DX9. Instead: per-register dirty tracking, at most one flush per stage per draw bounded by the REFLECTED extent, b0 = $Globals with c0-c7 vacated, b1 = SwgPixelEpilogue, b3 = SwgPerObject 128 B from a 256 B-aligned Map(WRITE_NO_OVERWRITE) ring bound with VSSetConstantBuffers1 offsets on 11_1 and a measured-and-reported rotating-DISCARD fallback on 11_0.

- Upload the registers the reference NEVER wrote: c95 = {0.0, 0.5, 1.0, 1/log(2)} and c49/c50/c51 = unitX/unitY/unitZ every frame exactly as Direct3d9_StateCache.cpp:230-244 (registers.inc aliases c0_0/c0_5/c1_0/cLog2e to c95.xyzw across 9 asm modules and terrain_dot3.inc consumes the unit vectors, so without these every 0.5 bias and 1.0 constant reads zero); VSCR_fog c10 from a REAL setFog with DX9's exact {0,0,density,density*density} packing (Direct3d9.cpp:3266-3267); VSCR_currentTime c48 from update() (Direct3d9.cpp:2288-2301, which the reference's `ms_currentTime += elapsedTime` at :441 omitted entirely). Keep the reference's correct VSCR_viewportData c9 upload in setViewport (:955-980) - without it all 2D/UI collapses to clip 0,0.

**Verify:** C7's reflection ABI guard passes on the converted .inc for every shader compiled so far; per-draw constant bytes <=256 counted, and no per-draw Map on any buffer larger than 256 B; a gl05 per-draw reference capture (P0-K) and a gl11 capture of the same pinned frame byte-compare across c0..c95 including c9, c10, c48, c49-c51 and c95.

### C9 — Buffers, descriptor map, input-layout cache, draw dispatch, primitive emulation - every drop path loud

- Direct3d11_{Static,Dynamic}VertexBufferData, Direct3d11_{Static,Dynamic}IndexBufferData, Direct3d11_VertexBufferVectorData - names are hard-bound by the existing friend grants (StaticVertexBuffer.h:47, DynamicVertexBuffer.h:46, StaticIndexBuffer.h:36, DynamicIndexBuffer.h:36, VertexBufferVector.h:31). ADOPT two reference decisions with their rationale: the monotonic-id sortKey (Direct3d11_StaticVertexBufferData.cpp:35) which avoids DX9's reinterpret_cast<int>(ptr) x64 truncation, and storing VertexBufferFormat BY VALUE (Direct3d11.cpp:341-346) because the engine binds stack-local temporary DynamicVertexBuffers and issues the draw after they die.

- REJECT the reference's static-buffer model (Direct3d11_StaticVertexBufferData.cpp:39-51,84-99 and _StaticIndexBufferData.cpp:26-38,57-76): USAGE_DEFAULT plus a permanently retained uninitialised CPU shadow re-uploaded with a full-buffer UpdateSubresource (null box) on every unlock. Use IMMUTABLE + pSysMem where never relocked (A14: strictly faster than DX9's MANAGED) and free the shadow after the single upload; DEFAULT + staging only where lock(readOnly) must return readable memory. Rings resized 2 MB -> 16 MB and 64 KB -> 4 MB (A16, because ClientTerrainSorter.cpp:36 batches 256 KB per node), NO_OVERWRITE with DISCARD on wrap, thin NON-OWNING wrappers, and drop the IsBadWritePtr/getTemporaryBuffer redirect that would mask real Map failures.

- Direct3d11_VertexBufferDescriptorMap ported verbatim (pure POD, no D3D) from the DX9 version, preserving the int8 vertexSize/offset constraint. Direct3d11_InputLayoutCache keyed on (format set, VS input-signature hash) reduced to the (texcoord index, dimension) set per A10, with a reserved 16-byte zero-fill stream at PER_INSTANCE/InstanceDataStepRate=0 for signature elements the vertex format does not supply - this reproduces DX9's (0,0,0,0) read, keeps the 'Missing vertex components in %s on shader %s' warning at Direct3d9.cpp:4000 visible, and avoids dropping materials where CreateInputLayout hard-fails.

- Direct3d11_PrimitiveEmulation.{h,cpp}: quad and fan index buffers PRE-CREATED at install, generously sized, FATAL on any grow - never the reference's draw-time Release+CreateBuffer with a silent `return` on failure (Direct3d11.cpp:1149-1150) which violates the zero-creations-per-frame invariant, and never its IASetIndexBuffer clobbering the engine's bound IB at :1154. Keep its correct (0,i+1,i+2) fan winding. Implement drawIndexedTriangleFan and drawPartialIndexedTriangleFan, which the reference left as literal `{}` at :726/:740 while assigning them at :1316/:1330. Assert the 16-bit index capacity instead of silently overflowing at ~16k quads / 64k fan verts.

- All 11 drawPartial* variants implemented per-variant (the reference's arithmetic at :728-739 looks correct and is a named A/B target because DX9 aliases every drawPartial* to its full-draw counterpart at Direct3d9.cpp:1104-1116 and recovers the range from ms_slice* state). getMaximumVertexBufferStreamCount reports the REAL limit - the reference reported 16 while setVertexBufferVector capped at MAX_STREAMS=2 (:1569) and InputLayoutMap at MAX_VERTEX_BUFFERS=2 with a Release-NOP DEBUG_FATAL, i.e. silent truncation.

- Every drop path is loud: a once-per-shader WARNING naming the shader filename AND the vertex-format flags, a Metrics dropped-draw counter, and FATAL in non-PRODUCTION - replacing the reference's silent bindDrawState `return false` (:1044-1050) with callers that just `return` (:1081-1082,:1095-1096) and an unnamed CreateInputLayout warning (Direct3d11_InputLayoutMap.cpp:203-211).

**Verify:** Unit test: construct a stack DynamicVertexBuffer, lock, copy, bind, destroy the temporary, then draw - debug layer clean (the ShadowVolume.cpp:127-146 pattern that four other subsystems also rely on); <=1 Map(WRITE_DISCARD) per ring per frame; input-layout creations after warm-up = 0; dropped-draw counter 0; per-phase draw and triangle counts within 2% of the DX9 baseline (Gate G1) for whatever slice is live.

### C10 — Textures and the format map

- Direct3d11_TextureData.{h,cpp} + Direct3d11_TextureFormatMap.{h,cpp}: all plain _UNORM, no sRGB anywhere (30,245 shipped .dds carry no colour-space metadata and DX9 lit in gamma space). Keep the reference's CheckFormatSupport probe (Direct3d11_TextureData.cpp:67-77), Texture3D for volume maps (:125-138) and ArraySize 6 + MISC_TEXTURECUBE for cubes (:147,152).

- TF_L_8 is EXPANDED to B8G8R8A8 on load with addConversion registered - never the reference's DXGI_FORMAT_R8_UNORM (:42), because D3DFMT_L8 samples (L,L,L,1) and R8 samples (R,0,0,1), D3D11 has no SRV component swizzle, and Texture.cpp's addConversion(TF_L_8, TF_L_8) leaves no fallback. Only 4 shipped textures use it so expansion is free. TF_P_8/RGB_888/RGB_565/RGB_555 are DECLINED via the CheckFormatSupport -> TextureFormatInfo::setSupported sweep: the reference mapped TF_RGB_555 to B5G5R5A1_UNORM (:35), which turns D3D9's IGNORED X1R5G5B5 pad bit into a live alpha bit, so any asset whose pad bit is 0 becomes fully transparent (the .dds census shows zero X1R5G5B5 assets, so declining is behaviour-neutral; A1R5G5B5 keeps its 4 assets on B5G5R5A1). Complete TextureFormatInfo.cpp's table - only 15 of TF_Count==17 entries are initialised, leaving TF_ABGR_16F/32F with name=NULL.

- Full LockData protocol (2D / volume / cube-face) using m_reserved for the staging handle; read-only locks on a 3-frame staging ring with MAP_FLAG_DO_NOT_WAIT; short and absent mip chains loaded as-is with MaxLOD clamped (1,722 shipped textures; GenerateMips does not work on BC).

- Every creation failure is FATAL with the C3 HRESULT formatter, extending the state-cache policy tree-wide. The reference set m_srv=0 (:186-189) and m_rtv=0 (:195-200) on failure and fell back to a blank B8G8R8A8 texture with the comment 'data may be wrong, but the client boots' (:164-183) - each of which is a category of broken frame a pixel diff flags and no log explains. Also delete its WARNING(true, ...) that fires on EVERY render-target creation including successes (:200-203), polluting the one file triage starts with.

**Verify:** A texture-load unit test over one shipped asset per format; the declined-format sweep produces the same TextureFormatInfo supported set DX9 ends up with; an L8 asset sampled through a shader that reads .g/.b/.a matches gl05; zero WARNINGs in warning.log on a clean load; debug layer clean.

### C11 — Shader data classes and the UI/2D slice - first pixels, with no replacement table anywhere

- Direct3d11_VertexShaderData, Direct3d11_PixelShaderProgramData, Direct3d11_ShaderImplementationData, Direct3d11_StaticShaderData - all four names are hard-bound by the pre-seeded friend grants (ShaderImplementation.h:45-46, StaticShader.h:35, StaticShaderTemplate.h:29, ShaderEffect.h:26), which are byte-identical between the trees.

- Program resolution is by EXACT asset filename against the converted .hlsl corpus. The reference's 28-row PS table and 9-entry VS if-chain matched by UNANCHORED strstr with hand-tuned ordering to dodge prefix collisions (Direct3d11_PixelShaderProgramData.cpp:346-391, Direct3d11_VertexShaderData.cpp:312-332); measured against the live corpus they captured 53 of 130 asm PS and 37 of 94 asm VS and rewrote most captured files with math that is not the file's math ('t.psh' alone caught 9 unrelated shaders and added a vertex-colour multiply the asset does not have while destroying the alpha). None of it is ported. Substring selection is unfixable in principle - any new asset silently steals or loses a match.

- Any //asm program with no converted asset is a FATAL in non-PRODUCTION and a counted PRODUCTION miss - never the reference's WARN-and-null-shader (:462-470, :695-699) that bindDrawState then swallowed, and never a fallback to a generic PS whose input signature does not match the bound VS (:815-820), which D3D11 rejects at draw so the surface silently vanishes.

- KEEP m_textureCoordinateSetTags and honour the 24-bit permutation key exactly as Direct3d9_VertexShaderData.cpp:676-732 does - the reference's replacement hook deleted the tags (Direct3d11_VertexShaderData.cpp:405-416) and hardcoded TEXCOORD0/1, so every asset binding DETA/SPEC/MASK to another physical set read the wrong stream (tfcl_5uv writes five oT registers; c_detail_specmap declares MAIN/DETA/SPEC).

- ADOPT verbatim, with reasoning: the SRV bound at the FXC-assigned t# recovered from reflection rather than assuming t#==s# (Direct3d11_StaticShaderData.cpp:343-344 + the sampler->texture map from C7); the global-texture pointer-to-pointer late binding for TAG_ENVM and leading-underscore tags (:103-104,134-142, mirroring Direct3d9_StaticShaderData.cpp ~392-396); and the stencil-reference publish-before-apply with s_appliedStencilReference forcing a re-bind when only the ref changed (:486 -> Direct3d11_ShaderImplementationData.cpp:102-104,144). Preserve the ms_active/ms_pass two-level redundancy skip.

- Bind all stages with ONE PSSetShaderResources + ONE PSSetSamplers (the reference bound one slot at a time at :154,:189). NO unconditional 16-slot SRV null clear on shader bind (:476-480) - the stale-SRV hazard is handled by C16's targeted unbinds driven by tracked RTV/SRV intent. NO per-stage self-sample probe (:158-190) and no dx11Trace: the logging was capped at 80 hits but the resolve-and-compare ran forever, in Release, per texture stage per draw.

- Convert and validate the UI/2D slice only: 2d_texture, 2d_view_alpha, 3d_vertex_color(_a), uicanvas_filtered/_z/_clamped, uicanvas_radar, ui_membrane, font, bad_vertex_shader, the nine preloaded vertexcolor templates, and texren_copy_c1a1.psh - which must become a real .hlsl asset replacing the string literal at Direct3d9_PixelShaderProgramData.cpp:133-143 (note its PSRC reads the symbolic c[textureFactor], not c2; only the stale PEXE blob was assembled against c2). drawQuadList drives the entire 2D UI so it is the primary smoke test.

**Verify:** Gate A on the slice: every enumerated permutation compiles at vs_4_0/ps_4_0 with warnings-as-errors and every VS input signature is satisfiable by the formats the corpus supplies; the reflection ABI guard passes for every slice shader with every sampler on its expected texture slot; login and character-select render pixel-identically to the gl05 capture at uiScale 1.0 (non-unity waits on C12); <=3 API calls per draw and <=10 per shader change counted directly; zero runtime shader compiles on a warm cache; dropped-draw counter 0.

### C12 — Settle the half-pixel / one-to-one-UV question from a capture, not an assertion

- Implement getOneToOneUVMapping to whatever P0-D's gl05 RenderDoc A/B proves DX9 actually samples. DX9 returns a texel-CENTRE-to-centre mapping with a (w-1)/w SPAN (Direct3d9.cpp:3552-3558: u0=0.5/w, u1=(w-1+0.5)/w); the reference returns flat 0,0,1,1 with UNREF'd dimensions (Direct3d11.cpp:984-993) arguing 'DX11 texel centers are already aligned', which conflates the D3D9 half-PIXEL rasterisation offset (genuinely fixed in D3D10+, a vertex-position concern) with texture coordinates (unaffected by that rule). The two differ by a half texel of offset AND a 1/w scale.

- Add Graphics::usesHalfPixelOffset(), set in Graphics::install from the loaded backend, and read it from CuiManager::ms_pixelOffset (CuiManager.cpp:328) and the -0.5f offsets in Bloom.cpp:338 and PostProcessingEffectsManager.cpp:225. Engine-side only, no ABI change. Any residual rasterisation delta is corrected at the VERTEX POSITIONS, never by redefining what the accessor returns.

**Verify:** UI text, icons and the radar render pixel-identically to the gl05 captures at uiScale 1.0 AND at a non-unity uiScale - the half-pixel trap the plan calls out as a visual-parity gate item; the RenderDoc A/B artifacts are attached to the commit as the decision record so the convention cannot be re-litigated by assertion.

### C13 — Alpha test + alpha-fade opacity + bloom flag as one increment

- These are one mechanism, not three: DX9 rescales D3DRS_ALPHAREF per draw by ms_alphaFadeOpacity.a (Direct3d9.cpp:3932-3946), so alpha test cannot be designed without alpha fade.

- Implement setAlphaFadeOpacity for real with DX9's dirty-flagged .r/.a tracking (Direct3d9.cpp:3455-3486) and setBloomEnabled writing .g (Direct3d9.cpp:4560-4566), and forward all three into the dot3 PS constants exactly as Direct3d9_LightManager.cpp:654 does. Delete the reference's hardcoded VectorRgba(0,0,0,1) at Direct3d11_LightManager.cpp:64-70 - which is also why object dissolve/fade-in was dead and bloom-flagged surfaces never signalled the bloom pass. Note the justifying comment repeated across the reference's replacement shaders ('alphaFadeOpacity is a STUB=0, do NOT read it') is FALSE in that same tree, since its LightManager writes 1.0 into that lane at :355/:384.

- Alpha test becomes the b1 SwgPixelEpilogue scalar with EXACT 8-bit semantics - `if (round(a*255) <= alphaTestRef8) discard;` (A17). All 422 alpha-tested passes use Greater, and A000 resolves to ref 0 where clip(a-0) would KEEP the pixels DX9 discards, so the comparison must be integer-exact and the ref must be a constant (not a literal) because of the alpha-fade rescale. Consume the m_alphaTestReferenceValue/Valid the reference captured at Direct3d11_StaticShaderData.cpp:306-314 and NEVER read anywhere in its 40 files - the single largest silent image divergence in that tree (foliage, fences, grates, hair cards, UI masks). Place the discard before any dependent texture fetch that would sink it.

- Alpha-fade blend variants come from C6's census-enumerated set, never created at draw time.

**Verify:** Gate B' alpha-tested case with MSAA off and on; a foliage/fence/grate/hair-card frame diffs <=1/255 per channel on >=99.9% of pixels against the gl05 capture, with the A000 ref-0 boundary case explicitly included; an object dissolve sequence matches gl05 frame for frame; the bloom flag observably reaches the bloom pass; the GPU-time budget for alpha-test-as-discard on 422 passes is recorded at 3840x1080.

### C14 — Fog, end to end, with the interpolant modifier measured

- `: FOG` becomes a plain VS interpolant with the interpolation modifier fixed by P0-E's gl05 A/B on a long fogged surface - measured, not guessed, because DX9's fixed-function fog unit and an SM4 interpolant are not necessarily the same. Every //asm world VS in the corpus writes oFog via fog.inc and NO reference replacement emitted a FOG output, so fog was missing backend-wide, not in one slot.

- b1 SwgPixelEpilogue carries fogColor.rgb + fogEnable and the PS applies lerp(fogColor, rgb, saturate(fog*fogEnable)), folded into an existing trailing lerp/modulate where one exists. fogEnable MUST be a separate constant: setFog(false,...) at Direct3d9.cpp:3269-3273 clears only D3DRS_FOGENABLE and leaves the c10 density live, and it is used as a scoped disable around the heat composite (ShaderPrimitiveSorter.cpp:566) and the star field (StarAppearance.cpp:347) - an unconditional lerp would fog both. The c10 upload itself already landed in C8.

**Verify:** Gate B' fixed triangles with fog values outside [0,1] rendered on both backends and diffed; the heat composite and the star field are provably unfogged on gl11 exactly as on gl05; a fogged terrain vista diffs <=1/255 on >=99.9% of pixels; the per-pixel-fog GPU-time budget across ~90 shader pairs is recorded at 3840x1080.

### C15 — LightManager with DX9's constants as literals and nothing else

- Port Direct3d9_LightManager's selection and sorting UNCHANGED (it touches no D3D) and upload LightData(28) at c16 and PixelDot3Data(5) at PS c0 by memcpy, preserving the .w/.a lane packing. Drop the extendedLightData upload (zero live references; the C++ struct covers only 4 of its 8 rows). Keep the Phase-0 light over-dirtying fix.

- The constants are LITERALS with no config indirection: ambient floor 0.30f (Direct3d9_LightManager.cpp:559), hemi-tangent 0.65f and hemi-back 0.30f (:784-792). NOT ported: ms_ambientFloor=0.36 / ms_hemiBack=0.48 / ms_hemiTangent=0.74 (Direct3d11_LightManager.cpp:56-62), the six [ClientGraphics/Lighting] keys (:105-114), the zero-ambient POB 'interior gate' (:307-320) which DX9 has no equivalent for at all and which makes the divergence scene-dependent, and the FABRICATED second directional sun (:413-432) at 45% intensity and 30 degrees azimuth offset. The second sun is also a false-success trap: its guard at :420 requires parallel[0] non-null, which the DX9-faithful swapLight cascade never satisfies for a single world directional (the sun lands in parallelSpecular[0]), so it currently does nothing and would start firing the moment light selection changes.

- Note the corollary for C19: because the dominant sun lives in parallelSpecular_0 (c17/c18), every reference VS replacement that claimed to add sun lighting read only c20-c23 and therefore read all-zero registers.

**Verify:** An outdoor-noon frame and a cantina-interior frame each diff <=1/255 on >=99.9% of pixels against gl05; the c16..c43 LightData block and the PS c0 PixelDot3Data block byte-compare to the gl05 per-draw capture; the DebugFlags levers lightingFullAmbient and forceAllLightsOn/Off reproduce DX9's behaviour exactly.

### C16 — Direct3d11_RenderTarget and the RT-capable texture bake

- Direct3d11_RenderTarget.{h,cpp} - the reference had NO counterpart at all for Direct3d9_RenderTarget.cpp's 341 lines, inlining part of it at Direct3d11.cpp:509-643. RTVs created directly on destination textures honouring the mip semantics Graphics.cpp:1010-1029 assumes (dimensions shifted right by mipmapLevel) and honouring the cubeFace/mipmapLevel arguments the reference's null-desc RTV silently ignored (Direct3d11_TextureData.cpp:196); bind a correctly sized depth buffer for off-screen targets instead of the reference's 'depth only when the RT size exactly equals the screen' rule (Direct3d11.cpp:535-536), which rendered every other-sized RT depth-less.

- KEEP the reference's two genuine wins: its reproduction of D3D9's SetRenderTarget viewport auto-reset (:546-555, :604-614), which OMSetRenderTargets does not do; and the GPU-to-GPU bake copy that avoids gl05's synchronous GetRenderTargetData at Direct3d9_RenderTarget.cpp:302 plus D3DXLoadSurfaceFromSurface at :317 (once per frame per queued TextureRenderer, i.e. every character-face bake). Delete its 'return TRUE when no bake was in progress' paper-over (:622-623) and its unchecked destination-format assumption.

- Plumb TCF_renderTarget through TextureRendererTemplate::fetchCompatibleTexture (currently TextureList::fetch(0,...), so no RTV is possible today) per A13 - the reference sidestepped this with a scratch RT plus a copy, which works but keeps an extra full-surface copy per bake; read its success as evidence the engine change was never ATTEMPTED, not that it is optional. DEFAULT usage, never IMMUTABLE (BIND_RENDER_TARGET is illegal with IMMUTABLE); shader-blit fallback for destinations that cannot carry an RTV; Texture::copyFrom as a shader blit (CopySubresourceRegion can neither scale nor convert).

- Replace the blanket unbindAllPixelShaderResources (Direct3d11.cpp:497-503, called at the top of every setRenderTarget at :517, plus its duplicate inline copy in StaticShaderData) with unbinds of exactly the hazardous slots, driven by tracked RTV/SRV intent. Also close the z-fail shadow-volume far-cap question here rather than by C6's compare inversion or a global depth-clip toggle.

**Verify:** Texture-bake readbacks per frame = 0 on route R5 and blocking staging maps = 0 on all routes; a character-face bake pixel-diffs to the gl05 bake; a cube-face and a non-zero-mip render target both work (unreachable in the reference); the debug layer reports zero SRV/RTV hazards with the blanket 16-slot clear removed - which is the proof the underlying binding-lifetime bug was actually fixed rather than papered over.

### C17 — Submission-order fidelity: real texture sort key + index optimiser

- Implement a real getTextureSortKey from the bound textures - the reference returned 0 (Direct3d11_StaticShaderData.cpp:451-454), which collapses the engine's texture-batching sort so draw ORDER differs from gl05. That is a parity concern, not just a perf regression.

- Direct3d11_IndexOptimizer.{h,cpp} implementing Forsyth/Tipsify behind the previously-no-op optimizeIndexBuffer (Direct3d11.cpp:742 vs Direct3d9.cpp:4493-4520, which built a D3DXMesh and rewrote the caller's index array in place); single caller SoftwareBlendSkeletalShaderPrimitive.cpp:1311.

- Both change triangle/draw submission order, which is observable wherever blend order or z-equal coincident geometry matters, so they land and are gated TOGETHER.

**Verify:** Gate G1 per-phase draw and triangle counts within 2% of the DX9 baseline on all six routes; ACMR on a fixed reference set of skinned meshes within 5% of the D3DXMESHOPT_VERTEXCACHE result (the one regression class every API-level counter is structurally blind to); the Gate-C outlier list does not grow when the sort key goes live.

### C18 — Direct3d11_WindowPresenter for the nine presentToWindow consumers

- Per-HWND FLIP_SEQUENTIAL swap-chain cache compositing the SceneTarget, replacing D3D9's Present(NULL,NULL,hDestWindowOverride,NULL). The reference's presentToWindow ignored its hwnd/width/height and forwarded to present() (Direct3d11.cpp:449), so there is no prior art here at all.

- Add a return-value check plus a one-time WARNING at the Graphics::present(HWND,...) facade - present's bool return is unchecked at every one of the nine call sites, so a stubbed presenter blacks out every tool viewport silently.

**Verify:** All nine consumers render on gl11: five TerrainEditor views, Viewer, TextureBuilder, SwgGodClient, swgClientQtWidgets - and a deliberately broken presenter now produces a WARNING instead of a black viewport.

### C19 — Shared shader libraries with both DX9 source patches reproduced - the hand-off into the Phase-3 corpus

- Port shared_program/functions.inc, pixel_program/include/functions.inc, the 472-line vertex_program/include/functions.inc and terrain_dot3.inc.

- Reproduce BOTH behavioural DX9 patches as explicit commented HLSL, because our parity oracle is NOT stock: the c_ambient.inc rewrite `mov r7, vColor0` -> `add r7, vColor0, c16` (Direct3d9_VertexShaderData.cpp:110-172; c_ambient.inc is included by tfcl*, tfcsl*, c_simple*, envmask_specmap_c, c_detail_specmap, tfcl_env) and the `max(ambient + diffuse, 0.85)` diffuse floor injected into every //hlsl VS (:434-447). Without the first, every skinned character (vColor0 == 0) differs from the baseline by exactly the ambient term and it reads as a DX11 lighting bug. The reference compensated for their absence with LightManager tunables and PS +0.35f lifts instead, producing a different image by construction. Also record the _WIN64 D3DXSHADER_USE_LEGACY_D3DX9_31_DLL flag at :657 as a live warning about what -Gec must tolerate.

- Include the `if (!dot3) result += parallelSpecular[0]` term that both diffuse.inc and calculateDiffuseLighting contain, since the sun lives at c17/c18 under the DX9 selection policy.

- Then convert per program on the plan's draw-share order: a_simple.psh first (4,840 .sht), then skybox/skybox_6sided/stars/gradient_sky/cloudlayer, then dot3_terrain_imp1, then the mt_lerp/lerpmask/smap_mt/smt family. Only three reference replacements survive inspection as faithful (VS_SHADOWVOLUME, PS_SHADOWVOLUME, PS_TEX/PS_COLOR for 2d_texture/vertex_color) plus VS_GRADIENT_SKY and TEXREN_COPY_C1A1 modulo one dropped output each - salvage those five as .hlsl assets and re-derive everything else from the asset, treating the PEXE chunks as the behavioural oracle since DX9 runs the PEXE for //asm, not the PSRC text. Do NOT trust comments in the reference tree: several are demonstrably stale against that tree's own final state (VS_TFCSL's LightManager claim was reverted by its last commit; the 'alphaFadeOpacity is a stub=0' justification is contradicted at Direct3d11_LightManager.cpp:64-70/:355/:384).

**Verify:** Gate B per-file numerical equivalence as each conversion lands (VS outputs within 1e-5 relative, PS within 1 LSB of 8-bit, evaluated as pure functions over randomised inputs); a skinned-character frame matches the gl05 capture INCLUDING the ambient term, which is the specific proof that both source patches were reproduced; Gate B' catches the SM4-vs-SM2 COLOR0/COLOR1 clamp difference across the 84 writers that Gate B is structurally blind to.

## Accepted risks

- Point sprites stay unimplemented through the standup. All six slots route through C3's Direct3d11_Unimplemented accounting (counted + one WARNING + FATAL at DEBUG_LEVEL>=1) rather than the reference's silent `{}` at Direct3d11.cpp:430-435. D3D11 has no point-size/point-sprite rasterizer state, so parity genuinely requires geometry expansion; the only shipped consumer is StarAppearance.cpp:337-354, which sets only setPointSize and setPointSpriteEnable, and there is no setPointScaleEnable caller anywhere, so D3DRS_POINTSCALEENABLE stays FALSE and DX9 applies no attenuation. ACCEPTED: the star field is an ENUMERATED Gate-C outlier until the Phase-7 instanced-quad conversion. Risk bounded to one appearance in space scenes and it is visible in the log, not silent.

- supportsScissorRect reports FALSE between C3 and C6. ACCEPTED as an honest temporary degradation - the engine skips scissoring rather than enabling it and expecting clipping that never happens, which is what the reference did (true at Direct3d11.cpp:403 with a no-op at :669 and scissorEnable=false at :1003). Reversed in C6 when the state and the rasterizer flag both become real.

- The gamma/brightness/contrast composite is compiled but PINNED TO IDENTITY for every parity capture and for the shipped default. SetGammaRamp does nothing on a windowed D3D9 swap chain and the client ships windowed=1, so enabling the composite makes persisted non-default slider values (range 0.5-1.5, SwgCuiOptGraphics.cpp:170) newly effective. ACCEPTED as an intended fix per the plan, documented as a behaviour change affecting only users who moved the sliders, and kept out of the parity baseline so it cannot be confused with a port defect. Screenshots go through the same 256-entry table DX9 built, so captures still match.

- MSAA stays off (SampleDesc.Count=1) for the parity baseline, matching gl05's effectively-off behaviour. antiAlias/antiAliasSampleCount are carried in ConfigDirect3d11 and validated with CheckMultisampleQualityLevels but default to the measured gl05 state. ACCEPTED: enabling it is a Phase-7 item with its own gate, and the reference's `supportsAntialias(){return false;}` was at least honest about the same choice.

- Our Forsyth/Tipsify index order will not be byte-identical to D3DX's, so triangle order WITHIN a single skinned draw differs from DX9 by construction. ACCEPTED: order inside one index buffer cannot change output for opaque z-tested geometry; it can for coincident z-equal or blended triangles inside one draw, which is bounded, enumerated and triaged in Gate C, and the ACMR gate holds GPU vertex cost within 5% of the D3DX result.

- 'Byte parity' means parity with OUR PATCHED DX9, not stock DX9. Direct3d9_VertexShaderData.cpp:110-172 rewrites the shipped c_ambient.inc, :434-447 injects the max(ambient+diffuse, 0.85) floor, and :657 passes D3DXSHADER_USE_LEGACY_D3DX9_31_DLL on _WIN64. ACCEPTED and written into the baseline definition; C19 reproduces both patches as commented HLSL so the equality is intentional and reviewable rather than accidental.

- GfxStats reaches the backend through a second out-of-band export alongside GetGlApiStructSize rather than a Gl_api slot, which deviates from a literal reading of Phase 0's 'counters published across Gl_api'. ACCEPTED to preserve A1's byte-stable Gl_api through Phase 4; Headless.cpp:41-60's void** blanket fill over sizeof(Gl_api)/sizeof(void*) is the same reason the size guard itself cannot be a struct field.

- Deleting Gl_api::setDynamicIndexBufferSize changes sizeof(Gl_api) before Phase 4. ACCEPTED because it lands in P0-A while only gl05/gl06/gl07 and gl00 exist and all of them are edited in one commit; deferring it makes it a five-project lockstep edit and leaves Graphics.cpp:1697's unguarded NULL call live in the meantime (DX9 never assigns the slot).

- Direct3d11 ships x64-only while swg.sln:1607-1612 still advertises three Win32 solution configurations, so a Win32 solution build SILENTLY SKIPS the renderer instead of failing. ACCEPTED as a recorded decision per plan :544, and no Win32 ProjectConfigurationPlatforms rows are added - rows referencing undeclared configurations break solution load.

- The plan's 'builds with no DirectX SDK installed' criterion cannot be proven at solution level until Phase 5 deletes DX9, because scripts/Build-X64Client.ps1:49 unconditionally sets $env:DXSDK_DIR for the surviving DX9 projects. ACCEPTED: it is proven instead by a standalone msbuild of Direct3d11.vcxproj with the variable cleared, written into C1's gate as a separate check.

- The reference's clientObject shadow work is NOT carried: the eight [ClientObject] keys at ShadowVolume.cpp:897-913 (shadowSkipPlanarVolumes=1), the five at ShadowManager.cpp:172-176 (shadowMinScreenSizePixels=2, shadowCasterCullMarginMeters=64 despite a '0 = stock' comment on its declaration, shadowMinCasterRadiusMeters=0.25), and the enlarged DPVS cull model through MeshAppearanceTemplate/MeshAppearance/DetailAppearance. ACCEPTED CONSEQUENCE: the flickering degenerate z-fail 'blade' on the Mos Eisley flag reproduces on gl11 exactly as it does on gl05 and appears in the Gate-C outlier list as a PRE-EXISTING DX9 artifact. Every one of those knobs changes DX9 output identically, so carrying any of them in the port's commit stream makes the parity gate unfalsifiable; if the fix is wanted it is a separate re-baselined engine commit after Gate G1, routed through ConfigClientObject's KEY_FLOAT/KEY_BOOL table, with the dead prism path deleted rather than converted to runtime branches and the always-on SHADOWINV diagnostic (ShadowVolume.cpp:1809-1836) behind DebugFlags.

- vs_4_0/ps_4_0 with an 11_0 feature-level floor (FATAL below), not the reference's vs_5_0/ps_5_0 and its silent 10_1/10_0 fallback. ACCEPTED per A4: the live corpus needs nothing from SM5 (no tessellation/compute/UAV, max sampler slot s4) and pinning one profile keeps the numerical-equivalence harness valid.

- Runtime shader compilation survives as an instrumented fail-safe (d3dcompiler_47.dll shipped) because StaticShader::setTextureCoordinateSet (StaticShader.cpp:401-415) is a public runtime mutator that changes the permutation key, so the 152-entry manifest is asserted closed, not proven closed. ACCEPTED with a CI-gated zero-compiles-per-frame counter and a manifest-miss counter, rather than offline-bake-only where a missed permutation renders nothing with no recovery.

- TF_RGB_555 is DECLINED rather than mapped, on the strength of the .dds census showing zero shipped X1R5G5B5 assets. ACCEPTED with a verification step in C10: if the sweep declines a format some runtime-created texture actually requests, the fallback is B8G8R8A8 through the existing TextureFormatInfo::setSupported path, never B5G5R5A1 with D3D9's ignored pad bit promoted to a live alpha bit.

## Parity risks found in the prior attempt

Each is either designed out by a commit above or accepted explicitly.

### [blocker] Retuned lighting constants in Direct3d11_LightManager. ms_ambientFloor=0.36 (Direct3d11_LightManager.cpp:56), ms_hemiBack=0.48 (:58), ms_hemiTangent=0.74 (:59), all overridable from [ClientGraphics/Lighting] at :105-108, applied at :254-263 and :314-320. Plus an 'interior gate' at :313-320 that suppresses the ambient floor entirely for zero-ambient POB cells — behaviour DX9 has no equivalent for at all.

**Cost:** Our DX9 oracle uses a hard 0.30 floor (Direct3d9_LightManager.cpp:559) and 0.65 tangent / 0.30 back (Direct3d9_LightManager.cpp:784-792). Every lit surface in the game differs in luminance from the gl05 baseline, so the parity gate can never pass and — worse — the residual delta looks like a DX11 lighting bug rather than a deliberate retune. The interior gate additionally changes which pixels get any floor at all, making the divergence scene-dependent and non-uniform.

**Instead:** Port Direct3d9_LightManager's literal 0.30f / 0.65f / 0.30f with no config indirection and no interior gate. If the retune is wanted, land it as a separate engine commit after parity gate G1 with its own re-baseline, and route it through a ConfigDirect3d11 accessor rather than raw ConfigFile in the light path.

**Evidence:** Direct3d11_LightManager.cpp:56,58,59,105-108,254-263,313-320 vs Direct3d9_LightManager.cpp:559 (minAmbient=0.3f), :784-792 (0.65f tangent, 0.30f back)

### [blocker] Synthesized second directional sun, on by default at 45% intensity and 30 degrees azimuth offset. Direct3d11_LightManager.cpp:413-432 fabricates lightData.parallel[1] by rotating parallel[0]'s direction about world-up when the world supplies only one parallel light.

**Cost:** There is no counterpart anywhere in Direct3d9_LightManager.cpp. This is exactly the 'modern lighting' the plan excludes, and it changes the shading of every outdoor surface. It also produces a false success signal during bring-up: the guard at :420 requires ms_currentLights.parallel[0] non-null, which the DX9-faithful swapLight cascade never satisfies for a single world directional (the sun lands in parallelSpecular[0]), so on the ordinary outdoor case c20-c23 are all zero and the feature silently does nothing — meaning anyone who 'validates' it is validating a no-op that will start firing the moment light selection changes.

**Instead:** Delete the block and the two config keys outright. Do not port secondSunIntensityPercent/secondSunOffsetDegrees into ConfigDirect3d11.

**Evidence:** Direct3d11_LightManager.cpp:413-432, guard at :420; config keys at :109-113; no equivalent in Direct3d9_LightManager.cpp

### [blocker] Knowingly inverted compare-function mapping. Direct3d11_StateCache.cpp:89-90 maps engine compare index 5 to D3D11_COMPARISON_GREATER_EQUAL and 6 to NOT_EQUAL. DX9's authoritative table maps 5 to D3DCMP_NOTEQUAL and 6 to D3DCMP_GREATEREQUAL (Direct3d9_ShaderImplementationData.cpp:33-34 within the Compare[] array at :31-42). The comment at :82-88 admits the divergence and records that the parity fix was tried and REVERTED because it regressed stencil-shadow artifacts.

**Cost:** The .sht/.eft asset data encodes the INDEX, so DX9's table order is the binary contract regardless of its own mislabeled enum comments. Any pass using compare 5 or 6 gets the opposite test — z or stencil comparisons inverted. Carrying this masks a different, unfound bug (almost certainly the depth-clip / z-fail far-cap issue that the disableDepthClip knob was also invented for), and it guarantees that whichever pass actually uses 5/6 diverges from gl05 forever.

**Instead:** Map 5 to NOT_EQUAL and 6 to GREATER_EQUAL, matching Direct3d9_ShaderImplementationData.cpp:31-42 index-for-index. Then find the real cause of the stencil-shadow regression — enumerate which shipped passes actually use compare 5/6 and in which slot before writing any code, and fix the z-fail volume closure properly rather than compensating with a wrong compare.

**Evidence:** Direct3d11_StateCache.cpp:82-90 vs Direct3d9_ShaderImplementationData.cpp:31-42

### [blocker] Alpha test is captured and then never read. Direct3d11_StaticShaderData.cpp:306-314 reads pass.m_alphaTestEnable + shader.getAlphaTestReferenceValue into m_alphaTestReferenceValue/Valid (declared Direct3d11_StaticShaderData.h:106-107); a grep of the entire 40-file tree shows those two members are written once and read nowhere. Direct3d11_ShaderImplementationData.cpp:68-70 still carries the 'must move into the pixel shader as clip() later' comment.

**Cost:** Every alpha-tested surface in the game renders wrong: foliage, fences, grates, hair cards, UI masks. DX9 sets D3DRS_ALPHAREF per draw at Direct3d9.cpp:3939/:3946 plus ALPHATESTENABLE/ALPHAFUNC in the implementation pass. This is not a deferrable gap — it is the single largest silent image divergence in the prior tree, and four of the hand-written replacement pixel shaders are written on the assumption that it stays broken.

**Instead:** Design alpha test and setAlphaFadeOpacity together as one increment, because DX9 modulates D3DRS_ALPHAREF by ms_alphaFadeOpacity.a at Direct3d9.cpp:3939. Land the plan's SwgPixelEpilogue cbuffer at b1 (dx11-port-plan.md:76: alphaTestRef8 + alphaTestEnable + fogColor + fogEnable) and emit clip() from the epilogue, with the ref quantised to 8 bits exactly as D3DRS_ALPHAREF is.

**Evidence:** Direct3d11_StaticShaderData.cpp:306-314, Direct3d11_StaticShaderData.h:106-107, Direct3d11_ShaderImplementationData.cpp:68-70 vs Direct3d9.cpp:3932-3946

### [blocker] setFog is an empty body still wired into the api table: Direct3d11.cpp:672 `void setFog(bool, real, const PackedArgb &) {}`, assigned at :1274. A full enumeration of every VS constant write in the prior tree (11 call sites total) shows nothing ever writes register c10.

**Cost:** DX9 uploads VSCR_fog (c10) as {0,0,density,density*density} at Direct3d9.cpp:3266-3267 on the VSPS path, plus D3DRS_FOGENABLE/FOGCOLOR/FOGVERTEXMODE(EXP2). c10 is therefore permanently zero on DX11, so every world vertex shader's fog term evaluates to zero and terrain/distant geometry lose atmospheric attenuation entirely. Compounding it, no replacement VS emits a FOG output and no replacement PS applies a fog blend, while every //asm world VS in the corpus writes oFog via fog.inc — so fog is missing backend-wide, not just in one slot.

**Instead:** Implement setFog to upload c10 with DX9's exact {0,0,density,sqr(density)} packing, and carry fogColor/fogEnable in the b1 SwgPixelEpilogue since D3D11 has no fixed-function fog unit. Determine the FOG interpolation modifier empirically against a DX9 RenderDoc capture before committing to a semantic — do not guess.

**Evidence:** Direct3d11.cpp:672,1274; all VS constant writes at Direct3d11.cpp:979, Direct3d11_ConstantBuffers.cpp:203,230,259,260, Direct3d11_LightManager.cpp:484,485, Direct3d11_StaticShaderData.cpp:367,374,385 — none is c10; vs Direct3d9.cpp:3245-3274

### [blocker] The 28-row pixel-shader and 9-entry vertex-shader replacement tables select by unanchored strstr on the TreeFile path, with hand-tuned ordering to dodge prefix collisions. Direct3d11_PixelShaderProgramData.cpp:346-391 and Direct3d11_VertexShaderData.cpp:312-332. Unmatched //asm programs get a WARNING and a null shader (Direct3d11_VertexShaderData.cpp:462-470, Direct3d11_PixelShaderProgramData.cpp:695-699), which bindDrawState then silently swallows.

**Cost:** Measured against the live corpus, the 9+28 selectors capture 37 of 94 asm VS and 53 of 130 asm PS, and for most captured files the substituted math is not the file's math. One row ('t.psh' -> PS_T_LIT at :108-111) catches 9 unrelated shaders and rewrites 8 of them, adding a vertex-colour multiply the asset does not have and destroying the alpha. 57 asm VS and 77 asm PS get nothing and their draws vanish — including terrain_dot3_blend0-3, terrain_base/layer, skybox, skybox_6sided, the saber and water families and the whole h_* character set. Substring selection is unfixable in principle: any new asset silently steals or loses a match.

**Instead:** Do not carry the tables or the mechanism. Author real versioned .hlsl assets keyed by exact filename (or better, replacing the .vsh/.psh in the shader pipeline), and make any unmatched //asm program a hard FATAL in non-PRODUCTION builds so a missing translation cannot fail open. Treat the shipped PEXE chunks as the behavioural oracle, since DX9 runs the PEXE for //asm rather than the PSRC text.

**Evidence:** Direct3d11_PixelShaderProgramData.cpp:346-391 (28-row table, strstr at :381-391), Direct3d11_VertexShaderData.cpp:312-332 (if-chain), :462-470 (warn+skip), Direct3d11_PixelShaderProgramData.cpp:695-699

### [blocker] Silent draw dropping is the default failure mode. bindDrawState returns false with no log when the VS bytecode is null, the vertex format count is 0, or CreateInputLayout failed (Direct3d11.cpp:1044-1050); issueDraw and issueDrawIndexed then just `return` (:1081-1082, :1095-1096). CreateInputLayout failure caches a null and warns without naming the shader or the format (Direct3d11_InputLayoutMap.cpp:203-211). getPixelShaderForVertexShader falls back to the generic (wrong-signature) PS on pairing failure (Direct3d11_PixelShaderProgramData.cpp:820).

**Cost:** Missing geometry produces no log line at draw time, which makes a pixel-diff gate meaningless: the diff shows a hole and there is nothing in warning.log to attribute it to. The generic-PS fallback is worse than nothing — D3D11 rejects a signature mismatch at draw and the frame quietly loses the surface. During bring-up this converts every shader translation defect into an unattributable black region.

**Instead:** Make each drop path loud: a once-per-shader WARNING naming the shader filename and the vertex format flags, plus a per-frame dropped-draw counter surfaced in the Metrics module. FATAL in non-PRODUCTION. Never fall back to a PS whose input signature does not match the bound VS — return null and count the drop.

**Evidence:** Direct3d11.cpp:1044-1050,1081-1082,1095-1096; Direct3d11_InputLayoutMap.cpp:203-211; Direct3d11_PixelShaderProgramData.cpp:815-820

### [blocker] The prior backend cannot produce a frame capture. screenShot returns false (Direct3d11.cpp:645), writeImage returns false (:752), lockBackBuffer/unlockBackBuffer return false (:445-446), and there is no WriteTga module in the project at all.

**Cost:** A byte-parity gate compares captured frames. A backend that cannot capture cannot be gated, so every parity claim about the prior tree is unfalsifiable by construction — including its own claims of success. DX9 implements all four for real (Direct3d9.cpp:2634 screenShot, :4617 writeImage, :2384/:2437 lock/unlock, using ms_colorCorrectionTable so captures match on-screen gamma) plus shared/WriteTga.cpp.

**Instead:** Land the capture path FIRST, before any comparison work. The plan's backend-owned offscreen Direct3d11_SceneTarget (dx11-port-plan.md:299) solves lockBackBuffer, presentToWindow, gamma, MSAA and screenshots together; build that, port WriteTga.{cpp,h} into src/shared, and re-add jpeg-static.lib for the GSSF_jpg path DX9 has at Direct3d9.cpp:2766-2823.

**Evidence:** Direct3d11.cpp:645,752,445,446; no WriteTga in the prior project's src/shared (MemoryManagerHook.cpp, PaddedVector.h, SetupDll.{cpp,h} only) vs Direct3d9.cpp:2634,4617,2384,2437

### [blocker] The vertex_shader_constants.inc textual surgery inverts DX9 semantics and destroys a constant register. Direct3d11_ShaderCompiler.cpp:90-95 does three blind replaceAll passes on the TRE asset text: `#pragma` -> `//pragma`, `const bool` -> `static const bool`, and `: register(b` -> `= true; //`.

**Cost:** Two distinct defects. (1) The b-register pass forces all eight `light_*_enabled` booleans to compile-time TRUE. DX9 never writes them at all — Direct3d9_StateCache::setVertexShaderConstants(int, const BOOL*, int) at Direct3d9_StateCache.cpp:542 has zero callers in the whole DX9 backend, so b0..b7 keep the D3D9 default FALSE. Any shader that ever branches on them accumulates lights DX9 skips. (2) The pragma pass comments out `#pragma def(vs, c95, 0.0, 0.5f, 1.0f, 1.4426950408889634f)` (the last line of the shipped include), which is the ONLY thing that would have supplied c95's literal defaults on DX11 — and nothing uploads c95 either. registers.inc:76-79 aliases c0_0/c0_5/c1_0/cLog2e to c95.xyzw, used by 9 asm modules, so any faithful asm->HLSL port reading those aliases gets zeros: every 0.5 bias, every 1.0 constant and the fog log2(e) exponent collapse.

**Instead:** Verified against the corpus: `light_*_enabled` is declared at vertex_shader_constants.inc:128-135 and referenced by no other file in the shader tree, so neutralise to `false` (matching D3D9's default), not `true`, and do it as an explicit -D define rather than a blind text edit on an asset. Separately, upload c95 = {0.0, 0.5, 1.0, 1/log(2)} and c49/c50/c51 = unitX/unitY/unitZ every frame exactly as Direct3d9_StateCache::setConstants does at Direct3d9_StateCache.cpp:230-244 (terrain_dot3.inc consumes the unit vectors).

**Evidence:** Direct3d11_ShaderCompiler.cpp:90-95; Direct3d9_StateCache.cpp:542 (no callers), :230-244 (c95 + unitX/Y/Z uploads); asset vertex_shader_constants.inc:128-135,138 and vertex_program/modules/registers.inc:76-79

### [blocker] Two DX9 source patches that OUR parity oracle applies are absent from the prior tree. (a) Direct3d9_VertexShaderData.cpp:110-172 rewrites the shipped c_ambient.inc, replacing `mov r7, vColor0` with `add r7, vColor0, c16`. (b) Direct3d9_VertexShaderData.cpp:434-447 source-patches every //hlsl VS, replacing `lightData.ambient.ambientColor + diffuseSpecular.diffuse` with `max(..., 0.85)`. A grep for c_ambient/vColor0/0.85 across the prior DX11 tree finds only comments.

**Cost:** Our DX9 baseline is not stock. Byte-parity with 'DX9' means parity with OUR patched DX9. Without (a), every skinned/character mesh (which has vColor0 == 0) loses the scene ambient term and differs from the baseline by exactly that amount — and it will read as a DX11 lighting bug. Without (b), every //hlsl vertex-lit surface loses the diffuse floor. The prior tree instead compensates elsewhere with LightManager ambient-floor tunables and PS +0.35f lifts, which produces a different image by construction.

**Instead:** Reproduce both patches in the DX11 shader chokepoint as the plan requires (dx11-port-plan.md:403), preferably as commented HLSL in the converted asset rather than a runtime textual rewrite, and note in the source that the DX9 oracle is patched so the equality is intentional. c_ambient.inc is included by tfcl*, tfcsl*, c_simple*, envmask_specmap_c, c_detail_specmap and tfcl_env.

**Evidence:** Direct3d9_VertexShaderData.cpp:110-172 (verified: AMBIENT_SEARCH 'mov r7, vColor0' -> AMBIENT_REPLACE 'add r7, vColor0, c16'), :434-447 (SEARCH/REPLACE with max(...,0.85)); asset vertex_program/modules/c_ambient.inc confirms the stock body

### [high] Two config escape hatches whose only purpose is to restore known-wrong global rasterizer state, read via function-local statics inside the hot state fetch. [Direct3d11] forceCullNone at Direct3d11_StateCache.cpp:291-293 overwrites CullMode with CULL_NONE for every rasterizer state; [Direct3d11] disableDepthClip at :302-303 sets DepthClipEnable=FALSE globally.

**Cost:** forceCullNone restores the 2026-06-04 blanket override that the comment at :281-290 documents as having caused gl11-only 'micro building shadows'. disableDepthClip changes rasterisation for every pass to work around z-fail shadow-volume far-cap clipping; the comment at :295-301 itself concedes 'Stock gl05/gl11 both leave clipping ON'. A knob that can silently move the whole image makes any parity result non-reproducible — a passing capture proves nothing unless the cfg is also pinned.

**Instead:** Drop both. If winding is wrong the fix belongs in the transform upload, not a cull override. If the z-fail far cap is clipping, close the volume geometrically or scope depth-clamp to the shadow passes via a real pass-level state, not a global toggle. Any diagnostic lever that can change output must be DebugFlags-gated and absent from Release, not a ConfigFile key.

**Evidence:** Direct3d11_StateCache.cpp:281-293, 295-303

### [high] A permanent per-draw, per-texture-stage diagnostic probe with per-call file I/O. Direct3d11_StaticShaderData.cpp:158-190 re-derives the texture's Direct3d11_TextureData a second time (duplicating the work at :133-149), reads its RTV and compares against Direct3d11Namespace::getCurrentRenderTargetView(); on a hit it snprintf's and calls dx11Trace, which does CreateFileA + SetFilePointer + WriteFile + CloseHandle per call (Direct3d11.cpp:1011-1030). It needs a private-namespace forward-declaration hack at :121-122.

**Cost:** Only the LOGGING is capped at 80 hits (:177-178); the resolve-and-compare runs unconditionally forever, in Release, for every texture stage of every draw. This is a per-frame cost the plan explicitly forbids, added to diagnose a bug that was already root-caused (the stale-SRV leak). It also makes any perf measurement on the prior tree meaningless.

**Instead:** Delete it. The hazard it detects belongs in the centralized resource-state intent the plan's DX12-prep rule calls for, validated once by the D3D11 debug layer, not by a shipped per-draw probe.

**Evidence:** Direct3d11_StaticShaderData.cpp:121-122,158-190; Direct3d11.cpp:1011-1030

### [high] drainDebugMessages runs in present() every frame (Direct3d11.cpp:942, body :887-932). It iterates ID3D11InfoQueue, does a per-message `operator new(length)` heap allocation, and opens/appends/closes logs\ext\dx11_debug.txt with a fresh CreateFileA per logged message. The debug layer is hardcoded off at :1397-1401 with dead activation code at :1410-1411, so ms_infoQueue is permanently null and the whole function is inert.

**Cost:** The function's own comment at :894-896 admits the per-message loop 'tanks the frame rate'. It is a per-frame cost the plan forbids that only appears free because the diagnostic it serves is disabled — so the moment anyone turns the debug layer on to investigate a parity defect, the frame rate collapses and the measurement is invalid. Also DeleteFileA on two relative paths at install (:1429-1430) puts file deletion in the runtime directory inside a graphics backend.

**Instead:** Use SetBreakOnSeverity with a deny-list plus the engine's Report facility. Gate the debug layer on a real `[Direct3d11] debugLayer` key defaulting on for DEBUG_LEVEL 1/2 (dx11-port-plan.md:295) instead of a literal `bool debugLayer = false;`. Never open a file handle per message, never delete files from the backend, and route all logging through warning.log rather than raw CreateFileA on CWD-relative paths.

**Evidence:** Direct3d11.cpp:887-932,942,1393-1411,1429-1430

### [high] An unconditional 16-slot PSSetShaderResources null-clear on every static-shader bind and every render-target transition. Direct3d11_StaticShaderData.cpp:476-480 (inside the ms_active-changed branch of apply) and Direct3d11.cpp:497-503 (unbindAllPixelShaderResources, called at the top of setRenderTarget at :517).

**Cost:** A blanket 16-slot clear per shader bind is a real API cost in the hot path against the plan's per-shader-change budget of <=10 calls and one batched PSSetShaderResources (dx11-port-plan.md:713). It is also a bandaid: the comments at :472-479 and :490-496 describe papering over a stale-SRV leak where the scene render target survived on t0 and got sampled as diffuse (cantina chair, bank floor). Carrying the clear means the underlying binding-lifetime bug stays unfound.

**Instead:** Null exactly the hazardous slots, driven by centralized resource-state intent — track which resources are simultaneously RTV and SRV candidates and unbind those specifically at the transition. Batch all of a shader's SRVs into one PSSetShaderResources call rather than the current one-slot-at-a-time PSSetShaderResources/PSSetSamplers per stage (Direct3d11_StaticShaderData.cpp:154,189).

**Evidence:** Direct3d11_StaticShaderData.cpp:472-480,154,189; Direct3d11.cpp:490-503,517

### [high] Per-draw Map(WRITE_DISCARD) of the FULL constant buffers. Direct3d11_ConstantBuffers::apply (:243-277) is called from bindDrawState on every draw (Direct3d11.cpp:1069). The transform combine at :249-262 writes c0-c7 and sets ms_vsDirty every draw; LightManager::selectLights (called at Direct3d11.cpp:1063) writes the whole lightData/extendedLightData block per draw (Direct3d11_LightManager.cpp:484-485). upload() then Maps the entire buffer with WRITE_DISCARD (:89-100) — 1536 bytes for VS (VS_REGISTERS=96) and 512 for PS.

**Cost:** This is precisely the failure mode the plan calls arithmetically fatal: 'without it the per-draw constant traffic is 12x DX9 and the parity gate is arithmetically unreachable' (dx11-port-plan.md:36). WRITE_DISCARD forces a full buffer rename per draw on both buffers, against a budget of <=256 constant bytes per draw, no per-draw Map on any buffer larger than 256 B, and no per-draw buffer rename (dx11-port-plan.md:715). VS_REGISTERS=96 also exceeds DX9's VSCR_MAX=68, so the shadow is larger than the contract it mirrors.

**Instead:** Adopt the CPU-shadow + register(cN)-into-$Globals-at-16N insight (which is genuinely correct and verified), but implement the plan's split: keep b0 = $Globals with c0-c7 VACATED, and put objectWorldCameraProjectionMatrix + objectWorldMatrix in a 128-byte SwgPerObject at b3 from a 256-byte-aligned WRITE_NO_OVERWRITE ring (dx11-port-plan.md:76,338,352). Add per-register dirty tracking so the flush is bounded by the reflected extent, not the full allocation, and derive the shadow size from VSCR_MAX plus an enumerated c95 tail rather than a magic 96.

**Evidence:** Direct3d11_ConstantBuffers.cpp:33-52 (VS_REGISTERS=96), :89-100 (upload, full-buffer WRITE_DISCARD), :243-277 (apply); Direct3d11.cpp:1063,1069; Direct3d11_LightManager.cpp:484-485; dx11-port-plan.md:36,76,338,352,715

### [high] Draw-time shader compilation and no bytecode cache anywhere. getPixelShaderForVertexShader (Direct3d11_PixelShaderProgramData.cpp:791-821) is called from bindDrawState on every draw (Direct3d11.cpp:1058-1062) and, on first encounter of a (PS,VS) pair, reflects the VS signature, builds paired HLSL and calls D3DCompile — inside the draw. Direct3d11_ShaderCompiler::compile (:162-214) has no include cache (unlike DX9's ms_includeCache at Direct3d9_VertexShaderData.cpp:72) and no disk bytecode cache. Both caches are keyed by raw ID3DBlob* (m_pairedByVs at :798; Direct3d11_InputLayoutMap Key::vsByteCode at Direct3d11_InputLayoutMap.cpp:31).

**Cost:** A D3DCompile inside a draw is a multi-millisecond hitch the plan forbids ('Zero runtime shader compiles on a warm cache', dx11-port-plan.md:380). With no cache, ~400 programs recompile on every load. The raw-pointer keys are a latent aliasing bug: a freed and re-allocated ID3DBlob at the same address returns a stale paired PS or a stale (possibly null) input layout. Targets are also vs_5_0/ps_5_0 (Direct3d11_VertexShaderData.cpp:523, Direct3d11_PixelShaderProgramData.cpp:754,809) where the plan specifies 4_0.

**Instead:** Keep the chokepoint shape and the two include-override surgeries (the pixel_shader_constants.inc engine layout at Direct3d11_ShaderCompiler.cpp:39-62 and the POINT_PREFIX at :25 are both correct and mirror DX9). Add an include cache and a keyed disk bytecode cache, warm all permutations at load, target vs_4_0/ps_4_0, key every cache by a stable identity (shader name + permutation key + content hash) rather than a blob address, and stop discarding fxc warnings on successful compiles (:210-211) — implicit truncation and register-packing warnings are exactly what a parity investigation needs.

**Evidence:** Direct3d11_PixelShaderProgramData.cpp:791-821,754,809; Direct3d11_ShaderCompiler.cpp:100-155,162-214,210-211; Direct3d11_InputLayoutMap.cpp:28-42; Direct3d11_VertexShaderData.cpp:523; dx11-port-plan.md:36,380

### [high] Two hand-tuned visual redesigns dressed as fixes. (a) PS_BLOOM_DOWNSAMPLE_FMT + buildBloomDownsampleSource (Direct3d11_PixelShaderProgramData.cpp:305-330) overrides the //hlsl asset 2d_downsample_4x4.psh with a luminance-threshold/soft-knee/intensity glow extractor, three magic constants read from client.cfg [ClientGame/Bloom] at :324-326 and sprintf'd into the source, taking precedence over both the asm and paired-HLSL paths at :682-683. (b) A `+0.35f` ambient lift baked into PS_A_SIMPLE (:139-142) and PS_MUL_MTA (:148-157).

**Cost:** (a) The stock asset accumulates `sample.rgb * sample.a` over 16 taps — an author-authored alpha glow mask. Replacing it with a luminance threshold is a post-process redesign, and it introduces a runtime source-formatting step plus a hard ConfigFile dependency in the backend DLL. (b) The 0.35 lift is a brightness deviation stacked on top of the VS replacements' missing dynamic lighting — a workaround on a workaround. Worse, it is applied inconsistently: mul_mta.psh and mul_mt.psh are byte-identical assets, yet only mul_mta gets the lift, so two identical draw paths now differ from each other as well as from DX9. a_simple.psh is the highest-traffic asm PS in the corpus.

**Instead:** Reject both. Translate 2d_downsample_4x4.psh faithfully (rgb*alpha, /16, author's alpha) and let bloom stay as SWG shipped it; if bloom is later wanted, that is a post-parity feature with its own baseline. Remove the +0.35f and fix the actual cause — the dropped dynamic interior lighting in the vertex path.

**Evidence:** Direct3d11_PixelShaderProgramData.cpp:285-292,305-330,324-326,682-683,139-142,148-157

### [high] getOneToOneUVMapping returns a flat 0,0,1,1 with UNREF'd dimensions and the comment 'DX9 needed a half-texel shift; DX11 texel centers are already aligned' (Direct3d11.cpp:984-993). DX9 returns a texel-CENTRE-to-texel-CENTRE mapping: u0 = 0.5/w, v0 = 0.5/h, u1 = (w-1+0.5)/w, v1 = (h-1+0.5)/h (Direct3d9.cpp:3552-3558).

**Cost:** The two differ by a half texel of offset AND a (w-1)/w scale — not just an offset. The comment conflates the D3D9 half-PIXEL rasterisation offset (a vertex-position concern, genuinely fixed in D3D10+) with texture coordinates (which are unaffected by that rule). Every UI blit and every post-process quad built from this is a candidate half-texel/scale mismatch, which is exactly the kind of sub-pixel divergence that fails a byte-parity gate everywhere at once while looking like nothing.

**Instead:** Settle this FIRST, before writing code against either convention, because it touches every UI element. Capture a known UI blit under gl05 in RenderDoc, read the actual UVs and the actual sampled texels, and reproduce that. If a rasterisation offset genuinely needs correcting, correct it at the vertex positions, not by redefining what this accessor returns.

**Evidence:** Direct3d11.cpp:984-993 vs Direct3d9.cpp:3552-3558

### [high] Brightness/contrast/gamma reimplemented as a full-screen back-buffer pass that both deviates numerically from DX9 and leaves device state clobbered and untracked. Shader at Direct3d11.cpp:171-181, pass at :287-335, called from present() at :941.

**Cost:** Four numerical deviations from DX9's 256-entry D3DGAMMARAMP (Direct3d9.cpp:2064-2083): a saturate() inserted BEFORE the pow at :178 that DX9 does not have; evaluation in full float per pixel instead of through a quantised 8-bit scanout LUT; alpha forced to 1.0 at :180 over the back buffer's alpha; and the 1:1 copy sampled with a LINEAR sampler (:212-219). It also populates no colorCorrectionTable, so screenshots could never match on-screen gamma. Separately, the pass ends with ms_postRaster (CULL_NONE), ms_postBlend, ms_postDepth (depth off), ms_postCb displacing the PS $Globals at b0, IASetInputLayout(0) and a back-buffer RTV with NO depth view bound — while ms_currentRtv/ms_currentDsv are never updated, so the tracked and actual bindings disagree across the frame boundary. clearViewport (:856-857) clears the TRACKED views, not the bound ones. Rasterizer state is only re-bound on setFillMode/setCullMode, so the next frame's first draws can run with CULL_NONE.

**Instead:** Keep the identity short-circuit (:409-419), which is sound. Reproduce DX9's curve through a 256-entry LUT texture so the quantisation matches, drop the extra saturate, preserve back-buffer alpha, sample with POINT, and populate a colorCorrectionTable for the capture path. Run the pass from the plan's Direct3d11_SceneTarget rather than copying the back buffer, and save/restore (or explicitly re-bind) every state the pass touches, including the tracked ms_currentRtv/ms_currentDsv.

**Evidence:** Direct3d11.cpp:171-181,205-219,222-241,287-335,409-419,856-857,941,999-1005 vs Direct3d9.cpp:2064-2083

### [high] Two texture format mappings diverge from DX9. Direct3d11_TextureData.cpp:42 maps TF_L_8 to DXGI_FORMAT_R8_UNORM ('shader reads .r'); :35 maps TF_RGB_555 to DXGI_FORMAT_B5G5R5A1_UNORM.

**Cost:** TF_L_8 is a hard sampling difference the plan explicitly forbids: D3DFMT_L8 samples (L,L,L,1) whereas R8_UNORM samples (R,0,0,1), D3D11 has no SRV component swizzle to fix it, and Texture.cpp's addConversion(TF_L_8, TF_L_8) leaves no fallback (dx11-port-plan.md:156-158). Any shader reading .g/.b/.a from an L8 texture gets zeros instead of the luminance/one. TF_RGB_555: D3D9 used X1R5G5B5 where the top bit is ignored; B5G5R5A1 makes it a live alpha bit, so a texture whose padding bit happens to be 0 becomes fully transparent.

**Instead:** Expand TF_L_8 to B8G8R8A8 on load (only 4 shipped textures use it, so the cost is nil) per dx11-port-plan.md:156. For TF_RGB_555, either force the alpha bit to 1 during the load repack or map to a format with no alpha channel; verify against a shipped 555 asset before choosing. Keep the CheckFormatSupport probe at :67-77 — that part is right.

**Evidence:** Direct3d11_TextureData.cpp:35,42,67-77; dx11-port-plan.md:156-158

### [high] Non-fatal resource-creation failures that produce a wrong image with no signal. Direct3d11_TextureData.cpp:186-189 sets m_srv = 0 on CreateShaderResourceView failure; :195-200 sets m_rtv = 0 on CreateRenderTargetView failure; :164-183 falls back to a blank B8G8R8A8 texture at the real dimensions when no runtime format is supported, with a WARNING and the comment 'data may be wrong, but the client boots'. Direct3d11_ConstantBuffers.cpp:94-100 (upload) silently returns when Map fails.

**Cost:** A null SRV samples black; a null RTV makes setRenderTarget fall through to a different path; the blank-texture fallback boots with visibly wrong content. A silently-skipped constant upload leaves stale transform/light constants, so the object draws in the wrong place with the wrong lighting. Every one of these is a category of broken frame that a pixel diff will flag and that no log will explain. Note the state-cache and buffer classes get this exactly right (FATAL on failure, with correct reasoning about D3D11 default-state fallbacks, at Direct3d11_StateCache.cpp:196-199,249-253,313-316,361-363 and Direct3d11_StaticVertexBufferData.cpp:51) — the discipline is inconsistent, not absent.

**Instead:** Extend the state-cache FATAL policy (and its comments, which are worth carrying verbatim) to every resource creation and to Map: a failed SRV/RTV/texture/constant upload cannot degrade gracefully in a parity build. Add a local HRESULT formatter to replace DX9's FATAL_DX_HR/DXGetErrorString9. Also fix Direct3d11_TextureData.cpp:200-203, where WARNING(true, ...) fires on EVERY render-target creation including successes and pollutes warning.log — the first file anyone triages.

**Evidence:** Direct3d11_TextureData.cpp:164-203; Direct3d11_ConstantBuffers.cpp:89-100 vs Direct3d11_StateCache.cpp:196-199,249-253,313-316,361-363

### [high] Four separate sampler-filter deviations in one function, plus a hardcoded anisotropy cap. Direct3d11_StateCache.cpp:125 `filterIsLinear(f) { return f == 2 || f == 4 || f == 5; }` and :332-350.

**Cost:** Verified against DX9's authoritative TextureFilter[] table (Direct3d9_StaticShaderData.cpp:57-67): TF_flatCubic(4) and TF_gaussianCubic(5) map to D3DTEXF_NONE in DX9 but are treated as LINEAR here; TF_none(0) mip maps to D3DTEXF_NONE in DX9 (mipmapping OFF, which in D3D11 requires MaxLOD=0, not a filter bit) but :340 treats it as 'linear-ish'; TF_invalid(6) maps to D3DTEXF_LINEAR in DX9 but falls through to POINT here. MaxAnisotropy is clamped to a literal 16 at :349 where DX9 clamps to the device cap via Direct3d9::getMaxAnisotropy() (Direct3d9_StaticShaderData.cpp:200). Filtering differences are diffuse, low-contrast and everywhere — the hardest class of parity defect to attribute.

**Instead:** Port DX9's TextureFilter[] as a literal index table rather than predicate functions, so the mapping is auditable index-for-index. Implement TF_none mip as MinLOD=MaxLOD=0 on the sampler, not a filter bit. Clamp MaxAnisotropy to the real device capability.

**Evidence:** Direct3d11_StateCache.cpp:125,332-350 vs Direct3d9_StaticShaderData.cpp:57-67 (verified table: TF_none=NONE, TF_point=POINT, TF_linear=LINEAR, TF_anisotropic=ANISOTROPIC, TF_flatCubic=NONE, TF_gaussianCubic=NONE, TF_invalid=LINEAR), :200

### [high] State-cache lookups build a heap-allocating std::string from the raw descriptor bytes and walk a std::map<std::string,...> on every fetch. descKey at Direct3d11_StateCache.cpp:98-101; call sites :189, :242, :306, :354. getRasterizerState is reached from setFillMode/setCullMode (Direct3d11.cpp:428-429), which the engine calls unconditionally — Graphics::setCullMode has no unchanged-value early-out (Graphics.cpp:852-857).

**Cost:** D3D11_BLEND_DESC is 264 bytes and D3D11_SAMPLER_DESC 52, all far past the small-string optimisation, so each lookup is a malloc + memcpy + red-black-tree walk with full-length memcmp per comparison. Blend/depth-stencil/sampler are fetched at construct time so those are load-time only — but the rasterizer is fetched per setCullMode call, and setCullMode is invoked in save/restore pairs from hot paths (ShadowVolume.cpp:511,604,694,696,787; IndexedTriangleListShaderPrimitive.cpp:130,135; RibbonAppearance.cpp:212,214; DynamicMeshAppearance.cpp:237,241). That is a heap allocation per shadow volume and per ribbon/list draw, where DX9's equivalent is a cached integer compare in Direct3d9_StateCache::setRenderState.

**Instead:** Key the caches on a packed integer or a fixed-size POD struct with a hash map, no allocation. Better, follow the plan's proto-PSO direction: resolve all four state objects once per shader-implementation pass at load and keep a device-side 'currently bound' shadow so a redundant setCullMode is a single integer compare with no lookup at all.

**Evidence:** Direct3d11_StateCache.cpp:96-101,189,242,306,354; Direct3d11.cpp:428-429,999-1005; Graphics.cpp:852-857

### [high] Static vertex and index buffers use D3D11_USAGE_DEFAULT plus a permanently retained CPU shadow, uploaded with a full-buffer UpdateSubresource on every unlock. Direct3d11_StaticVertexBufferData.cpp:39-51 (shadow alloc + DEFAULT buffer), :84-99 (lock returns the shadow, unlock UpdateSubresource with pDstBox = 0); Direct3d11_StaticIndexBufferData.cpp:26-38, :57-76.

**Cost:** UpdateSubresource with a null destination box re-uploads the ENTIRE buffer even when the engine relocked one vertex, and the shadow is never freed, so every static mesh in the world carries a permanent duplicate in system memory. The plan requires IMMUTABLE + pSysMem as the default and calls IMMUTABLE strictly faster than DX9's MANAGED (dx11-port-plan.md:156-158, 354). Also `new unsigned char[allocSize]` is uninitialised, so a read-only lock on a never-written buffer returns heap garbage where DX9's MANAGED lock returns the real contents.

**Instead:** Per dx11-port-plan.md:354: IMMUTABLE + pSysMem where the buffer is never relocked (the overwhelming majority), DEFAULT + a staging buffer only where lock(readOnly) must return readable memory, and free the shadow after the single upload in the IMMUTABLE case. Where a shadow must persist, track the dirty range and pass a real D3D11_BOX. Do keep the monotonic-id sortKey (Direct3d11_StaticVertexBufferData.cpp:35) — it correctly avoids DX9's reinterpret_cast<int>(ptr) x64 truncation — and the store-VertexBufferFormat-by-value decision (Direct3d11.cpp:341-346), whose stated rationale about stack-local temporary DynamicVertexBuffers is sound.

**Evidence:** Direct3d11_StaticVertexBufferData.cpp:33-51,84-99; Direct3d11_StaticIndexBufferData.cpp:26-38,57-76; dx11-port-plan.md:156-158,354

### [high] Semantics quietly dropped in still-wired api slots. setScissorRect is `{}` at Direct3d11.cpp:669 while supportsScissorRect() returns TRUE at :403, and updateRasterizerState always passes scissorEnable=false (:1003) so D3D11_RASTERIZER_DESC::ScissorEnable is never TRUE either. All six point-sprite setters are `{}` at :430-435 (assigned :1240-1245). setAlphaFadeOpacity is `{}` at :679 and setBloomEnabled `{}` at :746, with Direct3d11_LightManager.cpp:64-70 hardcoding VectorRgba(0,0,0,1) in their place. optimizeIndexBuffer is `{}` at :742. getTextureSortKey returns 0 (Direct3d11_StaticShaderData.cpp:451-454).

**Cost:** supportsScissorRect()==true while setScissorRect does nothing is worse than reporting false: the engine enables clipping and expects it. Point sprites: DX9 sets six real render states (Direct3d9.cpp:2169,2177,2185,2193,2199,2206); D3D11 has no such rasterizer state, so parity genuinely requires geometry expansion — this is a design gap, not an omission. Alpha fade being a no-op kills all object dissolve/fade-in and, via Direct3d9_LightManager.cpp:654, the bloom flag too; and the justifying comment in the replacement shaders ('alphaFadeOpacity is a STUB=0') is FALSE in that same tree, since LightManager writes 1.0 into that slot. optimizeIndexBuffer and getTextureSortKey both change SUBMISSION ORDER, which is observable wherever blend order or z-equal coincident geometry matters — so neither is parity-safe as a no-op.

**Instead:** Implement scissor for real or return false from supportsScissorRect — never both. Implement point sprites via VS/GS quad expansion, and treat it as scheduled work with its own A/B, not a stub. Plumb setAlphaFadeOpacity and setBloomEnabled into the b1 epilogue alongside alpha test (they are the same increment). Port DX9's vertex-cache optimisation or accept it as a measured, gated order change; derive a real texture sort key. Re-derive every comment in the prior tree from the asset before trusting it — several are demonstrably stale relative to the same tree's final state.

**Evidence:** Direct3d11.cpp:403,430-435,669,679,742,746,1003,1240-1245; Direct3d11_StaticShaderData.cpp:451-454; Direct3d11_LightManager.cpp:64-70,355,384 vs Direct3d9.cpp:2169-2206,3129-3143,3455-3488,4493-4520 and Direct3d9_LightManager.cpp:599-609,654

### [high] getVideoMemoryInMegabytes returns a hardcoded 2048 (Direct3d11.cpp:392, '// TODO Phase 3: query DXGI adapter desc'). getShaderCapability returns a hardcoded ShaderCapability(2,0) set at :1423-1425.

**Cost:** The memory number is not cosmetic: SetupClientGraphics.cpp:95 gates DOT3 at >=40 MB and :101/:107 gate POST and HEAT at >=100 MB off this value, so a fake 2048 can select a DIFFERENT SHADER IMPLEMENTATION SET than DX9 would on the same adapter. It also drives PRODUCTION resolution clamps (Direct3d9.cpp:1156-1168). The 2,0 hardcode is probably the right answer for parity — it keeps DOT3/POST/HEAT paths selected as gl05 does, matching dx11-port-plan.md:A3 — but it is asserted, not verified.

**Instead:** Query DXGI_ADAPTER_DESC.DedicatedVideoMemory and publish it to CrashReportInformation as DX9 does at Direct3d9.cpp:1153. Confirm ShaderCapability(2,0) by reading what gl05 actually reports on the reference hardware before relying on it, and keep the `==` test at ShaderImplementation.cpp:552 untouched per plan A3.

**Evidence:** Direct3d11.cpp:392,1423-1425; SetupClientGraphics.cpp:95,101,107; Direct3d9.cpp:1150-1168

### [high] The prior backend cannot be built or run in a _DEBUG / DEBUG_LEVEL configuration. Direct3d11.cpp:1204-1352 (fillApiTable) assigns 113 of the 118 Gl_api slots and omits exactly the five #ifdef _DEBUG slots, which it never even defines: setTexturesEnabled, showMipmapLevels, getShowMipmapLevels, setBadVertexBufferVertexShaderCombination, getRenderedVerticesPointsLinesTrianglesCalls. There is no Direct3d11_Metrics module (DX9 ships Direct3d9_Metrics.{cpp,h}, 313+121 lines, ~60 counters), zero DebugFlags registrations (DX9 has 24), and zero CrashReportInformation calls (DX9 has six).

**Cost:** Graphics.cpp:1158, :1208 and :1266 wrap three of the five in NOT_NULL(), so a DEBUG_LEVEL 1/2 build hard-fatals. That eliminates the Optimized and Debug configurations — exactly the builds a parity investigation runs in. With no Metrics there are no draw/state/constant counters, so neither the parity gate nor the perf gate has evidence; with no DebugFlags there are none of DX9's 24 bisect levers (disablePass0..3+, minimumDrawPrimitives*, noTextures, lightingFullAmbient, forceAllLightsOn/Off, disableStaticShaderCaching...) which are how you narrow a divergence; with no crash-report text a field crash carries no adapter/driver identity. Empty pixSetMarker/pixBeginEvent/pixEndEvent (:748-750) also flatten every RenderDoc capture into an undifferentiated draw list — actively sabotaging the parity workflow the markers exist to serve.

**Instead:** Port Direct3d9_Metrics to Direct3d11_Metrics and implement all five _DEBUG slots so DEBUG_LEVEL 1/2 builds. Register a Direct3d11 DebugFlags section mirroring DX9's 24 levers. Wire the three pix* slots to ID3DUserDefinedAnnotation in ALL configurations (dx11-port-plan.md:295) — it is trivial and it is the instrumentation a RenderDoc A/B campaign runs on. Add the six CrashReportInformation lines. Note DX9's own Metrics is #ifdef _DEBUG with its report caller inside a #if 0, so the always-on GfxStats work on the DX9 side is a prerequisite for either gate.

**Evidence:** Direct3d11.cpp:1204-1352,748-750; Graphics.cpp:1158,1208,1266; Direct3d9.cpp:1004-1008,1531-1554,4572-4599; Direct3d9_Metrics.h:30-99

### [high] The prior tree's largest engine-side change is a cluster of shadow-visual tuning in clientObject that ships enabled by default: eight [ClientObject] keys in ShadowVolume.cpp:897-913 (shadowSkipPlanarVolumes=1), five in ShadowManager.cpp:172-176 (shadowMinScreenSizePixels=2, shadowCasterCullMarginMeters=64, shadowMinCasterRadiusMeters=0.25), plus an enlarged DPVS cull model threaded through MeshAppearanceTemplate / MeshAppearance / DetailAppearance. All read raw ConfigFile inline rather than going through ConfigClientObject (which is byte-identical between the trees).

**Cost:** Every one of these changes DX9 output identically — they move the baseline itself. If any lands in the same commit stream as the DX11 backend the parity gate becomes unfalsifiable: a divergence can no longer be attributed to the renderer. Three ship ON, and shadowCasterCullMarginMeters is declared 0.f with a '0 = stock' comment while install() reads it with default 64.f — so the declaration and the comment both say off while the feature is on, inflating every shadow-casting mesh's DPVS cull volume by 64 m. The file also carries an always-on SHADOWINV diagnostic (ShadowVolume.cpp:1809-1836: snprintf + std::set<std::string> find/insert + O(primitives) scan + O(vertices) AABB scan per volume per frame, outside any _DEBUG guard) and converts the stock compile-time SHADOW_EXTRUDE_TO_POINT #if arms into runtime branches in the innermost per-edge extrusion loop — in service of a prism path that is dead code (m_usePrismExtrude forced false at :1849, never set true).

**Instead:** Carry none of it into the port. If the flag-shadow fix is wanted, land it as a separate engine commit with its own re-baseline AFTER parity gate G1, route the keys through ConfigClientObject's existing KEY_FLOAT/KEY_BOOL table, fix the 0.f-vs-64.f contradiction, delete the dead prism path rather than making it runtime, keep our compile-time SHADOW_EXTRUDE_TO_POINT guards, and put any inventory diagnostic behind DebugFlags. Also reject the getVolumeBoundingRadius cache (ShadowVolume.cpp:2027), which is written once and never invalidated despite a comment claiming per-frame refresh — for skeletal casters it freezes the first pose's extent.

**Evidence:** ShadowVolume.cpp:897-913,1809-1836,1849,2027; ShadowManager.cpp:95,172-176; ConfigClientObject.cpp:113

### [medium] Device-loss handling is absent while presenting as implemented. The four add/remove callback slots are assigned (Direct3d11.cpp:1220-1223) and the registries maintained (:397-400, :771-783), but ms_deviceLostCallbacks / ms_deviceRestoredCallbacks are never iterated anywhere in the tree. wasDeviceReset() returns false forever (:395), isGdiVisible() false (:394), displayModeChanged() is `{}` (:388), flushResources() is `{}` (:393). present() detects DXGI_ERROR_DEVICE_REMOVED/RESET at :944-949 and only writes a trace line.

**Cost:** The engine's device-lost contract is silently unfulfilled: a registry written and never read. After a real device removal nothing re-uploads resources and nothing recovers — the client renders garbage or nothing with one line in a temp log file. DX9 implements all of these (Direct3d9.cpp:1651, :1708, :1715, :1722-1731).

**Instead:** Either invoke the registries on a real DXGI device-removal path with resource re-creation, or do not accept registrations — a half-implemented contract is worse than an honest refusal. Implement displayModeChanged as at minimum a swapchain re-check on WM_DISPLAYCHANGE. flushResources' EvictManagedResources half is legitimately n/a on D3D11, but the fullReset half (used on video-settings changes) must be honoured.

**Evidence:** Direct3d11.cpp:388,393-400,771-783,944-949,1220-1223 vs Direct3d9.cpp:1651,1708,1715,1722-1731

### [medium] install ignores five Gl_install fields DX9 honours and hardcodes the swap chain. Direct3d11.cpp:1358-1439 reads only window/width/height/windowed/engineOwnsWindow/borderless/x/y; colorBufferBitDepth, alphaBufferBitDepth, zBufferBitDepth and stencilBufferBitDepth are dropped (depth hardcoded D24_UNORM_S8_UINT at :815), skipInitialClearViewport is dropped, and gl_install->windowedModeChanged is never stored (DX9 stores it at Direct3d9.cpp:983). scd.Windowed = TRUE unconditionally at :1386 and SetFullscreenState is never called. BufferCount 2 + SWAP_EFFECT_DISCARD (:1376,:1387) vs DX9's windowed D3DSWAPEFFECT_COPY (Direct3d9.cpp:2095). RefreshRate hardcoded 60/1 (:1380-1381). Present(0,0) always (:943). Feature levels accept 10_1/10_0 (:1391). Back buffer R8G8B8A8 while the bake scratch is B8G8R8A8 (:1379 vs :474).

**Cost:** Exclusive fullscreen does not exist, so the shipped user setting [Direct3d9] fullscreenRefreshRate=120 in _client/options.cfg is silently discarded and DXGI's SetGammaControl is unavailable (which is why gamma had to become a shader pass). The engine is never notified of backend-initiated windowed-mode changes. Present(0,0) is always no-vsync where DX9 selects the interval from ConfigDirect3d9::getAllowTearing() (Direct3d9.cpp:2118-2121) — that alone invalidates any frame-pacing comparison. Accepting a 10_0 device silently ships a renderer below the 11_0 floor the plan requires. Mixed channel order across the tree is a latent copy/format mismatch.

**Instead:** Honour all four bit-depth fields and skipInitialClearViewport; store windowedModeChanged and call it. Implement real fullscreen and drive the refresh rate from ConfigDirect3d11. Select the present interval from an allowTearing key. FATAL below feature level 11_0 (dx11-port-plan.md:295). Pick one channel order and use it everywhere.

**Evidence:** Direct3d11.cpp:815,943,1358-1439,1376-1391,1379,474 vs Direct3d9.cpp:983,2095,2105,2118-2121; _client/options.cfg:20-21

### [medium] Fan/quad emulation creates GPU resources inside the draw path, clobbers the bound index buffer, fails silently, and leaves the indexed fan variants entirely unimplemented. Direct3d11.cpp:1112-1157 (issueQuadOrFanList) does Release + CreateBuffer at draw time whenever primitiveCount exceeds capacity, `return`s with no log on CreateBuffer failure (:1149-1150), and IASetIndexBuffer's its own buffer at :1154 relying on the engine to re-bind before the next indexed draw. drawIndexedTriangleFan (:726) and drawPartialIndexedTriangleFan (:740) are literally `{}` yet assigned at :1316 and :1330.

**Cost:** Draw-time buffer creation violates the plan's 'zero state-object / input-layout / cbuffer / shader creations per frame' budget (dx11-port-plan.md:380) and is a frame hitch. The silent return drops geometry with no signal. The index-buffer clobber is an undocumented ordering assumption that will break the moment a caller issues two indexed draws around a fan. The empty indexed-fan slots silently drop every indexed fan DX9 draws via D3DPT_TRIANGLEFAN (Direct3d9.cpp:4254, :4416). 16-bit indices also cap a batch at ~16k quads / 64k fan verts with no check.

**Instead:** Keep the conversion approach and the (0, i+1, i+2) winding, which is correct. Pre-create both index buffers at install sized to the worst measured batch, FATAL on creation failure, save and restore the engine's bound index buffer around the draw (or route all index binding through backend-tracked state), implement the two indexed fan variants, and assert the 16-bit capacity rather than overflowing silently.

**Evidence:** Direct3d11.cpp:726,740,1112-1157,1316,1330 vs Direct3d9.cpp:4254,4416; dx11-port-plan.md:380

### [medium] Register numbers baked into shader data files are duplicated as local enums and bare integer literals in four independent places, and the shadow exceeds the contract. Direct3d11_ConstantBuffers.cpp:33-41 declares a local anonymous enum naming only 5 of ~24 VSCR entries; Direct3d11_StaticShaderData.cpp:33-40 and Direct3d11.cpp:978 hardcode c9/c11/c44/c47 as integers. VS_REGISTERS=96 (Direct3d11_ConstantBuffers.cpp:46) against DX9's VSCR_MAX=68. There is no DX11 counterpart to Direct3d9_VertexShaderConstantRegisters.h, Direct3d9_PixelShaderConstantRegisters.h, Direct3d9_VertexShaderVertexRegisters.h or Direct3d9_VertexShaderUtilities.h.

**Cost:** Those headers' own comments warn three times each that the register numbers are baked into the shipped shader binaries. Four uncoordinated copies of an ABI that must never drift is a defect generator: a single edit in one place produces a wrong constant in a shader with no compile error and no runtime error, only a wrong image. The oversized shadow also means the flush moves 1536 bytes where the contract is 68 registers.

**Instead:** Promote the register enums to a single shared header (dx11-port-plan.md A20 puts them in clientGraphics/include/public as ShaderConstantRegisters.h) and have both backends and ShaderBuilder include it — no local enums, no bare integers anywhere. Derive the shadow size from VSCR_MAX plus an enumerated c95 tail per dx11-port-plan.md:352, not a magic 96.

**Evidence:** Direct3d11_ConstantBuffers.cpp:33-46; Direct3d11_StaticShaderData.cpp:33-40; Direct3d11.cpp:978; dx11-port-plan.md:352 and A20

### [medium] The replacement path deletes the engine's texture-coordinate-set permutation tags. Direct3d11_VertexShaderData.cpp:405-416 sets m_hlsl=true then `delete m_textureCoordinateSetTags; m_textureCoordinateSetTags = 0;`. Separately, getMaximumVertexBufferStreamCount returns 16 (Direct3d11.cpp:744) while setVertexBufferVector caps at MAX_STREAMS=2 (:1569) and Direct3d11_InputLayoutMap uses MAX_VERTEX_BUFFERS=2 with only a DEBUG_FATAL on overflow (Direct3d11_InputLayoutMap.cpp:24,36).

**Cost:** Dropping the tags discards the tag-to-physical-set contract (the vTextureCoordinateSet<TAG> defines parsed at :375-393 and the 24-bit key applied at :478-508, mirroring Direct3d9_VertexShaderData.cpp:676-732). The replacements hardcode TEXCOORD0/TEXCOORD1, so any asset binding DETA/SPEC/MASK to a set other than 0/1 reads the wrong stream — verified against the assets: tfcl_5uv writes five oT registers, c_detail_specmap declares MAIN/DETA/SPEC, and both are served by a single-UV replacement. Also, with getCompiled short-circuiting on a null tag list (:550-558), all permutations collapse to one compile. Reporting 16 streams while handling 2 is a Release-mode silent truncation, since DEBUG_FATAL is a NOP in Release.

**Instead:** Keep the tags and honour the 24-bit key exactly as Direct3d9_VertexShaderData.cpp:676-732 does for the asm path — any faithful rewrite of tfcl_2uv..5uv, tfal_2uv..5uv, tfcsl_2uv, envmask_specmap_detail*, c_detail_specmap or c_simple_bump requires it. Report the real stream limit or raise the real limit, and make the overflow a hard failure rather than a debug-only assert.

**Evidence:** Direct3d11_VertexShaderData.cpp:375-393,405-416,478-508,550-558; Direct3d11.cpp:744,1569; Direct3d11_InputLayoutMap.cpp:24,36 vs Direct3d9_VertexShaderData.cpp:676-732

### [medium] There is no ConfigDirect3d11 module at all (no Config* file among the prior tree's 40 files). The entire [Direct3d11] surface is two lazily-initialised function-local statics inside a state-fetch function, both of which are behaviour toggles (Direct3d11_StateCache.cpp:291, :302). All 19 keys ConfigDirect3d9::install reads (ConfigDirect3d9.cpp:61-91) have zero DX11 counterpart.

**Cost:** Nothing is read at install, nothing is validated, nothing is logged, and there is no single place to audit backend configuration — so a parity capture cannot be shown to have been taken under a known configuration. Concretely lost: antiAlias (default TRUE, silently becomes false since SampleDesc.Count=1 is hardcoded), fullscreenRefreshRate (actually set to 120 in _client/options.cfg:20-21 and silently discarded), allowTearing (default false, silently becomes always-tearing), createShaders and shaderCapabilityOverride (the two main bisect levers for a shader-selection divergence), dynamicIndexBufferSize (live in DX9 at Direct3d9_DynamicIndexBufferData.cpp:35; ring sizing changes DISCARD cadence, which is what the perf gate measures).

**Instead:** Author a real ConfigDirect3d11.{h,cpp} that reads every key once in install with FATAL validation on out-of-range values (DX9 does exactly this for vertexProcessingMode at ConfigDirect3d9.cpp:90) and publishes to CrashReportInformation. Carry adapter, antiAlias, fullscreenRefreshRate, allowTearing, createShaders, shaderCapabilityOverride, both dynamic buffer sizes, discardDynamicBuffersAtBeginningOfFrame and screenShotBackBuffer; add debugLayer and featureLevelCap; drop the D3D9-caps-only keys. Note only the 3-arg getKeyInt/getKeyBool are DLLEXPORT (ConfigFile.h:148-149), so the percent-scaled-int idiom is a genuine constraint — a string key like shaderCachePath needs getKeyString exported first.

**Evidence:** Direct3d11_StateCache.cpp:291,302; ConfigDirect3d9.cpp:61-94; _client/options.cfg:20-21; ConfigFile.h:148-152; dx11-port-plan.md:301

### [medium] Build defects in the otherwise-excellent Direct3d11.vcxproj. The Optimized|x64 configuration is Optimization=Disabled + RuntimeLibrary=MultiThreadedDebug + BasicRuntimeChecks=EnableFastChecks (:147,151,152) — a verbatim copy of the Direct3d9.vcxproj:435,439,440 defect. A vestigial VSPS define survives (:80,115,149). Nine output-path literals hardcode a src/compile/win32/Direct3d11/<Config> layout that does not exist in our tree (:57-58,64-65,70-71,100,106-107,135,141-142,167,173-174). jpeg-static.lib and the libjpeg-turbo include are dropped. The project appears nowhere in the reference swg.sln.

**Cost:** Shipping an unoptimised debug-CRT renderer as 'Optimized' makes every perf number from that configuration meaningless. VSPS is a DX9-only FFP/shader source switch (Direct3d9.cpp:95,304,365,427) with no DX11 meaning; leaving it invites #ifdef VSPS code in a backend that has no such duality. The path literals are silently overridden by our Directory.Build.targets:4-5,36-42, so copying them yields a project that builds correctly but documents the wrong output location — a maintenance trap worse than a break. Without jpeg-static the JPEG screenshot path (Direct3d9.cpp:2766-2823) cannot be ported at parity. Without a swg.sln entry and a SwgClient ProjectDependencies edge, scripts/Build-X64Client.ps1's /t:SwgClient never builds gl11 at all.

**Instead:** Adopt what the file OMITS — zero $(DXSDK_DIR), no MachineX86/BaseAddress//SAFESEH:NO, no d3dx9/DxErr/ddraw, x64-only, correct four DX libs — that is the single most reusable artifact in the prior attempt. Then: set Optimized to Optimization=Full AND remove BasicRuntimeChecks (/O2 with /RTC1 is a hard D8016 error) while KEEPING MultiThreadedDebug + _DEBUG + DEBUG_LEVEL=1 so _ITERATOR_DEBUG_LEVEL stays 2 and matches SwgClient.vcxproj:281,283 (std::vector crosses the DLL boundary in setLights/getOtherAdapterRects); delete the VSPS define; delete all nine path literals and add the Directory.Build.targets TargetName row for gl11_$(SwgOutputSuffix); re-add jpeg-static.lib plus the libjpeg-turbo include; add the swg.sln Project() entry, six x64 ProjectConfigurationPlatforms rows, the SwgClient dependency edge, and gl11 entries in the four build/stage scripts. Author the ClCompile/ClInclude groups against the plan, not by copying: ConfigDirect3d11, Direct3d11_Metrics, Direct3d11_RenderTarget and WriteTga are all missing.

**Evidence:** Direct3d11.vcxproj:57-58,64-65,70-71,80,100,106-107,115,135,141-142,147,149,151-152,167,173-174 vs Direct3d9.vcxproj:206,231,435,439,440 and SwgClient.vcxproj:275,281,283; Directory.Build.targets:4-5,10-12,36-42; dx11-port-plan.md:201,203,289,301,307,325

## Immediate next actions

1. Land P0-A as one commit against gl05/gl06/gl07 plus Headless, while a second implementation does not yet exist: add the extern "C" uint32 GetGlApiStructSize() export to Gl_dll.def/Graphics.cpp and validate it immediately after GetApi with a FATAL, delete Gl_api::setDynamicIndexBufferSize (Gl_dll.def:184 + the unguarded call at Graphics.cpp:1695-1697 + Direct3d9.cpp:224/3545), and delete Gl_metrics (Graphics.def:37-103). Prove it by building a deliberately mismatched DEBUG_LEVEL pair and confirming it FATALs with a readable message instead of loading. Confirm and record that the other two items the plan lists here are already done in our tree: the lockBackBuffer rect assignment (Graphics.cpp:1049-1054) and the Direct3d11/Direct3d11_RenderTarget friend grants (Texture.h:34-35,111-112; Transform.h:28).

2. Land C1: author /e/SWG/64bit-server/client-tools/src/engine/client/application/Direct3d11/build/win32/Direct3d11.vcxproj from scratch adopting the reference project's OMISSIONS (zero $(DXSDK_DIR), no MachineX86/BaseAddress//SAFESEH:NO, no d3dx9/DxErr/ddraw, x64-only, the four SDK DX libs) while fixing its three defects (Optimized|x64 -> Optimization=Full with BasicRuntimeChecks REMOVED and MultiThreadedDebug KEPT; delete the VSPS define; delete all nine output-path literals) and re-adding jpeg-static.lib plus the libjpeg-turbo include; copy src/shared/{MemoryManagerHook,SetupDll,PaddedVector,WriteTga} verbatim from Direct3d9/src/shared; wire Directory.Build.targets:12, the swg.sln Project() block after :209, six x64 configuration rows, and the SwgClient ProjectDependencies line at swg.sln:706-772; add gl11 to the four build/stage scripts. Prove a standalone msbuild with $env:DXSDK_DIR cleared emits src/build/win32/x64/Release/gl11_r.dll, that /t:SwgClient also builds it, and that rasterMajor=11 in /e/SWG/64bit-server/_client/options.cfg:18 LoadLibrary's it and refuses to install with a log line.

3. Start the three DX9-side measurements in parallel with C2/C3, because each gates a later commit and none can be recovered cheaply once code is written against a guess: (a) P0-C, the m_zCompare / m_stencilCompare / m_stencilCompareCounterClockwise census over the 1,518 decoded passes through the swapped Compare table plus the blend-factor census over passes with alphaBlendEnable==false, which gates C6 and C13; (b) P0-D, one gl05 RenderDoc A/B on a known UI blit settling the one-to-one-UV / half-texel convention, which gates C12 and touches every UI element; (c) P0-E, one gl05 capture on a long fogged surface fixing the `: FOG` interpolation modifier, which gates C14 and ~90 shader pairs.

## Corrections made during implementation

The sequence above was written before any of it was built. These are the places
the tree disagreed with it, recorded here rather than silently edited above, so
the reasoning that produced the original plan stays reviewable.

### C3 cannot end with a cleared frame in the client

C3's stated exit criterion was that `rasterMajor=11` presents a cleared frame.
It cannot, and the reason is in the engine rather than in the backend.

`SetupClientGraphics::install` does not stop when `Graphics::install` returns. It
immediately gates DOT3, POST and HEAT on the reported shader capability and video
memory, then loads `texture/defaultcubemap.dds` and calls `setGlobalTexture`
twice, then preloads the vertex-colour shader templates. So `createTextureData`,
`setGlobalTexture`, `createShaderImplementationGraphicsData`,
`createStaticShaderGraphicsData`, `createVertexShaderData`,
`createPixelShaderProgramData` and `setBadVertexShaderStaticShader` are all
reached before the first frame is even attempted.

A cleared frame therefore requires the texture and shader factories, which belong
to later commits. What C3 delivers instead is: the device and swap chain come up,
the window is shown at the requested size, capabilities are reported honestly,
and the run then stops at the first resource factory with that slot named. That
is a real, verifiable milestone; it is just not a picture.

The "cleared frame presents" gate moves to the commit that first has a texture
and a shader to draw with.

### The Gl_api layout count was wrong in both directions

The audit and this document both said Gl_api has three binary layouts, one per
DEBUG_LEVEL. It has two. `_DEBUG` is defined for the Debug **and** Optimized
configurations, while `PRODUCTION` is 1 only for Release, so Optimized and Debug
share a layout and Release has its own. The size guard is unaffected -- it
compares actual sizes -- but the comment explaining it was misleading and has
been corrected in the code.

Separately, the slot count is 117, not 118 or 120. Parsing `Gl_dll.def`
mechanically yields 118 entries, one of which is the `CallbackFunction` typedef
rather than a slot.

### Video memory is not reported from Direct3D at all in DX9

`getVideoMemoryInMegabytes` was assumed to be a straightforward query to mirror.
DX9 creates a **DirectDraw** object and reads `DDCAPS.dwVidMemTotal`, defaulting
to 32 MB (`Direct3d9.cpp:1651-1657`). That number gates DOT3 at 40 MB and POST
and HEAT at 100 MB.

DirectDraw's reported total is unreliable on modern hardware, so the shipping DX9
client may well be running with POST and HEAT disabled without anyone having
chosen that. DXGI's `DedicatedVideoMemory` is the honest answer and will usually
be much larger, which means DX11 can switch those effects on and produce image
differences that are not port defects.

Consequence for the gates: what gl05 actually reports on the test hardware has to
be captured, not assumed, and gl11 has to be compared against observed gl05
behaviour. Enabling POST and HEAT is a deliberate change with its own
measurement, not a side effect of a renderer swap. It also adds a real `ddraw`
consumer to the Phase 5 deletion list, which had counted only a header leak.

### The plan was wrong about extendedLightData

C15's description says to drop the extendedLightData upload because it has no live
references and the C++ side covers only half its rows.

It is uploaded. `Direct3d9_LightManager` writes four rows at vertex c60..c63 from a
real `ExtendedLightData` struct wrapping a `HemisphericLightData`, with the row
count computed as `sizeof(ExtendedLightData) / (4 * sizeof(float))` rather than
written as a literal. Following the plan here would have deleted a live upload and
left four rows of whatever the register file previously held for any shader that
reads them.

Carried, not dropped. If it later turns out no shipped shader reads c60..c63, that
is a separate finding needing its own evidence, and the way to establish it is to
decode the corpus -- not to infer it from the C++ side looking incomplete.

### The bool registers confirm the prior attempt inverted shipped behaviour

The plan said the `register(bN)` light-enable booleans are never written, and that
is exactly right: `SetVertexShaderConstantB` has zero callers anywhere in the tree,
so those registers hold D3D9's device default of FALSE for the entire life of the
process. There is no `SetVertexShaderConstantI` wrapper either, so i-registers are
likewise never written.

The consequence is sharper than the plan stated. The prior DX11 attempt's textual
surgery rewrote those declarations from `register(bN)` to `= true`, which does not
merely neutralise them -- it INVERTS them, forcing every light-enable branch on
where the shipped behaviour is off. Any shader whose output depends on one was
rendering something DX9 never rendered.

### Only four constant rows survive a device rebuild

`Direct3d9_StateCache::restoreDevice` re-establishes exactly four rows: c95 and the
three unit vectors at c49..c51. c9 comes back indirectly because setViewport is
called immediately afterwards, and c0..c7 because the transform dirty flag is set.
Nothing re-uploads c8, c10, c11..c15, c16+, c44, c45, c47, c48, c52..c59 or c60+ --
they are rewritten by the ordinary drawing flow.

Reproducing that exact set matters more than reseeding everything would: it keeps
both backends agreeing about which constants are stale after a rebuild, which is
the difference between a comparable capture and a coincidence.

### The per-draw constant ring needs two capabilities, not one

The plan describes binding a constant ring "with VSSetConstantBuffers1 offsets on
11_1". That is two independent features, and both are optional on feature level
11_0 hardware even though the interface exposing them is 11_1:

  ConstantBufferOffsetting                 bind a sub-range of one buffer
  MapNoOverwriteOnDynamicConstantBuffer    append to a dynamic constant buffer
                                           without renaming it

A ring needs both. The device queries both at install, and the fallback to rotating
buffers with DISCARD names whichever one was missing, so a machine that is slower
for this reason says so instead of looking like a regression in whatever landed
most recently.

### drawPartial is overload resolution, not aliasing

C9 says DX9 "aliases every drawPartial* to its full-draw counterpart at
Direct3d9.cpp:1104-1116 and recovers the range from ms_slice* state". That reading
is wrong, and it is wrong in a way that produces garbage on screen.

`Direct3d9Namespace` declares each draw name TWICE -- a no-argument overload and a
parameterised one. The assignment `ms_glApi.drawPartialTriangleList = drawTriangleList;`
therefore selects the PARAMETERISED overload, by overload resolution against the
Gl_api slot's function-pointer type. It only looks like aliasing. The twelve partial
entry points have their own implementations, and they read nothing from the slice
counts -- the caller's start and primitive count are the range.

A port that writes one function per name and assigns it to both slots either passes
uninitialised arguments to the full versions or silently ignores the caller's range
in the partial ones. All 25 draw slots must be distinct functions for a second
reason too: the engine stores `Graphics::drawX` addresses in lookup tables and
compares them for identity, so they cannot be collapsed onto one another.

### The dynamic ring cannot be resized during a parity port

C9 says to resize the rings from 2 MB to 16 MB and 64 KB to 4 MB, citing terrain
batches that exhaust the current size.

ShadowVolume::install derives a permanent batch budget from the ring:

    const int numberOfLockableDynamicVertices = vertexBuffer.getNumberOfLockableDynamicVertices (true);
    ms_maximumVertexBufferSize = numberOfLockableDynamicVertices - (numberOfLockableDynamicVertices % 4);

`getNumberOfLockableDynamicVertices(true)` returns the WHOLE ring divided by the
vertex size. So the ring size sets shadow batch sizes, which set draw counts --
which is exactly what the geometry-integrity gate compares within 2%. Growing the
ring fails that gate by construction, and the cause would look like a shadow bug.

It also approaches a hard ceiling: `Index` is `unsigned short` throughout the engine,
so a batch above 65535 vertices cannot be addressed by any index buffer at all.

DX9 sizes the ring from video memory in tiers of 256 KB to 2 MB, and its
config-driven size is commented out, so the tiers are the only policy. That policy
is reproduced exactly. Resizing belongs in the performance phase with its own
measurement and its own re-baseline of draw counts.

### The buffer-touching slots must be class members

C9 describes the buffer slots as functions to implement, without noting where they
live. They cannot be namespace functions: `setVertexBuffer`,
`setVertexBufferVector` and `setIndexBuffer` read `m_graphicsData` and
`m_vertexBufferList` out of the engine's buffer objects, and the engine befriends
`class Direct3d11`. Free functions in a namespace have no friend access. DX9 has the
same three as class members with a comment saying precisely that.

The five buffer headers also granted friendship to `class Direct3d11` but not to the
per-buffer data classes, where DX9 grants both. Added.

### An accounted stub is the wrong tool for a value the engine consumes structurally

`getMaximumVertexBufferStreamCount` was left as an accounted stub returning zero.
The accounting facility makes a missing feature audible, which is right for a
feature -- but this value is read during install to size a per-stream shadow array
and gates the multi-stream skinned path on being above one. Zero sized that array to
nothing and silently disabled skinned multi-stream, and the counter said nothing
useful about either consequence.

The lesson generalises: a slot whose RETURN VALUE the engine builds structure from
needs a real answer from the first commit that can give one, even when the feature
behind it is unimplemented. Slots whose EFFECT is missing are what the accounting is
for.

### DX9's format table mismaps three formats, and parity means reproducing that

The plan treated the engine-format to DXGI mapping as a lookup with three awkward
entries (24-bit, palettised, luminance). Reading DX9's own table changed the answer.

`Direct3d9_TextureData.cpp:39-41` maps **all three** of `TF_RGB_888`, `TF_RGB_565` and
`TF_RGB_555` to `D3DFMT_R8G8B8` -- three consecutive identical entries, and no D3D9
driver has ever supported that format. So `CheckDeviceFormat` has always failed for all
three and their conversion lists have always fallen through: an R5G6B5 `.dds` has always
loaded as `TF_ARGB_1555`, and an R8G8B8 one as `TF_XRGB_8888`.

`DXGI_FORMAT_B5G6R5_UNORM` matches the engine's 565 row exactly, so mapping it would
work -- and would halve those textures' memory, and DX9's 24-bit row arithmetic was
wrong anyway (`filePitch` is computed from the engine's 2-byte `pixelByteCount` while
the surface would have been 3 bytes per pixel, so the row loop was simply wrong, and
harmless only because it was dead). But turning it on changes which format real assets
load in, which is a visual change, and this port earns visual changes only after parity
is proven. All three are therefore declined, and the header records that 565 is one
table entry away from being enabled.

`TF_RGB_555` should stay declined even then: D3D9's `X1R5G5B5` ignores bit 15 while
`B5G5R5A1_UNORM` reads it as alpha, so any asset whose pad bit happens to be zero would
become fully transparent.

### The engine locks textures in a format they are not stored in, routinely

The plan had no conversion path. DX9 has a substantial one --
`CreateOffscreenPlainSurface` into `D3DPOOL_SCRATCH`, `D3DXLoadSurfaceFromSurface` in
whichever direction the lock needs, the surface parked in `LockData::m_reserved` -- and
it is reached constantly:

- `Texture::load` picks the runtime format from `ms_conversions[sourceFormat]` and then
  `loadSurface` locks every mip level in the **source file's** format. Whenever the
  file's format is one the backend declines, every row written is converted.
- `Texture::computeRepresentativeColor` locks the smallest mip level as `TF_ARGB_8888`
  and reads one pixel, whatever the texture is stored as. For the DXT textures that are
  nearly the whole asset set, that is a block decode -- and it drives interface colours
  and LOD tinting, so a wrong answer is visible rather than academic.

There is no D3DX in D3D11, so `Direct3d11_TextureConverter` implements it: mask-driven
conversion between any two uncompressed engine formats via a 32-bit BGRA intermediate,
plus BC1/BC2/BC3 decode. Compression is deliberately absent and fatal if requested --
nothing in the engine writes a compressed texture in a non-compressed format.

Narrow channels widen by bit replication rather than by a shift, so a 5-bit 31 becomes
255 and a converted read agrees with what the sampler would have produced.

### The pitch contract rules out a mapped resource, and non-discard locks rule out DYNAMIC

Three constraints together decide the upload path:

1. A lock reports the pitch of the **caller's** format, and `loadSurface` collapses a
   whole mip level into a single file read when that pitch matches the file's
   (`Texture.cpp:398-403`). A tight scratch buffer honours that; a mapped staging row
   pitch would silently force the slow row-by-row branch on every texture.
2. Cube maps are locked face by face while not flagged dynamic, and a `DYNAMIC` resource
   can only map its single subresource. `defaultcubemap.dds` -- the first texture the
   client creates -- is 48 (face, level) locks before the first frame.
3. Any lock whose format differs from the resource's needs a CPU buffer to convert
   through regardless.

So every lock allocates a tight scratch buffer, and `unlock` uploads with
`UpdateSubresource`, converting first when the formats differ. `USAGE_DEFAULT`
throughout; there is one upload path, not two.

### A non-discarding lock cannot mean "read the texture back"

`loadSurface` passes `discardContents = isDynamic()`, which is **false** for every
`.dds` in the game. Taken literally, that makes every mip level of every texture at load
time a staging copy plus a CPU wait -- for contents that are about to be overwritten
completely. DX9 pays nothing there: a `MANAGED` surface locked without `D3DLOCK_DISCARD`
just hands back its system-memory copy.

The resolution is to track which subresources have ever been written. A subresource that
never has holds undefined contents -- exactly what a freshly created D3D9 `MANAGED`
surface holds -- so its read-back is skipped and the scratch is zeroed. Every genuine
read-modify-write is still served correctly: the mouse-cursor alpha pass
(`Graphics.cpp:1315`, which reads the alpha of all 1024 pixels before rewriting them),
`computeRepresentativeColor`, and the ground-environment strip. Those read-backs are
counted in `blockingStagingMaps`, so they cannot hide.

### Two DX9 defects in this area that were not copied

`Direct3d9_TextureData::remove` destroys its `MemoryBlockManager` **before** draining the
global texture list. If the list were not already empty, the drain's final
`Texture::release` would run `operator delete` against a null manager. It only ever works
because ExitChain ordering happens to empty the list first. The DX11 version drains
first.

`Stage::getTextureSortKey` casts the D3D texture pointer to `int`
(`Direct3d9_StaticShaderData.cpp:388`), which on x64 discards the top 32 bits -- the x64
DX9 build ships with C4311 and C4302 warnings on that line, so two distinct textures
batch as one whenever they differ only above bit 31. `getSortKey` folds the whole pointer
instead.

### The state cache needed its destroy hook before the first texture could die

`Direct3d11_StateCache` had no shader-resource slots, so a texture destructor had nothing
to unbind from -- which is exactly how a port ends up with a shadow holding a freed
address, a redundant-bind skip, and a wrong image with nothing in the log. DX9 gets this
right and it is the easiest thing in the file to leave out. The sixteen slots (matching
DX9's `cms_samplers`) and `destroyShaderResource` were added in the same commit as the
texture class, not in the later commit that will start binding them.

## Verification: the texture converter

The block decoder is written from the specification and is load-bearing --
`computeRepresentativeColor` runs it on nearly every texture in the game -- so it was
tested rather than reviewed.

The harness compiles the **shipping** `Direct3d11_TextureConverter.cpp` source text
against a stand-in for the DLL's precompiled header (the file is copied in and verified
byte-identical by SHA-256 on every run, because a quoted `#include` resolves against its
own directory first and would otherwise pull in all of `sharedFoundation`), links it with
the real `TextureFormatInfo.cpp`, and compares against Pillow as an independent decoder.

Results, all bit-exact:

- Every uncompressed format round-trips through `TF_ARGB_8888` losslessly in its used
  bits: 8888, XRGB_8888, 4444, 1555, 565, A_8, L_8.
- `TF_L_8` expands to (L, L, L, 255), not (L, 0, 0, 255).
- `TF_ARGB_1555` 0xffff widens to 255 in all four channels; its cleared alpha bit widens
  to 0.
- `TF_XRGB_8888` reads as opaque whatever its pad byte holds.
- All eleven block-compressed `.dds` files in `client-assets/texture` (six DXT1, five
  DXT5) decode with a worst per-channel colour difference of **0** and zero alpha
  mismatches against Pillow.
- Synthetic blocks cover the modes the shipped assets do not reach, all exact: BC1
  four-colour (`c0 > c1`); BC1 three-colour, with the punch-through index producing
  (0,0,0,0); BC2 explicit 4-bit alpha producing the ramp 0, 17, ..., 255, which pins the
  nibble order; BC3 alpha in the six-interpolant branch (`a0 > a1`); BC3 alpha in the
  four-interpolant branch (`a0 <= a1`) with index 6 = 0 and index 7 = 255; and 1x1, 2x3
  and 3x1 levels, where most of the block is clipped away.

The `canConvert` gate was checked to refuse compression, palettised and float pairs, and
to offer DXT decode, 24-to-32-bit widening and identical pairs.

The harness lives in the session scratchpad rather than the tree: this codebase has no
unit-test target, and adding one is a change worth proposing on its own rather than
smuggling in behind a texture commit. `build.ps1`, `main.cpp`, `bccheck.py` and
`bcmodes.py` reproduce every number above.

## C11 is blocked on inputs, and the blockers are not code

C10 finished with the build green in all three configurations and the full client
linking. C11 -- the four shader data classes, and the commit that would produce the
first pixels -- was read out of the DX9 backend in full before being written, and that
reading turned up two blockers that no amount of implementation clears.

### The shader corpus is not in this repository

`client-assets` contains eleven `.sht` files and one `swgsource_3.0.tre`. It contains no
`.eft` effects, no `.vsh` vertex programs, no `.psh` pixel programs and no `.inc`
shader includes. `client.cfg` names only `searchTree_00_8=swgsource_3.0.tre` and
`.include`s five further config files that are also absent, so the base TRE stack comes
from an external retail install at runtime.

What that blocks, concretely:

- Nothing can be compiled, so no shader path can be tested at all. The register-ABI work
  (`/Gec` placing `register(cN)` at `$Globals` byte offset 16N, verified with fxc) is the
  right mechanism, but whether the real corpus compiles at `vs_4_0` is unknown until the
  corpus exists.
- The split between HLSL and assembly programs cannot be counted. An earlier figure of
  96 assembly programs appears in the plan; it is not verifiable from this tree and
  should be treated as unknown rather than repeated.
- `Direct3d11_ShaderCompiler`'s include handler resolves paths out of the TRE set. With
  no TRE set there is nothing to resolve.

This needs either the base TRE files added to `client-assets`, or a path to an existing
SWG install to point `client.cfg` at.

### D3D11 has no assembler, and some of the corpus is assembly

Shader program text begins with a marker line that is either `//hlsl vs_1_1` or
`//asm vs_1_1` (`Direct3d9_VertexShaderData.cpp:362-427`, which documents both forms with
worked examples). DX9 branches on it: HLSL goes to `D3DXCompileShader`, assembly to
`D3DXAssembleShader`.

`D3DCompile` compiles HLSL only. `D3DAssemble` is not in `d3dcompiler_47`, and even if it
were, D3D9 vertex/pixel shader assembly is not a D3D11 shader model. So every `//asm`
program in the corpus has exactly three possible fates: translate it to HLSL, hand-write
a replacement, or accept that the effects using it do not render.

That is a content decision over an unknown number of files, and it cannot even be scoped
until the corpus is available. Until then the DX11 program classes should refuse an
`//asm` program loudly and name the file, so that the first run produces a listable work
item instead of a blank screen.

### The DX9 x64 baseline is not vanilla, and parity means inheriting its patches

This one is a decision rather than a blocker, and it changes what "byte-parity with DX9"
means. The x64 DX9 backend on `x64-dx9-vanilla` patches shader source at load time in
three places, all inside `Direct3d9_VertexShaderData.cpp`:

1. Every include gets `#define point _pt_lights` prepended, because modern compilers treat
   `point` as a reserved word and the SOE includes use it as a field name on `LightData`
   (`Include::Include`, lines 104-108). This one is mechanical and has no visual effect --
   it should be reproduced without further thought.

2. `c_ambient.inc`'s `mov r7, vColor0` is rewritten to `add r7, vColor0, c16`
   (lines 110-121). The stated reason: skinned meshes have no baked `vColor0`, so
   characters received zero ambient and went solid black in some scenes. **This changes
   the image.**

3. Every occurrence of `lightData.ambient.ambientColor + diffuseSpecular.diffuse` is
   rewritten to `max(lightData.ambient.ambientColor + diffuseSpecular.diffuse, 0.85)`
   (lines 430-450). The stated reason: dot3 bump shaders skip the parallel-spec light
   slot, so outdoor characters rendered dark. **This changes the image, and 0.85 is a
   tuned constant, not a derived one.**

Patches 2 and 3 are described in their own comments as fixes for dark or black
characters. They are part of the baseline this port is being measured against, so
reproducing them is what parity requires -- but they are also exactly the kind of thing
a port is a good moment to revisit, and 0.85 is a number somebody chose by looking at a
screen.

`Direct3d11_ShaderCompiler`'s include handler currently applies none of the three. That
is a deliberate gap, not an oversight: which of them to carry forward is the user's call,
and guessing would either bake in a hack silently or change the lighting silently.

### Why this is the stopping point rather than a reason to write more

The four classes could be written from the DX9 source and the engine headers without the
corpus. They would be roughly 3,300 lines with no way to compile a single shader, no way
to reach the first-pixel gate, and no way to check any of it -- against a standard that
so far has been to verify every load-bearing claim, empirically where possible (the
`/Gec` offsets with fxc, the gamma table, the four state tables, the viewport packing,
and the whole texture converter against an independent decoder).

Writing that much unverifiable code in one stretch, on top of a decision about whether
the baseline's lighting patches are part of the target, is how a port acquires the
problems this one has so far avoided.

### Correction: the corpus is present, and the assembly problem is measured

The section above says the shader corpus is not in this repository. That is wrong, and
the error was mine: `client-assets` is only the project's **overlay**, which is why it
holds eleven `.sht` files and one TRE and no effects at all. The base asset set is in
`E:\SWG\64bit-server\_client` -- 209 TRE files including `bottom.tre`, plus all five of
the `.cfg` files that `client-assets/client.cfg` includes. Nothing needs to be supplied.

Resolved through the search-tree order in those cfg files, the corpus is:

| kind | unique files |
| --- | --- |
| `.sht` shaders | 17,192 |
| `.eft` effects | 258 |
| `.vsh` vertex programs | 286 |
| `.psh` pixel programs | 454 |
| `.inc` shader includes | 27 under the program trees |

`.vsh` files are raw text whose first line declares the language. `.psh` files are IFF
wrapped, carrying the source in a `PSRC` chunk and precompiled D3D9 bytecode in a `PEXE`
chunk.

**Language split.** Vertex: 190 HLSL (87 `vs_1_1`, 68 `vs_2_0`, 35 declaring no profile),
96 assembly. Pixel: 322 HLSL (204 `ps_2_0`, 109 `ps_1_1`, 8 `ps_1_4`, 1 `ps_1_0`), 130
assembly (124 `ps.1.1`, 5 `ps.1.0`, 1 `ps.1.4`), and 2 whose first line is a comment.
226 assembly programs in total, so the plan's figure of 96 was the vertex half only.

**Reachability, which is what actually sizes the work.** `ShaderImplementation::load_000N`
compares each `SCAP` entry against `Graphics::getShaderCapability()` for **exact**
equality and abandons the whole implementation when none matches. DX9 reports
`ShaderCapability(2,0)` on any adapter with vs and ps 2.0, which is everything this
century. Parsing all 258 effects: 814 implementations, of which **355 are live at 2.0**.

The stored `SCAP` value is not the runtime one -- versions 0004 and 0005 go through
`remapOldInconsistentShaderCapabilityLevels` so our 2.0 is stored as `0x0300`, 0006 uses
the recent remap, and 0007 and later compare directly. Checked against
`a_emis_full.eft` by hand: its first implementation stores `[0x200, 0x205, 0x300]`,
mapping to 1.1/1.4/2.0, and is live; its second stores `[0x100, 0x105]`, is dead at 2.0,
and is a fixed-function pass with no programs at all.

That last detail is the important one: a single VSPS implementation typically covers 1.1
through 2.0 rather than there being a separate 2.0 variant. So the older assembly programs
are **not** superseded on modern hardware -- they are what DX9 runs today, via
`D3DXAssembleShader` for vertex assembly and via the `PEXE` blob for pixel assembly.

**156 assembly programs are reachable at capability 2.0** -- 92 pixel, 64 vertex. The
other 70 are referenced only by implementations that never load.

**Size of the translation.** Those 156 programs contain 981 instructions in total, a mean
of 6.3 each, and the largest is `cloudlayer.vsh` at 35. They use 26 distinct opcodes:

```
mov 220   tex 199   mul 188   lrp 53   mad 51   mad_sat 19   dp3 17   add 13
dp3_sat 12   texcoord 9   add_sat 8   texm3x2pad 5   texm3x2tex 5   max 4
texld 3   m3x3 3   rsq 3   texcrd 2   m4x4 2   m4x3 2   sub 2   mul_x2 1
add_d2 1   phase 1   exp 1   rcp 1
```

Almost all of that is a direct mechanical rewrite into HLSL: the ps.1.x model has no flow
control, at most eight instructions, four texture registers, and fixed modifier
semantics (`_sat` clamps, `_x2` scales, `_d2` halves). The awkward cases are small and
enumerable -- `texm3x2pad`/`texm3x2tex` (ten uses across five programs, the 2x2 matrix
texture-addressing pair) and the single `phase` marker, which is ps.1.4's two-phase
boundary.

So this is a transpiler plus about five hand-written programs, not 156 hand-written
shaders. The transpiler's output is also directly checkable: each translated program can
be compiled with `fxc` and its emitted `$Globals` layout compared against the register
contract, and the DX9 build can be asked to assemble the original for numerical
comparison on the same inputs.

**What DX9 does that DX11 must therefore also do.** `Direct3d9_PixelShaderProgramData`
recompiles the `PSRC` source instead of using the `PEXE` blob, and overrides
`pixel_program/include/pixel_shader_constants.inc` with an engine-layout version, because
the TRE copy declares `textureFactor` at `c3` where the engine uploads
`dot3LightTangentMinusDiffuseColor`. Without the override, recompiled shaders multiply
colour by a negative value and characters render black. Any DX11 path that compiles from
source inherits that override as a hard requirement, not an option.

## The HLSL corpus, measured

With the corpus located and search-tree precedence handled correctly, the question "does
the shipped shader corpus compile under D3D11" stopped being a matter of opinion. Every
HLSL program in the resolved corpus was compiled with `fxc` at `vs_4_0` / `ps_4_0` with
`/Gec`, exactly as the backend will, applying every transform the backend applies.

**512 of 517 compile. All five failures are unreachable at shader capability 2.0.**

The five are `a_2blend_dirt_bump_ps11`, `simple_bump_light_pass_ps11` and
`specmap_bump_light_pass_ps11` (X3017, a float3 assigned to a float4),
`a_detail_cbmp_ps14` (X4019, a duplicated register), and `saber_blade.vsh` (X3004,
referring to `cameraPosition` where the resolved include calls it `cameraPosition_w`).
They are worth fixing eventually; none of them blocks anything.

### Search-tree precedence had to be fixed first, and it changed every number

`TreeFile::addSearchNode` sorts by priority descending and inserts with `std::lower_bound`
against an `a->priority > b->priority` comparator. `lower_bound` returns the first
position whose priority is *not greater*, so an equal-priority node is inserted **before**
the ones already present: **the last tree added at a given priority is searched first.**
The doc comment above that function says "inserted after the last priority match", which
is the opposite, and following the comment gets the resolution backwards for the 72 trees
that all sit at priority 0.

That is not a detail. Resolving in the wrong order picked a 601-byte
`shared_program/functions.inc` from a base tree instead of the 2,398-byte copy in
`patch_11_00.tre`, and the difference is that the correct one defines `intensity()` and
`tex2DDxt5CompressedNormal()`. It also picked an older `vertex_shader_constants.inc` whose
`Dot3Light` has `direction_o` and `cameraPosition_o` the wrong way round relative to the
C++ struct the engine uploads, and which declares a single `userConstant` at c46 where the
engine's enum has `userConstant0..7` at c52-c59. Once precedence was right, the corpus and
the engine agreed everywhere.

Before the fix: 157 of 541 compiled. After: 318. The rest came from the two transforms
below.

### Two new transforms, both forced by the compiler

**Flattened vertex constants.** The shipped include binds registers to aggregates --
`Material material : register(c11)`, `LightData lightData : register(c16)`.
`d3dcompiler_47` rejects that with X3202 and D3DX rejects it with X4016; DX9's x64 build
escapes via `D3DXSHADER_USE_LEGACY_D3DX9_31_DLL` and `D3DCompile` has no equivalent
switch. Without a replacement, 192 vertex programs do not build at all.

The replacement declares one `float4` per register and rebuilds the structs as `static`
values, leaving shader source untouched -- including `lightData.point[i]` with a loop
index, which a macro-based flattening cannot express. The register numbers come from the
C++ structs the engine actually blits (`Direct3d9_LightManager.h`), every member of which
is a four-float type and therefore owns exactly one register: 1 + 3 + 4 + 4 + 12 + 4 = 28,
so `lightData` runs c16 to c43, and the shipped include puts `textureFactor` at c44. That
arithmetic closing exactly is the check that the layout is the engine's and not a guess.

**The vertex register clause.** Programs declare inputs as
`float4 position : POSITION0 : register(v0);`, and a location semantic on a struct member
is X3202 as well. Measured before acting: across all 192 vertex programs carrying such a
clause, every (semantic, vN) pair agrees with the engine's own assignment
(`VSVR_position` 0, normal 3, pointSize 4, color0 5, color1 6, textureCoordinateSet0..7
7..14) except in four files. So for the other 188 the semantic already carries the
binding, D3D11's input layout matches on semantic anyway, and dropping the clause loses
nothing.

Where they disagree, the register is what DX9 honoured and the semantic was decorative, so
the semantic is rewritten to the one owning that register rather than the clause being
dropped silently. Two of the four are reachable (`a_detail_dirt_bump_vs20` and
`a_detail_specmap_bump_vs20_for_ps20`, both off by one across their texture coordinate
sets). One binds `POSITION0` to v1, a register the engine never writes -- DX9 read
undefined data there -- and it is reported rather than guessed at.

### An ordering bug in the corpus that DX9 hides with its PEXE fallback

`pixel_program/include/functions.inc` uses `materialSpecularPower`, which
`pixel_shader_constants.inc` defines, but several programs include `functions.inc` first,
so the identifier is undefined at the point of use. DX9 never notices, because a
source-compile failure falls back to the program's precompiled `PEXE` blob. There is no
`PEXE` path in D3D11, so the dependency is made explicit: the constants override carries an
include guard and the served `functions.inc` pulls it in itself. Two of the programs this
fixes, `2d_bloom.psh` and `2d_blur.psh`, are what `Bloom::install` fetches by name.

### Reachability had to be measured over shaders, not just effects

A first pass scanned `effect/*.eft` only and undercounted, because **43 `.sht` files carry
their implementation inline** rather than naming an effect -- `shader/2d_bloom.sht` is 725
bytes with no `.eft` reference anywhere in it. Scanning all 17,192 resolved shaders plus
258 effects gives 1,095 implementations, 462 of them live at capability 2.0, referencing
461 programs.

`SCAP` is compared for **exact** equality against `Graphics::getShaderCapability()`, and
DX9 reports `ShaderCapability(2,0)` on any adapter with vs and ps 2.0. A single VSPS
implementation usually spans 1.1 through 2.0 rather than there being a separate 2.0
variant, so the old assembly programs are not superseded on modern hardware -- they are
what DX9 runs today.

### What is left: 97 assembly programs

Language split across the resolved corpus: 192 HLSL and 94 assembly vertex programs; 325
HLSL and 128 assembly pixel programs.

Of the 222 assembly programs, **97 are reachable at capability 2.0** -- 36 vertex, 61
pixel. Together they contain **511 instructions**, a mean of 5.3 each, the largest being
`cloudlayer.vsh` at 35, over 22 distinct opcodes with no flow control anywhere:

```
mov 132  tex 112  mul 79  lrp 25  mad 23  dp3 8  add 7  mad_sat 4  texcoord 3
dp3_sat 3  max 3  add_sat 2  m4x4 2  m3x3 2  rsq 2  mul_x2 1  texm3x2pad 1
texm3x2tex 1  m4x3 1  sub 1  exp 1  rcp 1
```

`texm3x2pad`/`texm3x2tex` appear once each, in a single program. There is no `phase`, so
no ps.1.4 two-phase program is reachable. This is a transpiler plus one awkward file, and
its output is checkable three ways: `fxc` compiles it, the emitted `$Globals` layout can be
compared against the register contract, and the DX9 build can assemble the original for
numerical comparison.

Until that exists the compiler refuses a non-HLSL program by name, so the first run
enumerates exactly which ones are wanted rather than rendering something wrong.

## The assembly translation, scoped -- and a defect in the installed asset set

Sizing the transpiler meant expanding the includes, and that turned up something more
important than the size.

### The real size

The earlier figure of 511 instructions counted only the program files. The assembly vertex
programs are thin shells -- `tf.vsh` is nine lines, six of them `#include` -- and nearly all
their work lives in shared modules under `vertex_program/modules/`. Counting those once
each:

| | files | instructions |
| --- | --- | --- |
| reachable programs | 97 | 475 |
| shared modules they include | 15 | 344 |
| **total** | **112** | **819** |

The four largest modules (`diffuse_specular.inc` 80, `diffuse_specular_lookup.inc` 80,
`diffuse.inc` 69, `dot3_diffuse.inc` 62) are 291 of the 344, and they are the lighting
loops.

### The instruction set, surveyed rather than remembered

With includes expanded and `registers.inc`'s name-to-register defines substituted, the
whole reachable assembly corpus uses:

- **vertex, 16 opcodes:** `dp3 mul mad rsq max sub rcp dst mov add m4x4 m3x3 m4x3 exp lit min`.
  No `sge`, `slt`, `log`, `dp4`, `expp`, `logp`, no address register, no flow control.
  Destinations are only `r`, `oPos`, `oD`, `oT`, `oFog`, with write masks. Sources are `r`,
  `c`, `v` with swizzles and negation. `dst` and `lit` both have HLSL intrinsics of the same
  name.
- **pixel, 12 opcodes:** `tex mul mov lrp mad add mad_sat add_sat dp3 dp3_sat texcoord mul_x2`
  plus `texm3x2pad`/`texm3x2tex` once each in a single program. Source modifiers in use are
  `_bx2`, `_bias`, complement (`1-r0`) and `.a`/`.w` selectors. 39 co-issue (`+`) markers.
  Constants appear as `c[symbolicName]`, and every name used is one the engine-layout
  `pixel_shader_constants.inc` already defines -- so they translate to themselves.

`registers.inc` maps `c0_0`, `c0_5`, `c1_0` and `cLog2e` to `c95.x/y/z/w`, which is what the
`#pragma def(vs, c95, ...)` directive preloads. Only the assembly programs read c95, so the
translation materialises those four as literals rather than depending on an upload.

A flat register file is the right vehicle for the translated constants: `fxc` places
`float4 c[96] : register(c0)` at `$Globals` offset 0 with size 1536, so element *i* sits at
byte 16*i* -- byte-identical to the DX9 constant ABI, verified.

### ILM_visuals.tre ships a diffuse.inc that cannot assemble

`vertex_program/modules/diffuse.inc` exists in five trees. The one that wins resolution is
`ILM_visuals.tre`'s, at priority 5, and it references three symbols --
`cExtLtData_parallelSpec_0_tangentColor`, `_tangentMinusDiffuse` and `_tangentMinusBack` --
that **nothing in any of the 209 TREs defines.** Searched exhaustively; there are no
`#define`s for them anywhere, and DX9 passes only the texture-coordinate macros and `TARGET`
to the assembler.

`Direct3d9_VertexShaderData.cpp:746` is `FATAL(FAILED(result), ("Could not compile shader
%s %d", ...))` -- a hard fatal in every configuration, not a debug one. So on this
installation, the first time one of the programs including that module is created, the DX9
client should die.

**Sixteen reachable assembly vertex programs include it, and fifteen of them are base game
content from `patch_00.tre`:** `c_2blend`, `c_2blend_decal_dirt`, `c_2blend_decal_lowmem`,
`c_2blend_dirt`, `c_alpha_envmask`, `c_detail_dirt`, `c_simple`, `c_simple_openuv`,
`a_vertcoloronly`, `tfal`, `tfcl`, `tfcl_2uv`, `tfcl_3uv`, `tfcl_4uv`, `tfcl_4uv_b`,
`tfcl_env`. The `c_` and `tfcl_` prefixes are cell and cell-terrain shaders -- building
interiors.

So a mod tree replaced a shared include and broke sixteen base programs that depend on it.

### What the base version does, and why the 0.85 floor exists

The base `diffuse.inc` differs from ILM's in exactly the interesting way. It gates every
diffuse light term behind assembly static branching:

```
#if VERTEX_SHADER_VERSION >= 20
if cLightData_parallelSpecular_0_enabled
#endif
max r0, r0, c0_0
mad r7, cLightData_parallelSpecular_0_diffuseColor, r0.y, r7
#if VERTEX_SHADER_VERSION >= 20
endif
#endif
```

Those `cLightData_*_enabled` names are the b0..b7 boolean constant registers. **The engine
never writes them** -- the `BOOL` overload of
`Direct3d9_StateCache::setVertexShaderConstants` has no callers anywhere in the backend --
so all eight are false, and the base assembly path therefore contributes **no diffuse
lighting at all**.

That is almost certainly the root cause the two inherited lighting patches are treating.
`c_ambient.inc` was patched to add the global ambient because "characters end up with zero
ambient contribution", and the per-vertex diffuse was floored at `max(..., 0.85)` because
"dot3 characters rendered dark". Both are compensating downstream for eight booleans that
were never uploaded. ILM's rewrite attacks the same problem from the other direction --
delete the branches, use hemispheric terms -- and fails only because it forgot to define its
new constants.

Writing the booleans is a strictly better fix than a tuned floor, and it is one line in the
constant upload. It is also a visual change, so it belongs after parity, not before -- but
it should be recorded as the likely correct answer rather than left as folklore.

### Why this stops the transpiler

The transpiler itself is ready to write: 24 opcodes, no flow control, a verified constant
vehicle, and every symbolic constant name already resolving. What it cannot decide is what
those sixteen programs should compute, and there is no defensible default:

- Translating ILM's version as written produces code referencing undefined constants; there
  is nothing to translate them to.
- Defining the missing names as the engine's `extendedLightData` (c60..c63, whose
  `HemisphericLightData` fields are `backColor`, `tangentColor`, `tangentMinusBackColor`,
  `tangentMinusDiffuseColor` -- a plausible match by name) makes the mod's intent work, and
  produces lighting DX9 does not currently produce.
- Using the base `diffuse.inc` instead reproduces pre-mod behaviour: bool-gated, all
  branches off, no diffuse contribution, i.e. the dark result the inherited patches exist to
  mask.

Each is a different image, none of them is what DX9 renders today, and DX9 does not render
anything today for this content -- it fatals. That is a content decision.

## The texture-coordinate key is gone

This is the one deliberate structural departure from DX9 in the shader path, so it gets its
own section.

### What DX9 does

A vertex program declares its texture coordinate sets by TAG -- MAIN, NRML, DOT3, DTLA --
and each material decides which of the vertex buffer's numbered sets a given tag reads,
through `StaticShader::getTextureCoordinateSet`. Under D3D9 vertex inputs were fixed
registers v7 to v14, so which register a tag lived in had to be settled at COMPILE time.

DX9 therefore packs the per-tag set indices into a 24-bit key
(`Direct3d9_StaticShaderData.cpp:550-575`), compiles a separate variant of the program for
every key it encounters, keeps them in a `std::map` per program, and does a map lookup per
draw behind a one-entry fast path (`Direct3d9_VertexShaderData.cpp:761-770`).

### What this backend does instead

D3D11 binds vertex inputs by semantic NAME and INDEX, resolved by the input layout at bind
time. The remapping the key encodes is exactly what an input layout does.

So a program is compiled ONCE against a canonical mapping -- tag i becomes `TEXCOORD i` in
declaration order -- and `Direct3d11_InputLayoutCache` places the element for tag i at the
offset of whichever set the material pointed that tag at. What the shader computes does not
change; only which slot it reads from, and the layout compensates.

The mapping becomes part of the layout cache key, which is where it belongs: it describes
how a material wires a vertex buffer to a program, not anything about the program.

### What that removes

- N compiled variants per program, and the per-program shader map.
- The per-draw map lookup.
- A failure mode. DX9 assigns the declared DIMENSION to the set index rather than to the
  tag, so two tags mapping to one set with different dimensions collide -- which is what
  `Direct3d9_VertexShaderData.cpp:594`'s "Competing dimensions" assertion guards against.
  Under canonical indexing each tag owns its own index and the collision cannot arise.

It also makes offline bytecode precompilation tractable, which it was not before: 614
programs, one blob each, instead of an unbounded key space to enumerate or a persistent
cache to ship and invalidate.

### Verified before it was built

Every reachable program compiles under the canonical mapping -- 192 HLSL vertex programs and
36 converted from assembly -- because that is the mapping the fxc sweep already used. The
declared dimension follows the tag rather than the set: for `a_specmap_bump_vs20.vsh`, tags
`[MAIN, NRML, DOT3]` produce `float2 TEXCOORD0; float2 TEXCOORD1; float4 TEXCOORD2`,
independently of any key.

Two behaviours are preserved deliberately:

- A material naming a set the vertex buffer does not carry warns and falls back to set 0,
  which is what DX9 does at `Direct3d9_StaticShaderData.cpp:564`.
- A program with no tags at all -- the converted assembly programs that address sets by
  number, `cloudlayer` being the one -- binds its sets in natural order.

Element widths come from the vertex buffer, not from the shader's declaration. The input
assembler fills components the buffer does not supply with 0 and w with 1, which is what
D3D9 did for an unwritten register component, so a two-component set feeding a
four-component declaration is correct rather than merely legal.

### The remaining gap

`Direct3d11_StaticShaderData` does not exist yet, so nothing calls
`setCurrentTextureCoordinateSetMapping` and the mapping count is zero at every draw --
which means sets currently bind in natural order. That is the correct behaviour for a
tagless program and the wrong behaviour for a tagged one, and it is the next thing to wire.
The seam is in place so that wiring it is a single call from where the static shader is
applied.
