// ======================================================================
//
// Direct3d11_TextureConverter.cpp
// copyright (c) 2026 Galaxies Reborn
//
// ======================================================================

#include "FirstDirect3d11.h"
#include "Direct3d11_TextureConverter.h"

#include "clientGraphics/TextureFormatInfo.h"

// ======================================================================

namespace Direct3d11_TextureConverterNamespace
{
	// The intermediate. Byte order B, G, R, A ascending, which is TF_ARGB_8888's row
	// layout on a little-endian machine and also DXGI_FORMAT_B8G8R8A8_UNORM's.
	int const cms_intermediateByteCount = 4;
	int const cms_blockPixels = 16;
	int const cms_blockSide = 4;

	// ----------------------------------------------------------------------
	// Bit twiddling

	inline int lowestSetBit(uint32 mask)
	{
		if (!mask)
			return 0;

		int bit = 0;
		while (!(mask & 1))
		{
			mask >>= 1;
			++bit;
		}
		return bit;
	}

	// Widen an n-bit channel to 8 bits by replicating its bits upward, so the maximum
	// value of the narrow channel becomes 255 rather than 255 minus the slack. This is
	// what the hardware does when it samples the narrow format, so a converted read
	// agrees with what a shader would have seen.
	inline uint8 widenChannel(uint32 value, int bits)
	{
		if (bits <= 0)
			return 0;
		if (bits >= 8)
			return static_cast<uint8>(value & 0xff);

		uint32 accumulated = value;
		int filled = bits;
		while (filled < 8)
		{
			accumulated = (accumulated << bits) | value;
			filled += bits;
		}

		return static_cast<uint8>(accumulated >> (filled - 8));
	}

	inline uint32 narrowChannel(uint8 value, int bits)
	{
		if (bits <= 0)
			return 0;
		if (bits >= 8)
			return value;

		return static_cast<uint32>(value) >> (8 - bits);
	}

	inline uint32 readLittleEndian(uint8 const *source, int byteCount)
	{
		uint32 value = 0;
		for (int i = 0; i < byteCount; ++i)
			value |= static_cast<uint32>(source[i]) << (i * 8);
		return value;
	}

	inline void writeLittleEndian(uint8 *destination, int byteCount, uint32 value)
	{
		for (int i = 0; i < byteCount; ++i)
			destination[i] = static_cast<uint8>((value >> (i * 8)) & 0xff);
	}

	// ----------------------------------------------------------------------
	/**
	 * One row of an uncompressed engine format into the B, G, R, A intermediate.
	 *
	 * Driven entirely by the masks and bit counts the engine already publishes, so
	 * every uncompressed pair is covered without a table of pairs. Two conventions
	 * the masks alone do not capture:
	 *
	 *   A format with no alpha channel reads as opaque, not as zero. That is what
	 *   the sampler does for X8R8G8B8 and R5G6B5.
	 *
	 *   TF_L_8 is a LUMINANCE format, not a red-only one. The engine's own table
	 *   files its single byte under rBitCount/rMask, but D3DFMT_L8 samples as
	 *   (L, L, L, 1), so the byte is replicated across all three colour channels.
	 *   Treating it as red would make every shader reading such a texture see zero
	 *   in green and blue.
	 */

	void decodeRow(TextureFormat format, uint8 const *source, uint8 *destination, int width)
	{
		TextureFormatInfo const &info = TextureFormatInfo::getInfo(format);

		if (format == TF_L_8)
		{
			for (int x = 0; x < width; ++x)
			{
				uint8 const luminance = source[x];
				destination[(x * 4) + 0] = luminance;
				destination[(x * 4) + 1] = luminance;
				destination[(x * 4) + 2] = luminance;
				destination[(x * 4) + 3] = 255;
			}
			return;
		}

		int const bytes = info.pixelByteCount;

		int const aShift = lowestSetBit(info.aMask);
		int const rShift = lowestSetBit(info.rMask);
		int const gShift = lowestSetBit(info.gMask);
		int const bShift = lowestSetBit(info.bMask);

		for (int x = 0; x < width; ++x)
		{
			uint32 const pixel = readLittleEndian(source + (x * bytes), bytes);

			destination[(x * 4) + 0] = widenChannel((pixel & info.bMask) >> bShift, info.bBitCount);
			destination[(x * 4) + 1] = widenChannel((pixel & info.gMask) >> gShift, info.gBitCount);
			destination[(x * 4) + 2] = widenChannel((pixel & info.rMask) >> rShift, info.rBitCount);
			destination[(x * 4) + 3] = info.aBitCount ? widenChannel((pixel & info.aMask) >> aShift, info.aBitCount) : 255;
		}
	}

