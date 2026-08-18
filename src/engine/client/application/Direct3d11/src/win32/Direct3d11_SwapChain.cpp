// ======================================================================
//
// Direct3d11_SwapChain.cpp
// copyright (c) 2026 Galaxies Reborn
//
// ======================================================================

#include "FirstDirect3d11.h"
#include "Direct3d11_SwapChain.h"

#include "ConfigDirect3d11.h"
#include "Direct3d11_Device.h"
#include "Direct3d11_ImageWriter.h"

// TEMPORARY DIAGNOSTIC: RenderDoc's in-application API, from the installed SDK.
#include "C:/Program Files/RenderDoc/renderdoc_app.h"
#include "Direct3d11_StateCache.h"
#include "Direct3d11_Metrics.h"
#include "Direct3d11_ConstantBuffers.h"
#include "Direct3d11_RenderTarget.h"
#include "Direct3d11_SceneTarget.h"

#include "clientGraphics/Gl_dll.def"

// ======================================================================

namespace Direct3d11_SwapChainNamespace
{
	constexpr int cs_debugScreenshotIntervalFrames = 600;
	constexpr int cs_debugScreenshotQualityPercent = 100;
	constexpr int cs_debugScreenshotNameBytes = 64;

	bool ms_installed;

	HWND ms_window;
	bool ms_engineOwnsWindow;
	bool ms_borderlessWindow;
	bool ms_windowed = true;
	int ms_windowX;
	int ms_windowY;
	void (*ms_windowedModeChanged)(bool windowed);

	IDXGISwapChain1 *ms_swapChain;
	ID3D11RenderTargetView *ms_backBufferView;
	ID3D11Texture2D *ms_depthStencilBuffer;
	ID3D11DepthStencilView *ms_depthStencilView;
	DXGI_FORMAT ms_depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	int ms_width;
	int ms_height;
	UINT ms_swapChainFlags;
	bool ms_warnedAboutSubRectDepthClear;

	D3D11_VIEWPORT ms_viewport;

	// The engine never re-pushes its resolution to the backend, so the window
	// position sentinel matters: Graphics uses INT_MAX for "no saved position",
	// not 0 and not -1 (Graphics.cpp:103-104).
	int const cms_noSavedWindowPosition = 0x7fffffff;

	bool createSwapChain();
	bool createBackBufferViews();
	void releaseBackBufferViews();
	void updateWindowSettings();
} // namespace Direct3d11_SwapChainNamespace
using namespace Direct3d11_SwapChainNamespace;

// ======================================================================
/**
 * Give the hidden 640x480 popup Os handed us the style, size, position and
 * visibility the configuration asks for.
 *
 * Skipped when the engine does not own the window: in that case the host
 * application owns its style and placement and we are a guest in its client
 * area.
 */

