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
	float ms_specularPower = 32.0f;
	bool ms_noTextures = false;

	// Tracked as a real value rather than a "have we bound one" flag, because null IS the normal
	// binding and the shadow has to be able to say so.
	ID3D11GeometryShader *ms_geometryShader = NULL;

	ID3D11BlendState *ms_blendState;
	float ms_blendFactor[4];
	uint32 ms_sampleMask;

	ID3D11DepthStencilState *ms_depthStencilState;
	uint32 ms_stencilReference;

	ID3D11RasterizerState *ms_rasterizerState;

	ID3D11VertexShader *ms_vertexShader;
	ID3D11PixelShader *ms_pixelShader;
	ID3D11InputLayout *ms_inputLayout;
	D3D11_PRIMITIVE_TOPOLOGY ms_primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;

	ID3D11ShaderResourceView *ms_shaderResource[Direct3d11_StateCache::cms_shaderResourceSlots];
	ID3D11SamplerState *ms_samplerState[Direct3d11_StateCache::cms_shaderResourceSlots];

	void resetShadow();
} // namespace Direct3d11_StateCacheNamespace
using namespace Direct3d11_StateCacheNamespace;

// ======================================================================
/**
 * Zero the shadow, for the two moments when nothing is bound and that is a fact.
 *
 * Deliberately private, and deliberately not exposed as an invalidate(): a caller that has
 * already bound something behind this cache's back cannot be served by it. See the note above
 * setBlendState for why that is, and why nothing needs to.
 */

void Direct3d11_StateCacheNamespace::resetShadow()
{
	ms_blendState = NULL;
	ms_blendFactor[0] = ms_blendFactor[1] = ms_blendFactor[2] = ms_blendFactor[3] = 0.0f;
	ms_sampleMask = 0;

	ms_depthStencilState = NULL;
	ms_stencilReference = 0;

	ms_rasterizerState = NULL;

	ms_vertexShader = NULL;
	ms_pixelShader = NULL;
	ms_geometryShader = NULL;
	ms_inputLayout = NULL;
	ms_primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;

	for (int i = 0; i < Direct3d11_StateCache::cms_shaderResourceSlots; ++i)
	{
		ms_shaderResource[i] = NULL;
		ms_samplerState[i] = NULL;
	}
}

// ----------------------------------------------------------------------

void Direct3d11_StateCache::install()
{
	// A fresh immediate context has nothing bound, so a zeroed shadow is accurate rather than
	// merely empty.
	resetShadow();
}

// ----------------------------------------------------------------------

void Direct3d11_StateCache::remove()
{
	resetShadow();
}

// ----------------------------------------------------------------------

void Direct3d11_StateCache::setSpecularPower(float power)
{
	ms_specularPower = power;
}

// ----------------------------------------------------------------------

float Direct3d11_StateCache::getSpecularPower()
{
	return ms_specularPower;
}

// ----------------------------------------------------------------------

int Direct3d11_StateCache::auditAgainstDevice(char const *what)
{
	ID3D11DeviceContext1 *const context = Direct3d11_Device::getContext();
	if (!context)
		return 0;

	ID3D11VertexShader *vertexShader = NULL;
	ID3D11PixelShader *pixelShader = NULL;
	ID3D11GeometryShader *geometryShader = NULL;
	ID3D11InputLayout *inputLayout = NULL;

	context->VSGetShader(&vertexShader, NULL, NULL);
	context->PSGetShader(&pixelShader, NULL, NULL);
	context->GSGetShader(&geometryShader, NULL, NULL);
	context->IAGetInputLayout(&inputLayout);

	int disagreements = 0;

	if (vertexShader != ms_vertexShader)
	{
		++disagreements;
		WARNING(true, ("Direct3d11: the state cache disagrees with the device at %s -- vertex shader shadow says %p, device has %p. A bind this cache believes is redundant will be skipped and the wrong program will draw.", what, static_cast<void *>(ms_vertexShader), static_cast<void *>(vertexShader)));
	}

	if (pixelShader != ms_pixelShader)
	{
		++disagreements;
		WARNING(true, ("Direct3d11: the state cache disagrees with the device at %s -- pixel shader shadow says %p, device has %p. A bind this cache believes is redundant will be skipped and the wrong program will draw.", what, static_cast<void *>(ms_pixelShader), static_cast<void *>(pixelShader)));
	}

	if (geometryShader != ms_geometryShader)
	{
		++disagreements;
		WARNING(true, ("Direct3d11: the state cache disagrees with the device at %s -- geometry shader shadow says %p, device has %p.", what, static_cast<void *>(ms_geometryShader), static_cast<void *>(geometryShader)));
	}

	if (inputLayout != ms_inputLayout)
	{
		++disagreements;
		WARNING(true, ("Direct3d11: the state cache disagrees with the device at %s -- input layout shadow says %p, device has %p.", what, static_cast<void *>(ms_inputLayout), static_cast<void *>(inputLayout)));
	}

	if (vertexShader)
		vertexShader->Release();
	if (pixelShader)
		pixelShader->Release();
	if (geometryShader)
		geometryShader->Release();
	if (inputLayout)
		inputLayout->Release();

	return disagreements;
}

