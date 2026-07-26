// ======================================================================
//
// Direct3d11_RenderTarget.cpp
// copyright (c) 2026 Galaxies Reborn
//
// ======================================================================

#include "FirstDirect3d11.h"
#include "Direct3d11_RenderTarget.h"

#include "Direct3d11_Device.h"
#include "Direct3d11_Metrics.h"
#include "Direct3d11_SceneTarget.h"
#include "Direct3d11_StateCache.h"
#include "Direct3d11_TextureData.h"
#include "clientGraphics/Texture.h"

#include <map>

// ======================================================================

namespace Direct3d11_RenderTargetNamespace
{
	// A view is identified by the resource it looks at and the subresource within it, which is
	// what makes one cache serve flat textures, mip levels and cube faces alike.
	struct ViewKey
	{
		ID3D11Resource *resource;
		int             subresource;

		bool operator <(ViewKey const &rhs) const
		{
			if (resource != rhs.resource)
				return resource < rhs.resource;
			return subresource < rhs.subresource;
		}
	};

	struct DepthBuffer
	{
		ID3D11Texture2D        *texture;
		ID3D11DepthStencilView *view;
	};

	struct SizeKey
	{
		int width;
		int height;

		bool operator <(SizeKey const &rhs) const
		{
			if (width != rhs.width)
				return width < rhs.width;
			return height < rhs.height;
		}
	};

	typedef std::map<ViewKey, ID3D11RenderTargetView *> ViewCache;
	typedef std::map<SizeKey, DepthBuffer>              DepthCache;

	bool                    ms_installed;

	ViewCache               ms_viewCache;
	DepthCache              ms_depthCache;

	// What is bound. Null colour means the scene target, which is the resting state.
	ID3D11RenderTargetView *ms_currentColorView;
	ID3D11DepthStencilView *ms_currentDepthView;
	int                     ms_currentWidth;
	int                     ms_currentHeight;
	bool                    ms_primaryTargetSet;

	// The scratch target the baked path renders into, and where its result has to go.
	ID3D11Texture2D        *ms_bakeTexture;
	ID3D11RenderTargetView *ms_bakeView;
	int                     ms_bakeWidth;
	int                     ms_bakeHeight;
	DXGI_FORMAT             ms_bakeFormat;

	ID3D11Resource         *ms_bakeDestination;
	int                     ms_bakeDestinationSubresource;
	int                     ms_bakeCopyWidth;
	int                     ms_bakeCopyHeight;

	bool                    ms_reportedCompressedBake;

	ID3D11RenderTargetView *acquireRenderTargetView(ID3D11Resource *resource, DXGI_FORMAT format, CubeFace cubeFace, int mipmapLevel, int subresource);
	ID3D11DepthStencilView *acquireDepthStencilView(int width, int height);
	bool                    ensureBakeTarget(DXGI_FORMAT format, int width, int height);
	void                    releaseBakeTarget();
	bool                    canBeRenderTarget(DXGI_FORMAT format);
	void                    bind(ID3D11RenderTargetView *colorView, ID3D11DepthStencilView *depthView, int width, int height);
	bool                    describeTexture(ID3D11Resource *resource, D3D11_TEXTURE2D_DESC &description);
}

using namespace Direct3d11_RenderTargetNamespace;

// ======================================================================

void Direct3d11_RenderTarget::install()
{
	DEBUG_FATAL(ms_installed, ("Direct3d11_RenderTarget already installed"));

	ms_installed = true;
	ms_bakeFormat = DXGI_FORMAT_UNKNOWN;

	setRenderTargetToPrimary();
}

// ----------------------------------------------------------------------

void Direct3d11_RenderTarget::remove()
{
	if (!ms_installed)
		return;

	for (ViewCache::iterator i = ms_viewCache.begin(); i != ms_viewCache.end(); ++i)
		if (i->second)
			i->second->Release();
	ms_viewCache.clear();

	for (DepthCache::iterator i = ms_depthCache.begin(); i != ms_depthCache.end(); ++i)
	{
		if (i->second.view)
			i->second.view->Release();
		if (i->second.texture)
			i->second.texture->Release();
	}
	ms_depthCache.clear();

	releaseBakeTarget();

	ms_currentColorView = NULL;
	ms_currentDepthView = NULL;
	ms_installed = false;
}

// ======================================================================

bool Direct3d11_RenderTargetNamespace::describeTexture(ID3D11Resource *resource, D3D11_TEXTURE2D_DESC &description)
{
	if (!resource)
		return false;

	ID3D11Texture2D *texture2d = NULL;
	if (FAILED(resource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&texture2d))) || !texture2d)
		return false;

	texture2d->GetDesc(&description);
	texture2d->Release();
	return true;
}

