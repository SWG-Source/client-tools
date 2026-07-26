// ======================================================================
//
// Direct3d11_StateCache.cpp
// copyright (c) 2026 Galaxies Reborn
//
// ======================================================================

#include "FirstDirect3d11.h"
#include "Direct3d11_StateCache.h"

#include "Direct3d11_Device.h"
#include "Direct3d11_Metrics.h"

// ======================================================================

namespace Direct3d11_StateCacheNamespace
{
	ID3D11BlendState         *ms_blendState;
	float                     ms_blendFactor[4];
	uint32                    ms_sampleMask;

	ID3D11DepthStencilState  *ms_depthStencilState;
	uint32                    ms_stencilReference;

	ID3D11RasterizerState    *ms_rasterizerState;

	ID3D11VertexShader       *ms_vertexShader;
	ID3D11PixelShader        *ms_pixelShader;
	ID3D11InputLayout        *ms_inputLayout;
	D3D11_PRIMITIVE_TOPOLOGY  ms_primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;

	ID3D11ShaderResourceView *ms_shaderResource[Direct3d11_StateCache::cms_shaderResourceSlots];
	ID3D11SamplerState       *ms_samplerState[Direct3d11_StateCache::cms_shaderResourceSlots];
}
using namespace Direct3d11_StateCacheNamespace;

// ======================================================================

void Direct3d11_StateCache::install()
{
	invalidate();
}

// ----------------------------------------------------------------------

void Direct3d11_StateCache::remove()
{
	invalidate();
}

// ----------------------------------------------------------------------
/**
 * Forget the shadow.
 *
 * Anything that binds state without going through here has to call this, or the
 * next bind through here will be skipped as redundant when it is not. That is the
 * one way a cache like this produces a wrong image rather than merely a slow one.
 */

void Direct3d11_StateCache::invalidate()
{
	ms_blendState = NULL;
	ms_blendFactor[0] = ms_blendFactor[1] = ms_blendFactor[2] = ms_blendFactor[3] = 0.0f;
	ms_sampleMask = 0;

	ms_depthStencilState = NULL;
	ms_stencilReference = 0;

	ms_rasterizerState = NULL;

	ms_vertexShader = NULL;
	ms_pixelShader = NULL;
	ms_inputLayout = NULL;
	ms_primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;

	for (int i = 0; i < cms_shaderResourceSlots; ++i)
	{
		ms_shaderResource[i] = NULL;
		ms_samplerState[i] = NULL;
	}
}

// ======================================================================

void Direct3d11_StateCache::setBlendState(ID3D11BlendState *state, float const blendFactor[4], uint32 sampleMask)
{
	++Direct3d11_Metrics::blendStateBindCalls;

	bool const factorMatches =
		ms_blendFactor[0] == blendFactor[0] &&
		ms_blendFactor[1] == blendFactor[1] &&
		ms_blendFactor[2] == blendFactor[2] &&
		ms_blendFactor[3] == blendFactor[3];

	if (state == ms_blendState && sampleMask == ms_sampleMask && factorMatches)
		return;

	ID3D11DeviceContext1 * const context = Direct3d11_Device::getContext();
	if (!context)
		return;

	++Direct3d11_Metrics::blendStateBindMisses;

	ms_blendState = state;
	ms_blendFactor[0] = blendFactor[0];
	ms_blendFactor[1] = blendFactor[1];
	ms_blendFactor[2] = blendFactor[2];
	ms_blendFactor[3] = blendFactor[3];
	ms_sampleMask = sampleMask;

	context->OMSetBlendState(state, blendFactor, sampleMask);
}

// ----------------------------------------------------------------------
/**
 * Bind a depth-stencil state and its reference value.
 *
 * The reference is part of the binding call in D3D11, not a separate state as it
 * was in D3D9, so a change to the reference alone still has to re-bind. Missing
 * that is how a per-shader stencil reference silently becomes zero for every
 * shadow after the first.
 */

void Direct3d11_StateCache::setDepthStencilState(ID3D11DepthStencilState *state, uint32 stencilReference)
{
	++Direct3d11_Metrics::depthStencilStateBindCalls;

	if (state == ms_depthStencilState && stencilReference == ms_stencilReference)
		return;

	ID3D11DeviceContext1 * const context = Direct3d11_Device::getContext();
	if (!context)
		return;

	++Direct3d11_Metrics::depthStencilStateBindMisses;

	ms_depthStencilState = state;
	ms_stencilReference = stencilReference;

	context->OMSetDepthStencilState(state, stencilReference);
}

// ----------------------------------------------------------------------

void Direct3d11_StateCache::setRasterizerState(ID3D11RasterizerState *state)
{
	++Direct3d11_Metrics::rasterizerStateBindCalls;

	if (state == ms_rasterizerState)
		return;

	ID3D11DeviceContext1 * const context = Direct3d11_Device::getContext();
	if (!context)
		return;

	++Direct3d11_Metrics::rasterizerStateBindMisses;

	ms_rasterizerState = state;
	context->RSSetState(state);
}

