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
	static void install();
	static void remove();

	// Compare every shadowed binding against what the device actually has, and report each
	// disagreement by name. Returns the number found, so a caller can act on it.
	//
	// This exists because a redundancy cache that is wrong does not draw slowly, it draws the
	// wrong thing: a bind the shadow believes is already in place is skipped, and the previous
	// program stays bound. That failure reaches the log as a vertex/pixel linkage error naming
	// two anonymous stages, and it cost a long diagnosis once already.
	//
	// Called once per present when the debugLayer key is on, which is cheap enough to leave in
	// and catches any future path that writes device state without going through here -- within
	// one frame of it being written, rather than whenever someone next reads the log carefully.
	static int auditAgainstDevice(char const *what);

	static void setBlendState(ID3D11BlendState *state, float const blendFactor[4], uint32 sampleMask);
	static void setDepthStencilState(ID3D11DepthStencilState *state, uint32 stencilReference);
	static void setRasterizerState(ID3D11RasterizerState *state);

	static void setVertexShader(ID3D11VertexShader *shader);
	static void setPixelShader(ID3D11PixelShader *shader);

	// Null for the overwhelming majority of draws. Shadowed like the others so that binding and
	// unbinding the point sprite expander around one primitive is two calls a frame rather than
	// two per draw.
	static void setGeometryShader(ID3D11GeometryShader *shader);
	static void setInputLayout(ID3D11InputLayout *inputLayout);
	static void setPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY topology);

	// Sixteen slots, matching DX9's cms_samplers. The engine's own stage count is
	// smaller, but the shaders address samplers by register and the reflection pass
	// packs them densely, so the shadow covers what a ps_4_0 program can name.
	static constexpr int cms_shaderResourceSlots = 16;

	// A debug lever: draw everything untextured. DX9 has the same switch and uses it the same
	// way, to tell "the texture is wrong" apart from "the geometry or the shader is wrong".
	static void setTexturesEnabled(bool enabled);

	static void setShaderResource(int slot, ID3D11ShaderResourceView *view);
	static void setSamplerState(int slot, ID3D11SamplerState *sampler);

	// The applied material's specular power. Recorded rather than uploaded: in the shipped
	// pixel layout the power is the w component of the dot3 light direction register, so the
	// light manager writes it as part of that register rather than on its own. DX9 keeps it
	// here for the same reason.
	static void setSpecularPower(float power);
	static float getSpecularPower();

	// A texture is about to be destroyed. Unbind it from every slot holding it and
	// clear the shadow.
	//
	// This is not optional and it is not a debug aid. Without it the shadow keeps a
	// freed address; the very next texture the allocator hands out at that address
	// compares equal, the bind is skipped as redundant, and the draw samples whatever
	// the context still has -- a wrong image with nothing in the log. DX9 gets this
	// right (Direct3d9_TextureData's destructor calls destroyTexture before Release)
	// and it is the single easiest thing to leave out of a port.
	static void destroyShaderResource(ID3D11ShaderResourceView *view);

	// A resource is about to become a render target. Unbind every shader resource slot
	// looking at it, and clear the shadow.
	//
	// Same failure as destroyShaderResource guards, arrived at differently: D3D11 will not
	// let a resource be a render target and a shader resource at once, and it resolves the
	// conflict by unbinding the shader resource without telling anyone outside the debug
	// layer. The shadow would then skip the next bind of that view as redundant and the draw
	// would sample whatever the context really has.
	static void unbindShaderResourcesForResource(ID3D11Resource *resource);

private:
	Direct3d11_StateCache();
	Direct3d11_StateCache(Direct3d11_StateCache const &);
	Direct3d11_StateCache &operator=(Direct3d11_StateCache const &);
};

// ======================================================================

#endif
