// ======================================================================
//
// Direct3d11_ShaderSignature.cpp
// copyright (c) 2026 Galaxies Reborn
//
// ======================================================================

#include "FirstDirect3d11.h"
#include "Direct3d11_ShaderSignature.h"

#include <string>
#include <vector>

// ======================================================================

namespace Direct3d11_ShaderSignatureNamespace
{
	// Every slot a float4, so each occupies a whole register and packing follows declaration order
	// exactly. See the header for the measurement that says narrower types repack.
	char const cms_interpolants[] =
		"struct SwgInterpolants\n"
		"{\n"
		"\tfloat4 swgPosition  : SV_Position;\n"
		"\tfloat4 swgColor0    : COLOR0;\n"
		"\tfloat4 swgColor1    : COLOR1;\n"
		"\tfloat4 swgFog       : FOG;\n"
		"\tfloat4 swgTexCoord0 : TEXCOORD0;\n"
		"\tfloat4 swgTexCoord1 : TEXCOORD1;\n"
		"\tfloat4 swgTexCoord2 : TEXCOORD2;\n"
		"\tfloat4 swgTexCoord3 : TEXCOORD3;\n"
		"\tfloat4 swgTexCoord4 : TEXCOORD4;\n"
		"\tfloat4 swgTexCoord5 : TEXCOORD5;\n"
		"\tfloat4 swgTexCoord6 : TEXCOORD6;\n"
		"\tfloat4 swgTexCoord7 : TEXCOORD7;\n"
		"};\n";

	struct Slot
	{
		char const *semantic;
		char const *member;
	};

	// The corpus writes COLOR and POSITION with and without an explicit 0, so both spellings map.
	Slot const cms_slots[] =
	{
		{ "POSITION",  "swgPosition"  },
		{ "POSITION0", "swgPosition"  },
		{ "COLOR",     "swgColor0"    },
		{ "COLOR0",    "swgColor0"    },
		{ "COLOR1",    "swgColor1"    },
		{ "FOG",       "swgFog"       },
		{ "TEXCOORD",  "swgTexCoord0" },
		{ "TEXCOORD0", "swgTexCoord0" },
		{ "TEXCOORD1", "swgTexCoord1" },
		{ "TEXCOORD2", "swgTexCoord2" },
		{ "TEXCOORD3", "swgTexCoord3" },
		{ "TEXCOORD4", "swgTexCoord4" },
		{ "TEXCOORD5", "swgTexCoord5" },
		{ "TEXCOORD6", "swgTexCoord6" },
		{ "TEXCOORD7", "swgTexCoord7" },
	};

	struct Declaration
	{
		std::string type;
		std::string name;
		std::string semantic;
	};

	bool        isIdentifierCharacter(char c);
	char const *memberForSemantic(char const *semantic);
	int         widthOfType(char const *type);
	int         compilableLength(char const *source, int length);
	bool        findEntryPoint(char const *source, int length, int &nameStart, int &parameterOpen, int &parameterClose);
	bool        findBody(char const *source, int length, int from, int &bodyStart, int &bodyEnd);
	bool        findStruct(char const *source, int length, char const *typeName, int &bodyStart, int &bodyEnd);
	void        parseDeclarations(char const *source, int from, int to, std::vector<Declaration> &result);
	std::string widen(std::string const &expression, int width);
	std::string narrow(std::string const &expression, int width);
	std::string readInterpolant(char const *member, int width);
	char       *duplicate(std::string const &text, int &resultLength);
}

using namespace Direct3d11_ShaderSignatureNamespace;

// ======================================================================

char const *Direct3d11_ShaderSignature::getInterpolantDeclaration()
{
	return cms_interpolants;
}

// ----------------------------------------------------------------------

int Direct3d11_ShaderSignature::getInterpolantDeclarationLength()
{
	return isizeof(cms_interpolants) - 1;
}

// ======================================================================

bool Direct3d11_ShaderSignatureNamespace::isIdentifierCharacter(char c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
}

// ----------------------------------------------------------------------
/**
 * How much of a program the compiler will actually read: everything up to the first NUL.
 *
 * This is load bearing, and it cost a debugging session. Shader text out of the TRE stack is not
 * reliably NUL-free -- the PSRC chunk of an IFF-wrapped pixel program carries a trailing NUL inside
 * the length the engine reports. D3DCompile stops at it.
 *
 * That was harmless while every transform edited the source in place or prepended to it. It stops
 * being harmless the moment something is APPENDED, which is exactly what a signature wrapper does:
 * the wrapper lands after the NUL, the compiler never sees it, and the failure is
 * "'main': entrypoint not found" on a source file that visibly contains main.
 *
 * The offline harness did not reproduce it because it strips NULs when it writes its test file --
 * so "it compiles in the harness" was true and useless. Anything appending to shader text has to
 * use this length, not the reported one.
 */