// ----------------------------------------------------------------------

// ======================================================================
// There is deliberately no invalidate().
//
// One used to exist, for the benefit of a pass that bound its shaders and states straight to
// the context and then told the cache its shadows were stale. It could not work. Every field
// this cache shadows is a pointer, and NULL is a legal value for all of them -- a null pixel
// shader is what a pass with no pixel program binds, a null input layout is what a shader that
// reads no vertex buffer binds. So clearing the shadows to NULL did not say "nothing is known",
// it said "NULL is bound", and the very next bind of NULL was skipped as redundant. The scene
// target's composite left its own fullscreen pixel shader on the device that way, and the next
// material with no pixel program inherited it: a two-element interpolant signature against a
// twelve-element vertex output, which D3D11 rejects as a linkage error and does not draw.
//
// Expressing "unknown" properly would mean a sentinel pointer value that cannot occur, or a
// validity flag beside every field. Both are machinery to support something nothing needs to
// do. Device state this cache shadows is set through this cache, and then the shadow is right
// by construction. auditAgainstDevice() is what keeps that true.
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

	ID3D11DeviceContext1 *const context = Direct3d11_Device::getContext();
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

	ID3D11DeviceContext1 *const context = Direct3d11_Device::getContext();
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

	ID3D11DeviceContext1 *const context = Direct3d11_Device::getContext();
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

	ID3D11DeviceContext1 *const context = Direct3d11_Device::getContext();
	if (!context)
		return;

	++Direct3d11_Metrics::vertexShaderBindMisses;

	ms_vertexShader = shader;
	context->VSSetShader(shader, NULL, 0);
}

// ----------------------------------------------------------------------

void Direct3d11_StateCache::setGeometryShader(ID3D11GeometryShader *shader)
{
	if (shader == ms_geometryShader)
		return;

	ms_geometryShader = shader;

	ID3D11DeviceContext1 *const context = Direct3d11_Device::getContext();
	if (context)
		context->GSSetShader(shader, NULL, 0);
}

// ----------------------------------------------------------------------

void Direct3d11_StateCache::setPixelShader(ID3D11PixelShader *shader)
{
	++Direct3d11_Metrics::pixelShaderBindCalls;

	if (shader == ms_pixelShader)
		return;

	ID3D11DeviceContext1 *const context = Direct3d11_Device::getContext();
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

	ID3D11DeviceContext1 *const context = Direct3d11_Device::getContext();
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

	ID3D11DeviceContext1 *const context = Direct3d11_Device::getContext();
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

	ID3D11DeviceContext1 *const context = Direct3d11_Device::getContext();
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

	ID3D11DeviceContext1 *const context = Direct3d11_Device::getContext();
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

void Direct3d11_StateCache::setTexturesEnabled(bool enabled)
{
	if (ms_noTextures == !enabled)
		return;

	ms_noTextures = !enabled;

	// Unbind what is already bound rather than waiting for the next set, which the shadow would
	// skip as redundant. DX9 does not need this because its flag is read at bind time and its
	// cache is invalidated per frame anyway.
	ID3D11DeviceContext1 *const context = Direct3d11_Device::getContext();
	ID3D11ShaderResourceView *nothing = NULL;

	for (int i = 0; i < cms_shaderResourceSlots; ++i)
	{
		if (!ms_shaderResource[i])
			continue;

		ms_shaderResource[i] = NULL;

		if (context)
			context->PSSetShaderResources(static_cast<UINT>(i), 1, &nothing);
	}
}

// ----------------------------------------------------------------------

void Direct3d11_StateCache::unbindShaderResourcesForResource(ID3D11Resource *resource)
{
	if (!resource)
		return;

	ID3D11DeviceContext1 *const context = Direct3d11_Device::getContext();
	ID3D11ShaderResourceView *nothing = NULL;

	for (int i = 0; i < cms_shaderResourceSlots; ++i)
	{
		if (!ms_shaderResource[i])
			continue;

		// GetResource adds a reference, so it has to be dropped whether or not it matched.
		ID3D11Resource *bound = NULL;
		ms_shaderResource[i]->GetResource(&bound);

		bool const matches = (bound == resource);

		if (bound)
			bound->Release();

		if (!matches)
			continue;

		ms_shaderResource[i] = NULL;

		if (context)
			context->PSSetShaderResources(static_cast<UINT>(i), 1, &nothing);
	}
}

// ----------------------------------------------------------------------

void Direct3d11_StateCache::destroyShaderResource(ID3D11ShaderResourceView *view)
{
	if (!view)
		return;

	ID3D11DeviceContext1 *const context = Direct3d11_Device::getContext();
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
