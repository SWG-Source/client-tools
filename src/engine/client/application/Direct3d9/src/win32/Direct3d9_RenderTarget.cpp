// ======================================================================
//
// Direct3d9_RenderTarget.cpp
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstDirect3d9.h"
#include "Direct3d9_RenderTarget.h"

#include "Direct3d9.h"
#include "Direct3d9_TextureData.h"
#include "sharedFoundation/LessPointerComparator.h"
#include "sharedFoundation/PointerDeleter.h"
#include "clientGraphics/Texture.h"

#include <d3dx9.h>
#include <algorithm>
#include <map>

// ======================================================================

namespace Direct3d9_RenderTargetNamespace
{
	int const              cms_bakedTextureMaxDimension = 512;

	bool                   ms_installed;
	bool                   ms_primaryTargetSet;

	IDirect3DSurface9     *ms_primarySurface;
	IDirect3DSurface9     *ms_primaryDepthStencilSurface;

	IDirect3DTexture9     *ms_systemMemoryTexture;
	IDirect3DSurface9     *ms_systemMemorySurface;

	IDirect3DTexture9     *ms_renderTargetTexture;
	IDirect3DSurface9     *ms_renderSurface;

	IDirect3DSurface9     *ms_userSurface;

	int                    ms_copyWidth;
	int                    ms_copyHeight;

}
using namespace Direct3d9_RenderTargetNamespace;

// ======================================================================

void Direct3d9_RenderTarget::install()
{
	DEBUG_FATAL(ms_installed, ("Direct3d9_RenderTarget already installed"));

	ms_installed = true;
	restoreDevice();
}

// ----------------------------------------------------------------------

void Direct3d9_RenderTarget::remove()
{
	if (ms_installed)
	{
		lostDevice();
		ms_installed = false;
	}
}

// ----------------------------------------------------------------------

void Direct3d9_RenderTarget::restoreDevice()
{
	HRESULT hresult;

	IDirect3DDevice9 *device = NON_NULL(Direct3d9::getDevice());

	ms_primaryTargetSet = true;

	hresult = device->GetRenderTarget(0, &ms_primarySurface);
	FATAL_DX_HR("GetRenderTarget() failed %s", hresult);

	hresult = device->GetDepthStencilSurface(&ms_primaryDepthStencilSurface);
	FATAL_DX_HR("GetDepthStencilSurface() failed %s", hresult);

	{
		HRESULT hresult = device->EvictManagedResources();
		FATAL_DX_HR("EvictManagedResources failed %s", hresult);
	}

	if (!ms_renderTargetTexture) 
	{
		HRESULT hresult = device->CreateTexture(cms_bakedTextureMaxDimension, cms_bakedTextureMaxDimension, 0, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &ms_renderTargetTexture, NULL);
		FATAL_DX_HR("CreateTexture failed for render target texture %s", hresult);
		FATAL(ms_renderTargetTexture == NULL, ("RenderTarget renderTargetTexture is null"));
	}

	if (!ms_systemMemoryTexture) 
	{
		HRESULT hresult = device->CreateTexture(cms_bakedTextureMaxDimension, cms_bakedTextureMaxDimension, 0, 0, D3DFMT_X8R8G8B8, D3DPOOL_SYSTEMMEM, &ms_systemMemoryTexture, NULL);
		FATAL_DX_HR("CreateTexture failed for system memory texture %s", hresult);
		FATAL(ms_systemMemoryTexture == NULL, ("RenderTarget systemMemoryTexture is null"));
	}
}

// ----------------------------------------------------------------------

void Direct3d9_RenderTarget::lostDevice()
{
	DEBUG_FATAL(!ms_primaryTargetSet, ("Direct3d9_RenderTarget::lostDevice() designed to be called outside of beginTextureFrame/endTextureFrame pair"));

	ms_primaryTargetSet = true;

	if (ms_primarySurface)
	{
		IGNORE_RETURN(ms_primarySurface->Release());
		ms_primarySurface = NULL;
	}

	if (ms_primaryDepthStencilSurface)
	{
		IGNORE_RETURN(ms_primaryDepthStencilSurface->Release());
		ms_primaryDepthStencilSurface = NULL;
	}

	//-- these shouldn't be set, as that would indicate a lost device in between a beginTextureFrame() and endTextureFrame(),
	//   but let's be safe since Direct3DDevice9::Reset will fail if we have any outstanding D3DPOOL_DEFAULT textures
	//   not released.
	if (ms_userSurface)
	{
		IGNORE_RETURN(ms_userSurface->Release());
		ms_userSurface = 0;
	}

	// release the render target surface
	if (ms_renderSurface)
	{
		ms_renderSurface->Release();
		ms_renderSurface = NULL;
	}

	// release the system memory surface
	if (ms_systemMemorySurface)
	{
		ms_systemMemorySurface->Release();
		ms_systemMemorySurface = NULL;
	}

	// release the render target texture
	if (ms_renderTargetTexture)
	{
		ms_renderTargetTexture->Release();
		ms_renderTargetTexture = NULL;
	}

	// release the system memory texture
	if (ms_systemMemoryTexture)
	{
		ms_systemMemoryTexture->Release();
		ms_systemMemoryTexture = NULL;
	}
}