int Direct3d11_ShaderSignatureNamespace::compilableLength(char const *source, int length)
{
	for (int i = 0; i < length; ++i)
		if (source[i] == '\0')
			return i;

	return length;
}

// ----------------------------------------------------------------------

char const *Direct3d11_ShaderSignatureNamespace::memberForSemantic(char const *semantic)
{
	for (int i = 0; i < isizeof(cms_slots) / isizeof(cms_slots[0]); ++i)
		if (_stricmp(semantic, cms_slots[i].semantic) == 0)
			return cms_slots[i].member;

	return NULL;
}

// ----------------------------------------------------------------------
/**
 * Component count of a scalar or vector type, or 0 if it is neither.
 *
 * Only the float family appears in the corpus's interpolant declarations. half and int are accepted
 * because rejecting them silently would be worse than widening them, and a program using one would
 * otherwise fail with a confusing message about an unknown type rather than a clear one.
 */

int Direct3d11_ShaderSignatureNamespace::widthOfType(char const *type)
{
	char const *base = NULL;

	if (strncmp(type, "float", 5) == 0)
		base = type + 5;
	else if (strncmp(type, "half", 4) == 0)
		base = type + 4;
	else if (strncmp(type, "int", 3) == 0)
		base = type + 3;
	else
		return 0;

	if (base[0] == '\0')
		return 1;
	if (base[1] != '\0')
		return 0;
	if (base[0] >= '1' && base[0] <= '4')
		return base[0] - '0';

	return 0;
}

// ----------------------------------------------------------------------
/**
 * Find the entry point's name and the parentheses of its parameter list.
 *
 * Comment-aware, and that is not decoration: all 97 programs this port converted from assembly
 * carry the line "each #include below is a block of statements inside main()," and a scan that
 * ignored comments took that as the entry point.
 */

bool Direct3d11_ShaderSignatureNamespace::findEntryPoint(char const *source, int length, int &nameStart, int &parameterOpen, int &parameterClose)
{
	bool inLineComment = false;
	bool inBlockComment = false;

	for (int i = 0; i + 4 <= length; ++i)
	{
		if (inLineComment)
		{
			if (source[i] == '\n')
				inLineComment = false;
			continue;
		}

		if (inBlockComment)
		{
			if (source[i] == '*' && i + 1 < length && source[i + 1] == '/')
			{
				inBlockComment = false;
				++i;
			}
			continue;
		}

		if (source[i] == '/' && i + 1 < length)
		{
			if (source[i + 1] == '/') { inLineComment = true;  ++i; continue; }
			if (source[i + 1] == '*') { inBlockComment = true; ++i; continue; }
		}

		if (memcmp(source + i, "main", 4) != 0)
			continue;

		if (i > 0 && isIdentifierCharacter(source[i - 1]))
			continue;
		if (i + 4 < length && isIdentifierCharacter(source[i + 4]))
			continue;

		int j = i + 4;
		while (j < length && (source[j] == ' ' || source[j] == '\t' || source[j] == '\n' || source[j] == '\r'))
			++j;

		if (j >= length || source[j] != '(')
			continue;

		int depth = 0;
		for (int k = j; k < length; ++k)
		{
			if (source[k] == '(')
				++depth;
			else if (source[k] == ')')
			{
				--depth;
				if (depth == 0)
				{
					nameStart = i;
					parameterOpen = j;
					parameterClose = k;
					return true;
				}
			}
		}

		return false;
	}

	return false;
}

// ----------------------------------------------------------------------

bool Direct3d11_ShaderSignatureNamespace::findBody(char const *source, int length, int from, int &bodyStart, int &bodyEnd)
{
	int depth = 0;
	bodyStart = -1;
	bodyEnd = -1;

	for (int i = from; i < length; ++i)
	{
		if (source[i] == '{')
		{
			if (bodyStart < 0)
				bodyStart = i;
			++depth;
		}
		else if (source[i] == '}')
		{
			--depth;
			if (bodyStart >= 0 && depth == 0)
			{
				bodyEnd = i;
				return true;
			}
		}
	}

	return false;
}

// ----------------------------------------------------------------------