void Direct3d11_SwapChainNamespace::updateWindowSettings()
{
	if (!ms_engineOwnsWindow)
		return;

	// Borderless for a windowed-fullscreen presentation, a normal caption
	// otherwise. Never WS_EX_TOPMOST: it survives alt-tab and traps the desktop
	// behind a window the user cannot get past.
	DWORD style = WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	if (ms_windowed && !ms_borderlessWindow)
		style |= WS_OVERLAPPEDWINDOW;
	else
		style |= WS_POPUP;

	IGNORE_RETURN(SetWindowLongPtr(ms_window, GWL_STYLE, static_cast<LONG_PTR>(style)));

	// The requested size is a CLIENT size. A caption and borders sit outside it,
	// so the window rectangle has to be grown by however much this style costs,
	// or the rendered image is scaled into a client area smaller than the swap
	// chain and every mouse coordinate is off.
	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = ms_width;
	rect.bottom = ms_height;
	IGNORE_RETURN(AdjustWindowRect(&rect, style, FALSE));

	int const windowWidth = rect.right - rect.left;
	int const windowHeight = rect.bottom - rect.top;

	int x = ms_windowX;
	int y = ms_windowY;

	if (x == cms_noSavedWindowPosition || y == cms_noSavedWindowPosition)
	{
		// No saved position: centre on the primary display.
		int const screenWidth = GetSystemMetrics(SM_CXSCREEN);
		int const screenHeight = GetSystemMetrics(SM_CYSCREEN);
		x = (screenWidth - windowWidth) / 2;
		y = (screenHeight - windowHeight) / 2;
		if (x < 0)
			x = 0;
		if (y < 0)
			y = 0;
	}

	// The reveal must ACTIVATE. This is the call that first shows the hidden
	// popup Os created, and D3D9 reveals with SWP_SHOWWINDOW and no
	// SWP_NOACTIVATE -- the show itself grants focus. Passing SWP_NOACTIVATE
	// here left the client behind whatever else the desktop had focused at
	// startup (the trailing ShowWindow(SW_SHOW) does not reliably activate a
	// window that SetWindowPos just made visible without activation).
	IGNORE_RETURN(SetWindowPos(ms_window, NULL, x, y, windowWidth, windowHeight, SWP_NOZORDER | SWP_SHOWWINDOW | SWP_FRAMECHANGED));
	IGNORE_RETURN(ShowWindow(ms_window, SW_SHOW));
	IGNORE_RETURN(UpdateWindow(ms_window));
}

// ======================================================================

bool Direct3d11_SwapChainNamespace::createSwapChain()
{
	IDXGIFactory2 *const factory = Direct3d11_Device::getFactory();
	NOT_NULL(factory);

	bool const wantTearing = ConfigDirect3d11::getAllowTearing() && Direct3d11_Device::supportsTearing();

	ms_swapChainFlags = wantTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

	DXGI_SWAP_CHAIN_DESC1 description;
	Zero(description);
	description.Width = static_cast<UINT>(ms_width);
	description.Height = static_cast<UINT>(ms_height);
	description.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	description.SampleDesc.Count = 1;
	description.SampleDesc.Quality = 0;
	description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	description.BufferCount = static_cast<UINT>(ConfigDirect3d11::getSwapChainBufferCount());
	description.Scaling = DXGI_SCALING_STRETCH;
	description.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	// The engine clears with a colour whose alpha is zero -- PackedRgb::asUint32
	// produces alpha 0 and every scene passes it -- so an alpha-aware
	// presentation would composite the window as transparent.
	description.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
	description.Flags = ms_swapChainFlags;

	HRESULT hresult = factory->CreateSwapChainForHwnd(Direct3d11_Device::getDevice(), ms_window, &description, NULL, NULL, &ms_swapChain);

	if (FAILED(hresult) && ms_swapChainFlags)
	{
		WARNING(true, ("Direct3d11: the swap chain could not be created with tearing allowed (%s), retrying without it.", Direct3d11_Device::describeHresult(hresult)));
		ms_swapChainFlags = 0;
		description.Flags = 0;
		hresult = factory->CreateSwapChainForHwnd(Direct3d11_Device::getDevice(), ms_window, &description, NULL, NULL, &ms_swapChain);
	}

	if (FAILED(hresult) || !ms_swapChain)
	{
		WARNING(true, ("Direct3d11: CreateSwapChainForHwnd failed (%s).", Direct3d11_Device::describeHresult(hresult)));
		return false;
	}

	// DXGI's own alt-enter handler would put us into exclusive fullscreen behind
	// the engine's back, which is a mode this backend does not implement and does
	// not want.
	IGNORE_RETURN(factory->MakeWindowAssociation(ms_window, DXGI_MWA_NO_ALT_ENTER));

	return true;
}

// ----------------------------------------------------------------------

