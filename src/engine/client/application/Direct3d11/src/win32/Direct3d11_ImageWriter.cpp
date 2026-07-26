// ======================================================================
//
// Direct3d11_ImageWriter.cpp
// copyright (c) 2026 Galaxies Reborn
//
// Screen shots, image writing, and back buffer locking. All three read the
// scene target rather than the swap chain: a FLIP_DISCARD back buffer cannot be
// mapped at all, and its contents are undefined after Present.
//
// This is the commit that makes the parity work possible. Until something can
// produce a file from a frame, "identical to DX9" is not a claim that can be
// checked, and neither is its opposite.
//
// One deliberate difference from DX9 is worth stating plainly. DX9 applies its
// colour correction table to a screenshot only when NOT windowed
// (Direct3d9.cpp:2731-2733), because SetGammaRamp does nothing on a windowed
// swap chain -- so in windowed mode neither the screen nor the capture is
// corrected, and in fullscreen both are. Here correction happens in the
// composite, which is real in both modes, and captures are taken after it. At
// identity settings -- the shipped default, and what every parity capture pins
// -- there is no correction in either backend and the two agree byte for byte.
//
// ======================================================================

#include "FirstDirect3d11.h"
#include "Direct3d11_ImageWriter.h"

#include "Direct3d11_Device.h"
#include "Direct3d11_SceneTarget.h"
#include "Direct3d11_Unimplemented.h"
#include "WriteTga.h"

#include "clientGraphics/Gl_dll.def"
#include "clientGraphics/Graphics.h"
#include "sharedFoundation/Os.h"

// ======================================================================

namespace Direct3d11_ImageWriterNamespace
{
	ID3D11Texture2D  *ms_lockedTexture;
	uint8            *ms_lockedPixels;

	// The engine hands out 32-bit ARGB. The scene target is R8G8B8A8_UNORM, so
	// the red and blue channels are the other way round and every path through
	// here has to swap them exactly once.
	void              convertRgbaToArgbInPlace(uint8 *pixels, int width, int height, int pitch);
}
using namespace Direct3d11_ImageWriterNamespace;

// ======================================================================

void Direct3d11_ImageWriterNamespace::convertRgbaToArgbInPlace(uint8 *pixels, int width, int height, int pitch)
{
	for (int y = 0; y < height; ++y)
	{
		uint8 *row = pixels + (y * pitch);
		for (int x = 0; x < width; ++x, row += 4)
		{
			uint8 const red = row[0];
			row[0] = row[2];
			row[2] = red;
		}
	}
}

// ======================================================================
/**
 * Write the current frame to a file.
 *
 * TGA is implemented, through the same WriteTGA module the DX9 backend uses, so
 * the two produce identical files from identical pixels. JPG and BMP need the
 * JPEG encoder and WIC respectively and are named absences rather than silent
 * ones -- the parity harness captures TGA.
 */

bool Direct3d11_ImageWriter::screenShot(GlScreenShotFormat format, int quality, char const *fileName)
{
	UNREF(quality);

	if (format != GSSF_tga)
	{
		DX11_NOT_IMPLEMENTED("screenShot (bmp/jpg)");
		Graphics::setLastError("engine", "screenshot_failed_wrong_format");
		return false;
	}

	ID3D11Texture2D * const staging = Direct3d11_SceneTarget::createReadbackCopy();
	if (!staging)
	{
		Graphics::setLastError("engine", "screenshot_failed_unknown");
		return false;
	}

	ID3D11DeviceContext1 * const context = Direct3d11_Device::getContext();
	NOT_NULL(context);

	D3D11_MAPPED_SUBRESOURCE mapped;
	Zero(mapped);
	HRESULT const hresult = context->Map(staging, 0, D3D11_MAP_READ, 0, &mapped);
	if (FAILED(hresult))
	{
		WARNING(true, ("Direct3d11: the screen shot staging texture could not be mapped (%s).", Direct3d11_Device::describeHresult(hresult)));
		Graphics::setLastError("engine", "screenshot_failed_unknown");
		return false;
	}

	int const width  = Direct3d11_SceneTarget::getWidth();
	int const height = Direct3d11_SceneTarget::getHeight();

	convertRgbaToArgbInPlace(static_cast<uint8 *>(mapped.pData), width, height, static_cast<int>(mapped.RowPitch));

	char buffer[Os::MAX_PATH_LENGTH];
	sprintf(buffer, "%s.tga", fileName);
	WriteTGA::write(buffer, width, height, static_cast<uint8 const *>(mapped.pData), true, static_cast<int>(mapped.RowPitch));

	context->Unmap(staging, 0);

	return true;
}

