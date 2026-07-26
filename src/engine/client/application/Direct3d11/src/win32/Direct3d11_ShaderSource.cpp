// ======================================================================
//
// Direct3d11_ShaderSource.cpp
// copyright (c) 2026 Galaxies Reborn
//
// ======================================================================

#include "FirstDirect3d11.h"
#include "Direct3d11_ShaderSource.h"

#include <string.h>

// ======================================================================

namespace Direct3d11_ShaderSourceNamespace
{
	// ------------------------------------------------------------------
	// Patch 4: the engine's own pixel constant layout.
	//
	// Transcribed from Direct3d9_PixelShaderProgramData.cpp, which introduced it for the
	// x64 build and documents at length why the TRE copy cannot be used once shaders are
	// recompiled from source. Flat float4 declarations plus macros -- note that it is
	// already in exactly the shape patch 5 has to put the vertex constants into, for the
	// same reason.

	// The guard is not decoration. Several programs include functions.inc BEFORE the
	// constants file it depends on, so the constants have to be pulled in from
	// functions.inc as well (see cms_pixelFunctionsPrologue), and without a guard the
	// second inclusion redeclares every register and fails with X4019.
	char const cms_pixelShaderConstants[] =
		"#ifndef D3D11_PIXEL_SHADER_CONSTANTS\n"
		"#define D3D11_PIXEL_SHADER_CONSTANTS\n"
		"float4    packedRegister0        : register(c0);\n"
		"float4    packedRegister1        : register(c1);\n"
		"float4    packedRegister2        : register(c2);\n"
		"float4    packedRegister3        : register(c3);\n"
		"float4    packedRegister4        : register(c4);\n"
		"float4    textureFactor          : register(c5);\n"
		"float4    textureFactor2         : register(c6);\n"
		"float4    materialSpecularColor  : register(c7);\n"
		"float4    userConstants[17]      : register(c8);\n"
		"\n"
		"static const float bloomSpecularRgbScale = 0.5;\n"
		"static const float bloomSpecularAlphaScale = 0.65;\n"
		"\n"
		"#define dot3LightDirection      packedRegister0.xyz\n"
		"#define materialSpecularPower   packedRegister0.w\n"
		"#define dot3LightDiffuseColor   packedRegister1.rgb\n"
		"#define alphaFadeOpacityEnabled packedRegister1.a\n"
		"#define dot3LightSpecularColor  packedRegister2.rgb\n"
		"#define alphaFadeOpacity        packedRegister2.a\n"
		"#define dot3LightTangentMinusDiffuseColor packedRegister3.rgb\n"
		"#define bloomEnabled            packedRegister3.a\n"
		"#define dot3LightTangentMinusBackColor    packedRegister4.rgb\n"
		"#define timeElapsed             packedRegister4.a\n"
		"#endif\n";

	// ------------------------------------------------------------------
	// Patch 7: make the pixel functions include order-independent.
	//
	// pixel_program/include/functions.inc uses materialSpecularPower, which
	// pixel_shader_constants.inc defines -- but a number of programs include functions.inc
	// FIRST, so the identifier is undefined at the point of use. DX9 survives that only
	// because a source-compile failure falls back to the program's precompiled PEXE blob.
	// There is no PEXE path here, so the dependency is made explicit instead. Two of the
	// programs this fixes, 2d_bloom.psh and 2d_blur.psh, are what Bloom::install fetches
	// by name.

	char const cms_pixelFunctionsPath[] = "pixel_program/include/functions.inc";
	char const cms_pixelFunctionsPrologue[] = "#include \"pixel_program/include/pixel_shader_constants.inc\"\n";

	// ------------------------------------------------------------------
	// Patch 5: the vertex constants, flattened.
	//
	// Types are byte-identical to the shipped include so that any shader written against
	// it still compiles. Only the binding changes: one float4 per register, and the
	// aggregates rebuilt as statics.
	//
	// Field order inside Dot3Light is cameraPosition_o BEFORE direction_o, matching
	// Dot3Data in Direct3d9_LightManager.h. An older copy of this include in the TRE
	// stack has those two the other way round, and that copy resolves first unless
	// search-tree precedence is handled correctly -- equal-priority trees resolve
	// last-added-first, which is the opposite of what TreeFile's own comment claims.