bool Direct3d11_SwapChainNamespace::createBackBufferViews()
{
	ID3D11Device1 *const device = Direct3d11_Device::getDevice();
	NOT_NULL(device);

	ID3D11Texture2D *backBuffer = NULL;
	HRESULT hresult = ms_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&backBuffer));
	if (FAILED(hresult) || !backBuffer)
	{
		WARNING(true, ("Direct3d11: IDXGISwapChain1::GetBuffer failed (%s).", Direct3d11_Device::describeHresult(hresult)));
		return false;
	}

	hresult = device->CreateRenderTargetView(backBuffer, NULL, &ms_backBufferView);
	backBuffer->Release();
	if (FAILED(hresult) || !ms_backBufferView)
	{
		WARNING(true, ("Direct3d11: CreateRenderTargetView on the back buffer failed (%s).", Direct3d11_Device::describeHresult(hresult)));
		return false;
	}

	// Stencil is not optional: the shadow volume passes count into it, so a
	// depth-only format would silently lose every stencil shadow.
	DXGI_FORMAT const formats[] = {DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_FORMAT_D32_FLOAT_S8X24_UINT};

	for (int i = 0; i < 2; ++i)
	{
		D3D11_TEXTURE2D_DESC depthDescription;
		Zero(depthDescription);
		depthDescription.Width = static_cast<UINT>(ms_width);
		depthDescription.Height = static_cast<UINT>(ms_height);
		depthDescription.MipLevels = 1;
		depthDescription.ArraySize = 1;
		depthDescription.Format = formats[i];
		depthDescription.SampleDesc.Count = 1;
		depthDescription.Usage = D3D11_USAGE_DEFAULT;
		depthDescription.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		hresult = device->CreateTexture2D(&depthDescription, NULL, &ms_depthStencilBuffer);
		if (SUCCEEDED(hresult) && ms_depthStencilBuffer)
		{
			ms_depthStencilFormat = formats[i];
			break;
		}
	}

	if (!ms_depthStencilBuffer)
	{
		WARNING(true, ("Direct3d11: no depth-stencil format with stencil could be created (%s).", Direct3d11_Device::describeHresult(hresult)));
		return false;
	}

	hresult = device->CreateDepthStencilView(ms_depthStencilBuffer, NULL, &ms_depthStencilView);
	if (FAILED(hresult) || !ms_depthStencilView)
	{
		WARNING(true, ("Direct3d11: CreateDepthStencilView failed (%s).", Direct3d11_Device::describeHresult(hresult)));
		return false;
	}

	// Come up with a viewport covering the whole target, so a frame that presents
	// before anything calls setViewport is still well defined.
	ms_viewport.TopLeftX = 0.0f;
	ms_viewport.TopLeftY = 0.0f;
	ms_viewport.Width = static_cast<float>(ms_width);
	ms_viewport.Height = static_cast<float>(ms_height);
	ms_viewport.MinDepth = 0.0f;
	ms_viewport.MaxDepth = 1.0f;

	ID3D11DeviceContext1 *const context = Direct3d11_Device::getContext();
	context->RSSetViewports(1, &ms_viewport);
	context->OMSetRenderTargets(1, &ms_backBufferView, ms_depthStencilView);

	return true;
}

// ----------------------------------------------------------------------

void Direct3d11_SwapChainNamespace::releaseBackBufferViews()
{
	ID3D11DeviceContext1 *const context = Direct3d11_Device::getContext();
	if (context)
		context->OMSetRenderTargets(0, NULL, NULL);

	if (ms_depthStencilView)
	{
		ms_depthStencilView->Release();
		ms_depthStencilView = NULL;
	}

	if (ms_depthStencilBuffer)
	{
		ms_depthStencilBuffer->Release();
		ms_depthStencilBuffer = NULL;
	}

	if (ms_backBufferView)
	{
		ms_backBufferView->Release();
		ms_backBufferView = NULL;
	}
}

// ======================================================================

