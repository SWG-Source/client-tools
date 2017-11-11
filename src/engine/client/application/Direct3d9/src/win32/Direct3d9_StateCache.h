// ======================================================================
//
// Direct3d9_StateCache.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_Direct3d9_StateCache_H
#define INCLUDED_Direct3d9_StateCache_H

// ======================================================================

class Direct3d9_TextureData;

#include "Direct3d9.h"
#include "Direct3d9_Metrics.h"
#include "clientGraphics/Graphics.def"

// ======================================================================

class Direct3d9_StateCache
{
public:

	static void install(int maxStreamCount);
	static void remove();

#ifdef _DEBUG
	static void setTexturesEnabled(bool enabled);
#endif

	static void forceRenderState(D3DRENDERSTATETYPE state, DWORD value);
	static void forceRenderState(D3DRENDERSTATETYPE state, int value);
	static void forceRenderState(D3DRENDERSTATETYPE state, float value);
	static void setRenderState(D3DRENDERSTATETYPE state, DWORD value);
	static void setRenderState(D3DRENDERSTATETYPE state, int value);
	static void setRenderState(D3DRENDERSTATETYPE state, float value);

	static void forceSamplerState(int sampler, D3DSAMPLERSTATETYPE state, DWORD value);
	static void setSamplerState(int sampler, D3DSAMPLERSTATETYPE state, DWORD value);

	static void forceTextureStageState(int stage, D3DTEXTURESTAGESTATETYPE state, DWORD value);
	static void forceTextureStageState(int stage, D3DTEXTURESTAGESTATETYPE state, int value);
	static void forceTextureStageState(int stage, D3DTEXTURESTAGESTATETYPE state, float value);
	static void setTextureStageState(int stage, D3DTEXTURESTAGESTATETYPE state, float value);
	static void setTextureStageState(int stage, D3DTEXTURESTAGESTATETYPE state, int value);
	static void setTextureStageState(int stage, D3DTEXTURESTAGESTATETYPE state, DWORD value);

	static void forceIndexBuffer(IDirect3DIndexBuffer9 *indexBuffer);
	static void setIndexBuffer(IDirect3DIndexBuffer9 *indexBuffer);

	static void forceStreamSource(int streamIndex, IDirect3DVertexBuffer9 *stream, UINT byteOffset, UINT stride);
	static void setStreamSource(int streamIndex, IDirect3DVertexBuffer9 *stream, UINT byteOffset, UINT stride);

	static void forceVertexDeclaration(IDirect3DVertexDeclaration9 *vertexDeclaration);
	static void setVertexDeclaration(IDirect3DVertexDeclaration9 *vertexDeclaration);

	static void setTexture(int stage, Direct3d9_TextureData const *texture);
	static void destroyTexture(Direct3d9_TextureData const *texture);

	static void lostDevice();
	static void restoreDevice();

#ifdef VSPS
	static void forceVertexShader(IDirect3DVertexShader9 *vertexShader);
	static void setVertexShader(IDirect3DVertexShader9 *vertexShader);

	static void forcePixelShader(IDirect3DPixelShader9 *pixelShader);
	static void setPixelShader(IDirect3DPixelShader9 *pixelShader);

	static void setVertexShaderConstants(int index, const void *data, int numberOfConstants);
	static void setVertexShaderConstants(int index, const BOOL *data, int numberOfConstants);
	static void setPixelShaderConstants(int index, const void *data, int numberOfConstants);

	static void resetTextureCoordinateIndices();
#endif

	static void  setSpecularPower(float power);
	static float getSpecularPower();

private:

	enum
	{
		cms_stages   = 8,
		cms_samplers = 16
	};

	struct StreamData
	{
		IDirect3DVertexBuffer9 *m_stream;
		UINT                    m_offset;
		UINT                    m_stride;
	};

