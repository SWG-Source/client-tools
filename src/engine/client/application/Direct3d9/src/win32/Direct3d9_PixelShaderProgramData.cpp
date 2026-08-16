// ======================================================================
//
// Direct3d9_PixelShaderProgramData.cpp
// Copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "FirstDirect3d9.h"
#include "Direct3d9_PixelShaderProgramData.h"

#ifdef VSPS

#include "Direct3d9.h"
#include "ConfigDirect3d9.h"
#include "clientGraphics/ShaderCapability.h"

// ======================================================================

Direct3d9_PixelShaderProgramData::Direct3d9_PixelShaderProgramData(ShaderImplementation::Pass::PixelShader::Program const & pixelShaderProgram)
: ShaderImplementationPassPixelShaderProgramGraphicsData(),
	m_pixelShader(NULL)
{
#ifdef _DEBUG
	if (ConfigDirect3d9::getCreateShaders())
#endif
	{
		if (Direct3d9::supportsPixelShaders())
		{
#ifdef _DEBUG
			int const pixelShaderVersionRequested = ShaderCapability(pixelShaderProgram.getVersionMajor(), pixelShaderProgram.getVersionMinor());
			DEBUG_FATAL(pixelShaderVersionRequested > Direct3d9::getShaderCapability(), ("%s is compiled for %d.%d but we only support %d.%d", pixelShaderProgram.getFileName(), GetShaderCapabilityMajor(pixelShaderVersionRequested), GetShaderCapabilityMinor(pixelShaderVersionRequested), GetShaderCapabilityMajor(Direct3d9::getShaderCapability()), GetShaderCapabilityMinor(Direct3d9::getShaderCapability())));
#endif
			HRESULT const hresult = Direct3d9::getDevice()->CreatePixelShader(pixelShaderProgram.m_exe, &m_pixelShader);
			FATAL_DX_HR("CreatePixelShader failed %s", hresult);
		}
	}
}

// ----------------------------------------------------------------------

Direct3d9_PixelShaderProgramData::~Direct3d9_PixelShaderProgramData()
{
	if (m_pixelShader)
		m_pixelShader->Release();
}

// ======================================================================

#endif
