# Direct3D 9 to Direct3D 11 client port

Design of record for converting the Star Wars Galaxies client renderer from
Direct3D 9 to a native x64 Direct3D 11 renderer, on branch `x64-dx11-vanilla`.

Requirements this plan is written against: professional and clean, no bandaids,
no leftover DX9 artifacts, DX11 as the only out-of-the-box path, AAA-title
performance practice, and no client performance degradation.

Every file reference below was verified against the tree at the time of writing.

## Prior art in this tree, and why it is not the starting point

An earlier Direct3D 11 effort left artifacts in the shipped client directory with
no corresponding source in any repository, branch, or working tree:

- `_client/gl11_r.dll` (1,417,728 bytes, 2026-07-21) — no source in any ref
- `_client/Adria-DX11-NOTICE.md`, `_client/Adria-DX11-LICENSE.txt`
- `_client/Graphics-Reborn-Shadertoy-References.md`
- `_client/PhaseWaterCameraHarness.exe`
- `_client/options.cfg.dx11-bak-20260725` (a `rasterMajor=11` config, reverted to `5`)

The engine-side hooks for it *are* committed: `rasterMajor == 11` maps to
`gl11_r.dll` in `Graphics.cpp:228-234`, and eleven `clientGraphics` headers carry
`friend class Direct3d11*` grants.

That effort was a physically-based renderer, not a parity port — its notice
documents GGX, Smith, Schlick, Reinhard, and a "Phase 41 terrain-reflectivity
control" scaling dielectric F0. Its diagnostic strings and its `b2` constant
buffer convention are useful as inventory. Its design is not a parity reference,
and the binary is explicitly not a correctness oracle: it is quarantined in
Phase 0 so it cannot shadow a new build.

## Headline

Convert the renderer by replacing exactly one DLL behind an unchanged contract, then subtract DX9. `Graphics::install` already LoadLibrary's `gl%02d_r.dll` by `rasterMajor`, so DX9 and DX11 are mutually exclusive at runtime and there is never a forked engine path — the whole port is a new `Direct3d11` project filling the existing 118-slot `Gl_api` verbatim, plus one missing `friend class Direct3d11_RenderTarget` in Texture.h. The backend reports exactly `ShaderCapability(2,0)`, compiles the existing 553 KB shader corpus with `D3DCompile` at vs_4_0/ps_4_0 under `D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY` — which places the DX9 `register(cN)` file into `$Globals` at byte offset 16×N, so the asset ABI survives untouched — and renders into a backend-owned offscreen `Direct3d11_SceneTarget` that single-handedly solves lockBackBuffer, presentToWindow, gamma, MSAA and screenshots. The one shader-source change required in Phase 2 is vacating c0–c7 (the two per-draw matrices) out of `$Globals` into a 128-byte `SwgPerObject` ring buffer; without it the per-draw constant traffic is 12× DX9 and the parity gate is arithmetically unreachable. Nothing DX11 merges until an instrumented DX9 baseline exists, because the shipped renderer has zero instrumentation (`Direct3d9_Metrics` is `#ifdef _DEBUG` and its only report caller is inside a `#if 0`), which makes "no degradation" currently unfalsifiable. DX9 deletion is a single subtraction commit gated on all nine DX9-linking projects being retired or ported first.

## Architecture decisions

### A1. Keep `struct Gl_api` byte-for-byte unchanged through Phase 4; add an out-of-band `extern "C" uint32 GetGlApiStructSize()` guard in Phase 0 and validate it in `Graphics::install` right after `GetApi`, FATAL on mismatch.

Gl_api has three distinct binary layouts (5 slots under `#ifdef _DEBUG` at Gl_dll.def:101-110, 4 under `#if PRODUCTION == 0` at 235-240) discriminated only by the _r/_o/_d filename suffix chosen from DEBUG_LEVEL at Graphics.cpp:200-208. A DEBUG_LEVEL mismatch loads cleanly then calls the wrong function through every shifted slot — the plan's single worst silent-failure mode, and there is no version check today. The guard must be a separate export, not a struct field, because Headless.cpp:41-60 blanket-fills the struct as `void**` over `sizeof(Gl_api)/sizeof(void*)` and would stomp it.

Touches:
- `src/engine/client/library/clientGraphics/src/win32/Gl_dll.def`
- `src/engine/client/library/clientGraphics/src/win32/Graphics.cpp`
- `src/engine/client/application/Headless/src/shared/Headless.cpp`

### A2. Backend selection stays LoadLibrary/GetProcAddress on `rasterMajor`. In the same commit that flips the default to 11, coerce any value outside {0,11} to 11 with a non-fatal WARNING before the DLL name is formatted.

Verified `#define DEBUG_FATAL(a,b) NOP` in Release (Fatal.h:42-46), so the `else DEBUG_FATAL("unknown rasterizer")` arm at Graphics.cpp:236 does nothing and a stale `rasterMajor=5` falls straight into `sprintf(library, ".\\gl%02d_r.dll", 5); LoadLibrary(library)` at :239-241 — which, once gl05_r.dll is deleted, lands on the PRODUCTION MessageBox at :245 reading "DirectX 9 not being properly installed". options.cfg is per-user and every existing player has rasterMajor=5 on disk. This is the most likely way "DX11 out of the box" fails in the field.

Touches:
- `src/engine/client/library/clientGraphics/src/win32/Graphics.cpp`
- `src/engine/shared/library/sharedFoundation/src/shared/Fatal.h`
- `_client/options.cfg`

### A3. `Direct3d11::getShaderCapability()` returns exactly `ShaderCapability(2,0)`. Do not relax the `==` test at ShaderImplementation.cpp:552, do not add a capability tier, and keep `GraphicsOptionTags::set(TAG_DX9, true)` for rasterMajor 11 as a permanently documented asset-compatibility mapping.

ShaderCapability.h:16 DEBUG_FATALs outside {0.0,0.2,0.3,1.1,1.4,2.0}; 2.0 is the highest tier in any shipped .eft and the only value that keeps DOT3/POST/HEAT enabled (SetupClientGraphics.cpp:95-111, which also needs videoMemory>=100MB) and keeps `misc/asynchronous_loader_data_2.iff` resolvable (Game.cpp:916). Relaxing to `>=` is actively wrong: measured SCAP sets include [0.2,0.3] ordered before higher tiers in 437 implementations, so `>=` would select an FFP implementation for hundreds of effects. Renaming TAG_DX9 would require re-tagging OPTN chunks in 280 .eft plus 79 inline-EFCT .sht for zero functional gain.

Touches:
- `src/engine/client/library/clientGraphics/src/shared/ShaderCapability.h`
- `src/engine/client/library/clientGraphics/src/shared/ShaderImplementation.cpp`
- `src/engine/client/library/clientGraphics/src/win32/SetupClientGraphics.cpp`

### A4. Compile the entire ported corpus with `D3DCompile` at vs_4_0/ps_4_0 (feature level 11_0 floor) under `D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY`. Never pass `D3DCOMPILE_PACK_MATRIX_ROW_MAJOR`. Upload matrix bytes identical to today, keep HLSL default column_major and `mul(v, M)`.

-Gec is mandatory (without it `tex2D` is hard error X3523) and it places `register(cN)`-annotated constants into `$Globals` at byte offset exactly 16×N, verified by reflection against every VSCR_/PSCR_ enumerator — so the asset-baked constant ABI survives with no renumbering and no struct repacking. 4_0 over 5_0 because the live set needs nothing from SM5 (no tessellation/compute/UAV, max sampler slot s4), and pinning one profile keeps the numerical-equivalence harness valid. The recon's transposition warning is wrong: gl05 (FFP+VSPS, the shipped default) and gl07 (VSPS-only) upload algebraically identical bytes, and column_major makes cN a column so `dp4 o0.x, v0, cb0[0]` reproduces DX9 exactly.

Touches:
- `src/engine/client/application/Direct3d9/src/win32/Direct3d9_VertexShaderConstantRegisters.h`
- `src/engine/client/application/Direct3d9/src/win32/Direct3d9_PixelShaderConstantRegisters.h`
- `src/engine/client/application/Direct3d9/src/win32/Direct3d9.cpp`

### A5. Authoritative constant-buffer slot map, single source of truth for backend and shader sources: b0 = `$Globals` (asset ABI; VS c8..c95 live with c0..c7 VACATED, PS c0..c24), b1 = `SwgPixelEpilogue` (alphaTestRef8, alphaTestEnable, fogColor.rgb, fogEnable), b2 = RESERVED, b3 = `SwgPerObject` (objectWorldCameraProjectionMatrix + objectWorldMatrix, 128 B in a 256 B-aligned NO_OVERWRITE ring).

Resolves a direct contradiction between lenses and an arithmetically fatal gap. A monolithic b0 forces a full ~1536 B reflected-extent upload per draw because c0-c7 are the per-draw matrices — 3.3-4.6 MB/frame at 3000 draws versus DX9's 384 KB (Direct3d9.cpp:3902-3916 uploads 8 registers into a driver-batched register file). Vacating only those two declarations from vertex_shader_constants.inc costs two lines and shifts nothing else, because explicit `register(cN)` annotations pin every remaining offset. Epilogue constants go in b1, not "rows past c24", so the PS `$Globals` extent stays exactly PSCR_MAX and the reflection ABI guard has no plan-introduced exceptions. b2 is reserved to match the prior gl11 effort's documented convention.

Touches:
- `_client/data_other_00.tre#vertex_program/include/vertex_shader_constants.inc`
- `_client/data_other_00.tre#pixel_program/include/pixel_shader_constants.inc`
- `src/engine/client/application/Direct3d9/src/win32/Direct3d9_StateCache.cpp`

### A6. Reflection (`D3DReflect`) is mandatory for every compiled shader and produces three artifacts: an ABI offset guard asserting each named `$Globals` constant sits at 16×its register; a per-shader sampler-register → texture-register map; and the cached VS input signature.

Under -Gec samplers keep their declared s# but textures are packed densely in declaration order among USED samplers only — verified with fxc: with s1 unused, `sampler s2 : register(s2)` binds to t1 and a samplerCUBE at s3 to t2. The .eft PTXM chunk binds textures by sampler slot index, so assuming t#==s# puts textures on wrong samplers across a large fraction of 23,394 .sht with no error. The shipped gl11_r.dll carries the exact diagnostics ("mapped sampler %d to texture %d; expected %d", "did not preserve %s at byte offset %u"), confirming this bit the prior effort. The signature is required because DX11 validates input layouts against specific VS bytecode.

Touches:
- `src/engine/client/application/Direct3d9/src/win32/Direct3d9_StaticShaderData.cpp`
- `src/engine/client/library/clientGraphics/src/shared/ShaderImplementation.cpp`

### A7. Offline bake is the primary shader path (a build step emits DXBC + reflection metadata for the frozen 152-entry permutation manifest), AND `d3dcompiler_47.dll` from the Windows SDK redist ships as an instrumented fail-safe. Every runtime compile in PRODUCTION increments an always-on counter; CI gates that counter at zero.