bool Direct3d11_SwapChain::install(Gl_install *gl_install)
{
	NOT_NULL(gl_install);
	DEBUG_FATAL(ms_installed, ("Direct3d11_SwapChain::install called twice"));

	// Gl_install is a stack local in Graphics::install and is invalid the moment
	// install returns, so everything needed later is copied now.
	ms_window = gl_install->window;
	ms_width = gl_install->width;
	ms_height = gl_install->height;
	ms_windowed = gl_install->windowed;
	ms_engineOwnsWindow = gl_install->engineOwnsWindow;
	ms_borderlessWindow = gl_install->borderlessWindow;
	ms_windowX = gl_install->windowX;
	ms_windowY = gl_install->windowY;
	ms_windowedModeChanged = gl_install->windowedModeChanged;

	FATAL(!ms_window, ("Direct3d11: install was given a null window."));
	FATAL(ms_width <= 0 || ms_height <= 0, ("Direct3d11: install was given a %dx%d frame buffer.", ms_width, ms_height));

	updateWindowSettings();

	if (!createSwapChain())
		return false;

	if (!createBackBufferViews())
		return false;

	if (!Direct3d11_SceneTarget::install(ms_width, ms_height))
		return false;

	// The engine reads width, height and windowed back out of Gl_install and
	// builds its UI canvas size, mouse clip rectangle and viewport bounds checks
	// from them. Anything clamped or refused here has to be reported back, or
	// every one of those is quietly wrong.
	gl_install->width = ms_width;
	gl_install->height = ms_height;
	gl_install->windowed = ms_windowed;

	if (!gl_install->skipInitialClearViewport)
		Direct3d11_SwapChain::clearViewport(true, 0, true, 1.0f, true, 0);

	ms_installed = true;
	return true;
}

// ----------------------------------------------------------------------

void Direct3d11_SwapChain::remove()
{
	Direct3d11_SceneTarget::remove();

	releaseBackBufferViews();

	if (ms_swapChain)
	{
		ms_swapChain->Release();
		ms_swapChain = NULL;
	}

	ms_installed = false;
}

// ----------------------------------------------------------------------

HWND Direct3d11_SwapChain::getWindow()
{
	return ms_window;
}

// ----------------------------------------------------------------------

int Direct3d11_SwapChain::getWidth()
{
	return ms_width;
}

// ----------------------------------------------------------------------

int Direct3d11_SwapChain::getHeight()
{
	return ms_height;
}

// ----------------------------------------------------------------------

bool Direct3d11_SwapChain::isWindowed()
{
	return ms_windowed;
}

// ======================================================================
/**
 * Clear some or all of the current target.
 *
 * The argument is a D3DCOLOR: 0xAARRGGBB. Alpha is forced opaque because every
 * caller in the engine passes a PackedRgb whose alpha byte is zero, and a
 * transparent back buffer composites the window against the desktop.
 *
 * The rectangle matters. D3D9's Clear is bounded by the viewport and the scissor
 * rectangle; ClearRenderTargetView ignores both. ShaderPrimitiveSorter clears
 * one primitive's footprint inside a shared heat buffer that way, and
 * GroundScene's letterbox bars depend on it, so a whole-target clear would erase
 * work already done this frame with nothing logged. Colour therefore clears
 * through ClearView with the viewport rectangle.
 *
 * Depth and stencil have no rectangle-limited clear in D3D11 -- doing it
 * properly needs a full-screen draw with the depth state set, which arrives with
 * the state objects. Until then a sub-rectangle depth clear says so once rather
 * than silently clearing everything.
 */

