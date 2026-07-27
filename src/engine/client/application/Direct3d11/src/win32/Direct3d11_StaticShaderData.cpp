// ======================================================================
//
// Direct3d11_StaticShaderData.cpp
// copyright (c) 2026 Galaxies Reborn
//
// ======================================================================

#include "FirstDirect3d11.h"
#include "Direct3d11_StaticShaderData.h"

#include "Direct3d11.h"
#include "Direct3d11_ConstantBuffers.h"
#include "Direct3d11_Device.h"
#include "Direct3d11_Metrics.h"
#include "Direct3d11_PixelShaderProgramData.h"
#include "Direct3d11_ShaderImplementationData.h"
#include "Direct3d11_LightManager.h"
#include "Direct3d11_StateCache.h"
#include "Direct3d11_StateObjectCache.h"
#include "Direct3d11_StateTables.h"
#include "Direct3d11_TextureData.h"
#include "Direct3d11_VertexShaderData.h"

#include "clientGraphics/Material.h"
#include "clientGraphics/ShaderConstantRegisters.h"
#include "clientGraphics/ShaderEffect.h"
#include "clientGraphics/StaticShaderTemplate.h"
#include "clientGraphics/Texture.h"
#include "sharedFoundation/Tag.h"

#include <math.h>

// ======================================================================

namespace Direct3d11_StaticShaderDataNamespace
{
	Tag const TAG_A000 = TAG(A,0,0,0);
	Tag const TAG_A001 = TAG(A,0,0,1);
	Tag const TAG_A128 = TAG(A,1,2,8);
	Tag const TAG_A255 = TAG(A,2,5,5);

	int  ms_liveInstanceCount;
	bool ms_installed;

	bool ms_reportedFogColor;
	bool ms_reportedFullAmbient;

	// ConvertTagToStaticString does not exist in this tree and ConvertTagToString needs a
	// buffer, so warnings that name a tag get one from here. A rotating set of buffers keeps
	// a message that names two tags from showing the same one twice.
	char const *tagText(Tag tag)
	{
		enum { BUFFERS = 4 };
		static char text[BUFFERS][8];
		static int next;

		char * const buffer = text[next];
		next = (next + 1) % BUFFERS;

		ConvertTagToString(tag, buffer);
		return buffer;
	}

	// ------------------------------------------------------------------
	/**
	 * Resolve one texture sampler into a texture pointer and a sampler state.
	 *
	 * Both the material and the effect's sampler declaration can specify addressing and
	 * filtering; the material wins where it says anything, which is what DX9's
	 * TA/TF macros express. A material that says nothing at all leaves the effect's value.
	 */
	void resolveSampler(StaticShaderTemplate::TextureData const &textureData,
		ShaderImplementationPassPixelShaderTextureSampler const &declaration,
		D3D11_SAMPLER_DESC &description)
	{
		int const addressU = (textureData.addressU != StaticShaderTemplate::TA_invalid)
			? static_cast<int>(textureData.addressU) : static_cast<int>(declaration.m_textureAddressU);
		int const addressV = (textureData.addressV != StaticShaderTemplate::TA_invalid)
			? static_cast<int>(textureData.addressV) : static_cast<int>(declaration.m_textureAddressV);
		int const addressW = (textureData.addressW != StaticShaderTemplate::TA_invalid)
			? static_cast<int>(textureData.addressW) : static_cast<int>(declaration.m_textureAddressW);

		int const mipFilter = (textureData.mipFilter != StaticShaderTemplate::TF_invalid)
			? static_cast<int>(textureData.mipFilter) : static_cast<int>(declaration.m_textureMipFilter);
		int const minFilter = (textureData.minificationFilter != StaticShaderTemplate::TF_invalid)
			? static_cast<int>(textureData.minificationFilter) : static_cast<int>(declaration.m_textureMinificationFilter);
		int const magFilter = (textureData.magnificationFilter != StaticShaderTemplate::TF_invalid)
			? static_cast<int>(textureData.magnificationFilter) : static_cast<int>(declaration.m_textureMagnificationFilter);

		Zero(description);

		bool isAnisotropic = false;
		description.Filter = Direct3d11_StateTables::getFilter(minFilter, magFilter, mipFilter, isAnisotropic);

		description.AddressU = Direct3d11_StateTables::getTextureAddress(addressU);
		description.AddressV = Direct3d11_StateTables::getTextureAddress(addressV);
		description.AddressW = Direct3d11_StateTables::getTextureAddress(addressW);

		// D3D9 had to ask the device for its maximum; every D3D11 feature level this backend
		// runs on supports the full 16, so the cap is the API's rather than the adapter's.
		description.MaxAnisotropy = static_cast<UINT>(clamp(1, textureData.maxAnisotropy, static_cast<int>(D3D11_REQ_MAXANISOTROPY)));

		// D3D9's mip filter of NONE means "do not use mip maps", which in D3D11 is expressed
		// by clamping the LOD range rather than by the filter. Without this, a texture whose
		// material asks for no mip mapping would still be filtered across its chain.
		if (Direct3d11_StateTables::isMipFilterDisabled(mipFilter))
		{
			description.MinLOD = 0.0f;
			description.MaxLOD = 0.0f;
		}
		else
		{
			description.MinLOD = -D3D11_FLOAT32_MAX;
			description.MaxLOD = D3D11_FLOAT32_MAX;
		}

		// D3D9's default border colour was opaque black and nothing in the engine sets one,
		// so leaving the zeroed description would give transparent black and change every
		// bordered fetch.
		description.BorderColor[0] = 0.0f;
		description.BorderColor[1] = 0.0f;
		description.BorderColor[2] = 0.0f;
		description.BorderColor[3] = 1.0f;

		description.ComparisonFunc = D3D11_COMPARISON_NEVER;
	}