bool Direct3d11_ShaderSignatureNamespace::findStruct(char const *source, int length, char const *typeName, int &bodyStart, int &bodyEnd)
{
	int const typeLength = static_cast<int>(strlen(typeName));

	for (int i = 0; i + 6 <= length; ++i)
	{
		if (memcmp(source + i, "struct", 6) != 0)
			continue;
		if (i > 0 && isIdentifierCharacter(source[i - 1]))
			continue;
		if (isIdentifierCharacter(source[i + 6]))
			continue;

		int j = i + 6;
		while (j < length && (source[j] == ' ' || source[j] == '\t' || source[j] == '\n' || source[j] == '\r'))
			++j;

		if (j + typeLength > length || memcmp(source + j, typeName, static_cast<size_t>(typeLength)) != 0)
			continue;
		if (j + typeLength < length && isIdentifierCharacter(source[j + typeLength]))
			continue;

		if (findBody(source, length, j + typeLength, bodyStart, bodyEnd))
			return true;
	}

	return false;
}

// ----------------------------------------------------------------------
/**
 * Pull "<type> <name> : <SEMANTIC>" triples out of a range.
 *
 * Serves both a struct body and a parameter list. Leading "in" and "uniform" are skipped; a
 * trailing ": register(vN)" clause is ignored, because the vertex input transform strips those but
 * this must not depend on having run after it.
 */

void Direct3d11_ShaderSignatureNamespace::parseDeclarations(char const *source, int from, int to, std::vector<Declaration> &result)
{
	int i = from;

	while (i < to)
	{
		// Skip separators and comments.
		while (i < to && (source[i] == ' ' || source[i] == '\t' || source[i] == '\n' || source[i] == '\r' || source[i] == ',' || source[i] == ';'))
			++i;

		if (i + 1 < to && source[i] == '/' && source[i + 1] == '/')
		{
			while (i < to && source[i] != '\n')
				++i;
			continue;
		}

		if (i >= to)
			break;

		// A word.
		int wordStart = i;
		while (i < to && isIdentifierCharacter(source[i]))
			++i;

		if (i == wordStart)
		{
			++i;
			continue;
		}

		std::string word(source + wordStart, static_cast<size_t>(i - wordStart));

		if (word == "in" || word == "uniform" || word == "const")
			continue;

		// If this word is a type, the next word is the name and a colon should follow.
		if (widthOfType(word.c_str()) == 0)
			continue;

		while (i < to && (source[i] == ' ' || source[i] == '\t' || source[i] == '\n' || source[i] == '\r'))
			++i;

		int nameStart = i;
		while (i < to && isIdentifierCharacter(source[i]))
			++i;

		if (i == nameStart)
			continue;

		std::string name(source + nameStart, static_cast<size_t>(i - nameStart));

		while (i < to && (source[i] == ' ' || source[i] == '\t' || source[i] == '\n' || source[i] == '\r'))
			++i;

		if (i >= to || source[i] != ':')
			continue;

		++i;
		while (i < to && (source[i] == ' ' || source[i] == '\t' || source[i] == '\n' || source[i] == '\r'))
			++i;

		int semanticStart = i;
		while (i < to && isIdentifierCharacter(source[i]))
			++i;

		if (i == semanticStart)
			continue;

		Declaration declaration;
		declaration.type = word;
		declaration.name = name;
		declaration.semantic.assign(source + semanticStart, static_cast<size_t>(i - semanticStart));

		// "register" here is a location clause, not a semantic.
		if (_stricmp(declaration.semantic.c_str(), "register") == 0)
			continue;

		result.push_back(declaration);
	}
}

// ----------------------------------------------------------------------

std::string Direct3d11_ShaderSignatureNamespace::widen(std::string const &expression, int width)
{
	switch (width)
	{
		case 1:  return "float4((" + expression + "), 0.0f, 0.0f, 0.0f)";
		case 2:  return "float4((" + expression + "), 0.0f, 0.0f)";
		case 3:  return "float4((" + expression + "), 0.0f)";
		default: return "(" + expression + ")";
	}
}

// ----------------------------------------------------------------------