void Direct3d11_SwapChain::clearViewport(bool clearColor, uint32 colorValue, bool clearDepth, real depthValue, bool clearStencil, uint32 stencilValue)
{
	DX11_ASSERT_MAIN_THREAD();

	ID3D11DeviceContext1 *const context = Direct3d11_Device::getContext();
	if (!context)
		return;

	// Whatever is bound, which is the scene target most of the time but is a texture inside a
	// texture frame. Asking the render target class rather than the scene target directly is
	// the difference between clearing the surface being drawn into and clearing the scene.
	//
	// Never the back buffer either way: that is written exactly once per frame, by the
	// composite.
	ID3D11RenderTargetView *const colorView = Direct3d11_RenderTarget::getCurrentRenderTargetView();
	ID3D11DepthStencilView *const depthView = Direct3d11_RenderTarget::getCurrentDepthStencilView();

	int const targetWidth = Direct3d11_RenderTarget::getCurrentWidth();
	int const targetHeight = Direct3d11_RenderTarget::getCurrentHeight();

	bool const wholeTarget =
		ms_viewport.TopLeftX == 0.0f &&
		ms_viewport.TopLeftY == 0.0f &&
		ms_viewport.Width == static_cast<float>(targetWidth) &&
		ms_viewport.Height == static_cast<float>(targetHeight);

	if (clearColor && colorView)
	{
		float const color[4] =
			{
				static_cast<float>((colorValue >> 16) & 0xff) / 255.0f,
				static_cast<float>((colorValue >> 8) & 0xff) / 255.0f,
				static_cast<float>((colorValue) & 0xff) / 255.0f,
				1.0f};

		if (wholeTarget)
			context->ClearRenderTargetView(colorView, color);
		else
		{
			D3D11_RECT rect;
			rect.left = static_cast<LONG>(ms_viewport.TopLeftX);
			rect.top = static_cast<LONG>(ms_viewport.TopLeftY);
			rect.right = static_cast<LONG>(ms_viewport.TopLeftX + ms_viewport.Width);
			rect.bottom = static_cast<LONG>(ms_viewport.TopLeftY + ms_viewport.Height);
			context->ClearView(colorView, color, &rect, 1);
		}
	}

	if ((clearDepth || clearStencil) && depthView)
	{
		UINT flags = 0;
		if (clearDepth)
			flags |= D3D11_CLEAR_DEPTH;
		if (clearStencil)
			flags |= D3D11_CLEAR_STENCIL;

		if (!wholeTarget && !ms_warnedAboutSubRectDepthClear)
		{
			ms_warnedAboutSubRectDepthClear = true;
			WARNING(true, ("Direct3d11: a depth/stencil clear was asked for inside a sub-viewport, which D3D11 cannot do directly. The whole depth buffer was cleared instead."));
		}

		context->ClearDepthStencilView(depthView, flags, static_cast<FLOAT>(depthValue), static_cast<UINT8>(stencilValue));
	}
}

// ----------------------------------------------------------------------
/**
 * Start a frame.
 *
 * Rebinding the render target is not redundant. In the flip model, Present
 * unbinds the back buffer from every output-merger slot, and D3D11 reports a
 * draw into an unbound target only as a debug-layer warning. The texture-bake
 * path can also leave a texture's own view bound between its endScene and this
 * one.
 */

void Direct3d11_SwapChain::beginScene()
{
	DX11_ASSERT_MAIN_THREAD();

	ID3D11DeviceContext1 *const context = Direct3d11_Device::getContext();
	if (!context)
		return;

	// Rebind whatever is current rather than the scene target unconditionally. The texture
	// baker calls beginScene and then setRenderTarget, so binding the scene here is right for
	// it; but it also calls beginScene once per mip inside a sequence that has a texture bound,
	// and stomping that would send a mip's geometry to the screen instead of the texture.
	Direct3d11_RenderTarget::bindCurrent();

	// The engine's viewport, restored on top of the full-target one the bind sets.
	context->RSSetViewports(1, &ms_viewport);
}

// ----------------------------------------------------------------------

void Direct3d11_SwapChain::endScene()
{
	DX11_ASSERT_MAIN_THREAD();

	// D3D9 needed a matched BeginScene/EndScene pair around every draw. D3D11
	// has no such concept, so there is genuinely nothing to do here. Left as a
	// real implementation rather than routed through the unimplemented
	// accounting, because "nothing to do" and "not written yet" are different
	// claims and the log should not confuse them.
}