	// ----------------------------------------------------------------------
	/**
	 * The intermediate back out into one row of an uncompressed engine format.
	 *
	 * Bits the destination has no channel for are left zero. For TF_XRGB_8888 that is
	 * the ignored pad byte, which the sampler reads as opaque whatever it holds, so
	 * zero is as good as anything and is at least deterministic.
	 */

	void encodeRow(TextureFormat format, uint8 const *source, uint8 *destination, int width)
	{
		TextureFormatInfo const &info = TextureFormatInfo::getInfo(format);

		if (format == TF_L_8)
		{
			// The red channel, matching what decodeRow replicated out of it. A
			// weighted luminance would be more principled but would not round-trip,
			// and would differ from D3DX, which also took the channel named by the
			// mask.
			for (int x = 0; x < width; ++x)
				destination[x] = source[(x * 4) + 2];
			return;
		}

		int const bytes = info.pixelByteCount;

		int const aShift = lowestSetBit(info.aMask);
		int const rShift = lowestSetBit(info.rMask);
		int const gShift = lowestSetBit(info.gMask);
		int const bShift = lowestSetBit(info.bMask);

		for (int x = 0; x < width; ++x)
		{
			uint32 pixel = 0;
			pixel |= (narrowChannel(source[(x * 4) + 0], info.bBitCount) << bShift) & info.bMask;
			pixel |= (narrowChannel(source[(x * 4) + 1], info.gBitCount) << gShift) & info.gMask;
			pixel |= (narrowChannel(source[(x * 4) + 2], info.rBitCount) << rShift) & info.rMask;
			pixel |= (narrowChannel(source[(x * 4) + 3], info.aBitCount) << aShift) & info.aMask;

			writeLittleEndian(destination + (x * bytes), bytes, pixel);
		}
	}

	// ----------------------------------------------------------------------
	// Block decode. BC1, BC2 and BC3, which is every compressed format this engine
	// has. DXT2 decodes identically to DXT3 and DXT4 identically to DXT5: the
	// premultiplied-alpha distinction is a content convention that changes which
	// blend factors the application should pick, never how the block is read, and
	// D3D9 did not un-premultiply either.

	// Colours 0 and 1 of a BC1 colour block, decoded from 565.
	inline void decodeEndpoint(uint32 packed, uint8 *bgra)
	{
		bgra[0] = widenChannel(packed & 0x1f, 5);
		bgra[1] = widenChannel((packed >> 5) & 0x3f, 6);
		bgra[2] = widenChannel((packed >> 11) & 0x1f, 5);
		bgra[3] = 255;
	}

	/**
	 * Expand one 8-byte colour block into 16 B,G,R,A pixels.
	 *
	 * @param opaqueMode  true for the colour block of a BC2 or BC3 block, where the
	 *                    four-colour interpretation is used unconditionally. Only a
	 *                    standalone BC1 block gets the three-colour-plus-transparent
	 *                    mode, and only when its endpoints are ordered that way.
	 */
	void decodeColourBlock(uint8 const *block, uint8 *pixels, bool opaqueMode)
	{
		uint32 const packed0 = static_cast<uint32>(block[0]) | (static_cast<uint32>(block[1]) << 8);
		uint32 const packed1 = static_cast<uint32>(block[2]) | (static_cast<uint32>(block[3]) << 8);
		uint32 const indices = readLittleEndian(block + 4, 4);

		uint8 colour[4][4];
		decodeEndpoint(packed0, colour[0]);
		decodeEndpoint(packed1, colour[1]);

		if (opaqueMode || packed0 > packed1)
		{
			for (int c = 0; c < 3; ++c)
			{
				colour[2][c] = static_cast<uint8>(((2 * static_cast<int>(colour[0][c])) + static_cast<int>(colour[1][c])) / 3);
				colour[3][c] = static_cast<uint8>((static_cast<int>(colour[0][c]) + (2 * static_cast<int>(colour[1][c]))) / 3);
			}
			colour[2][3] = 255;
			colour[3][3] = 255;
		}
		else
		{
			for (int c = 0; c < 3; ++c)
			{
				colour[2][c] = static_cast<uint8>((static_cast<int>(colour[0][c]) + static_cast<int>(colour[1][c])) / 2);
				colour[3][c] = 0;
			}
			colour[2][3] = 255;

			// The punch-through index. Both colour and alpha are zero, which is what
			// the hardware produces for it.
			colour[3][3] = 0;
		}

		for (int i = 0; i < cms_blockPixels; ++i)
		{
			uint32 const index = (indices >> (i * 2)) & 3;
			pixels[(i * 4) + 0] = colour[index][0];
			pixels[(i * 4) + 1] = colour[index][1];
			pixels[(i * 4) + 2] = colour[index][2];
			pixels[(i * 4) + 3] = colour[index][3];
		}
	}

