// ======================================================================
//
// Direct3d11_RenderTarget.h
// copyright (c) 2026 Galaxies Reborn
//
// Which surface the pipeline is drawing into, and the render-to-texture paths.
//
// ----------------------------------------------------------------------
// The single authority on what is bound
//
// Before this class existed, Direct3d11_SwapChain bound and cleared the scene target directly.
// That is only correct while the scene target is the only target there is. Everything that
// clears, or rebinds at beginScene, now asks this class instead -- otherwise a clear issued
// inside a texture frame would clear the scene and leave the texture untouched, which is a bug
// that produces a plausible-looking frame and a wrong texture.
//
// setRenderTarget(NULL) returns to the scene target, never to the back buffer. The back buffer
// is written exactly once per frame, by the composite.
//
// ----------------------------------------------------------------------
// Two paths, because the engine has two kinds of destination
//
//   A texture created with TCF_renderTarget. Bloom's three buffers, the heat buffer and the
//   post-processing primary buffer are all this. A render target view onto the requested mip
//   and cube face, bound directly. No copy.
//
//   A texture that is NOT a render target. The runtime texture baker
//   (BlueprintTextureRendererTemplate, which builds customised character and clothing
//   textures) renders every mip level of an ordinary texture this way, then asks for the
//   result to be copied in.
//
// The second path is where D3D11 is straightforwardly better than what it replaces, and the
// difference is worth stating because the DX9 code looks bizarre without it. D3D9 could not
// copy from a render target into a managed-pool texture at all, so
// Direct3d9_RenderTarget renders into a shared 512x512 X8R8G8B8 target, picks a mip of that
// shared target big enough to cover the request, reads it back to system memory with
// GetRenderTargetData, and then pushes it into the destination with
// D3DXLoadSurfaceFromSurface. That is a full GPU-to-CPU-to-GPU round trip per mip, per baked
// texture, and it is why the 512 cap and the mip-selection ladder exist.
//
// D3D11 has CopySubresourceRegion, which is GPU to GPU. So this renders into a scratch target
// of the exact size and format required and copies straight across. No readback, no stall, no
// size cap, no mip ladder, and Direct3d11_Metrics::blockingStagingMaps stays at zero where the
// D3D9 shape would have driven it up once per baked mip.
//
// The one case the direct copy cannot serve is a destination whose format cannot be a render
// target -- a block-compressed one. D3DXLoadSurfaceFromSurface compressed on the CPU as part
// of its conversion; nothing in D3D11 does. That case reports itself by name rather than
// silently producing a blank texture; see the note at the copy for what to build if it fires.
//
// ----------------------------------------------------------------------
// Three details D3D9 gave away for free
//
//   Binding a render target resets the viewport. D3D9's SetRenderTarget sets the viewport to
//   the full extent of the new target as a side effect. D3D11 leaves the viewport alone, so a
//   smaller target bound while a full-screen viewport is set would scissor away everything
//   outside the old rectangle. Replicated explicitly on every bind.
//
//   Depth. D3D9 allowed a render target smaller than the bound depth-stencil surface; D3D11
//   requires them to match. So: the scene depth is bound when the sizes agree, which is the
//   exact-parity case; a cached depth buffer of matching size is bound when they do not; and
//   NULL is bound for the baked path, which is what DX9 does there explicitly.
//
//   Views are cached, not created per bind. Direct3d11_Metrics requires zero resource
//   creations inside a frame, and Bloom rebinds the same three textures every frame.
//
// ======================================================================

#ifndef INCLUDED_Direct3d11_RenderTarget_H
#define INCLUDED_Direct3d11_RenderTarget_H

// ======================================================================

#include "clientGraphics/Texture.def"

#include <d3d11_1.h>

class Texture;

// ======================================================================

class Direct3d11_RenderTarget
{
public:

	static void install();
	static void remove();

	static void setRenderTargetToPrimary();
	static void setRenderTarget(Texture *texture, CubeFace cubeFace, int mipmapLevel);
	static bool copyRenderTargetToNonRenderTargetTexture();

	// What is bound right now. Clears and rebinds go through these rather than reaching for
	// the scene target.
	static ID3D11RenderTargetView *getCurrentRenderTargetView();
	static ID3D11DepthStencilView *getCurrentDepthStencilView();
	static int                     getCurrentWidth();
	static int                     getCurrentHeight();

	// Rebind whatever is current. Called from beginScene, which must not stomp a texture
	// target the engine bound before it.
	static void bindCurrent();

	// The scene target's buffers have been replaced -- a resize, or a multisampling change.
	// Every view this class holds onto it is now stale, so drop them and rebind.
	static void sceneTargetRebuilt();

	// A texture resource is about to be released. Drop every view cached onto it.
	//
	// Not optional: a cached view outliving its resource is a dangling COM pointer that the
	// next allocation at the same address makes look valid.
	static void releaseViewsFor(ID3D11Resource *resource);

private:

	Direct3d11_RenderTarget();
	Direct3d11_RenderTarget(Direct3d11_RenderTarget const &);
	Direct3d11_RenderTarget &operator =(Direct3d11_RenderTarget const &);
};

// ======================================================================

#endif
