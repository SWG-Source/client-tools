// ======================================================================
//
// Direct3d11_VertexShaderData.cpp
// copyright (c) 2026 Galaxies Reborn
//
// ======================================================================

#include "FirstDirect3d11.h"
#include "Direct3d11_VertexShaderData.h"

#include "Direct3d11_Device.h"
#include "Direct3d11_InputLayoutCache.h"
#include "Direct3d11_Metrics.h"
#include "Direct3d11_ShaderCompiler.h"
#include "Direct3d11_ShaderPrograms.h"
#include "Direct3d11_ShaderReflection.h"
#include "Direct3d11_ShaderSource.h"

#include "sharedFoundation/Tag.h"

#include <vector>

// ======================================================================

namespace Direct3d11_VertexShaderDataNamespace
{
	int ms_liveInstanceCount;
	bool ms_installed;
}
using namespace Direct3d11_VertexShaderDataNamespace;

// ======================================================================

void Direct3d11_VertexShaderData::install()
{
	DEBUG_FATAL(ms_installed, ("Direct3d11_VertexShaderData::install called twice"));
	ms_installed = true;
}

// ----------------------------------------------------------------------

void Direct3d11_VertexShaderData::remove()
{
	DEBUG_WARNING(ms_liveInstanceCount != 0, ("Direct3d11: %d vertex program(s) are still alive at shutdown.", ms_liveInstanceCount));

	ms_installed = false;
}

// ----------------------------------------------------------------------

int Direct3d11_VertexShaderData::getLiveInstanceCount()
{
	return ms_liveInstanceCount;
}

// ======================================================================

Direct3d11_VertexShaderData::Direct3d11_VertexShaderData(ShaderImplementationPassVertexShader const &vertexShader)
:
	ShaderImplementationPassVertexShaderGraphicsData(),
	m_vertexShader(vertexShader),
	m_source(NULL),
	m_sourceLength(0),
	m_textureCoordinateSetTags(NULL),
	m_bytecode(NULL),
	m_shader(NULL)
{
	++ms_liveInstanceCount;

	parseHeader();
	compile();
}

// ----------------------------------------------------------------------

Direct3d11_VertexShaderData::~Direct3d11_VertexShaderData()
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
		m_signatureHash = 0;
	}

	delete m_textureCoordinateSetTags;
	m_textureCoordinateSetTags = NULL;
}

// ======================================================================
/**
 * Read the language marker and the leading #define block.
 *
 * The block is where the program declares which texture coordinate set tags it uses, in
 * the order that becomes their canonical TEXCOORD index. The file's own definitions of
 * those macros are then discarded along with the block: the runtime supplies its own,
 * which is what makes one compiled variant serve every material.
 */

void Direct3d11_VertexShaderData::parseHeader()
{
	Direct3d11_ShaderPrograms::Header header;

	if (!Direct3d11_ShaderPrograms::parseHeader(m_vertexShader.m_text, m_vertexShader.m_textLength, header))
	{
		// Names the program, because "which one" is the entire question when this fires. The
		// engine's loader does not check TreeFile::open's result before calling length() on it
		// (ShaderImplementation.cpp's ShaderImplementationPassVertexShader::load), so a program
		// that resolves to nothing arrives here rather than being refused there.
		WARNING(true, ("Direct3d11: vertex program '%s' has no source text (text %p, length %d), so it cannot be compiled.",
			m_vertexShader.getFilename(),
			static_cast<void const *>(m_vertexShader.m_text),
			m_vertexShader.m_textLength));
		return;
	}

	if (header.language != Direct3d11_ShaderPrograms::L_hlsl)
	{
		// Refused loudly rather than half-compiled. scripts/asm2hlsl converts these offline
		// and client-assets carries the results, so an assembly program arriving here means
		// the converted overlay is not deployed.
		WARNING(true, ("Direct3d11: vertex program '%s' is %s. D3DCompile has no assembler; the converted HLSL in client-assets is missing from the search path.",
			m_vertexShader.getFilename(),
			(header.language == Direct3d11_ShaderPrograms::L_assembly) ? "Direct3D 9 assembly" : "not marked as HLSL"));
		return;
	}

	DEBUG_WARNING(header.tooManyTags, ("Direct3d11: vertex program '%s' declares more than %d texture coordinate sets; the engine supports %d.",
		m_vertexShader.getFilename(),
		static_cast<int>(Direct3d11_ShaderPrograms::MAX_TEXTURE_COORDINATE_SETS),
		static_cast<int>(Direct3d11_ShaderPrograms::MAX_TEXTURE_COORDINATE_SETS)));

	// The tags are kept as engine Tags because that is what the input layout cache matches
	// against; the shared parser deals in the four-character codes so that it needs nothing from
	// the engine.
	for (int i = 0; i < header.tagCount; ++i)
	{
		char tagName[5];
		memcpy(tagName, header.tags[i], 4);
		tagName[4] = 0;

		if (!m_textureCoordinateSetTags)
			m_textureCoordinateSetTags = new TextureCoordinateSetTags;

		m_textureCoordinateSetTags->push_back(ConvertStringToTag(tagName));
	}

	m_source = m_vertexShader.m_text + header.sourceOffset;
	m_sourceLength = header.sourceLength;
	m_header = header;
}