// ----------------------------------------------------------------------
/**
 * Show the frame.
 *
 * No caller checks the return value -- Game::run wraps both present calls in
 * IGNORE_RETURN -- so a failure that only returns false would leave a frozen
 * window and an empty log. Failures are reported here instead.
 */

// ----------------------------------------------------------------------
// Consumer overlay callbacks (Gl_api v35 tail slots) -- see the header note.
// Written from the consumer's thread at registration, read on the render
// thread: a raw aligned pointer store, and each invoke site snapshots it.

namespace Direct3d11_SwapChainConsumerCallbacks
{
	void (*s_frameCallback)() = 0;
	void (*s_resizeCallback)(int phase, int width, int height) = 0;
}
using namespace Direct3d11_SwapChainConsumerCallbacks;

void Direct3d11_SwapChain::setConsumerFrameCallback(void (*fn)())
{
	s_frameCallback = fn;
	REPORT_LOG(true, ("Direct3d11: consumer frame callback %s\n", fn ? "REGISTERED" : "cleared"));
}

void Direct3d11_SwapChain::setConsumerResizeCallback(void (*fn)(int phase, int width, int height))
{
	s_resizeCallback = fn;
	REPORT_LOG(true, ("Direct3d11: consumer resize callback %s\n", fn ? "REGISTERED" : "cleared"));
}

// ----------------------------------------------------------------------