	// ------------------------------------------------------------------

	char const *describeSrvDimension(D3D_SRV_DIMENSION dimension)
	{
		switch (dimension)
		{
			case D3D_SRV_DIMENSION_TEXTURE2D:      return "Texture2D";
			case D3D_SRV_DIMENSION_TEXTURECUBE:    return "TextureCube";
			case D3D_SRV_DIMENSION_TEXTURE3D:      return "Texture3D";
			case D3D_SRV_DIMENSION_TEXTURE1D:      return "Texture1D";
			case D3D_SRV_DIMENSION_TEXTURE2DMS:    return "Texture2DMS";
			case D3D_SRV_DIMENSION_UNKNOWN:        return "nothing";
			default:                               break;
		}

		return "an unrecognised resource type";
	}
}
using namespace Direct3d11_StaticShaderDataNamespace;

// ======================================================================

void Direct3d11_StaticShaderData::install()
{
	DEBUG_FATAL(ms_installed, ("Direct3d11_StaticShaderData::install called twice"));
	ms_installed = true;

	ms_reportedFogColor = false;
	ms_reportedFullAmbient = false;
}

// ----------------------------------------------------------------------

void Direct3d11_StaticShaderData::remove()
{
	DEBUG_WARNING(ms_liveInstanceCount != 0, ("Direct3d11: %d static shader(s) are still alive at shutdown.", ms_liveInstanceCount));
	ms_installed = false;
}

// ----------------------------------------------------------------------

int Direct3d11_StaticShaderData::getLiveInstanceCount()
{
	return ms_liveInstanceCount;
}

// ======================================================================

Direct3d11_StaticShaderData::Direct3d11_StaticShaderData(StaticShader const &shader)
:
	StaticShaderGraphicsData(),
	m_implementation(NULL),
	m_passes(NULL),
	m_passCount(0)
{
	++ms_liveInstanceCount;
	construct(shader);
}

// ----------------------------------------------------------------------

Direct3d11_StaticShaderData::~Direct3d11_StaticShaderData()
{
	--ms_liveInstanceCount;
	destroy();
}

// ----------------------------------------------------------------------

void Direct3d11_StaticShaderData::destroy()
{
	// Sampler states belong to Direct3d11_StateObjectCache and textures to the engine, so
	// neither is released here.
	for (int i = 0; i < m_passCount; ++i)
		delete [] m_passes[i].stages;

	delete [] m_passes;
	m_passes = NULL;
	m_passCount = 0;
	m_implementation = NULL;
}

// ----------------------------------------------------------------------

void Direct3d11_StaticShaderData::update(StaticShader const &shader)
{
	// The engine calls this when a material's values change under a shader that already
	// exists. Rebuilding is correct and is what DX9 does; the cost is bounded because
	// everything expensive -- the sampler states, the compiled programs -- is shared and
	// merely looked up again.
	destroy();
	construct(shader);
}