// ----------------------------------------------------------------------

bool Direct3d11_RenderTargetNamespace::canBeRenderTarget(DXGI_FORMAT format)
{
	ID3D11Device1 * const device = Direct3d11_Device::getDevice();
	if (!device || format == DXGI_FORMAT_UNKNOWN)
		return false;

	UINT support = 0;
	if (FAILED(device->CheckFormatSupport(format, &support)))
		return false;

	return (support & D3D11_FORMAT_SUPPORT_RENDER_TARGET) != 0;
}

// ----------------------------------------------------------------------
/**
 * A render target view onto one subresource, created once and kept.
 *
 * The view's dimension follows the texture: a cube face is a slice of a 2D array as far as
 * D3D11 is concerned, and CubeFace's order is D3D's own (+X, -X, +Y, -Y, +Z, -Z), so the
 * enumerator doubles as the array slice.
 */

ID3D11RenderTargetView *Direct3d11_RenderTargetNamespace::acquireRenderTargetView(ID3D11Resource *resource, DXGI_FORMAT format, CubeFace cubeFace, int mipmapLevel, int subresource)
{
	ViewKey key;
	key.resource = resource;
	key.subresource = subresource;

	ViewCache::iterator const found = ms_viewCache.find(key);
	if (found != ms_viewCache.end())
		return found->second;

	ID3D11Device1 * const device = Direct3d11_Device::getDevice();
	if (!device)
		return NULL;

	D3D11_RENDER_TARGET_VIEW_DESC description;
	Zero(description);
	description.Format = format;

	if (cubeFace == CF_none)
	{
		description.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		description.Texture2D.MipSlice = static_cast<UINT>(mipmapLevel);
	}
	else
	{
		description.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		description.Texture2DArray.MipSlice        = static_cast<UINT>(mipmapLevel);
		description.Texture2DArray.FirstArraySlice = static_cast<UINT>(cubeFace);
		description.Texture2DArray.ArraySize       = 1;
	}

	ID3D11RenderTargetView *view = NULL;
	HRESULT const hresult = device->CreateRenderTargetView(resource, &description, &view);

	if (FAILED(hresult) || !view)
	{
		WARNING(true, ("Direct3d11: a render target view for mip %d face %d could not be created (%s).", mipmapLevel, static_cast<int>(cubeFace), Direct3d11_Device::describeHresult(hresult)));
		return NULL;
	}

	ms_viewCache[key] = view;
	return view;
}

// ----------------------------------------------------------------------
/**
 * A depth-stencil buffer matching a target's dimensions.
 *
 * Only reached for a render target whose size differs from the scene's, because D3D11 requires
 * the depth and colour extents to agree where D3D9 did not. Cached by size: the handful of
 * off-screen sizes the engine uses are stable for the life of the run.
 *
 * The format follows the scene target's, which picked it by probing at install, so these cannot
 * disagree with it.
 */

ID3D11DepthStencilView *Direct3d11_RenderTargetNamespace::acquireDepthStencilView(int width, int height)
{
	SizeKey key;
	key.width = width;
	key.height = height;

	DepthCache::iterator const found = ms_depthCache.find(key);
	if (found != ms_depthCache.end())
		return found->second.view;

	ID3D11Device1 * const device = Direct3d11_Device::getDevice();
	if (!device)
		return NULL;

	D3D11_TEXTURE2D_DESC description;
	Zero(description);
	description.Width            = static_cast<UINT>(width);
	description.Height           = static_cast<UINT>(height);
	description.MipLevels        = 1;
	description.ArraySize        = 1;
	description.Format           = Direct3d11_SceneTarget::getDepthFormat();
	description.SampleDesc.Count = 1;
	description.Usage            = D3D11_USAGE_DEFAULT;
	description.BindFlags        = D3D11_BIND_DEPTH_STENCIL;

	DepthBuffer buffer;
	buffer.texture = NULL;
	buffer.view = NULL;

	HRESULT hresult = device->CreateTexture2D(&description, NULL, &buffer.texture);
	if (FAILED(hresult) || !buffer.texture)
	{
		WARNING(true, ("Direct3d11: a %dx%d depth buffer for an off-screen render target could not be created (%s). That target will render without depth.", width, height, Direct3d11_Device::describeHresult(hresult)));
		return NULL;
	}

	hresult = device->CreateDepthStencilView(buffer.texture, NULL, &buffer.view);
	if (FAILED(hresult) || !buffer.view)
	{
		WARNING(true, ("Direct3d11: a depth-stencil view for a %dx%d off-screen render target could not be created (%s).", width, height, Direct3d11_Device::describeHresult(hresult)));
		buffer.texture->Release();
		return NULL;
	}

	++Direct3d11_Metrics::renderTargetSwitches;
	ms_depthCache[key] = buffer;
	return buffer.view;
}

