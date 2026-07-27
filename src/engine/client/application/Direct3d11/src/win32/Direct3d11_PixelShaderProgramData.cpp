// ======================================================================
//
// Direct3d11_PixelShaderProgramData.cpp
// copyright (c) 2026 Galaxies Reborn
//
// ======================================================================

#include "FirstDirect3d11.h"
#include "Direct3d11_PixelShaderProgramData.h"

#include "Direct3d11_Device.h"
#include "Direct3d11_Metrics.h"
#include "Direct3d11_ShaderCompiler.h"
#include "Direct3d11_ShaderReflection.h"
#include "Direct3d11_ShaderPrograms.h"
#include "Direct3d11_ShaderSource.h"

#include <vector>

// ======================================================================

namespace Direct3d11_PixelShaderProgramDataNamespace
{
	int  ms_liveInstanceCount;
	bool ms_installed;
}
using namespace Direct3d11_PixelShaderProgramDataNamespace;

// ======================================================================

void Direct3d11_PixelShaderProgramData::install()
{
	DEBUG_FATAL(ms_installed, ("Direct3d11_PixelShaderProgramData::install called twice"));
	ms_installed = true;
}

// ----------------------------------------------------------------------

void Direct3d11_PixelShaderProgramData::remove()
{
	DEBUG_WARNING(ms_liveInstanceCount != 0, ("Direct3d11: %d pixel program(s) are still alive at shutdown.", ms_liveInstanceCount));
	ms_installed = false;
}

// ----------------------------------------------------------------------

int Direct3d11_PixelShaderProgramData::getLiveInstanceCount()
{
	return ms_liveInstanceCount;
}

// ======================================================================

Direct3d11_PixelShaderProgramData::Direct3d11_PixelShaderProgramData(ShaderImplementationPassPixelShaderProgram const &program)
:
	ShaderImplementationPassPixelShaderProgramGraphicsData(),
	m_program(program),
	m_bytecode(NULL),
	m_shader(NULL),
	m_reflection(),
	m_reportedDimensionMismatch(0)
{
	++ms_liveInstanceCount;
	compile();
}

// ----------------------------------------------------------------------

Direct3d11_PixelShaderProgramData::~Direct3d11_PixelShaderProgramData()
{
	--ms_liveInstanceCount;

	if (m_shader)
	{
		m_shader->Release();
		m_shader = NULL;
	}

	if (m_bytecode)
	{
		m_bytecode->Release();
		m_bytecode = NULL;
	}
}

// ======================================================================

void Direct3d11_PixelShaderProgramData::compile()
{
	char const * const name = m_program.getFileName();

	if (!m_program.m_source || m_program.m_sourceLength <= 0)
	{
		// The only other thing a .psh carries is PEXE, which is Direct3D 9 bytecode. There
		// is nothing this backend can do with it.
		WARNING(true, ("Direct3d11: pixel program '%s' has no PSRC source chunk, and its PEXE bytecode is Direct3D 9 and unusable here.", name));
		return;
	}

	// The engine's loader NUL-terminates m_source, and m_sourceLength excludes the
	// terminator, so the length is the text length.
	char const * const source = m_program.m_source;
	int const sourceLength = m_program.m_sourceLength;

	Direct3d11_ShaderPrograms::Macros macros;
	macros.buildForPixelProgram();

	m_bytecode = Direct3d11_ShaderCompiler::compilePixelShader(source, sourceLength, name, macros.get());
	if (!m_bytecode)
		return;

	ID3D11Device1 * const device = Direct3d11_Device::getDevice();
	if (!device)
		return;

	// Reflect before creating the shader object. The reflection is what says which texture
	// register each sampler register pairs with, and the engine cannot bind a texture
	// correctly without it -- fxc packs texture registers densely across the samplers the
	// program actually samples, so the mapping is not the identity.
	IGNORE_RETURN(Direct3d11_ShaderReflection::reflect(m_bytecode, name, false, m_reflection));

	HRESULT const hresult = device->CreatePixelShader(m_bytecode->GetBufferPointer(), m_bytecode->GetBufferSize(), NULL, &m_shader);

	Direct3d11_Device::setDebugName(m_shader, name);
	if (FAILED(hresult) || !m_shader)
	{
		WARNING(true, ("Direct3d11: pixel program '%s' compiled but the shader object could not be created (%s).", name, Direct3d11_Device::describeHresult(hresult)));
		m_shader = NULL;
	}
}

// ======================================================================
