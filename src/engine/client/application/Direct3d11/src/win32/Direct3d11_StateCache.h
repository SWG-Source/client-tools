// ======================================================================
//
// Direct3d11_StateCache.h
// copyright (c) 2026 Galaxies Reborn
//
// A shadow of what is currently bound, so a redundant bind costs a comparison
// instead of a driver call.
//
// Distinct from Direct3d11_StateObjectCache, which owns the objects. This owns
// only the knowledge of which of them the context is holding.
//
// Every bind is counted twice: once as a call, once as a miss when it actually
// reached the device. Without both numbers a cache's hit rate is a belief rather
// than a measurement -- and DX9 is the cautionary tale, since its vertex
// declaration cache has been a permanent 100% miss for twenty years because
// forceVertexDeclaration never assigns the shadow it compares against.
//
// ======================================================================

#ifndef INCLUDED_Direct3d11_StateCache_H
#define INCLUDED_Direct3d11_StateCache_H

// ======================================================================

#include <d3d11_1.h>

// ======================================================================

class Direct3d11_StateCache
{
public:

	static void  install();
	static void  remove();

	// Forget everything. Called when the context state is invalidated from
	// outside this cache -- after a resize, or after the composite, which binds
	// its own state deliberately.
	static void  invalidate();

	static void  setBlendState(ID3D11BlendState *state, float const blendFactor[4], uint32 sampleMask);
	static void  setDepthStencilState(ID3D11DepthStencilState *state, uint32 stencilReference);
	static void  setRasterizerState(ID3D11RasterizerState *state);

	static void  setVertexShader(ID3D11VertexShader *shader);
	static void  setPixelShader(ID3D11PixelShader *shader);
	static void  setInputLayout(ID3D11InputLayout *inputLayout);
	static void  setPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY topology);

private:

	Direct3d11_StateCache();
	Direct3d11_StateCache(Direct3d11_StateCache const &);
	Direct3d11_StateCache &operator =(Direct3d11_StateCache const &);
};

// ======================================================================

#endif
