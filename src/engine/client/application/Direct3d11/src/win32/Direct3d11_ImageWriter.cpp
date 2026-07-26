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
#include "Direct3d11_Metrics.h"
#include "Direct3d11_SceneTarget.h"
#include "Direct3d11_Unimplemented.h"
#include "WriteTga.h"

#include "clientGraphics/Gl_dll.def"
#include "clientGraphics/Graphics.h"
#include "sharedFoundation/Os.h"

#include <wincodec.h>

// ======================================================================

namespace Direct3d11_ImageWriterNamespace
{
	ID3D11Texture2D  *ms_lockedTexture;
	uint8            *ms_lockedPixels;

	// The engine hands out 32-bit ARGB. The scene target is R8G8B8A8_UNORM, so
	// the red and blue channels are the other way round and every path through
	// here has to swap them exactly once.
	void              convertRgbaToArgbInPlace(uint8 *pixels, int width, int height, int pitch);

	// ------------------------------------------------------------------
	// The containers TGA does not cover.
	//
	// DX9 reaches these through D3DXSaveSurfaceToFile, which does not exist for D3D11. The
	// replacement is WIC, which ships with Windows, needs no third-party encoder, and is what
	// DirectXTex uses for exactly this. It covers BMP, JPEG and PNG; it has no encoder for DDS,
	// PPM, PFM or Radiance HDR, and those are named rather than silently producing a BMP with
	// the wrong extension.
	//
	// Pixels arrive as 32-bit BGRA in memory, which is what convertRgbaToArgbInPlace leaves
	// behind and what the engine calls ARGB -- the same bytes, named from the other end.

	bool              writeWithWic(char const *fileName, int width, int height, int pitch, uint8 const *pixelsBgra, GUID const &container, int quality);
	GUID const       *containerForImageFormat(Gl_imageFormat imageFormat);
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
 * Which WIC container an engine image format wants, or null if WIC has none.
 */

GUID const *Direct3d11_ImageWriterNamespace::containerForImageFormat(Gl_imageFormat imageFormat)
{
	switch (imageFormat)
	{
		// A DIB is a BMP without the file header as far as the engine's callers are concerned;
		// nothing reads one back, so the container is the same.
		case GLIF_bmp:
		case GLIF_dib:
			return &GUID_ContainerFormatBmp;

		case GLIF_jpg:
			return &GUID_ContainerFormatJpeg;

		case GLIF_png:
			return &GUID_ContainerFormatPng;

		default:
			return NULL;
	}
}

// ----------------------------------------------------------------------
/**
 * Encode 32-bit BGRA pixels into a WIC container.
 *
 * A frame's worth of COM objects created and released per call, which is right for something a
 * user triggers by pressing a key. Nothing is cached: a screen shot is not a frame path, and a
 * cached factory would be one more thing to tear down on device loss.
 */

bool Direct3d11_ImageWriterNamespace::writeWithWic(char const *fileName, int width, int height, int pitch, uint8 const *pixelsBgra, GUID const &container, int quality)
{
	NOT_NULL(fileName);
	NOT_NULL(pixelsBgra);

	if (width <= 0 || height <= 0)
		return false;

	// The client initialises COM before graphics comes up, but this must not depend on which
	// apartment model it chose -- RPC_E_CHANGED_MODE means COM is up in the other one, which is
	// fine for WIC. S_FALSE means it was already initialised on this thread.
	HRESULT const coInitialise = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	bool const uninitialise = SUCCEEDED(coInitialise);

	IWICImagingFactory *factory = NULL;
	IWICBitmapEncoder *encoder = NULL;
	IWICBitmapFrameEncode *frame = NULL;
	IPropertyBag2 *options = NULL;
	IWICStream *stream = NULL;
	bool succeeded = false;

	HRESULT hresult = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory));

	if (SUCCEEDED(hresult))
		hresult = factory->CreateStream(&stream);

	if (SUCCEEDED(hresult))
	{
		// WIC wants a wide path. The engine's file names are ASCII, so this is a widen rather
		// than a code page conversion.
		wchar_t wide[Os::MAX_PATH_LENGTH];
		int i = 0;
		for (; fileName[i] && i < (isizeof(wide) / isizeof(wide[0])) - 1; ++i)
			wide[i] = static_cast<wchar_t>(static_cast<unsigned char>(fileName[i]));
		wide[i] = L'\0';

		hresult = stream->InitializeFromFilename(wide, GENERIC_WRITE);
	}

	if (SUCCEEDED(hresult))
		hresult = factory->CreateEncoder(container, NULL, &encoder);

	if (SUCCEEDED(hresult))
		hresult = encoder->Initialize(stream, WICBitmapEncoderNoCache);

	if (SUCCEEDED(hresult))
		hresult = encoder->CreateNewFrame(&frame, &options);

	if (SUCCEEDED(hresult))
	{
		// JPEG is the only one of these that has a quality knob. The engine passes 0..100; WIC
		// wants 0..1, and a zero from a caller that did not care would otherwise encode an
		// unusable image, so it is treated as "unspecified" and left at the encoder's default.
		if (options && IsEqualGUID(container, GUID_ContainerFormatJpeg) && quality > 0)
		{
			PROPBAG2 property;
			Zero(property);
			property.pstrName = const_cast<LPOLESTR>(L"ImageQuality");

			VARIANT value;
			VariantInit(&value);
			value.vt = VT_R4;
			value.fltVal = static_cast<float>(quality > 100 ? 100 : quality) / 100.0f;

			IGNORE_RETURN(options->Write(1, &property, &value));
		}

		hresult = frame->Initialize(options);
	}

	if (SUCCEEDED(hresult))
		hresult = frame->SetSize(static_cast<UINT>(width), static_cast<UINT>(height));

	if (SUCCEEDED(hresult))
	{
		// The source is opaque, and asking for a format the container cannot store makes the
		// encoder convert rather than fail. BGRA in, whatever the container needs out.
		WICPixelFormatGUID pixelFormat = GUID_WICPixelFormat32bppBGRA;
		hresult = frame->SetPixelFormat(&pixelFormat);
	}

	if (SUCCEEDED(hresult))
		hresult = frame->WritePixels(
			static_cast<UINT>(height),
			static_cast<UINT>(pitch),
			static_cast<UINT>(pitch * height),
			const_cast<BYTE *>(pixelsBgra));

	if (SUCCEEDED(hresult))
		hresult = frame->Commit();

	if (SUCCEEDED(hresult))
		hresult = encoder->Commit();

	succeeded = SUCCEEDED(hresult);

	if (!succeeded)
		WARNING(true, ("Direct3d11: WIC could not write '%s' (%s).", fileName, Direct3d11_Device::describeHresult(hresult)));

	if (options) options->Release();
	if (frame)   frame->Release();
	if (encoder) encoder->Release();
	if (stream)  stream->Release();
	if (factory) factory->Release();

	if (uninitialise)
		CoUninitialize();

	return succeeded;
}

