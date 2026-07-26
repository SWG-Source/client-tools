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

private:

	Direct3d11_PixelShaderProgramData();
	Direct3d11_PixelShaderProgramData(Direct3d11_PixelShaderProgramData const &);
	Direct3d11_PixelShaderProgramData &operator =(Direct3d11_PixelShaderProgramData const &);

	void compile();

private:

	ShaderImplementationPassPixelShaderProgram const &m_program;

	ID3DBlob          *m_bytecode;
	ID3D11PixelShader *m_shader;
};

// ======================================================================

inline ID3D11PixelShader *Direct3d11_PixelShaderProgramData::getPixelShader() const
{
	return m_shader;
}

// ======================================================================

#endif