std::string Direct3d11_ShaderSignatureNamespace::readInterpolant(char const *member, int width)
{
	std::string expression = std::string("swgIn.") + member;

	// A COLOR interpolant is clamped to [0,1]. D3D9 did that in hardware -- anything carrying the
	// COLOR semantic was saturated at the interpolator, for every pixel shader version -- and
	// D3D10 removed the clamp. Nothing in the shipped corpus knows that.
	//
	// The shipped assembly programs multiply straight by v0 with no saturate of their own:
	// a_2blend_dirt.psh, which is what the starport's interior walls use, ends
	//
	//     mul r0.rgb, r0, v0
	//
	// and the vertex side accumulates ambient plus lights into oD0 without clamping either, because
	// under D3D9 it did not have to. Interior ambient is boosted, so oD0 arrives above one, the
	// multiply has nothing to hold it down, and the wall saturates to white against an UNORM
	// target. That is the blown-out interior: the texture is fine and the light is not, and the
	// missing piece is a clamp the hardware used to do.
	//
	// Clamping here rather than in each program covers every program at once -- 349 of them --
	// including the hand-written HLSL that reads COLOR0 directly.
	if (strcmp(member, "swgColor0") == 0 || strcmp(member, "swgColor1") == 0)
		expression = "saturate(" + expression + ")";

	return narrow(expression, width);
}

// ----------------------------------------------------------------------

std::string Direct3d11_ShaderSignatureNamespace::narrow(std::string const &expression, int width)
{
	switch (width)
	{
		case 1:  return expression + ".x";
		case 2:  return expression + ".xy";
		case 3:  return expression + ".xyz";
		default: return expression;
	}
}

// ----------------------------------------------------------------------

char *Direct3d11_ShaderSignatureNamespace::duplicate(std::string const &text, int &resultLength)
{
	resultLength = static_cast<int>(text.size());

	char * const result = new char[text.size() + 1];
	memcpy(result, text.c_str(), text.size() + 1);
	return result;
}

// ======================================================================
/**
 * Give a vertex program the canonical output signature.
 *
 * The original entry point is renamed and kept intact; a generated wrapper calls it and widens each
 * member it declared into the matching canonical slot.
 */

