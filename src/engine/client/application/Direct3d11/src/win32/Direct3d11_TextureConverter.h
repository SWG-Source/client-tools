// ======================================================================
//
// Direct3d11_TextureConverter.h
// copyright (c) 2026 Galaxies Reborn
//
// Pixel conversion between engine texture formats.
//
// This exists because the engine locks a texture in whatever format it feels like,
// which is routinely not the format the texture is stored in, and D3D11 has no
// equivalent of the D3DX call DX9 leaned on. Three separate engine paths need it:
//
//   1. Texture::loadSurface locks every mip level in the SOURCE FILE's format. When
//      that format is one this backend declines, the resource is in the fallback
//      format and every row written has to be converted. An R8G8B8 .dds writes
//      24-bit rows into a 32-bit resource.
//
//   2. Texture::computeRepresentativeColor locks the smallest mip level as
//      TF_ARGB_8888 and reads one pixel, whatever the texture is stored as. For the
//      DXT textures that make up nearly the whole asset set, that is a block
//      decode. It is called for every texture whose representative colour the
//      interface or the LOD system asks for, so getting it wrong is not subtle: it
//      tints things.
//
//   3. TF_L_8 is stored expanded to B8G8R8A8, so both directions of every L8 lock
//      pass through here.
//
// The route is always via 32-bit B, G, R, A -- the layout of TF_ARGB_8888, which is
// also the engine's own idea of a pixel. Going through one intermediate rather than
// writing N-squared direct paths is the difference between a table of formats and a
// table of format PAIRS, and the conversions this needs are not hot: they happen at
// load, once per representative colour, and never inside a frame.
//
// Channels narrower than 8 bits are widened by bit replication, not by a shift: a
// 5-bit 31 becomes 255, not 248. That is what D3DX did and what the hardware does
// when it samples, so a 1555 texture read back as 8888 gives the same numbers the
// shader would have seen.
//
// Encoding TO a block-compressed format is deliberately absent. DX9 could do it
// (D3DX would compress), but no engine path asks for it -- loadSurface locks a DXT
// file's levels in the DXT format the resource already has, and nothing else writes
// a compressed texture -- so implementing a compressor here would be several hundred
// lines of untested, unreachable code whose quality nobody could evaluate. Asking
// for one is fatal and names both formats.
//
// ======================================================================

#ifndef INCLUDED_Direct3d11_TextureConverter_H
#define INCLUDED_Direct3d11_TextureConverter_H

// ======================================================================

#include "clientGraphics/Texture.def"

// ======================================================================

class Direct3d11_TextureConverter
{
public:
	// True when convert() can carry pixels from one format to the other. False means
	// the pair is unreachable by design rather than merely unimplemented -- callers
	// should report the pair, not silently skip the copy.
	static bool canConvert(TextureFormat source, TextureFormat destination);

	// Convert a width x height region. Pitches are in bytes and describe the two
	// buffers as laid out for their own formats; for a compressed format the pitch
	// is one ROW OF BLOCKS and the buffer holds ceil(height/4) of them.
	//
	// Source and destination may not overlap.
	static void convert(
		TextureFormat source, uint8 const *sourceData, int sourcePitch,
		TextureFormat destination, uint8 *destinationData, int destinationPitch,
		int width, int height);

private:
	Direct3d11_TextureConverter();
	Direct3d11_TextureConverter(Direct3d11_TextureConverter const &);
	Direct3d11_TextureConverter &operator=(Direct3d11_TextureConverter const &);
};

// ======================================================================

#endif