	// Sixteen explicit 4-bit alphas, two per byte, low nibble first.
	void decodeExplicitAlpha(uint8 const *block, uint8 *pixels)
	{
		for (int i = 0; i < cms_blockPixels; ++i)
		{
			uint8 const both = block[i / 2];
			uint8 const nibble = static_cast<uint8>((i & 1) ? (both >> 4) : (both & 0x0f));
			pixels[(i * 4) + 3] = widenChannel(nibble, 4);
		}
	}

	// Two endpoints and sixteen 3-bit indices, the BC4 alpha encoding.
	void decodeInterpolatedAlpha(uint8 const *block, uint8 *pixels)
	{
		int const alpha0 = block[0];
		int const alpha1 = block[1];

		uint8 alpha[8];
		alpha[0] = static_cast<uint8>(alpha0);
		alpha[1] = static_cast<uint8>(alpha1);

		if (alpha0 > alpha1)
		{
			for (int i = 1; i < 7; ++i)
				alpha[i + 1] = static_cast<uint8>((((7 - i) * alpha0) + (i * alpha1)) / 7);
		}
		else
		{
			for (int i = 1; i < 5; ++i)
				alpha[i + 1] = static_cast<uint8>((((5 - i) * alpha0) + (i * alpha1)) / 5);
			alpha[6] = 0;
			alpha[7] = 255;
		}

		uint64 bits = 0;
		for (int i = 0; i < 6; ++i)
			bits |= static_cast<uint64>(block[2 + i]) << (i * 8);

		for (int i = 0; i < cms_blockPixels; ++i)
		{
			uint32 const index = static_cast<uint32>((bits >> (i * 3)) & 7);
			pixels[(i * 4) + 3] = alpha[index];
		}
	}

	void decodeBlock(TextureFormat format, uint8 const *block, uint8 *pixels)
	{
		switch (format)
		{
			case TF_DXT1:
				decodeColourBlock(block, pixels, false);
				break;

			case TF_DXT2:
			case TF_DXT3:
				decodeColourBlock(block + 8, pixels, true);
				decodeExplicitAlpha(block, pixels);
				break;

			case TF_DXT4:
			case TF_DXT5:
				decodeColourBlock(block + 8, pixels, true);
				decodeInterpolatedAlpha(block, pixels);
				break;

			default:
				FATAL(true, ("Direct3d11: %s is not a block-compressed format.", TextureFormatInfo::getInfo(format).name ? TextureFormatInfo::getInfo(format).name : "<unnamed>"));
				break;
		}
	}

	// ----------------------------------------------------------------------

	inline bool isFloatFormat(TextureFormat format)
	{
		return format == TF_ABGR_16F || format == TF_ABGR_32F;
	}

	char const *describe(TextureFormat format)
	{
		if (static_cast<int>(format) < 0 || static_cast<int>(format) >= static_cast<int>(TF_Count))
			return "<out of range>";

		char const * const name = TextureFormatInfo::getInfo(format).name;
		return name ? name : "<unnamed>";
	}
}
using namespace Direct3d11_TextureConverterNamespace;

// ======================================================================

bool Direct3d11_TextureConverter::canConvert(TextureFormat source, TextureFormat destination)
{
	if (static_cast<int>(source) < 0 || static_cast<int>(source) >= static_cast<int>(TF_Count))
		return false;
	if (static_cast<int>(destination) < 0 || static_cast<int>(destination) >= static_cast<int>(TF_Count))
		return false;

	// A straight copy always works, including for the compressed and float formats,
	// because nothing is being reinterpreted.
	if (source == destination)
		return true;

	// Palettised. There is no palette to consult -- the engine never gives one to the
	// backend -- so nothing can be made of these bytes.
	if (source == TF_P_8 || destination == TF_P_8)
		return false;

	// The float formats would need a genuinely different intermediate. Nothing
	// reaches them: they have no entries in the engine's conversion table at all, and
	// no caller passes one in a preference list, so this is unreachable rather than
	// merely unwritten.
	if (isFloatFormat(source) || isFloatFormat(destination))
		return false;

	// Compression is not implemented. See the header for why it cannot arrive.
	if (TextureFormatInfo::getInfo(destination).compressed)
		return false;

	return true;
}