// ======================================================================
/**
 * Write the current frame to a file.
 *
 * TGA goes through the same WriteTGA module the DX9 backend uses, so the two produce identical
 * files from identical pixels -- which is why the parity harness captures TGA. BMP and JPG go
 * through WIC, replacing the D3DXSaveSurfaceToFile DX9 used.
 */

bool Direct3d11_ImageWriter::screenShot(GlScreenShotFormat format, int quality, char const *fileName)
{
	if (format != GSSF_tga && format != GSSF_bmp && format != GSSF_jpg)
	{
		DX11_NOT_IMPLEMENTED("screenShot (unknown format)");
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
	// A read map with no DO_NOT_WAIT flag blocks until the GPU is done with the
	// copy. That is acceptable for a screen shot, which is a deliberate user
	// action, and it is counted so it can never hide inside a benchmark frame.
	++Direct3d11_Metrics::blockingStagingMaps;

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

	bool written = false;
	char buffer[Os::MAX_PATH_LENGTH];

	if (format == GSSF_tga)
	{
		sprintf(buffer, "%s.tga", fileName);
		WriteTGA::write(buffer, width, height, static_cast<uint8 const *>(mapped.pData), true, static_cast<int>(mapped.RowPitch));
		written = true;
	}
	else
	{
		bool const isJpeg = (format == GSSF_jpg);
		sprintf(buffer, "%s.%s", fileName, isJpeg ? "jpg" : "bmp");

		written = writeWithWic(
			buffer, width, height, static_cast<int>(mapped.RowPitch),
			static_cast<uint8 const *>(mapped.pData),
			isJpeg ? GUID_ContainerFormatJpeg : GUID_ContainerFormatBmp,
			quality);
	}

	context->Unmap(staging, 0);

	if (!written)
		Graphics::setLastError("engine", "screenshot_failed_unknown");

	return written;
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

	NOT_NULL(fileName);
	NOT_NULL(pixelsARGB);

	if (imageFormat == GLIF_tga)
	{
		WriteTGA::write(fileName, width, height, reinterpret_cast<uint8 const *>(pixelsARGB), true, pitch);
		return true;
	}

	GUID const * const container = containerForImageFormat(imageFormat);
	if (!container)
	{
		// DDS, PPM, PFM and Radiance HDR. WIC has no encoder for any of them, and writing one
		// of these formats' bytes by hand is a different job from choosing a container. Named,
		// because a caller silently receiving a BMP under a .dds name is worse.
		DX11_NOT_IMPLEMENTED("writeImage (dds/ppm/pfm/hdr)");
		return false;
	}

	// No quality argument reaches this entry point, so the encoder default stands.
	return writeWithWic(fileName, width, height, pitch, reinterpret_cast<uint8 const *>(pixelsARGB), *container, 0);
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
	++Direct3d11_Metrics::backBufferMaps;
	++Direct3d11_Metrics::blockingStagingMaps;

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
