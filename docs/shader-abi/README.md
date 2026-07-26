# Shader constant ABI probe

The DX11 port rests on one property of the HLSL compiler: with
`D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY` (`/Gec`), a constant declared
`register(cN)` is placed in the auto-generated `$Globals` constant buffer at byte
offset exactly `16 * N` -- and keeps that offset even when the shader never reads
it.

That property is what lets twenty years of shader assets keep their constant
layout with nothing renumbered, and it is why the engine's existing
register-indexed constant setters keep working unchanged. It is a property of a
compiler flag rather than a language guarantee, so it is worth being able to
re-establish on demand rather than believing.

`register-abi-probe.hlsl` is the smallest shader that exercises it: constants at
scattered registers of which only some are referenced, and four samplers of which
one is declared and never sampled. Compile it with the Windows SDK's fxc:

    fxc /T vs_4_0 /Gec /O3 /Fc probe.asm register-abi-probe.hlsl

`register-abi-probe.expected.txt` is what that produced on the Windows SDK
10.0.26100.0 compiler. Two things in it matter.

Every constant sits at `16 * N`, including the ones marked `[unused]`:

    objectWorldCameraProjectionMatrix  c0   offset    0
    objectWorldMatrix                  c4   offset   64   [unused]
    cameraPosition                     c8   offset  128   [unused]
    viewportData                       c9   offset  144   [unused]
    fog                               c10   offset  160
    textureFactor                     c44   offset  704   [unused]
    currentTime                       c48   offset  768   [unused]
    literalConstants                  c95   offset 1520

`c95` at 1520 plus its own 16 bytes is 1536, which is 96 rows -- the value
`VSCR_CBUFFER_ROWS` derives in clientGraphics/ShaderConstantRegisters.h.

And textures do NOT share numbering with samplers. `skipMe` at `s1` is declared
but never sampled, so it is eliminated, and the surviving textures pack densely:

    mainSampler    s0 -> t0
    detailSampler  s2 -> t1     not t2
    thirdSampler   s3 -> t2     not t3

Binding textures on the assumption that `t == s` therefore puts them on the wrong
samplers with no error from anything. The backend reads the pairing back through
reflection instead; see Direct3d11_ShaderReflection.cpp.