	char const cms_vertexShaderConstants[] =
		"struct Material\n"
		"{\n"
		"	float4   diffuseColor;\n"
		"	float4   ambientColor;\n"
		"	float4   specularColor;\n"
		"	float4   emissiveColor;\n"
		"	float    specularPower;\n"
		"};\n"
		"\n"
		"struct AmbientLight\n"
		"{\n"
		"	float4   ambientColor;\n"
		"};\n"
		"\n"
		"struct ParallelSpecularLight\n"
		"{\n"
		"	float3   direction_w;\n"
		"	float4   diffuseColor;\n"
		"	float4   specularColor;\n"
		"};\n"
		"\n"
		"struct ParallelLight\n"
		"{\n"
		"	float3   direction_w;\n"
		"	float4   diffuseColor;\n"
		"};\n"
		"\n"
		"struct PointSpecularLight\n"
		"{\n"
		"	float3   position_w;\n"
		"	float4   diffuseColor;\n"
		"	float4   attenuation;\n"
		"	float4   specularColor;\n"
		"};\n"
		"\n"
		"struct PointLight\n"
		"{\n"
		"	float3   position_w;\n"
		"	float4   diffuseColor;\n"
		"	float4   attenuation;\n"
		"};\n"
		"\n"
		"struct Dot3Light\n"
		"{\n"
		"	float3   cameraPosition_o;\n"
		"	float3   direction_o;\n"
		"	float4   diffuseColor;\n"
		"	float4   specularColor;\n"
		"};\n"
		"\n"
		"struct DiffuseSpecular\n"
		"{\n"
		"	float4   diffuse;\n"
		"	float4   specular;\n"
		"};\n"
		"\n"
		"static const int NumberOfParallelSpecularLights = 1;\n"
		"static const int NumberOfParallelLights         = 2;\n"
		"static const int NumberOfPointSpecularLights    = 1;\n"
		"static const int NumberOfPointLights            = 4;\n"
		"static const int NumberOfDot3Lights             = 1;\n"
		"\n"
		"struct LightData\n"
		"{\n"
		"	AmbientLight           ambient;\n"
		"	ParallelSpecularLight  parallelSpecular[NumberOfParallelSpecularLights];\n"
		"	ParallelLight          parallel[NumberOfParallelLights];\n"
		"	PointSpecularLight     pointSpecular[NumberOfPointSpecularLights];\n"
		"	PointLight             point[NumberOfPointLights];\n"
		"	Dot3Light              dot3[NumberOfDot3Lights];\n"
		"};\n"
		"\n"
		"float4x4  objectWorldCameraProjectionMatrix : register(c0);\n"
		"float4x4  objectWorldMatrix : register(c4);\n"
		"float3    cameraPosition_w : register(c8);\n"
		"float4    viewportData : register(c9);\n"
		"float4    fog : register(c10);\n"
		"\n"
		"float4    vsMaterialDiffuseColor  : register(c11);\n"
		"float4    vsMaterialAmbientColor  : register(c12);\n"
		"float4    vsMaterialSpecularColor : register(c13);\n"
		"float4    vsMaterialEmissiveColor : register(c14);\n"
		"float4    vsMaterialSpecularPower : register(c15);\n"
		"\n"
		"float4    vsAmbientColor : register(c16);\n"
		"\n"
		"float4    vsParallelSpecular0Direction : register(c17);\n"
		"float4    vsParallelSpecular0Diffuse   : register(c18);\n"
		"float4    vsParallelSpecular0Specular  : register(c19);\n"
		"\n"
		"float4    vsParallel0Direction : register(c20);\n"
		"float4    vsParallel0Diffuse   : register(c21);\n"
		"float4    vsParallel1Direction : register(c22);\n"
		"float4    vsParallel1Diffuse   : register(c23);\n"
		"\n"
		"float4    vsPointSpecular0Position    : register(c24);\n"
		"float4    vsPointSpecular0Diffuse     : register(c25);\n"
		"float4    vsPointSpecular0Attenuation : register(c26);\n"
		"float4    vsPointSpecular0Specular    : register(c27);\n"
		"\n"
		"float4    vsPoint0Position    : register(c28);\n"
		"float4    vsPoint0Diffuse     : register(c29);\n"
		"float4    vsPoint0Attenuation : register(c30);\n"
		"float4    vsPoint1Position    : register(c31);\n"
		"float4    vsPoint1Diffuse     : register(c32);\n"
		"float4    vsPoint1Attenuation : register(c33);\n"
		"float4    vsPoint2Position    : register(c34);\n"
		"float4    vsPoint2Diffuse     : register(c35);\n"
		"float4    vsPoint2Attenuation : register(c36);\n"
		"float4    vsPoint3Position    : register(c37);\n"
		"float4    vsPoint3Diffuse     : register(c38);\n"
		"float4    vsPoint3Attenuation : register(c39);\n"
		"\n"
		"float4    vsDot30CameraPosition : register(c40);\n"
		"float4    vsDot30Direction      : register(c41);\n"
		"float4    vsDot30Diffuse        : register(c42);\n"
		"float4    vsDot30Specular       : register(c43);\n"
		"\n"
		"float4    textureFactor : register(c44);\n"
		"float4    textureFactor2 : register(c45);\n"
		"float4    textureScroll : register(c47);\n"
		"float     currentTime : register(c48);\n"
		"float3    unitX : register(c49);\n"
		"float3    unitY : register(c50);\n"
		"float3    unitZ : register(c51);\n"
		"\n"
		"float4    userConstant0 : register(c52);\n"
		"float4    userConstant1 : register(c53);\n"
		"float4    userConstant2 : register(c54);\n"
		"float4    userConstant3 : register(c55);\n"
		"float4    userConstant4 : register(c56);\n"
		"float4    userConstant5 : register(c57);\n"
		"float4    userConstant6 : register(c58);\n"
		"float4    userConstant7 : register(c59);\n"
		"\n"
		"static Material material =\n"
		"{\n"
		"	vsMaterialDiffuseColor,\n"
		"	vsMaterialAmbientColor,\n"
		"	vsMaterialSpecularColor,\n"
		"	vsMaterialEmissiveColor,\n"
		"	vsMaterialSpecularPower.x\n"
		"};\n"
		"\n"
		"static LightData lightData =\n"
		"{\n"
		"	{ vsAmbientColor },\n"
		"	{\n"
		"		{ vsParallelSpecular0Direction.xyz, vsParallelSpecular0Diffuse, vsParallelSpecular0Specular }\n"
		"	},\n"
		"	{\n"
		"		{ vsParallel0Direction.xyz, vsParallel0Diffuse },\n"
		"		{ vsParallel1Direction.xyz, vsParallel1Diffuse }\n"
		"	},\n"
		"	{\n"
		"		{ vsPointSpecular0Position.xyz, vsPointSpecular0Diffuse, vsPointSpecular0Attenuation, vsPointSpecular0Specular }\n"
		"	},\n"
		"	{\n"
		"		{ vsPoint0Position.xyz, vsPoint0Diffuse, vsPoint0Attenuation },\n"
		"		{ vsPoint1Position.xyz, vsPoint1Diffuse, vsPoint1Attenuation },\n"
		"		{ vsPoint2Position.xyz, vsPoint2Diffuse, vsPoint2Attenuation },\n"
		"		{ vsPoint3Position.xyz, vsPoint3Diffuse, vsPoint3Attenuation }\n"
		"	},\n"
		"	{\n"
		"		{ vsDot30CameraPosition.xyz, vsDot30Direction.xyz, vsDot30Diffuse, vsDot30Specular }\n"
		"	}\n"
		"};\n"
		"\n"
		// The light-enable booleans. The shipped include binds these to b0..b7, and nothing
		// in the engine ever writes a boolean constant register -- the BOOL overload of
		// Direct3d9_StateCache::setVertexShaderConstants has no callers anywhere -- so in
		// the shipping client they are all false. Declaring them as false constants
		// reproduces that exactly. The earlier DX11 attempt set them true, which inverts it.
		//
		// They are not unread, though, and an earlier version of this comment claimed they
		// were. The BASE vertex_program/modules/diffuse.inc gates every diffuse light term
		// behind `if cLightData_..._enabled` / `endif` -- assembly static branching -- so
		// with all eight false the base assembly path contributes no diffuse lighting at
		// all. That is very likely what the two inherited lighting patches (c_ambient.inc
		// and the 0.85 diffuse floor) are compensating for. No HLSL program reads them, and
		// the copy of diffuse.inc that actually wins resolution here is ILM_visuals.tre's,
		// which removed the branches -- which is why a search of the resolved corpus finds
		// no readers. Writing the booleans properly is the obvious candidate fix once
		// parity has been established, and is a strictly better answer than a tuned floor.
		"static const bool light_parallelSpecular_0_enabled = false;\n"
		"static const bool light_parallel_0_enabled         = false;\n"
		"static const bool light_parallel_1_enabled         = false;\n"
		"static const bool light_pointSpecular_0_enabled    = false;\n"
		"static const bool light_point_0_enabled            = false;\n"
		"static const bool light_point_1_enabled            = false;\n"
		"static const bool light_point_2_enabled            = false;\n"
		"static const bool light_point_3_enabled            = false;\n";
		// The shipped include ends with
		//     #pragma def(vs, c95, 0.0, 0.5f, 1.0f, 1.4426950408889634f)
		// a D3DX assembler directive that preloads c95. Only the ASSEMBLY vertex programs
		// read it, through registers.inc's `#define c0_0 c95.x` and friends; no HLSL
		// program mentions c95 at all. Omitted rather than translated -- when the assembly
		// programs are translated they will carry those four values as literals instead of
		// depending on an upload.