// ----------------------------------------------------------------------
/**
 * Write pixels the caller already has.
 *
 * Nothing is read back here -- the data comes from the caller -- so this needs
 * no device work at all beyond choosing a container.
 */

bool Direct3d11_ImageWriter::writeImage(char const *fileName, int width, int height, int pitch, int const *pixelsARGB, bool alphaExtend, Gl_imageFormat imageFormat, Rectangle2d const *subRect)
{
	UNREF(alphaExtend);
	UNREF(subRect);

	if (imageFormat != GLIF_tga)
	{
		DX11_NOT_IMPLEMENTED("writeImage (non-tga)");
		return false;
	}

	NOT_NULL(fileName);
	NOT_NULL(pixelsARGB);

	WriteTGA::write(fileName, width, height, reinterpret_cast<uint8 const *>(pixelsARGB), true, pitch);
	return true;
}

// ======================================================================
/**
 * Hand the caller a readable copy of the frame.
 *
 * Bink is the only consumer, and it fills in a rectangle and then passes null
 * for it, so the whole surface is mapped and Bink offsets into it itself using
 * the reported pitch. Anything that returned a rectangle-relative pointer, or a
 * pitch that did not describe a full-surface mapping, would corrupt memory
 * outside the video region rather than merely drawing wrong.
 *
 * Note this path is normally unreachable: supportsDynamicTextures() returns true
 * and BinkVideo::performBlitting returns at its first line when it does.
 */

bool Direct3d11_ImageWriter::lockBackBuffer(Gl_pixelRect &pixels, RECT const *lockRect)
{
	UNREF(lockRect);

	DEBUG_FATAL(ms_lockedTexture, ("Direct3d11: lockBackBuffer called while already locked"));

	ID3D11Texture2D * const staging = Direct3d11_SceneTarget::createReadbackCopy();
	if (!staging)
		return false;

	ID3D11DeviceContext1 * const context = Direct3d11_Device::getContext();
	NOT_NULL(context);

	D3D11_MAPPED_SUBRESOURCE mapped;
	Zero(mapped);
	HRESULT const hresult = context->Map(staging, 0, D3D11_MAP_READ, 0, &mapped);
	if (FAILED(hresult))
	{
		WARNING(true, ("Direct3d11: lockBackBuffer could not map the staging texture (%s).", Direct3d11_Device::describeHresult(hresult)));
		return false;
	}

	ms_lockedTexture = staging;
	ms_lockedPixels  = static_cast<uint8 *>(mapped.pData);

	convertRgbaToArgbInPlace(ms_lockedPixels, Direct3d11_SceneTarget::getWidth(), Direct3d11_SceneTarget::getHeight(), static_cast<int>(mapped.RowPitch));

	// Gl_pixelRect carries no dimensions: the caller already knows them from
	// Graphics, and reads by pitch from the base pointer.
	pixels.pixels    = ms_lockedPixels;
	pixels.pitch     = mapped.RowPitch;
	pixels.colorBits = 32;
	pixels.alphaBits = 8;

	return true;
}

// ----------------------------------------------------------------------

bool Direct3d11_ImageWriter::unlockBackBuffer()
{
	if (!ms_lockedTexture)
	{
		DEBUG_WARNING(true, ("Direct3d11: unlockBackBuffer called without a matching lock"));
		return false;
	}

	ID3D11DeviceContext1 * const context = Direct3d11_Device::getContext();
	if (context)
		context->Unmap(ms_lockedTexture, 0);

	ms_lockedTexture = NULL;
	ms_lockedPixels = NULL;

	return true;
}

// ======================================================================
