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
#include "clientGraphics/ShaderImplementation.h"
#include "sharedFile/TreeFile.h"

#include <d3d9.h>
#include <d3dx9.h>
#include <string.h>

// ======================================================================
// x64 character-rendering fix.
//
// The .psh files in our TRE set carry BOTH the HLSL source (PSRC chunk) and
// a precompiled ps_2_0 binary (PEXE chunk). The original engine ignored the
// source and used the PEXE blob directly. That precompiled blob is stale /
// mismatched relative to the (runtime-recompiled) vertex shaders, which is
// why customizable humanoid characters rendered dark even though the source
// math + our vertex-shader fixes say they should be lit.
//
// Recompiling the HLSL PSRC source fresh, with the same modern d3dx9 the
// vertex shaders already use, makes the two sides agree. Assembly pixel
// shaders (`//asm` / `ps.1.1`) keep using their PEXE blob - those work fine
// (e.g. a_simple.psh for NPC armor).
// ======================================================================

namespace Direct3d9_PixelShaderProgramDataNamespace
{
	// ==================================================================
	// x64 black-character fix - pixel_shader_constants.inc register layout.
	//
	// THE BUG: the TRE's `pixel_program/include/pixel_shader_constants.inc`
	// (the live copy is in patch_11_00.tre) declares the PS constant
	// registers in a layout that does NOT match the engine's hard-coded
	// `Direct3d9_PixelShaderConstantRegisters` enum.
	//
	//   TRE inc layout        engine PSCR_* enum / upload layout
	//   --------------        ----------------------------------
	//   c0 dot3LightDirection      c0 dot3LightDirection      (.w=specPower)
	//   c1 dot3LightDiffuseColor   c1 dot3LightDiffuseColor
	//   c2 dot3LightSpecularColor  c2 dot3LightSpecularColor
	//   c3 textureFactor   <--->   c3 dot3LightTangentMinusDiffuseColor
	//   c4 textureFactor2  <--->   c4 dot3LightTangentMinusBackColor
	//   c5 materialSpecular <-->   c5 textureFactor
	//   c6 materialSpecPower<-->   c6 textureFactor2
	//   c7 alphaFadeHolder <--->   c7 materialSpecularColor
	//   c8 userConstants[17]       c8 userConstants[17]
	//
	// The original engine never tripped on this because it used the
	// precompiled PEXE bytecode, which was built against the engine layout.
	// Once we recompile from PSRC source (the x64 character fix), the
	// recompiled shader reads `textureFactor` from c3 - but the engine's
	// Direct3d9_LightManager uploads `dot3LightTangentMinusDiffuseColor`
	// there, and that value is NEGATIVE (~-0.35) for the synthesised
	// hemispheric suns. h_specmap_bump_ps20.psh does
	//   result.rgb = diffuseColor * allDiffuseLight * textureFactor.rgb
	// so a negative textureFactor.rgb drives result.rgb negative -> the GPU
	// clamps it to 0 -> humanoid characters and clothing render solid black
	// (in-world AND in inventory icon viewers - same shader).
	//
	// THE FIX: when the compiler asks for pixel_shader_constants.inc, hand
	// back the canonical engine-layout version (identical to the
	// swg-main/serverdata copy, which matches the PSCR_* enum exactly)
	// instead of the mismatched TRE file. Every other include passes
	// through from the TRE unchanged. This makes recompiled-from-PSRC
	// shaders binary-compatible with the engine's constant uploads, exactly
	// like the PEXE blobs were.
	// ==================================================================
	char const ENGINE_PIXEL_SHADER_CONSTANTS_INC[] =
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
		"#define timeElapsed             packedRegister4.a\n";

