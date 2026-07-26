// ======================================================================
//
// Direct3d11_VertexShaderData.cpp
// copyright (c) 2026 Galaxies Reborn
//
// ======================================================================

#include "FirstDirect3d11.h"
#include "Direct3d11_VertexShaderData.h"

#include "Direct3d11_Device.h"
#include "Direct3d11_Metrics.h"
#include "Direct3d11_ShaderCompiler.h"
#include "Direct3d11_ShaderSource.h"

#include "sharedFoundation/Tag.h"

#include <vector>

// ======================================================================

namespace Direct3d11_VertexShaderDataNamespace
{
	Tag const TAG_DOT3 = TAG(D,O,T,3);

	int ms_liveInstanceCount;
	bool ms_installed;

	// The macros the canonical mapping needs. Rebuilt per program, which happens once per
	// program for the life of the process, so the storage is reused rather than allocated.
	std::vector<D3D_SHADER_MACRO> ms_macros;
	std::vector<std::string>      ms_macroText;

	// ------------------------------------------------------------------

	void skipRestOfTheLine(char const *&s, char const *end)
	{
		// Backslash-newline continues the line. The corpus's own
		// DECLARE_textureCoordinateSets is written that way, so stopping at the first
		// physical newline would leave a stray continuation in the compilable source --
		// which is a syntax error rather than a subtle one. DX9's skipRestOfTheLine walks
		// over the continuation for the same reason.
		while (s < end && *s != '\n' && *s != '\r')
		{
			if (*s == '\\')
			{
				++s;
				if (s < end && *s == '\r')
					++s;
				if (s < end && *s == '\n')
					++s;
			}
			else
			{
				++s;
			}
		}

		while (s < end && (*s == '\n' || *s == '\r'))
			++s;
	}