	// ------------------------------------------------------------------

	char const * const cms_pixelConstantsPath  = "pixel_program/include/pixel_shader_constants.inc";
	char const * const cms_vertexConstantsPath = "vertex_program/include/vertex_shader_constants.inc";
	char const * const cms_ambientPath         = "c_ambient.inc";

	char const cms_ambientSearch[]  = "mov r7, vColor0";
	char const cms_ambientReplace[] = "add r7, vColor0, c16";

	char const cms_diffuseSearch[]  = "lightData.ambient.ambientColor + diffuseSpecular.diffuse";
	char const cms_diffuseReplace[] = "max(lightData.ambient.ambientColor + diffuseSpecular.diffuse, 0.85)";

	D3D_SHADER_MACRO const cms_pointRename = { "point", "_pt_lights" };

	// ------------------------------------------------------------------
	/**
	 * Replace every occurrence of one string with another, or return null when there are
	 * none. Length is carried explicitly because shader text is not guaranteed to be
	 * NUL terminated when it comes out of a TRE.
	 */

	char *replaceAll(char const *source, int length, char const *search, int searchLength, char const *replace, int replaceLength, int &resultLength)
	{
		int occurrences = 0;
		for (int i = 0; i + searchLength <= length; )
		{
			if (memcmp(source + i, search, static_cast<size_t>(searchLength)) == 0)
			{
				++occurrences;
				i += searchLength;
			}
			else
			{
				++i;
			}
		}

		if (!occurrences)
			return NULL;

		int const growth = occurrences * (replaceLength - searchLength);
		char * const result = new char[length + growth];

		char *destination = result;
		for (int i = 0; i < length; )
		{
			if (i + searchLength <= length && memcmp(source + i, search, static_cast<size_t>(searchLength)) == 0)
			{
				memcpy(destination, replace, static_cast<size_t>(replaceLength));
				destination += replaceLength;
				i += searchLength;
			}
			else
			{
				*destination++ = source[i++];
			}
		}

		resultLength = static_cast<int>(destination - result);
		return result;
	}

