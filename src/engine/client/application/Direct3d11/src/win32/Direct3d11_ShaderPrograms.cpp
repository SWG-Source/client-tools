// ======================================================================
//
// Direct3d11_ShaderPrograms.cpp
// copyright (c) 2026 Galaxies Reborn
//
// ======================================================================

#include "Direct3d11_ShaderPrograms.h"

#include "Direct3d11_ShaderSource.h"

#include <stdio.h>
#include <string.h>

// ======================================================================

namespace Direct3d11_ShaderProgramsNamespace
{
	// The tag whose set is four components rather than two. Spelled as characters rather than
	// built with the engine's TAG macro, because nothing here includes the engine.
	char const * const cms_dot3Tag = "DOT3";

	// ------------------------------------------------------------------

	void skipRestOfTheLine(char const *&s, char const *end)
	{
		// Backslash-newline continues the line. The corpus's own DECLARE_textureCoordinateSets is
		// written that way, so stopping at the first physical newline would leave a stray
		// continuation in the compilable source -- a syntax error rather than a subtle one. DX9's
		// skipRestOfTheLine walks over the continuation for the same reason.
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

	// ------------------------------------------------------------------

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
using namespace Direct3d11_ShaderProgramsNamespace;

// ======================================================================

Direct3d11_ShaderPrograms::Header::Header()
:
	language(L_unknown),
	sourceOffset(0),
	sourceLength(0),
	tagCount(0),
	tooManyTags(false)
{
	memset(tags, 0, sizeof(tags));
}

// ======================================================================

Direct3d11_ShaderPrograms::Macros::Macros()
:
	m_count(0)
{
	memset(m_macros, 0, sizeof(m_macros));
	memset(m_names, 0, sizeof(m_names));
	memset(m_definitions, 0, sizeof(m_definitions));
	m_declaration[0] = 0;
}

// ----------------------------------------------------------------------

void Direct3d11_ShaderPrograms::Macros::add(char const *name, char const *definition)
{
	// One short of MAX_MACROS: terminate() needs the last entry.
	if (m_count >= MAX_MACROS - 1)
		return;

	// The pointers handed to D3DCompile have to outlive this call, so the text is copied into
	// this object rather than referenced.
	strncpy(m_names[m_count], name, TEXT_BYTES - 1);
	m_names[m_count][TEXT_BYTES - 1] = 0;

	strncpy(m_definitions[m_count], definition, TEXT_BYTES - 1);
	m_definitions[m_count][TEXT_BYTES - 1] = 0;

	m_macros[m_count].Name = m_names[m_count];
	m_macros[m_count].Definition = m_definitions[m_count];
	++m_count;
}

// ----------------------------------------------------------------------

void Direct3d11_ShaderPrograms::Macros::terminate()
{
	m_macros[m_count].Name = NULL;
	m_macros[m_count].Definition = NULL;
}

// ----------------------------------------------------------------------

void Direct3d11_ShaderPrograms::Macros::buildForVertexProgram(Header const &header)
{
	m_count = 0;
	m_declaration[0] = 0;

	int declarationLength = 0;

	for (int i = 0; i < header.tagCount; ++i)
	{
		char tagName[5];
		memcpy(tagName, header.tags[i], 4);
		tagName[4] = 0;

		char name[TEXT_BYTES];
		char definition[TEXT_BYTES];
		sprintf(name, "textureCoordinateSet%s", tagName);
		sprintf(definition, "textureCoordinateSet%d", i);
		add(name, definition);

		int const dimension = (memcmp(header.tags[i], cms_dot3Tag, 4) == 0) ? 4 : 2;

		char member[96];
		int const memberLength = sprintf(member, "float%d textureCoordinateSet%d : TEXCOORD%d;", dimension, i, i);

		if (declarationLength + memberLength < DECLARATION_BYTES)
		{
			memcpy(m_declaration + declarationLength, member, static_cast<size_t>(memberLength));
			declarationLength += memberLength;
			m_declaration[declarationLength] = 0;
		}
	}

	// Only when the program declares sets. Defining it empty would be equivalent, but a program
	// that declares none also never references it, and an absent macro is what DX9 left it as.
	if (header.tagCount)
	{
		if (m_count < MAX_MACROS - 1)
		{
			strncpy(m_names[m_count], "DECLARE_textureCoordinateSets", TEXT_BYTES - 1);
			m_names[m_count][TEXT_BYTES - 1] = 0;
			m_macros[m_count].Name = m_names[m_count];
			m_macros[m_count].Definition = m_declaration;
			++m_count;
		}
	}

	// Inherited from the DX9 x64 build: `point` is reserved in modern HLSL and the shipped
	// includes use it as a field name on LightData.
	D3D_SHADER_MACRO const pointRename = Direct3d11_ShaderSource::getPointRenameMacro();
	if (pointRename.Name && pointRename.Definition)
		add(pointRename.Name, pointRename.Definition);

	terminate();
}

// ----------------------------------------------------------------------

void Direct3d11_ShaderPrograms::Macros::buildForPixelProgram()
{
	m_count = 0;
	m_declaration[0] = 0;

	D3D_SHADER_MACRO const pointRename = Direct3d11_ShaderSource::getPointRenameMacro();
	if (pointRename.Name && pointRename.Definition)
		add(pointRename.Name, pointRename.Definition);

	terminate();
}

// ----------------------------------------------------------------------

D3D_SHADER_MACRO const *Direct3d11_ShaderPrograms::Macros::get() const
{
	return m_macros;
}

// ----------------------------------------------------------------------

int Direct3d11_ShaderPrograms::Macros::getCount() const
{
	return m_count;
}

// ======================================================================

bool Direct3d11_ShaderPrograms::parseHeader(char const *text, int length, Header &header)
{
	// The null check comes before any pointer arithmetic, and the order is the whole point.
	//
	// Forming `begin + length` when begin is null is undefined behaviour, and an optimiser is
	// entitled to reason backwards from it: the arithmetic is only defined if begin is non-null,
	// therefore begin is non-null, therefore the null test is dead and can be deleted. In a
	// Release build it was, and the first run of the client crashed reading a small address --
	// exactly what walking a null buffer with a non-null end looks like.
	if (!text || length <= 0)
		return false;

	char const * const begin = text;
	char const * const end = begin + length;
	char const *cursor = begin;

	for (bool done = false; !done && cursor < end; )
	{
		// Where this line started: the first line that is neither the marker nor a #define is
		// where the compilable source begins.
		char const * const lineStart = cursor;

		char token[128];
		char const *scan = cursor;
		getToken(scan, end, token, static_cast<int>(sizeof(token)));

		if (strcmp(token, "//hlsl") == 0)
		{
			header.language = L_hlsl;
			cursor = scan;
			skipRestOfTheLine(cursor, end);
		}
		else if (strcmp(token, "//asm") == 0)
		{
			header.language = L_assembly;
			cursor = scan;
			skipRestOfTheLine(cursor, end);
		}
		else if (strcmp(token, "#define") == 0)
		{
			getToken(scan, end, token, static_cast<int>(sizeof(token)));

			// A tag define names a set by tag rather than by number:
			// textureCoordinateSetMAIN, not textureCoordinateSet0.
			if (strncmp(token, "textureCoordinateSet", 20) == 0)
			{
				char const * const suffix = token + 20;
				if (strlen(suffix) == 4)
				{
					if (header.tagCount < MAX_TEXTURE_COORDINATE_SETS)
					{
						memcpy(header.tags[header.tagCount], suffix, 4);
						++header.tagCount;
					}
					else
					{
						header.tooManyTags = true;
					}
				}
			}

			cursor = scan;
			skipRestOfTheLine(cursor, end);
		}
		else
		{
			cursor = lineStart;
			done = true;
		}
	}

	header.sourceOffset = static_cast<int>(cursor - begin);
	header.sourceLength = static_cast<int>(end - cursor);
	return true;
}

// ======================================================================
/**
 * FNV-1a, 64 bit.
 *
 * Not a cryptographic hash and does not need to be: this detects a changed asset, not a forged
 * one. Chosen over the engine's Crc because it is eight bytes rather than four -- the cache keys
 * every program in the corpus by this value and uses it as a file name, and a 32-bit space with
 * six hundred entries is close enough to a birthday collision to be worth avoiding for free --
 * and because it is nine lines with no dependency, which matters for a header that has to
 * compile in a tool that does not link the engine.
 */

Direct3d11ShaderHash Direct3d11_ShaderPrograms::hashBytes(void const *data, int length)
{
	Direct3d11ShaderHash const offsetBasis = 14695981039346656037ULL;
	Direct3d11ShaderHash const prime = 1099511628211ULL;

	Direct3d11ShaderHash hash = offsetBasis;

	unsigned char const *at = static_cast<unsigned char const *>(data);
	for (int i = 0; i < length; ++i)
	{
		hash ^= static_cast<Direct3d11ShaderHash>(at[i]);
		hash *= prime;
	}

	return hash;
}

// ----------------------------------------------------------------------

Direct3d11ShaderHash Direct3d11_ShaderPrograms::hashCompilerInput(char const *preparedSource, int preparedLength, Macros const &macros, char const *target, unsigned int flags)
{
	// Folded in a fixed order, and every field separated by its own length, so that two different
	// inputs cannot produce the same byte stream by running into each other. Without the lengths,
	// a macro named "AB" defined as "C" and one named "A" defined as "BC" hash identically.
	Direct3d11ShaderHash hash = hashBytes(target, static_cast<int>(strlen(target)));

	hash ^= hashBytes(&flags, static_cast<int>(sizeof(flags)));
	hash *= 1099511628211ULL;

	int const count = macros.getCount();
	hash ^= hashBytes(&count, static_cast<int>(sizeof(count)));
	hash *= 1099511628211ULL;

	D3D_SHADER_MACRO const * const entries = macros.get();
	for (int i = 0; i < count; ++i)
	{
		int const nameLength = entries[i].Name ? static_cast<int>(strlen(entries[i].Name)) : 0;
		int const definitionLength = entries[i].Definition ? static_cast<int>(strlen(entries[i].Definition)) : 0;

		hash ^= hashBytes(&nameLength, static_cast<int>(sizeof(nameLength)));
		hash *= 1099511628211ULL;
		hash ^= hashBytes(entries[i].Name, nameLength);
		hash *= 1099511628211ULL;

		hash ^= hashBytes(&definitionLength, static_cast<int>(sizeof(definitionLength)));
		hash *= 1099511628211ULL;
		hash ^= hashBytes(entries[i].Definition, definitionLength);
		hash *= 1099511628211ULL;
	}

	hash ^= hashBytes(&preparedLength, static_cast<int>(sizeof(preparedLength)));
	hash *= 1099511628211ULL;
	hash ^= hashBytes(preparedSource, preparedLength);
	hash *= 1099511628211ULL;

	return hash;
}

// ======================================================================
