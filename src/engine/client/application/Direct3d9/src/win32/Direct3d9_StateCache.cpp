// ======================================================================
//
// Direct3d9_StateCache.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstDirect3d9.h"
#include "Direct3d9_StateCache.h"

#include "Direct3d9_TextureData.h"
#include "Direct3d9_VertexShaderConstantRegisters.h"
#include "PaddedVector.h"

#include "sharedDebug/DebugFlags.h"
#include "Direct3d9.h"

// ======================================================================

IDirect3DDevice9                          *Direct3d9_StateCache::ms_device;
Direct3d9_StateCache::RenderStateCache     Direct3d9_StateCache::ms_renderStateCache;
Direct3d9_StateCache::SamplerCache         Direct3d9_StateCache::ms_samplerStateCache;
Direct3d9_StateCache::TextureStagesCache   Direct3d9_StateCache::ms_textureStagesStateCache;
Direct3d9_StateCache::TextureCache         Direct3d9_StateCache::ms_textureCache;
IDirect3DVertexDeclaration9               *Direct3d9_StateCache::ms_vertexDeclaration;
Direct3d9_StateCache::StreamData          *Direct3d9_StateCache::ms_streamDataCache;
int                                        Direct3d9_StateCache::ms_maxStreamCount;
IDirect3DIndexBuffer9                     *Direct3d9_StateCache::ms_indexBuffer;
UINT                                       Direct3d9_StateCache::ms_indexBufferOffset;

float                                      Direct3d9_StateCache::ms_specularPower;

#ifdef VSPS
IDirect3DVertexShader9                    *Direct3d9_StateCache::ms_vertexShader;
IDirect3DPixelShader9                     *Direct3d9_StateCache::ms_pixelShader;
#endif

#ifdef _DEBUG
bool                                       Direct3d9_StateCache::ms_noTextures;
#endif

// ======================================================================