	// ------------------------------------------------------------------

	bool endsWith(char const *text, char const *suffix)
	{
		size_t const textLength = strlen(text);
		size_t const suffixLength = strlen(suffix);
		if (suffixLength > textLength)
			return false;
		return _stricmp(text + textLength - suffixLength, suffix) == 0;
	}

	// The engine's vertex register assignment. Registers 1 and 2 are absent on purpose:
	// nothing binds them, which is why a program asking for v1 is reported.
	struct RegisterSemantic
	{
		int          reg;
		char const  *semantic;
	};

	RegisterSemantic const cms_registerSemantics[] =
	{
		{  0, "POSITION0"  },
		{  3, "NORMAL0"    },
		{  4, "PSIZE0"     },
		{  5, "COLOR0"     },
		{  6, "COLOR1"     },
		{  7, "TEXCOORD0"  },
		{  8, "TEXCOORD1"  },
		{  9, "TEXCOORD2"  },
		{ 10, "TEXCOORD3"  },
		{ 11, "TEXCOORD4"  },
		{ 12, "TEXCOORD5"  },
		{ 13, "TEXCOORD6"  },
		{ 14, "TEXCOORD7"  }
	};

	char const *getSemanticForRegister(int reg)
	{
		for (size_t i = 0; i < sizeof(cms_registerSemantics) / sizeof(cms_registerSemantics[0]); ++i)
			if (cms_registerSemantics[i].reg == reg)
				return cms_registerSemantics[i].semantic;

		return NULL;
	}

