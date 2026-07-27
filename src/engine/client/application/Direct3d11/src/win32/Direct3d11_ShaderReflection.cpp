// ======================================================================
//
// Direct3d11_ShaderReflection.cpp
// copyright (c) 2026 Galaxies Reborn
//
// ======================================================================

#include "FirstDirect3d11.h"
#include "Direct3d11_ShaderReflection.h"

#include "Direct3d11_Device.h"

#include "clientGraphics/ShaderConstantRegisters.h"

#include <d3dcompiler.h>

// ======================================================================

namespace Direct3d11_ShaderReflectionNamespace
{
	// The names the shipped .inc files declare, paired with the register each one
	// is annotated with. This is the guard's entire subject matter: for every one
	// of these that a shader declares, its byte offset in $Globals must be exactly
	// sixteen times the register.
	//
	// The names come from the register enumeration in
	// clientGraphics/ShaderConstantRegisters.h. Where a shipped .inc turns out to
	// spell one differently, the fix is to correct the spelling HERE -- never to
	// relax the offset check, and never to renumber the register.
	struct NamedConstant
	{
		char const *name;
		int         registerIndex;
	};

	NamedConstant const cms_vertexConstants[] =
	{
		{ "objectWorldCameraProjectionMatrix", VSCR_objectWorldCameraProjectionMatrix },
		{ "objectWorldMatrix",                 VSCR_objectWorldMatrix                 },
		{ "cameraPosition",                    VSCR_cameraPosition                    },
		{ "viewportData",                      VSCR_viewportData                      },
		{ "fog",                               VSCR_fog                               },
		{ "material",                          VSCR_material                          },
		{ "lightData",                         VSCR_lightData                         },
		{ "textureFactor",                     VSCR_textureFactor                     },
		{ "textureFactor2",                    VSCR_textureFactor2                    },
		{ "textureScroll",                     VSCR_textureScroll                     },
		{ "currentTime",                       VSCR_currentTime                       },
		{ "unitX",                             VSCR_unitX                             },
		{ "unitY",                             VSCR_unitY                             },
		{ "unitZ",                             VSCR_unitZ                             },
		{ "userConstant0",                     VCSR_userConstant0                     },
		{ "userConstant1",                     VCSR_userConstant1                     },
		{ "userConstant2",                     VCSR_userConstant2                     },
		{ "userConstant3",                     VCSR_userConstant3                     },
		{ "userConstant4",                     VCSR_userConstant4                     },
		{ "userConstant5",                     VCSR_userConstant5                     },
		{ "userConstant6",                     VCSR_userConstant6                     },
		{ "userConstant7",                     VCSR_userConstant7                     },
		{ "extendedLightData",                 VCSR_extendedLightData                 },

		// The converted assembly half of the corpus does not use these names at all. Its generated
		// asm_constants.inc declares one flat array -- `float4 c[96] : register(c0)` -- and reaches
		// the engine's registers through #defines onto c[N], so reflection sees exactly one variable
		// called "c" at offset 0. Without this entry the guard checked nothing for all 94 converted
		// vertex programs, which is precisely the half most likely to have a layout mistake.
		{ "c",                                 VSCR_objectWorldCameraProjectionMatrix }
	};

	// These are the names the pixel side actually DECLARES, which is not the same as the names the
	// shaders read.
	//
	// Six of the nine entries here used to be unmatchable. The first five were spelled
	// dot3LightDirection, dot3LightDiffuseColor and so on -- but
	// pixel_program/include/pixel_shader_constants.inc, and the copy this DLL substitutes for it,
	// declare five VARIABLES called packedRegister0..4 and then make those friendly names
	// PREPROCESSOR MACROS onto swizzles of them (dot3LightDirection is packedRegister0.xyz,
	// materialSpecularPower is packedRegister0.w). A macro leaves no trace in reflection, so
	// strcmp could never match one. The ninth was "userConstant" against a declaration of
	// userConstants[17].
	//
	// Only textureFactor, textureFactor2 and materialSpecularColor were ever checked, and any
	// program that reads none of those three tripped the vacuous-guard warning below -- which it
	// did, for pixel_program/vertex_color.psh.
	NamedConstant const cms_pixelConstants[] =
	{
		{ "packedRegister0",                   PSCR_dot3LightDirection                },
		{ "packedRegister1",                   PSCR_dot3LightDiffuseColor             },
		{ "packedRegister2",                   PSCR_dot3LightSpecularColor            },
		{ "packedRegister3",                   PSCR_dot3LightTangentMinusDiffuseColor },
		{ "packedRegister4",                   PSCR_dot3LightTangentMinusBackColor    },
		{ "textureFactor",                     PSCR_textureFactor                     },
		{ "textureFactor2",                    PSCR_textureFactor2                    },
		{ "materialSpecularColor",             PSCR_materialSpecularColor             },
		{ "userConstants",                     PSCR_userConstant                      },

		// And the converted assembly pixel programs, which declare two flat arrays of their own:
		// `float4 psC[8] : register(c0)` and `float4 psUserConstants[17] : register(c8)`.
		{ "psC",                               PSCR_dot3LightDirection                },
		{ "psUserConstants",                   PSCR_userConstant                      }
	};

