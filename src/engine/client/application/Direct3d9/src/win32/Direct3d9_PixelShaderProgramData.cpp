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
#include "Direct3d9_ShaderCache.h"
#include "clientGraphics/ShaderCapability.h"
#include "clientGraphics/ShaderImplementation.h"
#include "sharedFile/TreeFile.h"

#include <d3d9.h>
#include <d3dcompiler.h>
#include <float.h>
#include <string.h>
#include <vector>

// ======================================================================
// Recompile-from-PSRC pixel path (D3DX-free).
//
// This tree's shader corpus (the asm2hlsl-converted set) carries the real
// program as HLSL text in the PSRC chunk and only a STUB in the PEXE chunk
// (vertex_color.psh: 4 bytes) -- the DX11 pipeline compiles from source and
// never needed the bytecode. Handing such a stub to CreatePixelShader is an
// AV inside the d3d9 runtime's shader validator (it walks tokens off the
// end of the allocation; that was the wholesale-replacement wave's boot
// crash on BOTH datasets, crash-dump-confirmed at
// Direct3DShaderValidatorCreate9+0x9cd loading 2d_vertexcolor.sht).
//
// So the D3D9 pixel path compiles from source whenever the PSRC carries an
// `//hlsl` directive, exactly the design the pre-wave tree ran -- but
// through D3DCompile (d3dcompiler_47, the same DLL the vertex path already
// uses) instead of D3DXCompileShader, keeping the x64 line D3DX-free. The
// PEXE blob remains the path for plain-asm programs (stock data carries
// real bytecode there). Compiled bytecode round-trips through
// Direct3d9_ShaderCache, so a warm session creates from cached bytes
// without touching the compiler at all.
// ======================================================================

namespace Direct3d9_PixelShaderProgramDataNamespace
{
	// Bump when anything in this file changes the emitted bytecode
	// (prefix text, include override, target defaulting) -- it feeds the
	// ShaderCache hash so stale cache entries self-invalidate.
	uint32_t const cms_pixelRewriteVersion = 1;

	// ==================================================================
	// pixel_shader_constants.inc register-layout override.
	//
	// THE BUG (inherited finding, kept verbatim from the pre-wave tree):
	// the TRE's `pixel_program/include/pixel_shader_constants.inc` declares
	// the PS constant registers in a layout that does NOT match the
	// engine's hard-coded `Direct3d9_PixelShaderConstantRegisters` enum.
	// The original engine never tripped on this because it used the
	// precompiled PEXE bytecode, which was built against the engine layout.
	// Once we compile from PSRC source, the compiled shader would read
	// `textureFactor` from c3 -- where the engine uploads
	// `dot3LightTangentMinusDiffuseColor` (negative for hemispheric suns),
	// driving result.rgb negative -> clamped black characters.
	//
	// THE FIX: when the compiler asks for pixel_shader_constants.inc, hand
	// back the canonical engine-layout version (matches the PSCR_* enum
	// exactly). Every other include passes through from the TRE unchanged.
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
	// texren_copy_c1a1.psh face-bake substitution (inherited fix, kept
	// verbatim). The stock program is ps.1.1 ASM reading its tint from c2
	// while the engine uploads the texture factor to c5 -- every runtime-
	// baked face texture came out black. ps_1_x asm cannot be recompiled
	// by the modern toolchain, so substitute a ps_2_0 HLSL equivalent that
	// reads textureFactor from the engine-layout register.
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