	bool isIdentifierCharacter(char c)
	{
		return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
	}

	// ------------------------------------------------------------------
	// Patch 8: the pixel epilogue -- the alpha test.
	//
	// D3D9's ALPHATESTENABLE/ALPHAFUNC/ALPHAREF have no D3D11 equivalent whatsoever; the
	// test has to become a discard inside the pixel shader. Doing that with a compiled
	// variant per compare function would reintroduce exactly the specialisation this port
	// went to some trouble to remove, so it is driven from a constant instead.
	//
	// Six of the engine's eight compare functions reduce to a single multiply-add against
	// the alpha, which is why the constant is a (scale, bias) pair rather than a function
	// index and a reference:
	//
	//   Always          scale  0, bias  +1     clip(+1), never discards
	//   Never           scale  0, bias  -1     clip(-1), always discards
	//   GreaterOrEqual  scale +1, bias  -r     keeps a >= r
	//   Greater         scale +1, bias  -(r+e) keeps a >  r
	//   LessOrEqual     scale -1, bias  +r     keeps a <= r
	//   Less            scale -1, bias  +(r-e) keeps a <  r
	//
	// e is one step of an eight bit alpha, because that is the precision D3D9 compared at.
	// Equal and NotEqual cannot be written this way; they are also not something an alpha
	// test is ever used for, and the backend reports it rather than approximating if one
	// ever appears.
	//
	// So the cost is one mad and one clip per pixel, with no branch and no variant. An
	// explicit cbuffer at b1 keeps this out of $Globals, whose byte offsets are the register
	// ABI every shader depends on -- verified: the /Gec register(cN) globals still land at
	// 16N with this present.

	char const cms_pixelEpiloguePrologue[] =
		"cbuffer SwgPixelEpilogue : register(b1)\n"
		"{\n"
		"\tfloat4 swgAlphaTest;\n"
		"};\n"
		"float4 swgPixelEpilogue(float4 swgColour)\n"
		"{\n"
		"\tclip(swgColour.a * swgAlphaTest.x + swgAlphaTest.y);\n"
		"\treturn swgColour;\n"
		"}\n";

	// ------------------------------------------------------------------
	/**
	 * Wrap every return in the pixel entry point with the epilogue.
	 *
	 * The entry's signature is deliberately left alone. Every one of the 386 HLSL pixel
	 * programs returns float4 with a COLOR semantic, but their parameter lists vary from no
	 * parameters to ten, and 61 take a single struct while 323 take loose semantic
	 * parameters -- so a wrapper that had to reproduce a parameter list would be the fragile
	 * part of this. Rewriting the returns needs none of that, and it does not change the
	 * shader's inputs at all, which matters because a pixel input has to be satisfiable by
	 * whatever vertex program is paired with it.
	 */