// ----------------------------------------------------------------------

void Direct3d11_StateCache::setVertexShader(ID3D11VertexShader *shader)
{
	++Direct3d11_Metrics::vertexShaderBindCalls;

	if (shader == ms_vertexShader)
		return;

	ID3D11DeviceContext1 * const context = Direct3d11_Device::getContext();
	if (!context)
		return;

	++Direct3d11_Metrics::vertexShaderBindMisses;

	ms_vertexShader = shader;
	context->VSSetShader(shader, NULL, 0);
}

// ----------------------------------------------------------------------

void Direct3d11_StateCache::setPixelShader(ID3D11PixelShader *shader)
{
	++Direct3d11_Metrics::pixelShaderBindCalls;

	if (shader == ms_pixelShader)
		return;

	ID3D11DeviceContext1 * const context = Direct3d11_Device::getContext();
	if (!context)
		return;

	++Direct3d11_Metrics::pixelShaderBindMisses;

	ms_pixelShader = shader;
	context->PSSetShader(shader, NULL, 0);
}

// ----------------------------------------------------------------------

void Direct3d11_StateCache::setInputLayout(ID3D11InputLayout *inputLayout)
{
	++Direct3d11_Metrics::inputLayoutBindCalls;

	if (inputLayout == ms_inputLayout)
		return;

	ID3D11DeviceContext1 * const context = Direct3d11_Device::getContext();
	if (!context)
		return;

	++Direct3d11_Metrics::inputLayoutBindMisses;

	ms_inputLayout = inputLayout;
	context->IASetInputLayout(inputLayout);
}

// ----------------------------------------------------------------------

void Direct3d11_StateCache::setPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY topology)
{
	if (topology == ms_primitiveTopology)
		return;

	ID3D11DeviceContext1 * const context = Direct3d11_Device::getContext();
	if (!context)
		return;

	ms_primitiveTopology = topology;
	context->IASetPrimitiveTopology(topology);
}

// ======================================================================

void Direct3d11_StateCache::setShaderResource(int slot, ID3D11ShaderResourceView *view)
{
	DEBUG_FATAL(slot < 0 || slot >= cms_shaderResourceSlots, ("Direct3d11: shader resource slot %d is out of range 0..%d.", slot, cms_shaderResourceSlots - 1));

	++Direct3d11_Metrics::shaderResourceBindCalls;

	if (view == ms_shaderResource[slot])
		return;

	ID3D11DeviceContext1 * const context = Direct3d11_Device::getContext();
	if (!context)
		return;

	++Direct3d11_Metrics::shaderResourceBindMisses;

	ms_shaderResource[slot] = view;
	context->PSSetShaderResources(static_cast<UINT>(slot), 1, &view);
}

// ----------------------------------------------------------------------
/**
 * Bind a sampler.
 *
 * Samplers are immutable objects shared through Direct3d11_StateObjectCache, so the same
 * material's sampler is the same pointer every draw and the shadow comparison is a hit in
 * the steady state. DX9 had to push seven individual sampler states per stage per draw to
 * express the same thing.
 */

void Direct3d11_StateCache::setSamplerState(int slot, ID3D11SamplerState *sampler)
{
	DEBUG_FATAL(slot < 0 || slot >= cms_shaderResourceSlots, ("Direct3d11: sampler slot %d is out of range 0..%d.", slot, cms_shaderResourceSlots - 1));

	++Direct3d11_Metrics::samplerBindCalls;

	if (sampler == ms_samplerState[slot])
		return;

	ID3D11DeviceContext1 * const context = Direct3d11_Device::getContext();
	if (!context)
		return;

	++Direct3d11_Metrics::samplerBindMisses;

	ms_samplerState[slot] = sampler;
	context->PSSetSamplers(static_cast<UINT>(slot), 1, &sampler);
}

// ----------------------------------------------------------------------
/**
 * Forget a view that is about to be freed, unbinding it first.
 *
 * The unbind matters as much as the forgetting. A view left bound keeps the resource
 * alive from the context's reference, so the Release in the texture's destructor
 * would not actually destroy it, and the texture would appear to leak until something
 * else happened to bind over that slot.
 */

void Direct3d11_StateCache::destroyShaderResource(ID3D11ShaderResourceView *view)
{
	if (!view)
		return;

	ID3D11DeviceContext1 * const context = Direct3d11_Device::getContext();
	ID3D11ShaderResourceView *nothing = NULL;

	for (int i = 0; i < cms_shaderResourceSlots; ++i)
	{
		if (ms_shaderResource[i] != view)
			continue;

		ms_shaderResource[i] = NULL;

		if (context)
			context->PSSetShaderResources(static_cast<UINT>(i), 1, &nothing);
	}
}

// ======================================================================
