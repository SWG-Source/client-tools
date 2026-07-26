// ======================================================================
//
// Direct3d11_TextureFormatMap.cpp
// copyright (c) 2026 Galaxies Reborn
//
// ======================================================================

#include "FirstDirect3d11.h"
#include "Direct3d11_TextureFormatMap.h"

#include "Direct3d11_Device.h"

#include "clientGraphics/TextureFormatInfo.h"
#include "sharedFoundation/ConfigSharedFoundation.h"

// ======================================================================

namespace Direct3d11_TextureFormatMapNamespace
{
	// One entry per engine format, in enumeration order. UNKNOWN means declined; the
	// header says why for each of the five.
	//
	// The colour orders are not interchangeable. The engine stores a pixel as
	// 0xAARRGGBB, which little-endian is the bytes B, G, R, A ascending, so the
	// B8G8R8A8 family is the direct equivalent of what D3D9 called A8R8G8B8.
	// Choosing the R8G8B8A8 family instead reads the same bytes with red and blue
	// exchanged.
	DXGI_FORMAT const cms_dxgiFormat[TF_Count] =
	{
		DXGI_FORMAT_B8G8R8A8_UNORM,       // TF_ARGB_8888
		DXGI_FORMAT_B4G4R4A4_UNORM,       // TF_ARGB_4444
		DXGI_FORMAT_B5G5R5A1_UNORM,       // TF_ARGB_1555
		DXGI_FORMAT_B8G8R8X8_UNORM,       // TF_XRGB_8888  -- sole choice in two UI paths
		DXGI_FORMAT_UNKNOWN,              // TF_RGB_888    -- nothing is 24 bits
		DXGI_FORMAT_UNKNOWN,              // TF_RGB_565    -- declined for parity
		DXGI_FORMAT_UNKNOWN,              // TF_RGB_555    -- parity, and the pad-bit trap
		DXGI_FORMAT_BC1_UNORM,            // TF_DXT1
		DXGI_FORMAT_BC2_UNORM,            // TF_DXT2       -- premultiplied tag dropped
		DXGI_FORMAT_BC2_UNORM,            // TF_DXT3
		DXGI_FORMAT_BC3_UNORM,            // TF_DXT4       -- premultiplied tag dropped
		DXGI_FORMAT_BC3_UNORM,            // TF_DXT5
		DXGI_FORMAT_A8_UNORM,             // TF_A_8
		DXGI_FORMAT_B8G8R8A8_UNORM,       // TF_L_8        -- stored expanded, see the header
		DXGI_FORMAT_UNKNOWN,              // TF_P_8        -- palettised, no D3D11 equivalent
		DXGI_FORMAT_R16G16B16A16_FLOAT,   // TF_ABGR_16F
		DXGI_FORMAT_R32G32B32A32_FLOAT    // TF_ABGR_32F
	};
}
using namespace Direct3d11_TextureFormatMapNamespace;

// ======================================================================
/**
 * Ask the device which formats it can sample, and tell the engine.
 *
 * The capability tested is TEXTURE2D plus SHADER_SAMPLE, which is the closest
 * equivalent of what DX9 asked for. DX9 checked neither cube nor render-target
 * capability even though it went on to create cube textures with the result, so
 * testing only these two is exact parity. TEXTURECUBE is tested as well, but only to
 * produce a clear message: the very first texture the client loads is a cube map, so
 * a device that could sample a format but not cube it would otherwise fail inside
 * CreateTexture2D with nothing naming the cause.
 */

void Direct3d11_TextureFormatMap::install()
{
	ID3D11Device1 * const device = Direct3d11_Device::getDevice();
	NOT_NULL(device);

	bool const verbose = ConfigSharedFoundation::getVerboseHardwareLogging();

	for (int i = 0; i < static_cast<int>(TF_Count); ++i)
	{
		TextureFormat const format = static_cast<TextureFormat>(i);
		DXGI_FORMAT const dxgiFormat = cms_dxgiFormat[i];

		bool supported = false;

		if (dxgiFormat != DXGI_FORMAT_UNKNOWN)
		{
			UINT support = 0;
			if (SUCCEEDED(device->CheckFormatSupport(dxgiFormat, &support)))
			{
				supported = ((support & D3D11_FORMAT_SUPPORT_TEXTURE2D) != 0) && ((support & D3D11_FORMAT_SUPPORT_SHADER_SAMPLE) != 0);

				WARNING(supported && !(support & D3D11_FORMAT_SUPPORT_TEXTURECUBE), ("Direct3d11: format %s can be sampled but not used for a cube map; a cube texture in this format will fail to create.", TextureFormatInfo::getInfo(format).name ? TextureFormatInfo::getInfo(format).name : "<unnamed>"));
			}
		}

		TextureFormatInfo::setSupported(format, supported);

		REPORT_LOG(verbose, ("Direct3d11 texture format %-14s %s\n",
			TextureFormatInfo::getInfo(format).name ? TextureFormatInfo::getInfo(format).name : "<unnamed>",
			supported ? "supported" : "not supported"));
	}

	// These two are load-bearing rather than nice to have. TF_ARGB_8888 terminates
	// almost every preference list the engine registers, and TF_XRGB_8888 is the
	// SOLE entry in two of them, so a device that cannot do them cannot run the
	// client at all -- better to say so here than to fail on an arbitrary texture.
	FATAL(!TextureFormatInfo::getInfo(TF_ARGB_8888).supported, ("Direct3d11: this device cannot sample B8G8R8A8, which nearly every texture in the game falls back to."));
	FATAL(!TextureFormatInfo::getInfo(TF_XRGB_8888).supported, ("Direct3d11: this device cannot sample B8G8R8X8, which two interface paths require as their only option."));

	// TF_A_8 and TF_L_8 each have a conversion list containing only themselves, so
	// there is nothing to fall through to and an asset in either format would be
	// fatal at load. Say so now instead. TF_L_8's entry is B8G8R8A8, already checked
	// above, so only TF_A_8 can actually be missing.
	WARNING(!TextureFormatInfo::getInfo(TF_A_8).supported, ("Direct3d11: this device cannot sample A8_UNORM; any alpha-only texture in the asset set will be fatal, because TF_A_8 has no fallback format."));
}

// ======================================================================

DXGI_FORMAT Direct3d11_TextureFormatMap::getDxgiFormat(TextureFormat format)
{
	if (static_cast<int>(format) < 0 || static_cast<int>(format) >= static_cast<int>(TF_Count))
		return DXGI_FORMAT_UNKNOWN;

	return cms_dxgiFormat[format];
}

// ----------------------------------------------------------------------

TextureFormat Direct3d11_TextureFormatMap::getStorageFormat(TextureFormat format)
{
	// TF_L_8's resource is B8G8R8A8, so its rows are laid out as TF_ARGB_8888 and
	// every read or write of one goes through the converter. Everything else is
	// stored in its own layout.
	if (format == TF_L_8)
		return TF_ARGB_8888;

	return format;
}

// ======================================================================
