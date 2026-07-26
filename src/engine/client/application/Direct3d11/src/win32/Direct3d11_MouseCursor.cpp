// ======================================================================
//
// Direct3d11_MouseCursor.cpp
// copyright (c) 2026 Galaxies Reborn
//
// ======================================================================

#include "FirstDirect3d11.h"
#include "Direct3d11_MouseCursor.h"

#include "Direct3d11_Device.h"
#include "Direct3d11_Metrics.h"
#include "Direct3d11_SwapChain.h"
#include "Direct3d11_TextureData.h"
#include "clientGraphics/Texture.h"

#include <map>

// ======================================================================

namespace Direct3d11_MouseCursorNamespace
{
	// A texture can be used with more than one hot spot, so the hot spot is part of the key.
	struct CursorKey
	{
		Texture const *texture;
		int            hotSpotX;
		int            hotSpotY;

		bool operator <(CursorKey const &rhs) const
		{
			if (texture != rhs.texture)
				return texture < rhs.texture;
			if (hotSpotX != rhs.hotSpotX)
				return hotSpotX < rhs.hotSpotX;
			return hotSpotY < rhs.hotSpotY;
		}
	};

	typedef std::map<CursorKey, HCURSOR> CursorCache;

	CursorCache ms_cursors;
	HCURSOR     ms_current;
	bool        ms_reportedFormat;

	HCURSOR build(Texture const &texture, int hotSpotX, int hotSpotY);
	bool    readTexture(Texture const &texture, int &width, int &height, std::vector<uint8> &bgra);
}

using namespace Direct3d11_MouseCursorNamespace;

// ======================================================================
/**
 * Copy a texture's top mip level to the CPU as 32-bit BGRA.
 *
 * Only the two 8-8-8-8 orders are handled. Every cursor in the shipped interface is an ARGB
 * texture, and a compressed or reduced-precision cursor would look wrong regardless -- so an
 * unexpected format is reported once and refused rather than approximated.
 */

bool Direct3d11_MouseCursorNamespace::readTexture(Texture const &texture, int &width, int &height, std::vector<uint8> &bgra)
{
	Direct3d11_TextureData const * const data = static_cast<Direct3d11_TextureData const *>(texture.getGraphicsData());
	if (!data)
		return false;

	ID3D11Resource * const resource = data->getResource();
	ID3D11Device1 * const device = Direct3d11_Device::getDevice();
	ID3D11DeviceContext1 * const context = Direct3d11_Device::getContext();

	if (!resource || !device || !context)
		return false;

	ID3D11Texture2D *source = NULL;
	if (FAILED(resource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&source))) || !source)
		return false;

	D3D11_TEXTURE2D_DESC description;
	source->GetDesc(&description);

	bool swapRedAndBlue = false;
	switch (description.Format)
	{
		case DXGI_FORMAT_B8G8R8A8_UNORM:
		case DXGI_FORMAT_B8G8R8X8_UNORM:
			swapRedAndBlue = false;
			break;

		case DXGI_FORMAT_R8G8B8A8_UNORM:
			swapRedAndBlue = true;
			break;

		default:
			if (!ms_reportedFormat)
			{
				ms_reportedFormat = true;
				WARNING(true, ("Direct3d11: the mouse cursor texture is DXGI format %d, which this backend does not read back for a cursor, so the interface will draw its own. Reported once.", static_cast<int>(description.Format)));
			}
			source->Release();
			return false;
	}

	// A staging copy of the whole top level. The cursor texture is small -- 32x32 or 64x64 --
	// so this is a trivial allocation and it happens once per cursor.
	D3D11_TEXTURE2D_DESC staging = description;
	staging.MipLevels      = 1;
	staging.ArraySize      = 1;
	staging.Usage          = D3D11_USAGE_STAGING;
	staging.BindFlags      = 0;
	staging.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	staging.MiscFlags      = 0;

	ID3D11Texture2D *readback = NULL;
	HRESULT hresult = device->CreateTexture2D(&staging, NULL, &readback);
	if (FAILED(hresult) || !readback)
	{
		WARNING(true, ("Direct3d11: a staging copy for the mouse cursor could not be created (%s).", Direct3d11_Device::describeHresult(hresult)));
		source->Release();
		return false;
	}

	context->CopySubresourceRegion(readback, 0, 0, 0, 0, source, 0, NULL);

	D3D11_MAPPED_SUBRESOURCE mapped;
	Zero(mapped);

	// Blocking, and counted. Bounded by the number of distinct cursors, not by frames.
	++Direct3d11_Metrics::blockingStagingMaps;

	hresult = context->Map(readback, 0, D3D11_MAP_READ, 0, &mapped);
	if (FAILED(hresult))
	{
		WARNING(true, ("Direct3d11: the mouse cursor staging copy could not be mapped (%s).", Direct3d11_Device::describeHresult(hresult)));
		readback->Release();
		source->Release();
		return false;
	}

	width  = static_cast<int>(description.Width);
	height = static_cast<int>(description.Height);

	bgra.resize(static_cast<size_t>(width) * static_cast<size_t>(height) * 4);

	for (int y = 0; y < height; ++y)
	{
		uint8 const * const row = static_cast<uint8 const *>(mapped.pData) + (static_cast<size_t>(y) * mapped.RowPitch);
		uint8 * const out = &bgra[static_cast<size_t>(y) * static_cast<size_t>(width) * 4];

		for (int x = 0; x < width; ++x)
		{
			uint8 const * const pixel = row + (x * 4);
			uint8 * const target = out + (x * 4);

			if (swapRedAndBlue)
			{
				target[0] = pixel[2];
				target[1] = pixel[1];
				target[2] = pixel[0];
			}
			else
			{
				target[0] = pixel[0];
				target[1] = pixel[1];
				target[2] = pixel[2];
			}

			target[3] = pixel[3];
		}
	}

	context->Unmap(readback, 0);
	readback->Release();
	source->Release();
	return true;
}