// ======================================================================

void Direct3d11_StaticShaderData::construct(StaticShader const &shader)
{
	// StaticShader has no accessor for this; the engine names this class a friend of both
	// StaticShaderTemplate and ShaderEffect so it can be reached, exactly as DX9 does.
	ShaderEffect const * const effect = shader.getStaticShaderTemplate().m_effect;
	ShaderImplementation const * const implementation = effect ? effect->m_implementation : NULL;

	// No isSupported() check: it is not exported across the DLL boundary, and it would be
	// redundant anyway. ShaderImplementation::load only calls
	// createShaderImplementationGraphicsData for an implementation whose capability matched,
	// so an unsupported one never reaches a material that could be drawn. DX9 does not check
	// either.
	if (!implementation)
		return;

	int const passCount = implementation->m_pass ? static_cast<int>(implementation->m_pass->size()) : 0;
	if (passCount <= 0)
		return;

	m_implementation = implementation;
	m_passes = new Pass[passCount];
	m_passCount = passCount;

	for (int passNumber = 0; passNumber < passCount; ++passNumber)
	{
		ShaderImplementation::Pass const &source = *(*implementation->m_pass)[static_cast<size_t>(passNumber)];
		Pass &pass = m_passes[passNumber];

		Zero(pass);
		pass.fogMode = static_cast<int>(source.m_fogMode);
		pass.fullAmbient = shader.containsPrecalculatedVertexLighting();

		// ----------------------------------------------------------
		// The vertex program, and the texture coordinate routing it needs

		pass.vertexShader = source.m_vertexShader;

		if (source.m_vertexShader)
		{
			Direct3d11_VertexShaderData const * const vertexData =
				static_cast<Direct3d11_VertexShaderData const *>(source.m_vertexShader->m_graphicsData);

			if (vertexData)
			{
				Direct3d11_VertexShaderData::TextureCoordinateSetTags const * const tags = vertexData->getTextureCoordinateSetTags();
				if (tags)
				{
					int const tagCount = static_cast<int>(tags->size());
					DEBUG_WARNING(tagCount > MAX_TEXTURE_COORDINATE_SETS, ("Direct3d11: vertex program declares %d texture coordinate set tags; %d are carried.", tagCount, static_cast<int>(MAX_TEXTURE_COORDINATE_SETS)));

					int const carried = (tagCount < MAX_TEXTURE_COORDINATE_SETS) ? tagCount : MAX_TEXTURE_COORDINATE_SETS;
					pass.textureCoordinateSetMappingCount = carried;

					for (int i = 0; i < carried; ++i)
					{
						uint8 set = 0;
						if (!shader.getTextureCoordinateSet((*tags)[static_cast<size_t>(i)], set))
						{
							// DX9 warns and uses set 0. Same choice, so a material missing a
							// tag looks the same in both backends.
							DEBUG_WARNING(true, ("Direct3d11: material '%s' has no texture coordinate set for tag %s; using 0.",
								shader.getShaderTemplate().getName().getString(),
								tagText((*tags)[static_cast<size_t>(i)])));
							set = 0;
						}

						if (set > 7)
						{
							DEBUG_WARNING(true, ("Direct3d11: material '%s' names texture coordinate set %d, which is out of range; using 0.",
								shader.getShaderTemplate().getName().getString(), static_cast<int>(set)));
							set = 0;
						}

						pass.textureCoordinateSetMapping[i] = static_cast<int>(set);
					}
				}
			}
		}

		if (source.m_pixelShader)
			pass.pixelShaderProgram = source.m_pixelShader->m_program;

		// ----------------------------------------------------------
		// Material colours, laid out as D3D9's D3DMATERIAL9 was
		//
		// Diffuse, ambient, specular, emissive, then power -- five rows at c11, padded so
		// the power's row is whole. The order is not alphabetical or arbitrary: it is the
		// order the shipped vertex_shader_constants.inc declares its Material struct in, and
		// the engine uploaded the C++ struct verbatim.

		if (source.m_materialTag)
		{
			Material material;
			bool const found = shader.getMaterial(source.m_materialTag, material);

			if (!found)
				DEBUG_WARNING(true, ("Direct3d11: material tag %s is missing from shader '%s'; using a white default.", tagText(source.m_materialTag), shader.getName() ? shader.getName() : "<unnamed>"));

			VectorArgb const diffuse  = found ? material.getDiffuseColor()  : VectorArgb(1.0f, 1.0f, 1.0f, 1.0f);
			VectorArgb const ambient  = found ? material.getAmbientColor()  : VectorArgb(1.0f, 1.0f, 1.0f, 1.0f);
			VectorArgb const specular = found ? material.getSpecularColor() : VectorArgb(0.0f, 0.0f, 0.0f, 1.0f);
			VectorArgb const emissive = found ? material.getEmissiveColor() : VectorArgb(0.0f, 0.0f, 0.0f, 1.0f);
			float const power         = found ? material.getSpecularPower() : 0.0f;

			float *row = pass.material;
			row[0] = diffuse.r;  row[1] = diffuse.g;  row[2] = diffuse.b;  row[3] = diffuse.a;   row += 4;
			row[0] = ambient.r;  row[1] = ambient.g;  row[2] = ambient.b;  row[3] = ambient.a;   row += 4;
			row[0] = specular.r; row[1] = specular.g; row[2] = specular.b; row[3] = specular.a;  row += 4;
			row[0] = emissive.r; row[1] = emissive.g; row[2] = emissive.b; row[3] = emissive.a;  row += 4;
			row[0] = power;      row[1] = 0.0f;       row[2] = 0.0f;       row[3] = 0.0f;

			pass.materialValid = true;
		}

		// ----------------------------------------------------------
		// Texture factors
		//
		// Packed 0xAARRGGBB, which is the engine's PackedArgb order, unpacked to floats.

		if (source.m_textureFactorTag)
		{
			uint32 factor = 0;
			if (shader.getTextureFactor(source.m_textureFactorTag, factor))
			{
				pass.textureFactorValid = true;
				pass.textureFactor[0].r = static_cast<float>((factor >> 16) & 0xff) / 255.0f;
				pass.textureFactor[0].g = static_cast<float>((factor >>  8) & 0xff) / 255.0f;
				pass.textureFactor[0].b = static_cast<float>((factor >>  0) & 0xff) / 255.0f;
				pass.textureFactor[0].a = static_cast<float>((factor >> 24) & 0xff) / 255.0f;
			}
			else
				DEBUG_WARNING(true, ("Direct3d11: texture factor %s is missing from '%s'.", tagText(source.m_textureFactorTag), shader.getShaderTemplate().getName().getString()));
		}

		if (source.m_textureFactorTag2)
		{
			uint32 factor = 0;
			if (shader.getTextureFactor(source.m_textureFactorTag2, factor))
			{
				pass.textureFactorValid = true;
				pass.textureFactor[1].r = static_cast<float>((factor >> 16) & 0xff) / 255.0f;
				pass.textureFactor[1].g = static_cast<float>((factor >>  8) & 0xff) / 255.0f;
				pass.textureFactor[1].b = static_cast<float>((factor >>  0) & 0xff) / 255.0f;
				pass.textureFactor[1].a = static_cast<float>((factor >> 24) & 0xff) / 255.0f;
			}
			else
				DEBUG_WARNING(true, ("Direct3d11: texture factor %s is missing from '%s'.", tagText(source.m_textureFactorTag2), shader.getShaderTemplate().getName().getString()));
		}

		// ----------------------------------------------------------
		// Texture scroll rates

		if (source.m_textureScrollTag)
		{
			StaticShaderTemplate::TextureScroll scroll;
			pass.textureScrollValid = true;

			if (shader.getTextureScroll(source.m_textureScrollTag, scroll))
			{
				pass.textureScroll[0] = scroll.u1;
				pass.textureScroll[1] = scroll.v1;
				pass.textureScroll[2] = scroll.u2;
				pass.textureScroll[3] = scroll.v2;
			}
			else
			{
				// DX9 keeps the upload and zeroes the rates rather than skipping it, so the
				// register holds a known value instead of whatever the last material left.
				DEBUG_WARNING(true, ("Direct3d11: texture scroll tag %s is missing from '%s'; using zero.", tagText(source.m_textureScrollTag), shader.getName() ? shader.getName() : "<unnamed>"));
			}
		}

		// ----------------------------------------------------------
		// The two reference values

		if (source.m_alphaTestEnable)
		{
			pass.alphaTestReferenceValid = true;

			// Four tags are literal values rather than lookups.
			if (source.m_alphaTestReferenceValueTag == TAG_A255)
				pass.alphaTestReference = 255;
			else if (source.m_alphaTestReferenceValueTag == TAG_A128)
				pass.alphaTestReference = 128;
			else if (source.m_alphaTestReferenceValueTag == TAG_A001)
				pass.alphaTestReference = 1;
			else if (source.m_alphaTestReferenceValueTag == TAG_A000)
				pass.alphaTestReference = 0;
			else if (!shader.getAlphaTestReferenceValue(source.m_alphaTestReferenceValueTag, pass.alphaTestReference))
			{
				DEBUG_WARNING(true, ("Direct3d11: alpha reference %s is missing from '%s'; using 1.", tagText(source.m_alphaTestReferenceValueTag), shader.getName() ? shader.getName() : "<unnamed>"));
				pass.alphaTestReference = 1;
			}
		}

		if (source.m_stencilEnable)
		{
			if (shader.getStencilReferenceValue(source.m_stencilReferenceValueTag, pass.stencilReference))
				pass.stencilReferenceValid = true;
			else
				DEBUG_WARNING(true, ("Direct3d11: stencil reference %s is missing from '%s'.", tagText(source.m_stencilReferenceValueTag), shader.getName() ? shader.getName() : "<unnamed>"));
		}

		// ----------------------------------------------------------
		// The texture stages
		//
		// One entry per sampler the effect declares. DX9 sizes this by the highest sampler
		// index so the render code can index it directly; the same is done here, and the
		// unused entries stay empty and bind nothing.

		if (source.m_pixelShader && source.m_pixelShader->m_textureSamplers)
		{
			int const stageCount = source.m_pixelShader->m_maxTextureSampler + 1;
			if (stageCount > 0)
			{
				pass.stages = new Stage[stageCount];
				pass.stageCount = stageCount;

				for (int i = 0; i < stageCount; ++i)
				{
					pass.stages[i].texture = NULL;
					pass.stages[i].sampler = NULL;
					pass.stages[i].samplerSlot = i;
					pass.stages[i].placeholder = false;
				}

				ShaderImplementation::Pass::PixelShader::TextureSamplers const &samplers = *source.m_pixelShader->m_textureSamplers;
				for (size_t s = 0; s < samplers.size(); ++s)
				{
					ShaderImplementationPassPixelShaderTextureSampler const * const sampler = samplers[s];
					if (!sampler || !sampler->m_textureTag)
						continue;

					int const index = sampler->m_textureIndex;
					if (index < 0 || index >= stageCount)
					{
						DEBUG_WARNING(true, ("Direct3d11: a texture sampler names stage %d, outside 0..%d.", index, stageCount - 1));
						continue;
					}

					Stage &stage = pass.stages[index];

					StaticShaderTemplate::TextureData textureData;
					if (!shader.getTextureData(sampler->m_textureTag, textureData))
					{
						DEBUG_WARNING(true, ("Direct3d11: '%s' has no texture data for %s; using wrapped linear defaults.",
							shader.getShaderTemplate().getName().getString(), tagText(sampler->m_textureTag)));

						textureData.placeholder         = false;
						textureData.addressU            = StaticShaderTemplate::TA_wrap;
						textureData.addressV            = StaticShaderTemplate::TA_wrap;
						textureData.addressW            = StaticShaderTemplate::TA_wrap;
						textureData.mipFilter           = StaticShaderTemplate::TF_linear;
						textureData.minificationFilter  = StaticShaderTemplate::TF_linear;
						textureData.magnificationFilter = StaticShaderTemplate::TF_linear;
						textureData.maxAnisotropy       = 1;
						textureData.texture             = 0;
					}

					if (Direct3d11_TextureData::isGlobalTexture(sampler->m_textureTag))
					{
						// The registry hands out the address of its stored pointer, so a later
						// setGlobalTexture is seen by every material already built.
						stage.placeholder = false;
						stage.texture = Direct3d11_TextureData::getGlobalTexture(sampler->m_textureTag);
					}
					else
					{
						stage.placeholder = textureData.placeholder;
						if (textureData.texture)
							stage.texture = reinterpret_cast<Direct3d11_TextureData const * const *>(textureData.texture->getGraphicsDataAddress());
					}

					D3D11_SAMPLER_DESC description;
					resolveSampler(textureData, *sampler, description);
					stage.sampler = Direct3d11_StateObjectCache::getSamplerState(description);
				}
			}
		}

		// ----------------------------------------------------------
		// What cannot be honoured yet

		if (pass.fogMode != static_cast<int>(ShaderImplementation::Pass::FM_Normal) && !ms_reportedFogColor)
		{
			ms_reportedFogColor = true;
			WARNING(true, ("Direct3d11: a pass asks for a fog colour override, which D3D11 has no state for -- fog has to be applied in the pixel shader from constant buffer b1. Those surfaces fog with the scene colour. Reported once."));
		}

		UNREF(ms_reportedFullAmbient);
	}
}