// ----------------------------------------------------------------------

void Direct3d9_RenderTarget::setRenderTargetToPrimary()
{
	DEBUG_FATAL(!ms_installed, ("Direct3d9_RenderTarget not installed"));
	NOT_NULL(ms_primarySurface);
	NOT_NULL(ms_primaryDepthStencilSurface);

	if (ms_primaryTargetSet)
		return;

	IDirect3DDevice9 *device = NON_NULL(Direct3d9::getDevice());

	HRESULT hresult = device->SetRenderTarget(0, ms_primarySurface);
	FATAL_DX_HR("SetRenderTarget failed %s", hresult);

	hresult = device->SetDepthStencilSurface(ms_primaryDepthStencilSurface);
	FATAL_DX_HR("SetDepthStencilSurface failed %s", hresult);

	//-- success
	ms_primaryTargetSet = true;
}

// ----------------------------------------------------------------------

void Direct3d9_RenderTarget::setRenderTarget(Texture *texture, CubeFace cubeFace, int mipmapLevel)
{
	NOT_NULL(ms_renderTargetTexture);
	NOT_NULL(ms_systemMemoryTexture);
	NOT_NULL(texture);

	//-- set the new surface as the render target
	HRESULT hresult;
	IDirect3DDevice9 * const device = NON_NULL(Direct3d9::getDevice());

	// save the primary surface
	ms_primaryTargetSet = false;

	if (!texture->isRenderTarget())
	{
		// keep track of user surface
		IDirect3DBaseTexture9 * d3dBaseTexture = safe_cast<Direct3d9_TextureData*>(texture->m_graphicsData)->getBaseTexture();
		if (cubeFace == CF_none)
		{
			hresult = static_cast<IDirect3DTexture9 *>(d3dBaseTexture)->GetSurfaceLevel(mipmapLevel, &ms_userSurface);
			FATAL_DX_HR("GetSurfaceLevel() failed for render d3dTexture %s", hresult);
		}
		else
		{
			hresult = static_cast<IDirect3DCubeTexture9 *>(d3dBaseTexture)->GetCubeMapSurface(Direct3d9::getD3dCubeFace(cubeFace), mipmapLevel, &ms_userSurface);
			FATAL_DX_HR("GetSurfaceLevel() failed for render d3dTexture %s", hresult);
		}
		NOT_NULL(ms_userSurface);

		// setup the copy rect (area within source which will be copied to dest)
		ms_copyWidth = std::max(1U, static_cast<uint>(texture->getWidth() >> mipmapLevel));
		ms_copyHeight = std::max(1U, static_cast<uint>(texture->getHeight() >> mipmapLevel));

		// select render target mip level based on maximum copy dimension
		// mip levels are used to reduce amount of data to be copied between render target and system texture
		// mip level is selected based on size of user texture.
		int mip = 0;
		{
			int copyDimension = ms_copyWidth > ms_copyHeight ? ms_copyWidth : ms_copyHeight;
			if (copyDimension > cms_bakedTextureMaxDimension)
			{
				// oversize base texture is not currently treated as fatal error.  later, in end texture frame, the baked texture is just ignored.
				DEBUG_WARNING(true, ("render target too big %d %d", ms_copyWidth, ms_copyHeight));
			}
			if (copyDimension > (cms_bakedTextureMaxDimension >> 1))
				mip = 0;
			else
				if (copyDimension > (cms_bakedTextureMaxDimension >> 2))
					mip = 1;
				else
					if (copyDimension > (cms_bakedTextureMaxDimension >> 3))
						mip = 2;
					else
						mip = 3;
		}

		// get the temporary system memory surface that will be used later in the call to GetRenderTargetData().
		//		this surface matches the dimensions and format of the render target surface
		hresult = ms_systemMemoryTexture->GetSurfaceLevel(mip, &ms_systemMemorySurface);
		FATAL_DX_HR("GetSurfaceLevel() failed for system memory surface %s", hresult);

		// get the render target surface
		hresult = ms_renderTargetTexture->GetSurfaceLevel(mip, &ms_renderSurface);
		FATAL_DX_HR("GetSurfaceLevel() failed for render target surface %s", hresult);
		NOT_NULL(ms_renderSurface);

		// set depth stencil surface to null
		hresult = device->SetDepthStencilSurface(NULL);
		FATAL_DX_HR("SetDepthStencilSurface() failed %s", hresult);
	}
	else
	{
		IDirect3DBaseTexture9 * d3dBaseTexture = safe_cast<Direct3d9_TextureData*>(texture->m_graphicsData)->getBaseTexture();

		if (cubeFace == CF_none)
		{
			hresult = static_cast<IDirect3DTexture9 *>(d3dBaseTexture)->GetSurfaceLevel(mipmapLevel, &ms_renderSurface);
			FATAL_DX_HR("GetSurfaceLevel() failed for render d3dTexture %s", hresult);
		}
		else
		{
			hresult = static_cast<IDirect3DCubeTexture9 *>(d3dBaseTexture)->GetCubeMapSurface(Direct3d9::getD3dCubeFace(cubeFace), mipmapLevel, &ms_renderSurface);
			FATAL_DX_HR("GetSurfaceLevel() failed for render d3dTexture %s", hresult);
		}
	}

	// set the render target
	hresult = device->SetRenderTarget(0, ms_renderSurface);
	FATAL_DX_HR("SetRenderTarget() failed %s", hresult);

	if (texture->isRenderTarget())
	{
		IGNORE_RETURN(ms_renderSurface->Release());
		ms_renderSurface = NULL;
	}
}

