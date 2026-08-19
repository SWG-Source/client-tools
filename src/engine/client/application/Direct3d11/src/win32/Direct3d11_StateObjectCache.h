// ======================================================================
//
// Direct3d11_StateObjectCache.h
// copyright (c) 2026 Galaxies Reborn
//
// Immutable state objects, created once and shared.
//
// D3D9 set render state piecemeal and could dedup it at runtime. D3D11 requires
// the state to be baked into objects up front, and creating one is expensive.
// Creating them lazily at bind time is the canonical way a DX11 port ends up
// slower than the D3D9 code it replaced: the work moves from a cheap redundant
// setter into an allocation, and it lands inside the frame.
//
// So nothing here is created during a frame. State objects are built either at
// install, for the combinations that are engine state rather than asset state, or
// at asset construction time, where the descriptor is already known -- DX9
// gathers exactly that tuple in Pass::construct and Stage::construct and merely
// replays it later, so this is a structural fit rather than a redesign.
//
// Two things are asserted rather than hoped for. Creation during a frame is
// counted, and the count is required to be zero after warm-up. And each cache has
// a capacity: the asset census predicts counts well inside these, so exceeding
// one means something dynamic has leaked into a cache key, which is a design
// defect and not a tuning problem.
//
// Keys are packed PODs compared by value. The prior DX11 attempt built a
// std::string out of the raw descriptor bytes on every lookup -- a heap
// allocation, a memcpy of up to 264 bytes and a red-black-tree walk, on a path
// reached from setCullMode, which the engine issues in save/restore pairs around
// every shadow volume and every ribbon draw.
//
// ======================================================================

#ifndef INCLUDED_Direct3d11_StateObjectCache_H
#define INCLUDED_Direct3d11_StateObjectCache_H

// ======================================================================

#include <d3d11_1.h>

// ======================================================================

class Direct3d11_StateObjectCache
{
public:
	// Capacities, asserted at runtime.
	static constexpr int MAX_BLEND_STATES = 64;
	static constexpr int MAX_DEPTH_STENCIL_STATES = 128;
	static constexpr int MAX_RASTERIZER_STATES = 32;
	static constexpr int MAX_SAMPLER_STATES = 64;

	// The rasterizer state the engine drives directly, rather than through an
	// asset: fill mode, cull mode and whether scissoring is on. Every combination
	// is pre-created at install, because these change during a frame and must
	// never cause a creation.
	struct RasterizerKey
	{
		uint8 fillMode; // GlFillMode
		uint8 cullMode; // GlCullMode
		uint8 scissorEnabled;
		uint8 pad;
	};

public:
	static void install();
	static void remove();

	static ID3D11BlendState *getBlendState(D3D11_BLEND_DESC const &description);
	static ID3D11DepthStencilState *getDepthStencilState(D3D11_DEPTH_STENCIL_DESC const &description);
	static ID3D11SamplerState *getSamplerState(D3D11_SAMPLER_DESC const &description);

	// Pre-enumerated, so this is a lookup and never a creation.
	static ID3D11RasterizerState *getRasterizerState(int fillMode, int cullMode, bool scissorEnabled);

	static int getBlendStateCount();
	static int getDepthStencilStateCount();
	static int getRasterizerStateCount();
	static int getSamplerStateCount();

private:
	Direct3d11_StateObjectCache();
	Direct3d11_StateObjectCache(Direct3d11_StateObjectCache const &);
	Direct3d11_StateObjectCache &operator=(Direct3d11_StateObjectCache const &);
};

// ======================================================================

#endif