char *Direct3d11_ShaderSignature::wrapVertexProgram(char const *name, char const *source, int length, int &resultLength)
{
	// Everything past the first NUL is invisible to the compiler, so it is invisible here too --
	// otherwise the wrapper appended below lands after it. See compilableLength.
	length = compilableLength(source, length);

	int nameStart = 0;
	int parameterOpen = 0;
	int parameterClose = 0;

	if (!findEntryPoint(source, length, nameStart, parameterOpen, parameterClose))
	{
		WARNING(true, ("Direct3d11: vertex program '%s' has no recognisable entry point, so its signature cannot be canonicalised and it will not link to any pixel program.", name));
		return NULL;
	}

	// The return type is the identifier before the entry point's name.
	int typeEnd = nameStart;
	while (typeEnd > 0 && (source[typeEnd - 1] == ' ' || source[typeEnd - 1] == '\t' || source[typeEnd - 1] == '\n' || source[typeEnd - 1] == '\r'))
		--typeEnd;

	int typeStart = typeEnd;
	while (typeStart > 0 && isIdentifierCharacter(source[typeStart - 1]))
		--typeStart;

	if (typeEnd <= typeStart)
	{
		WARNING(true, ("Direct3d11: vertex program '%s' has an entry point with no return type.", name));
		return NULL;
	}

	std::string const returnType(source + typeStart, static_cast<size_t>(typeEnd - typeStart));

	if (returnType == "void")
	{
		WARNING(true, ("Direct3d11: vertex program '%s' returns void, so it writes its outputs through parameters. That shape is not in the corpus and is not handled; it will not link.", name));
		return NULL;
	}

	int structStart = 0;
	int structEnd = 0;
	if (!findStruct(source, length, returnType.c_str(), structStart, structEnd))
	{
		WARNING(true, ("Direct3d11: vertex program '%s' returns '%s', whose declaration could not be found, so its outputs cannot be mapped.", name, returnType.c_str()));
		return NULL;
	}

	std::vector<Declaration> outputs;
	parseDeclarations(source, structStart + 1, structEnd, outputs);

	if (outputs.empty())
	{
		WARNING(true, ("Direct3d11: vertex program '%s' has an output struct with no semantic members.", name));
		return NULL;
	}

	// The whole parameter list, forwarded verbatim. Every vertex program in the corpus takes a
	// single struct, but forwarding the text rather than the parsed form means an unusual one still
	// works as long as it compiles.
	std::string const parameters(source + parameterOpen + 1, static_cast<size_t>(parameterClose - parameterOpen - 1));

	// The parameter's NAME, for the forwarded call. Last identifier in the list.
	int argEnd = parameterClose;
	while (argEnd > parameterOpen && (source[argEnd - 1] == ' ' || source[argEnd - 1] == '\t' || source[argEnd - 1] == '\n' || source[argEnd - 1] == '\r'))
		--argEnd;

	int argStart = argEnd;
	while (argStart > parameterOpen && isIdentifierCharacter(source[argStart - 1]))
		--argStart;

	std::string const argument(source + argStart, static_cast<size_t>(argEnd - argStart));

	// ------------------------------------------------------------------
	// Build it: the original with its entry renamed, then the declaration, then the wrapper.

	std::string result;
	result.reserve(static_cast<size_t>(length) + 4096);

	result.append(source, static_cast<size_t>(nameStart));
	result.append("swgVertexMain");
	result.append(source + nameStart + 4, static_cast<size_t>(length - nameStart - 4));

	result.append("\n\n");
	result.append(cms_interpolants);
	result.append("\nSwgInterpolants main(");
	result.append(parameters);
	result.append(")\n{\n\t");
	result.append(returnType);
	result.append(" swgOriginal = swgVertexMain(");
	result.append(argument);
	result.append(");\n\n\tSwgInterpolants swgResult = (SwgInterpolants)0;\n\n");

	// 1.0 is the fog factor's no-fog value. Assigned first so a program that DOES write fog
	// overwrites it below, and one that does not is left unfogged rather than fully fogged, which
	// is what the zero from the struct initialiser would mean.
	result.append("\t// No-fog default; overwritten below if the program writes a fog output.\n");
	result.append("\tswgResult.swgFog = float4(1.0f, 1.0f, 1.0f, 1.0f);\n\n");

	bool mappedAnything = false;

	for (size_t i = 0; i < outputs.size(); ++i)
	{
		char const * const member = memberForSemantic(outputs[i].semantic.c_str());
		if (!member)
		{
			WARNING(true, ("Direct3d11: vertex program '%s' writes semantic '%s', which is not one of the twelve the canonical interpolant set covers. It will not reach the pixel stage.", name, outputs[i].semantic.c_str()));
			continue;
		}

		int const width = widthOfType(outputs[i].type.c_str());
		if (width == 0)
		{
			WARNING(true, ("Direct3d11: vertex program '%s' declares output '%s' as type '%s', whose width is not recognised.", name, outputs[i].name.c_str(), outputs[i].type.c_str()));
			continue;
		}

		result.append("\tswgResult.");
		result.append(member);
		result.append(" = ");
		result.append(widen("swgOriginal." + outputs[i].name, width));
		result.append(";\n");

		mappedAnything = true;
	}

	if (!mappedAnything)
	{
		WARNING(true, ("Direct3d11: vertex program '%s' has no output that maps to a canonical slot.", name));
		return NULL;
	}

	result.append("\n\treturn swgResult;\n}\n");

	return duplicate(result, resultLength);
}

// ======================================================================
/**
 * Give a pixel program the canonical input signature, and apply the epilogue.
 *
 * The original entry point is renamed and kept; the wrapper narrows each canonical slot back to the
 * type the original expects. The alpha test and fog blend happen here rather than being injected
 * into the original body, which is both simpler and the only place the fog factor is in scope.
 */