// ----------------------------------------------------------------------

bool Direct3d9_RenderTarget::copyRenderTargetToNonRenderTargetTexture()
{
	//-- if this ever occurs, the only thing I could think of is that the surface was lost between a
	//   beginTextureFrame() and endTextureFrame() call.
	DEBUG_FATAL(!ms_renderSurface, ("endTextureFrame() called with no matching beginTextureFrame()"));

	bool result = true;

	// if the user texture dimensions are within the size of the render target texture
	// copy the render target surface to the user surface
	if (ms_copyWidth <= cms_bakedTextureMaxDimension && ms_copyHeight <= cms_bakedTextureMaxDimension)
	{
		// get the device
		IDirect3DDevice9 *device = NON_NULL(Direct3d9::getDevice());

		// copy the render target data to the system memory surface
		HRESULT hresult = device->GetRenderTargetData(ms_renderSurface, ms_systemMemorySurface);
		if (hresult == D3DERR_DEVICELOST)
			result = false;
		else
		{
			FATAL_DX_HR("GetRenderTargetData failed %s", hresult);

			// get the copy size from upper left corner of the first pixel to the lower right corner of the last pixel
			RECT rMoveRect;
			rMoveRect.left = 0;
			rMoveRect.top = 0;
			rMoveRect.right = ms_copyWidth;
			rMoveRect.bottom = ms_copyHeight;

			// copy the system memory surface to the user surface
			hresult = D3DXLoadSurfaceFromSurface(ms_userSurface, NULL, &rMoveRect, ms_systemMemorySurface, NULL, &rMoveRect, D3DX_FILTER_NONE, 0);
			FATAL_DX_HR("D3DXLoadSurfaceFromSurface failed %s", hresult);
		}
	}
	
	//-- release references to user and render surface
	IGNORE_RETURN(ms_userSurface->Release());
	ms_userSurface = 0;

	IGNORE_RETURN(ms_systemMemorySurface->Release());
	ms_systemMemorySurface = 0;

	IGNORE_RETURN(ms_renderSurface->Release());
	ms_renderSurface = 0;

	//-- set the render target to the primary surface
	// -TRF- this probably could be lazily set within Direct3d9::beginScene() so
	//       that multiple texture rendering calls don't force flip-flopping between
	//       texture 1, primary surface, texture 2, primary surface, etc.
	setRenderTargetToPrimary();

	return result;
}

// ======================================================================