	char *injectPixelEpilogue(char const *name, char const *source, int length, int &resultLength)
	{
		// Find "main", then its parameter list, then its body.
		char const *entry = NULL;
		for (int i = 0; i + 4 <= length; ++i)
		{
			if (memcmp(source + i, "main", 4) != 0)
				continue;

			bool const startBoundary = (i == 0) || !isIdentifierCharacter(source[i - 1]);
			bool const endBoundary = (i + 4 >= length) || !isIdentifierCharacter(source[i + 4]);
			if (!startBoundary || !endBoundary)
				continue;

			// Skip whitespace and require an opening parenthesis: this is the entry point
			// rather than a mention of the word.
			int j = i + 4;
			while (j < length && (source[j] == ' ' || source[j] == '\t' || source[j] == '\n' || source[j] == '\r'))
				++j;

			if (j < length && source[j] == '(')
			{
				entry = source + i;
				break;
			}
		}

		if (!entry)
		{
			WARNING(true, ("Direct3d11: '%s' has no recognisable main, so the alpha test epilogue could not be injected. Alpha-tested pixels will not be discarded.", name));
			return NULL;
		}

		// Walk to the body's opening brace, then find its matching close.
		int position = static_cast<int>(entry - source);
		int depth = 0;
		int bodyStart = -1;
		int bodyEnd = -1;

		for (int i = position; i < length; ++i)
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
					break;
				}
			}
		}

		if (bodyStart < 0 || bodyEnd < 0)
		{
			WARNING(true, ("Direct3d11: '%s' has a main whose body could not be delimited, so the alpha test epilogue could not be injected.", name));
			return NULL;
		}

		// Worst case: every return grows by the call text, plus the prologue.
		int const prologueLength = static_cast<int>(sizeof(cms_pixelEpiloguePrologue) - 1);
		char const openText[] = "swgPixelEpilogue(";
		int const openLength = static_cast<int>(sizeof(openText) - 1);

		int returnCount = 0;
		for (int i = bodyStart; i < bodyEnd; ++i)
		{
			if (memcmp(source + i, "return", 6) != 0 || i + 6 > bodyEnd)
				continue;
			bool const startBoundary = !isIdentifierCharacter(source[i - 1]);
			bool const endBoundary = !isIdentifierCharacter(source[i + 6]);
			if (startBoundary && endBoundary)
				++returnCount;
		}

		if (!returnCount)
		{
			WARNING(true, ("Direct3d11: '%s' has a main with no return statement, so the alpha test epilogue could not be injected.", name));
			return NULL;
		}

		char * const result = new char[prologueLength + length + (returnCount * (openLength + 1)) + 1];
		char *destination = result;

		memcpy(destination, cms_pixelEpiloguePrologue, static_cast<size_t>(prologueLength));
		destination += prologueLength;

		int i = 0;
		while (i < length)
		{
			bool rewrote = false;

			if (i >= bodyStart && i < bodyEnd && i + 6 <= length && memcmp(source + i, "return", 6) == 0)
			{
				bool const startBoundary = (i == 0) || !isIdentifierCharacter(source[i - 1]);
				bool const endBoundary = (i + 6 >= length) || !isIdentifierCharacter(source[i + 6]);

				if (startBoundary && endBoundary)
				{
					// Take the expression up to its terminating semicolon. An HLSL return
					// expression cannot itself contain one.
					int end = i + 6;
					while (end < length && source[end] != ';')
						++end;

					if (end < length)
					{
						memcpy(destination, "return ", 7);
						destination += 7;
						memcpy(destination, openText, static_cast<size_t>(openLength));
						destination += openLength;

						int const expressionLength = end - (i + 6);
						memcpy(destination, source + i + 6, static_cast<size_t>(expressionLength));
						destination += expressionLength;

						*destination++ = ')';
						*destination++ = ';';

						i = end + 1;
						rewrote = true;
					}
				}
			}

			if (!rewrote)
				*destination++ = source[i++];
		}

		resultLength = static_cast<int>(destination - result);
		return result;
	}

}
using namespace Direct3d11_ShaderSourceNamespace;

// ======================================================================

D3D_SHADER_MACRO const &Direct3d11_ShaderSource::getPointRenameMacro()
{
	return cms_pointRename;
}

