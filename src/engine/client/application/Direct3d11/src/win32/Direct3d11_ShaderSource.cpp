// ======================================================================
//
// Direct3d11_ShaderSource.cpp
// copyright (c) 2026 Galaxies Reborn
//
// ======================================================================

#include "FirstDirect3d11.h"
#include "Direct3d11_ShaderSource.h"

#include "Direct3d11_ShaderSignature.h"

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
		// Feature marker, mirroring D3D11_PIXEL_SHADER_CONSTANTS in the pixel include:
		// lets a shader source that must compile on more than one backend detect that
		// THIS include already declares the extended blocks (notably the hemispheric
		// c60..c63 registers) and alias them instead of redeclaring the registers --
		// a raw redeclaration is X4019 here and correct on a backend whose include
		// stops at c59.
		"#define D3D11_VERTEX_SHADER_CONSTANTS\n"
		"\n"
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
		// The hemispheric block at c60..c63, which the light manager fills from
		// Direct3d11_LightManager::HemisphericLightData. Field ORDER here is that struct's
		// order and must stay so -- it is a register layout, not a naming choice.
		//
		// The HLSL names are not the C++ names: the shipped include calls the last two
		// tangentColorMinusBackColor and tangentColorMinusDiffuseColor where the C++ struct says
		// tangentMinusBackColor and tangentMinusDiffuseColor. The asset's spelling wins, because
		// the asset is what the shipped functions.inc reads.
		"float4    vsExtendedParallelSpecular0BackColor                    : register(c60);\n"
		"float4    vsExtendedParallelSpecular0TangentColor                 : register(c61);\n"
		"float4    vsExtendedParallelSpecular0TangentColorMinusBackColor    : register(c62);\n"
		"float4    vsExtendedParallelSpecular0TangentColorMinusDiffuseColor : register(c63);\n"
		"\n"
		// float4, and the asset settles it in both directions.
		//
		// float3 looked right from calculateDiffuseParallelHemisphericLight, which does
		// "float3 color = extendedLight.tangentColor;". But
		// calculateDiffuseSpecularParallelHemisphericLight assigns that same field into
		// DiffuseSpecular::diffuse, which is a float4 -- it sets .a on it a few lines later --
		// and then does "diffuse += (-lighting.y * extendedLight.tangentColorMinusDiffuseColor)".
		// float4 += float3 is X3017, a hard error.
		//
		// So float4 is the only width that compiles both uses. The float3 assignment truncates,
		// which is X3206, a warning this include already produces in eight other places and which
		// the shipped build lived with. A warning in one function beats an error in another.
		"struct HemisphericLightData\n"
		"{\n"
		"	float4 backColor;\n"
		"	float4 tangentColor;\n"
		"	float4 tangentColorMinusBackColor;\n"
		"	float4 tangentColorMinusDiffuseColor;\n"
		"};\n"
		"\n"
		"struct ExtendedLightData\n"
		"{\n"
		"	HemisphericLightData parallelSpecular[NumberOfParallelSpecularLights];\n"
		"};\n"
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
		"static ExtendedLightData extendedLightData =\n"
		"{\n"
		"	{\n"
		"		{\n"
		"			vsExtendedParallelSpecular0BackColor,\n"
		"			vsExtendedParallelSpecular0TangentColor,\n"
		"			vsExtendedParallelSpecular0TangentColorMinusBackColor,\n"
		"			vsExtendedParallelSpecular0TangentColorMinusDiffuseColor\n"
		"		}\n"
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

	char const *const cms_pixelConstantsPath = "pixel_program/include/pixel_shader_constants.inc";
	char const *const cms_vertexConstantsPath = "vertex_program/include/vertex_shader_constants.inc";
	char const *const cms_ambientPath = "c_ambient.inc";

	char const cms_ambientSearch[] = "mov r7, vColor0";
	char const cms_ambientReplace[] = "add r7, vColor0, c16";

	char const cms_diffuseSearch[] = "lightData.ambient.ambientColor + diffuseSpecular.diffuse";
	char const cms_diffuseReplace[] = "max(lightData.ambient.ambientColor + diffuseSpecular.diffuse, 0.85)";

	D3D_SHADER_MACRO const cms_pointRename = {"point", "_pt_lights"};

	// ------------------------------------------------------------------
	/**
	 * Replace every occurrence of one string with another, or return null when there are
	 * none. Length is carried explicitly because shader text is not guaranteed to be
	 * NUL terminated when it comes out of a TRE.
	 */

	char *replaceAll(char const *source, int length, char const *search, int searchLength, char const *replace, int replaceLength, int &resultLength)
	{
		int occurrences = 0;
		for (int i = 0; i + searchLength <= length;)
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
		char *const result = new char[length + growth];

		char *destination = result;
		for (int i = 0; i < length;)
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
		int reg;
		char const *semantic;
	};

	RegisterSemantic const cms_registerSemantics[] =
		{
			{0, "POSITION0"},
			{3, "NORMAL0"},
			{4, "PSIZE0"},
			{5, "COLOR0"},
			{6, "COLOR1"},
			{7, "TEXCOORD0"},
			{8, "TEXCOORD1"},
			{9, "TEXCOORD2"},
			{10, "TEXCOORD3"},
			{11, "TEXCOORD4"},
			{12, "TEXCOORD5"},
			{13, "TEXCOORD6"},
			{14, "TEXCOORD7"}};

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

	// ------------------------------------------------------------------
	// Fog, which shares the epilogue because it is the other thing D3D9 did after the pixel
	// shader and D3D11 does not do at all.
	//
	// D3D9's fog blender is a fixed-function stage: FOGENABLE on, the factor taken from the
	// vertex shader's oFog output, blended against FOGCOLOR before alpha blending. It survived
	// into the VSPS path -- DX9 sets FOGVERTEXMODE to NONE on every draw, but that only disables
	// the fixed-function COMPUTATION of the factor, not the blend, and the corpus computes the
	// factor itself: 179 of the 192 HLSL vertex programs call calculateFog and write the result
	// to a FOG output.
	//
	// So the vertex half already works and the pixel half was missing entirely. Adding it costs
	// one lerp, and an input the entry point did not have.
	//
	// The factor's sense is D3D9's: calculateFog returns exp(-distanceSquared * density^2),
	// which is 1 at the camera and falls to 0 with distance, so the blend keeps the surface
	// colour where the factor is 1. Only rgb is touched -- D3D9's fog never affected alpha.
	//
	// swgFogColor.a carries the enable rather than a separate constant, so that disabling fog
	// forces the factor to 1 and the lerp becomes a no-op the driver can fold away. That also
	// fixes the previous setFog, which returned early when disabled and left the last density
	// in c10 behind.

	char const cms_pixelEpiloguePrologue[] =
		"cbuffer SwgPixelEpilogue : register(b1)\n"
		"{\n"
		"\tfloat4 swgAlphaTest;\n"
		"\tfloat4 swgFogColor;\n"
		"};\n"
		"float4 swgPixelEpilogue(float4 swgColour, float swgFog)\n"
		"{\n"
		"\tclip(swgColour.a * swgAlphaTest.x + swgAlphaTest.y);\n"
		"\tfloat swgFactor = lerp(1.0f, saturate(swgFog), swgFogColor.a);\n"
		"\tswgColour.rgb = lerp(swgFogColor.rgb, swgColour.rgb, swgFactor);\n"
		"\treturn swgColour;\n"
		"}\n";

	// The name of the fog input this backend adds when a program has none of its own.
	char const cms_injectedFogName[] = "swgFogFactor";

	// ------------------------------------------------------------------
	/**
	 * Read an identifier backwards from just past its last character.
	 *
	 * Used to recover a parameter's name from the text before its semantic colon, and a
	 * function's return type from the text before its name.
	 */

	bool readIdentifierBackwards(char const *source, int end, int &identifierStart, int &identifierEnd)
	{
		int i = end;
		while (i > 0 && (source[i - 1] == ' ' || source[i - 1] == '\t' || source[i - 1] == '\n' || source[i - 1] == '\r'))
			--i;

		identifierEnd = i;

		while (i > 0 && isIdentifierCharacter(source[i - 1]))
			--i;

		identifierStart = i;
		return identifierEnd > identifierStart;
	}

	// ------------------------------------------------------------------
	/**
	 * Find the entry point's name, and the parentheses around its parameter list.
	 *
	 * "main" followed by an opening parenthesis, which distinguishes the entry point from a
	 * mention of the word in a comment or an identifier that contains it.
	 */

	bool findEntryPoint(char const *source, int length, int &nameStart, int &parameterOpen, int &parameterClose)
	{
		// Comments are skipped, and that is not defensive tidiness. Every one of the 97 programs
		// this port converted from assembly carries the line
		//
		//   // original is preserved: each #include below is a block of statements inside main(),
		//
		// in which "main" is followed by a parenthesis and preceded by the word "inside". A scan
		// that ignored comments took that as the entry point and "inside" as its return type.
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
				if (source[i + 1] == '/')
				{
					inLineComment = true;
					++i;
					continue;
				}
				if (source[i + 1] == '*')
				{
					inBlockComment = true;
					++i;
					continue;
				}
			}

			if (memcmp(source + i, "main", 4) != 0)
				continue;

			bool const startBoundary = (i == 0) || !isIdentifierCharacter(source[i - 1]);
			bool const endBoundary = (i + 4 >= length) || !isIdentifierCharacter(source[i + 4]);
			if (!startBoundary || !endBoundary)
				continue;

			int j = i + 4;
			while (j < length && (source[j] == ' ' || source[j] == '\t' || source[j] == '\n' || source[j] == '\r'))
				++j;

			if (j >= length || source[j] != '(')
				continue;

			// Match the parenthesis. A parameter list can nest them in a default value or an
			// array bound, so this counts rather than scanning for the first close.
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

	// ------------------------------------------------------------------
	/**
	 * Find the braces around a body, starting the search at a given position.
	 */

	bool findBody(char const *source, int length, int from, int &bodyStart, int &bodyEnd)
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

	// ------------------------------------------------------------------
	/**
	 * Whether a range contains a FOG semantic, and if so what the thing carrying it is called.
	 *
	 * Semantics are case insensitive in HLSL, so this is too. The name is what precedes the
	 * colon, which for a parameter list is the parameter and for a struct member is the member.
	 */

	bool findFogSemantic(char const *source, int from, int to, int &nameStart, int &nameEnd)
	{
		for (int i = from; i + 1 < to; ++i)
		{
			if (source[i] != ':')
				continue;

			int j = i + 1;
			while (j < to && (source[j] == ' ' || source[j] == '\t' || source[j] == '\n' || source[j] == '\r'))
				++j;

			if (j + 3 > to)
				continue;

			if (!((source[j] == 'F' || source[j] == 'f') &&
				  (source[j + 1] == 'O' || source[j + 1] == 'o') &&
				  (source[j + 2] == 'G' || source[j + 2] == 'g')))
				continue;

			// FOG exactly, not FOGSOMETHING. A trailing index would be a different semantic.
			if (j + 3 < to && isIdentifierCharacter(source[j + 3]))
				continue;

			if (readIdentifierBackwards(source, i, nameStart, nameEnd))
				return true;
		}

		return false;
	}

	// ------------------------------------------------------------------
	/**
	 * Wrap every return in the pixel entry point with the epilogue, and give it a fog input.
	 *
	 * The returns are rewritten rather than the entry being wrapped in a new function. Every one
	 * of the HLSL pixel programs returns float4 with a COLOR semantic, but their parameter lists
	 * run from none to ten, so a wrapper that had to reproduce a parameter list would be the
	 * fragile part of this. Rewriting returns needs none of that.
	 *
	 * The signature is touched in exactly one way: a fog input is added when the program does
	 * not already have one. Two of the 325 do -- water_pass1 and water_pass2_ps20 -- and those
	 * keep theirs, since a second FOG semantic would not compile.
	 *
	 * That input is the reason injectFogOutput exists. D3D11 requires a pixel shader's input
	 * signature to be a subset of the vertex shader's output signature, and a pixel program is
	 * paired with different vertex programs by different effects -- so a FOG input here is only
	 * safe if EVERY vertex program emits FOG. 179 of 192 already do; the other 13 have one
	 * added.
	 */

	char *injectPixelEpilogue(char const *name, char const *source, int length, int &resultLength)
	{
		int nameStart = 0;
		int parameterOpen = 0;
		int parameterClose = 0;

		if (!findEntryPoint(source, length, nameStart, parameterOpen, parameterClose))
		{
			WARNING(true, ("Direct3d11: '%s' has no recognisable main, so the alpha test and fog epilogue could not be injected.", name));
			return NULL;
		}

		int bodyStart = -1;
		int bodyEnd = -1;

		if (!findBody(source, length, parameterClose, bodyStart, bodyEnd))
		{
			WARNING(true, ("Direct3d11: '%s' has a main whose body could not be delimited, so the alpha test and fog epilogue could not be injected.", name));
			return NULL;
		}

		// Does it already take a fog input? If so its name is what the epilogue call passes,
		// and nothing is added to the parameter list.
		int fogNameStart = 0;
		int fogNameEnd = 0;
		bool const hasOwnFog = findFogSemantic(source, parameterOpen, parameterClose, fogNameStart, fogNameEnd);

		char fogName[64];
		if (hasOwnFog)
		{
			int const fogNameLength = fogNameEnd - fogNameStart;
			DEBUG_FATAL(fogNameLength <= 0 || fogNameLength >= isizeof(fogName), ("Direct3d11: '%s' has a fog parameter whose name is %d characters.", name, fogNameLength));
			memcpy(fogName, source + fogNameStart, static_cast<size_t>(fogNameLength));
			fogName[fogNameLength] = '\0';
		}
		else
		{
			strcpy(fogName, cms_injectedFogName);
		}

		// Whether the parameter list has anything in it decides whether the inserted parameter
		// needs a leading comma. Two of the corpus programs take none at all.
		bool listIsEmpty = true;
		for (int i = parameterOpen + 1; i < parameterClose; ++i)
			if (source[i] != ' ' && source[i] != '\t' && source[i] != '\n' && source[i] != '\r')
			{
				listIsEmpty = false;
				break;
			}

		char parameterText[128];
		parameterText[0] = '\0';
		if (!hasOwnFog)
			sprintf(parameterText, "%sin float %s : FOG", listIsEmpty ? "" : ", ", cms_injectedFogName);

		int const parameterTextLength = static_cast<int>(strlen(parameterText));

		// Worst case: every return grows by the call text and the fog argument, plus the
		// prologue and the inserted parameter.
		int const prologueLength = static_cast<int>(sizeof(cms_pixelEpiloguePrologue) - 1);
		char const openText[] = "swgPixelEpilogue(";
		int const openLength = static_cast<int>(sizeof(openText) - 1);

		// ", " plus the name plus ")" plus ";" -- the closing text of a rewritten return.
		int const closeLength = 2 + static_cast<int>(strlen(fogName)) + 2;

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

		char *const result = new char[prologueLength + length + parameterTextLength + (returnCount * (openLength + closeLength)) + 1];
		char *destination = result;

		memcpy(destination, cms_pixelEpiloguePrologue, static_cast<size_t>(prologueLength));
		destination += prologueLength;

		int i = 0;
		while (i < length)
		{
			// The fog parameter goes in immediately before the parameter list's closing
			// parenthesis, which keeps it after any existing parameter and away from the
			// semantics on them.
			if (i == parameterClose && parameterTextLength)
			{
				memcpy(destination, parameterText, static_cast<size_t>(parameterTextLength));
				destination += parameterTextLength;
			}

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

						*destination++ = ',';
						*destination++ = ' ';

						size_t const fogNameLength = strlen(fogName);
						memcpy(destination, fogName, fogNameLength);
						destination += fogNameLength;

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

	// ------------------------------------------------------------------
	/**
	 * Give a vertex program a FOG output if it has none.
	 *
	 * Needed because the pixel epilogue reads one. D3D11 requires a pixel shader's input
	 * signature to be a subset of the bound vertex shader's output signature, and effects pair
	 * pixel and vertex programs freely, so the fog input is only safe once every vertex program
	 * emits fog.
	 *
	 * 179 of the 192 HLSL vertex programs already do. The 13 that do not are the passes that
	 * were never fogged in the first place -- 2d, 2d_texture, ui, ui_radar, the bloom mask, the
	 * screen shader, zwrite_mask, the two texture-renderer passes, dot3_terrain_imp1 and 2,
	 * membrane and saber_blade -- so they get a constant 1.0, which is the factor's
	 * no-fog value and leaves them looking exactly as they did.
	 *
	 * All 192 return a declared struct, with no out parameters anywhere in the corpus, so the
	 * member can be added to that struct and assigned through a temporary. The temporary is used
	 * rather than assigning through the returned expression because it works whatever that
	 * expression is, not only when it happens to be a local variable.
	 */

	char *injectFogOutput(char const *name, char const *source, int length, int &resultLength)
	{
		int nameStart = 0;
		int parameterOpen = 0;
		int parameterClose = 0;

		if (!findEntryPoint(source, length, nameStart, parameterOpen, parameterClose))
			return NULL;

		// The return type is the identifier before the entry point's name.
		int typeStart = 0;
		int typeEnd = 0;
		if (!readIdentifierBackwards(source, nameStart, typeStart, typeEnd))
			return NULL;

		int const typeLength = typeEnd - typeStart;

		char typeName[64];
		if (typeLength <= 0 || typeLength >= isizeof(typeName))
			return NULL;
		memcpy(typeName, source + typeStart, static_cast<size_t>(typeLength));
		typeName[typeLength] = '\0';

		// A void entry point would be writing through out parameters, which nothing in the
		// corpus does. Reported rather than guessed at, because the fix is a different shape.
		if (strcmp(typeName, "void") == 0)
		{
			WARNING(true, ("Direct3d11: vertex program '%s' returns void, so it must write its outputs through parameters. A fog output cannot be added this way and any pixel program paired with it will fail to bind.", name));
			return NULL;
		}

		// Find "struct <typeName>" and its braces.
		int structBodyStart = -1;
		int structBodyEnd = -1;

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

			if (findBody(source, length, j + typeLength, structBodyStart, structBodyEnd))
				break;
		}

		if (structBodyStart < 0 || structBodyEnd < 0)
		{
			WARNING(true, ("Direct3d11: vertex program '%s' returns '%s', whose declaration could not be found, so a fog output could not be added.", name, typeName));
			return NULL;
		}

		int bodyStart = -1;
		int bodyEnd = -1;
		if (!findBody(source, length, parameterClose, bodyStart, bodyEnd))
		{
			WARNING(true, ("Direct3d11: vertex program '%s' has a main whose body could not be delimited, so a fog output could not be added.", name));
			return NULL;
		}

		// Does the output struct already declare a fog member, and if so is it ever assigned?
		//
		// Declared-but-never-assigned is a real case, not a hypothetical: fxc drops an output
		// element that is never written, so such a program compiles with no FOG in its output
		// signature and every pixel program paired with it then fails to bind. Two of the
		// converted programs are like this. So a member that exists but is never written gets
		// the same constant the missing ones get, under its own name.
		int fogNameStart = 0;
		int fogNameEnd = 0;
		bool const hasMember = findFogSemantic(source, structBodyStart, structBodyEnd, fogNameStart, fogNameEnd);

		char memberName[64];
		if (hasMember)
		{
			int const memberNameLength = fogNameEnd - fogNameStart;
			if (memberNameLength <= 0 || memberNameLength >= isizeof(memberName))
				return NULL;
			memcpy(memberName, source + fogNameStart, static_cast<size_t>(memberNameLength));
			memberName[memberNameLength] = '\0';

			// Look for ".<member>" followed by a single '=' inside main. A compound assignment
			// or a comparison does not count as producing the value, but neither appears in the
			// corpus, and treating them as "not assigned" only adds a redundant store.
			char pattern[68];
			sprintf(pattern, ".%s", memberName);
			int const patternLength = static_cast<int>(strlen(pattern));

			for (int i = bodyStart; i + patternLength < bodyEnd; ++i)
			{
				if (memcmp(source + i, pattern, static_cast<size_t>(patternLength)) != 0)
					continue;
				if (isIdentifierCharacter(source[i + patternLength]))
					continue;

				int j = i + patternLength;
				while (j < bodyEnd && (source[j] == ' ' || source[j] == '\t'))
					++j;

				if (j < bodyEnd && source[j] == '=' && (j + 1 >= bodyEnd || source[j + 1] != '='))
					return NULL; // assigned; nothing to do
			}
		}
		else
		{
			strcpy(memberName, "swgFog");
		}

		char const memberText[] = "\tfloat swgFog : FOG;\n";
		int const memberLength = hasMember ? 0 : static_cast<int>(sizeof(memberText) - 1);

		// The replacement for "return <expression>;". A block, so it is valid anywhere the
		// original statement was -- including as the single statement of an unbraced if.
		char prefixText[128];
		sprintf(prefixText, "{ %s swgFogOut = ", typeName);

		char suffixText[128];
		sprintf(suffixText, "; swgFogOut.%s = 1.0f; return swgFogOut; }", memberName);

		int const prefixLength = static_cast<int>(strlen(prefixText));
		int const suffixLength = static_cast<int>(strlen(suffixText));

		int returnCount = 0;
		for (int i = bodyStart; i < bodyEnd; ++i)
		{
			if (memcmp(source + i, "return", 6) != 0 || i + 6 > bodyEnd)
				continue;
			if (isIdentifierCharacter(source[i - 1]) || isIdentifierCharacter(source[i + 6]))
				continue;
			++returnCount;
		}

		if (!returnCount)
		{
			WARNING(true, ("Direct3d11: vertex program '%s' has a main with no return statement, so a fog output could not be added.", name));
			return NULL;
		}

		char *const result = new char[length + memberLength + (returnCount * (prefixLength + suffixLength)) + 1];
		char *destination = result;

		int i = 0;
		while (i < length)
		{
			// The new member goes in just before the struct's closing brace.
			if (i == structBodyEnd)
			{
				memcpy(destination, memberText, static_cast<size_t>(memberLength));
				destination += memberLength;
			}

			bool rewrote = false;

			if (i >= bodyStart && i < bodyEnd && i + 6 <= length && memcmp(source + i, "return", 6) == 0)
			{
				bool const startBoundary = (i == 0) || !isIdentifierCharacter(source[i - 1]);
				bool const endBoundary = (i + 6 >= length) || !isIdentifierCharacter(source[i + 6]);

				if (startBoundary && endBoundary)
				{
					int end = i + 6;
					while (end < length && source[end] != ';')
						++end;

					if (end < length)
					{
						memcpy(destination, prefixText, static_cast<size_t>(prefixLength));
						destination += prefixLength;

						int const expressionLength = end - (i + 6);
						memcpy(destination, source + i + 6, static_cast<size_t>(expressionLength));
						destination += expressionLength;

						memcpy(destination, suffixText, static_cast<size_t>(suffixLength));
						destination += suffixLength;

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

} // namespace Direct3d11_ShaderSourceNamespace
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
	char const *const line = source + i;

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
		char *const result = new char[prologueLength + length];
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
		char *const replaced = replaceAll(source, length,
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
		char const *const scanSource = current ? current : source;

		// Rewrite in place is impossible -- the replacement is never longer than the text
		// it replaces, but it is easier to reason about a fresh buffer, and this runs once
		// per program per texture-coordinate key, not per frame.
		char *const stripped = new char[currentLength];
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

								char const *const wanted = getSemanticForRegister(reg);

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

		delete[] current;
		current = stripped;
		currentLength = static_cast<int>(destination - stripped);
	}

	// Patch 8: the canonical interpolant signature, applied last so it wraps whatever the earlier
	// rewrites produced rather than being rewritten by them.
	//
	// This replaces two earlier transforms and is simpler than either. D3D11 links the vertex and
	// pixel stages by REGISTER, not by semantic name, so two independently compiled programs
	// disagree about where a semantic lives and every draw is rejected -- which is what made the
	// first working run of this client render nothing at all. Direct3d11_ShaderSignature gives both
	// stages one shared declaration, so the registers agree by construction.
	//
	// The alpha test and fog blend go in the generated pixel wrapper, which is a better place than
	// where they used to be: rewriting every `return` inside the original entry point and threading
	// a FOG parameter into its signature. And the vertex side no longer needs a fog output injected
	// into its own struct, because the canonical struct always has one.
	{
		char const *const scanSource = current ? current : source;

		int wrappedLength = 0;
		char *wrapped = NULL;

		if (isVertexProgram)
		{
			wrapped = Direct3d11_ShaderSignature::wrapVertexProgram(name, scanSource, currentLength, wrappedLength);
		}
		else
		{
			// The epilogue's cbuffer and function have to be declared before the wrapper that calls
			// them, and the wrapper is appended at the end -- so the prologue goes in front of the
			// whole program first, and the wrap happens over the result.
			int const prologueLength = isizeof(cms_pixelEpiloguePrologue) - 1;

			char *const withPrologue = new char[prologueLength + currentLength + 1];
			memcpy(withPrologue, cms_pixelEpiloguePrologue, static_cast<size_t>(prologueLength));
			memcpy(withPrologue + prologueLength, scanSource, static_cast<size_t>(currentLength));
			withPrologue[prologueLength + currentLength] = '\0';

			wrapped = Direct3d11_ShaderSignature::wrapPixelProgram(name, withPrologue, prologueLength + currentLength, wrappedLength);

			delete[] withPrologue;
		}

		if (wrapped)
		{
			delete[] current;
			current = wrapped;
			currentLength = wrappedLength;
		}
		else
		{
			// Without a canonical signature the program cannot link to anything, so failing here is
			// terminal for it. wrapVertexProgram and wrapPixelProgram have already said why.
			delete[] current;
			return NULL;
		}
	}

	if (!current)
		return NULL;

	patchedLength = currentLength;
	return current;
}

// ======================================================================
