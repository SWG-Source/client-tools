// ======================================================================
//
// Direct3d11_ShaderSignature.h
// copyright (c) 2026 Galaxies Reborn
//
// Makes every vertex program and every pixel program agree on one interpolant layout, so that
// D3D11 will link them.
//
// ----------------------------------------------------------------------
// The problem, stated precisely
//
// D3D9 linked the vertex and pixel stages by SEMANTIC NAME. A pixel shader reading TEXCOORD2 was
// wired to whatever the vertex shader wrote to TEXCOORD2, and neither had to know anything about
// the other. The whole corpus is authored on that assumption and is internally consistent.
//
// D3D11 links by REGISTER. The pixel input signature must be register-compatible with the vertex
// output signature -- matching slots, and component masks that are subsets. Semantic names are how
// the runtime checks the pairing, not how the hardware wires it.
//
// This backend compiles the two stages independently, because that is how the engine stores them:
// a .vsh and a .psh are separate assets, paired by an effect pass at load time. fxc assigns
// registers by packing each signature densely in that program's own declaration order. So the same
// semantic lands wherever its position in that particular program implies, and two independently
// compiled programs disagree.
//
// The result is not a wrong image. It is NO image: every draw is rejected with
// "Signatures between stages are incompatible", the draw counters still count the calls that were
// issued, and the render target stays black. That is exactly what the first working run of this
// client did -- 134,770 draw calls, 2.3 million triangles, nothing on screen.
//
// ----------------------------------------------------------------------
// Why narrowing is not enough, measured
//
// The obvious fix -- declare the same semantics in the same order on both sides -- is not
// sufficient, because fxc PACKS NARROW INTERPOLANTS TOGETHER. Compiled and checked:
//
//   a vertex shader writing  float fog : FOG; float4 tc0 : TEXCOORD0;
//     puts FOG in register 3 mask x, TEXCOORD0 in register 4 mask xyzw
//
//   a pixel shader reading   float fog : FOG; float2 tc0 : TEXCOORD0;
//     puts FOG in register 3 mask x, TEXCOORD0 in register 3 mask yz -- the SAME register
//
// So the widths decide the register assignment as much as the order does, and the corpus uses
// every width: TEXCOORD0 alone appears as float2 in 173 vertex programs, float4 in 33 and float3
// in 11. Canonicalising order without canonicalising width would fix some pairs and not others.
//
// ----------------------------------------------------------------------
// What this does instead
//
// One struct, twelve registers, every one a float4 so that each occupies a whole register and
// packing is one-to-one with declaration order:
//
//   register 0   SV_Position   (from the corpus's POSITION0)
//   register 1   COLOR0
//   register 2   COLOR1
//   register 3   FOG
//   registers 4-11  TEXCOORD0 through TEXCOORD7
//
// Twelve is the exact union of what the corpus uses on either side of the boundary, measured over
// all 228 vertex and 386 pixel programs -- not a guess with room to spare. Nothing reads a semantic
// that nothing writes. The limit for a shader model 4 stage is 32, so there is no risk of running
// out, and unused slots cost interpolation bandwidth rather than correctness.
//
// Neither program is rewritten internally. Each keeps its own entry point, renamed, and gets a
// generated wrapper that becomes the new entry point:
//
//   a vertex program's wrapper calls the original, then widens each member it declared into the
//   matching canonical slot
//
//   a pixel program's wrapper narrows each canonical slot back to the type and name the original
//   entry point expects, and calls it
//
// Because both wrappers use the same struct DECLARATION, both signatures pack identically, and the
// registers agree by construction rather than by coincidence.
//
// ----------------------------------------------------------------------
// Two things the wrapper subsumes
//
// The pixel wrapper is also where the epilogue goes, which is simpler than what it replaces. The
// alpha test and fog blend used to be injected by rewriting every `return` inside the original
// entry point and adding a FOG parameter to its signature. Now the wrapper does it once, at the
// single point where the original's result becomes the shader's result, and the fog factor comes
// out of the canonical struct instead of a parameter that had to be threaded in.
//
// The vertex wrapper subsumes the fog OUTPUT injection for the same reason: the canonical struct
// always has a FOG slot, so a program that never wrote fog no longer needs one added to its own
// struct. Such a program gets 1.0, which is the factor's no-fog value -- NOT the zero the struct
// initialiser would otherwise leave, which means fully fogged.
//
// ======================================================================

#ifndef INCLUDED_Direct3d11_ShaderSignature_H
#define INCLUDED_Direct3d11_ShaderSignature_H

// ======================================================================

class Direct3d11_ShaderSignature
{
public:

	// The shared declaration, emitted into both stages verbatim. Identical text is what makes the
	// packing identical.
	static char const *getInterpolantDeclaration();
	static int         getInterpolantDeclarationLength();

	// Wrap a program's entry point so its signature is the canonical one. Returns newly allocated
	// source for the caller to delete, or null with the reason reported.
	//
	// applyEpilogue only applies to the pixel side, and exists so the alpha test and fog blend land
	// in the wrapper rather than being injected into the original body.
	static char *wrapVertexProgram(char const *name, char const *source, int length, int &resultLength);
	static char *wrapPixelProgram(char const *name, char const *source, int length, int &resultLength);

private:

	Direct3d11_ShaderSignature();
	Direct3d11_ShaderSignature(Direct3d11_ShaderSignature const &);
	Direct3d11_ShaderSignature &operator =(Direct3d11_ShaderSignature const &);
};

// ======================================================================

#endif