	void getToken(char const *&s, char const *end, char *destination, int destinationSize)
	{
		int written = 0;

		while (s < end && (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r'))
			++s;

		while (s < end && *s != ' ' && *s != '\t' && *s != '\n' && *s != '\r')
		{
			if (written < destinationSize - 1)
				destination[written++] = *s;
			++s;
		}

		destination[written] = 0;
	}
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

	ms_macros.clear();
	ms_macroText.clear();
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
	// The null check comes before any pointer arithmetic, and the order is the whole point.
	//
	// This previously read
	//
	//   char const *text = m_vertexShader.m_text;
	//   char const * const end = text + m_vertexShader.m_textLength;
	//   if (!text || ...) return;
	//
	// which forms `null + n` when m_text is null. That is undefined behaviour, and an optimiser
	// is entitled to reason backwards from it: the arithmetic is only defined if m_text is
	// non-null, therefore m_text is non-null, therefore the `!text` test below is dead and can be
	// deleted. In a Release build it was, and the first run of the client crashed here with an
	// access violation reading a small address -- exactly what walking a null buffer with a
	// non-null end looks like.
	//
	// The pointer arithmetic now happens only after the pointer is known good.
	char const * const begin = m_vertexShader.m_text;
	int const length = m_vertexShader.m_textLength;

	if (!begin || length <= 0)
	{
		// Names the program, because "which one" is the entire question when this fires. The
		// engine's loader does not check TreeFile::open's result before calling length() on it
		// (ShaderImplementation.cpp's ShaderImplementationPassVertexShader::load), so a program
		// that resolves to nothing arrives here rather than being refused there.
		WARNING(true, ("Direct3d11: vertex program '%s' has no source text (text %p, length %d), so it cannot be compiled.",
			m_vertexShader.getFilename(), static_cast<void const *>(begin), length));
		return;
	}

	char const *text = begin;
	char const * const end = begin + length;

	bool isHlsl = false;
	bool isAssembly = false;

	for (bool done = false; !done && text < end; )
	{
		// Remember where this line started: the first line that is neither the marker nor
		// a #define is where the compilable source begins.
		char const * const lineStart = text;

		char token[128];
		char const *cursor = text;
		getToken(cursor, end, token, isizeof(token));

		if (strcmp(token, "//hlsl") == 0)
		{
			isHlsl = true;
			text = cursor;
			skipRestOfTheLine(text, end);
		}
		else if (strcmp(token, "//asm") == 0)
		{
			isAssembly = true;
			text = cursor;
			skipRestOfTheLine(text, end);
		}
		else if (strcmp(token, "#define") == 0)
		{
			getToken(cursor, end, token, isizeof(token));

			// A tag define names a set by tag rather than by number:
			// textureCoordinateSetMAIN, not textureCoordinateSet0.
			if (strncmp(token, "textureCoordinateSet", 20) == 0)
			{
				char const * const suffix = token + 20;
				if (strlen(suffix) == 4)
				{
					if (!m_textureCoordinateSetTags)
						m_textureCoordinateSetTags = new TextureCoordinateSetTags;
					m_textureCoordinateSetTags->push_back(ConvertStringToTag(suffix));
				}
			}

			text = cursor;
			skipRestOfTheLine(text, end);
		}
		else
		{
			text = lineStart;
			done = true;
		}
	}

	if (isAssembly || !isHlsl)
	{
		// Refused loudly rather than half-compiled. scripts/asm2hlsl converts these offline
		// and client-assets carries the results, so an assembly program arriving here means
		// the converted overlay is not deployed.
		WARNING(true, ("Direct3d11: vertex program '%s' is %s. D3DCompile has no assembler; the converted HLSL in client-assets is missing from the search path.",
			m_vertexShader.getFilename(),
			isAssembly ? "Direct3D 9 assembly" : "not marked as HLSL"));
		return;
	}

	m_source = text;
	m_sourceLength = static_cast<int>(end - text);
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

	ms_macros.clear();
	ms_macroText.clear();

	int const tagCount = m_textureCoordinateSetTags ? static_cast<int>(m_textureCoordinateSetTags->size()) : 0;

	DEBUG_WARNING(tagCount > 8, ("Direct3d11: vertex program '%s' declares %d texture coordinate sets; the engine supports 8.", m_vertexShader.getFilename(), tagCount));

	if (tagCount)
	{
		// Reserve so that the strings do not move while pointers into them are taken.
		ms_macroText.reserve(static_cast<size_t>((tagCount * 2) + 2));

		std::string declaration;

		for (int i = 0; i < tagCount; ++i)
		{
			Tag const tag = (*m_textureCoordinateSetTags)[i];

			char tagName[5];
			ConvertTagToString(tag, tagName);

			char indexText[8];
			sprintf(indexText, "%d", i);

			ms_macroText.push_back(std::string("textureCoordinateSet") + tagName);
			ms_macroText.push_back(std::string("textureCoordinateSet") + indexText);

			int const dimension = (tag == TAG_DOT3) ? 4 : 2;

			char member[96];
			sprintf(member, "float%d textureCoordinateSet%d : TEXCOORD%d;", dimension, i, i);
			declaration += member;
		}

		ms_macroText.push_back("DECLARE_textureCoordinateSets");
		ms_macroText.push_back(declaration);
	}

	// Built after every string is in place, so none of these pointers can dangle.
	for (size_t i = 0; i + 1 < ms_macroText.size(); i += 2)
	{
		D3D_SHADER_MACRO macro;
		macro.Name = ms_macroText[i].c_str();
		macro.Definition = ms_macroText[i + 1].c_str();
		ms_macros.push_back(macro);
	}

	// Inherited from the DX9 x64 build: `point` is reserved in modern HLSL and the shipped
	// includes use it as a field name on LightData.
	ms_macros.push_back(Direct3d11_ShaderSource::getPointRenameMacro());

	D3D_SHADER_MACRO const terminator = { NULL, NULL };
	ms_macros.push_back(terminator);

	m_bytecode = Direct3d11_ShaderCompiler::compileVertexShader(m_source, m_sourceLength, m_vertexShader.getFilename(), &ms_macros.front());
	if (!m_bytecode)
		return;

	ID3D11Device1 * const device = Direct3d11_Device::getDevice();
	if (!device)
		return;

	HRESULT const hresult = device->CreateVertexShader(m_bytecode->GetBufferPointer(), m_bytecode->GetBufferSize(), NULL, &m_shader);
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

// ======================================================================