// ----------------------------------------------------------------------

void Direct3d11_TextureConverter::convert(
	TextureFormat source,      uint8 const *sourceData,      int sourcePitch,
	TextureFormat destination, uint8       *destinationData, int destinationPitch,
	int width, int height)
{
	NOT_NULL(sourceData);
	NOT_NULL(destinationData);

	FATAL(!canConvert(source, destination), ("Direct3d11: no conversion from %s to %s is implemented.", describe(source), describe(destination)));

	if (width <= 0 || height <= 0)
		return;

	// Same format. Row by row rather than one memcpy, because the two pitches need
	// not agree -- one side may be a mapped resource.
	if (source == destination)
	{
		TextureFormatInfo const &info = TextureFormatInfo::getInfo(source);

		int rows = height;
		int bytesPerRow = width * info.pixelByteCount;
		if (info.compressed)
		{
			rows = (height + info.blockHeight - 1) / info.blockHeight;
			bytesPerRow = (width + info.blockWidth - 1) / info.blockWidth * info.blockSize;
		}

		// Clip against both pitches as well as the row's own length: either side can
		// be a mapped resource whose pitch is padded, and either can be the shorter.
		int copyBytes = bytesPerRow;
		if (sourcePitch < copyBytes)
			copyBytes = sourcePitch;
		if (destinationPitch < copyBytes)
			copyBytes = destinationPitch;

		for (int y = 0; y < rows; ++y)
			memcpy(destinationData + (y * destinationPitch), sourceData + (y * sourcePitch), static_cast<size_t>(copyBytes));

		return;
	}

	TextureFormatInfo const &sourceInfo = TextureFormatInfo::getInfo(source);

	if (sourceInfo.compressed)
	{
		// Decode block row by block row. A level narrower or shorter than a block
		// still occupies a whole block, so the loops clip against the region rather
		// than against the block grid -- which is what makes a 1x1 mip level of a
		// DXT texture work, and that is exactly the case
		// Texture::computeRepresentativeColor hits on nearly every texture.
		uint8 blockPixels[cms_blockPixels * cms_intermediateByteCount];
		uint8 intermediateRow[cms_blockSide * cms_intermediateByteCount];

		int const blockColumns = (width + sourceInfo.blockWidth - 1) / sourceInfo.blockWidth;
		int const blockRows = (height + sourceInfo.blockHeight - 1) / sourceInfo.blockHeight;

		for (int blockY = 0; blockY < blockRows; ++blockY)
		{
			uint8 const * const blockRow = sourceData + (blockY * sourcePitch);

			for (int blockX = 0; blockX < blockColumns; ++blockX)
			{
				decodeBlock(source, blockRow + (blockX * sourceInfo.blockSize), blockPixels);

				int const baseX = blockX * 4;
				int const baseY = blockY * 4;

				for (int innerY = 0; innerY < 4; ++innerY)
				{
					int const y = baseY + innerY;
					if (y >= height)
						break;

					int const pixels = (width - baseX < 4) ? (width - baseX) : 4;
					if (pixels <= 0)
						break;

					memcpy(intermediateRow, blockPixels + (innerY * 4 * cms_intermediateByteCount), static_cast<size_t>(pixels * cms_intermediateByteCount));

					if (destination == TF_ARGB_8888)
					{
						// The overwhelmingly common destination, and the
						// intermediate's own layout, so there is nothing to encode.
						memcpy(destinationData + (y * destinationPitch) + (baseX * 4), intermediateRow, static_cast<size_t>(pixels * cms_intermediateByteCount));
					}
					else
					{
						int const destinationBytes = TextureFormatInfo::getInfo(destination).pixelByteCount;
						encodeRow(destination, intermediateRow, destinationData + (y * destinationPitch) + (baseX * destinationBytes), pixels);
					}
				}
			}
		}

		return;
	}

	// Uncompressed to uncompressed, one row at a time through the intermediate.
	int const intermediateBytes = width * cms_intermediateByteCount;
	uint8 * const intermediate = new uint8[intermediateBytes];

	for (int y = 0; y < height; ++y)
	{
		decodeRow(source, sourceData + (y * sourcePitch), intermediate, width);
		encodeRow(destination, intermediate, destinationData + (y * destinationPitch), width);
	}

	delete [] intermediate;
}

// ======================================================================