// ======================================================================
/**
 * Which language does this program declare?
 *
 * The first non-blank line is either "//hlsl <profile>", "//asm <profile>", or -- for
 * most of the pixel programs -- a bare "ps.1.1" style version directive, which opens an
 * assembly program with no marker comment at all. Anything else is unknown, and the
 * caller should say so rather than guess.
 */

Direct3d11_ShaderSource::Language Direct3d11_ShaderSource::getLanguage(char const *source, int sourceLength)
{
	NOT_NULL(source);

	int i = 0;
	while (i < sourceLength && (source[i] == ' ' || source[i] == '\t' || source[i] == '\r' || source[i] == '\n'))
		++i;

	int const remaining = sourceLength - i;
	char const * const line = source + i;

	if (remaining >= 6 && memcmp(line, "//hlsl", 6) == 0)
		return L_hlsl;

	if (remaining >= 5 && memcmp(line, "//asm", 5) == 0)
		return L_assembly;

	// "ps.1.1", "ps.1.4", "vs.1.1" and friends.
	if (remaining >= 3 && (memcmp(line, "ps.", 3) == 0 || memcmp(line, "vs.", 3) == 0))
		return L_assembly;

	return L_unknown;
}

// ======================================================================

char const *Direct3d11_ShaderSource::getIncludeOverride(char const *path, int &length)
{
	NOT_NULL(path);

	if (endsWith(path, cms_pixelConstantsPath))
	{
		length = static_cast<int>(sizeof(cms_pixelShaderConstants) - 1);
		return cms_pixelShaderConstants;
	}

	if (endsWith(path, cms_vertexConstantsPath))
	{
		length = static_cast<int>(sizeof(cms_vertexShaderConstants) - 1);
		return cms_vertexShaderConstants;
	}

	length = 0;
	return NULL;
}

// ----------------------------------------------------------------------

char *Direct3d11_ShaderSource::patchIncludeContents(char const *path, char const *contents, int length, int &patchedLength)
{
	NOT_NULL(path);
	NOT_NULL(contents);

	if (endsWith(path, cms_ambientPath))
	{
		return replaceAll(contents, length,
			cms_ambientSearch, static_cast<int>(sizeof(cms_ambientSearch) - 1),
			cms_ambientReplace, static_cast<int>(sizeof(cms_ambientReplace) - 1),
			patchedLength);
	}

	if (endsWith(path, cms_pixelFunctionsPath))
	{
		int const prologueLength = static_cast<int>(sizeof(cms_pixelFunctionsPrologue) - 1);
		char * const result = new char[prologueLength + length];
		memcpy(result, cms_pixelFunctionsPrologue, static_cast<size_t>(prologueLength));
		memcpy(result + prologueLength, contents, static_cast<size_t>(length));
		patchedLength = prologueLength + length;
		return result;
	}

	return NULL;
}

// ======================================================================
/**
 * Patch a program's own source.
 *
 * Two edits, applied in this order because the second can lengthen lines the first
 * searches: the diffuse floor, then the vertex register strip.
 */

