// ======================================================================
// Headless_TextureGraphicsData.cpp
//
// Stub TextureGraphicsData for the Headless graphics driver. Nothing
// actually renders, but the engine still calls Texture::load which locks
// the texture, memcpy's pixel bytes in, then unlocks. If lock leaves
// LockData with a null pixel buffer the memcpy AVs. So we allocate a
// scratch buffer per lock big enough to hold the requested rect at the
// requested format, write it into LockData, and free it on unlock. The
// pixels are never used (the Headless API never uploads to a GPU).
// ======================================================================

#include "FirstHeadless.h"
#include "Headless_TextureGraphicsData.h"

#include "clientGraphics/TextureFormatInfo.h"

#include <cstdlib>

// ----------------------------------------------------------------------

Headless_TextureGraphicsData::Headless_TextureGraphicsData(TextureFormat format) : m_format(format)
{
}

// ----------------------------------------------------------------------

TextureFormat Headless_TextureGraphicsData::getNativeFormat() const
{
	return m_format;
}

// ----------------------------------------------------------------------

void Headless_TextureGraphicsData::lock(LockData &lockData)
{
	// Compute the size of one slice of the requested rect at the LockData's format.
	const TextureFormatInfo &tfi = TextureFormatInfo::getInfo(lockData.m_format);

	int const width = lockData.m_width > 0 ? lockData.m_width : 1;
	int const height = lockData.m_height > 0 ? lockData.m_height : 1;
	int const depth = lockData.m_depth > 0 ? lockData.m_depth : 1;

	int pitch;
	int sliceBytes;
	if (tfi.compressed)
	{
		int const blocksWide = (width + tfi.blockWidth - 1) / tfi.blockWidth;
		int const blocksHigh = (height + tfi.blockHeight - 1) / tfi.blockHeight;
		pitch = blocksWide * tfi.blockSize;
		sliceBytes = pitch * blocksHigh;
	}
	else
	{
		pitch = width * tfi.pixelByteCount;
		sliceBytes = pitch * height;
	}

	int const totalBytes = sliceBytes * depth;

	// Throw-away scratch buffer. The engine will write into it via memcpy and
	// never read it back through Headless because we never present a frame.
	lockData.m_pixelData = std::malloc(static_cast<size_t>(totalBytes));
	lockData.m_pitch = pitch;
	lockData.m_slicePitch = sliceBytes;
}

// ----------------------------------------------------------------------

void Headless_TextureGraphicsData::unlock(LockData &lockData)
{
	std::free(lockData.m_pixelData);
	lockData.m_pixelData = NULL;
}

// ----------------------------------------------------------------------

void Headless_TextureGraphicsData::copyFrom(
	int /*surfaceLevel*/,
	TextureGraphicsData const & /*rhs*/,
	int /*srcX*/,
	int /*srcY*/,
	int /*srcWidth*/,
	int /*srcHeight*/,
	int /*dstX*/,
	int /*dstY*/,
	int /*dstWidth*/,
	int /*dstHeight*/)
{
	// No-op.
}

// ======================================================================