// ----------------------------------------------------------------------

bool Direct3d11_RenderTargetNamespace::ensureBakeTarget(DXGI_FORMAT format, int width, int height)
{
	if (ms_bakeTexture && ms_bakeFormat == format && ms_bakeWidth >= width && ms_bakeHeight >= height)
		return true;

	releaseBakeTarget();

	ID3D11Device1 * const device = Direct3d11_Device::getDevice();
	if (!device)
		return false;

	D3D11_TEXTURE2D_DESC description;
	Zero(description);
	description.Width            = static_cast<UINT>(width);
	description.Height           = static_cast<UINT>(height);
	description.MipLevels        = 1;
	description.ArraySize        = 1;
	description.Format           = format;
	description.SampleDesc.Count = 1;
	description.Usage            = D3D11_USAGE_DEFAULT;
	description.BindFlags        = D3D11_BIND_RENDER_TARGET;

	HRESULT hresult = device->CreateTexture2D(&description, NULL, &ms_bakeTexture);
	if (FAILED(hresult) || !ms_bakeTexture)
	{
		WARNING(true, ("Direct3d11: the %dx%d scratch target for baking a texture could not be created (%s).", width, height, Direct3d11_Device::describeHresult(hresult)));
		ms_bakeTexture = NULL;
		return false;
	}

	hresult = device->CreateRenderTargetView(ms_bakeTexture, NULL, &ms_bakeView);
	if (FAILED(hresult) || !ms_bakeView)
	{
		WARNING(true, ("Direct3d11: a render target view for the texture bake scratch could not be created (%s).", Direct3d11_Device::describeHresult(hresult)));
		releaseBakeTarget();
		return false;
	}

	ms_bakeWidth = width;
	ms_bakeHeight = height;
	ms_bakeFormat = format;
	return true;
}

// ----------------------------------------------------------------------

void Direct3d11_RenderTargetNamespace::releaseBakeTarget()
{
	if (ms_bakeView)
	{
		ms_bakeView->Release();
		ms_bakeView = NULL;
	}

	if (ms_bakeTexture)
	{
		ms_bakeTexture->Release();
		ms_bakeTexture = NULL;
	}

	ms_bakeWidth = 0;
	ms_bakeHeight = 0;
	ms_bakeFormat = DXGI_FORMAT_UNKNOWN;
}

// ----------------------------------------------------------------------
/**
 * Bind, and set the viewport to the whole target.
 *
 * The viewport is not incidental. D3D9's SetRenderTarget reset it as a side effect and the
 * engine's callers rely on that -- Bloom binds a quarter-size buffer and draws a full-screen
 * quad without touching the viewport. Leaving D3D11's viewport at the previous target's size
 * would clip that quad to a corner.
 */