	int const cms_vertexConstantCount = sizeof(cms_vertexConstants) / sizeof(cms_vertexConstants[0]);
	int const cms_pixelConstantCount  = sizeof(cms_pixelConstants) / sizeof(cms_pixelConstants[0]);

	bool ms_warnedAboutVacuousGuard;

	void checkConstantBuffer(ID3D11ShaderReflection *reflection, char const *name, bool isVertexShader, Direct3d11_ShaderReflection::Result &result);
	void checkInterpolantSignature(ID3D11ShaderReflection *reflection, char const *name, bool isVertexShader);
	void buildSamplerMap(ID3D11ShaderReflection *reflection, char const *name, Direct3d11_ShaderReflection::Result &result);
}
using namespace Direct3d11_ShaderReflectionNamespace;

// ======================================================================

Direct3d11_ShaderReflection::Result::Result()
:
	constantExtentBytes(0),
	samplerCount(0),
	textureCount(0),
	checkedConstantCount(0)
{
	for (int i = 0; i < MAX_SAMPLER_SLOTS; ++i)
	{
		samplerToTexture[i] = -1;
		textureDimension[i] = D3D_SRV_DIMENSION_UNKNOWN;
	}
}

// ======================================================================
/**
 * Verify $Globals puts every constant we recognise at 16 times its register.
 *
 * A mismatch is fatal and says which constant, what offset it got, and what it
 * should have been. There is no useful way to continue: the shader will read some
 * other constant's value, and the resulting image is wrong in a way that looks
 * like a content or lighting bug rather than a layout bug.
 */

void Direct3d11_ShaderReflectionNamespace::checkConstantBuffer(ID3D11ShaderReflection *reflection, char const *name, bool isVertexShader, Direct3d11_ShaderReflection::Result &result)
{
	ID3D11ShaderReflectionConstantBuffer * const globals = reflection->GetConstantBufferByName("$Globals");
	if (!globals)
		return;

	D3D11_SHADER_BUFFER_DESC bufferDescription;
	Zero(bufferDescription);
	if (FAILED(globals->GetDesc(&bufferDescription)))
		return;

	result.constantExtentBytes = static_cast<int>(bufferDescription.Size);

	NamedConstant const * const table = isVertexShader ? cms_vertexConstants : cms_pixelConstants;
	int const tableCount = isVertexShader ? cms_vertexConstantCount : cms_pixelConstantCount;

	for (UINT i = 0; i < bufferDescription.Variables; ++i)
	{
		ID3D11ShaderReflectionVariable * const variable = globals->GetVariableByIndex(i);
		if (!variable)
			continue;

		D3D11_SHADER_VARIABLE_DESC variableDescription;
		Zero(variableDescription);
		if (FAILED(variable->GetDesc(&variableDescription)) || !variableDescription.Name)
			continue;

		// Every constant in this buffer must start on a sixteen-byte row,
		// recognised or not: the register file is addressed in rows.
		FATAL((variableDescription.StartOffset & 15) != 0, ("Direct3d11: in '%s', constant '%s' starts at byte %u, which is not a multiple of 16. The register file is addressed in 16-byte rows, so this constant cannot be addressed by any register number.", name, variableDescription.Name, variableDescription.StartOffset));

		for (int t = 0; t < tableCount; ++t)
			if (strcmp(variableDescription.Name, table[t].name) == 0)
			{
				unsigned int const expected = static_cast<unsigned int>(table[t].registerIndex) * 16;
				FATAL(variableDescription.StartOffset != expected, ("Direct3d11: in '%s', constant '%s' landed at byte offset %u but the shader assets require %u (register %d times 16). The register(cN) to $Globals offset correspondence has broken -- every constant in the corpus is now suspect. Check that D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY is still being passed.", name, variableDescription.Name, variableDescription.StartOffset, expected, table[t].registerIndex));

				++result.checkedConstantCount;
				break;
			}
	}
}