// ======================================================================
/**
 * A batching key that does not truncate.
 *
 * DX9 casts the D3D texture pointer straight to int, which on x64 discards the top half and
 * makes two distinct textures sort as one whenever they differ only above bit 31. The
 * texture class folds the whole pointer instead; this returns the first stage that has one.
 */

int Direct3d11_StaticShaderData::getTextureSortKey() const
{
	for (int passNumber = 0; passNumber < m_passCount; ++passNumber)
	{
		Pass const &pass = m_passes[passNumber];
		for (int i = 0; i < pass.stageCount; ++i)
		{
			Direct3d11_TextureData const * const * const holder = pass.stages[i].texture;
			if (holder && *holder)
				return (*holder)->getSortKey();
		}
	}

	return 0;
}

// ======================================================================

bool Direct3d11_StaticShaderData::apply(int passNumber) const
{
	if (passNumber < 0 || passNumber >= m_passCount)
		return false;

	Pass const &pass = m_passes[passNumber];

	// The pass state the effect owns, with this material's stencil reference.
	if (m_implementation && m_implementation->m_graphicsData)
	{
		Direct3d11_ShaderImplementationData const * const implementationData =
			static_cast<Direct3d11_ShaderImplementationData const *>(m_implementation->m_graphicsData);

		implementationData->apply(passNumber, pass.stencilReferenceValid ? pass.stencilReference : 0);

		// The alpha test needs both halves: the compare function is the effect's and the
		// reference is this material's. A pass that does not test pushes Always, which the
		// derivation turns into a clip that never discards -- pushed rather than skipped,
		// because otherwise the previous material's test would still be in the buffer.
		if (implementationData->isAlphaTestEnabled(passNumber))
			Direct3d11_ConstantBuffers::setAlphaTest(implementationData->getAlphaTestFunction(passNumber), static_cast<int>(pass.alphaTestReference));
		else
			Direct3d11_ConstantBuffers::setAlphaTest(static_cast<int>(ShaderImplementation::Pass::C_Always), 0);
	}

	// ------------------------------------------------------------------
	// The vertex program, its bytecode for the input layout, and the routing

	ID3D11VertexShader *vertexShader = NULL;
	Direct3d11_VertexShaderData const *vertexData = NULL;

	if (pass.vertexShader)
	{
		vertexData = static_cast<Direct3d11_VertexShaderData const *>(pass.vertexShader->m_graphicsData);
		if (vertexData)
			vertexShader = vertexData->getVertexShader();
	}

	Direct3d11_StateCache::setVertexShader(vertexShader);

	if (vertexData)
		Direct3d11::setCurrentVertexShaderBytecode(vertexData->getBytecode(), vertexData->getBytecodeSize());
	else
		Direct3d11::setCurrentVertexShaderBytecode(NULL, 0);

	// This is where the specialisation DX9 compiled into the shader is handed over.
	Direct3d11::setCurrentTextureCoordinateSetMapping(pass.textureCoordinateSetMapping, pass.textureCoordinateSetMappingCount);

	// A material carrying precalculated vertex lighting wants the ambient register forced to
	// white, because its vertex colours already contain the lighting and must not be lit twice.
	Direct3d11_LightManager::setFullAmbientOn(pass.fullAmbient);

	// ------------------------------------------------------------------
	// Constants

	if (pass.materialValid)
	{
		Direct3d11_ConstantBuffers::setVertexShaderConstants(VSCR_material, pass.material, 5);

		// The pixel side wants the specular colour and the power, and in the engine's pixel
		// layout the power is one component of a register shared with the dot3 light
		// direction -- so it is written as a component, not a row.
		Direct3d11_ConstantBuffers::setPixelShaderConstants(PSCR_materialSpecularColor, pass.material + 8, 1);

		// The power is recorded, not uploaded. In the engine's pixel layout it is the w
		// component of the dot3 light direction register, which the light manager owns and
		// rewrites wholesale -- so writing it here as well would be overwritten, and the two
		// writers would race on which ran last. DX9 records it in exactly the same place.
		Direct3d11_StateCache::setSpecularPower(pass.material[16]);
	}

	if (pass.textureFactorValid)
	{
		Direct3d11_ConstantBuffers::setVertexShaderConstants(VSCR_textureFactor, pass.textureFactor, 2);
		Direct3d11_ConstantBuffers::setPixelShaderConstants(PSCR_textureFactor, pass.textureFactor, 2);
	}

	if (pass.textureScrollValid)
	{
		// The stored values are rates; the register wants the current offset, wrapped to the
		// fractional part so it does not lose precision as the session runs on.
		float const currentTime = Direct3d11::getCurrentTimeValue();

		float scroll[4];
		for (int i = 0; i < 4; ++i)
		{
			double whole = 0.0;
			scroll[i] = static_cast<float>(modf(static_cast<double>(pass.textureScroll[i]) * static_cast<double>(currentTime), &whole));
		}

		Direct3d11_ConstantBuffers::setVertexShaderConstants(VSCR_textureScroll, scroll, 1);
	}

	// ------------------------------------------------------------------
	// Textures and samplers
	//
	// The sampler goes to its own slot; the TEXTURE goes to whichever slot the compiled
	// program paired with that sampler, which is not generally the same number.

	Direct3d11_PixelShaderProgramData const *pixelData = NULL;
	if (pass.pixelShaderProgram)
		pixelData = static_cast<Direct3d11_PixelShaderProgramData const *>(pass.pixelShaderProgram->m_graphicsData);

	for (int i = 0; i < pass.stageCount; ++i)
	{
		Stage const &stage = pass.stages[i];

		if (stage.placeholder)
		{
			DEBUG_WARNING(true, ("Direct3d11: rendering with a placeholder texture."));
			const_cast<Stage &>(stage).placeholder = false;
		}

		int const textureSlot = pixelData ? pixelData->getTextureSlotForSampler(stage.samplerSlot) : -1;
		if (textureSlot < 0)
			continue;

		ID3D11ShaderResourceView *view = NULL;
		if (stage.texture && *stage.texture)
		{
			Direct3d11_TextureData const * const textureData = *stage.texture;
			view = textureData->getShaderResourceView();

			// D3D11 links a texture read to a declared resource type. D3D9 before ps_2_0 did
			// not: `tex t1` sampled whatever kind of texture happened to be bound, so a
			// converted program has no dimension to carry over and gets Texture2D by default.
			// Where the engine then binds a cube map, every draw is rejected -- and the debug
			// layer's report names a slot number and no program, which is not enough to act on.
			D3D_SRV_DIMENSION const declared = pixelData->getTextureDimensionForSlot(textureSlot);
			D3D_SRV_DIMENSION const actual = textureData->getViewDimension();

			if (declared != D3D_SRV_DIMENSION_UNKNOWN && declared != actual && !pixelData->hasReportedDimensionMismatch(textureSlot))
			{
				pixelData->noteReportedDimensionMismatch(textureSlot);
				WARNING(true, ("Direct3d11: '%s' declares %s for texture slot %d (D3D9 sampler stage %d) but the bound texture is %s. D3D11 rejects the read, so this draw does not render. The program's sampler declaration has to match the kind of texture the materials using it bind.",
					pass.pixelShaderProgram->getFileName(),
					describeSrvDimension(declared), textureSlot, stage.samplerSlot,
					describeSrvDimension(actual)));
			}
		}

		Direct3d11_StateCache::setShaderResource(textureSlot, view);

		if (stage.sampler)
			Direct3d11_StateCache::setSamplerState(stage.samplerSlot, stage.sampler);
	}

	// DX9's VSPS path returns true unconditionally; the answer means "a vertex shader is in
	// charge", which for this backend is only true when one was actually bound.
	return vertexShader != NULL;
}

// ======================================================================
