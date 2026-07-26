# asm2hlsl — Direct3D 9 shader assembly to HLSL

`D3DCompile` has no assembler, and Direct3D 9 shader assembly is not a Direct3D 11 shader
model. The shipped SWG shader corpus is roughly two thirds HLSL and one third assembly, so
the assembly half has to become HLSL before a D3D11 backend can render what DX9 renders.

This converts it, offline. The output is loose `.vsh` and `.psh` files in `client-assets`,
which override the TRE copies through the same search-path mechanism the project already
uses for its custom textures and shaders. After conversion the DX11 backend never sees an
assembly program, which is the point: no DX9 shader artefacts left in the running client.

## Pipeline

```
python shadercorpus.py     # resolve and extract the corpus from the TRE stack
python classify.py         # split programs by declared language
python reachable2.py       # which programs a live implementation can reach
python asm2hlsl_run.py     # convert the reachable assembly
python verifyconv.py       # compile every conversion with fxc
```

`shadercorpus.py` needs `E:\SWG\64bit-server\_client` (the deployed client: 209 TREs plus
the five `.cfg` files `client.cfg` includes). Everything downstream works from its output.

## Search-tree precedence, which is easy to get backwards

`TreeFile::addSearchNode` keeps its list sorted by priority descending and inserts with
`std::lower_bound` against an `a->priority > b->priority` comparator. `lower_bound` returns
the first position whose priority is *not greater*, so an equal-priority node is inserted
**before** the ones already there: **the last tree added at a given priority is searched
first.** The doc comment above that function says the opposite, and it matters, because all
72 trees in `client.cfg` sit at priority 0.

Resolving backwards picks a 601-byte `shared_program/functions.inc` from a base tree instead
of the 2,398-byte copy in `patch_11_00.tre` that actually defines `intensity()` and
`tex2DDxt5CompressedNormal()`, and an older `vertex_shader_constants.inc` whose `Dot3Light`
has its first two fields transposed relative to the C++ struct the engine uploads.

## What the conversion preserves

**Module structure.** A converted module is a block of HLSL statements that the converted
program `#include`s inside `main()`, exactly as the assembly had it. The shared lighting code
is translated once, the output diffs against the input, and the inherited `c_ambient.inc`
patch stays a runtime concern rather than being baked into 23 generated files.

**The constant ABI.** Constants resolve into a flat `float4 c[96] : register(c0)`. `fxc`
places that at `$Globals` offset 0 with size 1536, so element *i* sits at byte 16*i* —
byte-identical to the register file the engine already uploads. `registers.inc` becomes
`#define`s onto `c[N]`, so module bodies keep the symbolic names they were written with.

**The texture-coordinate indirection.** The engine rewrites the tag-to-set mapping per vertex
buffer, so converted programs keep the `#define` header the runtime parses, use the HLSL-form
`textureCoordinateSetTAG` spelling, and reach the sets through the input struct instance the
way the hand-written HLSL programs do.

**Whole-register constant reads.** Assembly reads a whole constant register:
`mul r1, r1, c[dot3LightSpecularColor]` multiplies all four channels. The engine-layout
`pixel_shader_constants.inc` defines those same names as partial swizzles of packed
registers, which is correct for the HLSL programs written against it and wrong here, so the
converted pixel programs get their own `asm_constants.inc` binding each name to a whole
register.

## Decisions worth knowing about

**`c95`.** `registers.inc` maps `c0_0`, `c0_5`, `c1_0` and `cLog2e` onto `c95.x/y/z/w`, which
a D3DX `#pragma def` preloaded. Only assembly ever read them, and the D3D11 path has no
equivalent directive, so they are emitted as literals rather than depending on an upload.

**`cExtLtData_parallelSpec_0_*`.** `ILM_visuals.tre` replaced
`vertex_program/modules/diffuse.inc` with a version referencing three constants that nothing
in any of the 209 TREs defines. DX9 hard-FATALs when assembly fails to compile, so sixteen
reachable vertex programs — fifteen of them base content from `patch_00.tre`, all cell and
interior shaders — cannot be created on DX9 as things stand. They are mapped here onto the
engine's `extendedLightData` block at `c60..c63`, whose `HemisphericLightData` fields are
`backColor`, `tangentColor`, `tangentMinusBackColor` and `tangentMinusDiffuseColor` in that
order, which is an exact name match for what the module wants. That is a deliberate choice to
make installed content work rather than to reproduce a crash.

**Vertex inputs are `float4`.** Assembly treats an input as a whole register and some
programs swizzle all four components of the normal. D3D11 fills components the input layout
does not supply with 0, and `w` with 1, which is what D3D9 did for an unwritten register
component.

**Co-issue markers are dropped.** A `+` pairs an alpha instruction with the preceding colour
one. The two write disjoint masks, so the pairing carries no semantics worth preserving.

## Verification

`verifyconv.py` overlays the conversion on the extracted corpus, applies the same include
overrides the backend applies, and compiles every output with `fxc` at `vs_4_0` / `ps_4_0`
with `/Gec` — the exact flags `Direct3d11_ShaderCompiler` uses.

All 97 reachable assembly programs compile. Together with 512 of 517 HLSL programs (the five
failures are unreachable at shader capability 2.0), the whole reachable corpus builds.

Compiling is not the same as being correct. The real gate is a visual comparison against
`gl05`, which needs the client running.