bool Direct3d11_SwapChain::present()
{
	DX11_ASSERT_MAIN_THREAD();

	if (!ms_swapChain)
		return false;

	// Whatever the debug layer complained about during this frame. No-op unless debugLayer is on.
	Direct3d11_Device::drainDebugMessages();

	// One check per frame that the redundancy cache still describes the device. Costs four
	// interface calls a frame and only when the debug layer is on; catches any path that writes
	// device state without going through the cache, in the frame it happens rather than in a
	// linkage error a thousand draws later.
	if (ConfigDirect3d11::getDebugLayer())
		IGNORE_RETURN(Direct3d11_StateCache::auditAgainstDevice("end of frame"));

	// The one place the back buffer is written. Bound here rather than left bound
	// from the last frame, because the flip model unbinds it at Present.
	ID3D11DeviceContext1 *const context = Direct3d11_Device::getContext();
	if (context && ms_backBufferView)
	{
		context->OMSetRenderTargets(1, &ms_backBufferView, NULL);

		D3D11_VIEWPORT fullTarget;
		fullTarget.TopLeftX = 0.0f;
		fullTarget.TopLeftY = 0.0f;
		fullTarget.Width = static_cast<float>(ms_width);
		fullTarget.Height = static_cast<float>(ms_height);
		fullTarget.MinDepth = 0.0f;
		fullTarget.MaxDepth = 1.0f;
		context->RSSetViewports(1, &fullTarget);

		Direct3d11_SceneTarget::composite();
	}

	// TEMPORARY DIAGNOSTIC: ask RenderDoc for a capture at a chosen frame.
	{
		int const captureFrame = ConfigDirect3d11::getDebugRenderDocFrame();
		if (captureFrame > 0)
		{
			static int renderDocFrameNumber = 0;
			++renderDocFrameNumber;

			if (renderDocFrameNumber == captureFrame)
			{
				// Present in the process only when launched under renderdoccmd, so a miss here is
				// the ordinary case and not worth a warning beyond saying so once.
				HMODULE const renderDoc = GetModuleHandleA("renderdoc.dll");
				if (renderDoc)
				{
					pRENDERDOC_GetAPI const getApi = reinterpret_cast<pRENDERDOC_GetAPI>(GetProcAddress(renderDoc, "RENDERDOC_GetAPI"));
					RENDERDOC_API_1_1_2 *api = NULL;

					if (getApi && getApi(eRENDERDOC_API_Version_1_1_2, reinterpret_cast<void **>(&api)) == 1 && api)
					{
						api->TriggerCapture();
						WARNING(true, ("Direct3d11: asked RenderDoc to capture the frame after %d.", captureFrame));
					}
					else
					{
						WARNING(true, ("Direct3d11: renderdoc.dll is loaded but its API could not be obtained."));
					}
				}
				else
				{
					WARNING(true, ("Direct3d11: debugRenderDocFrame is set but renderdoc.dll is not loaded; launch under renderdoccmd."));
				}
			}
		}
	}

	// TEMPORARY DIAGNOSTIC: an automatic screenshot, taken before Present while the back buffer
	// still holds this frame. Written through Direct3d11_ImageWriter, which is the same path the
	// game's own screenshot key uses, so what lands on disk is what the renderer produced rather
	// than whatever the desktop compositor had on top.
	{
		int const shotFrame = ConfigDirect3d11::getDebugScreenshotFrame();
		if (shotFrame > 0)
		{
			static int frameNumber = 0;
			++frameNumber;

			// The requested frame, then every 600 after it, so a run yields a few views without
			// filling the disk.
			if (frameNumber == shotFrame || (frameNumber > shotFrame && ((frameNumber - shotFrame) % cs_debugScreenshotIntervalFrames) == 0))
			{
				char name[cs_debugScreenshotNameBytes];
				sprintf(name, "debugshot_%05d", frameNumber);
				IGNORE_RETURN(Direct3d11_ImageWriter::screenShot(GSSF_tga, cs_debugScreenshotQualityPercent, name));
			}
		}
	}

	// Consumer overlay draw point (Gl_api v35): the back buffer holds the finished
	// frame -- after the composite/gamma writes and the debug screenshot read,
	// before Present. Snapshot so a concurrent clear cannot fault mid-call.
	{
		void (*const frameCallback)() = s_frameCallback;
		if (frameCallback)
			frameCallback();
	}

	UINT const syncInterval = (ms_swapChainFlags & DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING) ? 0 : 1;
	UINT const presentFlags = (ms_swapChainFlags & DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING) ? DXGI_PRESENT_ALLOW_TEARING : 0;

	HRESULT const hresult = ms_swapChain->Present(syncInterval, presentFlags);

	++Direct3d11_Metrics::presents;

	Direct3d11_Device::checkForDeviceRemoved(hresult, "Present");

	if (hresult == DXGI_STATUS_OCCLUDED)
	{
		// The window is fully hidden. Not an error, and not worth a log line
		// every frame while the user has something else maximised.
		return true;
	}

	if (FAILED(hresult))
	{
		++Direct3d11_Metrics::presentFailures;
		WARNING(true, ("Direct3d11: Present failed (%s).", Direct3d11_Device::describeHresult(hresult)));
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------
/**
 * Change the size of the frame buffer.
 *
 * Note what DX9 does NOT do here: it does not resize the OS window. In windowed
 * mode the smaller back buffer is stretched into the unchanged client area at
 * Present, which is exactly how a cut scene displays a 640x480 video in a
 * full-size window. DXGI_SCALING_STRETCH on the swap chain reproduces that, so
 * the window is deliberately left alone.
 */

void Direct3d11_SwapChain::resize(int width, int height)
{
	DX11_ASSERT_MAIN_THREAD();

	if (!ms_swapChain || width <= 0 || height <= 0)
		return;

	if (width == ms_width && height == ms_height)
		return;

	// The two real subscribers -- Bink's video surface and the post-processing
	// manager -- size their render targets to the frame buffer, so they have to
	// let go before the buffers change and rebuild afterwards.
	Direct3d11_Device::fireDeviceLost();

	// Consumer resize phase 0 (Gl_api v35): the consumer must release every
	// back-buffer-referencing view NOW -- an outstanding reference fails
	// ResizeBuffers.
	{
		void (*const resizeCallback)(int, int, int) = s_resizeCallback;
		if (resizeCallback)
			resizeCallback(0, width, height);
	}

	releaseBackBufferViews();

	ms_width = width;
	ms_height = height;

	HRESULT const hresult = ms_swapChain->ResizeBuffers(0, static_cast<UINT>(width), static_cast<UINT>(height), DXGI_FORMAT_UNKNOWN, ms_swapChainFlags);
	Direct3d11_Device::checkForDeviceRemoved(hresult, "ResizeBuffers");
	FATAL(FAILED(hresult), ("Direct3d11: ResizeBuffers to %dx%d failed (%s).", width, height, Direct3d11_Device::describeHresult(hresult)));

	IGNORE_RETURN(createBackBufferViews());
	IGNORE_RETURN(Direct3d11_SceneTarget::resize(width, height));

	// The scene target's views are new, and the render target class is holding the old ones.
	Direct3d11_RenderTarget::sceneTargetRebuilt();

	Direct3d11_Device::fireDeviceRestored();

	// Consumer resize phase 1 (Gl_api v35): the new views exist -- rebuild.
	{
		void (*const resizeCallback)(int, int, int) = s_resizeCallback;
		if (resizeCallback)
			resizeCallback(1, width, height);
	}
}

// ----------------------------------------------------------------------
/**
 * Switch between windowed and windowed-fullscreen.
 *
 * SetFullscreenState is never called. The callback at the end is unconditional,
 * exactly as DX9 does it: it is the only channel by which Graphics learns the
 * current mode, and calling it only on an actual change leaves
 * Graphics::isWindowed permanently stale -- which DirectInput is wired to.
 */

void Direct3d11_SwapChain::setWindowedMode(bool windowed)
{
	DX11_ASSERT_MAIN_THREAD();

	ms_windowed = windowed;

	updateWindowSettings();

	if (ms_windowedModeChanged)
		ms_windowedModeChanged(ms_windowed);
}

// ----------------------------------------------------------------------

void Direct3d11_SwapChain::setViewport(int x, int y, int width, int height, real minZ, real maxZ)
{
	DX11_ASSERT_MAIN_THREAD();

	ID3D11DeviceContext1 *const context = Direct3d11_Device::getContext();
	if (!context)
		return;

	ms_viewport.TopLeftX = static_cast<float>(x);
	ms_viewport.TopLeftY = static_cast<float>(y);
	ms_viewport.Width = static_cast<float>(width);
	ms_viewport.Height = static_cast<float>(height);
	ms_viewport.MinDepth = static_cast<float>(minZ);
	ms_viewport.MaxDepth = static_cast<float>(maxZ);

	context->RSSetViewports(1, &ms_viewport);
	++Direct3d11_Metrics::viewportSetCalls;

	// The other half, and it is not optional: the engine's 2D shaders map pixel coordinates to
	// clip space through vertex constant register 9, which the shipped shader assets have baked
	// in. Without this every piece of UI transforms from a zeroed register and lands nowhere.
	Direct3d11_ConstantBuffers::setViewportData(x, y, width, height);
}

// ----------------------------------------------------------------------
/**
 * Release what can be released.
 *
 * D3D9's non-reset path calls EvictManagedResources, which has no D3D11
 * equivalent: there is no managed pool and the runtime handles residency itself.
 * The full-reset path recreates the buffers, which is what the engine is asking
 * for when it wants everything rebuilt.
 */

void Direct3d11_SwapChain::flushResources(bool fullReset)
{
	DX11_ASSERT_MAIN_THREAD();

	if (!fullReset)
		return;

	int const width = ms_width;
	int const height = ms_height;

	// Force the rebuild through the resize path by invalidating the cached size.
	ms_width = 0;
	resize(width, height);
}

// ======================================================================
