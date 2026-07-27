// ======================================================================
//
// Direct3d11_PixelShaderProgramData.h
// copyright (c) 2026 Galaxies Reborn
//
// One pixel program: compiled from its embedded HLSL source, once.
//
// A .psh carries both the source (PSRC chunk) and precompiled Direct3D 9 bytecode (PEXE).
// This backend can only use the source: PEXE holds ps_1_x or ps_2_0 tokens, which D3D11
// cannot consume in any form. That removes the fallback DX9 relies on -- when a source
// compile fails, DX9 quietly uses PEXE instead (Direct3d9_PixelShaderProgramData.cpp:316)
// -- so a failure here is visible where DX9 hid it. That is the right way round, and it is
// how the include-ordering defect in the shipped functions.inc was found at all.
//
// Recompiling from source rather than using PEXE is also what the x64 DX9 build does, and
// for a documented reason: the PEXE blobs in the TRE set are stale relative to the
// runtime-recompiled vertex shaders, which rendered characters dark. Both backends now
// agree on compiling from source.
//
// There is no per-material specialisation. Pixel programs have no equivalent of the vertex
// texture-coordinate key -- their inputs are whatever the vertex program wrote -- so one
// compiled program serves every material that references it.
//
// Assembly is refused by name, as it is for vertex programs. scripts/asm2hlsl converts
// those offline and client-assets ships the results; one arriving here as assembly means
// the converted overlay is not on the search path.
//
// ======================================================================

#ifndef INCLUDED_Direct3d11_PixelShaderProgramData_H
#define INCLUDED_Direct3d11_PixelShaderProgramData_H

// ======================================================================

#include "Direct3d11_ShaderReflection.h"

#include "clientGraphics/ShaderImplementation.h"

#include <d3d11_1.h>

// ======================================================================

class Direct3d11_PixelShaderProgramData : public ShaderImplementationPassPixelShaderProgramGraphicsData
{
public:

	static void install();
	static void remove();

	static int  getLiveInstanceCount();

public:

	explicit Direct3d11_PixelShaderProgramData(ShaderImplementationPassPixelShaderProgram const &program);
	virtual ~Direct3d11_PixelShaderProgramData();

	// Null when the program could not be built. The draw path drops and counts the draw
	// rather than failing at startup, which keeps the failure attached to the material that
	// needs it.
	ID3D11PixelShader *getPixelShader() const;

	// Which texture register a given sampler register pairs with, or -1 when the shader
	// declares that sampler but never samples through it.
	//
	// This is not the identity, and assuming it were would bind textures to the wrong
	// samplers silently. fxc packs texture registers densely across the samplers a shader
	// actually USES: a program declaring s0, s1, s2 and s3 but only sampling s0, s2 and s3
	// gets t0, t1 and t2, so s2 reads t1. The engine addresses its textures by D3D9 sampler
	// stage, so the translation has to come from reflection.
	int  getTextureSlotForSampler(int samplerSlot) const;

	// The resource dimension this program declared for a texture slot. The engine's texture
	// knows its own kind, so a disagreement is a free comparison at bind time rather than a
	// debug layer error at draw time -- and this one names the program.
	D3D_SRV_DIMENSION getTextureDimensionForSlot(int textureSlot) const;

	// One report per slot, not one per draw: the same material redraws every frame.
	bool  hasReportedDimensionMismatch(int textureSlot) const;
	void  noteReportedDimensionMismatch(int textureSlot) const;

	// Bytes of $Globals this program declares. Bounding a constant upload by this rather
	// than by the whole register file is the difference between per-draw constant traffic
	// that matches DX9 and traffic an order of magnitude worse.
	int  getConstantExtentBytes() const;

private:

	Direct3d11_PixelShaderProgramData();
	Direct3d11_PixelShaderProgramData(Direct3d11_PixelShaderProgramData const &);
	Direct3d11_PixelShaderProgramData &operator =(Direct3d11_PixelShaderProgramData const &);

	void compile();

private:

	ShaderImplementationPassPixelShaderProgram const &m_program;

	ID3DBlob          *m_bytecode;
	ID3D11PixelShader *m_shader;

	Direct3d11_ShaderReflection::Result m_reflection;

	// Which texture slots have already had a declared-versus-bound dimension mismatch reported.
	// Mutable because the report happens on a const apply path, and the alternative is either a
	// line per draw or a non-const apply for the sake of a diagnostic.
	mutable unsigned int m_reportedDimensionMismatch;
};

// ======================================================================

inline ID3D11PixelShader *Direct3d11_PixelShaderProgramData::getPixelShader() const
{
	return m_shader;
}

// ----------------------------------------------------------------------

inline int Direct3d11_PixelShaderProgramData::getTextureSlotForSampler(int samplerSlot) const
{
	if (samplerSlot < 0 || samplerSlot >= Direct3d11_ShaderReflection::MAX_SAMPLER_SLOTS)
		return -1;

	return m_reflection.samplerToTexture[samplerSlot];
}

// ----------------------------------------------------------------------

inline D3D_SRV_DIMENSION Direct3d11_PixelShaderProgramData::getTextureDimensionForSlot(int textureSlot) const
{
	if (textureSlot < 0 || textureSlot >= Direct3d11_ShaderReflection::MAX_SAMPLER_SLOTS)
		return D3D_SRV_DIMENSION_UNKNOWN;

	return m_reflection.textureDimension[textureSlot];
}

// ----------------------------------------------------------------------

inline bool Direct3d11_PixelShaderProgramData::hasReportedDimensionMismatch(int textureSlot) const
{
	if (textureSlot < 0 || textureSlot >= Direct3d11_ShaderReflection::MAX_SAMPLER_SLOTS)
		return true;

	return (m_reportedDimensionMismatch & (1u << textureSlot)) != 0;
}

// ----------------------------------------------------------------------

inline void Direct3d11_PixelShaderProgramData::noteReportedDimensionMismatch(int textureSlot) const
{
	if (textureSlot >= 0 && textureSlot < Direct3d11_ShaderReflection::MAX_SAMPLER_SLOTS)
		m_reportedDimensionMismatch |= (1u << textureSlot);
}

// ----------------------------------------------------------------------

inline int Direct3d11_PixelShaderProgramData::getConstantExtentBytes() const
{
	return m_reflection.constantExtentBytes;
}

// ======================================================================

#endif
