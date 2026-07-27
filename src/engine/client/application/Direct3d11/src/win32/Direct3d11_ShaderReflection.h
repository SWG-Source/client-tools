// ======================================================================
//
// Direct3d11_ShaderReflection.h
// copyright (c) 2026 Galaxies Reborn
//
// What the compiler actually produced, read back and checked against what the
// assets assume.
//
// Two things come out of here, and both are things that cannot safely be assumed.
//
// The constant ABI. Under backwards-compatibility mode a constant declared
// register(cN) lands in $Globals at byte offset 16 * N. The whole port depends on
// that: it is why no shader has to be renumbered and why the engine's existing
// register-based constant setters keep working. But it is a property of a
// compiler flag, not a language guarantee, so it is verified per shader rather
// than trusted. If it ever stops holding, every constant in the corpus is
// silently off by some amount, which produces black characters, wrong fog and
// mis-scrolling textures with nothing in any log.
//
// The sampler-to-texture mapping. In shader model 4 a sampler and a texture are
// separate objects with separate slots, where D3D9 had one combined stage.
// Samplers keep the s# they were declared with, but textures are packed densely
// in declaration order among the samplers that are actually USED -- so with s1
// unused, a sampler declared at s2 pairs with texture t1, not t2. The engine binds
// by stage index, so assuming t == s puts textures on the wrong samplers with no
// error anywhere. The prior attempt hit exactly this and its shipped binary still
// carries the diagnostic strings from working it out.
//
// ======================================================================

#ifndef INCLUDED_Direct3d11_ShaderReflection_H
#define INCLUDED_Direct3d11_ShaderReflection_H

// ======================================================================

#include <d3d11_1.h>

// ======================================================================

class Direct3d11_ShaderReflection
{
public:

	enum
	{
		MAX_SAMPLER_SLOTS = 16
	};

	class Result
	{
	public:

		Result();

		// Bytes of $Globals the shader actually declares. An upload bounded by
		// this rather than by the whole register file is the difference between
		// per-draw constant traffic that matches DX9 and traffic that is an order
		// of magnitude worse.
		int   constantExtentBytes;

		// For each sampler slot, the texture slot it pairs with, or -1.
		int   samplerToTexture[MAX_SAMPLER_SLOTS];

		// For each TEXTURE slot, the resource dimension the compiled program declared for it,
		// or D3D_SRV_DIMENSION_UNKNOWN where the slot carries nothing.
		//
		// This is the one property of a sampler that a converted ps_1_x program cannot recover
		// from its source: D3D9 assembly declared no dimension before ps_2_0, so tex t1 sampled
		// whatever kind of texture was bound. HLSL has to commit -- Texture2D or TextureCube --
		// and D3D11 refuses the read when the view disagrees with the declaration. Recording it
		// here is what lets the bind site say which program and which slot.
		D3D_SRV_DIMENSION textureDimension[MAX_SAMPLER_SLOTS];

		int   samplerCount;
		int   textureCount;

		// How many named constants the ABI guard recognised and checked. Zero
		// means the guard verified nothing at all, which is worse than a failure
		// because it looks like a pass.
		int   checkedConstantCount;
	};

public:

	// Reflects the blob, fills the result, and FATALs on an ABI violation.
	// isVertexShader selects which register-name table to check against.
	static bool  reflect(ID3DBlob *bytecode, char const *name, bool isVertexShader, Result &result);

private:

	Direct3d11_ShaderReflection();
	Direct3d11_ShaderReflection(Direct3d11_ShaderReflection const &);
	Direct3d11_ShaderReflection &operator =(Direct3d11_ShaderReflection const &);
};

// ======================================================================

#endif