void Direct3d9_StateCache::install(int maxStreamCount)
{
	ms_device = Direct3d9::getDevice();

#define RS(state,value) forceRenderState(state, value)
#define TSS(state,value) forceTextureStageState(i, state, value)
#define SSS(state,value) forceSamplerState(i, state, value)

	RS(D3DRS_ZENABLE,                           TRUE);
	RS(D3DRS_FILLMODE,                          D3DFILL_SOLID);
	RS(D3DRS_SHADEMODE,                         D3DSHADE_GOURAUD);
	RS(D3DRS_ZWRITEENABLE,                      TRUE);
	RS(D3DRS_ALPHATESTENABLE,                   FALSE);
	RS(D3DRS_LASTPIXEL,                         TRUE);
	RS(D3DRS_SRCBLEND,                          D3DBLEND_ONE);
	RS(D3DRS_DESTBLEND,                         D3DBLEND_ZERO);
	RS(D3DRS_CULLMODE,                          D3DCULL_CCW);
	RS(D3DRS_ZFUNC,                             D3DCMP_LESSEQUAL);
	RS(D3DRS_ALPHAREF,                          0);
	RS(D3DRS_ALPHAFUNC,                         D3DCMP_ALWAYS);
	RS(D3DRS_DITHERENABLE,                      FALSE);
	RS(D3DRS_ALPHABLENDENABLE,                  FALSE);
	RS(D3DRS_FOGENABLE,                         FALSE);
	RS(D3DRS_SPECULARENABLE,                    FALSE);
	RS(D3DRS_FOGCOLOR,                          0);
	RS(D3DRS_FOGTABLEMODE,                      D3DFOG_NONE);
	RS(D3DRS_FOGSTART,                          0.0f);
	RS(D3DRS_FOGEND,                            1.0f);
	RS(D3DRS_FOGDENSITY,                        1.0f);
	RS(D3DRS_RANGEFOGENABLE,                    TRUE);
	RS(D3DRS_STENCILENABLE,                     FALSE);
	RS(D3DRS_STENCILFAIL,                       D3DSTENCILOP_KEEP);
	RS(D3DRS_STENCILZFAIL,                      D3DSTENCILOP_KEEP);
	RS(D3DRS_STENCILPASS,                       D3DSTENCILOP_KEEP);
	RS(D3DRS_STENCILFUNC,                       D3DCMP_ALWAYS);
	RS(D3DRS_STENCILREF,                        0);
	RS(D3DRS_STENCILMASK,                       static_cast<DWORD>(0xffffffff));
	RS(D3DRS_STENCILWRITEMASK,                  static_cast<DWORD>(0xffffffff));
	RS(D3DRS_TEXTUREFACTOR,                     static_cast<DWORD>(0xffffffff));
	RS(D3DRS_WRAP0,                             0);
	RS(D3DRS_WRAP1,                             0);
	RS(D3DRS_WRAP2,                             0);
	RS(D3DRS_WRAP3,                             0);
	RS(D3DRS_WRAP4,                             0);
	RS(D3DRS_WRAP5,                             0);
	RS(D3DRS_WRAP6,                             0);
	RS(D3DRS_WRAP7,                             0);
	RS(D3DRS_CLIPPING,                          TRUE);
	RS(D3DRS_LIGHTING,                          TRUE);
	RS(D3DRS_AMBIENT,                           0x00000000);
	RS(D3DRS_FOGVERTEXMODE,                     D3DFOG_NONE);
	RS(D3DRS_COLORVERTEX,                       TRUE);
	RS(D3DRS_LOCALVIEWER,                       TRUE);
	RS(D3DRS_NORMALIZENORMALS,                  TRUE);
	RS(D3DRS_DIFFUSEMATERIALSOURCE,             D3DMCS_COLOR1);
	RS(D3DRS_SPECULARMATERIALSOURCE,            D3DMCS_COLOR2);
	RS(D3DRS_AMBIENTMATERIALSOURCE,             D3DMCS_MATERIAL);
	RS(D3DRS_EMISSIVEMATERIALSOURCE,            D3DMCS_MATERIAL);
	RS(D3DRS_VERTEXBLEND,                       D3DVBF_DISABLE);
	RS(D3DRS_CLIPPLANEENABLE,                   0);
	RS(D3DRS_POINTSIZE,                         1.0f);
	RS(D3DRS_POINTSIZE_MIN,                     1.0f);
	RS(D3DRS_POINTSPRITEENABLE,                 FALSE);
	RS(D3DRS_POINTSCALEENABLE,                  FALSE);
	RS(D3DRS_POINTSCALE_A,                      1.0f);
	RS(D3DRS_POINTSCALE_B,                      0.0f);
	RS(D3DRS_POINTSCALE_C,                      0.0f);
	RS(D3DRS_MULTISAMPLEANTIALIAS,              TRUE);
	RS(D3DRS_MULTISAMPLEMASK,                   static_cast<DWORD>(0xffffffff));
	RS(D3DRS_PATCHEDGESTYLE,                    D3DPATCHEDGE_DISCRETE);
	RS(D3DRS_DEBUGMONITORTOKEN,                 D3DDMT_ENABLE);
	RS(D3DRS_POINTSIZE_MAX,                     64.0f);
	RS(D3DRS_INDEXEDVERTEXBLENDENABLE,          FALSE);
	RS(D3DRS_COLORWRITEENABLE,                  D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_RED);
	RS(D3DRS_TWEENFACTOR,                       0.0f);
	RS(D3DRS_BLENDOP,                           D3DBLENDOP_ADD);
	RS(D3DRS_POSITIONDEGREE,                    D3DDEGREE_CUBIC);
	RS(D3DRS_NORMALDEGREE,                      D3DDEGREE_LINEAR);
	RS(D3DRS_SCISSORTESTENABLE,                 FALSE);
	RS(D3DRS_SLOPESCALEDEPTHBIAS,               0);
	RS(D3DRS_ANTIALIASEDLINEENABLE,             FALSE);
	RS(D3DRS_MINTESSELLATIONLEVEL,              1.0f);
	RS(D3DRS_MAXTESSELLATIONLEVEL,              1.0f);
	RS(D3DRS_ADAPTIVETESS_X,                    0.0f);
	RS(D3DRS_ADAPTIVETESS_Y,                    0.0f);
	RS(D3DRS_ADAPTIVETESS_Z,                    1.0f);
	RS(D3DRS_ADAPTIVETESS_W,                    0.0f);
//	RS(D3DRS_ENABLEADAPTIVETESSELATION,         FALSE);
	RS(D3DRS_TWOSIDEDSTENCILMODE,               FALSE);
	RS(D3DRS_CCW_STENCILFAIL,                   D3DSTENCILOP_KEEP);
	RS(D3DRS_CCW_STENCILZFAIL,                  D3DSTENCILOP_KEEP);
	RS(D3DRS_CCW_STENCILPASS,                   D3DSTENCILOP_KEEP);
	RS(D3DRS_CCW_STENCILFUNC,                   D3DCMP_ALWAYS);
	RS(D3DRS_COLORWRITEENABLE1,                 D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_RED);
	RS(D3DRS_COLORWRITEENABLE2,                 D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_RED);
	RS(D3DRS_COLORWRITEENABLE3,                 D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_RED);
	RS(D3DRS_BLENDFACTOR,                       static_cast<DWORD>(0xffffffff));
	RS(D3DRS_SRGBWRITEENABLE,                   0);
	RS(D3DRS_DEPTHBIAS,                         0.0f);
	RS(D3DRS_WRAP8,                             0);
	RS(D3DRS_WRAP9,                             0);
	RS(D3DRS_WRAP10,                            0);
	RS(D3DRS_WRAP11,                            0);
	RS(D3DRS_WRAP12,                            0);
	RS(D3DRS_WRAP13,                            0);
	RS(D3DRS_WRAP14,                            0);
	RS(D3DRS_WRAP15,                            0);
	RS(D3DRS_SEPARATEALPHABLENDENABLE,          FALSE);
	RS(D3DRS_SRCBLENDALPHA,                     D3DBLEND_ONE);
	RS(D3DRS_DESTBLENDALPHA,                    D3DBLEND_ZERO);
	RS(D3DRS_BLENDOPALPHA,                      D3DBLENDOP_ADD);

	{
		for (int i = 0; i < cms_stages; ++i)
		{
			TSS(D3DTSS_COLOROP,               D3DTOP_DISABLE);
			TSS(D3DTSS_COLORARG1,             D3DTA_TEXTURE);
			TSS(D3DTSS_COLORARG2,             D3DTA_TEXTURE);
			TSS(D3DTSS_ALPHAOP,               D3DTOP_DISABLE);
			TSS(D3DTSS_ALPHAARG1,             D3DTA_TEXTURE);
			TSS(D3DTSS_ALPHAARG2,             D3DTA_TEXTURE);
			TSS(D3DTSS_BUMPENVMAT00,          0.0f);
			TSS(D3DTSS_BUMPENVMAT01,          0.0f);
			TSS(D3DTSS_BUMPENVMAT10,          0.0f);
			TSS(D3DTSS_BUMPENVMAT11,          0.0f);
			TSS(D3DTSS_TEXCOORDINDEX,         i | D3DTSS_TCI_PASSTHRU);
			TSS(D3DTSS_BUMPENVLSCALE,         0.0f);
			TSS(D3DTSS_BUMPENVLOFFSET,        0.0f);
			TSS(D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
			TSS(D3DTSS_COLORARG0,             D3DTA_TEMP);
			TSS(D3DTSS_ALPHAARG0,             D3DTA_TEMP);
			TSS(D3DTSS_RESULTARG,             D3DTA_CURRENT);
			TSS(D3DTSS_CONSTANT,              0x00000000);
		}
	}

	{
		for (int i = 0; i < cms_samplers; ++i)
		{
			SSS(D3DSAMP_ADDRESSU,             D3DTADDRESS_WRAP);
			SSS(D3DSAMP_ADDRESSV,             D3DTADDRESS_WRAP);
			SSS(D3DSAMP_ADDRESSW,             D3DTADDRESS_WRAP);
			SSS(D3DSAMP_BORDERCOLOR,          0x00000000);
			SSS(D3DSAMP_MAGFILTER,            D3DTEXF_POINT);
			SSS(D3DSAMP_MINFILTER,            D3DTEXF_POINT);
			SSS(D3DSAMP_MIPFILTER,            D3DTEXF_NONE);
			SSS(D3DSAMP_MIPMAPLODBIAS,        0);
			SSS(D3DSAMP_MAXMIPLEVEL,          0);
			SSS(D3DSAMP_MAXANISOTROPY,        1);
			SSS(D3DSAMP_SRGBTEXTURE,          0);
			SSS(D3DSAMP_ELEMENTINDEX,         0);
			SSS(D3DSAMP_DMAPOFFSET,           256);
		}
	}

	ms_maxStreamCount  = maxStreamCount;
	ms_streamDataCache = NON_NULL(new StreamData[ms_maxStreamCount]);

	ms_specularPower = 32.f;

#undef RS
#undef TSS
#undef SSS

#ifdef _DEBUG
	DebugFlags::registerFlag(ms_noTextures, "Direct3d9", "noTextures");
#endif

	setConstants();
}

// ----------------------------------------------------------------------

void Direct3d9_StateCache::remove()
{
	if (ms_device)
		lostDevice();

	delete [] ms_streamDataCache;
	ms_streamDataCache = 0;
	ms_maxStreamCount  = 0;
	ms_device = NULL;
}

// ----------------------------------------------------------------------

void Direct3d9_StateCache::setConstants()
{
#ifdef VSPS
	// set constants
	// 1.0 / log(2.0) == log2(e)
	const float c95[4] = { 0.0f, 0.5f, 1.0f, 1.0f / static_cast<float>(log(2.0f)) };

	Direct3d9_StateCache::setVertexShaderConstants(95, c95, 1);
	PaddedVector paddedVector(1.0f, 0.0f, 0.0f);
	Direct3d9_StateCache::setVertexShaderConstants(VSCR_unitX, &paddedVector, 1);
	paddedVector.set(0.f, 1.f, 0.f);
	Direct3d9_StateCache::setVertexShaderConstants(VSCR_unitY, &paddedVector, 1);
	paddedVector.set(0.f, 0.f, 1.f);
	Direct3d9_StateCache::setVertexShaderConstants(VSCR_unitZ, &paddedVector, 1);
#endif
}
// ----------------------------------------------------------------------

#ifdef _DEBUG
void Direct3d9_StateCache::setTexturesEnabled(bool enabled)
{
	ms_noTextures = !enabled;
}
#endif

// ----------------------------------------------------------------------

void Direct3d9_StateCache::lostDevice()
{
	forceIndexBuffer(NULL);
	forceVertexDeclaration(NULL);
	{
		for (int i = 0; i < cms_samplers; ++i)
			setTexture(i, 0);
	}
	{
		for (int i = 0; i < ms_maxStreamCount; ++i)
			forceStreamSource(i, 0, 0, 0);
	}
#ifdef VSPS
	forceVertexShader(NULL);
	forcePixelShader(NULL);
#endif
}

// ----------------------------------------------------------------------

void Direct3d9_StateCache::restoreDevice()
{
#define RS(state)  forceRenderState(state, ms_renderStateCache[state])
#define TSS(state) forceTextureStageState(i, state, ms_textureStagesStateCache[i][state])
#define SSS(state) forceSamplerState(i, state, ms_samplerStateCache[i][state])

	RS(D3DRS_ZENABLE);
	RS(D3DRS_FILLMODE);
	RS(D3DRS_SHADEMODE);
	RS(D3DRS_ZWRITEENABLE);
	RS(D3DRS_ALPHATESTENABLE);
	RS(D3DRS_LASTPIXEL);
	RS(D3DRS_SRCBLEND);
	RS(D3DRS_DESTBLEND);
	RS(D3DRS_CULLMODE);
	RS(D3DRS_ZFUNC);
	RS(D3DRS_ALPHAREF);
	RS(D3DRS_ALPHAFUNC);
	RS(D3DRS_DITHERENABLE);
	RS(D3DRS_ALPHABLENDENABLE);
	RS(D3DRS_FOGENABLE);
	RS(D3DRS_SPECULARENABLE);
	RS(D3DRS_FOGCOLOR);
	RS(D3DRS_FOGTABLEMODE);
	RS(D3DRS_FOGSTART);
	RS(D3DRS_FOGEND);
	RS(D3DRS_FOGDENSITY);
	RS(D3DRS_RANGEFOGENABLE);
	RS(D3DRS_STENCILENABLE);
	RS(D3DRS_STENCILFAIL);
	RS(D3DRS_STENCILZFAIL);
	RS(D3DRS_STENCILPASS);
	RS(D3DRS_STENCILFUNC);
	RS(D3DRS_STENCILREF);
	RS(D3DRS_STENCILMASK);
	RS(D3DRS_STENCILWRITEMASK);
	RS(D3DRS_TEXTUREFACTOR);
	RS(D3DRS_WRAP0);
	RS(D3DRS_WRAP1);
	RS(D3DRS_WRAP2);
	RS(D3DRS_WRAP3);
	RS(D3DRS_WRAP4);
	RS(D3DRS_WRAP5);
	RS(D3DRS_WRAP6);
	RS(D3DRS_WRAP7);
	RS(D3DRS_CLIPPING);
	RS(D3DRS_LIGHTING);
	RS(D3DRS_AMBIENT);
	RS(D3DRS_FOGVERTEXMODE);
	RS(D3DRS_COLORVERTEX);
	RS(D3DRS_LOCALVIEWER);
	RS(D3DRS_NORMALIZENORMALS);
	RS(D3DRS_DIFFUSEMATERIALSOURCE);
	RS(D3DRS_SPECULARMATERIALSOURCE);
	RS(D3DRS_AMBIENTMATERIALSOURCE);
	RS(D3DRS_EMISSIVEMATERIALSOURCE);
	RS(D3DRS_VERTEXBLEND);
	RS(D3DRS_CLIPPLANEENABLE);
	RS(D3DRS_POINTSIZE);
	RS(D3DRS_POINTSIZE_MIN);
	RS(D3DRS_POINTSPRITEENABLE);
	RS(D3DRS_POINTSCALEENABLE);
	RS(D3DRS_POINTSCALE_A);
	RS(D3DRS_POINTSCALE_B);
	RS(D3DRS_POINTSCALE_C);
	RS(D3DRS_MULTISAMPLEANTIALIAS);
	RS(D3DRS_MULTISAMPLEMASK);
	RS(D3DRS_PATCHEDGESTYLE);
	RS(D3DRS_DEBUGMONITORTOKEN);
	RS(D3DRS_POINTSIZE_MAX);
	RS(D3DRS_INDEXEDVERTEXBLENDENABLE);
	RS(D3DRS_COLORWRITEENABLE);
	RS(D3DRS_TWEENFACTOR);
	RS(D3DRS_BLENDOP);
	RS(D3DRS_POSITIONDEGREE);
	RS(D3DRS_NORMALDEGREE);
	RS(D3DRS_SCISSORTESTENABLE);
	RS(D3DRS_SLOPESCALEDEPTHBIAS);
	RS(D3DRS_ANTIALIASEDLINEENABLE);
	RS(D3DRS_MINTESSELLATIONLEVEL);
	RS(D3DRS_MAXTESSELLATIONLEVEL);
	RS(D3DRS_ADAPTIVETESS_X);
	RS(D3DRS_ADAPTIVETESS_Y);
	RS(D3DRS_ADAPTIVETESS_Z);
	RS(D3DRS_ADAPTIVETESS_W);
//	RS(D3DRS_ENABLEADAPTIVETESSELATION);
	RS(D3DRS_TWOSIDEDSTENCILMODE);
	RS(D3DRS_CCW_STENCILFAIL);
	RS(D3DRS_CCW_STENCILZFAIL);
	RS(D3DRS_CCW_STENCILPASS);
	RS(D3DRS_CCW_STENCILFUNC);
	RS(D3DRS_COLORWRITEENABLE1);
	RS(D3DRS_COLORWRITEENABLE2);
	RS(D3DRS_COLORWRITEENABLE3);
	RS(D3DRS_BLENDFACTOR);
	RS(D3DRS_SRGBWRITEENABLE);
	RS(D3DRS_DEPTHBIAS);
	RS(D3DRS_WRAP8);
	RS(D3DRS_WRAP9);
	RS(D3DRS_WRAP10);
	RS(D3DRS_WRAP11);
	RS(D3DRS_WRAP12);
	RS(D3DRS_WRAP13);
	RS(D3DRS_WRAP14);
	RS(D3DRS_WRAP15);
	RS(D3DRS_SEPARATEALPHABLENDENABLE);
	RS(D3DRS_SRCBLENDALPHA);
	RS(D3DRS_DESTBLENDALPHA);
	RS(D3DRS_BLENDOPALPHA);

	{
		for (int i = 0; i < cms_stages; ++i)
		{
			TSS(D3DTSS_COLOROP);
			TSS(D3DTSS_COLORARG1);
			TSS(D3DTSS_COLORARG2);
			TSS(D3DTSS_ALPHAOP);
			TSS(D3DTSS_ALPHAARG1);
			TSS(D3DTSS_ALPHAARG2);
			TSS(D3DTSS_BUMPENVMAT00);
			TSS(D3DTSS_BUMPENVMAT01);
			TSS(D3DTSS_BUMPENVMAT10);
			TSS(D3DTSS_BUMPENVMAT11);
			TSS(D3DTSS_TEXCOORDINDEX);
			TSS(D3DTSS_BUMPENVLSCALE);
			TSS(D3DTSS_BUMPENVLOFFSET);
			TSS(D3DTSS_TEXTURETRANSFORMFLAGS);
			TSS(D3DTSS_COLORARG0);
			TSS(D3DTSS_ALPHAARG0);
			TSS(D3DTSS_RESULTARG);
			TSS(D3DTSS_CONSTANT);
		}
	}

	{
		for (int i = 0; i < cms_samplers; ++i)
		{
			SSS(D3DSAMP_ADDRESSU);
			SSS(D3DSAMP_ADDRESSV);
			SSS(D3DSAMP_ADDRESSW);
			SSS(D3DSAMP_BORDERCOLOR);
			SSS(D3DSAMP_MAGFILTER);
			SSS(D3DSAMP_MINFILTER);
			SSS(D3DSAMP_MIPFILTER);
			SSS(D3DSAMP_MIPMAPLODBIAS);
			SSS(D3DSAMP_MAXMIPLEVEL);
			SSS(D3DSAMP_MAXANISOTROPY);
			SSS(D3DSAMP_SRGBTEXTURE);
			SSS(D3DSAMP_ELEMENTINDEX);
			SSS(D3DSAMP_DMAPOFFSET);
		}
	}

#undef RS
#undef TSS
#undef SSS

	{
		for (int i = 0; i < cms_samplers; ++i)
		{
			ms_device->SetTexture(0, NULL);
			ms_textureCache[i] = 0;
		}
	}

	{
		for (int i = 0; i < ms_maxStreamCount; ++i)
			forceStreamSource(i, 0, 0, 0);
	}

	
#ifdef VSPS
	ms_vertexShader = 0;
	ms_pixelShader = 0;
#endif

	setConstants();
}

// ----------------------------------------------------------------------

void Direct3d9_StateCache::setTexture(int stageNumber, Direct3d9_TextureData const *texture)
{
#ifdef _DEBUG
	if (ms_noTextures)
		texture = NULL;

	if (texture && texture->firstTimeUsedThisFrame())
	{
		Direct3d9_Metrics::setTextureUniqueTextures += 1;
		Direct3d9_Metrics::textureMemoryUsed += texture->getMemorySize();
	}

	Direct3d9_Metrics::setTextureCalls += 1;
#endif

	if (texture != ms_textureCache[stageNumber])
	{
		const HRESULT hresult = ms_device->SetTexture(stageNumber, texture ? texture->getBaseTexture() : NULL);
		FATAL_DX_HR("SetTexture failed %s", hresult);
		ms_textureCache[stageNumber] = texture;

#ifdef _DEBUG
		Direct3d9_Metrics::setTextureCacheMisses += 1;
#endif
	}
}

// ----------------------------------------------------------------------

void Direct3d9_StateCache::destroyTexture(Direct3d9_TextureData const *texture)
{
	for (int i = 0; i < cms_samplers; ++i)
	{
		if (ms_textureCache[i] == texture)
		{
#ifdef _DEBUG
			Direct3d9_Metrics::setTextureCalls += 1;
#endif

			ms_device->SetTexture(i, NULL);
			ms_textureCache[i] = 0;
		}
	}
}

// ----------------------------------------------------------------------

#ifdef VSPS

void Direct3d9_StateCache::resetTextureCoordinateIndices()
{
	for (int i = 0; i < cms_stages; ++i)
	{
		setTextureStageState(i, D3DTSS_TEXCOORDINDEX, i | D3DTSS_TCI_PASSTHRU);
		setTextureStageState(i, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
	}
}

#endif

// ----------------------------------------------------------------------

#ifdef VSPS

void Direct3d9_StateCache::setVertexShaderConstants(int index, const void *data, int numberOfConstants)
{
	if (Direct3d9::supportsVertexShaders())
	{
		HRESULT result = ms_device->SetVertexShaderConstantF(index, reinterpret_cast<const float *>(data), numberOfConstants);
		FATAL_DX_HR("SetVertexShaderConstantF failed %s", result);

#ifdef _DEBUG
		Direct3d9_Metrics::setVertexShaderConstantsCalls += 1;
		Direct3d9_Metrics::setVertexShaderConstantsRegisters += numberOfConstants;
#endif
	}
}

#endif

// ----------------------------------------------------------------------

#ifdef VSPS

void Direct3d9_StateCache::setVertexShaderConstants(int index, const BOOL *data, int numberOfConstants)
{
	if (Direct3d9::supportsVertexShaders())
	{
		HRESULT result = ms_device->SetVertexShaderConstantB(index, data, numberOfConstants);
		FATAL_DX_HR("SetVertexShaderConstantB failed %s", result);
	}
}

#endif

// ----------------------------------------------------------------------

#ifdef VSPS

void Direct3d9_StateCache::setPixelShaderConstants(int index, const void *data, int numberOfConstants)
{
	if (Direct3d9::supportsPixelShaders())
	{
		HRESULT result = ms_device->SetPixelShaderConstantF(index, reinterpret_cast<const float *>(data), numberOfConstants);
		FATAL_DX_HR("SetPixelShaderConstant failed %s", result);

#ifdef _DEBUG
		Direct3d9_Metrics::setPixelShaderConstantsCalls += 1;
		Direct3d9_Metrics::setPixelShaderConstantsRegisters += numberOfConstants;
#endif
	}
}

#endif

// ======================================================================