// ======================================================================
/**
 * Turn a texture into an HCURSOR.
 *
 * A 32-bit top-down DIB for the colour, and a monochrome bitmap for the mask. The mask is
 * required by ICONINFO even for an alpha cursor and is left entirely zero: with a 32-bit colour
 * bitmap Windows composites from the alpha channel and uses the mask only for the 1-bit
 * fallback path, where all-zero means "draw the whole rectangle".
 */

HCURSOR Direct3d11_MouseCursorNamespace::build(Texture const &texture, int hotSpotX, int hotSpotY)
{
	int width = 0;
	int height = 0;
	std::vector<uint8> bgra;

	if (!readTexture(texture, width, height, bgra) || width <= 0 || height <= 0)
		return NULL;

	BITMAPV5HEADER header;
	Zero(header);
	header.bV5Size        = sizeof(BITMAPV5HEADER);
	header.bV5Width       = width;
	header.bV5Height      = -height;          // negative: top-down, matching the texture's rows
	header.bV5Planes      = 1;
	header.bV5BitCount    = 32;
	header.bV5Compression = BI_BITFIELDS;
	header.bV5RedMask     = 0x00ff0000;
	header.bV5GreenMask   = 0x0000ff00;
	header.bV5BlueMask    = 0x000000ff;
	header.bV5AlphaMask   = 0xff000000;

	HDC const screen = GetDC(NULL);
	if (!screen)
		return NULL;

	void *bits = NULL;
	HBITMAP const colour = CreateDIBSection(screen, reinterpret_cast<BITMAPINFO const *>(&header), DIB_RGB_COLORS, &bits, NULL, 0);
	ReleaseDC(NULL, screen);

	if (!colour || !bits)
	{
		if (colour)
			DeleteObject(colour);
		return NULL;
	}

	memcpy(bits, &bgra[0], bgra.size());

	HBITMAP const mask = CreateBitmap(width, height, 1, 1, NULL);
	if (!mask)
	{
		DeleteObject(colour);
		return NULL;
	}

	ICONINFO info;
	Zero(info);
	info.fIcon    = FALSE;                    // FALSE means cursor, and enables the hot spot
	info.xHotspot = static_cast<DWORD>(hotSpotX);
	info.yHotspot = static_cast<DWORD>(hotSpotY);
	info.hbmMask  = mask;
	info.hbmColor = colour;

	HCURSOR const cursor = reinterpret_cast<HCURSOR>(CreateIconIndirect(&info));

	// CreateIconIndirect copies both bitmaps, so they are ours to delete either way.
	DeleteObject(mask);
	DeleteObject(colour);

	if (!cursor)
		WARNING(true, ("Direct3d11: CreateIconIndirect failed for a %dx%d mouse cursor; the interface will draw its own.", width, height));

	return cursor;
}

// ======================================================================

bool Direct3d11_MouseCursor::set(Texture const &texture, int hotSpotX, int hotSpotY)
{
	HWND const window = Direct3d11_SwapChain::getWindow();
	if (!window)
		return false;

	// DX9 refuses when its window is not the foreground one, and so does this: setting the
	// cursor for a window that does not have the pointer would change it for the desktop.
	if (GetForegroundWindow() != window)
		return false;

	CursorKey key;
	key.texture  = &texture;
	key.hotSpotX = hotSpotX;
	key.hotSpotY = hotSpotY;

	HCURSOR cursor = NULL;

	CursorCache::iterator const found = ms_cursors.find(key);
	if (found != ms_cursors.end())
	{
		cursor = found->second;

		// A remembered failure. Kept in the cache deliberately: without it, a texture that
		// cannot be read would be read again on every frame the interface set it.
		if (!cursor)
			return false;
	}
	else
	{
		cursor = build(texture, hotSpotX, hotSpotY);
		ms_cursors[key] = cursor;

		if (!cursor)
			return false;
	}

	if (cursor == ms_current)
		return true;

	ms_current = cursor;

	// Both calls are needed. SetCursor changes it now; the class cursor is what Windows restores
	// on the next WM_SETCURSOR, which arrives on the very next mouse move -- so without it the
	// cursor reverts almost immediately.
	IGNORE_RETURN(SetClassLongPtr(window, GCLP_HCURSOR, reinterpret_cast<LONG_PTR>(cursor)));
	IGNORE_RETURN(SetCursor(cursor));

	return true;
}

// ----------------------------------------------------------------------

void Direct3d11_MouseCursor::remove()
{
	// The class cursor is dropped first: destroying a cursor the window class still points at
	// would leave Windows holding a freed handle.
	HWND const window = Direct3d11_SwapChain::getWindow();
	if (window && ms_current)
		IGNORE_RETURN(SetClassLongPtr(window, GCLP_HCURSOR, 0));

	for (CursorCache::iterator i = ms_cursors.begin(); i != ms_cursors.end(); ++i)
		if (i->second)
			IGNORE_RETURN(DestroyIcon(reinterpret_cast<HICON>(i->second)));

	ms_cursors.clear();
	ms_current = NULL;
}

// ======================================================================
