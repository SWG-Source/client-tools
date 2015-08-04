// ======================================================================
//
// Direct3d9_StaticShaderData.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstDirect3d9.h"
#include "Direct3d9_StaticShaderData.h"

#include "Direct3d9.h"
#include "Direct3d9_LightManager.h"
#include "Direct3d9_Metrics.h"
#include "Direct3d9_PixelShaderConstantRegisters.h"
#include "Direct3d9_ShaderImplementationData.h"
#include "Direct3d9_StateCache.h"
#include "Direct3d9_TextureData.h"
#include "Direct3d9_VertexShaderConstantRegisters.h"
#include "Direct3d9_VertexShaderData.h"

#include "clientGraphics/Material.h"
#include "clientGraphics/ShaderEffect.h"
#include "clientGraphics/ShaderImplementation.h"
#include "sharedDebug/DataLint.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/CrcLowerString.h"

#include <d3d9.h>
#include <d3dx9.h>
#include <stdio.h>

#ifdef _DEBUG
#include <string>
#endif

// ======================================================================

namespace Direct3d9_StaticShaderDataNamespace
{
	const Tag TAG_A255 = TAG(A,2,5,5);
	const Tag TAG_A128 = TAG(A,1,2,8);
	const Tag TAG_A001 = TAG(A,0,0,1);
	const Tag TAG_A000 = TAG(A,0,0,0);

	static const D3DTEXTUREADDRESS TextureAddress[] =
	{
		D3DTADDRESS_WRAP,                          // TA_wrap
		D3DTADDRESS_MIRROR,                        // TA_mirror
		D3DTADDRESS_CLAMP,                         // TA_clamp
		D3DTADDRESS_BORDER,                        // TA_border
		D3DTADDRESS_MIRRORONCE,                    // TA_mirrorOnce

		D3DTADDRESS_WRAP,                          // TA_invalid
	};

	static const D3DTEXTUREFILTERTYPE TextureFilter[] =
	{
		D3DTEXF_NONE,                              // TF_none
		D3DTEXF_POINT,                             // TF_point
		D3DTEXF_LINEAR,                            // TF_linear
		D3DTEXF_ANISOTROPIC,                       // TF_anisotropic
		D3DTEXF_NONE,                              // TF_flatCubic
		D3DTEXF_NONE,                              // TF_gaussianCubic

		D3DTEXF_LINEAR                             // TF_invalid
	};

#ifdef FFP

	static const D3DTRANSFORMSTATETYPE TransformLookup[] =
	{
		D3DTS_TEXTURE0,
		D3DTS_TEXTURE1,
		D3DTS_TEXTURE2,
		D3DTS_TEXTURE3,
		D3DTS_TEXTURE4,
		D3DTS_TEXTURE5,
		D3DTS_TEXTURE6,
		D3DTS_TEXTURE7,
	};

	static D3DMATRIX ms_textureTransformMatrix;

#endif

	Direct3d9_StaticShaderData const * ms_active;	
	int                                ms_pass;	
#ifdef _DEBUG
	bool                               ms_disableStaticShaderCaching;
#endif
	bool                               ms_usesVertexShader;
}
using namespace Direct3d9_StaticShaderDataNamespace;

#ifdef _DEBUG
bool                                             Direct3d9_StaticShaderData::Pass::ms_useDefaultMaterial;
Direct3d9_StaticShaderData::Pass::PaddedMaterial Direct3d9_StaticShaderData::Pass::ms_defaultMaterial;
Direct3d9_StaticShaderData::Pass::PaddedMaterial Direct3d9_StaticShaderData::Pass::ms_debugMaterial;
#ifdef VSPS
VectorRgba                                       Direct3d9_StaticShaderData::Pass::ms_debugTextureFactorData[2];
#endif
#endif

// ======================================================================

#ifdef _DEBUG

static const char *ConvertTagToStaticString(Tag tag)
{
	static char buffer[5];
	ConvertTagToString(tag, buffer);
	return buffer;
}

#endif

// ======================================================================

Direct3d9_StaticShaderData::Stage::Stage()
:
	m_placeholder(false),
#ifdef _FFP
	m_textureCoordinateSetValid(false),
	m_textureScrollValid(false),
#endif
	m_texture(NULL)
#ifdef FFP
	,
	m_textureCoordinateSet(0)
#endif
{
#ifdef FFP
	m_textureScroll[0] = 0.0f;
	m_textureScroll[1] = 0.0f;
#endif
}

// ----------------------------------------------------------------------

#ifdef FFP