Resolves a three-way contradiction (architecture: runtime+cache+ship redist; shader: offline-only+don't ship; perf: runtime with a zero-compiles-per-frame gate). Offline-only is unsafe because the permutation manifest is not provably closed — `StaticShader::setTextureCoordinateSet` (StaticShader.h:73, StaticShader.cpp:401-415) is a public runtime mutator that clones the tcs map and changes the permutation key, called from TextureBuilder — so a missed permutation would render nothing with no recovery. Runtime-only reintroduces the launch hitching the whole design set out to remove. Belt-and-braces with a measurable counter gives correctness plus enforceability.

Touches:
- `src/engine/client/library/clientGraphics/src/shared/StaticShader.cpp`
- `src/engine/client/application/Direct3d9/src/win32/Direct3d9_VertexShaderData.cpp`
- `scripts/Stage-X64Client.ps1`

### A8. The scene renders into a backend-owned offscreen `Direct3d11_SceneTarget` (R8G8B8A8_UNORM + D24_UNORM_S8_UINT, optionally multisampled), composited to the swap chain in `present()`. `setRenderTarget(NULL)` returns to the scene target, never to the back buffer.

One decision resolves five otherwise-separate problems created by the flip model: (1) `lockBackBuffer` can copy a known-format texture to staging instead of mapping an unmappable FLIP_DISCARD buffer; (2) `presentToWindow` (9 call sites across 5 TerrainEditor views, Viewer, TextureBuilder, SwgGodClient, swgClientQtWidgets) becomes a per-HWND swap chain fed from the composite; (3) brightness/contrast/gamma replaces `SetGammaRamp` (Direct3d9.cpp:2064-2083), which has no DX11 equivalent that works in the windowed mode the client actually ships; (4) windowed MSAA becomes possible with an obvious resolve point; (5) screenshots capture the corrected image, deleting `applyGammaCorrectionToXRGBSurface` and its 32-bit pointer truncation at Direct3d9.cpp:2613. PostProcessingEffectsManager already redirects the scene offscreen when POST is on, so alias rather than double-blit.

Touches:
- `src/engine/client/application/Direct3d9/src/win32/Direct3d9.cpp`
- `src/engine/client/library/clientGraphics/src/shared/PostProcessingEffectsManager.cpp`
- `src/engine/client/library/clientGraphics/src/win32/Graphics.cpp`

### A9. Two-tier state caching. `Direct3d11_StateObjectCache` hash-conses immutable Blend/DepthStencil/Rasterizer/Sampler objects created ONLY at `Pass::construct`/`Stage::construct` time or pre-enumerated at install; `Direct3d11_StateCache` shadows currently bound objects to skip redundant binds. Zero `Create*State` calls during a frame is an asserted invariant with a Release counter.

The immutable tuple is already gathered at load time — Direct3d9_ShaderImplementationData.cpp:239-243 computes `17 + 5*twoSided + 7*FFP` states in Pass::construct and :341-353 merely replays them; Direct3d9_StaticShaderData.cpp:200-209 collects exactly 7 sampler states per stage in Stage::construct. So this is a clean structural fit, and it collapses ~70-90 API calls per shader change to ~10. Lazy creation at bind time is the canonical way DX11 ports lose to DX9, and DX9 currently benefits from runtime dedup it will not get in DX11 (usePureDevice defaults false). Note Direct3d9_StateCache.h:340-361 `forceVertexDeclaration` never assigns `ms_vertexDeclaration`, so the DX9 declaration cache is a permanent 100% miss — a working shadow is free headroom.

Touches:
- `src/engine/client/application/Direct3d9/src/win32/Direct3d9_StateCache.h`
- `src/engine/client/application/Direct3d9/src/win32/Direct3d9_ShaderImplementationData.cpp`
- `src/engine/client/application/Direct3d9/src/win32/Direct3d9_StaticShaderData.cpp`

### A10. Input layouts are keyed on (vertex-format set, VS input-signature hash) reduced to the coarser (texcoord index, dimension) set. Signature elements the vertex format does not supply are satisfied by a 16-byte zero buffer bound at a reserved input slot with `PER_INSTANCE` / `InstanceDataStepRate=0`.

DX9 matched declarations by semantic at draw time; DX11 validates against specific bytecode and the signature varies per texture-coordinate-set permutation, so the format-only cache in Direct3d9_VertexDeclarationMap.cpp:103-251 is insufficient. DX9 also tolerated a VS declaring an input the buffer lacked — the engine has dedicated machinery for it (`setBadVertexBufferVertexShaderCombination`, the "Missing vertex components in %s on shader %s" warning at Direct3d9.cpp:4000), proving it occurs in shipped data — where CreateInputLayout hard-fails. The zero stream reproduces DX9's (0,0,0,0) read, keeps the warning visible, and avoids dropping materials, unlike the prior gl11 binary which rejected them.

Touches:
- `src/engine/client/application/Direct3d9/src/win32/Direct3d9_VertexDeclarationMap.cpp`
- `src/engine/client/application/Direct3d9/src/win32/Direct3d9_VertexBufferDescriptorMap.cpp`
- `src/engine/client/library/clientGraphics/src/shared/VertexBufferFormat.h`

### A11. Create the D3D11 device WITHOUT `D3D11_CREATE_DEVICE_SINGLETHREADED`. `ID3D11Device::Create*` is treated as free-threaded; the immediate context is main-thread-only with an `Os::isMainThread()` assert on every context-touching entry point.

Overrides the perf lens. `Graphics::createShaderImplementationGraphicsData` is called from `ShaderImplementation::load` (ShaderImplementation.cpp:419/479/539/599/671/742), which runs on whatever thread loads the .eft — including the AsynchronousLoader, since ShaderTemplateList.cpp:170 registers the 'sht' extension with it. Only textures assert main-thread (Direct3d9_TextureData.cpp:279). D3D9 got away with it because device creation is internally serialised; SINGLETHREADED turns that into intermittent device-state and heap corruption. The locking cost is irrelevant because draw-time creation is forbidden anyway, and thread-safe creation is the prerequisite for the Phase 7 async texture upload.

Touches:
- `src/engine/client/library/clientGraphics/src/shared/ShaderImplementation.cpp`
- `src/engine/client/library/clientGraphics/src/shared/ShaderTemplateList.cpp`
- `src/engine/client/application/Direct3d9/src/win32/Direct3d9_TextureData.cpp`

### A12. Delete the fixed-function pipeline in the BACKEND ONLY. `ShaderImplementationPassStage` and the STAG loader stay in clientGraphics permanently, as a documented asset-compatibility reader.

Corrects a blocker. Only IMPL versions 0004+ bail before parsing Pass forms; verified `load_0000` (ShaderImplementation.cpp:238-262) reserves and constructs every `Pass(iff)` — which parses STAG — and only then calls `checkOldVersionForSupport()`. So the STAG reader is still required to READ font.eft, defaulteffect.eft, a_skin.eft, e_shadowblob.eft and gradient_sky.eft. The "zero implementations selected at 2.0 contain PFFP or STAG" measurement is about selection, not parsing. What deletes is the backend translation: Direct3d9_ShaderImplementationData.cpp:25-124's TextureOperation/TextureArgument/MaterialSource tables and the :136-187/:365-386 stage cascade, never implemented in Direct3d11_ShaderImplementationData.

Touches:
- `src/engine/client/library/clientGraphics/src/shared/ShaderImplementation.cpp`
- `src/engine/client/library/clientGraphics/src/shared/ShaderImplementation.h`
- `src/engine/client/application/Direct3d9/src/win32/Direct3d9_ShaderImplementationData.cpp`

### A13. Texture bakes render through an RTV created directly on the destination texture, which requires plumbing `TCF_renderTarget` through `TextureRendererTemplate::fetchCompatibleTexture`. RT-capable textures are `D3D11_USAGE_DEFAULT`, never IMMUTABLE. A shader-blit fallback covers any destination format that cannot carry an RTV, and the zero-readback invariant is conditional on RT-capability.

Corrects a blocker: verified `fetchCompatibleTexture` calls `TextureList::fetch(0, ...)` — creationFlags 0 — so no RTV is currently possible and `BIND_RENDER_TARGET` is an illegal combination with IMMUTABLE. Without the plumbing the gate is unsatisfiable and the worst stall in the renderer survives: Direct3d9_RenderTarget.cpp:302 does a synchronous `GetRenderTargetData` GPU→CPU readback plus `D3DXLoadSurfaceFromSurface` at :317, once per frame per queued TextureRenderer (TextureRendererManager::alter, called from Game.cpp:1215) — i.e. every character face bake. DX11 queues more work ahead, so a naive staging+blocking-Map translation is worse than DX9.

Touches:
- `src/engine/client/library/clientTextureRenderer/src/shared/TextureRendererTemplate.cpp`
- `src/engine/client/application/Direct3d9/src/win32/Direct3d9_RenderTarget.cpp`
- `src/engine/client/library/clientTextureRenderer/src/shared/BlueprintTextureRendererTemplate.cpp`

### A14. Texture usage tiers: IMMUTABLE + pSysMem is the default; DEFAULT + UpdateSubresource for partial writes; DYNAMIC + Map(WRITE_DISCARD) only for TCF_dynamic; read-only locks via a per-texture staging ring with `D3D11_MAP_FLAG_DO_NOT_WAIT`. All formats plain `_UNORM` — no sRGB anywhere. TF_L_8 is expanded to B8G8R8A8 on load, not mapped to R8_UNORM.

D3DPOOL_MANAGED (Direct3d9_TextureData.cpp:282) keeps a sysmem shadow so a LockRect never stalls; a generic staging+CopyResource shim would add a full extra copy per texture and regress streaming versus DX9, whereas IMMUTABLE is strictly faster than MANAGED. No sRGB because all 30,245 shipped .dds use legacy headers with zero colour-space metadata and DX9 lit in gamma space for 20 years — *_SRGB would change every texture and be indistinguishable from a port bug. TF_L_8 cannot be "R8_UNORM with swizzle": D3D11 has no SRV component swizzle, D3DFMT_L8 samples (L,L,L,1) versus R8's (R,0,0,1), and `addConversion(TF_L_8, TF_L_8)` (Texture.cpp:132) leaves no fallback — only 4 shipped textures use it, so expansion is free.

Touches:
- `src/engine/client/application/Direct3d9/src/win32/Direct3d9_TextureData.cpp`
- `src/engine/client/library/clientGraphics/src/shared/Texture.cpp`
- `src/engine/client/library/clientGraphics/src/shared/TextureFormatInfo.cpp`

### A15. DXGI flip-model swap chain (`CreateSwapChainForHwnd`, FLIP_DISCARD, BufferCount 3, ALLOW_TEARING when IDXGIFactory5 reports it), borderless-fullscreen only, `MakeWindowAssociation(DXGI_MWA_NO_ALT_ENTER)`, never `SetFullscreenState`. No lost-device model: `wasDeviceReset()` returns false, the existing lost/restored callbacks fire only around `ResizeBuffers`, `DXGI_ERROR_DEVICE_REMOVED` is fatal.

Flip model is measurably lower latency and lower CPU overhead than the DX9 windowed `D3DSWAPEFFECT_COPY` path at Direct3d9.cpp:2096, so it is a free win. Dropping exclusive fullscreen deletes EnumAdapterModes, checkDisplayMode, the setWindowedMode Reset-with-rollback at Direct3d9.cpp:2009-2050 and the 60×Sleep(500) DEVICELOST retry loop at :2496-2545. The callbacks must be retained because exactly two subsystems subscribe and both need render targets recreated on resize: BinkVideo.cpp:120-121 and PostProcessingEffectsManager.cpp:107-108.

Touches:
- `src/engine/client/application/Direct3d9/src/win32/Direct3d9.cpp`
- `src/engine/client/library/clientGraphics/src/Bink/BinkVideo.cpp`
- `src/engine/client/library/clientGraphics/src/shared/PostProcessingEffectsManager.cpp`

### A16. Keep the dynamic vertex/index ring design unchanged in shape (NO_OVERWRITE, DISCARD on wrap) but resize VB 2 MB → 16 MB and IB 64 KB → 4 MB, preserve the vertex-index (not byte) offset semantics through BaseVertexLocation, and keep the wrapper objects as thin NON-OWNING handles over the shared global buffers whose destructors release nothing.

The ring is already correct AAA practice (Direct3d9_DynamicVertexBufferData.cpp:183-207) and maps 1:1. Resizing is required because ClientTerrainSorter.cpp:36 batches 256 KB per node, so 8 nodes exhaust the VB and ~2 exhaust the 32768-index IB, forcing mid-frame DISCARDs. Non-ownership is a hard compatibility constraint, not a preference: ShadowVolume.cpp:127-146 constructs a stack DynamicVertexBuffer, locks, copies, binds, and lets the temporary die while the stream source still points at the shared ring — as do ClientTerrainSorter.cpp:296, Bloom.cpp:333/390/445 and PostProcessingEffectsManager.cpp:216. Also drop the `IsBadWritePtr`/`getTemporaryBuffer` redirect at :211-213, which silently writes into a throwaway heap buffer and would mask real Map failures.

Touches:
- `src/engine/client/application/Direct3d9/src/win32/Direct3d9_DynamicVertexBufferData.cpp`
- `src/engine/client/application/Direct3d9/src/win32/Direct3d9_DynamicIndexBufferData.cpp`
- `src/engine/client/library/clientObject/src/shared/appearance/ShadowVolume.cpp`

### A17. Alpha test becomes one unconditional pixel-shader epilogue with exact 8-bit semantics — `if (round(a*255) <= alphaTestRef8) discard;` — driven by a b1 cbuffer scalar pre-multiplied by alpha-fade opacity. Fog becomes a plain VS interpolant plus a b1-driven `lerp(fogColor, rgb, saturate(fog*fogEnable))`. No permutation dimension for either.

Measured across 1,518 decoded PASS chunks all 422 alpha-tested passes use `Greater`, with refs A000(264)/MAIN(133)/A128(19)/SPEC(6); A000 resolves to ref 0 so `clip(a-0)` would KEEP the alpha==0 pixels DX9 discards — hence exact 8-bit comparison. Direct3d9.cpp:3944 rescales ALPHAREF per draw by alpha-fade, so it must be a constant not a literal. `fogEnable` is separately essential: `setFog(false,...)` at Direct3d9.cpp:3269-3273 clears only D3DRS_FOGENABLE and leaves the VSCR_fog density constant live, and it is used as a scoped disable around the heat composite (ShaderPrimitiveSorter.cpp:566) and the star field (StarAppearance.cpp:347) — an unconditional lerp would fog both.

Touches:
- `src/engine/client/application/Direct3d9/src/win32/Direct3d9.cpp`
- `src/engine/client/application/Direct3d9/src/win32/Direct3d9_StaticShaderData.cpp`
- `src/engine/client/library/clientTerrain/src/shared/appearance/StarAppearance.cpp`

### A18. Add a `Headless.vcxproj` producing gl00_{r,o,d} to swg.sln in Phase 1, so every Gl_api change is compile-checked against two independent implementations, and gl00 becomes a DX-free regression baseline for the measurement harness.

`rasterMajor=0` is the console/headless default (SetupClientGraphics.cpp:279) but there is no gl00_r.dll anywhere, no Headless.vcxproj, and zero "Headless" matches in swg.sln — so today a config value selects a DLL that has never existed. The Phase 6 Gl_api trim is unverifiable with the second implementation unbuildable, and Headless.cpp's `sizeof(Gl_api)/sizeof(void*)` blanket fill breaks silently on any layout change. The source is complete, so this is cheap insurance on the port's most dangerous surface.

Touches:
- `src/engine/client/application/Headless/src/shared/Headless.cpp`
- `src/build/win32/swg.sln`
- `src/engine/client/library/clientGraphics/src/win32/SetupClientGraphics.cpp`

### A19. One `Direct3d11` project producing gl11_{r,o,d}, linking only d3d11.lib/dxgi.lib/d3dcompiler.lib/dxguid.lib from the Windows SDK. `$(DXSDK_DIR)` must not appear on any include or library path. No FFP/VSPS variants, no BaseAddress, no /SAFESEH:NO, and a real MaxSpeed+MultiThreaded Optimized config.

Windows SDK 10.0.26100.0 supplies all four libs in Lib/um/x64, so zero vendored libraries are needed. Critically, the June 2010 DirectX SDK ALSO ships d3d11.h/dxgi.h/d3dcompiler.h and d3d11.lib/dxgi.lib, and `$(DXSDK_DIR)Include`/`Lib\x64` appear FIRST on the x64 paths of all three DX9 projects (Direct3d9.vcxproj:323/351) — cloning one as a template silently binds to D3D11.0/DXGI 1.1, losing ID3D11Device1 and IDXGIFactory2 and therefore CreateSwapChainForHwnd and the flip model. Direct3d9's Optimized|x64 also uses Optimization=Disabled with the MultiThreadedDebug CRT, which would ship an unoptimised debug-CRT renderer.

Touches:
- `src/engine/client/application/Direct3d9/build/win32/Direct3d9.vcxproj`
- `Directory.Build.targets`
- `src/build/win32/swg.sln`

### A20. Promote `Direct3d9_VertexShaderConstantRegisters.h`, `Direct3d9_PixelShaderConstantRegisters.h` and `Direct3d9_VertexShaderVertexRegisters.h` into `clientGraphics/include/public/clientGraphics/ShaderConstantRegisters.h` in Phase 0, values byte-identical, adding named `VSCR_c95 = 95` and a derived `VSCR_CBUFFER_ROWS`.

These are asset contract, not backend state — reflection confirms their values ARE the `$Globals` byte offsets divided by 16 — and they carry triple WARNING banners saying so. They are also already reached across a layering boundary: ShaderBuilder includes all three by relative path (PixelShaderProgramView.cpp:12, VertexShaderProgram.cpp:14-15), so the Phase 5 deletion of the Direct3d9 directory would break the only tool that validates .vsh and writes .psh, with no compile-time warning until someone builds it.

Touches:
- `src/engine/client/application/Direct3d9/src/win32/Direct3d9_VertexShaderConstantRegisters.h`
- `src/engine/client/application/ShaderBuilder/src/win32/PixelShaderProgramView.cpp`
- `src/engine/client/application/ShaderBuilder/src/win32/VertexShaderProgram.cpp`

## Phases

### Phase 0 — Measure, extract, de-risk — all on DX9

**Effort:** 4-6 weeks  
**Goal:** Make "no performance degradation" falsifiable, put the shader corpus under version control, and land every zero-risk subtraction and prerequisite while DX9 is still the shipping renderer. No DX11 code exists at the end of this phase.

#### Work

- Add always-on `GfxStats` to clientGraphics: plain uint32/uint64 counters published across Gl_api, counter set derived from Direct3d9_Metrics.h:30-99 (draws indexed/non-indexed, per-phase draws and triangles for all 13 phases, per-category bind CALLS and MISSES for input layout / VB slots / IB / VS / PS / blend / DS / RS / samplers / SRVs / viewport / scissor, constant bytes and update count per buffer, ring Map DISCARD vs NO_OVERWRITE counts and bytes, state-object / input-layout / shader-compile / cbuffer creations, RT switches, texture-bake readbacks, backbuffer maps, blocking staging maps). Re-enable `DebugFlags::callReportRoutines()` — it is inside a `#if 0` at Game.cpp:1117 so the entire report mechanism is dead even in Debug; DebugFlags itself is not PRODUCTION-gated.

- Add a preallocated ~3600-frame telemetry ring (one memcpy per frame, no in-frame I/O) with a CSV sink, plus ~20 always-on QPC scoped zones (cull/DPVS, sorter add, per-phase sort, per-phase draw, prepareToDraw split by primitive type, texture baking, post, bloom, heat, present) — needed because every NP_PROFILER_* macro is NOP at PRODUCTION==1.

- Add GPU timestamp queries to the DX9 backend — there are currently ZERO CreateQuery calls in the first-party tree. Triple-buffered D3DQUERYTYPE_TIMESTAMP/TIMESTAMPFREQ/TIMESTAMPDISJOINT read 3 frames late; default whole-frame, optional per-phase mode behind a flag.

- Add peak DXGI/adapter dedicated video memory and peak process private bytes to the capture — no memory metric exists in any lens's gate set, yet the port changes the residency model for the entire 3.4 GiB texture set.

- Build the deterministic benchmark harness: forced constant dt = 1/60 s, scripted camera routes modelled on the fixed-yaw-step state machine at ScreenShot360.cpp:95-107, refuse to measure until the AsynchronousLoader queue drains, discard 300 warm-up frames, measure 1800. Benchmark cfg overlay sets `allowTearing=true` (the default false forces D3DPRESENT_INTERVAL_ONE at Direct3d9.cpp:2119-2122, making frame time meaningless). No frame-limiter work needed: user.cfg frameRateLimit=0.0f and Clock.cpp:120 skips the limiter at 0, so the busy-spin at Clock.cpp:415-434 never runs.

- Author six routes: R0 static_360 (CI canary), R1 mos_eisley_walk, R2 cantina_interior (cell push/pop flush + CPU skinning), R3 tatooine_dune_flora, R4 combat_effects, R5 character_creation. Admit a route only if run-to-run p95 spread <= 3%. Run 6 routes x 3 runs x 2 resolutions (1920x1080 primary CPU gate, 3840x1080 matching the shipped options.cfg).

- Build the per-draw reference-capture harness on gl05_r: dump shader template, effect, selected implementation index, pass index, resolved .vsh/.psh, permutation key, full VS/PS constant blocks, and the RT after each draw. Pin currentTime and textureScroll (Direct3d9_StaticShaderData.cpp:914-926 recomputes scroll with modf every apply), seed particles, fix camera and resolution.

- Empirically log which effects fail `ShaderEffect` validation today (ShaderEffect.cpp:136/166) to settle the font.eft / defaulteffect.eft / a_skin.eft question with data rather than inference.

- Land and separately re-baseline four DX9-side fixes, one commit each: (a) the four pointer-truncating sort keys (ShaderEffect.h:90-92, Direct3d9_StaticVertexBufferData.cpp:140-143, Direct3d9_DynamicVertexBufferData.cpp:260-263, Direct3d9_StaticShaderData.cpp:385-390) replaced with monotonic 32-bit IDs, keeping ShaderPrimitiveSorter Entry at 56 bytes; (b) light over-dirtying — Direct3d9_LightManager.cpp:336 setObeysLightScale unconditionally sets ms_dirty and Direct3d9.cpp:2938 calls it on every setStaticShader, plus LightBitSet caching in ShaderPrimitiveSorter.cpp:1130-1145; (c) the O(n^2) selection sort at ParticleEmitter.cpp:668-681; (d) RenderWorldCommander.cpp:50's per-visible-object std::set insert.

- Census work required before any conversion: run the 1,518-pass decode over m_zCompare, m_stencilCompare and m_stencilCompareCounterClockwise (the alpha census was taken on the enum, not the post-table D3D value, and Direct3d9_ShaderImplementationData.cpp:31-41 maps C_GreaterOrEqual→NOTEQUAL and C_NotEqual→GREATEREQUAL); and census m_alphaBlendSource/Destination/Operation over passes with alphaBlendEnable==false, to derive the exact alpha-fade blend variant set from data rather than guessing 2 or 4.

- Extract the shader corpus with a committed resolver tool implementing TreeFile's REAL order (priority descending; last-added wins at equal priority per TreeFile.cpp:299-308; searchPath then searchTree then searchTOC within a priority — so sku0_client.toc at priority 0 beats all 64 priority-0 .tre files). Commit vertex_program/** (which exists in NO source tree), reconcile the 21 differing + 4 new ILM_visuals program files into serverdata/pixel_program, record per-file provenance. Do NOT use E:/SWG/PRE-CU-Reborn/.../vertex_program — its Dot3Light field order is swapped relative to the shipped .inc and the C++ struct.

- Commit the permutation enumerator and freeze the 152-entry (vsh, textureCoordinateSetKey) manifest across 118 vertex shaders. Mark `StaticShader::setTextureCoordinateSet` non-shipping (assert !PRODUCTION) so manifest closure becomes an enforced invariant. Tags in IFF chunks are byte-reversed — read little-endian or every key collapses to 0.

- Promote the three register headers to clientGraphics/ShaderConstantRegisters.h with forwarding shims left in the DX9 directory.

- Zero-risk subtractions: delete Gl_api::setDynamicIndexBufferSize (Graphics.cpp:1697 calls it unguarded, the DX9 backend never assigns the slot, zero callers anywhere — a guaranteed NULL deref); fix the Graphics::lockBackBuffer rect swap at Graphics.cpp:1049-1052 (bottom←y1, right←x1); delete Gl_metrics (Graphics.def:37-103, referenced nowhere).

- Remove the two header leaks from shipped libraries: `#include <d3d9.h>` from BinkDLL.h:1 with BinkDX9SurfaceType and BinkVideo::DX9SurfaceType (dead — its GetProcAddress name `_BinkDX9SurfaceType@4` is 32-bit stdcall and cannot bind on x64); and the DirectDraw 7 chain CuiLayer.h:17 → UICanvasGenerator.h:6 → UIDirect3DTextureCanvas.h → UIDirectDrawCanvas.h → `<ddraw.h>`, plus `extern UIDirect3DPrimaryCanvas* gPrimaryDisplay` at UIRenderGlobals.h:5. Then drop directx9\include from clientGraphics.vcxproj, clientUserInterface.vcxproj and swgClientUserInterface.vcxproj and verify by temporarily renaming the vendored ddraw.h.

- Add `friend class Direct3d11;` and `friend class Direct3d11_RenderTarget;` to the outer Texture class (Texture.h:107-112 grants Direct3d9_RenderTarget but no DX11 equivalent) and `friend class Direct3d11;` to Transform.h:26-27. Audit and note that the existing Direct3d11 friends in the five buffer headers are vestigial, and that DynamicIndexBuffer.h:33-35 names the STATIC index buffer data classes (copy-paste bug).

- Archive `_client/gl11_r.dll` OUT of the runtime directory (it is not in x64-runtime-manifest.json, not managed by Stage-X64Client.ps1, and would silently shadow a broken new build once rasterMajor defaults to 11). Mine only its diagnostic strings and hand-written-replacement filename list as a work inventory; it is explicitly NOT a parity reference — Adria-DX11-NOTICE.md documents GGX/Smith/Schlick/Reinhard and a "Phase 41 terrain-reflectivity control", i.e. a PBR renderer, not a parity port. Search for the branch that produced 41 phases before committing to a from-scratch rewrite.

#### Exit criteria

- A versioned baseline JSON is published covering 36 runs (6 routes x 3 runs x 2 resolutions) with mean/p50/p95/p99/p99.9/max, CPU frame time, CPU render-submit time, GPU frame time, peak VRAM, peak private bytes, the CPU-vs-GPU-bound verdict per route (ratio plus a confirming 640x360 resolution-scaling probe), and every GfxStats counter.

- Instrumentation overhead measured: telemetry on vs off changes p95 by <= 0.5%; zone timers <= 40 ns each and <= 100 us/frame; whole-frame GPU queries <= 0.5% of frame time; no GetData call can block the main thread.

- Each of the four DX9-side fixes has its own commit and its own re-baseline run, with the batching/constant-traffic improvement attributed to it and not available to be miscredited to DX11 later.

- The resolved shader corpus (313 .vsh, 493 .psh, 280 .eft, 27 .inc) is in version control with provenance, and the 152-entry permutation manifest is frozen and reproducible from the committed enumerator.

- zCompare/stencil-compare and non-blending-pass blend-factor censuses are published.

- `grep -rn 'd3d9\.h\|ddraw\.h' src/engine/client/library src/external/3rd/library/ui` returns nothing; clientGraphics/clientUserInterface/swgClientUserInterface compile with the vendored ddraw.h renamed away.

- gl05_r still ships and behaves identically apart from the four measured fixes; the DX9 per-draw reference capture reproduces byte-identically across runs.

#### Performance gate

Re-baseline after each DX9-side fix and publish the delta. Instrumentation overhead <= 0.5% p95. No route may change its CPU-vs-GPU-bound classification as a result of instrumentation.

### Phase 1 — gl11 loads, clears, presents

**Effort:** 3-4 weeks  
**Goal:** A shippable, testable DX11 backend that initialises, reports correct capabilities, presents a cleared frame at the right resolution, and survives alt-tab and resize — with every other Gl_api slot a safe stub. Proves the contract, the build, and the swap chain in isolation.

#### Work

- Create src/engine/client/application/Direct3d11/ with build/win32/Direct3d11.vcxproj, src/win32/FirstDirect3d11.h/.cpp (`#define COMPILE_DLL 1` then FirstSharedFoundation.h), and src/shared/ carrying MemoryManagerHook, SetupDll, WriteTga, PaddedVector copied verbatim. MemoryManagerHook is mandatory (routes DLL allocations through the engine MemoryManager with `_ReturnAddress()`, which is what makes the engine safely `delete` backend-allocated *GraphicsData and makes the std::vector crossings in setLights/getOtherAdapterRects safe); SetupDll.cpp:36-48's DliHook redirecting dllexport.dll to GetModuleHandle(NULL) is required for the DLL to load at all. Link d3d11/dxgi/d3dcompiler/dxguid/winmm/delayimp/legacy_stdio_definitions (+jpeg-static). ProjectReference DllExport, DelayLoadDLLs=DllExport.dll. Match DEBUG_LEVEL per configuration exactly to the DX9 projects and the EXE. NO $(DXSDK_DIR) anywhere.

- Add `<TargetName Condition="'$(MSBuildProjectName)'=='Direct3d11'">gl11_$(SwgOutputSuffix)</TargetName>` to Directory.Build.targets and one Project() entry plus six ProjectConfigurationPlatforms rows to swg.sln, with the new GUID added to SwgClient's ProjectDependencies block (swg.sln:706-763) — that block, not a ProjectReference, is what makes /t:SwgClient build the renderer.

- Add Headless.vcxproj producing gl00_{r,o,d} to swg.sln so Gl_api changes are compile-checked against two implementations.

- `Direct3d11_Device`: CreateDXGIFactory2 → IDXGIFactory2 (query IDXGIFactory5 for tearing), adapter honouring `[Direct3d11] adapter`, D3D11CreateDevice with {11_1, 11_0} and FATAL below 11_0, default (thread-safe) flags, Os::isMainThread() asserts on context entry points. Debug layer + ID3D11InfoQueue SetBreakOnSeverity(CORRUPTION|ERROR) with a benign deny-list, default on for DEBUG_LEVEL 1/2. Wire ID3DUserDefinedAnnotation to the existing pixSetMarker/pixBeginEvent/pixEndEvent slots in ALL configurations. Report getVideoMemoryInMegabytes from DXGI_ADAPTER_DESC.DedicatedVideoMemory (must exceed 100 or SetupClientGraphics.cpp:101-111 silently disables POST and HEAT). getOtherAdapterRects from IDXGIOutput::GetDesc — Graphics.cpp:313 installs it as a raw Os hook that must stay valid for process lifetime and independent of device state. Replace FATAL_DX_HR/DXGetErrorString9 with a local HRESULT formatter.

- `Direct3d11_SwapChain`: CreateSwapChainForHwnd, FLIP_DISCARD, BufferCount 3, R8G8B8A8_UNORM, ALLOW_TEARING when supported, MakeWindowAssociation(NO_ALT_ENTER), never SetFullscreenState. resize/setWindowedMode become ResizeBuffers plus a borderless style change, firing the existing deviceLost/deviceRestored lists. Honour the Gl_install in/out contract (Graphics.cpp:301-308 reads back width/height/windowed; the four bit-depth fields arrive as -1 meaning "pick a default" per SetupClientGraphics.cpp:263-266).

- `Direct3d11_SceneTarget`: offscreen R8G8B8A8_UNORM colour + D24_UNORM_S8_UINT depth (D32_FLOAT_S8X24 fallback; stencil IS used by the shadowvolume passes), SampleDesc from `[Direct3d11] antiAliasSampleCount` validated with CheckMultisampleQualityLevels, plus a resolved shader-readable twin. Composite at present: resolve if MSAA, then a full-screen triangle from SV_VertexID applying brightness/contrast/gamma. The curve must be a verified bit-exact identity at (1.0,1.0,1.0) — and document that enabling gamma in windowed mode is an intended fix, since SetGammaRamp does nothing on a windowed D3D9 swap chain today so persisted non-default slider values (range 0.5-1.5, SwgCuiOptGraphics.cpp:170) will newly take effect.

- `ConfigDirect3d11` over section literal "Direct3d11": carry adapter, allowTearing, fullscreenRefreshRate, dynamicVertexBufferSize, dynamicIndexBufferSize, screenShotBackBuffer, createShaders, shaderCapabilityOverride, antiAlias, discardDynamicBuffersAtBeginningOfFrame. Drop usePureDevice, vertexProcessingMode, useReferenceRasterizer, maxVertex/PixelShaderVersion, disableVertexAndPixelShaders, disableDynamicTextures, doNotLockBackBuffer. Add debugLayer, driverType (warp never silent), featureLevelCap, antiAliasSampleCount, swapChainBufferCount, shaderCacheEnabled/Path, shaderDebugInfo, useConstantBufferOffsets.

- `Direct3d11.cpp`: export GetApi filling ONLY verify and install (Graphics calls verify before install), populate the remaining ~107 slots inside install with explicitly distinct function names — no DX9-style overload aliasing, which silently binds the wrong overload on a signature typo. Export GetGlApiStructSize. Register DebugFlags under "Direct3d11". Report ShaderCapability(2,0), requiresVertexAndPixelShaders true, supportsScissorRect/TwoSidedStencil/StreamOffsets/DynamicTextures/MipmappedCubeMaps/Antialias true, supportsHardwareMouseCursor FALSE so CuiLayer_CursorInterface uses the OS path it already has. All resource/draw slots are safe no-op stubs this phase.

- Implement and prove the GetGlApiStructSize guard against gl05_r, gl00_r and gl11_r.

- `Direct3d11_QueryPool` (TIMESTAMP / TIMESTAMP_DISJOINT / PIPELINE_STATISTICS, triple-buffered, DONOTFLUSH) and `Direct3d11_Metrics` implementing the Phase-0 GfxStats contract plus the DX11-only creation counters.

#### Exit criteria

- `rasterMajor=11` in options.cfg launches SwgClient_r.exe, presents a cleared frame at the requested resolution in windowed mode, and exits cleanly. gl05_r remains the default and is unaffected.

- Alt-tab, minimise/restore, and a resolution change each complete without a device loss, with deviceLost/deviceRestored firing exactly once around ResizeBuffers.

- D3D11 debug layer produces zero CORRUPTION, zero ERROR and zero hazard warnings across a launch/resize/exit cycle.

- GetGlApiStructSize matches sizeof(Gl_api) for all three DEBUG_LEVELs; a deliberately mismatched build FATALs with a readable message instead of loading.

- getShaderCapability()==0x0200 and getVideoMemoryInMegabytes()>100 are observed at Graphics::install time, so DOT3/POST/HEAT stay enabled.

- Gate D smoke list is enumerated (not yet passing). ID3DUserDefinedAnnotation markers appear in a RenderDoc capture for all 13 sorter phases.

- gl00_r.dll and gl11_r.dll both build from swg.sln; a Gl_api edit that breaks either fails the build.

- No `$(DXSDK_DIR)` string in Direct3d11.vcxproj; the project builds on a machine with no DirectX SDK installed.

#### Performance gate

Present-only (cleared frame, no scene) frame time and present-to-present jitter <= the DX9 equivalent at both resolutions. Zero state-object, input-layout, cbuffer or shader creations per frame after warm-up. Peak private bytes within 5% of DX9 for an empty frame.

### Phase 2 — UI, text and 2D render pixel-identically

**Effort:** 6-8 weeks  
**Goal:** Bring up the entire resource, state, constant and shader infrastructure and validate it end-to-end on the smallest self-contained slice: drawQuadList, the uicanvas_* / 2d_* / font templates. This is where every architectural risk is proven before the 391-program corpus conversion begins.

#### Work

- Vacate c0-c7 from `$Globals`: remove the `: register(c0)` / `: register(c4)` annotations from objectWorldCameraProjectionMatrix and objectWorldMatrix in vertex_program/include/vertex_shader_constants.inc and declare them in `cbuffer SwgPerObject : register(b3)`. Explicit register annotations pin every remaining offset, so c8..c95 do not shift and no per-shader edit is needed. This is the single change that makes the per-draw budget reachable.

- Convert vertex_shader_constants.inc and pixel_shader_constants.inc to explicit cbuffers at b0 with the register file preserved 1:1 (VS rows 8..95, PS rows 0..24). Add explicit padding only after Material.specularPower and around the float3 unitX/Y/Z at c49-c51 — every light-struct float3 is followed by a float4 that cannot straddle a 16-byte row, so LightData lands on exactly c16..c43 matching the C++ byte layout. Rename `LightData::point` (HLSL reserved word) which removes the `#define point _pt_lights` injection; delete the dead `#if VERTEX_SHADER_VERSION >= 20` bool `register(b0..b7)` block (the runtime never defines the macro and never calls SetVertexShaderConstantB); replace the D3DX-only `#pragma def(vs, c95, ...)` with a static const; drop the extendedLightData declaration at c60 (zero live references, and the C++ ExtendedLightData covers only 4 of its 8 rows so c64..c67 are never written) keeping the rows as reserved padding.

- Delete the pixel_shader_constants.inc runtime override at Direct3d9_PixelShaderProgramData.cpp:82-105 — the copy that actually wins (sku0_client.toc, and ILM_visuals' byte-identical copy) already matches the PSCR enum, so removal is behaviour-neutral. The mismatched layout the comment describes lives in a shadowed patch archive.

- Author shared_program/sm4compat.hlsli: DECLARE_SAMPLER2D/CUBE/3D(name, slot) emitting Texture2D/TextureCube/Texture3D at t#n plus SamplerState at s#n (max measured slot is s4), token-pasting tex2D/tex3D/texCUBE wrappers so all 372+18+12 call sites are untouched, the alpha-test epilogue macro, the fog-blend macro, and the b0/b1/b3 cbuffer declarations. Convert tex2DDxt5CompressedNormal to a macro (the only sampler-taking helper with live callers) and delete the two _ps_14 sqrt variants (zero live callers).

- `Direct3d11_ShaderCompiler`: D3DCompile at vs_4_0/ps_4_0 with ENABLE_BACKWARDS_COMPATIBILITY + OPTIMIZATION_LEVEL3, ID3DInclude over TreeFile (keeping the leading '../../' strip), D3D_SHADER_MACRO synthesis preserving the tcs macro mechanism and bit-identical key packing. Strip ` : register(vN)` (fxc rejects it with X3202 with and without -Gec). Generate the pixel wrapper: rename the asset entry to SwgLegacyPixelMain keeping `: COLOR`, emit `main` with `: SV_Target` calling it and applying the b1-driven discard and fog lerp. Drop _clearfp(), the LEGACY_D3DX9_31 flag, and all four source patches.

- `Direct3d11_ShaderReflection`: assert every named `$Globals` constant at 16×register with a named failure diagnostic; record the reflected extent for bounded uploads; build the sampler→texture map from GetResourceBindingDesc; cache the input signature.

- `Direct3d11_ShaderCache` + the offline baker: cache keyed on SHA-256 of (canonicalised source, contents of every include actually opened, sorted macro set, target, flags, d3dcompiler version), storing DXBC plus reflection metadata. Baker walks the 152-entry manifest and all 251 live pixel programs and fails the build on any error.

- `Direct3d11_ConstantBuffers`: CPU shadow of the VS register file (size DERIVED from VSCR_MAX plus an enumerated c95 tail, not a magic 96) and the PS file, per-register dirty tracking, DX9-shaped setters so LightManager and StaticShaderData port unchanged, at most one flush per stage per draw, bounded by the reflected extent. SwgPerObject from a Map(WRITE_NO_OVERWRITE) ring bound with VSSetConstantBuffers1 offsets on 11_1; rotating 256 B buffers with WRITE_DISCARD on 11_0 (measure and report the difference, never adopt it silently).

- Buffers: Direct3d11_{Static,Dynamic}{Vertex,Index}BufferData and _VertexBufferVectorData (names fixed by engine friends and the prior DLL's RTTI). Rings resized to 16 MB / 4 MB, NO_OVERWRITE with DISCARD on wrap, ring cursor applied by the backend since Map returns the base, vertex-index offset preserved through BaseVertexLocation, getNumberOfLockableDynamicVertices(withDiscard) reporting remaining space when false and capacity when true. Static buffers IMMUTABLE where never relocked, DEFAULT+staging where lock(readOnly) must return readable memory. Thin non-owning wrappers, unit-tested against the ShadowVolume construct-bind-destroy-draw pattern. Implement VertexBufferVector properly rather than stubbing: ClientGraphics defaults disableMultiStreamVertexBuffers TRUE so it is dormant in shipping config, but the dot3 skeletal path uses a 2-stream vector and the MFC tools can enable it.

- Direct3d11_VertexBufferDescriptorMap ported verbatim from the DX9 version (pure POD, no D3D), preserving the int8 vertexSize/offset constraint. Direct3d11_InputLayoutCache keyed on (format set, VS signature) reduced to the (texcoord index, dimension) set, with the reserved zero-fill stream for missing components and the existing DEBUG_WARNING retained.

- Direct3d11_StateObjectCache (hash-consed, construct-time only, asserted caps: blend 64, DS 128, RS 32, sampler 64) and Direct3d11_StateCache (bound-object shadow with paired call/miss counters). Rasterizer states pre-enumerated as the full 3 cull x 2 fill x 2 scissor matrix at install because cull/fill/scissor are dynamic engine state. Alpha-fade blend variants enumerated from the Phase-0 census, derived by one unit-tested function (blend forced on with the AUTHORED factors; alpha write cleared iff the authored pass did not blend), never created at draw time. Carry the Compare table over exactly as-is including the C_GreaterOrEqual/C_NotEqual swap, as one shared table with a unit test asserting the swap.

- Direct3d11_ShaderImplementationData and Direct3d11_StaticShaderData (names hard-bound by pre-seeded friends). Pass::construct builds the blend/DS descs and fetches deduplicated objects once; Pass::apply is 3 binds. StaticShaderData reproduces the tcs key packing bit-for-bit, the ARGB→float unpack, the PaddedMaterial 5-float4 layout, and binds all stages with one PSSetShaderResources + one PSSetSamplers. Preserve the ms_active/ms_pass two-level redundancy skip and the global-texture pointer-to-pointer late binding (TAG_ENVM or any leading-underscore tag). update(shader) rewrites only the constant shadow and SRV table — Bloom calls setTexture per frame.

- Direct3d11_TextureData + _TextureFormatMap: all plain _UNORM, TF_L_8 expanded to B8G8R8A8 on load with addConversion registered, TF_P_8/RGB_888/RGB_565/RGB_555 declined via the same CheckFormatSupport→TextureFormatInfo::setSupported sweep (behaviour-neutral: DX9 already maps the first three to D3DFMT_R8G8B8 which no hardware supports). Complete TextureFormatInfo.cpp's table — only 15 of TF_Count==17 entries are initialised, leaving TF_ABGR_16F/32F with name=NULL. Full LockData protocol (2D/volume/cube-face) using m_reserved for the staging handle. Load short/absent mip chains as-is and clamp MaxLOD (1,722 shipped textures affected; GenerateMips does not work on BC).

- Direct3d11_LightManager: port selection/sorting unchanged (it touches no D3D), upload LightData(28) at c16 and PixelDot3Data(5) at PS c0 by memcpy, preserve the .w/.a lane packing and the existing x64-era ambient-floor / negative-clamp / ambient-fallback behaviour verbatim, drop the extendedLightData upload, and keep the Phase-0 dirty fix.

- Direct3d11_PrimitiveEmulation: quad-list index buffer (pre-sized generously — an immutable DX11 buffer must be recreated, so a mid-frame grow stalls) and triangle-fan emulation via a generated (0,i+1,i+2) buffer plus index remapping at IB creation for SPSPT_indexedTriangleFan. drawQuadList drives the entire 2D UI so it is the primary smoke test.

- Convert and validate the UI/2D slice only: 2d_texture, 2d_view_alpha, 3d_vertex_color(_a), uicanvas_filtered/_z/_clamped, uicanvas_radar, ui_membrane, font, bad_vertex_shader, plus the nine preloaded vertexcolor templates.

- Add `Graphics::usesHalfPixelOffset()`, set in Graphics::install from the loaded backend, and make CuiManager::ms_pixelOffset (CuiManager.cpp:328) read it. Engine-side only, no ABI change. Same treatment for the -0.5f offsets in Bloom.cpp:338 and PostProcessingEffectsManager.cpp:225.

#### Exit criteria

- Gate A passes for the UI slice: every enumerated permutation compiles at vs_4_0/ps_4_0 with warnings-as-errors, and every VS input signature is satisfiable by the formats the corpus supplies. `verifyCompatibility` (currently `#if 0` at ShaderImplementation.cpp:2346, with usesTextureCoordinateSet unconditionally returning true) becomes a real build-time check.

- The reflection ABI guard passes for every UI-slice shader: each named `$Globals` constant at 16×its register, every sampler mapped to its expected texture slot.

- UI, text, cursor and the login/character-select screens render pixel-identically to the DX9 capture at uiScale 1.0 AND a non-unity uiScale, with the half-pixel offset resolved.

- Per-draw <= 3 API calls and per-shader-change <= 10, counted directly. Zero state-object / input-layout / cbuffer / shader creations per frame. Zero runtime shader compiles on a warm cache. At most one Map(WRITE_DISCARD) per ring per frame.

- Unit test: construct a stack DynamicVertexBuffer, bind, destroy, draw — debug layer clean.

- State-object cache sizes within the asserted caps; a deliberate cap breach fails loudly.

- Cold-vs-warm shader cache load time measured and published.

#### Performance gate

A UI-only route (main menu / character select) at both resolutions: p50 <= DX9 p50, p95 <= DX9 p95, p99 <= DX9 p99 x 1.05, CPU render-submit <= DX9 x 0.85. Total API calls per frame >= 5x lower than DX9. Constant bytes per draw <= 256. All zero-invariants hold.

### Phase 3 — World renders at parity

**Effort:** 3-4 months  
**Goal:** Convert the full 391-program live corpus, implement the remaining resource paths, and pass every parity gate on all six routes. At the end of this phase DX11 is provably equivalent to DX9 but is not yet the default.

#### Work

- Mechanically convert the 107 live HLSL vertex shaders: delete the 392 `: register(vN)` sites; POSITION0 → SV_Position; `: FOG` → a plain interpolant with the empirically-determined interpolation modifier (verified against a DX9 capture on a long fogged surface, since DX9's fog unit and a TEXCOORD interpolant are not necessarily the same); wrap COLOR0/COLOR1 outputs in saturate() across all 84 writers to reproduce the SM2 output-register clamp; half → float (21 files); terminate the tcs prologue cleanly instead of leaving three stray global declarations.

- Mechanically convert the 188 live HLSL pixel shaders: 485 sampler declarations → DECLARE_SAMPLER2D; `: COLOR` → `: SV_Target`; add the fog blend where the paired VS emits a fog interpolant; add the alpha-test epilogue where the pass sets alphaTestEnable; add explicit saturates to the 17 files declaring ps_1_x (whose intermediates were clamped and are not at ps_4_0). Three hand edits: holonet_ps20.psh and water_pass2_ps20.psh take `: POSITION` as a PS input (SM4 allows only float4 SV_Position) and water_pass2_ps20.psh also takes `: FOG`.

- Port the three shared function libraries (shared_program/functions.inc, pixel_program/include/functions.inc, the 472-line vertex_program/include/functions.inc, plus terrain_dot3.inc). Reproduce the two BEHAVIOURAL DX9 patches as explicit commented HLSL — the c_ambient.inc `add r7, vColor0, c16` ambient mix and the `max(ambient+diffuse, 0.85)` diffuse floor — so DX11 matches today's output. Any change to them is a separate, reviewed visual change, not part of the port. Verify lit() compiles at vs_4_0.

- Hand-rewrite the 63 live assembly pixel shaders, sequenced by draw share: a_simple.psh first (4,840 .sht reference effect/a_simple.eft), then skybox/skybox_6sided/stars/gradient_sky/cloudlayer, then dot3_terrain_imp1, then the mt_lerp/lerpmask/smap_mt/smt family, then the rest. texren_copy_c1a1.psh (every runtime-baked character face) must become a real HLSL asset replacing the string embedded at Direct3d9_PixelShaderProgramData.cpp:133-143. Only 2 files use texm3x* and 8 use _bx2/_bias/_x2 — those need genuine reverse engineering. Recover constant meaning from ShaderBuilder's s_pixelConstantMap (PixelShaderProgramView.cpp:41-60) and texture meaning from the .eft PTXM bindings.

- Hand-rewrite the 33 live assembly vertex shaders; many have HLSL siblings to model on. Port the 20 vertex_program/modules/*.inc assembly bodies into the existing HLSL functions.inc rather than one-to-one, using registers.inc as the alias table. Regenerate the permutation manifest after each conversion since the asm prologue macro form changes.

- `Direct3d11_LegacyShaderLibrary` for anything still uncompilable, keyed by asset filename, every entry gated by the reflection ABI harness. Do NOT list font.eft / defaulteffect.eft / a_skin.eft here — verified they name no shader program and are rejected inside checkOldVersionForSupport (ShaderImplementation.cpp:747-763) before createShaderImplementationGraphicsData is reached, so they are already unsupported under the shipped DX9 build and there is nothing to key on.

- Promote ShaderEffect.cpp:136/166's "no implementation passed validation" DEBUG_WARNING (a NOP in PRODUCTION) to a hard FATAL in non-PRODUCTION and an always-on counter in PRODUCTION, so a missing implementation is never silent again.

- `Direct3d11_RenderTarget`: RTVs created directly on destination textures, honouring the mip semantics Graphics.cpp:1010-1029 assumes (dimensions shifted right by mipmapLevel) and binding a correctly sized depth buffer unlike DX9 which nulled it. Plumb TCF_renderTarget through TextureRendererTemplate::fetchCompatibleTexture (currently TextureList::fetch(0,...)) so bake destinations are RT-capable; DEFAULT usage, never IMMUTABLE. Delete the 512x512 shared RT, the SYSTEMMEM twin and the GetRenderTargetData round trip. Shader-blit fallback for non-RT-capable formats.

- Texture::copyFrom as a shader blit (arbitrary src/dst rects plus format conversion; CopySubresourceRegion can neither scale nor convert, and D3DXLoadSurfaceFromSurface has no successor). Read-only lock staging on a 3-frame ring with MAP_FLAG_DO_NOT_WAIT; promote SwgCuiTcgControl and SwgCuiWebBrowserWidget textures (created with creationFlags==0 and locked every frame) to DYNAMIC.

- Replace the remaining D3DX9 dependencies: Direct3d11_ImageWriter (WIC for bmp/jpg/png/dds plus the ported TGA writer; screenshots from the SceneTarget via staging — there is no GetFrontBufferData, so the desktop-grab and monitor-offset logic deletes; break the Gl_imageFormat→D3DXIMAGE_FILEFORMAT numeric coupling with an explicit map). Direct3d11_IndexOptimizer implementing Forsyth/Tipsify to replace D3DXCreateMeshFVF+OptimizeInplace (single caller SoftwareBlendSkeletalShaderPrimitive.cpp:1311), gated on measured ACMR. Matrix math to DirectXMath, VSPS convention only, FFP branches deleted.

- `Direct3d11_WindowPresenter`: per-HWND FLIP_SEQUENTIAL swap-chain cache compositing the SceneTarget, replacing Present(NULL,NULL,hDestWindowOverride,NULL). Add a return-value check plus a one-time WARNING at the Graphics::present(HWND,...) facade so a stubbed presenter fails loudly instead of silently blacking out every tool viewport.

- Point-sprite Gl_api slots become no-ops this phase (StarAppearance conversion is Phase 7). Audit the CuiLayer_CursorInterface acquired-focus latch, which ORs `Graphics::wasDeviceReset()` (permanently false under DX11) — feed the lost/restored callbacks into the same latch if the Os hook alone is insufficient, and add alt-tab/resize to the manual smoke checklist since no scripted route covers them.

#### Exit criteria

- Gate A: all 152 vertex permutations and all 251 live pixel programs compile clean; every input signature validated against the corpus.

- Gate B (shader interiors): per-shader numerical equivalence — DX9 vs DX11 builds evaluated as pure functions over randomised inputs, VS outputs within 1e-5 relative, PS outputs within 1 LSB of 8-bit — run per file as each conversion lands.

- Gate B' (stage boundaries, which Gate B is structurally blind to): fixed triangles with per-vertex COLOR0/COLOR1 and fog values outside [0,1] rendered on both backends and diffed; the fog interpolation modifier empirically matched; an alpha-tested case with MSAA off and on.

- Gate C: deterministic golden-scene per-draw diff on all six routes — same draw order, same shader/pass identity, byte-compared constant blocks, and <= 1/255 per channel on >= 99.9% of pixels per draw with every outlier triaged. Byte-comparing constants is what separates "wrong shader" from "wrong constants".

- Gate G1: per-phase draw-call and triangle counts within 2% of the DX9 baseline on every route — the geometry-integrity gate that prevents a "faster" build passing by drawing less.

- Gate D: the ~45 engine-hardcoded templates smoke-checked by name, including defaultshader/defaulteffect fallback behaviour preserved exactly as DX9 leaves it (verified already-unsupported, so NO new 2.0 implementations are authored — that would be a visual change in the opposite direction).

- All nine presentToWindow consumers render correctly on gl11: five TerrainEditor views, Viewer, TextureBuilder, SwgGodClient, swgClientQtWidgets.

- Texture-bake readbacks per frame = 0 on route R5; blocking staging maps per frame = 0 on all routes.

- ACMR on a fixed reference set of skinned meshes within 5% of the D3DX result.

- Debug layer clean on all six routes.

#### Performance gate

Gate G2 on all six routes at both resolutions: p50 <= DX9 p50, p95 <= DX9 p95, p99 <= DX9 p99 x 1.05, no frame above max(50 ms, DX9 max), CPU render-submit <= DX9 x 0.85, peak VRAM and private bytes within an explicit budget. All zero-invariants = 0. Plus a GPU-time-only criterion on the fill-bound routes (terrain, water, foliage) at 3840x1080 with a stated budget for the per-pixel fog and alpha-test-as-discard changes, since a GPU regression there can hide under CPU cost at 1080p and under an already-GPU-bound frame at 3840x1080.

### Phase 4 — DX11 becomes the default

**Effort:** 1 week  
**Goal:** Flip the out-of-the-box path with a one-config-line rollback, and make legacy user configs land on DX11 instead of a DirectX 9 error dialog.

#### Work

- Coerce any `rasterMajor` outside {0,11} to 11 with a non-fatal WARNING, placed before the DLL name is formatted at Graphics.cpp:239. This MUST land in the same commit as the default flip (it cannot land earlier without breaking the DX9 default) and its absence is the single most likely field failure.

- Change data.rasterMajor 5 → 11 in SetupClientGraphics.cpp:255 (setupDefaultGameData: SwgClient, SwgGodClient, SwgHeadlessClient, ParticleEditor, ShipComponentEditor) and :303 (setupDefaultMFCData: Viewer, TerrainEditor, TextureBuilder, NpcEditor, swgClientQtWidgets). Leave :279 at 0 for console until Phase 6 resolves rasterMajor entirely.

- Update _client/options.cfg rasterMajor=5 → 11 and migrate the shipped `[Direct3d9] fullscreenRefreshRate=120` section to `[Direct3d11]`.

- Fix SwgClientSetup: Options.cpp:659-671 writes only 5/6/7 and would silently revert any user who opens the tool. Either emit 11 or stop emitting the key. Its DX9-caps min-spec probe (ClientMachine.cpp:333-417: DirectDrawCreate for VRAM, Direct3DCreate9 for shader versions and EnumAdapterModes) must move to DXGI adapter enumeration plus D3D11 feature levels, or the tool must be retired — note it is x86 and cannot be staged by Stage-X64Client.ps1.

- Switch CuiManager::ms_pixelOffset and the Bloom / PostProcessingEffectsManager -0.5f offsets to the backend-conditional value in this same commit. Shipping DX11 as default with the D3D9 half-pixel correction still applied blurs every glyph, icon and radar texel and gets misdiagnosed as a filtering problem.

- Build-X64Client.ps1 / Stage-X64Client.ps1 and the God Client equivalents carry gl11 ALONGSIDE gl05/06/07 for exactly one release so rollback is a config edit. Add d3dcompiler_47.dll from the SDK redist to the staged file list.

- Publish the rebuilt shader assets as one TREE0005 archive (TreeFileRspBuilder + TreeFileBuilder) mounted as `searchTree_00_9` in client.cfg — priority 9, above swgsource_3.0.tre (8) and ILM_visuals.tre (5), below maxSearchPriority (12). A priority-0 searchTree entry would be silently shadowed by sku0_client.toc, and TREE0006 without a matching .toc loads zero files. Add a publish manifest and wire the archive build plus the shader bake into the Build/Stage scripts, neither of which currently invokes TreeFileBuilder.

#### Exit criteria

- A fresh install with no options.cfg launches on DX11.

- An options.cfg containing rasterMajor=5 (and 6, and 7, and 99) launches on DX11 with a WARNING in the log and NO MessageBox.

- Running SwgClientSetup_r.exe does not revert the renderer.

- x64-runtime-manifest.json lists gl11_$suffix.dll; rollback to DX9 is a single config line and is tested.

- The shader TRE at priority 9 wins over sku0_client.toc and ILM_visuals.tre for every rewritten path, verified by the resolver tool.

- Docs (docs/x64-gameplay-client.md, docs/god-client.md, README.md) updated to name gl11 and the corrected output path src/build/win32/x64/$(Configuration).

#### Performance gate

Re-verify Gate G2 on the actual shipped configuration (3840x1080, windowed=1) after the flip, not just the benchmark profile. R0 canary within 2% of the Phase-3 reference.

### Phase 5 — Delete DX9 — one subtraction commit

**Effort:** 3-4 weeks  
**Goal:** Remove every Direct3D 9, Direct3D 7 and DirectDraw artifact from the tree, the build, the prerequisites and the shipped client, gated on all nine DX9-linking projects being retired or ported first.

#### Work

- Entry gate checklist, all nine DX9-linking projects: Direct3d9 x3 (deleted here), ShaderBuilder (replaced by a headless validate+bake CLI in this commit — its CompileAsm path assembles at both vs_1_1 and vs_2_0 via D3DXAssembleShader which has no D3D11 successor, and its preview is a separate full D3D9 renderer at RenderView.cpp:209-345; preserve s_pixelConstantMap as committed documentation), UIBuilder and UIFontBuilder (retire or port — UIFontBuilder is nearly free: replace `<ddraw.h>` in FontImageExporterDDS.cpp:12 with the in-tree clientGraphics/Dds.h which already defines DDSPF_A8R8G8B8 and the DDSCAPS values, and drop ddraw.lib; UIBuilder's DX7 `<d3d.h>`/`<d3dtypes.h>` have no modern SDK replacement so it must be retired or rewritten), SwgClient, SwgClientSetup, SwgGodClient (strip DX9 libs and paths from all six configs each — no source in SwgClient or SwgGodClient references D3D at all, and the gl0X import libs are pointless since the renderer is LoadLibrary'd).

- Delete the Direct3d9 source tree, its three vcxproj, the three swg.sln Project blocks and all 26 GUID back-references across 14 projects, and the three Directory.Build.targets TargetName rows.

- Delete the vendored SDK and the DXSDK_DIR apparatus: src/external/3rd/library/directx9 (35 tracked x86 files, ~24 MB, currently on the x64 library path producing tolerated LNK4272), deps/x64/lib/{d3d9,d3dx9,DxErr}.lib, both directx-* manifest.json packages plus their vendoredInputs, Get-DirectXSdkStatus, the directx-redist install branch, and every $env:DXSDK_DIR assignment. This removes a 599 MB SDK from contributor onboarding and a 100 MB legacy runtime from end users.

- Scrub external/3rd/library/directx9 from all 36 vcxproj AdditionalIncludeDirectories and the ~49 unreferenced build/win32/*.rsp files (no vcxproj/props/targets references any .rsp, so they are orphaned legacy inputs that would become the only in-tree references to a deleted directory).

- Delete the orphaned DirectX trees: src/external/3rd/application/DXTex plus UiBuilder's dxtex.exe shell-out (ImportImageDialog.cpp:171, ImportImage.cpp:155 — a live functional dependency on a legacy SDK utility), src/engine/client/application/UiBuilder (161-file duplicate not in swg.sln), src/engine/client/application/SoePix and PixCounter.cpp:58's hard-coded "Microsoft DirectX 9.0 SDK (October 2004)" LoadLibrary inside server-shared sharedDebug, src/engine/client/application/SetBrightnessContrastGamma (a DX9 gamma-ramp recovery tool made pointless by the composite pass), src/engine/client/application/CreateShaderTemplate (unbuilt, hard-codes effect/simplemt1z.eft).

- Remove all Direct3d8/Direct3d9/OpenGL friend declarations from the ten clientGraphics headers and Transform.h, and the DIRECT3D8_EXPORTS define from DllExport.vcxproj (all six configs). Remove the TUIDirect3D*/TUIDirectDraw* enumerators from UITypeID.h after confirming nothing serialises UITypeID values.

- Delete gl05_r.dll, gl06_r.dll, gl07_r.dll from _client and from the Stage scripts; delete the System32\d3dx9_43.dll hard requirement and the d3d9/d3dx9_43/ddraw local-DLL cleanup list. Delete the broken Win32 scripts/Build-GodClient.ps1 and Stage-GodClient.ps1 which reference the nonexistent src/compile layout.

#### Exit criteria

- `grep -rniE 'd3d9|d3dx9|d3dx11|ddraw|dxerr|DXSDK_DIR|D3DPOOL|D3DFMT|IDirect3D' src/ scripts/ docs/ deps/` returns only deliberately documented exceptions (the GraphicsOptionTags TAG_DX9 asset-compat mapping, and historical comments).

- A clean checkout builds SwgClient and gl11 on a machine with NO DirectX SDK installed and no d3dx9_43.dll in System32.

- Setup-X64BuildPrerequisites.ps1 installs only VS Build Tools and the Windows SDK; Test-X64BuildPrerequisites.ps1 passes without any DirectX check.

- _client contains exactly one raster DLL (gl11) and no d3dx9/d3d9/ddraw dependency in any PE import table.

- ShaderBuilder is either gone or is a headless CLI that validates a .vsh/.psh across every manifest permutation with no D3DX and no MFC.

#### Performance gate

Subtraction only. Re-run the R0 canary and confirm p95 within 2% of the Phase-4 reference and all zero-invariants unchanged.

### Phase 6 — Trim the contract

**Effort:** 2-3 weeks  
**Goal:** Remove the DX9-shaped slots from Gl_api and the numeric backend indirection, now that both remaining implementations (gl11 and gl00) build and can be changed in lockstep.

#### Work

- Reduce Graphics.cpp:210-236 to two branches (0 → Headless, 11 → DX11), delete the DX8 warning and the dead DEBUG_FATAL arm, and fix the PRODUCTION MessageBox text that still names DirectX 9. Then retire `rasterMajor` entirely: remove the key from ConfigClientGraphics.cpp:83/133 and SetupClientGraphics::Data, collapse the DLL_NAME_FORMAT sprintf indirection, and stop SwgClientSetup emitting it — so no stale config can select anything.

- Keep `GraphicsOptionTags::set(TAG_DX9, true)` with a comment recording it as a permanent asset-compatibility mapping. Thousands of shipped .eft OPTN chunks carry the DX9 tag and setting it false makes those implementations silently vanish via ShaderImplementation.cpp:563-576; measured OPTN gating selects an option-gated implementation for 175 of 277 effects.

- Trim Gl_api and Graphics in lockstep with gl11 and gl00: delete setTextureTransform (zero callers outside Graphics.cpp:1563, pure fixed-function residue), getOneToOneUVMapping (zero callers, and its DX9 half-texel formula is wrong for DX11 anyway), and the five constant-true supports* queries (supportsScissorRect, supportsTwoSidedStencil, supportsStreamOffsets, supportsDynamicTextures, supportsMipmappedCubeMaps) with their engine callers updated — which also deletes the Radeon-8500 mipmapped-cubemap workaround at Texture.cpp:594-600.

- Change lockBackBuffer's signature from `const RECT *` to `const Gl_rect *` and consider `setLights(Light const *const *, int)`, removing the last Win32 type and the STL crossings from the ABI. Both are safe now that Headless is buildable.

- Shrink the VS constant buffer from the c95 tail once the last assembly consumer is gone (only 3 live assembly shaders referenced c95/cLog2e and all are rewritten by now), and make the size a derived constant rather than a literal 96.

- Prune the dead asset tiers: the 173 .vsh and 242 .psh unreachable at capability 2.0, vertex_program/modules/**, and the unreachable 0.2/0.3/1.1/1.4 implementations where a 2.0 sibling exists. Re-emitting the .eft requires a scriptable IFF writer — treat as optional, since leaving dead tiers costs load time not correctness. Keep ShaderImplementationPassStage and the STAG loader permanently: load_0000 through load_0003 parse every Pass (and therefore STAG) before checkOldVersionForSupport runs, so removing the reader makes font.eft, defaulteffect.eft, a_skin.eft, e_shadowblob.eft and gradient_sky.eft unreadable. Fix the three shader/*.dds files that are actually FORM SSHT (pt_anim_atom_ring, pt_dust_rocks, pt_rays_hourglass).

- Decide and record whether the Win32 platform configurations survive across all 129 swg.sln projects; they point at the nonexistent src/compile layout and only x64 is built or gitignored.

#### Exit criteria

- Gl_api contains no DX9-shaped slot; sizeof(Gl_api) changes and both gl11 and gl00 build and load with the size guard passing at all three DEBUG_LEVELs.

- No config value selects a nonexistent DLL; a config with an arbitrary rasterMajor is simply ignored.

- All six routes still pass Gate C pixel-diff and Gate G2 after the trim.

- The three asset-ABI register headers live only in clientGraphics; no tool reaches across a layering boundary.

#### Performance gate

R0 canary within 2% of the Phase-5 reference; all zero-invariants hold; no change in per-phase draw counts.

### Phase 7 — Win — beat the DX9 baseline

**Effort:** 2-3 months  
**Goal:** Spend the headroom created by DX11's cheaper state and constant paths on measured wins, each landing as an individually attributable change with its own gate.

#### Work

- Delete the one-sided stencil shadow fallback (ShadowVolume.cpp:1544-1675 adds 4 sorter entries per caster two-sided but 6-8 one-sided) and the per-caster-per-frame ProxyLocalShaderPrimitive heap allocation. Two-sided stencil is unconditional in DX11. This changes draw counts so it MUST come after Gate G1 parity is established.

- Flora instancing: ClientDynamicRadialFloraManager rebuilds all four vertices of 500 billboards per frame (160 bytes/quad); replace with one DrawIndexedInstanced and 32-byte instances reusing the existing 3000-index static IB, moving quad expansion and sway into the vertex shader.

- Particle quads in two measured steps: 6 unindexed vertices → 4 indexed via the existing quad IB (free 33% bandwidth cut), then full instancing. Fold the per-emitter cull-mode toggles into shadowed RSSetState.

- Star field: replace point sprites with instanced camera-facing quads of FIXED 2-pixel screen-space size derived from the SceneTarget dimensions via VSCR_viewportData — NOT distance-attenuated. Verified StarAppearance.cpp:337-354 sets only setPointSize and setPointSpriteEnable and there is no setPointScaleEnable caller anywhere, so D3DRS_POINTSCALEENABLE stays FALSE and DX9 applies no attenuation; adding it would change the entire space skybox. Then delete the six point-sprite Gl_api slots and their Graphics facade methods.

- Heat composite batching: batch all heat primitives into one pass per frame, replace copyFrom with CopySubresourceRegion, rely on the shadowed viewport/scissor to eliminate the 3-4 redundant setViewport calls per primitive, and drop the per-composite 4-vertex DynamicVertexBuffer.

- Post-process and bloom: drive all full-screen passes from SV_VertexID with no vertex buffer, removing four per-frame stack-constructed DynamicVertexBuffers, and fold the 16-register user-constant uploads into the b1/PS-user cbuffer. Consider a compute-shader blur to remove 4 of 6 raster passes.

- Static prop / duplicate-mesh instancing, gated on the baseline showing >= 15% of opaque draws in R1/R2 are duplicate-mesh draws (and measured against achievable run length per phase, since pushCell flushes phases 0-5 on every cell push).

- Async texture creation on the AsynchronousLoader thread now that the device is thread-safe (relax the Os::isMainThread assert, audit TextureList), gated on a >= 50% reduction in zone-entry frames over 33 ms with no steady-state regression.

- Optional full constant-buffer frequency split (PerFrame / PerView / PerMaterial / PerLightSet out of $Globals), gated on measurement: the monolithic b0 costs ~1.5 KB per shader-change and ~0 per draw, which is likely already a net CPU win over DX9's 800 x 8 SetVertexShaderConstantF calls — split only if the measured constant traffic proves otherwise.

- Optional MSAA on the SceneTarget with resolve, default-off until it costs <= 15% GPU frame time at 1920x1080 x4. Explicitly decide whether matching today's effectively-off behaviour is acceptable.

- GPU skinning research spike, time-boxed. CPU skinning is likely the largest main-thread cost in character-dense scenes, but SoftwareBlendSkeletalShaderPrimitive's system vertex buffer is also consumed by collide() and ShadowVolume::addPrimitive, so the spike must determine whether those can be served from a lower-frequency CPU skin (the every-other-frame path already exists) while the GPU drives rendering. Free win found here regardless: prepareToDraw performs the dynamic-VB lock and full memcpy every frame even when skinning was skipped.

#### Exit criteria

- Each item lands as its own commit with its own before/after route measurement; no bundling.

- Visual comparison at 10 fixed frames per route shows no difference for any item except MSAA (if enabled) and the star field (documented, reviewed).

- Debug layer clean; all zero-invariants still hold; state-object cache sizes still within caps.

#### Performance gate

Gate G3: on the CPU-bound routes DX11 p95 <= DX9 p95 x 0.85. R3 draw calls -30% and dynamic VB bytes -60%. R4 p95 -20% and dynamic VB bytes -60%. R5 p99 -30%. R1/R2 shadow-phase draw calls -30%. Zone-entry frames over 33 ms not increased.

## Deletions for a zero-DX9-artifact tree

Phase tags are `P0`..`P7`.

- P0 _client/gl11_r.dll — archived out of the runtime dir (1,417,728 B, Jul 21, predates the gl05/06/07 build, absent from x64-runtime-manifest.json, unmanaged by Stage-X64Client.ps1; would silently shadow a broken new build)

- P0 src/engine/client/library/clientGraphics/src/Bink/BinkDLL.h:1 `#include <d3d9.h>` + BinkDLL.h:192 BinkDX9SurfaceType + BinkDLL.cpp:60/120/183 + BinkVideo.h:272 BinkVideo::DX9SurfaceType

- P0 src/external/3rd/library/ui/src/win32/UICanvasGenerator.h:6 include of ../../../application/UiBuilder/UIDirect3DTextureCanvas.h (the DirectDraw7 leak into the shipped client's UI libraries)

- P0 src/external/3rd/library/ui/src/win32/UIRenderGlobals.h:5 `extern UIDirect3DPrimaryCanvas* gPrimaryDisplay`

- P0 src/external/3rd/library/ui/src/win32/UICanvasInitialization.cpp (ddraw.h + d3d.h; confirm uncompiled, then delete)

- P0 external/3rd/library/directx9/include from clientGraphics.vcxproj, clientUserInterface.vcxproj, swgClientUserInterface.vcxproj (all six configs each)

- P0 Gl_api::setDynamicIndexBufferSize slot (Gl_dll.def) + Graphics.h/.cpp:1695-1698 + Direct3d9.cpp:224/3545 (unguarded NULL deref, slot never assigned, zero callers)

- P0 Gl_metrics (src/engine/client/library/clientGraphics/src/win32/Graphics.def:37-103, referenced nowhere)

- P4 _client/options.cfg `[Direct3d9]` section (migrated to [Direct3d11])

- P5 src/engine/client/application/Direct3d9/ — entire tree, 47 files including p4integrate.bat

- P5 src/engine/client/application/Direct3d9/build/win32/{Direct3d9,Direct3d9_ffp,Direct3d9_vsps}.vcxproj

- P5 src/build/win32/swg.sln — three Project() blocks and all 26 ProjectDependencies back-references to {E89A79A9-…}, {2889F119-…}, {CF3B1EE2-…}, including SwgClient's block at :706-763

- P5 Directory.Build.targets — the three Direct3d9/_ffp/_vsps TargetName rows

- P5 src/external/3rd/library/directx9/ — include/ (24 headers incl. d3dx9*.h, dxerr9.h, d3d.h, d3dtypes.h) and lib/ (11 x86 import libs + d3dx9d.dll), 35 tracked files, ~24 MB

- P5 deps/x64/lib/d3d9.lib, deps/x64/lib/d3dx9.lib, deps/x64/lib/DxErr.lib

- P5 deps/build-prerequisites/manifest.json — the directx-sdk-june-2010 and directx-runtime-june-2010 package entries and the d3d9/d3dx9/DxErr vendoredInputs

- P5 scripts/Test-X64BuildPrerequisites.ps1:235-264 Get-DirectXSdkStatus and its $missing entry and status field

- P5 scripts/Setup-X64BuildPrerequisites.ps1:153-176 the directx-redist installMode branch and both DirectX switch arms at :215-229

- P5 scripts/Build-X64Client.ps1:49 and scripts/Build-X64GodClient.ps1:61 $env:DXSDK_DIR

- P5 scripts/Build-GodClient.ps1 and scripts/Stage-GodClient.ps1 (broken Win32 scripts referencing the nonexistent src/compile/win32/Direct3d9* layout)

- P5 scripts/Stage-X64Client.ps1 and Stage-X64GodClient.ps1 — gl05/gl06/gl07 entries, the System32\d3dx9_43.dll hard requirement, and d3d9.dll/d3dx9_43.dll/ddraw.dll from $localSystemDllNames

- P5 SwgClient.vcxproj and SwgGodClient.vcxproj — gl05_r.lib/gl06_r.lib/gl07_r.lib and d3d9.lib/d3dx.lib/d3dx9.lib/d3dx9dt.lib/ddraw.lib/DxErr9.lib from AdditionalDependencies, plus $(DXSDK_DIR)Lib\x64, the three src/compile/win32/Direct3d9* paths and directx9\lib from AdditionalLibraryDirectories (all six configs each)

- P5 external/3rd/library/directx9/include from the remaining ~33 vcxproj AdditionalIncludeDirectories

- P5 ~49 orphaned src/**/build/win32/*.rsp files containing directx9 paths (no vcxproj/props/targets references any .rsp)

- P5 src/external/3rd/application/DXTex/ (26-file MFC DDS editor, not in swg.sln)

- P5 src/engine/client/application/UiBuilder/ (161-file orphan duplicate; swg.sln references only src/external/3rd/application/UiBuilder)

- P5 UiBuilder dxtex.exe shell-out — ImportImageDialog.cpp:171 and ImportImage.cpp:155

- P5 src/engine/client/application/SoePix/ (SoePix.cpp/.def/.sln, absent from swg.sln)

- P5 src/engine/shared/library/sharedDebug/src/shared/PixCounter.cpp:58 SoePix.PIXPlugin LoadLibrary + the PixCounter facility (its only call site is already inside a #if 0 at Game.cpp:1250)

- P5 src/engine/client/application/SetBrightnessContrastGamma/ (DX9 gamma-ramp recovery tool, obsolete once gamma is a composite pass)

- P5 src/engine/client/application/CreateShaderTemplate/ (unbuilt, no vcxproj, hard-codes effect/simplemt1z.eft)

- P5 src/engine/client/application/ShaderBuilder/src/win32/RenderView.h/.cpp (private IDirect3DDevice9 + full FFP preview renderer) and the D3DXAssembleShader/D3DXCompileShader/D3DXDisassembleShader/ID3DXConstantTable paths in PixelShaderProgramView.cpp:162-420 and VertexShaderProgram.cpp:133-273

- P5 ShaderBuilder.vcxproj d3d9.lib;d3dx9.lib (all six configs) and its relative includes of the three Direct3d9_*Registers.h

- P5 src/external/3rd/application/UiFontBuilder/FontImageExporterDDS.cpp:12 `#include <ddraw.h>` and ddraw.lib from UIFontBuilder.vcxproj (replaced by the in-tree clientGraphics/Dds.h)

- P5 src/external/3rd/application/UiBuilder — retire the tool, or delete UIDirect3DPrimaryCanvas.cpp/UIDirect3DTextureCanvas.cpp/UIDirectDrawCanvas.cpp/UIDirectDrawPrimaryCanvas.cpp/UIDirectDrawOffscreenCanvas.cpp/DDSCanvasGenerator.cpp and d3d9.lib/d3dx.lib/ddraw.lib

- P5 friend class Direct3d8* / Direct3d9* / OpenGL declarations in Texture.h, StaticShader.h, StaticShaderTemplate.h, ShaderEffect.h, ShaderImplementation.h, StaticVertexBuffer.h, DynamicVertexBuffer.h, StaticIndexBuffer.h, DynamicIndexBuffer.h, VertexBufferVector.h and sharedMath/Transform.h:26-30

- P5 DllExport.vcxproj DIRECT3D8_EXPORTS define (all six configs)

- P5 src/external/3rd/library/ui/src/shared/core/UITypeID.h:105-109 TUIDirect3DPrimaryCanvas / TUIDirect3DTextureCanvas / TUIDirectDrawCanvas / TUIDirectDrawOffscreenCanvas / TUIDirectDrawPrimaryCanvas

- P5 _client/gl05_r.dll, _client/gl06_r.dll, _client/gl07_r.dll and their .lib/.exp/.pdb in src/build/win32/x64/$(Configuration)

- P6 Gl_api::setTextureTransform (+ Graphics.h:209 / Graphics.cpp:1563) and Gl_api::getOneToOneUVMapping (zero callers each)

- P6 Gl_api::supportsScissorRect, supportsTwoSidedStencil, supportsStreamOffsets, supportsDynamicTextures, supportsMipmappedCubeMaps (constant-true in DX11) and their engine branch sites, including the Radeon-8500 workaround at Texture.cpp:594-600

- P6 rasterMajor — ConfigClientGraphics.cpp:83/133, SetupClientGraphics::Data, SwgClientSetup Options.cpp:659-671, and the DLL_NAME_FORMAT sprintf indirection in Graphics.cpp

- P6 the c95 tail of the VS constant buffer, once the last assembly consumer is rewritten

- P6 173 unreachable .vsh and 242 unreachable .psh, vertex_program/modules/**, the ps_1_4 sqrt-approximation helpers in shared_program/functions.inc, and the stray ILM vertex_program/material_diffuse.inc

- P7 Gl_api::setPointSize / setPointSizeMin / setPointSizeMax / setPointScaleEnable / setPointScaleFactor / setPointSpriteEnable and their Graphics facade methods, after StarAppearance is converted to instanced quads

- P7 ShadowVolume.cpp:1544-1675 one-sided stencil path and the per-caster ProxyLocalShaderPrimitive heap allocations

- NOT DELETED (documented exception) src/engine/client/library/clientGraphics/src/shared/ShaderImplementation.h:435-570 ShaderImplementationPassStage and its STAG loader — load_0000..load_0003 parse every Pass, and therefore STAG, before checkOldVersionForSupport runs, so the reader is still required to READ font.eft, defaulteffect.eft, a_skin.eft, e_shadowblob.eft and gradient_sky.eft. Only the backend translation (Direct3d9_ShaderImplementationData.cpp:25-124 tables and :136-187/:365-386 cascade) is removed.

- NOT DELETED (documented exception) GraphicsOptionTags TAG_DX9 set true for rasterMajor 11 in Graphics.cpp:230-233 — a permanent asset-compatibility mapping; setting it false makes option-gated implementations vanish for 175 of 277 effects via ShaderImplementation.cpp:563-576.

## Performance gates

- G0 BASELINE ADMISSION (blocks every DX11 merge). A published DX9 baseline JSON covering 6 routes x 3 runs x 2 resolutions, each route admitted only at <= 3% run-to-run p95 spread, measured with vsync off (allowTearing=true — the default false forces D3DPRESENT_INTERVAL_ONE), forced dt = 1/60 s, AsynchronousLoader drained, 300 warm-up frames discarded, 1800 frames measured. Records frame-time mean/p50/p95/p99/p99.9/max, CPU frame time, CPU render-submit time, GPU frame time, peak dedicated VRAM, peak process private bytes, the CPU-vs-GPU verdict, per-phase draws and triangles, all bind call/miss counts, VS/PS constant calls and registers, dynamic VB/IB bytes and discards, RT switches, and peak draws per frame.

- BOUND CLASSIFICATION (re-applied every phase). GPU/CPU frame-time ratio > 0.95 = GPU bound, < 0.70 = CPU bound, else mixed, each confirmed by a 640x360 resolution-scaling probe (< 10% improvement at one-ninth the pixels = CPU bound). A route changing classification between phases is a regression signal requiring investigation.

- G1 GEOMETRY INTEGRITY (before any performance claim). Per-phase draw-call and triangle counts within 2% of the DX9 baseline on every route. Catches lost draws, duplicated passes, and mis-derived DrawIndexed arithmetic from the DX9 six-argument to DX11 three-argument collapse — without it a "faster" build can pass by drawing less.

- G2 PARITY (per route, all six, both resolutions). p50 <= DX9 p50 x 1.00; p95 <= DX9 p95 x 1.00; p99 <= DX9 p99 x 1.05; no frame above max(50 ms, DX9 max); CPU render-submit <= DX9 x 0.85 (DX11 must be cheaper — removing the fixed-function pipeline alone returns 16 texture-stage probes plus a SetTransform and a SetMaterial per draw, since the shipped gl05_r defines both FFP and VSPS); peak VRAM and private bytes within an explicit budget.

- G2 ZERO-INVARIANTS (hard zeros, measured after warm-up, every route). State-object creations = 0. Input-layout creations = 0. Shader compiles = 0. Constant-buffer creations = 0. Texture-bake readbacks = 0 (for RT-capable destinations). Backbuffer maps = 0. Blocking staging maps = 0. Permutation-manifest misses = 0. Any nonzero value means lazy creation or a synchronous readback leaked into the frame.

- CALL BUDGETS (counted directly). <= 3 API calls per draw (one SwgPerObject offset bump plus Draw/DrawIndexed) against ~28 in DX9. <= 10 API calls per shader change (3 state-object binds, one batched PSSetSamplers, one batched PSSetShaderResources, VSSetShader, PSSetShader, IASetInputLayout, <= 2 cbuffer rebinds) against 70-90 in DX9. Total API calls per frame >= 5x lower than DX9.

- CONSTANT TRAFFIC (revised — the DX9-registers x 16 B ceiling is arithmetically unreachable with a monolithic $Globals and is replaced). Per-draw constant bytes <= 256 (the SwgPerObject slice), no per-draw Map on any buffer larger than 256 B and no per-draw buffer rename. Per-shader-change bytes bounded by the reflected $Globals extent, never the full allocation. Total constant bytes per frame recorded and gated against a measured budget set in Phase 2, with the Phase-7 frequency split triggered only if that budget is exceeded.

- RING PRESSURE. <= 1 Map(WRITE_DISCARD) per frame on the vertex ring and 1 on the index ring (the beginFrame discard); every other lock WRITE_NO_OVERWRITE. Dynamic VB/IB bytes per frame <= DX9. Ring sizes must leave >= 2x headroom over measured peak usage on the worst route; > 50% utilisation means resize before merging.

- REDUNDANT-BIND HIT RATES (from paired Release call/miss counters). State objects >= 90%. Input layout >= 95% (the DX9 baseline is a permanent 100% MISS because forceVertexDeclaration never assigns ms_vertexDeclaration, so there is no excuse for not shadowing). SRVs >= 60%. Viewport and scissor >= 95% (both uncached in DX9, and setViewport is called 3-4 times per heat primitive plus 4 times in Bloom).

- STATE-OBJECT CACHE CAPS (asserted at runtime). Blend <= 64, depth-stencil <= 128, rasterizer <= 32, sampler <= 64. The asset census predicts counts well inside these; exceeding one means something dynamic leaked into a cache key and is a design defect, not a tuning issue.

- INSTRUMENTATION OVERHEAD. Telemetry on vs off changes p95 by <= 0.5%. Zone timers <= 40 ns each and <= 100 us/frame. Whole-frame GPU queries <= 0.5% of frame time; optional per-phase mode <= 2% and default-off. Any GetData that would block the main thread is a defect — queries are always read 3 frames late.

- GPU-TIME-ONLY GATE (per phase, fill-bound routes, 3840x1080). Explicit GPU frame-time budget for the two largest semantic ports: per-pixel fog across ~90 shader pairs, and alpha-test-as-discard on 422 passes with its early-Z implications. Frame-time percentiles alone cannot see these — they hide under CPU cost at 1080p and under an already-GPU-bound frame at 3840x1080. The discard must be placed before any dependent texture fetch that would sink it, and the fog blend folded into an existing trailing lerp/modulate where one exists.

- INVISIBLE-GPU-REGRESSION GATE. ACMR on a fixed reference set of skinned meshes under the in-house Forsyth/Tipsify optimiser within 5% of the D3DXMESHOPT_VERTEXCACHE result. This is the one regression class the draw-call and state-change gates structurally cannot detect: dropping the optimiser raises GPU vertex-shading cost on every character while every API-level counter stays flat.

- LOAD AND HITCH GATES (measured separately from steady-state percentiles). Scripted zone entry: cold-shader-cache load time reported not gated; warm-cache load time within 5% of DX9; frames exceeding 33 ms during zone entry must not increase. Required because DX9 hides pixel-shader compile cost behind the PEXE fallback that DX11 cannot use at all, so a warmed-route frame-time gate would completely miss a first-encounter compile-hitch regression.

- VISUAL PARITY IS PART OF THE PERFORMANCE GATE. 10 fixed frame indices per route, screenshots diffed DX9 vs DX11, every difference explicitly documented and accepted. A faster but visually degraded build fails. Includes UI text at uiScale 1.0 and non-1.0 (the half-pixel trap), the star field, the heat composite, and a non-default gamma value on both backends.

- G3 WIN GATE (Phase 7 exit). CPU-bound routes: p95 <= DX9 p95 x 0.85. R3 draw calls -30% and dynamic VB bytes -60%. R4 p95 -20% and dynamic VB bytes -60%. R5 p99 -30%. R1/R2 shadow-phase draw calls -30%.

- POST-MERGE PROTECTION. R0 static_360 in CI on every commit touching clientGraphics or the DX11 backend, gated at p95 within 2% of the sanctioned reference with all zero-invariants holding. Full 36-run matrix nightly. Reference values updated only by an explicit reviewed re-baseline commit, never automatically — an auto-drifting reference converts the no-regression requirement into a no-op.

- ATTRIBUTION DISCIPLINE (enforced at review). Every DX9-side fix — the four pointer-truncating sort keys, light over-dirtying plus LightBitSet caching, the O(n^2) particle sort, the per-frame terrain rebuild, RenderWorldCommander's per-object std::set insert — lands as its own commit on the DX9 baseline with its own re-baseline. A DX11 pull request whose diff touches any of these outside the backend is rejected until split out.

- PROHIBITED WITHOUT EVIDENCE. Deferred contexts and command lists require all three: main-thread render submit > 30% of CPU frame time, draws > 8000/frame, and at least two threads able to produce render lists. None hold today — Game.cpp:1224-1246 is a single-threaded loop and ShaderPrimitiveSorter.cpp:725 interleaves prepareToDraw CPU work with submission, so there is no separable submit phase and the ~10-50 us per-list overhead is pure loss.

## Risks

- Assembly shaders are the schedule's critical path and cannot be de-risked by tooling. 96 live programs (33 VS, 63 PS) have no compilation path at all — D3DCompiler_47 cannot assemble SM1-3 — and 2 of them (a_specmap_bump_diffuse.psh, a_specmap_bump_emismap.psh) use texm3x* register-combiner matrix ops with no mechanical translation. a_simple.psh alone is referenced by 4,840 .sht. If any rewrite is wrong the failure is a wrong-looking material, not a compile error.

- The permutation manifest is not provably closed. `StaticShader::setTextureCoordinateSet` (StaticShader.cpp:401-415) is a public runtime mutator that clones the tcs map and changes the permutation key, called from TextureBuilder. Under offline-bake-only, a permutation outside the 152-entry manifest renders nothing with no recovery. Mitigated by shipping d3dcompiler_47.dll as an instrumented fail-safe plus a CI-gated miss counter, but the manifest closure remains an assumption enforced by assertion rather than proven by construction.

- Gl_api's layout is DEBUG_LEVEL-dependent with no in-band version field, discriminated only by the _r/_o/_d filename suffix. A mismatched pair loads successfully and then calls the wrong function through every shifted slot. The Phase-0 GetGlApiStructSize guard closes it, but until then any experimental build combination is a silent catastrophic failure, and Headless.cpp's `void**` blanket fill means the guard cannot be a struct field.

- Constant-buffer packing drift is the highest-probability silent bug. The C++ upload structs (LightData 28 float4, PaddedMaterial 5, PixelDot3Data 5) are memcpy'd into a register file whose HLSL counterpart reproduces the DX9 per-register padding only because every float3 happens to be followed by a float4. Any future edit to those structs or the .inc breaks the correspondence with no compile error and reproduces exactly the documented black-character class of bug. The reflection ABI guard is the only defence and must never be bypassed.

- SM4 does not clamp COLOR0/COLOR1 interpolators where SM2 clamped the output registers. 84 live vertex shaders write them and several rely on the clamp. This will not fail to compile — it blows out highlights in bright scenes only, and the numerical-equivalence harness evaluating shaders as pure functions is structurally blind to it. Gate B' (stage-boundary triangles with out-of-range vertex values) is the only test that catches it.

- Enabling gamma in the composite pass makes a currently-inert setting effective. `SetGammaRamp` does nothing on a windowed D3D9 swap chain, and the client ships windowed=1, so every player who has ever moved the 0.5-1.5 brightness/contrast/gamma sliders has a persisted non-default value that does nothing today and will change the image under DX11 — a regression visible to a subset of users only, which is the hardest kind to reproduce.

- The FFP-tier assets are unreachable but not unread. Deleting ShaderImplementationPassStage on the strength of the "zero implementations selected at 2.0 contain STAG" measurement would break IFF parsing of five shipped pre-SCAP effects including defaulteffect.eft, the engine's hard fallback for every failed shader fetch. Selection and parsing are different operations and conflating them faults the asset loader.

- font.eft, defaulteffect.eft and a_skin.eft are already unsupported at capability 2.0 under the shipped DX9 build (verified: checkOldVersionForSupport returns before creating graphics data for non-vertex-shader implementations at capability >= 1.1). Authoring 2.0 implementations for them would make content render that currently falls back — a visual change in the opposite direction from what the gates look for. Preserving current behaviour is correct but means the port ships with its fallback shader unresolvable, and the diagnostic is a PRODUCTION NOP.

- Tool viability is coupled to the DX9 deletion in both directions. ShaderBuilder is the only tool that validates .vsh and writes .psh, links d3dx9, and includes three Direct3d9 private headers by relative path — so the corpus conversion depends on a tool the deletion breaks. Direct3d11_WindowPresenter is the other side: nine presentToWindow consumers black out silently (present's bool return is unchecked at every call site) if it slips past the DX9 deletion.

- The shipped TRE stack resolution order is counter-intuitive and the doc comment is wrong. TreeFile::addSearchNode inserts with lower_bound against a descending comparator so last-added wins at equal priority, and install() adds searchTOC after searchTree — so sku0_client.toc at priority 0 beats all 64 priority-0 .tre files. A new shader archive mounted at priority 0 would be silently shadowed and the client would keep loading the old sources while every test appeared to pass.

- No shipping instrumentation exists to detect any of the above post-merge. Direct3d9_Metrics is entirely #ifdef _DEBUG, its only report caller is inside a #if 0 at Game.cpp:1117, every NP_PROFILER_* macro is a NOP at PRODUCTION==1, and there are zero CreateQuery calls in the first-party tree. Until Phase 0 lands, "no performance degradation" is not a testable claim.

- No memory-footprint gate existed in any lens's design. Dropping D3DPOOL_MANAGED removes the runtime's sysmem shadow and demand eviction, declined formats force size-changing fallbacks, and the plan adds a full-resolution scene target plus a resolved twin, a 16 MB vertex ring, a 4 MB index ring, a constant ring and per-texture staging. A VRAM regression manifests as intermittent stutter under pressure on lower-memory GPUs that no fixed-route percentile gate reproduces.

- CPU costs the port does not touch will be blamed on it. CPU skinning of every skeletal character, the per-frame terrain VB/IB rebuild, the per-frame PrimitiveNode delete/new with a shader fetch, and RenderWorldCommander's per-visible-object red-black-tree insert are all substantial and all API-independent. The attribution discipline (fix and re-baseline on DX9 first, one commit each) is the only thing preventing both misattribution directions.

- The prior gl11_r.dll is a PBR renderer, not a parity reference. Adria-DX11-NOTICE.md documents GGX/Smith/Schlick/Reinhard and a "Phase 41 terrain-reflectivity control" scaling dielectric F0. Its shader-replacement list and b2 constant-buffer convention are useful inventory, but mining its design would pull the port away from the byte-parity baseline it is gated on — and 41 phases implies substantial source somewhere that this plan rewrites from scratch.

## Review objections answered as non-issues

- "Direct3d11_LegacyShaderLibrary cannot rescue font.eft / defaulteffect.eft / a_skin.eft" — correction ACCEPTED, remediation REJECTED. Verified at ShaderImplementation.cpp:747-763 that checkOldVersionForSupport returns without creating graphics data for a non-vertex-shader implementation at capability >= 1.1, so all three are ALREADY unsupported under the shipped DX9 build and the client already falls back. Authoring new 2.0 implementations would make content render that does not render today — a visual change in the opposite direction from the parity gate, and it would promote the scriptable IFF writer from optional to blocking. Decision: preserve current behaviour exactly, remove these three from the LegacyShaderLibrary scope, promote the ShaderEffect.cpp:136/166 warning to a non-PRODUCTION FATAL plus an always-on PRODUCTION counter so the condition is never silent again, and settle it empirically with the Phase-0 validation log.

- "rasterMajor=0 selects a gl00_r.dll that has never existed — delete the branch" — alternative REJECTED, the other option taken. Deleting rasterMajor 0 would leave Gl_api with a single implementation, and the Phase-6 trim of a struct whose layout mismatch is the plan's worst silent-failure mode would then be unverifiable. Headless's source is complete, so adding Headless.vcxproj in Phase 1 is cheap and buys a compile check against two implementations plus a DX-free regression baseline for the measurement harness. rasterMajor itself is retired in Phase 6, which resolves the dead-switch concern from the other direction.

- "Should sRGB be adopted?" — NO, permanently, and recorded as a deliberate decision. All 30,245 shipped .dds use legacy DDS headers with zero DX10 extended headers, so nothing carries colour-space metadata and DX9 sampled raw UNORM and lit in gamma space for 20 years. Choosing *_SRGB would visibly change every texture in the game and be indistinguishable from a port bug, and LKUP lighting tables must never be sRGB-decoded. The composite pass applies the brightness/contrast/gamma curve; the pipeline stays gamma-space.

- "Should a new ShaderCapability tier be added, or the == test relaxed to >=?" — NO to both. ShaderCapability.h:16 DEBUG_FATALs outside a fixed whitelist; 2.0 is the highest tier in any shipped asset; and relaxing to >= is actively wrong because 437 measured implementations carry SCAP [0.2,0.3] ordered BEFORE higher tiers, so >= would select a fixed-function implementation for hundreds of effects. A new tier would require re-emitting SCAP in 280 .eft plus 79 inline-EFCT .sht for zero Phase-1..6 benefit.

- "Should TAG_DX9 be renamed to a DX11 option tag?" — NO. Measured OPTN gating selects an option-gated implementation for 175 of 277 effects, and no shipped OPTN chunk mentions DX8/DX9/DX11 at all — the tag is consulted only for shader-feature gating, so reusing it changes nothing observable. Renaming would require re-tagging 280 .eft plus 79 .sht. Kept as a commented, permanent asset-compatibility mapping rather than an accidental vestige.

- "Should indices widen to 32-bit?" — NO. `typedef unsigned short Index` (Graphics.def:15) is threaded through every Static/DynamicIndexBuffer consumer and the .msh/.apt readers, every batch is under 65536 vertices, and DXGI_FORMAT_R16_UINT halves index bandwidth. Widening would ripple through the asset readers for no benefit.

- "Should Gl_api be trimmed up front, or kept byte-compatible?" — kept byte-compatible through Phase 4, trimmed in Phase 6. Trimming early forces lockstep edits to Graphics.h/.cpp, Gl_dll.def, three DX9 projects and Headless, at exactly the moment when the DX9 backend must remain a working parity reference and Headless is unbuildable. Once gl00 and gl11 both build and DX9 is deleted, the trim is a safe subtraction with a compile check on both sides.

- "Should deferred contexts or command lists be used?" — NO, with a three-condition evidence gate. The engine is a single-threaded loop and ShaderPrimitiveSorter.cpp:725 interleaves prepareToDraw CPU work (skinning, terrain rebuild, particle fill) with GPU submission inside the same per-entry loop, so there is no separable submit phase to parallelise. FinishCommandList/ExecuteCommandList at ~10-50 us per list is pure loss at the likely draw counts. The genuine parallelism win is hoisting prepareToDraw, which is a separate project.

- "Should occlusion queries drive culling?" — NO. DX9 used no queries at all and culling is entirely software dPVS, so there is no parity requirement and no reason to introduce GPU culling in a port mandated to change nothing visually. Queries are used only for GPU timing and pipeline statistics, which is capability DX9 never had.

- "Is the multi-stream VertexBufferVector path dormant enough to stub?" — dormant but implemented anyway. Verified ConfigClientGraphics.cpp:99 defaults disableMultiStreamVertexBuffers to TRUE, so it is off in shipping config and cannot be a parity blocker — but SoftwareBlendSkeletalShaderPrimitive's dot3 path binds a 2-stream vector and the MFC tools can enable it, so stubbing it would create a latent hole. Implemented in Phase 2 at low priority rather than deferred.

- "lockBackBuffer needs a careful staging design" — largely moot, do not over-invest. Its only engine consumer is BinkVideo's fallback path, and Bink is already non-functional on x64: binkw32.dll is 32-bit and BinkDLL.cpp binds by `_Name@N` stdcall decoration. The correct action is to route video through the existing TextureBlit path (shader/video_blit.sht with a TCF_dynamic texture, which the code already prefers) and implement lockBackBuffer as a minimal SceneTarget staging copy for completeness. The 565/5551 branches are dead under DXGI.

- "Feature level 11_1 may be needed for TF_ARGB_4444 (B4G4R4A4_UNORM)" — 11_0 floor stands. No shipped .dds uses A4R4G4B4 (census: DXT5 21,547 / DXT1 4,864 / A8R8G8B8 2,388 / DXT3 903 / X8R8G8B8 97 / A1R5G5B5 4 / L8 4), so the format only arises from runtime creation and can fall back to B8G8R8A8_UNORM via the existing TextureFormatInfo::setSupported sweep. 11_1 is requested opportunistically for VSSetConstantBuffers1 offset binding, with a measured-and-reported rotating-buffer fallback on 11_0.

- "Should the FFP/VSPS matrix convention divergence be treated as a black-screen risk?" — NO, it is a non-issue. gl05 (FFP+VSPS, the shipped default) stores D3D-layout matrices and uploads MultiplyTranspose(O2W, W2P); gl07 stores engine-layout and uploads Multiply(W2P, O2W). These are algebraically identical and produce identical constant bytes, so there is one convention and gl05 is the parity reference. Emit the same bytes, keep HLSL default column_major and mul(v, M), and never pass D3DCOMPILE_PACK_MATRIX_ROW_MAJOR.

- "Should Gl_imageFormat keep its D3DXIMAGE_FILEFORMAT ordering?" — the ordering is preserved but the coupling is broken. The enum values are currently cast straight to D3DX values; the DX11 ImageWriter defines an explicit value→WIC-container map so the engine enum stops meaning "a D3DX enum", without renumbering (which would break DllExport.cpp's Graphics::writeImage stub and any caller passing a literal).

- "Should shade mode and dither be handled?" — NO. Measured across all 1,518 decoded v0009/v0010 passes, shade mode is Gouraud in 1,518 and dither is off in 1,518. Neither needs a DX11 counterpart, and `nointerpolation` is never required. Stencil appears in only 7 passes (the inline shadowvolume_* templates), which bounds the depth-stencil state count.

## First actions

1. Archive _client/gl11_r.dll out of the runtime directory (dump its strings to the design notes first) and confirm the shipped client still launches on gl05_r. It is unmanaged by staging and will silently shadow every future DX11 build.

2. Land the GetGlApiStructSize guard in Gl_dll.def/Graphics.cpp and in the DX9 backend, and prove it FATALs on a deliberately mismatched DEBUG_LEVEL pair. This closes the port's worst silent-failure mode before any second implementation exists.

3. Land the four zero-risk subtractions in one commit: delete Gl_api::setDynamicIndexBufferSize (unguarded NULL deref, zero callers), fix the Graphics::lockBackBuffer rect swap at Graphics.cpp:1049-1052, delete Gl_metrics, and add the two missing Direct3d11 friend grants to Texture.h plus one to Transform.h.

4. Remove both header leaks from the shipped libraries — BinkDLL.h's `<d3d9.h>` and the CuiLayer.h→UICanvasGenerator.h→UIDirect3DTextureCanvas.h→ddraw.h chain — then drop directx9\include from clientGraphics, clientUserInterface and swgClientUserInterface and verify by renaming the vendored ddraw.h away.

5. Build the always-on GfxStats counter block, the telemetry ring, the QPC zone timers and DX9 GPU timestamp queries, and re-enable DebugFlags::callReportRoutines() at Game.cpp:1117. Nothing else can be gated until this exists.

6. Commit the TRE resolver tool implementing the real descending/last-added-wins order, extract and version-control vertex_program/** (which exists in no source tree), reconcile the 21 ILM_visuals program deltas, and freeze the 152-entry permutation manifest from a committed enumerator.

7. Author the six benchmark routes and capture the DX9 baseline (36 runs), including peak VRAM and private bytes, plus the per-draw reference capture that becomes the Gate C oracle. Log which effects fail ShaderEffect validation today to settle the font.eft/defaulteffect.eft/a_skin.eft question with data.

8. Run the three outstanding censuses before any shader is converted: zCompare and both stencil-compare fields through the swapped Compare table, and the blend factors of every pass with alphaBlendEnable==false (to derive the alpha-fade blend variant set from data rather than guessing).