	// ==================================================================
	// x64 black-face fix - texren_copy_c1a1.psh.
	//
	// Customizable face/head textures are baked at runtime by
	// BlueprintTextureRenderer, which composites layers (base skin, brows,
	// beard, freckles) into a render-target texture. Every layer is drawn
	// with texren_copy_c1a1.sht / texren_alphablend_c1a1.sht, and BOTH of
	// those .sht effects use the SAME pixel program: texren_copy_c1a1.psh.
	//
	// That .psh is a ps.1.1 ASSEMBLY shader:
	//     ps.1.1
	//     tex t0
	//     mul r0.rgb, t0, c2   +   mov r0.a, t0.a
	// i.e. result = sourceTexture * tint, where the tint is read from
	// constant register c2. But the engine uploads the texture factor (the
	// skin/feature tint) to c5 (PSCR_textureFactor), NOT c2. The ps.1.1
	// PEXE blob can't be recompiled (modern d3dx rejects ps_1_x asm), so it
	// kept reading c2 - which holds dot3LightSpecularColor (~0 in a texture
	// bake context). result = sourceTexture * 0 = BLACK -> every baked face
	// texture came out black -> humanoid faces render as a black mask while
	// the body/scalp (static textures, h_specmap_bump) render fine.
	//
	// Fix: substitute an HLSL ps_2_0 equivalent that reads `textureFactor`
	// from the engine-layout register (c5, via the pixel_shader_constants.inc
	// override above). Triggered by filename in the ctor below.
	// ==================================================================
	char const TEXREN_COPY_C1A1_PS20_HLSL[] =
		"#include \"pixel_program/include/pixel_shader_constants.inc\"\n"
		"sampler diffuseMap : register(s0);\n"
		"float4 main(float2 tcs_MAIN : TEXCOORD0) : COLOR\n"
		"{\n"
		"\tfloat4 t0 = tex2D(diffuseMap, tcs_MAIN);\n"
		"\tfloat4 result;\n"
		"\tresult.rgb = t0.rgb * textureFactor.rgb;\n"
		"\tresult.a   = t0.a;\n"
		"\treturn result;\n"
		"}\n";

	// Minimal D3DX include handler for the pixel-shader compile. The pixel
	// program includes (`pixel_program/include/*.inc`, `shared_program/*.inc`)
	// are plain text in the TRE; just open and hand them back - except for
	// pixel_shader_constants.inc, which we override (see note above).
	class PixelIncludeHandler : public ID3DXInclude
	{
	public:
		STDMETHOD(Open)(D3DXINCLUDE_TYPE, LPCSTR pFileName, LPCVOID, LPCVOID *ppData, UINT *pBytes)
		{
			// command-line-compiler style relative includes
			if (pFileName && strncmp(pFileName, "../../", 6) == 0)
				pFileName += 6;

			// x64 fix: override pixel_shader_constants.inc with the
			// engine-layout version so the recompiled shader's constant
			// registers line up with the engine's PSCR_* uploads.
			if (pFileName && strstr(pFileName, "pixel_shader_constants.inc"))
			{
				int const len = static_cast<int>(sizeof(ENGINE_PIXEL_SHADER_CONSTANTS_INC) - 1);
				char *buf = new char[len > 0 ? len : 1];
				memcpy(buf, ENGINE_PIXEL_SHADER_CONSTANTS_INC, len);
				*ppData = buf;
				*pBytes = static_cast<UINT>(len);
				return S_OK;
			}

			AbstractFile *file = TreeFile::open(pFileName, AbstractFile::PriorityData, true);
			if (!file)
				return E_FAIL;

			int const len = file->length();
			char *buf = new char[len > 0 ? len : 1];
			if (len > 0)
				file->read(buf, len);
			delete file;

			*ppData = buf;
			*pBytes = static_cast<UINT>(len);
			return S_OK;
		}

		STDMETHOD(Close)(LPCVOID pData)
		{
			delete [] const_cast<char *>(static_cast<char const *>(pData));
			return S_OK;
		}
	};
}

