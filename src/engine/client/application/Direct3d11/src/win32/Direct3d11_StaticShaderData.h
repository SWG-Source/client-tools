// ======================================================================
//
// Direct3d11_StaticShaderData.h
// copyright (c) 2026 Galaxies Reborn
//
// One material: the textures, samplers and constants a shader template resolves to, per
// pass.
//
// The division of labour between this and Direct3d11_ShaderImplementationData follows
// DX9's, because the engine's data is divided that way: an implementation owns what the
// EFFECT says (blend, depth, stencil operations, which programs), and this owns what the
// MATERIAL says (which textures, how they are filtered, the material colours, the texture
// factors, the reference values). One implementation is shared by every material that uses
// the effect.
//
// ----------------------------------------------------------------------
// Where the D3D11 shape differs
//
// Sampler state. DX9 pushes seven individual sampler states per stage per draw. All seven
// describe one immutable D3D11_SAMPLER_DESC, so each stage resolves to a single
// ID3D11SamplerState at construction, shared through Direct3d11_StateObjectCache. Per draw
// that is one bind instead of seven.
//
// Texture slots. The engine addresses a texture by its D3D9 sampler stage, and in D3D11
// the texture register is NOT the sampler register: fxc packs texture registers densely
// across the samplers a program actually samples, so a program declaring s0..s3 but
// sampling only s0, s2 and s3 reads them as t0, t1 and t2. The translation comes from the
// pixel program's reflection, which is why a stage binds through the program rather than
// straight to its own index. Assuming the identity would put much of the corpus's textures
// on the wrong samplers, and nothing would say so.
//
// Textures are held as a pointer to the engine's own pointer, exactly as DX9 does. That is
// what lets a texture reload, or a global texture being re-pointed, be picked up without
// rebuilding every material that references it.
//
// Constants keep DX9's upload shape -- register index plus a row count, over a CPU shadow
// that Direct3d11_ConstantBuffers flushes once per draw. The one exception is the
// material's specular power, which shares a register with the dot3 light direction in the
// engine's pixel layout and therefore has to be written as a single component.
//
// ----------------------------------------------------------------------
// Three things this cannot do yet, each recorded rather than dropped
//
//   The alpha test reference. Resolved here, because it is material data, but D3D11 has no
//   alpha test to give it to. It belongs in the b1 epilogue with the compare function the
//   pass carries.
//
//   The fog colour. DX9 sets D3DRS_FOGCOLOR per pass from the pass's fog mode. D3D11 has
//   no fog state either, so the colour belongs in the same epilogue.
//
//   Full ambient. DX9 tells its light manager whether this material carries precalculated
//   vertex lighting. There is no light manager here yet.
//
// All three are resolved and stored, so the epilogue and the light manager have something
// to read when they arrive, and each is reported once so the gap is visible.
//
// ======================================================================

#ifndef INCLUDED_Direct3d11_StaticShaderData_H
#define INCLUDED_Direct3d11_StaticShaderData_H

// ======================================================================

#include "clientGraphics/ShaderImplementation.h"
#include "clientGraphics/StaticShader.h"

#include "sharedMath/VectorRgba.h"

#include <d3d11_1.h>

class Direct3d11_TextureData;

// ======================================================================

class Direct3d11_StaticShaderData : public StaticShaderGraphicsData
{
public:
	static constexpr int MAX_TEXTURE_COORDINATE_SETS = 8;

	static void install();
	static void remove();

	static int getLiveInstanceCount();

public:
	explicit Direct3d11_StaticShaderData(StaticShader const &shader);
	virtual ~Direct3d11_StaticShaderData();

	virtual void update(StaticShader const &shader);
	virtual int getTextureSortKey() const;

	bool isValid() const;

	// Bind everything this material contributes to one pass, and the pass state the
	// implementation owns along with it. Returns whether a vertex shader was bound, which
	// is the answer DX9's setStaticShader passes back to the engine.
	bool apply(int passNumber) const;

private:
	Direct3d11_StaticShaderData();
	Direct3d11_StaticShaderData(Direct3d11_StaticShaderData const &);
	Direct3d11_StaticShaderData &operator=(Direct3d11_StaticShaderData const &);

	void construct(StaticShader const &shader);
	void destroy();

private:
	struct Stage
	{
		// The engine's pointer, not the texture: a reload replaces what it points at.
		Direct3d11_TextureData const *const *texture;
		ID3D11SamplerState *sampler;

		// The D3D9 sampler stage this stage occupies. The texture register is looked up
		// from it through the pixel program's reflection at apply time.
		int samplerSlot;

		bool placeholder;
	};

	struct Pass
	{
		ShaderImplementationPassVertexShader const *vertexShader;
		ShaderImplementationPassPixelShaderProgram const *pixelShaderProgram;

		// Which vertex buffer texture coordinate set each of the vertex program's tags
		// reads. DX9 compiled this into the shader as a key; here it goes to the input
		// layout instead.
		int textureCoordinateSetMapping[MAX_TEXTURE_COORDINATE_SETS];
		int textureCoordinateSetMappingCount;

		// c11..c15, in the byte layout D3D9's D3DMATERIAL9 had, padded to five whole rows.
		float material[20];
		bool materialValid;

		// c44 and c45 for the vertex side, and the engine's pixel textureFactor registers.
		VectorRgba textureFactor[2];
		bool textureFactorValid;

		// Scroll rates, turned into a scrolled offset at apply time from the current time.
		float textureScroll[4];
		bool textureScrollValid;

		uint8 alphaTestReference;
		bool alphaTestReferenceValid;

		uint32 stencilReference;
		bool stencilReferenceValid;

		int fogMode;
		bool fullAmbient;

		Stage *stages;
		int stageCount;
	};

	ShaderImplementation const *m_implementation;
	Pass *m_passes;
	int m_passCount;
};

// ======================================================================

inline bool Direct3d11_StaticShaderData::isValid() const
{
	return m_implementation != NULL;
}

// ======================================================================

#endif