	typedef DWORD                        RenderStateCache[210];
	typedef DWORD                        TextureStagesCache[cms_stages][33];
	typedef DWORD                        SamplerCache[cms_samplers][14];
	typedef Direct3d9_TextureData const *TextureCache[cms_samplers];

private:

	static void setConstants();

private:

	static IDirect3DDevice9 *             ms_device;
	static RenderStateCache               ms_renderStateCache;
	static SamplerCache                   ms_samplerStateCache;
	static TextureStagesCache             ms_textureStagesStateCache;
	static TextureCache                   ms_textureCache;
	static IDirect3DVertexDeclaration9 *  ms_vertexDeclaration;
	static StreamData *                   ms_streamDataCache;
	static int                            ms_maxStreamCount;

	static IDirect3DIndexBuffer9 *        ms_indexBuffer;
	static UINT                           ms_indexBufferOffset;
	
	static float                          ms_specularPower;

#ifdef VSPS
	static IDirect3DVertexShader9 *       ms_vertexShader;
	static IDirect3DPixelShader9  *       ms_pixelShader;
#endif

#ifdef _DEBUG
	static bool                           ms_noTextures;
#endif
};

// ======================================================================

#define STATE_CACHE_ALWAYS_FORCE 0

// ======================================================================

inline void Direct3d9_StateCache::forceRenderState(D3DRENDERSTATETYPE state, DWORD value)
{
#ifdef _DEBUG
	++Direct3d9_Metrics::setRenderStateCacheMisses;
#endif

	ms_renderStateCache[state] = value;
	HRESULT hresult = ms_device->SetRenderState(state, value);
	FATAL_DX_HR("Failed to set renderstate %s", hresult);
}

// ----------------------------------------------------------------------

inline void Direct3d9_StateCache::forceRenderState(D3DRENDERSTATETYPE state, int value)
{
	forceRenderState(state, static_cast<DWORD>(value));
}

// ----------------------------------------------------------------------

inline void Direct3d9_StateCache::forceRenderState(D3DRENDERSTATETYPE state, float value)
{
	forceRenderState(state, reinterpret_cast<DWORD*>(&value)[0]);
}

// ----------------------------------------------------------------------

inline void Direct3d9_StateCache::setRenderState(D3DRENDERSTATETYPE state, DWORD value)
{
#ifdef _DEBUG
	++Direct3d9_Metrics::setRenderStateCalls;
#endif

#if !STATE_CACHE_ALWAYS_FORCE
	if (ms_renderStateCache[state] != value)
#endif
		forceRenderState(state, value);
}

// ----------------------------------------------------------------------

inline void Direct3d9_StateCache::setRenderState(D3DRENDERSTATETYPE state, int value)
{
	setRenderState(state, static_cast<DWORD>(value));
}

// ----------------------------------------------------------------------

inline void Direct3d9_StateCache::setRenderState(D3DRENDERSTATETYPE state, float value)
{
	setRenderState(state, reinterpret_cast<DWORD*>(&value)[0]);
}

// ----------------------------------------------------------------------

inline void Direct3d9_StateCache::forceSamplerState(int sampler, D3DSAMPLERSTATETYPE state, DWORD value)
{
	DEBUG_FATAL(sampler >= cms_samplers, ("Sampler out of range %d/%d", sampler, cms_samplers));
#ifdef _DEBUG
	++Direct3d9_Metrics::setSamplerStateCacheMisses;
#endif
	ms_samplerStateCache[sampler][state] = value;
	HRESULT hresult = ms_device->SetSamplerState(static_cast<DWORD>(sampler), state, value);
	FATAL_DX_HR("SetSamplerState failed %s", hresult);
}

// ----------------------------------------------------------------------

inline void Direct3d9_StateCache::setSamplerState(int sampler, D3DSAMPLERSTATETYPE state, DWORD value)
{
	DEBUG_FATAL(sampler >= cms_samplers, ("Sampler out of range %d/%d", sampler, cms_samplers));

#ifdef _DEBUG
	++Direct3d9_Metrics::setSamplerStateCalls;
#endif

#if !STATE_CACHE_ALWAYS_FORCE
	if (ms_samplerStateCache[sampler][state] != value)
#endif
		forceSamplerState(sampler, state, value);
}