// ----------------------------------------------------------------------
/**
 * Compile once, against the canonical tag-to-index mapping.
 *
 * Tag i becomes TEXCOORD i, and its dimension comes from the tag rather than from whichever
 * vertex buffer set a material points it at -- DOT3 is four components, everything else two.
 * That is what makes the declaration, and therefore the compiled program, independent of the
 * material. The input layout carries the difference.
 */

void Direct3d11_VertexShaderData::compile()
{
	if (!m_source)
		return;

	Direct3d11_ShaderPrograms::Macros macros;
	macros.buildForVertexProgram(m_header);

	m_bytecode = Direct3d11_ShaderCompiler::compileVertexShader(m_source, m_sourceLength, m_vertexShader.getFilename(), macros.get());
	if (!m_bytecode)
		return;

	// Once, here. The bytecode is immutable for the life of this object, so its input signature
	// is too, and the draw path can just carry the number.
	m_signatureHash = Direct3d11_InputLayoutCache::hashVertexShaderSignature(m_bytecode->GetBufferPointer(), static_cast<unsigned int>(m_bytecode->GetBufferSize()));

	ID3D11Device1 * const device = Direct3d11_Device::getDevice();
	if (!device)
		return;

	// Reflect the vertex side too, which until now nothing did.
	//
	// Direct3d11_ShaderReflection::reflect had exactly one caller -- the pixel program -- so two
	// checks it performs had never run against a vertex shader at all: the constant ABI guard, which
	// is the thing keeping register(cN) at $Globals offset 16N and which has a vertex name table
	// written specifically for this call, and the interpolant signature guard. The signature one
	// matters most: a vertex shader's OUTPUT signature is one half of what D3D11 links, and a
	// mismatch there is not a compile error, it is a draw the runtime silently refuses.
	//
	// The result is discarded because a vertex program has no sampler mapping to keep; it is the
	// checking that is wanted, not the return value.
	{
		Direct3d11_ShaderReflection::Result reflectionResult;
		IGNORE_RETURN(Direct3d11_ShaderReflection::reflect(m_bytecode, m_vertexShader.getFilename(), true, reflectionResult));
	}

	HRESULT const hresult = device->CreateVertexShader(m_bytecode->GetBufferPointer(), m_bytecode->GetBufferSize(), NULL, &m_shader);

	Direct3d11_Device::setDebugName(m_shader, m_vertexShader.getFilename());
	if (FAILED(hresult) || !m_shader)
	{
		WARNING(true, ("Direct3d11: vertex program '%s' compiled but the shader object could not be created (%s).", m_vertexShader.getFilename(), Direct3d11_Device::describeHresult(hresult)));
		m_shader = NULL;
	}
}

// ======================================================================

void const *Direct3d11_VertexShaderData::getBytecode() const
{
	return m_bytecode ? m_bytecode->GetBufferPointer() : NULL;
}

// ----------------------------------------------------------------------

unsigned int Direct3d11_VertexShaderData::getBytecodeSize() const
{
	return m_bytecode ? static_cast<unsigned int>(m_bytecode->GetBufferSize()) : 0;
}

// ----------------------------------------------------------------------

uint32 Direct3d11_VertexShaderData::getSignatureHash() const
{
	return m_signatureHash;
}

// ======================================================================