char *Direct3d11_ShaderSignature::wrapPixelProgram(char const *name, char const *source, int length, int &resultLength)
{
	length = compilableLength(source, length);

	int nameStart = 0;
	int parameterOpen = 0;
	int parameterClose = 0;

	if (!findEntryPoint(source, length, nameStart, parameterOpen, parameterClose))
	{
		WARNING(true, ("Direct3d11: pixel program '%s' has no recognisable entry point, so its signature cannot be canonicalised and it will not link to any vertex program.", name));
		return NULL;
	}

	int bodyStart = 0;
	int bodyEnd = 0;
	if (!findBody(source, length, parameterClose, bodyStart, bodyEnd))
	{
		WARNING(true, ("Direct3d11: pixel program '%s' has an entry point whose body could not be delimited.", name));
		return NULL;
	}

	// Everything the corpus puts between ')' and '{' is a return semantic, ": COLOR". It has to go:
	// the renamed function is no longer an entry point, and a return semantic on an ordinary
	// function is at best ignored and at worst a warning nobody reads.
	std::string const trailer(source + parameterClose + 1, static_cast<size_t>(bodyStart - parameterClose - 1));
	bool const trailerIsSemantic = trailer.find(':') != std::string::npos;

	// ------------------------------------------------------------------
	// What the original entry point wants, and how to supply it from the canonical struct.

	std::vector<Declaration> inputs;
	parseDeclarations(source, parameterOpen + 1, parameterClose, inputs);

	std::string forwarded;
	bool structParameter = false;
	std::string structType;
	std::string structName;

	if (inputs.empty())
	{
		// Either it takes nothing, or it takes a single struct whose members carry the semantics.
		std::string list(source + parameterOpen + 1, static_cast<size_t>(parameterClose - parameterOpen - 1));

		size_t first = list.find_first_not_of(" \t\r\n");
		if (first != std::string::npos)
		{
			// "<Type> <name>", possibly with "in"/"const".
			std::vector<Declaration> dummy;
			int i = static_cast<int>(first);
			int const to = static_cast<int>(list.size());

			std::vector<std::string> words;
			while (i < to)
			{
				while (i < to && !isIdentifierCharacter(list[static_cast<size_t>(i)]))
					++i;
				int start = i;
				while (i < to && isIdentifierCharacter(list[static_cast<size_t>(i)]))
					++i;
				if (i > start)
					words.push_back(list.substr(static_cast<size_t>(start), static_cast<size_t>(i - start)));
			}

			while (!words.empty() && (words[0] == "in" || words[0] == "const" || words[0] == "uniform"))
				words.erase(words.begin());

			if (words.size() >= 2)
			{
				structType = words[0];
				structName = words[1];

				int inputStructStart = 0;
				int inputStructEnd = 0;
				if (findStruct(source, length, structType.c_str(), inputStructStart, inputStructEnd))
				{
					parseDeclarations(source, inputStructStart + 1, inputStructEnd, dummy);
					inputs = dummy;
					structParameter = true;
				}
				else
				{
					WARNING(true, ("Direct3d11: pixel program '%s' takes a '%s', whose declaration could not be found, so its inputs cannot be mapped.", name, structType.c_str()));
					return NULL;
				}
			}
		}
	}

	// ------------------------------------------------------------------

	std::string result;
	result.reserve(static_cast<size_t>(length) + 4096);

	result.append(source, static_cast<size_t>(nameStart));
	result.append("swgPixelMain");
	result.append(source + nameStart + 4, static_cast<size_t>(parameterClose + 1 - nameStart - 4));

	if (trailerIsSemantic)
		result.append("\n");
	else
		result.append(trailer);

	result.append(source + bodyStart, static_cast<size_t>(length - bodyStart));

	result.append("\n\n");
	result.append(cms_interpolants);
	result.append("\nfloat4 main(SwgInterpolants swgIn) : SV_Target\n{\n");

	if (structParameter)
	{
		result.append("\t");
		result.append(structType);
		result.append(" swgForwarded;\n");

		for (size_t i = 0; i < inputs.size(); ++i)
		{
			char const * const member = memberForSemantic(inputs[i].semantic.c_str());
			int const width = widthOfType(inputs[i].type.c_str());

			if (!member || width == 0)
			{
				WARNING(true, ("Direct3d11: pixel program '%s' reads '%s : %s', which the canonical interpolant set does not cover; it will receive zero.", name, inputs[i].name.c_str(), inputs[i].semantic.c_str()));
				result.append("\tswgForwarded.");
				result.append(inputs[i].name);
				result.append(" = 0;\n");
				continue;
			}

			result.append("\tswgForwarded.");
			result.append(inputs[i].name);
			result.append(" = ");
			result.append(readInterpolant(member, width));
			result.append(";\n");
		}

		result.append("\n\treturn swgPixelEpilogue(swgPixelMain(swgForwarded), swgIn.swgFog.x);\n}\n");
	}
	else
	{
		std::string arguments;

		for (size_t i = 0; i < inputs.size(); ++i)
		{
			if (!arguments.empty())
				arguments.append(", ");

			char const * const member = memberForSemantic(inputs[i].semantic.c_str());
			int const width = widthOfType(inputs[i].type.c_str());

			if (!member || width == 0)
			{
				WARNING(true, ("Direct3d11: pixel program '%s' reads '%s : %s', which the canonical interpolant set does not cover; it will receive zero.", name, inputs[i].name.c_str(), inputs[i].semantic.c_str()));
				arguments.append("(");
				arguments.append(inputs[i].type);
				arguments.append(")0");
				continue;
			}

			arguments.append(readInterpolant(member, width));
		}

		result.append("\treturn swgPixelEpilogue(swgPixelMain(");
		result.append(arguments);
		result.append("), swgIn.swgFog.x);\n}\n");
	}

	return duplicate(result, resultLength);
}

// ======================================================================