	// Include handler for the D3DCompile path: includes are plain text in
	// the TRE; open and hand them back -- except pixel_shader_constants.inc,
	// which is overridden with the engine-layout version above.
	class PixelIncludeHandler : public ID3DInclude
	{
	public:
		STDMETHOD(Open)(D3D_INCLUDE_TYPE, LPCSTR pFileName, LPCVOID, LPCVOID *ppData, UINT *pBytes)
		{
			// command-line-compiler style relative includes
			if (pFileName && strncmp(pFileName, "../../", 6) == 0)
				pFileName += 6;

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
			delete[] const_cast<char *>(static_cast<char const *>(pData));
			return S_OK;
		}
	};
} // namespace Direct3d9_PixelShaderProgramDataNamespace

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
			bool created = false;

			// ---- compile from PSRC source when it carries an //hlsl directive ----
			char const *const source = pixelShaderProgram.m_source;
			if (source && pixelShaderProgram.m_sourceLength > 0)
			{
				// Parse the leading "//hlsl ps_X_X" directive.
				char const *p = source;
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

				// Face-bake substitution: filename-triggered, see note above.
				bool useTexrenReplacement = false;
				{
					char const *const fileName = pixelShaderProgram.getFileName();
					if (!isHlsl && fileName && strstr(fileName, "texren_copy_c1a1"))
					{
						useTexrenReplacement = true;
						isHlsl = true;
						strcpy(target, "ps_2_0");
					}
				}

				if (isHlsl)
				{
					// Some pixel-program includes use `point` as a field name,
					// which collides with the HLSL reserved word -- same
					// prefix define the vertex-shader path uses.
					static char const PREFIX[] = "#define point _pt_lights\n";

					char const *compileText = NULL;
					int compileLen = 0;
					char *ownedText = NULL;

					if (useTexrenReplacement)
					{
						compileText = TEXREN_COPY_C1A1_PS20_HLSL;
						compileLen = static_cast<int>(sizeof(TEXREN_COPY_C1A1_PS20_HLSL) - 1);
					}
					else
					{
						int const prefixLen = static_cast<int>(sizeof(PREFIX) - 1);
						int const srcLen = static_cast<int>(strlen(source));

						ownedText = new char[prefixLen + srcLen + 1];
						memcpy(ownedText, PREFIX, prefixLen);
						memcpy(ownedText + prefixLen, source, srcLen);
						ownedText[prefixLen + srcLen] = '\0';

						compileText = ownedText;
						compileLen = prefixLen + srcLen;
					}

					// Warm start: bytecode cache keyed on the exact compile
					// input. A hit skips the compiler entirely.
					uint64_t const sourceHash = Direct3d9_ShaderCache::hashSource(
						compileText, static_cast<std::size_t>(compileLen), NULL, target, cms_pixelRewriteVersion);

					std::vector<unsigned char> cachedBytes;
					if (Direct3d9_ShaderCache::tryLoad(sourceHash, cachedBytes) && !cachedBytes.empty())
					{
						HRESULT const createResult = Direct3d9::getDevice()->CreatePixelShader(
							reinterpret_cast<DWORD const *>(&cachedBytes[0]), &m_pixelShader);
						if (SUCCEEDED(createResult) && m_pixelShader)
							created = true;
					}

					if (!created)
					{
						PixelIncludeHandler includeHandler;
						ID3DBlob *compiledShader = NULL;
						ID3DBlob *errorMessages = NULL;
						HRESULT const compileResult = D3DCompile(
							compileText, static_cast<SIZE_T>(compileLen),
							pixelShaderProgram.getFileName(),
							NULL, &includeHandler,
							"main", target, 0, 0,
							&compiledShader, &errorMessages);

						// the compiler can leave the FPU in a bad state on some shaders.
						_clearfp();

						if (SUCCEEDED(compileResult) && compiledShader)
						{
							HRESULT const createResult = Direct3d9::getDevice()->CreatePixelShader(
								reinterpret_cast<DWORD const *>(compiledShader->GetBufferPointer()), &m_pixelShader);
							if (SUCCEEDED(createResult) && m_pixelShader)
							{
								created = true;
								Direct3d9_ShaderCache::store(sourceHash, compiledShader->GetBufferPointer(), compiledShader->GetBufferSize());
							}
							else
							{
								WARNING(true, ("Direct3d9_PixelShaderProgramData: CreatePixelShader failed for recompiled %s (0x%08x) - falling back to PEXE\n",
											   pixelShaderProgram.getFileName(), static_cast<unsigned>(createResult)));
							}
						}
						else
						{
							WARNING(true, ("Direct3d9_PixelShaderProgramData: D3DCompile failed for %s: %s - falling back to PEXE\n",
										   pixelShaderProgram.getFileName(),
										   errorMessages ? static_cast<char const *>(errorMessages->GetBufferPointer()) : "(no error text)"));
						}

						if (compiledShader)
							compiledShader->Release();
						if (errorMessages)
							errorMessages->Release();
					}

					delete[] ownedText;
				}
			}

			// ---- fallback: the precompiled PEXE blob (plain-asm programs;
			// stock data carries real bytecode here) ----
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
