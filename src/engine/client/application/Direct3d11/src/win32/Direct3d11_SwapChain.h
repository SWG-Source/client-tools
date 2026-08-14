// ======================================================================
//
// Direct3d11_SwapChain.h
// copyright (c) 2026 Galaxies Reborn
//
// The swap chain, the back buffer, the depth-stencil buffer, and the game
// window itself.
//
// The window is the backend's responsibility. Os creates it hidden, WS_POPUP,
// 640x480 at the origin, and never shows it (Os.cpp:142-155), so a backend that
// only creates a swap chain renders correct frames into a window nobody sees.
// DX9 does this work in Direct3d9Namespace::updateWindowSettings; the same is
// done here, and skipped entirely when the engine does not own the window,
// which is how the editors host the renderer in their own widget.
//
// Flip model, borderless, never SetFullscreenState. Exclusive fullscreen is what
// makes alt-tab expensive and it is what the whole lost-device apparatus in the
// DX9 backend exists to survive; a borderless window on a mode-matched desktop
// looks the same and cannot lose its device.
//
// ======================================================================

#ifndef INCLUDED_Direct3d11_SwapChain_H
#define INCLUDED_Direct3d11_SwapChain_H

// ======================================================================

#include <d3d11_1.h>
#include <dxgi1_5.h>

struct Gl_install;

// ======================================================================

class Direct3d11_SwapChain
{
public:
	static bool install(Gl_install *gl_install);
	static void remove();

	// The window the swap chain is attached to. Needed by anything that has to talk to the
	// window system rather than to D3D -- the mouse cursor is the only such thing so far.
	static HWND getWindow();

	static int getWidth();
	static int getHeight();
	static bool isWindowed();

	// Gl_api surface this class answers.
	static void clearViewport(bool clearColor, uint32 colorValue, bool clearDepth, real depthValue, bool clearStencil, uint32 stencilValue);
	static void beginScene();
	static void endScene();
	static bool present();
	static void resize(int width, int height);
	static void setWindowedMode(bool windowed);
	static void setViewport(int x, int y, int width, int height, real minZ, real maxZ);
	static void flushResources(bool fullReset);

private:
	Direct3d11_SwapChain();
	Direct3d11_SwapChain(Direct3d11_SwapChain const &);
	Direct3d11_SwapChain &operator=(Direct3d11_SwapChain const &);
};

// ======================================================================

#endif
