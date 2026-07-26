// ======================================================================
//
// Direct3d11_SceneTarget.h
// copyright (c) 2026 Galaxies Reborn
//
// The offscreen colour and depth-stencil buffers the scene is rendered into,
// and the composite that puts them on the swap chain.
//
// One decision that solves five problems the flip model otherwise creates:
//
//   lockBackBuffer   a known-format texture can be copied to staging; a
//                    FLIP_DISCARD back buffer cannot be mapped at all
//   screenShot       capture reads a surface that still exists after Present
//   gamma            there is somewhere to apply colour correction, where
//                    SetGammaRamp has no DXGI equivalent that works windowed
//   MSAA             a multisampled scene target resolves at an obvious point,
//                    which a multisampled flip-model swap chain does not allow
//   presentToWindow  a per-window swap chain can be fed from the composite
//
// setRenderTarget(NULL) returns here, never to the back buffer. The back buffer
// is written exactly once per frame, by the composite.
//
// ======================================================================

#ifndef INCLUDED_Direct3d11_SceneTarget_H
#define INCLUDED_Direct3d11_SceneTarget_H

// ======================================================================

#include <d3d11_1.h>

// ======================================================================

class Direct3d11_SceneTarget
{
public:

	static bool                      install(int width, int height);
	static void                      remove();

	static bool                      resize(int width, int height);

	static ID3D11RenderTargetView   *getRenderTargetView();
	static ID3D11DepthStencilView   *getDepthStencilView();

	// Chosen by probing at install. Off-screen depth buffers created elsewhere use it so they
	// cannot disagree with the scene's.
	static DXGI_FORMAT               getDepthFormat();

	// One when multisampling is off. Anything binding its own colour buffer against the scene's
	// depth has to match this, because D3D11 requires the sample counts to agree.
	static int                       getSampleCount();

	// Turn multisampling on or off at runtime, rebuilding the buffers. Returns whether the
	// buffers are intact afterwards -- a false means the scene target is gone and the frame
	// cannot be drawn.
	//
	// D3D9 did this by recreating the whole device, because the multisample mode was a property
	// of the swap chain there. Here the swap chain is single-sampled by design and the scene
	// target is the only multisampled surface, so only it has to be rebuilt.
	static bool                      setAntialiasEnabled(bool enabled);
	static int                       getWidth();
	static int                       getHeight();

	// Draw the scene target onto whatever render target is currently bound,
	// applying colour correction if any is configured. Restores nothing: the
	// caller owns the context state around it.
	static void                      composite();

	// True when brightness, contrast and gamma are all exactly 1, in which case
	// composite() copies without touching a single channel value.
	static bool                      isColorCorrectionIdentity();

	static void                      setBrightnessContrastGamma(float brightness, float contrast, float gamma);

	// The 256-entry 8-bit table DX9 builds for the same settings. Exposed so the
	// capture path can reproduce DX9's screenshot behaviour exactly.
	static uint8 const              *getColorCorrectionTable();

	// A staging copy of the scene target, mappable for reading. Owned by this
	// class; valid until the next call or until remove().
	static ID3D11Texture2D          *createReadbackCopy();

private:

	Direct3d11_SceneTarget();
	Direct3d11_SceneTarget(Direct3d11_SceneTarget const &);
	Direct3d11_SceneTarget &operator =(Direct3d11_SceneTarget const &);
};

// ======================================================================

#endif