char *Direct3d11_ShaderSource::patchProgramSource(char const *name, char const *source, int length, bool isVertexProgram, int &patchedLength)
{
	NOT_NULL(source);

	char *current = NULL;
	int currentLength = length;

	{
		int replacedLength = 0;
		char * const replaced = replaceAll(source, length,
			cms_diffuseSearch, static_cast<int>(sizeof(cms_diffuseSearch) - 1),
			cms_diffuseReplace, static_cast<int>(sizeof(cms_diffuseReplace) - 1),
			replacedLength);

		if (replaced)
		{
			current = replaced;
			currentLength = replacedLength;
		}
	}

	if (isVertexProgram)
	{
		char const * const scanSource = current ? current : source;

		// Rewrite in place is impossible -- the replacement is never longer than the text
		// it replaces, but it is easier to reason about a fresh buffer, and this runs once
		// per program per texture-coordinate key, not per frame.
		char * const stripped = new char[currentLength];
		char *destination = stripped;

		int i = 0;
		while (i < currentLength)
		{
			// Look for ": register(vN)" and, if the preceding token was a semantic, drop
			// the clause and keep the semantic the register implies.
			if (scanSource[i] == ':')
			{
				int j = i + 1;
				while (j < currentLength && (scanSource[j] == ' ' || scanSource[j] == '\t'))
					++j;

				if (j + 8 < currentLength && memcmp(scanSource + j, "register", 8) == 0)
				{
					int k = j + 8;
					while (k < currentLength && (scanSource[k] == ' ' || scanSource[k] == '\t'))
						++k;

					if (k < currentLength && scanSource[k] == '(')
					{
						++k;
						while (k < currentLength && (scanSource[k] == ' ' || scanSource[k] == '\t'))
							++k;

						if (k < currentLength && (scanSource[k] == 'v' || scanSource[k] == 'V'))
						{
							++k;
							int reg = 0;
							bool digits = false;
							while (k < currentLength && scanSource[k] >= '0' && scanSource[k] <= '9')
							{
								reg = (reg * 10) + (scanSource[k] - '0');
								digits = true;
								++k;
							}
							while (k < currentLength && (scanSource[k] == ' ' || scanSource[k] == '\t'))
								++k;

							if (digits && k < currentLength && scanSource[k] == ')')
							{
								// Recover the semantic already written before this colon,
								// so a disagreement can be reported and corrected.
								int end = i - 1;
								while (end >= 0 && (scanSource[end] == ' ' || scanSource[end] == '\t'))
									--end;
								int start = end;
								while (start >= 0 && isIdentifierCharacter(scanSource[start]))
									--start;
								++start;

								char semantic[64];
								int const semanticLength = (end - start + 1 > 0 && end - start + 1 < isizeof(semantic) - 1) ? (end - start + 1) : 0;
								if (semanticLength)
								{
									memcpy(semantic, scanSource + start, static_cast<size_t>(semanticLength));
									semantic[semanticLength] = 0;
								}
								else
								{
									semantic[0] = 0;
								}

								char const * const wanted = getSemanticForRegister(reg);

								if (!wanted)
								{
									WARNING(true, ("Direct3d11: '%s' binds %s to vertex register v%d, which the engine never writes; DX9 read undefined data there. Leaving the semantic as declared.",
										name, semantic[0] ? semantic : "an input", reg));
								}
								else if (semantic[0] && _stricmp(semantic, wanted) != 0)
								{
									// The register is what DX9 honoured, so it wins.
									WARNING(true, ("Direct3d11: '%s' declares %s but binds vertex register v%d; using %s to match the register, as DX9 did.",
										name, semantic, reg, wanted));

									// Back up over everything already emitted from the
									// start of the semantic to just before this colon --
									// the semantic AND any whitespace after it -- then
									// write the corrected name. Backing up by the
									// semantic's length alone would eat the whitespace and
									// leave the old name in place.
									size_t const wantedLength = strlen(wanted);

									// Every real disagreement in the corpus is one
									// TEXCOORDn for another, so the corrected name is the
									// same length as what it replaces and the buffer cannot
									// grow. Stated rather than assumed, because a longer
									// replacement would need the clause's own slack
									// accounted for.
									DEBUG_FATAL(static_cast<int>(wantedLength) > (i - start), ("Direct3d11: correcting %s to %s in '%s' would lengthen the source.", semantic, wanted, name));

									destination -= (i - start);
									memcpy(destination, wanted, wantedLength);
									destination += wantedLength;
								}

								// Skip the whole ": register(vN)" clause.
								i = k + 1;
								continue;
							}
						}
					}
				}
			}

			*destination++ = scanSource[i++];
		}

		delete [] current;
		current = stripped;
		currentLength = static_cast<int>(destination - stripped);
	}

	if (!isVertexProgram)
	{
		// Patch 8: the alpha test epilogue. Applied last so that it wraps whatever the
		// earlier rewrites produced rather than being rewritten by them.
		char const * const scanSource = current ? current : source;

		int injectedLength = 0;
		char * const injected = injectPixelEpilogue(name, scanSource, currentLength, injectedLength);
		if (injected)
		{
			delete [] current;
			current = injected;
			currentLength = injectedLength;
		}
	}

	if (!current)
		return NULL;

	patchedLength = currentLength;
	return current;
}

// ======================================================================