// ======================================================================
/**
 * Pair each sampler slot with the texture slot it actually got.
 *
 * Textures are packed densely among the samplers that are used, so the pairing
 * has to be read from the shader rather than assumed. Both lists come back in
 * BindPoint order, which is declaration order, so pairing them by position is
 * what reproduces the compiler's own association.
 */

void Direct3d11_ShaderReflectionNamespace::buildSamplerMap(ID3D11ShaderReflection *reflection, char const *name, Direct3d11_ShaderReflection::Result &result)
{
	D3D11_SHADER_DESC shaderDescription;
	Zero(shaderDescription);
	if (FAILED(reflection->GetDesc(&shaderDescription)))
		return;

	int samplerSlots[Direct3d11_ShaderReflection::MAX_SAMPLER_SLOTS];
	int textureSlots[Direct3d11_ShaderReflection::MAX_SAMPLER_SLOTS];
	D3D_SRV_DIMENSION textureDimensions[Direct3d11_ShaderReflection::MAX_SAMPLER_SLOTS];
	int samplerCount = 0;
	int textureCount = 0;

	for (UINT i = 0; i < shaderDescription.BoundResources; ++i)
	{
		D3D11_SHADER_INPUT_BIND_DESC bindDescription;
		Zero(bindDescription);
		if (FAILED(reflection->GetResourceBindingDesc(i, &bindDescription)))
			continue;

		if (bindDescription.Type == D3D_SIT_SAMPLER)
		{
			if (samplerCount < Direct3d11_ShaderReflection::MAX_SAMPLER_SLOTS)
				samplerSlots[samplerCount++] = static_cast<int>(bindDescription.BindPoint);
		}
		else if (bindDescription.Type == D3D_SIT_TEXTURE)
		{
			if (textureCount < Direct3d11_ShaderReflection::MAX_SAMPLER_SLOTS)
			{
				textureDimensions[textureCount] = bindDescription.Dimension;
				textureSlots[textureCount++] = static_cast<int>(bindDescription.BindPoint);
			}
		}
	}

	result.samplerCount = samplerCount;
	result.textureCount = textureCount;

	// A sampler with no texture, or the reverse, means the shader does something
	// this mapping cannot express -- worth knowing before it renders wrong.
	WARNING(samplerCount != textureCount, ("Direct3d11: '%s' declares %d sampler(s) and %d texture(s). The stage-to-slot mapping assumes one texture per sampler.", name, samplerCount, textureCount));

	int const pairCount = (samplerCount < textureCount) ? samplerCount : textureCount;
	for (int i = 0; i < pairCount; ++i)
	{
		int const samplerSlot = samplerSlots[i];
		if (samplerSlot >= 0 && samplerSlot < Direct3d11_ShaderReflection::MAX_SAMPLER_SLOTS)
			result.samplerToTexture[samplerSlot] = textureSlots[i];

		int const textureSlot = textureSlots[i];
		if (textureSlot >= 0 && textureSlot < Direct3d11_ShaderReflection::MAX_SAMPLER_SLOTS)
			result.textureDimension[textureSlot] = textureDimensions[i];
	}
}

// ======================================================================

// ======================================================================
/**
 * Verify a program's interpolant signature is the canonical one, and name it if not.
 *
 * D3D11 links the vertex and pixel stages by REGISTER, so the only thing that makes independently
 * compiled programs pairable is that every one of them presents an identical signature.
 * Direct3d11_ShaderSignature wraps both stages to guarantee that -- and a guarantee nothing checks is
 * a guarantee that quietly stops holding. It already stopped holding once: the point sprite geometry
 * shader kept a hand-written three-element signature through the change that canonicalised the other
 * two stages, and the only symptom was 397 debug layer errors a minute in a build most people run
 * without the debug layer on.
 *
 * So this checks the thing that matters, at the moment the bytecode exists, and reports the PROGRAM
 * NAME. Without a name, a linkage error names two anonymous stages and the search space is the whole
 * corpus.
 *
 * A vertex shader is judged on its OUTPUT signature and a pixel shader on its INPUT signature: those
 * are the two halves that have to agree. Extra elements are as much of a failure as missing ones,
 * because a register the consumer does not expect shifts everything after it.
 */

