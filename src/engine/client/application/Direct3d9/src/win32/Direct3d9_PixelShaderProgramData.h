// ======================================================================
//
// Direct3d9_PixelShaderProgramData.h
// Copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_Direct3d9_PixelShaderProgramData_H
#define INCLUDED_Direct3d9_PixelShaderProgramData_H

#ifdef VSPS

// ======================================================================

struct IDirect3DPixelShader9;

#include "clientGraphics/ShaderImplementation.h"

// ======================================================================

class Direct3d9_PixelShaderProgramData : public ShaderImplementationPassPixelShaderProgramGraphicsData
{
public:

	Direct3d9_PixelShaderProgramData(ShaderImplementation::Pass::PixelShader::Program const & pixelShaderProgram);
	virtual ~Direct3d9_PixelShaderProgramData();

	IDirect3DPixelShader9 * getPixelShader() const;

private:

	IDirect3DPixelShader9 * m_pixelShader;
};

// ======================================================================

inline IDirect3DPixelShader9 * Direct3d9_PixelShaderProgramData::getPixelShader() const
{
	return m_pixelShader;
}

// ======================================================================

#endif
#endif
