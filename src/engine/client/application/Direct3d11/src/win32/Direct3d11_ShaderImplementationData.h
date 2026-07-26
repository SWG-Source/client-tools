// ======================================================================
//
// Direct3d11_ShaderImplementationData.h
// copyright (c) 2026 Galaxies Reborn
//
// One shader implementation: its passes, each resolved to the D3D11 state objects it
// needs.
//
// ----------------------------------------------------------------------
// Resolved once, not replayed per draw
//
// DX9 records each pass as a flat vector of (D3DRENDERSTATETYPE, DWORD) pairs and replays
// it a state at a time through its state cache on every apply
// (Direct3d9_ShaderImplementationData.cpp:341-355). It has to: D3D9 render states are
// individually settable.
//
// D3D11 has no individually settable render states. Blending, depth and stencil are
// immutable state objects, created up front and bound whole. So a pass here resolves its
// engine state into exactly two objects when it is constructed, and applying it is two
// binds instead of up to twenty-nine state calls. Both objects come from
// Direct3d11_StateObjectCache, so passes with identical state share them.
//
// This is also why Direct3d11_Metrics counts state object creations and requires them to be
// zero inside a frame: with construction-time resolution, a creation during a frame means a
// pass is being built mid-frame, which is the shape of problem this design exists to avoid.
//
// ----------------------------------------------------------------------
// What a pass does NOT own
//
//   Sampler state. DX9's VSPS path leaves Stage::construct empty
//   (Direct3d9_ShaderImplementationData.cpp:171-173) and its m_stage vector is compiled
//   only under FFP -- filtering and addressing are properties of a material's texture, not
//   of a pass, so they belong to the static shader.
//
//   The stencil reference. DX9 keeps it on the static shader as
//   m_stencilReferenceValue and sets it during that apply
//   (Direct3d9_StaticShaderData.cpp:932). In D3D11 the reference is an argument to
//   OMSetDepthStencilState rather than a state of its own, so this class hands out the
//   object and the caller supplies the reference. That is why apply takes one.
//
//   The fixed-function states DX9 records -- LIGHTING, COLORVERTEX, SPECULARENABLE and the
//   four MATERIALSOURCE states. They exist for gl06's fixed-function pipeline. This backend
//   is vertex-and-pixel-shader only, so they have no meaning here and are not carried.
//
//   The vertex shader. It is bound where the texture coordinate mapping is known, which is
//   the static shader.
//
// ----------------------------------------------------------------------
// Two states D3D11 cannot express, handled honestly
//
//   Alpha test. D3D9's ALPHATESTENABLE/ALPHAFUNC/ALPHAREF have no D3D11 equivalent at all;
//   the test has to become a discard in the pixel shader. The function is per-pass and is
//   recorded here, but the reference is dynamic -- DX9 computes it per object from
//   ms_alphaTestReferenceValue scaled by the alpha fade opacity
//   (Direct3d9.cpp:3939) -- so it belongs in a constant buffer rather than in a shader
//   variant. That is what constant buffer slot b1 was reserved for. Until the epilogue
//   exists, a pass that enables alpha test says so once rather than silently drawing
//   pixels that should have been discarded.
//
//   Flat shading. D3D9's SHADEMODE has no state equivalent; flat interpolation is a
//   `nointerpolation` modifier on the shader output, which means it is a property of the
//   compiled program. Reported once if any pass asks for it.
//
// Dithering is deliberately dropped rather than reported. D3D9's DITHERENABLE only ever
// affected output below 8 bits per channel, and every render target in this backend is
// 8 bits per channel or better, so it had no effect to reproduce.
//
// ======================================================================

#ifndef INCLUDED_Direct3d11_ShaderImplementationData_H
#define INCLUDED_Direct3d11_ShaderImplementationData_H

// ======================================================================

#include "clientGraphics/ShaderImplementation.h"

#include <d3d11_1.h>

// ======================================================================

class Direct3d11_ShaderImplementationData : public ShaderImplementationGraphicsData
{
public:

	static void install();
	static void remove();

	static int  getLiveInstanceCount();

public:

	explicit Direct3d11_ShaderImplementationData(ShaderImplementation const &implementation);
	virtual ~Direct3d11_ShaderImplementationData();

	int  getPassCount() const;

	// Bind the blend state, the depth-stencil state and the pixel shader for one pass.
	// The stencil reference belongs to the material, so it is passed in.
	void apply(int passNumber, uint32 stencilReference) const;

	// Whether this pass blends, which the sorter wants to know without applying anything.
	bool isAlphaBlendEnabled(int passNumber) const;

private:

	Direct3d11_ShaderImplementationData();
	Direct3d11_ShaderImplementationData(Direct3d11_ShaderImplementationData const &);
	Direct3d11_ShaderImplementationData &operator =(Direct3d11_ShaderImplementationData const &);

private:

	struct Pass
	{
		ID3D11BlendState        *blendState;
		ID3D11DepthStencilState *depthStencilState;

		// The program rather than the resolved shader, so that a development-time reload of
		// a pixel program is picked up without rebuilding every implementation that uses
		// it. It costs one indirection per pass per draw. DX9 makes the opposite trade under
		// PRODUCTION and keeps the program pointer otherwise; the indirection is worth more
		// than it costs.
		ShaderImplementationPassPixelShaderProgram const *pixelShaderProgram;

		bool                     alphaBlendEnable;
		bool                     alphaTestEnable;
		int                      alphaTestFunction;
	};

	Pass *m_passes;
	int   m_passCount;
};

// ======================================================================

inline int Direct3d11_ShaderImplementationData::getPassCount() const
{
	return m_passCount;
}

// ======================================================================

#endif