void Direct3d11_RenderTargetNamespace::bind(ID3D11RenderTargetView *colorView, ID3D11DepthStencilView *depthView, int width, int height)
{
	ID3D11DeviceContext1 * const context = Direct3d11_Device::getContext();
	if (!context)
		return;

	context->OMSetRenderTargets(1, &colorView, depthView);

	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width    = static_cast<float>(width);
	viewport.Height   = static_cast<float>(height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	context->RSSetViewports(1, &viewport);

	ms_currentColorView = colorView;
	ms_currentDepthView = depthView;
	ms_currentWidth = width;
	ms_currentHeight = height;

	++Direct3d11_Metrics::renderTargetSwitches;
}

// ======================================================================

void Direct3d11_RenderTarget::setRenderTargetToPrimary()
{
	if (ms_primaryTargetSet && ms_currentColorView == Direct3d11_SceneTarget::getRenderTargetView())
		return;

	bind(
		Direct3d11_SceneTarget::getRenderTargetView(),
		Direct3d11_SceneTarget::getDepthStencilView(),
		Direct3d11_SceneTarget::getWidth(),
		Direct3d11_SceneTarget::getHeight());

	ms_primaryTargetSet = true;
}

// ----------------------------------------------------------------------

void Direct3d11_RenderTarget::setRenderTarget(Texture *texture, CubeFace cubeFace, int mipmapLevel)
{
	if (!texture)
	{
		setRenderTargetToPrimary();
		return;
	}

	Direct3d11_TextureData * const data = static_cast<Direct3d11_TextureData *>(texture->m_graphicsData);
	if (!data)
	{
		WARNING(true, ("Direct3d11: setRenderTarget was given a texture with no graphics data; staying on the scene target."));
		return;
	}

	ID3D11Resource * const resource = data->getResource();
	D3D11_TEXTURE2D_DESC description;
	if (!resource || !describeTexture(resource, description))
	{
		WARNING(true, ("Direct3d11: setRenderTarget was given a texture that is not a 2D resource; staying on the scene target."));
		return;
	}

	ms_primaryTargetSet = false;

	// A resource cannot be a render target and a shader resource at once. D3D11 unbinds the
	// shader resource silently, which would leave this backend's shadow believing the texture
	// is still bound -- and a later bind of that same view would then be skipped as redundant,
	// sampling whatever the context actually has. Bloom hits this every frame: it samples the
	// buffer it wrote last pass and writes the buffer it sampled the pass before.
	Direct3d11_StateCache::unbindShaderResourcesForResource(resource);

	// The mip's own extent, which is what both paths need: the render target view covers the
	// mip, and the copy is sized by it.
	int const levelWidth  = std::max(1, static_cast<int>(description.Width)  >> mipmapLevel);
	int const levelHeight = std::max(1, static_cast<int>(description.Height) >> mipmapLevel);

	int const arraySlice  = (cubeFace == CF_none) ? 0 : static_cast<int>(cubeFace);
	int const subresource = static_cast<int>(D3D11CalcSubresource(static_cast<UINT>(mipmapLevel), static_cast<UINT>(arraySlice), description.MipLevels));

	// ------------------------------------------------------------------
	// The direct path: the texture can be a render target, so render into it.

	if ((description.BindFlags & D3D11_BIND_RENDER_TARGET) != 0)
	{
		ID3D11RenderTargetView * const view = acquireRenderTargetView(resource, description.Format, cubeFace, mipmapLevel, subresource);
		if (!view)
			return;

		// The scene's own depth when it is compatible, which is the exact-parity case and the
		// only one where the scene's depth contents mean anything; a private buffer otherwise.
		//
		// Compatible means the extents AND the sample count. A texture render target is always
		// single-sampled, so with multisampling on, the scene's depth cannot be paired with it
		// even at the same size -- D3D11 requires the sample counts to match, and this is the
		// case that would otherwise appear only when a user turned antialiasing on.
		ID3D11DepthStencilView *depthView = NULL;
		if (levelWidth == Direct3d11_SceneTarget::getWidth() &&
			levelHeight == Direct3d11_SceneTarget::getHeight() &&
			Direct3d11_SceneTarget::getSampleCount() == 1)
			depthView = Direct3d11_SceneTarget::getDepthStencilView();
		else
			depthView = acquireDepthStencilView(levelWidth, levelHeight);

		bind(view, depthView, levelWidth, levelHeight);

		ms_bakeDestination = NULL;
		return;
	}

	// ------------------------------------------------------------------
	// The baked path: render into a scratch target of the same format and copy afterwards.
	//
	// Same format, not a fixed one, so the copy is a straight subresource copy rather than a
	// conversion. D3D9 had to convert because its scratch was always X8R8G8B8.

	if (!canBeRenderTarget(description.Format))
	{
		if (!ms_reportedCompressedBake)
		{
			ms_reportedCompressedBake = true;
			WARNING(true, ("Direct3d11: a texture with DXGI format %d is being baked, but that format cannot be a render target -- almost certainly a block-compressed one. D3D9 handled this by compressing on the CPU inside D3DXLoadSurfaceFromSurface; D3D11 has no equivalent, so this bake is skipped and the texture will be left as it was. Fixing it means a block compressor: either a compute-shader one, or a readback through Direct3d11_TextureConverter at the cost of a pipeline stall per mip. Reported once.", static_cast<int>(description.Format)));
		}

		ms_bakeDestination = NULL;
		return;
	}

	if (!ensureBakeTarget(description.Format, levelWidth, levelHeight))
	{
		ms_bakeDestination = NULL;
		return;
	}

	// No depth, matching DX9, which sets its depth-stencil surface to NULL for exactly this
	// case. A baked texture is composited from 2D commands and has nothing to depth-test.
	bind(ms_bakeView, NULL, levelWidth, levelHeight);

	ms_bakeDestination            = resource;
	ms_bakeDestinationSubresource = subresource;
	ms_bakeCopyWidth              = levelWidth;
	ms_bakeCopyHeight             = levelHeight;
}

// ----------------------------------------------------------------------
/**
 * Move the finished bake into its destination.
 *
 * A single GPU copy. The DX9 shape this replaces is a GetRenderTargetData readback followed by
 * a D3DXLoadSurfaceFromSurface, both of which the CPU has to wait on.
 *
 * Returns whether the copy happened. DX9 returns false only on a lost device, which is what
 * the caller (the texture baker's per-mip loop) treats as "stop and try again later".
 */

bool Direct3d11_RenderTarget::copyRenderTargetToNonRenderTargetTexture()
{
	if (!ms_bakeDestination)
	{
		// Either the bake was refused above, or this was called without a matching
		// setRenderTarget. Neither is worth a fatal -- the caller's loop just moves on -- but
		// the target has to come back either way.
		setRenderTargetToPrimary();
		return false;
	}

	ID3D11DeviceContext1 * const context = Direct3d11_Device::getContext();
	if (!context)
	{
		ms_bakeDestination = NULL;
		return false;
	}

	D3D11_BOX box;
	box.left   = 0;
	box.top    = 0;
	box.front  = 0;
	box.right  = static_cast<UINT>(ms_bakeCopyWidth);
	box.bottom = static_cast<UINT>(ms_bakeCopyHeight);
	box.back   = 1;

	context->CopySubresourceRegion(
		ms_bakeDestination,
		static_cast<UINT>(ms_bakeDestinationSubresource),
		0, 0, 0,
		ms_bakeTexture,
		0,
		&box);

	ms_bakeDestination = NULL;

	// DX9 returns to the primary here rather than leaving it to the caller, and the caller
	// relies on it: the baker's loop calls setRenderTarget(NULL) afterwards, which would
	// otherwise be the only thing restoring it and would not run on the failure path.
	setRenderTargetToPrimary();

	return true;
}

// ======================================================================

void Direct3d11_RenderTarget::bindCurrent()
{
	// The resting state is the scene target, and nothing has been bound yet at the first
	// beginScene of the run.
	if (!ms_currentColorView)
	{
		setRenderTargetToPrimary();
		return;
	}

	bind(ms_currentColorView, ms_currentDepthView, ms_currentWidth, ms_currentHeight);
}

// ----------------------------------------------------------------------

void Direct3d11_RenderTarget::sceneTargetRebuilt()
{
	// The views are the scene target's own, not cached here, so there is nothing to release --
	// but ms_currentColorView may still hold a freed one, and setRenderTargetToPrimary skips
	// the bind when it thinks the primary is already current. Clearing first forces the rebind.
	ms_currentColorView = NULL;
	ms_currentDepthView = NULL;
	ms_primaryTargetSet = false;

	// The private depth buffers are keyed by size and their format follows the scene's, so a
	// resize leaves stale sizes cached and a format change leaves them wrong. Dropped
	// wholesale; they are rebuilt on demand and there are only a handful.
	for (DepthCache::iterator i = ms_depthCache.begin(); i != ms_depthCache.end(); ++i)
	{
		if (i->second.view)
			i->second.view->Release();
		if (i->second.texture)
			i->second.texture->Release();
	}
	ms_depthCache.clear();

	setRenderTargetToPrimary();
}

// ----------------------------------------------------------------------

void Direct3d11_RenderTarget::releaseViewsFor(ID3D11Resource *resource)
{
	if (!resource || ms_viewCache.empty())
		return;

	for (ViewCache::iterator i = ms_viewCache.begin(); i != ms_viewCache.end(); )
	{
		if (i->first.resource != resource)
		{
			++i;
			continue;
		}

		if (i->second == ms_currentColorView)
		{
			// The target being destroyed is the one bound. Fall back to the scene target rather
			// than leave a freed view in the context.
			ms_currentColorView = NULL;
			ms_currentDepthView = NULL;
			setRenderTargetToPrimary();
		}

		if (i->second)
			i->second->Release();

		ms_viewCache.erase(i++);
	}

	if (ms_bakeDestination == resource)
		ms_bakeDestination = NULL;
}

// ======================================================================

ID3D11RenderTargetView *Direct3d11_RenderTarget::getCurrentRenderTargetView()
{
	return ms_currentColorView;
}

// ----------------------------------------------------------------------

ID3D11DepthStencilView *Direct3d11_RenderTarget::getCurrentDepthStencilView()
{
	return ms_currentDepthView;
}

// ----------------------------------------------------------------------

int Direct3d11_RenderTarget::getCurrentWidth()
{
	return ms_currentWidth;
}

// ----------------------------------------------------------------------

int Direct3d11_RenderTarget::getCurrentHeight()
{
	return ms_currentHeight;
}

// ======================================================================