// ----------------------------------------------------------------------

inline void Direct3d9_StateCache::forceTextureStageState(int stage, D3DTEXTURESTAGESTATETYPE state, DWORD value)
{
	DEBUG_FATAL(stage >= cms_stages, ("stage out of range %d/%d", stage, cms_stages));
#ifdef _DEBUG
	++Direct3d9_Metrics::setTextureStageStateCacheMisses;
#endif
	ms_textureStagesStateCache[stage][state] = value;
	HRESULT hresult = ms_device->SetTextureStageState(static_cast<DWORD>(stage), state, value);
	FATAL_DX_HR("Failed to set texturestagestate %s", hresult);
}

// ----------------------------------------------------------------------

inline void Direct3d9_StateCache::forceTextureStageState(int stage, D3DTEXTURESTAGESTATETYPE state, int value)
{
	forceTextureStageState(stage, state, static_cast<DWORD>(value));
}

// ----------------------------------------------------------------------

inline void Direct3d9_StateCache::forceTextureStageState(int stage, D3DTEXTURESTAGESTATETYPE state, float value)
{
	forceTextureStageState(stage, state, reinterpret_cast<DWORD*>(&value)[0]);
}

// ----------------------------------------------------------------------

inline void Direct3d9_StateCache::setTextureStageState(int stage, D3DTEXTURESTAGESTATETYPE state, DWORD value)
{
	DEBUG_FATAL(stage >= cms_stages, ("stage out of range %d/%d", stage, cms_stages));
#ifdef _DEBUG
	++Direct3d9_Metrics::setTextureStageStateCalls;
#endif

#if !STATE_CACHE_ALWAYS_FORCE
	if (ms_textureStagesStateCache[stage][state] != value)
#endif
		forceTextureStageState(stage, state, value);
}

// ----------------------------------------------------------------------

inline void Direct3d9_StateCache::setTextureStageState(int stage, D3DTEXTURESTAGESTATETYPE state, int value)
{
	setTextureStageState(stage, state, static_cast<DWORD>(value));
}

// ----------------------------------------------------------------------

inline void Direct3d9_StateCache::setTextureStageState(int stage, D3DTEXTURESTAGESTATETYPE state, float value)
{
	setTextureStageState(stage, state, reinterpret_cast<DWORD*>(&value)[0]);
}

// ----------------------------------------------------------------------

inline void Direct3d9_StateCache::forceIndexBuffer(IDirect3DIndexBuffer9 *indexBuffer)
{
#ifdef _DEBUG
	++Direct3d9_Metrics::setIndexBufferCacheMisses;
#endif

	ms_indexBuffer = indexBuffer;
	HRESULT hresult = ms_device->SetIndices(ms_indexBuffer);
	FATAL_DX_HR("SetIndices failed %s", hresult);
}

// ----------------------------------------------------------------------

inline void Direct3d9_StateCache::setIndexBuffer(IDirect3DIndexBuffer9 *indexBuffer)
{
#if !STATE_CACHE_ALWAYS_FORCE
	if (ms_indexBuffer != indexBuffer)
#endif
		forceIndexBuffer(indexBuffer);
}

// ----------------------------------------------------------------------

inline void Direct3d9_StateCache::forceStreamSource(int streamIndex, IDirect3DVertexBuffer9 *stream, UINT byteOffset, UINT stride)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, streamIndex, ms_maxStreamCount);

#ifdef _DEBUG
	++Direct3d9_Metrics::setStreamSourceCacheMisses;
#endif

	//-- save cache settings
	ms_streamDataCache[streamIndex].m_stream = stream;
	ms_streamDataCache[streamIndex].m_offset = byteOffset;
	ms_streamDataCache[streamIndex].m_stride = stride;

	//-- set stream
	HRESULT hresult = ms_device->SetStreamSource(static_cast<DWORD>(streamIndex), stream, byteOffset, stride);
	FATAL_DX_HR("Failed to set stream source %s", hresult);
}

