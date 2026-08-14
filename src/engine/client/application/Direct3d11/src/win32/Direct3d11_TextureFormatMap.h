// ======================================================================
//
// Direct3d11_TextureFormatMap.h
// copyright (c) 2026 Galaxies Reborn
//
// Engine texture formats to DXGI formats, and the support sweep that tells the
// engine which ones it may choose.
//
// The sweep is not optional. Every entry in the engine's format table starts
// unsupported, and createTextureData walks a caller-supplied preference list
// looking for the first supported one. With no sweep the walk matches nothing and
// the first texture the client loads is fatal.
//
// Everything stays plain _UNORM. No _SRGB variant is used anywhere: the shipped
// .dds files carry no colour-space metadata, the engine has lit in gamma space
// throughout its life, and an sRGB view would change every texture at once in a way
// indistinguishable from a port defect.
//
// ----------------------------------------------------------------------
// Two formats: the one the engine is told, and the one the resource has
//
// getDxgiFormat answers "what does the resource holding this engine format look
// like", and getStorageFormat answers "whose byte layout does that resource have".
// They differ for exactly one format, TF_L_8, and the distinction is what keeps
// that case from being a lie:
//
//   D3DFMT_L8 samples as (L, L, L, 1). The obvious DXGI counterpart R8_UNORM
//   samples as (R, 0, 0, 1), and D3D11 has no SRV component swizzle to reconcile
//   them, so renaming the format would silently change what every shader reading
//   such a texture sees. Instead an L8 texture is created as B8G8R8A8 and the
//   single channel is replicated across RGB on upload, while the engine continues
//   to be told the format is TF_L_8 -- which keeps its one-byte-per-pixel pitch
//   contract intact. It costs four times the memory for a format almost nothing
//   uses, and TF_L_8's conversion list has no fallback entry, so declining it
//   instead would make any L8 asset fatal.
//
// ----------------------------------------------------------------------
// Five engine formats are declined, and the last three are declined for parity
//
//   TF_RGB_888  no DXGI format is 24 bits per pixel.
//
//   TF_P_8      palettised. D3D11 has no palette support at all.
//
//   TF_RGB_565  representable -- DXGI_FORMAT_B5G6R5_UNORM matches the engine's row
//   TF_RGB_555  for 565 exactly -- but declined anyway, because DX9 has never
//   TF_RGB_888  supported any of the three. DX9's translation table maps all of
//               TF_RGB_888, TF_RGB_565 and TF_RGB_555 to D3DFMT_R8G8B8
//               (Direct3d9_TextureData.cpp:39-41): three consecutive identical
//               entries, and no D3D9 driver has ever supported that format. So
//               CheckDeviceFormat has always failed for all three and their
//               conversion lists have always fallen through -- an R5G6B5 .dds has
//               always loaded as TF_ARGB_1555, and an R8G8B8 .dds as
//               TF_XRGB_8888.
//
//               Mapping 565 here would quietly change which format those assets
//               load in. It is very likely the better answer -- it halves their
//               memory and DX9's 24-bit row arithmetic was wrong anyway -- but it
//               is a visual change, and this port earns visual changes only after
//               parity is proven. Turning it on later is one table entry.
//
//               TF_RGB_555 additionally carries a real trap: D3D9's X1R5G5B5
//               IGNORES bit 15, while the nearest DXGI format B5G5R5A1_UNORM reads
//               it as alpha, so any asset whose pad bit happens to be zero would
//               become fully transparent. That one should stay declined even after
//               parity is abandoned.
//
// Declining is safe for four of the five: TF_RGB_888, TF_RGB_565 and TF_RGB_555 all
// appear only as non-final entries in lists that end in TF_ARGB_8888. TF_P_8 is the
// exception -- its conversion list contains only itself, so a PNM1 .dds is fatal.
// That is not a regression: D3DFMT_P8 is equally unsupported everywhere, so on DX9
// the same asset produced a DEBUG_FATAL in a debug build and a null texture that
// draws nothing in a release one.
//
// ======================================================================

#ifndef INCLUDED_Direct3d11_TextureFormatMap_H
#define INCLUDED_Direct3d11_TextureFormatMap_H

// ======================================================================

#include "clientGraphics/Texture.def"

#include <d3d11_1.h>

// ======================================================================

class Direct3d11_TextureFormatMap
{
public:
	// Probes the device and publishes the results through
	// TextureFormatInfo::setSupported. Must run before the first createTextureData.
	static void install();

	// The DXGI format a resource of this engine format is created with, or
	// DXGI_FORMAT_UNKNOWN when the engine format is declined.
	static DXGI_FORMAT getDxgiFormat(TextureFormat format);

	// The engine format whose row layout that resource actually has. Identity for
	// everything except TF_L_8, which is stored as TF_ARGB_8888.
	static TextureFormat getStorageFormat(TextureFormat format);

private:
	Direct3d11_TextureFormatMap();
	Direct3d11_TextureFormatMap(Direct3d11_TextureFormatMap const &);
	Direct3d11_TextureFormatMap &operator=(Direct3d11_TextureFormatMap const &);
};

// ======================================================================

#endif