using namespace Direct3d9_PixelShaderProgramDataNamespace;

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

			bool created = false;

			// ---- x64 fix: recompile from HLSL PSRC source when available ----
			char const * const source = pixelShaderProgram.m_source;
			if (source && pixelShaderProgram.m_sourceLength > 0)
			{
				// Parse the leading "//hlsl ps_X_X" / "//asm ..." directive.
				char const * p = source;
				while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
					++p;

				bool isHlsl = false;
				char target[16] = "ps_2_0";
				if (strncmp(p, "//hlsl", 6) == 0)
				{
					isHlsl = true;
					p += 6;
					while (*p == ' ' || *p == '\t')
						++p;
					int ti = 0;
					while (*p && *p != ' ' && *p != '\t' && *p != '\r' && *p != '\n' && ti < 15)
						target[ti++] = *p++;
					target[ti] = '\0';
					if (ti == 0)
						strcpy(target, "ps_2_0");
				}

				// x64 black-face fix: the texren_copy_c1a1.psh face-bake shader
				// is ps.1.1 ASM that reads its tint from the wrong constant
				// register (c2, vs the engine's c5). It can't be recompiled as
				// asm, so substitute a hand-written HLSL ps_2_0 equivalent that
				// reads textureFactor from the engine-layout register. See the
				// TEXREN_COPY_C1A1_PS20_HLSL note above.
				bool useTexrenReplacement = false;
				{
					char const * const fileName = pixelShaderProgram.getFileName();
					if (!isHlsl && fileName && strstr(fileName, "texren_copy_c1a1"))
					{
						useTexrenReplacement = true;
						isHlsl = true;
						strcpy(target, "ps_2_0");
					}
				}

				if (isHlsl)
				{
					// The pixel-program includes (vertex_shader_constants-style)
					// can use `point` as a struct field which collides with the
					// modern HLSL `point` reserved word - prepend the same
					// #define the vertex-shader path uses. Harmless if unused.
					static char const PREFIX[] = "#define point _pt_lights\n";

					char const *compileText = NULL;   // text handed to the compiler
					int         compileLen  = 0;
					char       *ownedText   = NULL;   // non-NULL only when we allocated

					if (useTexrenReplacement)
					{
						compileText = TEXREN_COPY_C1A1_PS20_HLSL;
						compileLen  = static_cast<int>(sizeof(TEXREN_COPY_C1A1_PS20_HLSL) - 1);
					}
					else
					{
						int const prefixLen = static_cast<int>(sizeof(PREFIX) - 1);
						int const srcLen    = pixelShaderProgram.m_sourceLength;

						ownedText = new char[prefixLen + srcLen + 1];
						memcpy(ownedText, PREFIX, prefixLen);
						memcpy(ownedText + prefixLen, source, srcLen);
						ownedText[prefixLen + srcLen] = '\0';

						compileText = ownedText;
						compileLen  = prefixLen + srcLen;
					}

					PixelIncludeHandler includeHandler;
					ID3DXBuffer *compiledShader = NULL;
					ID3DXBuffer *errorMessages  = NULL;
					HRESULT const compileResult = D3DXCompileShader(
						compileText, compileLen,
						NULL, &includeHandler,
						"main", target, 0,
						&compiledShader, &errorMessages, NULL);

					// d3dx leaves the FPU in a bad state on some shaders.
					_clearfp();

					if (SUCCEEDED(compileResult) && compiledShader)
					{
						HRESULT const createResult = Direct3d9::getDevice()->CreatePixelShader(
							reinterpret_cast<DWORD const *>(compiledShader->GetBufferPointer()), &m_pixelShader);
						if (SUCCEEDED(createResult) && m_pixelShader)
						{
							created = true;
						}
						else
						{
							WARNING(true, ("Direct3d9_PixelShaderProgramData: CreatePixelShader failed for recompiled %s (0x%08x) - falling back to PEXE\n",
								pixelShaderProgram.getFileName(), static_cast<unsigned>(createResult)));
						}
					}
					else
					{
						WARNING(true, ("Direct3d9_PixelShaderProgramData: D3DXCompileShader failed for %s: %s - falling back to PEXE\n",
							pixelShaderProgram.getFileName(),
							errorMessages ? static_cast<char const *>(errorMessages->GetBufferPointer()) : "(no error text)"));
					}

					if (compiledShader)
						compiledShader->Release();
					if (errorMessages)
						errorMessages->Release();
					delete [] ownedText;
				}
			}

			// ---- fallback: use the precompiled PEXE blob ----
			if (!created)
			{
				HRESULT const hresult = Direct3d9::getDevice()->CreatePixelShader(pixelShaderProgram.m_exe, &m_pixelShader);
				FATAL_DX_HR("CreatePixelShader failed %s", hresult);
			}
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