// ----------------------------------------------------------------------

inline void Direct3d9_StateCache::setStreamSource(int streamIndex, IDirect3DVertexBuffer9 *stream, UINT byteOffset, UINT stride)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, streamIndex, ms_maxStreamCount);

#ifdef _DEBUG
	++Direct3d9_Metrics::setStreamSourceCalls;
#endif

	//-- check cache
#if !STATE_CACHE_ALWAYS_FORCE
	if (  (stream != ms_streamDataCache[streamIndex].m_stream) 
		|| (ms_streamDataCache[streamIndex].m_offset != byteOffset)
		|| (ms_streamDataCache[streamIndex].m_stride != stride)
		)
#endif
		forceStreamSource(streamIndex, stream, byteOffset, stride);
}

// ----------------------------------------------------------------------

inline void Direct3d9_StateCache::forceVertexDeclaration(IDirect3DVertexDeclaration9 *vertexDeclaration)
{
#ifdef _DEBUG
	++Direct3d9_Metrics::setVertexDeclarationCacheMisses;
#endif

	HRESULT hresult = ms_device->SetVertexDeclaration(vertexDeclaration);
	FATAL_DX_HR("SetVertexDeclaration failed %s", hresult);
}

// ----------------------------------------------------------------------

inline void Direct3d9_StateCache::setVertexDeclaration(IDirect3DVertexDeclaration9 *vertexDeclaration)
{
#ifdef _DEBUG
	++Direct3d9_Metrics::setVertexDeclarationCalls;
#endif

#if !STATE_CACHE_ALWAYS_FORCE
	if (ms_vertexDeclaration != vertexDeclaration)
#endif
		forceVertexDeclaration(vertexDeclaration);
}

// ----------------------------------------------------------------------

#ifdef VSPS

inline void Direct3d9_StateCache::forceVertexShader(IDirect3DVertexShader9 *vertexShader)
{
#ifdef _DEBUG
	++Direct3d9_Metrics::setVertexShaderCacheMisses;
#endif

	ms_vertexShader = vertexShader;
	const HRESULT hresult = ms_device->SetVertexShader(ms_vertexShader);
	FATAL_DX_HR("SetVertexShader failed %s", hresult);
}

#endif

// ----------------------------------------------------------------------

#ifdef VSPS

inline void Direct3d9_StateCache::setVertexShader(IDirect3DVertexShader9 *vertexShader)
{
#ifdef _DEBUG
	++Direct3d9_Metrics::setVertexShaderCalls;
#endif

#if !STATE_CACHE_ALWAYS_FORCE
	if (vertexShader != ms_vertexShader)
#endif
		forceVertexShader(vertexShader);
}

#endif

// ----------------------------------------------------------------------

#ifdef VSPS

inline void Direct3d9_StateCache::forcePixelShader(IDirect3DPixelShader9 *pixelShader)
{
	ms_pixelShader = pixelShader;
	const HRESULT hresult = ms_device->SetPixelShader(ms_pixelShader);
	FATAL_DX_HR("SetPixelShader failed %s", hresult);
}

#endif

// ----------------------------------------------------------------------

#ifdef VSPS

inline void Direct3d9_StateCache::setPixelShader(IDirect3DPixelShader9 *pixelShader)
{
#if !STATE_CACHE_ALWAYS_FORCE
	if (pixelShader != ms_pixelShader)
#endif
		forcePixelShader(pixelShader);
}

#endif

// ----------------------------------------------------------------------

inline void Direct3d9_StateCache::setSpecularPower(float power)
{
	ms_specularPower = power;
}

// ----------------------------------------------------------------------

inline float Direct3d9_StateCache::getSpecularPower()
{
	return ms_specularPower;
}

// ======================================================================

#endif