void Direct3d11_ShaderReflectionNamespace::checkInterpolantSignature(ID3D11ShaderReflection *reflection, char const *name, bool isVertexShader)
{
	D3D11_SHADER_DESC description;
	Zero(description);
	if (FAILED(reflection->GetDesc(&description)))
		return;

	// Semantic name and index for each of the twelve canonical slots, in register order. SV_Position
	// is reported by reflection under that name even though the asset spells it POSITION0.
	struct Expected
	{
		char const *semantic;
		UINT        index;
	};

	static Expected const expected[] =
	{
		{ "SV_Position", 0 },
		{ "COLOR",       0 },
		{ "COLOR",       1 },
		{ "FOG",         0 },
		{ "TEXCOORD",    0 },
		{ "TEXCOORD",    1 },
		{ "TEXCOORD",    2 },
		{ "TEXCOORD",    3 },
		{ "TEXCOORD",    4 },
		{ "TEXCOORD",    5 },
		{ "TEXCOORD",    6 },
		{ "TEXCOORD",    7 },
	};

	int const expectedCount = isizeof(expected) / isizeof(expected[0]);
	UINT const actualCount = isVertexShader ? description.OutputParameters : description.InputParameters;

	bool matches = (actualCount == static_cast<UINT>(expectedCount));

	if (matches)
	{
		for (int i = 0; i < expectedCount; ++i)
		{
			D3D11_SIGNATURE_PARAMETER_DESC parameter;
			Zero(parameter);

			HRESULT const hresult = isVertexShader
				? reflection->GetOutputParameterDesc(static_cast<UINT>(i), &parameter)
				: reflection->GetInputParameterDesc(static_cast<UINT>(i), &parameter);

			if (FAILED(hresult))
			{
				matches = false;
				break;
			}

			// Register order is what linkage cares about, so the element at index i must be the slot
			// expected at register i.
			if (parameter.Register != static_cast<UINT>(i) ||
				parameter.SemanticIndex != expected[i].index ||
				!parameter.SemanticName ||
				_stricmp(parameter.SemanticName, expected[i].semantic) != 0)
			{
				matches = false;
				break;
			}
		}
	}

	if (matches)
		return;

	// Report every offender by name, not once overall: the set is what needs fixing, and one
	// example hides the rest. The corpus is a few hundred programs, so the log stays readable.
	WARNING(true, ("Direct3d11: '%s' does not present the canonical interpolant signature -- it declares %u %s element(s) where the canonical set has %d. Any draw pairing it with a canonical program will be rejected by D3D11 as a linkage error and will not render.",
		name, actualCount, isVertexShader ? "output" : "input", expectedCount));

	for (UINT i = 0; i < actualCount; ++i)
	{
		D3D11_SIGNATURE_PARAMETER_DESC parameter;
		Zero(parameter);

		HRESULT const hresult = isVertexShader
			? reflection->GetOutputParameterDesc(i, &parameter)
			: reflection->GetInputParameterDesc(i, &parameter);

		if (SUCCEEDED(hresult))
			WARNING(true, ("Direct3d11:     %s: %s%u at register %u mask 0x%x",
				name,
				parameter.SemanticName ? parameter.SemanticName : "<null>",
				parameter.SemanticIndex, parameter.Register, static_cast<unsigned>(parameter.Mask)));
	}
}

// ======================================================================

bool Direct3d11_ShaderReflection::reflect(ID3DBlob *bytecode, char const *name, bool isVertexShader, Result &result)
{
	NOT_NULL(bytecode);
	NOT_NULL(name);

	ID3D11ShaderReflection *reflection = NULL;
	HRESULT const hresult = D3DReflect(bytecode->GetBufferPointer(), bytecode->GetBufferSize(), IID_ID3D11ShaderReflection, reinterpret_cast<void **>(&reflection));
	if (FAILED(hresult) || !reflection)
	{
		WARNING(true, ("Direct3d11: '%s' could not be reflected (%s), so its constant layout and sampler mapping are unverified.", name, Direct3d11_Device::describeHresult(hresult)));
		return false;
	}

	checkConstantBuffer(reflection, name, isVertexShader, result);
	buildSamplerMap(reflection, name, result);
	checkInterpolantSignature(reflection, name, isVertexShader);

	reflection->Release();

	// A guard that matches nothing passes every time, which is indistinguishable
	// from working. If no shader has yet had a single recognised constant, the name
	// table is wrong and the ABI is not actually being checked -- say so once,
	// loudly, rather than reporting silent success forever.
	if (!result.checkedConstantCount && !ms_warnedAboutVacuousGuard)
	{
		ms_warnedAboutVacuousGuard = true;
		WARNING(true, ("Direct3d11: '%s' declared %d constant(s) in $Globals and NONE of them matched a known name, so the constant ABI guard checked nothing. The name table in Direct3d11_ShaderReflection.cpp does not match what the shipped .inc files declare and must be corrected.", name, result.constantExtentBytes / 16));
	}

	return true;
}

// ======================================================================