void Direct3d9_StaticShaderData::Stage::construct(const StaticShader &shader, const ShaderImplementation::Pass &pass, const ShaderImplementation::Pass::Stage &stage)
{
	if (stage.m_textureTag)
	{
		StaticShaderTemplate::TextureData textureData;
		bool result = shader.getTextureData(stage.m_textureTag, textureData);
		if (!result)
		{
			DEBUG_WARNING(true, ("%s Could not get texture data for [%s]", shader.getShaderTemplate().getName().getString(), ConvertTagToStaticString(stage.m_textureTag)));

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

		if (Direct3d9_TextureData::isGlobalTexture(stage.m_textureTag))
		{
			m_placeholder = false;
			m_texture = Direct3d9_TextureData::getGlobalTexture(stage.m_textureTag);
		}
		else
		{
			m_placeholder = textureData.placeholder;
			if (textureData.texture)
				m_texture = reinterpret_cast<Direct3d9_TextureData const * const *>(textureData.texture->getGraphicsDataAddress());
			else
				m_texture = NULL;
		}

		const uint count = 7;
		m_samplerStates.reserve(count);
		m_samplerStates.clear();

#define TA(a,b) StaticShaderTemplate::TextureAddress a = textureData.a != StaticShaderTemplate::TA_invalid ? textureData.a : static_cast<StaticShaderTemplate::TextureAddress>(stage.b)

		TA(addressU, m_textureAddressU);
		TA(addressV, m_textureAddressV);
		TA(addressW, m_textureAddressW);

#undef TA

		DEBUG_WARNING(DataLint::isEnabled () && (addressU == StaticShaderTemplate::TA_invalid || addressV == StaticShaderTemplate::TA_invalid || addressW == StaticShaderTemplate::TA_invalid), ("Old shader %s with new effect", shader.getStaticShaderTemplate().getName().getString()));

#define TF(f,g) StaticShaderTemplate::TextureFilter f  = textureData.f != StaticShaderTemplate::TF_invalid ? textureData.f : static_cast<StaticShaderTemplate::TextureFilter>(stage.g)

		TF(mipFilter,           m_textureMipFilter);
		TF(minificationFilter,  m_textureMinificationFilter);
		TF(magnificationFilter, m_textureMagnificationFilter);

#undef TF

		m_samplerStates.push_back(SamplerState(D3DSAMP_MAXANISOTROPY, clamp(static_cast<DWORD>(1), static_cast<DWORD>(textureData.maxAnisotropy), Direct3d9::getMaxAnisotropy())));

#define TSSM(tss, v, m) m_samplerStates.push_back(SamplerState(tss, m[v]))

		TSSM(D3DSAMP_ADDRESSU,   addressU,             TextureAddress);
		TSSM(D3DSAMP_ADDRESSV,   addressV,             TextureAddress);
		TSSM(D3DSAMP_ADDRESSW,   addressW,             TextureAddress);
		TSSM(D3DSAMP_MIPFILTER,  mipFilter,            TextureFilter);
		TSSM(D3DSAMP_MINFILTER,  minificationFilter,   TextureFilter);
		TSSM(D3DSAMP_MAGFILTER,  magnificationFilter,  TextureFilter);

#undef TSSM

		uint8 textureCoordinateSet;
		result = shader.getTextureCoordinateSet(stage.m_textureCoordinateSetTag, textureCoordinateSet);
		// E3Hack DEBUG_FATAL(!result, ("Could not get texture coordinate set"));
		if (!result)
			textureCoordinateSet = 1;

		m_textureCoordinateSet      = textureCoordinateSet;
		m_textureCoordinateSetValid = true;

		switch (stage.m_textureCoordinateGeneration)
		{
			case ShaderImplementation::Pass::Stage::CG_passThru:
				m_textureCoordinateSet |= D3DTSS_TCI_PASSTHRU;
				m_textureScrollValid = false;
				break;

			case ShaderImplementation::Pass::Stage::CG_cameraSpaceNormal:
				m_textureCoordinateSet |= D3DTSS_TCI_CAMERASPACENORMAL;
				m_textureScrollValid = false;
				break;

			case ShaderImplementation::Pass::Stage::CG_cameraSpacePosition:
				m_textureCoordinateSet |= D3DTSS_TCI_CAMERASPACEPOSITION;
				m_textureScrollValid = false;
				break;

			case ShaderImplementation::Pass::Stage::CG_cameraSpaceReflectionVector:
				m_textureCoordinateSet |= D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR;
				m_textureScrollValid = false;
				break;

			case ShaderImplementation::Pass::Stage::CG_scroll1:
				m_textureCoordinateSet |= D3DTSS_TCI_PASSTHRU;
				{
					StaticShaderTemplate::TextureScroll textureScroll;
					if (shader.getTextureScroll(pass.m_textureScrollTag, textureScroll))
					{
						m_textureScrollValid = true;
						m_textureScroll[0] = textureScroll.u1;
						m_textureScroll[1] = textureScroll.v1;
					}
					else
					{
						DEBUG_WARNING(true, ("Could not find texture scroll tag %s in shader %s", ConvertTagToStaticString(pass.m_textureScrollTag), shader.getName()));
						m_textureScrollValid = false;
					}
				}
				break;

			case ShaderImplementation::Pass::Stage::CG_scroll2:
				m_textureCoordinateSet |= D3DTSS_TCI_PASSTHRU;
				{
					StaticShaderTemplate::TextureScroll textureScroll;
					if (shader.getTextureScroll(pass.m_textureScrollTag, textureScroll))
					{
						m_textureScrollValid = true;
						m_textureScroll[0] = textureScroll.u2;
						m_textureScroll[1] = textureScroll.v2;
					}
					else
					{
						DEBUG_WARNING(true, ("Could not find texture scroll tag %s in shader %s", ConvertTagToStaticString(pass.m_textureScrollTag), shader.getName()));
						m_textureScrollValid = false;
					}
				}
				break;
		}
	}
	else
	{
		m_placeholder = false;
		m_texture = NULL;
		m_textureCoordinateSet = 0;
		m_textureCoordinateSetValid = false;
		m_textureScrollValid = false;
	}
}

#endif

// ----------------------------------------------------------------------

#ifdef VSPS

void Direct3d9_StaticShaderData::Stage::construct(const StaticShader &shader, const ShaderImplementation::Pass::PixelShader::TextureSampler &textureSampler)
{
	if (textureSampler.m_textureTag)
	{
		StaticShaderTemplate::TextureData textureData;
		const bool result = shader.getTextureData(textureSampler.m_textureTag, textureData);
		if (!result)
		{
			DEBUG_WARNING(true, ("%s Could not get texture data for [%s]", shader.getShaderTemplate().getName().getString(), ConvertTagToStaticString(textureSampler.m_textureTag)));

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

		if (Direct3d9_TextureData::isGlobalTexture(textureSampler.m_textureTag))
		{
			m_placeholder = false;
			m_texture = Direct3d9_TextureData::getGlobalTexture(textureSampler.m_textureTag);
		}
		else
		{
			m_placeholder = textureData.placeholder;
			if (textureData.texture)
				m_texture = reinterpret_cast<Direct3d9_TextureData const * const *>(textureData.texture->getGraphicsDataAddress());
			else
				m_texture = NULL;
		}

		const uint count = 7;
		m_samplerStates.reserve(count);
		m_samplerStates.clear();

#define TA(a,b) StaticShaderTemplate::TextureAddress a = textureData.a != StaticShaderTemplate::TA_invalid ? textureData.a : static_cast<StaticShaderTemplate::TextureAddress>(textureSampler.b)

		TA(addressU, m_textureAddressU);
		TA(addressV, m_textureAddressV);
		TA(addressW, m_textureAddressW);

#undef TA

		DEBUG_WARNING(DataLint::isEnabled () && (addressU == StaticShaderTemplate::TA_invalid || addressV == StaticShaderTemplate::TA_invalid || addressW == StaticShaderTemplate::TA_invalid), ("Old shader %s with new effect", shader.getStaticShaderTemplate().getName().getString()));

#define TF(f,g) StaticShaderTemplate::TextureFilter f  = textureData.f != StaticShaderTemplate::TF_invalid ? textureData.f : static_cast<StaticShaderTemplate::TextureFilter>(textureSampler.g)

		TF(mipFilter,           m_textureMipFilter);
		TF(minificationFilter,  m_textureMinificationFilter);
		TF(magnificationFilter, m_textureMagnificationFilter);

#undef TF

		m_samplerStates.push_back(SamplerState(D3DSAMP_MAXANISOTROPY, clamp(static_cast<DWORD>(1), static_cast<DWORD>(textureData.maxAnisotropy), Direct3d9::getMaxAnisotropy())));

#define TSSM(tss, v, m) m_samplerStates.push_back(SamplerState(tss, m[v]))

		TSSM(D3DSAMP_ADDRESSU,   addressU,             TextureAddress);
		TSSM(D3DSAMP_ADDRESSV,   addressV,             TextureAddress);
		TSSM(D3DSAMP_ADDRESSW,   addressW,             TextureAddress);
		TSSM(D3DSAMP_MIPFILTER,  mipFilter,            TextureFilter);
		TSSM(D3DSAMP_MINFILTER,  minificationFilter,   TextureFilter);
		TSSM(D3DSAMP_MAGFILTER,  magnificationFilter,  TextureFilter);

#undef TSSM
	}
	else
	{
		m_placeholder = false;
		m_texture = NULL;
	}

#ifdef FFP
	// texture coordinates may be used without textures with the texcoord pixel shader instruction
	m_textureCoordinateSet      = 0;
	m_textureCoordinateSetValid = false;
#endif
}

#endif

// ----------------------------------------------------------------------

bool Direct3d9_StaticShaderData::Stage::getTextureSortKey(int &value) const
{
	if (m_texture)
	{
		value = reinterpret_cast<int>((*m_texture)->getBaseTexture());
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------

void Direct3d9_StaticShaderData::Stage::apply(int stage) const
{
	if (m_placeholder)
	{
		DEBUG_WARNING(true, ("Trying to render with a place holder texture"));
		const_cast<Stage*>(this)->m_placeholder = false;
	}

	if (m_texture)
	{
		Direct3d9_StateCache::setTexture(stage, *m_texture);

		const SamplerStates::const_iterator end = m_samplerStates.end();
		for (SamplerStates::const_iterator i = m_samplerStates.begin(); i != end; ++i)
		{
			const SamplerState &sampler = *i;
			Direct3d9_StateCache::setSamplerState(stage, sampler.state, sampler.value);
		}
	}
	else
		Direct3d9_StateCache::setTexture(stage, NULL);

#ifdef FFP
#ifdef VSPS
	if (stage < 8)
#endif
	{

		if (m_textureCoordinateSetValid)
			Direct3d9_StateCache::setTextureStageState(stage, D3DTSS_TEXCOORDINDEX, m_textureCoordinateSet);

		if (m_textureScrollValid)
		{
			float const currentTime = Direct3d9::getCurrentTime();
			double junk;
			ms_textureTransformMatrix._31 = static_cast<float>(modf(m_textureScroll[0] * currentTime, &junk));
			ms_textureTransformMatrix._32 = static_cast<float>(modf(m_textureScroll[1] * currentTime, &junk));
			Direct3d9_StateCache::setTextureStageState(stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
			const HRESULT hresult = Direct3d9::getDevice()->SetTransform(TransformLookup[stage], &ms_textureTransformMatrix);
			FATAL_DX_HR("SetTransform(view) failed %s", hresult);
		}
		else
		{
			Direct3d9_StateCache::setTextureStageState(stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
		}
	}
#endif
}

// ======================================================================

void Direct3d9_StaticShaderData::Pass::install()
{
#ifdef FFP
	ms_textureTransformMatrix._11 = 1.0f;
	ms_textureTransformMatrix._12 = 0.0f;
	ms_textureTransformMatrix._13 = 0.0f;
	ms_textureTransformMatrix._14 = 0.0f;
	ms_textureTransformMatrix._21 = 0.0f;
	ms_textureTransformMatrix._22 = 1.0f;
	ms_textureTransformMatrix._23 = 0.0f;
	ms_textureTransformMatrix._24 = 0.0f;
	ms_textureTransformMatrix._31 = 0.0f;
	ms_textureTransformMatrix._32 = 0.0f;
	ms_textureTransformMatrix._33 = 1.0f;
	ms_textureTransformMatrix._34 = 0.0f;
	ms_textureTransformMatrix._41 = 0.0f;
	ms_textureTransformMatrix._42 = 0.0f;
	ms_textureTransformMatrix._43 = 0.0f;
	ms_textureTransformMatrix._44 = 1.0f;
#endif

#ifdef _DEBUG
	DEBUG_FATAL(PSCR_textureFactor + 1 != PSCR_textureFactor2, ("constants not next to each other"));

	DebugFlags::registerFlag(ms_useDefaultMaterial, "Direct3d9", "useDefaultMaterial");

	ms_defaultMaterial.material.Ambient.r  = 1.0f;
	ms_defaultMaterial.material.Ambient.g  = 1.0f;
	ms_defaultMaterial.material.Ambient.b  = 1.0f;
	ms_defaultMaterial.material.Ambient.a  = 1.0f;

	ms_defaultMaterial.material.Diffuse.r  = 1.0f;
	ms_defaultMaterial.material.Diffuse.g  = 1.0f;
	ms_defaultMaterial.material.Diffuse.b  = 1.0f;
	ms_defaultMaterial.material.Diffuse.a  = 1.0f;

	ms_defaultMaterial.material.Emissive.r = 0.0f;
	ms_defaultMaterial.material.Emissive.g = 0.0f;
	ms_defaultMaterial.material.Emissive.b = 0.0f;
	ms_defaultMaterial.material.Emissive.a = 0.0f;

	ms_defaultMaterial.material.Specular.r = 0.0f;
	ms_defaultMaterial.material.Specular.g = 0.0f;
	ms_defaultMaterial.material.Specular.b = 0.0f;
	ms_defaultMaterial.material.Specular.a = 0.0f;

	ms_defaultMaterial.material.Power      = 32.0f;
	ms_defaultMaterial.pad1                =  0.0f;
	ms_defaultMaterial.pad2                =  0.0f;
	ms_defaultMaterial.pad3                =  0.0f;

	ms_debugMaterial.material.Ambient.r  = 100.0f;
	ms_debugMaterial.material.Ambient.g  =   0.0f;
	ms_debugMaterial.material.Ambient.b  = 100.0f;
	ms_debugMaterial.material.Ambient.a  =   1.0f;

	ms_debugMaterial.material.Diffuse.r  = 100.0f;
	ms_debugMaterial.material.Diffuse.g  =   0.0f;
	ms_debugMaterial.material.Diffuse.b  = 100.0f;
	ms_debugMaterial.material.Diffuse.a  =   1.0f;

	ms_debugMaterial.material.Emissive.r = 100.0f;
	ms_debugMaterial.material.Emissive.g =   0.0f;
	ms_debugMaterial.material.Emissive.b = 100.0f;
	ms_debugMaterial.material.Emissive.a =   1.0f;

	ms_debugMaterial.material.Specular.r = 100.0f;
	ms_debugMaterial.material.Specular.g =   0.0f;
	ms_debugMaterial.material.Specular.b = 100.0f;
	ms_debugMaterial.material.Specular.a =   1.0f;

	ms_debugMaterial.material.Power      =  32.0f;
	ms_debugMaterial.pad1                =   0.0f;
	ms_debugMaterial.pad2                =   0.0f;
	ms_debugMaterial.pad3                =   0.0f;

#ifdef VSPS
	ms_debugTextureFactorData[0].r = 1.0f;
	ms_debugTextureFactorData[0].g = 0.0f;
	ms_debugTextureFactorData[0].b = 1.0f;
	ms_debugTextureFactorData[0].a = 1.0f;

	ms_debugTextureFactorData[1].r = 1.0f;
	ms_debugTextureFactorData[1].g = 0.0f;
	ms_debugTextureFactorData[1].b = 1.0f;
	ms_debugTextureFactorData[1].a = 1.0f;
#endif
#endif
}

// ----------------------------------------------------------------------

void Direct3d9_StaticShaderData::Pass::construct(const StaticShader &shader, const ShaderImplementation::Pass &pass)
{
#if defined(FFP) && defined(VSPS)
	if (pass.m_vertexShader)
#endif
	{
#if defined(VSPS)
		// pack the requested texture coordinate sets into uint32. Since each texture coordinate set index can only be a value between
		// 0 and 7, we can fit this into 3 bits.  Since D3D9 only supports 8 texture coordinate sets, this requires a total of 24 bits.
		// We process them in reverse order so we can index into the list by right-shifting.
		uint32 textureCoordinateSetKey = 0;
		ShaderImplementation::Pass::VertexShader const & vertexShader = *pass.m_vertexShader;
		Direct3d9_VertexShaderData const * vertexShaderData = safe_cast<Direct3d9_VertexShaderData const *>(vertexShader.m_graphicsData);
		Direct3d9_VertexShaderData::TextureCoordinateSetTags const * textureCoordinateSetTags = vertexShaderData->getTextureCoordinateSetTags();
		if (textureCoordinateSetTags)
		{
			const int numberOfTextureCoordinateSetTags = textureCoordinateSetTags->size();
			DEBUG_FATAL(numberOfTextureCoordinateSetTags > 8, ("too many texture coordinate sets"));
			for (int i = 0; i < numberOfTextureCoordinateSetTags ; ++i)
			{
				uint8 textureCoordinate = 0;
				if (!shader.getTextureCoordinateSet((*textureCoordinateSetTags)[i], textureCoordinate))
				{
					DEBUG_WARNING(true, ("Missing texture coordinate set tag %s for shader %s, defaulting to 0", ConvertTagToStaticString((*textureCoordinateSetTags)[i]), shader.getShaderTemplate().getName().getString()));
					textureCoordinate = 0;
				}
				
				if (textureCoordinate > 7)
				{
					DEBUG_WARNING(true, ("shader [%s]: texture coordinate out of range 0/%d/7, resetting to 0", shader.getName() ? shader.getName() : "<NULL shader name>", static_cast<int>(textureCoordinate)));
					textureCoordinate = 0;
				}

				textureCoordinateSetKey = textureCoordinateSetKey | (textureCoordinate << (i * 3));
			}
		}

#if PRODUCTION == 0
		m_textureCoordinateSetKey = textureCoordinateSetKey;
		m_vertexShader = pass.m_vertexShader;
#else
		m_vertexShader = vertexShaderData->getVertexShader(textureCoordinateSetKey);
#endif

#endif
	}
#if defined(FFP) && defined(VSPS)
	else
	{
		m_vertexShader = false;
	}
#endif

	if (pass.m_materialTag)
	{
		Material material;
		const bool result = shader.getMaterial(pass.m_materialTag, material);
		if (result)
		{
			m_material.material.Ambient.r  = material.getAmbientColor().r;
			m_material.material.Ambient.g  = material.getAmbientColor().g;
			m_material.material.Ambient.b  = material.getAmbientColor().b;
			m_material.material.Ambient.a  = material.getAmbientColor().a;

			m_material.material.Diffuse.r  = material.getDiffuseColor().r;
			m_material.material.Diffuse.g  = material.getDiffuseColor().g;
			m_material.material.Diffuse.b  = material.getDiffuseColor().b;
			m_material.material.Diffuse.a  = material.getDiffuseColor().a;

			m_material.material.Emissive.r = material.getEmissiveColor().r;
			m_material.material.Emissive.g = material.getEmissiveColor().g;
			m_material.material.Emissive.b = material.getEmissiveColor().b;
			m_material.material.Emissive.a = material.getEmissiveColor().a;

			m_material.material.Specular.r = material.getSpecularColor().r;
			m_material.material.Specular.g = material.getSpecularColor().g;
			m_material.material.Specular.b = material.getSpecularColor().b;
			m_material.material.Specular.a = material.getSpecularColor().a;

			m_material.material.Power      = material.getSpecularPower();
			m_material.pad1                = 0.0f;
			m_material.pad2                = 0.0f;
			m_material.pad3                = 0.0f;
		}
		else
		{
			DEBUG_WARNING(true, ("Could not find material tag %s in shader %s", ConvertTagToStaticString(pass.m_materialTag), shader.getName()));
			m_material.material.Ambient.r  = 1.0;
			m_material.material.Ambient.g  = 1.0;
			m_material.material.Ambient.b  = 1.0;
			m_material.material.Ambient.a  = 1.0;

			m_material.material.Diffuse.r  = 1.0;
			m_material.material.Diffuse.g  = 1.0;
			m_material.material.Diffuse.b  = 1.0;
			m_material.material.Diffuse.a  = 1.0;

			m_material.material.Emissive.r = 0.0;
			m_material.material.Emissive.g = 0.0;
			m_material.material.Emissive.b = 0.0;
			m_material.material.Emissive.a = 1.0;

			m_material.material.Specular.r = 0.0;
			m_material.material.Specular.g = 0.0;
			m_material.material.Specular.b = 0.0;
			m_material.material.Specular.a = 1.0;

			m_material.material.Power      = 0.0;
			m_material.pad1                = 0.0f;
			m_material.pad2                = 0.0f;
			m_material.pad3                = 0.0f;
		}

		m_materialValid       = true;
	}
	else
		m_materialValid = false;

	m_textureFactorValid = false;
	if (pass.m_textureFactorTag)
	{
		uint32 textureFactor = 0;
		const bool result = shader.getTextureFactor(pass.m_textureFactorTag, textureFactor);
		if (result)
		{
			m_textureFactorValid = true;

#ifdef FFP
			m_textureFactor = textureFactor;
#endif

#ifdef VSPS
			m_textureFactorData[0].r = static_cast<float>((textureFactor >> 16) & 0xff) / 255.0f;
			m_textureFactorData[0].g = static_cast<float>((textureFactor >>  8) & 0xff) / 255.0f;
			m_textureFactorData[0].b = static_cast<float>((textureFactor >>  0) & 0xff) / 255.0f;
			m_textureFactorData[0].a = static_cast<float>((textureFactor >> 24) & 0xff) / 255.0f;
#endif
		}
		else
			DEBUG_WARNING(true, ("Could not find texture factor %s in %s", ConvertTagToStaticString(pass.m_textureFactorTag), shader.getShaderTemplate().getName().getString()));
	}
	if (pass.m_textureFactorTag2)
	{
#if defined(FFP) && defined(VSPS)
		WARNING(!m_vertexShader, ("Shader has textureFactor2 but uses FFP"));
#endif
		WARNING(!pass.m_textureFactorTag, ("Shader has textureFactor2 but no textureFactor"));

#ifdef VSPS

		uint32 textureFactor2 = 0;
		const bool result = shader.getTextureFactor(pass.m_textureFactorTag2, textureFactor2);
		if (result)
		{
			m_textureFactorValid = true;

			m_textureFactorData[1].r = static_cast<float>((textureFactor2 >> 16) & 0xff) / 255.0f;
			m_textureFactorData[1].g = static_cast<float>((textureFactor2 >>  8) & 0xff) / 255.0f;
			m_textureFactorData[1].b = static_cast<float>((textureFactor2 >>  0) & 0xff) / 255.0f;
			m_textureFactorData[1].a = static_cast<float>((textureFactor2 >> 24) & 0xff) / 255.0f;
		}
		else
			DEBUG_WARNING(true, ("Could not find texture factor %s in %s", ConvertTagToStaticString(pass.m_textureFactorTag), shader.getShaderTemplate().getName().getString()));
#endif
	}

	if (pass.m_textureScrollTag)
	{
		StaticShaderTemplate::TextureScroll textureScroll;
		if (shader.getTextureScroll(pass.m_textureScrollTag, textureScroll))
		{
			m_textureScrollValid = true;
			m_textureScroll[0] = textureScroll.u1;
			m_textureScroll[1] = textureScroll.v1;
			m_textureScroll[2] = textureScroll.u2;
			m_textureScroll[3] = textureScroll.v2;
		}
		else
		{
			DEBUG_WARNING(true, ("Could not find texture scroll tag %s for shader %s", ConvertTagToStaticString(pass.m_textureScrollTag), shader.getName()));
			m_textureScrollValid = true;
			m_textureScroll[0] = 0.0f;
			m_textureScroll[1] = 0.0f;
			m_textureScroll[2] = 0.0f;
			m_textureScroll[3] = 0.0f;
		}
	}
	else
	{
#ifdef _DEBUG
		m_textureScrollValid = false;
		m_textureScroll[0] = 0.0f;
		m_textureScroll[1] = 0.0f;
		m_textureScroll[2] = 0.0f;
		m_textureScroll[3] = 0.0f;
#endif

		m_textureScrollValid = false;
	}

	if (pass.m_alphaTestEnable)
	{
		m_alphaTestReferenceValueValid = true;
		if (pass.m_alphaTestReferenceValueTag == TAG_A255)
			m_alphaTestReferenceValue = 255;
		else
			if (pass.m_alphaTestReferenceValueTag == TAG_A128)
				m_alphaTestReferenceValue = 128;
			else
				if (pass.m_alphaTestReferenceValueTag == TAG_A001)
					m_alphaTestReferenceValue = 1;
				else
					if (pass.m_alphaTestReferenceValueTag == TAG_A000)
						m_alphaTestReferenceValue = 0;
					else
						if (!shader.getAlphaTestReferenceValue(pass.m_alphaTestReferenceValueTag, m_alphaTestReferenceValue))
						{
							// @todo @e3hack put this back in
							DEBUG_WARNING(true, ("Could not find alpha reference value %s for shader %s, defaulting to 1", ConvertTagToStaticString(pass.m_alphaTestReferenceValueTag), shader.getName()));
							m_alphaTestReferenceValue = 1;
						}
	}
	else
		m_alphaTestReferenceValueValid = false;

	if (pass.m_stencilEnable)
	{
		const bool result = shader.getStencilReferenceValue(pass.m_stencilReferenceValueTag, m_stencilReferenceValue);
		if (result)
			m_stencilReferenceValueValid = true;
		else
			DEBUG_WARNING(true, ("Could not find stencil reference value"));
	}
	else
		m_stencilReferenceValueValid = false;

	m_fullAmbient = shader.containsPrecalculatedVertexLighting();
	m_fogMode = pass.m_fogMode;

#if defined(FFP) && defined(VSPS)
	if (pass.m_pixelShader)
#endif
	{
#ifdef VSPS
		// construct the shared data for each texture texture sample ("stage")
		// not all source texture stages have to exist, but we'll create all the destination "stages" so that the rendering code can be more simple.
		m_stage.resize(pass.m_pixelShader->m_maxTextureSampler + 1);
		ShaderImplementation::Pass::PixelShader::TextureSamplers::const_iterator end = pass.m_pixelShader->m_textureSamplers->end();
		for (ShaderImplementation::Pass::PixelShader::TextureSamplers::const_iterator i = pass.m_pixelShader->m_textureSamplers->begin(); i != end; ++i)
			m_stage[(*i)->m_textureIndex].construct(shader, **i);
#endif
	}
#if defined(FFP) && defined(VSPS)
	else
#endif
	{
#if FFP
		m_stage.resize(pass.m_stage->size());
		ShaderImplementation::Pass::Stages::const_iterator j = pass.m_stage->begin();
		const Stages::iterator end = m_stage.end();
		for (Stages::iterator i = m_stage.begin(); i != end; ++i, ++j)
			i->construct(shader, pass, **j);
#endif
	}
}

// ----------------------------------------------------------------------

bool Direct3d9_StaticShaderData::Pass::getTextureSortKey(int &value) const
{
	Stages::const_iterator end = m_stage.end();
	for (Stages::const_iterator i = m_stage.begin(); i != end; ++i)
		if (i->getTextureSortKey(value))
			return true;

	return false;
}

// ----------------------------------------------------------------------

bool Direct3d9_StaticShaderData::Pass::apply() const
{
#ifdef FFP
	IDirect3DDevice9  *device = Direct3d9::getDevice();
#endif

#ifdef VSPS
#if PRODUCTION == 0
	IDirect3DVertexShader9 * vertexShader = m_vertexShader == 0 ?
		0 : static_cast<Direct3d9_VertexShaderData const *>(m_vertexShader->m_graphicsData)->getVertexShader(m_textureCoordinateSetKey);
	Direct3d9_StateCache::setVertexShader(vertexShader);
#else
	Direct3d9_StateCache::setVertexShader(m_vertexShader);
#endif
#endif
	if (m_materialValid)
	{
#ifdef _DEBUG
		if (ms_useDefaultMaterial)
		{
#ifdef FFP
			const HRESULT hresult = device->SetMaterial(&ms_defaultMaterial.material);
			FATAL_DX_HR("SetMaterial failed %s", hresult);
#endif

#ifdef VSPS
			DEBUG_FATAL(sizeof(m_material) != sizeof(float) * 4 * 5, ("PaddedMaterial size is wrong %d/%d", sizeof(m_material), sizeof(float) * 4 * 5));
			Direct3d9_StateCache::setVertexShaderConstants(VSCR_material, &ms_defaultMaterial, 5);
			Direct3d9_StateCache::setPixelShaderConstants(PSCR_materialSpecularColor, &ms_defaultMaterial.material.Specular, 1);
			Direct3d9_StateCache::setSpecularPower(ms_defaultMaterial.material.Power);
#endif
		}
		else
#endif
		{
#ifdef FFP
			const HRESULT hresult = device->SetMaterial(&m_material.material);
			FATAL_DX_HR("SetMaterial failed %s", hresult);
#endif

#ifdef VSPS
			DEBUG_FATAL(sizeof(m_material) != sizeof(float) * 4 * 5, ("PaddedMaterial size is wrong %d/%d", sizeof(m_material), sizeof(float) * 4 * 5));
			Direct3d9_StateCache::setVertexShaderConstants(VSCR_material, &m_material, 5);
			Direct3d9_StateCache::setPixelShaderConstants(PSCR_materialSpecularColor, &m_material.material.Specular, 1);
			Direct3d9_StateCache::setSpecularPower(m_material.material.Power);
#endif
		}

#ifdef _DEBUG
		Direct3d9_Metrics::setMaterialCalls += 1;
#endif
	}
#if DEBUG_LEVEL == DEBUG_LEVEL_DEBUG
	else
	{
#ifdef FFP
		const HRESULT hresult = device->SetMaterial(&ms_debugMaterial.material);
		FATAL_DX_HR("SetMaterial failed %s", hresult);
#endif

#ifdef VSPS
		DEBUG_FATAL(sizeof(m_material) != sizeof(float) * 4 * 5, ("PaddedMaterial size is wrong %d/%d", sizeof(m_material), sizeof(float) * 4 * 5));
		Direct3d9_StateCache::setVertexShaderConstants(VSCR_material, &ms_debugMaterial, 5);
		Direct3d9_StateCache::setPixelShaderConstants(PSCR_materialSpecularColor, &ms_debugMaterial.material.Specular, 1);
		Direct3d9_StateCache::setSpecularPower(ms_debugMaterial.material.Power);
#endif
	}
#endif

	if (m_textureFactorValid)
	{
#ifdef FFP
		Direct3d9_StateCache::setRenderState(D3DRS_TEXTUREFACTOR, m_textureFactor);
#endif
#ifdef VSPS
		Direct3d9_StateCache::setVertexShaderConstants(VSCR_textureFactor, &m_textureFactorData, 2);
		Direct3d9_StateCache::setPixelShaderConstants(PSCR_textureFactor, &m_textureFactorData, 2);
#endif
	}
#if DEBUG_LEVEL == DEBUG_LEVEL_DEBUG
	else
	{
#ifdef FFP
		Direct3d9_StateCache::setRenderState(D3DRS_TEXTUREFACTOR, static_cast<DWORD>(0xffff00ff));
#endif
#ifdef VSPS
		Direct3d9_StateCache::setVertexShaderConstants(VSCR_textureFactor, &ms_debugTextureFactorData, 2);
		Direct3d9_StateCache::setPixelShaderConstants(PSCR_textureFactor, &ms_debugTextureFactorData, 2);
#endif
	}
#endif

	if (m_textureScrollValid)
	{
#ifdef VSPS
		float scroll[4];
		float const currentTime = Direct3d9::getCurrentTime();
		double junk;
		scroll[0] = static_cast<float>(modf(m_textureScroll[0] * currentTime, &junk));
		scroll[1] = static_cast<float>(modf(m_textureScroll[1] * currentTime, &junk));
		scroll[2] = static_cast<float>(modf(m_textureScroll[2] * currentTime, &junk));
		scroll[3] = static_cast<float>(modf(m_textureScroll[3] * currentTime, &junk));
		Direct3d9_StateCache::setVertexShaderConstants(VSCR_textureScroll, scroll, 1);
#endif
	}

	if (m_alphaTestReferenceValueValid)
		Direct3d9::setAlphaTestReferenceValue(m_alphaTestReferenceValue);

	if (m_stencilReferenceValueValid)
		Direct3d9_StateCache::setRenderState(D3DRS_STENCILREF, m_stencilReferenceValue);

	switch (m_fogMode)
	{
		case ShaderImplementation::Pass::FM_Normal:
 			Direct3d9_StateCache::setRenderState(D3DRS_FOGCOLOR, Direct3d9::getFogColor());
			break;

		case ShaderImplementation::Pass::FM_Black:
 			Direct3d9_StateCache::setRenderState(D3DRS_FOGCOLOR, 0);
			break;

		case ShaderImplementation::Pass::FM_White:
 			Direct3d9_StateCache::setRenderState(D3DRS_FOGCOLOR, static_cast<DWORD>(0xffffffff));
			break;

		default:
			DEBUG_FATAL(true, ("Unknown fog mode"));
	}

	int stageNumber = 0;
	Stages::const_iterator end = m_stage.end();
	for (Stages::const_iterator i = m_stage.begin(); i != end; ++i, ++stageNumber)
		i->apply(stageNumber);

	Direct3d9_LightManager::setFullAmbientOn(m_fullAmbient);

#if defined(FFP) && defined(VSPS)
	return m_vertexShader != NULL;
#endif
#ifdef FFP
	return false;
#endif
#ifdef VSPS
	return true;
#endif
}

// ======================================================================

void Direct3d9_StaticShaderData::install()
{
	Pass::install();
#ifdef _DEBUG
	DebugFlags::registerFlag(ms_disableStaticShaderCaching, "Direct3d9", "disableStaticShaderCaching");
#endif
}

// ----------------------------------------------------------------------

void Direct3d9_StaticShaderData::beginFrame()
{
	ms_active = NULL;
	ms_pass = -1;
}

// ----------------------------------------------------------------------

Direct3d9_StaticShaderData::Direct3d9_StaticShaderData(const StaticShader &shader)
: StaticShaderGraphicsData(),
	m_implementation(shader.getStaticShaderTemplate().m_effect->m_implementation)
{
	construct(shader);
}

// ----------------------------------------------------------------------

Direct3d9_StaticShaderData::~Direct3d9_StaticShaderData()
{
	if (ms_active == this)
		ms_active = NULL;
}

// ----------------------------------------------------------------------

void Direct3d9_StaticShaderData::construct(const StaticShader &shader)
{
	m_pass.resize(m_implementation->m_pass->size());
	ShaderImplementation::Passes::const_iterator j = m_implementation->m_pass->begin();
	const Passes::iterator end = m_pass.end();
	for (Passes::iterator i = m_pass.begin(); i != end; ++i, ++j)
		i->construct(shader, **j);
}

// ----------------------------------------------------------------------

int Direct3d9_StaticShaderData::getTextureSortKey() const
{
	const Passes::const_iterator end = m_pass.end();
	for (Passes::const_iterator i = m_pass.begin(); i != end; ++i)
	{
		int value = 0;
		if (i->getTextureSortKey(value))
			return value;
	}

	return 0;
}

// ----------------------------------------------------------------------

void Direct3d9_StaticShaderData::update(const StaticShader &shader)
{
	construct(shader);

	// if you change the active shader, make sure it gets updated
	if (ms_active == this)
	{
		ms_active = NULL;
		apply(ms_pass);
	}
}

// ----------------------------------------------------------------------

bool Direct3d9_StaticShaderData::apply(int pass) const
{
	if (
#ifdef _DEBUG
		ms_disableStaticShaderCaching ||  
#endif
		ms_active != this || ms_pass != pass)
	{
		ms_active = this;
		ms_pass = pass;
		static_cast<const Direct3d9_ShaderImplementationData *>(m_implementation->m_graphicsData)->apply(pass);
		ms_usesVertexShader = m_pass[pass].apply();
	}
#ifdef _DEBUG
	else
	{
		Direct3d9_Metrics::reusedStaticShaders += 1;		
	}
#endif

	return ms_usesVertexShader;
}

// ======================================================================
