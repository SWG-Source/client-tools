// ======================================================================
//
// Texture.cpp
//
// copyright 1998 Bootprint Entertainment
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/Texture.h"

#include "clientGraphics/ConfigClientGraphics.h"
#include "clientGraphics/Dds.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/TextureFormatInfo.h"
#include "clientGraphics/TextureList.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/Production.h"

namespace DDS
{
	const DDS_PIXELFORMAT DDSPF_L8_WHACK = { sizeof(DDS_PIXELFORMAT), 0, 0, 8, 0x000000ff, 0x00000000, 0x000000000, 0x00000000 };
}

// ======================================================================

namespace TextureNamespace
{
	void addConversion(TextureFormat source, TextureFormat destination);

	int const                         MAX_TEXTURE_CONVERSIONS = 6;

	bool                              ms_installed;
	MemoryBlockManager *ms_memoryBlockManager;
	int                               ms_conversionCount[TF_Count];
	TextureFormat                     ms_conversions[TF_Count][MAX_TEXTURE_CONVERSIONS];

	int ms_discardHighestMipMapLevels;
	int ms_discardHighestNormalMipMapLevels;
}
using namespace TextureNamespace;

// ======================================================================

TextureGraphicsData::~TextureGraphicsData()
{
}

// ======================================================================

void Texture::install(void)
{
	DEBUG_FATAL(ms_installed, ("Texture already installed"));
	ms_installed = true;
	ExitChain::add(remove, "Texture::remove()");

	ms_memoryBlockManager = new MemoryBlockManager("Texture::ms_memoryBlockManager", true, sizeof(Texture), 0, 0, 0);

	ms_discardHighestMipMapLevels = ConfigClientGraphics::getDiscardHighestMipMapLevels();

#if PRODUCTION == 1
	if (Graphics::getVideoMemoryInMegabytes () < 40)
		ms_discardHighestMipMapLevels = 1;
#endif

	ms_discardHighestNormalMipMapLevels = ConfigClientGraphics::getDiscardHighestNormalMipMapLevels();
#if PRODUCTION == 1
	if (Graphics::getVideoMemoryInMegabytes () < 40)
		ms_discardHighestNormalMipMapLevels = 1;
#endif

	//-- tmp hack for TOW beta
	ms_discardHighestMipMapLevels = 0;
	ms_discardHighestNormalMipMapLevels = 0;

	addConversion(TF_ARGB_8888, TF_ARGB_8888);
	addConversion(TF_ARGB_8888, TF_ARGB_4444);

	addConversion(TF_ARGB_4444, TF_ARGB_4444);
	addConversion(TF_ARGB_4444, TF_ARGB_8888);

	addConversion(TF_ARGB_1555, TF_ARGB_1555);
	addConversion(TF_ARGB_1555, TF_ARGB_4444);
	addConversion(TF_ARGB_1555, TF_ARGB_8888);

	addConversion(TF_XRGB_8888, TF_XRGB_8888);
	addConversion(TF_XRGB_8888, TF_RGB_888);
	addConversion(TF_XRGB_8888, TF_ARGB_8888);
	addConversion(TF_XRGB_8888, TF_RGB_565);
	addConversion(TF_XRGB_8888, TF_RGB_555);

	addConversion(TF_RGB_888,   TF_RGB_888);
	addConversion(TF_RGB_888,   TF_XRGB_8888);
	addConversion(TF_RGB_888,   TF_ARGB_8888);
	addConversion(TF_RGB_888,   TF_RGB_565);
	addConversion(TF_RGB_888,   TF_RGB_555);

	addConversion(TF_RGB_565,   TF_RGB_565);
	addConversion(TF_RGB_565,   TF_RGB_555);
	addConversion(TF_RGB_565,   TF_ARGB_1555);
	addConversion(TF_RGB_565,   TF_RGB_888);
	addConversion(TF_RGB_565,   TF_ARGB_8888);

	addConversion(TF_RGB_555,   TF_RGB_555);
	addConversion(TF_RGB_555,   TF_RGB_565);
	addConversion(TF_RGB_555,   TF_ARGB_1555);
	addConversion(TF_RGB_555,   TF_RGB_888);
	addConversion(TF_RGB_555,   TF_ARGB_8888);

	addConversion(TF_DXT1,      TF_DXT1);
	addConversion(TF_DXT1,      TF_ARGB_1555);
	addConversion(TF_DXT1,      TF_ARGB_8888);

	addConversion(TF_DXT2,      TF_DXT2);
	addConversion(TF_DXT2,      TF_ARGB_8888);

	addConversion(TF_DXT3,      TF_DXT3);
	addConversion(TF_DXT3,      TF_ARGB_8888);

	addConversion(TF_DXT4,      TF_DXT4);
	addConversion(TF_DXT4,      TF_ARGB_8888);

	addConversion(TF_DXT5,      TF_DXT5);
	addConversion(TF_DXT5,      TF_ARGB_8888);

	addConversion(TF_A_8,       TF_A_8);
	addConversion(TF_L_8,       TF_L_8);

	addConversion(TF_P_8,       TF_P_8);
}

// ----------------------------------------------------------------------

void Texture::remove(void)
{
	DEBUG_FATAL(!ms_installed, ("Texture not installed"));

	ms_installed = false;
	delete ms_memoryBlockManager;
	ms_memoryBlockManager = 0;
}

// ----------------------------------------------------------------------

void TextureNamespace::addConversion(TextureFormat source, TextureFormat destination)
{
	DEBUG_FATAL(ms_conversionCount[source] == MAX_TEXTURE_CONVERSIONS, ("Too many texture conversions"));
	ms_conversions[source][ms_conversionCount[source]++] = destination;
}

// ----------------------------------------------------------------------

void *Texture::operator new(size_t size)
{
	UNREF(size);
	NOT_NULL (ms_memoryBlockManager);
	DEBUG_FATAL(size != sizeof (Texture), ("bad size"));
	DEBUG_FATAL(size != static_cast<size_t> (ms_memoryBlockManager->getElementSize()), ("installed with bad size"));

	return ms_memoryBlockManager->allocate();
}

// ----------------------------------------------------------------------

void Texture::operator delete(void *pointer)
{
	NOT_NULL (ms_memoryBlockManager);
	ms_memoryBlockManager->free(pointer);
}

// ======================================================================

Texture::Texture(CrcString const & fileName)
:
	m_referenceCount(0),
	m_crcString(fileName),
	m_renderTarget(false),
	m_cube(false),
	m_volume(false),
	m_dynamic(false),
	m_cursorHack(false),
	m_width(0),
	m_height(0),
	m_depth(0),
	m_mipmapLevelCount(0),
	m_graphicsData(0),
	m_representativeColorComputed(0),
	m_representativeColor()
{
	load(m_crcString.getString());
}

// ----------------------------------------------------------------------
/**
 * Construct a 1x1x1 texture with the given rgba profile.
 */

Texture::Texture(int alpha, int red, int green, int blue)
:
	m_referenceCount(0),
	m_crcString(),
	m_renderTarget(false),
	m_cube(false),
	m_volume(false),
	m_dynamic(false),
	m_cursorHack(false),
	m_width(1),
	m_height(1),
	m_depth(1),
	m_mipmapLevelCount(1),
	m_graphicsData(0),
	m_representativeColorComputed(0),
	m_representativeColor()
{
	TextureFormat sourceFormat = TF_ARGB_8888;

	m_graphicsData = Graphics::createTextureData(*this, &sourceFormat, 1);

	LockData lockData(sourceFormat, 0, 0, 0, 1, 1, true);
	lock(lockData);

		reinterpret_cast<uint32*>(lockData.getPixelData())[0] = (clamp(0, alpha, 255) << 24) | (clamp(0, red, 255) << 16) | (clamp(0, green, 255) << 8) | (clamp(0, blue, 255) << 0);
		
	unlock(lockData);
}

// ----------------------------------------------------------------------
/**
 * Construct a texture from raw pixel data.
 *
 * @param newPixelData The pixel data.
 * @param newFormat    The pixel format.
 * @param newWidth     The width of the texture and pixel data.
 * @param newHeight    The height of the texture and pixel data.
 */

Texture::Texture(const void *newPixelData, TextureFormat newFormat, int newWidth, int newHeight, int newDepth, const TextureFormat *runtimeFormatArray, int runtimeFormatCount)
:
	m_referenceCount(0),
	m_crcString(),
	m_renderTarget(false),
	m_cube(false),
	m_volume(newDepth > 1),
	m_dynamic(false),
	m_cursorHack(false),
	m_width(newWidth),
	m_height(newHeight),
	m_depth(newDepth),
	m_mipmapLevelCount(1),
	m_graphicsData(0),
	m_representativeColorComputed(0),
	m_representativeColor()
{
	m_graphicsData = Graphics::createTextureData(*this, runtimeFormatArray, runtimeFormatCount);

	LockData lockData(newFormat, 0, 0, 0, 0, m_width, m_height, m_depth, true);
	lock(lockData);

		const TextureFormatInfo &tfi             = TextureFormatInfo::getInfo(newFormat);
		const int                pitch           = m_width * tfi.pixelByteCount;
		uint8                   *lockedDataWrite = reinterpret_cast<uint8*>(lockData.getPixelData());
		const uint8             *readData        = reinterpret_cast<const uint8*>(newPixelData);

		if (lockData.getPitch() == pitch && lockData.getSlicePitch() == pitch * m_height)
		{
			imemcpy(lockedDataWrite, readData, pitch * m_height * m_depth);
		}
		else
		{
			uint8 *lockedSliceWrite = lockedDataWrite;
			for (int z = 0; z < m_depth; ++z, lockedSliceWrite += lockData.getSlicePitch())
			{
				lockedDataWrite = lockedSliceWrite;
				for (int y = 0; y < m_height; ++y, lockedDataWrite += lockData.getPitch(), readData += pitch)
					imemcpy(lockedDataWrite, readData, pitch);
			}
		}

	unlock(lockData);
}

// ----------------------------------------------------------------------

Texture::Texture(int creationFlags, int newWidth, int newHeight, int numberOfMipMapLevels, const TextureFormat *runtimeTextureFormats, int textureFormatCount)
:
	m_referenceCount(0),
	m_crcString(),
	m_renderTarget((creationFlags&TCF_renderTarget)!=0),
	m_cube((creationFlags&TCF_cubeMap)!=0),
	m_volume(false),
	m_dynamic((creationFlags&TCF_dynamic)!=0),
	m_cursorHack(false),
	m_width(newWidth),
	m_height(newHeight),
	m_depth(1),
	m_mipmapLevelCount(numberOfMipMapLevels),
	m_graphicsData(NULL),
	m_representativeColorComputed(0),
	m_representativeColor()
{
	m_graphicsData = Graphics::createTextureData(*this, runtimeTextureFormats, textureFormatCount);
}

// ----------------------------------------------------------------------

Texture::~Texture(void)
{
	delete m_graphicsData;
}

// ----------------------------------------------------------------------

void Texture::fetch() const
{
	++m_referenceCount;
}

// ----------------------------------------------------------------------

void Texture::release() const
{
	if (--m_referenceCount < 1)
	{
		FATAL(m_referenceCount < 0, ("Texture reference count has gone negative"));
		TextureList::removeFromList(this);
		delete const_cast<Texture*>(this);
	}
}

// ----------------------------------------------------------------------

void Texture::loadSurface(TextureFormat format, CubeFace face, AbstractFile *fileInterface, int numberOfHighestMipmapLevelsToDiscard, int numberOfLowestMipmapLevelsToDiscard)
{
	UNREF(numberOfHighestMipmapLevelsToDiscard);

	// get pixel size info from source format
	const TextureFormatInfo &tfi = TextureFormatInfo::getInfo(format);

	// lock the entire texture rectangle, move the bits into the gl managed memory
	for (int i = 0; i < numberOfHighestMipmapLevelsToDiscard + m_mipmapLevelCount + numberOfLowestMipmapLevelsToDiscard; ++i)
	{
		int width  = m_width;
		int height = m_height;
		int depth  = m_depth;

		if (i < numberOfHighestMipmapLevelsToDiscard)
		{
			width  <<= (numberOfHighestMipmapLevelsToDiscard - i);
			height <<= (numberOfHighestMipmapLevelsToDiscard - i);
			depth  <<= (numberOfHighestMipmapLevelsToDiscard - i);
		}
		else
		{
			width >>= (i - numberOfHighestMipmapLevelsToDiscard); //lint !e702 // Shift right of signed quantity
			height >>= (i - numberOfHighestMipmapLevelsToDiscard); //lint !e702 // Shift right of signed quantity
			depth >>= (i - numberOfHighestMipmapLevelsToDiscard); //lint !e702 // Shift right of signed quantity

			if (width == 0)
				width = 1;
			if (height == 0)
				height = 1;
			if (depth == 0)
				depth = 1;
		}

		// figure out the pitch of a row in the file and how many rows there are in a slice
		//   how many slices
		int filePitch = 0;
		int fileHeight = 0;
		if (tfi.compressed)
		{
			// compressed files are in integral blocks
			filePitch  = (width  + tfi.blockWidth  - 1) / tfi.blockWidth * tfi.blockSize;
			fileHeight = (height + tfi.blockHeight - 1) / tfi.blockHeight;
		}
		else
		{
			filePitch  = width * tfi.pixelByteCount;
			fileHeight = height;
		}

		int fileSlices = depth;

		// determine whether to load or skip this mipmap
		if (i >= numberOfHighestMipmapLevelsToDiscard && i < numberOfHighestMipmapLevelsToDiscard+m_mipmapLevelCount)
		{
			LockData lockData(format, face, i - numberOfHighestMipmapLevelsToDiscard, 0, 0, 0, width, height, depth, m_dynamic); 

			lock(lockData);

				// if the file and surface pitches are the same, we can do this all in a single read
				int surfacePitch = lockData.getPitch();
				if (filePitch == surfacePitch)
				{
					surfacePitch = filePitch *= fileHeight * fileSlices;
					fileHeight = 1;
					fileSlices = 1;
				}

				// read in all the data
				uint8 *lockedDataWrite = reinterpret_cast<uint8*>(lockData.getPixelData());

				for (int z = 0; z < fileSlices; ++z)
				{
					for (int y = 0; y < fileHeight; ++y, lockedDataWrite += surfacePitch)
					{
						const int bytesRead = fileInterface->read(lockedDataWrite, filePitch);
						DEBUG_FATAL(bytesRead != filePitch, ("read failed"));
						UNREF(bytesRead);
					}
				}

			unlock(lockData);								
		}
		else
		{
			const bool result = fileInterface->seek(AbstractFile::SeekCurrent, filePitch * fileHeight * fileSlices);
			DEBUG_FATAL(!result, ("seek failed"));
			UNREF(result);
		}
	}
}

// ----------------------------------------------------------------------

inline static bool matches(const DDS::DDS_PIXELFORMAT &src, const DDS::DDS_PIXELFORMAT &dst)
{
	if (src.dwRGBBitCount != dst.dwRGBBitCount)
		return false;

	if ((src.dwFlags & 0x7ff) != (dst.dwFlags & 0x7ff)) 
		return false;

	if (src.dwFlags & DDS::DDS_RGB)
	{
		if (src.dwRBitMask != dst.dwRBitMask)
			return false;
		if (src.dwGBitMask != dst.dwGBitMask)
			return false;
		if (src.dwBBitMask != dst.dwBBitMask)
			return false;
	}
	else if (src.dwFlags & DDS::DDS_LUMINANCE)
	{
		if (src.dwRBitMask != dst.dwRBitMask)
			return false;
	}
	else if (src.dwFlags & DDS::DDS_INTENSITY)
	{
		if (src.dwRBitMask != dst.dwRBitMask)
			return false;
	}
	if (src.dwFlags & DDS::DDS_ALPHA)
	{
		if (src.dwABitMask != dst.dwABitMask)
			return false;
	}

	return true;
}

// ----------------------------------------------------------------------

void Texture::load(const char * fileName)
{
	AbstractFile *fileInterface = TreeFile::open(fileName, AbstractFile::PriorityData, true);
	if (!fileInterface)
	{
		DEBUG_FATAL(fileName == TextureList::getDefaultTextureName(), ("Could not open default texture"));
		WARNING(true, ("Could not open texture %s", fileName));
		load(TextureList::getDefaultTextureName());
		return;
	}

	TextureFormat sourceFormat = TF_Count;

	// validate the DDS magic header
	unsigned long magic = 0;
	const int magicRead = fileInterface->read(&magic, 4);
	UNREF(magicRead);
	DEBUG_FATAL(magicRead != 4, ("Could not read DDS magic header"));
	DEBUG_FATAL(magic != DDS::MakeFourCC('D', 'D', 'S', ' '), ("DDS magic number invalid"));

	// get texture information
	DDS::DDS_HEADER  ddsHeader;
	const int headerRead = fileInterface->read(&ddsHeader, sizeof(ddsHeader));
	DEBUG_FATAL(headerRead != isizeof(ddsHeader), ("DDS header read failed"));
	UNREF(headerRead);

	// pixelformat flags in the DDS header are one of
	// DDS_FOURCC, DDS_RGBA or DDS_RGB. For the RGB and RGBA
	// format files, the bitcount and bitmasks are used to determine
	// the appropriate pixelformat of the texture as it is represnted
	// internally by the TextureFormat enumeration
	m_height = static_cast<int>(ddsHeader.dwHeight);
	m_width  = static_cast<int>(ddsHeader.dwWidth);

	m_cube   = ddsHeader.dwComplexFlags & DDS::DDS_COMPLEX_FLAGS_CUBEMAP ? true : false;
	m_volume = ddsHeader.dwComplexFlags & DDS::DDS_COMPLEX_FLAGS_VOLUMEMAP ? true : false;
	m_depth = ddsHeader.dwHeaderFlags & DDS::DDS_HEADER_FLAGS_VOLUME ? static_cast<int>(ddsHeader.dwDepth) : 1;

	m_mipmapLevelCount = static_cast<int>(ddsHeader.dwMipMapCount);
	if (m_mipmapLevelCount == 0)
		m_mipmapLevelCount = 1;

#ifdef _DEBUG
	{
		if (m_mipmapLevelCount > 1)
		{
			int minWidth = m_width >> m_mipmapLevelCount;
			if (minWidth == 0)
				minWidth = 1;

			int minHeight = m_height >> m_mipmapLevelCount;
			if (minHeight == 0)
				minHeight = 1;

			int minDepth = m_depth >> m_mipmapLevelCount;
			if (minDepth == 0)
				minDepth = 1;

			DEBUG_WARNING(minWidth > 1 || minHeight > 1 || minDepth > 1, ("Texture %s has too few mipmaps %dx%dx%d@%d (smallest map is %dx%dx%d)", fileName, m_width, m_height, m_depth, m_mipmapLevelCount, minWidth, minHeight, minDepth));
		}
	}
#endif

	// determine pixelformat
	// Compressed formats
	bool dxt = false;
	if(ddsHeader.ddspf.dwFlags & DDS::DDS_FOURCC)
	{
		FATAL(m_volume, ("Compressed volume textures not supported (yet)"));
		if(ddsHeader.ddspf.dwFourCC == DDS::DDSPF_PNM1.dwFourCC)
			sourceFormat = TF_P_8;
		else
		{
			dxt = true;
			if(ddsHeader.ddspf.dwFourCC == DDS::DDSPF_DXT1.dwFourCC)
				sourceFormat = TF_DXT1;
			else if(ddsHeader.ddspf.dwFourCC == DDS::DDSPF_DXT2.dwFourCC)
				sourceFormat = TF_DXT2;
			else if(ddsHeader.ddspf.dwFourCC == DDS::DDSPF_DXT3.dwFourCC)
				sourceFormat = TF_DXT3;
			else if(ddsHeader.ddspf.dwFourCC == DDS::DDSPF_DXT4.dwFourCC)
				sourceFormat = TF_DXT4;
			else if(ddsHeader.ddspf.dwFourCC == DDS::DDSPF_DXT5.dwFourCC)
				sourceFormat = TF_DXT5;
			else
				DEBUG_FATAL(true, ("Unknown 4cc texture format"));
		}
	}
	// Component formats
	else
	{
		// RGBA Formats
		if (matches(ddsHeader.ddspf, DDS::DDSPF_A8R8G8B8))
			sourceFormat = TF_ARGB_8888;
		else if (matches(ddsHeader.ddspf, DDS::DDSPF_A1R5G5B5))
			sourceFormat = TF_ARGB_1555;
		else if (matches(ddsHeader.ddspf, DDS::DDSPF_A4R4G4B4))
			sourceFormat = TF_ARGB_4444;
		// RGB formats
		else if (matches(ddsHeader.ddspf, DDS::DDSPF_R8G8B8))
			sourceFormat = TF_RGB_888;
		else if (matches(ddsHeader.ddspf, DDS::DDSPF_R5G6B5))
			sourceFormat = TF_RGB_565;
		else if (matches(ddsHeader.ddspf, DDS::DDSPF_R3G3B2))
			sourceFormat = TF_RGB_565;

		// Luminescence formats
		else if (matches(ddsHeader.ddspf, DDS::DDSPF_L8))
			sourceFormat = TF_L_8;
		else if (matches(ddsHeader.ddspf, DDS::DDSPF_L8_WHACK))
			sourceFormat = TF_L_8;

		// Alpha formats
		else if (matches(ddsHeader.ddspf, DDS::DDSPF_A8))
			sourceFormat = TF_A_8;
		else
		{
			delete fileInterface;

			DEBUG_WARNING(true, ("Unknown component format for texture %s", fileName));
			load(TextureList::getDefaultTextureName());
			return;
		}
	}

	// the radeon 8500 doesn't support mipmapped cube maps
	int numberOfLowestMipmapLevelsToDiscard = 0;
	if (m_cube && m_mipmapLevelCount > 1 && !Graphics::supportsMipmappedCubeMaps())
	{
		numberOfLowestMipmapLevelsToDiscard = m_mipmapLevelCount - 1;
		m_mipmapLevelCount = 1;
	}

	bool const isNormalMapName = strstr(fileName, "_n.dds") != 0;

#ifdef _DEBUG
	if (DataLint::isEnabled())
	{
		if (isNormalMapName)
			DEBUG_WARNING(sourceFormat != TF_ARGB_8888, ("[%s] has the normal map convention, but is not ARGB (%d != %d)", fileName, sourceFormat, TF_ARGB_8888));
		else
			DEBUG_WARNING(!(sourceFormat == TF_DXT1 || sourceFormat == TF_DXT3 || sourceFormat == TF_DXT5), ("[%s] is not compressed (%d)", fileName, sourceFormat));
	}
#endif

	// option to discard top mipmap levels for texture memory & performance
	int pleaseDiscard = ms_discardHighestMipMapLevels;
	if (isNormalMapName != NULL && ms_discardHighestNormalMipMapLevels > ms_discardHighestMipMapLevels)
		pleaseDiscard = ms_discardHighestNormalMipMapLevels;
	int numberOfHighestMipmapLevelsToDiscard = clamp(0, pleaseDiscard, m_mipmapLevelCount - 1);
	if (numberOfHighestMipmapLevelsToDiscard)
	{
		if (dxt)
		{
			// dxt compressed textures can be no smaller than 4x4
			while (((m_width >> numberOfHighestMipmapLevelsToDiscard) < 4) || ((m_height >> numberOfHighestMipmapLevelsToDiscard) < 4) || ((m_depth >> numberOfHighestMipmapLevelsToDiscard) < 4))
				--numberOfHighestMipmapLevelsToDiscard;
		}
		else
		{
			// normal textures can be no smaller than 1x1
			while (((m_width >> numberOfHighestMipmapLevelsToDiscard) < 1) || ((m_height >> numberOfHighestMipmapLevelsToDiscard) < 1) || ((m_depth >> numberOfHighestMipmapLevelsToDiscard) < 1))
				--numberOfHighestMipmapLevelsToDiscard;
		}
	
		m_width >>= numberOfHighestMipmapLevelsToDiscard;
		m_height >>= numberOfHighestMipmapLevelsToDiscard;
		m_depth >>= numberOfHighestMipmapLevelsToDiscard;
		m_mipmapLevelCount -= numberOfHighestMipmapLevelsToDiscard;
	}

	if (!m_graphicsData)
		m_graphicsData = Graphics::createTextureData(*this, ms_conversions[sourceFormat], ms_conversionCount[sourceFormat]);

	if (m_cube)
	{
		for (int i = CF_positiveX; i <= CF_negativeZ; ++i)
			loadSurface(sourceFormat, static_cast<CubeFace>(i), fileInterface, numberOfHighestMipmapLevelsToDiscard, numberOfLowestMipmapLevelsToDiscard);
	}
	else
	{
		// NOTE: works for 2d and volume textures...
		loadSurface(sourceFormat, CF_none, fileInterface, numberOfHighestMipmapLevelsToDiscard, numberOfLowestMipmapLevelsToDiscard);
	}

	delete fileInterface;
}

// ----------------------------------------------------------------------

PackedArgb Texture::getRepresentativeColor() const
{
	if (m_representativeColorComputed < 5)
	{
		m_representativeColor = computeRepresentativeColor();
		++m_representativeColorComputed;
	}

	return m_representativeColor;
}

// ----------------------------------------------------------------------
/**
 * Lock the entire texture at the specified Mipmap level
 *
 * @param lockData Specifies the requested region to lock.
 * @see Texture::LockData
*/

void Texture::lock(LockData &lockData)
{
	DEBUG_FATAL(lockData.m_pixelData, ("already locked"));
	lockData.m_readOnly = false;
	NOT_NULL(m_graphicsData);
	m_graphicsData->lock(lockData);
}

// ----------------------------------------------------------------------
/**
 * Lock the entire texture at the specified Mipmap level for read-only access.
 *
 * @param lockData Specifies the requested region to lock.
 * @see Texture::LockData
*/

void Texture::lockReadOnly(LockData &lockData) const
{
	DEBUG_FATAL(lockData.m_discardContents, ("cannot use lockReadOnly() and discardContents"));
	DEBUG_FATAL(lockData.m_pixelData, ("already locked"));
	lockData.m_readOnly = true;
	NOT_NULL(m_graphicsData);
	m_graphicsData->lock(lockData);
}

// ----------------------------------------------------------------------

void Texture::unlock(LockData &lockData) const
{
	DEBUG_FATAL(!lockData.m_pixelData, ("not locked"));
	NOT_NULL(m_graphicsData);
	m_graphicsData->unlock(lockData);
}

// ----------------------------------------------------------------------

TextureFormat Texture::getNativeFormat() const
{
	NOT_NULL(m_graphicsData);
	return m_graphicsData->getNativeFormat();
}

// ----------------------------------------------------------------------

PackedArgb Texture::computeRepresentativeColor() const
{
	//-- Lock a single pixel in a simple format we can handle.
	//   NOTE: this will be an expensive call for textures not already in TF_ARGB_8888 format.
	LockData  lockData(TF_ARGB_8888, getMipmapLevelCount() - 1, 0, 0, 1, 1, false);
	lockReadOnly(lockData);

	//-- Retrieve the pixel value.
	void *pixelData = lockData.getPixelData();
	NOT_NULL(pixelData);

	PackedArgb  color(*reinterpret_cast<uint32 const*>(pixelData));
#if 0
	DEBUG_REPORT_LOG(true, ("Texture:rep color[%s] = a/r/g/b [%d/%d/%d/%d].\n", getName(), color.getA(), color.getR(), color.getG(), color.getB()));
#endif

	//-- Unlock data.
	unlock(lockData);

	return color;
}

//----------------------------------------------------------------------

void Texture::copyFrom(int surfaceLevel, Texture const & rhs, int srcX, int srcY, int srcWidth, int srcHeight, int dstX, int dstY, int dstWidth, int dstHeight)
{
	m_graphicsData->copyFrom(surfaceLevel, *rhs.getGraphicsData(), srcX, srcY, srcWidth, srcHeight, dstX, dstY, dstWidth, dstHeight);
}

//----------------------------------------------------------------------
void Texture::copyPixels(const void *pixelData, TextureFormat sourceFormat, int width, int height)
{
	if(sourceFormat != m_graphicsData->getNativeFormat())
	{
		WARNING(true, ("copyPixelsFromSource: Source and Destination texture formats are not equal.\n"));
		return;
	}

	if(m_width != width || m_height != height)
	{
		WARNING(true, ("copyPixelsFromSource: Source and Destination height/width are not equal.\n"));
		return;
	}
		
	LockData lockData(sourceFormat, 0, 0, 0, 0, m_width, m_height, m_depth, true);
	lock(lockData);

	const TextureFormatInfo &tfi             = TextureFormatInfo::getInfo(sourceFormat);
	const int                pitch           = m_width * tfi.pixelByteCount;
	uint8                   *lockedDataWrite = reinterpret_cast<uint8*>(lockData.getPixelData());
	const uint8             *readData        = reinterpret_cast<const uint8*>(pixelData);

	if (lockData.getPitch() == pitch && lockData.getSlicePitch() == pitch * m_height)
	{
		imemcpy(lockedDataWrite, readData, pitch * m_height * m_depth);
	}
	else
	{
		uint8 *lockedSliceWrite = lockedDataWrite;
		for (int z = 0; z < m_depth; ++z, lockedSliceWrite += lockData.getSlicePitch())
		{
			lockedDataWrite = lockedSliceWrite;
			for (int y = 0; y < m_height; ++y, lockedDataWrite += lockData.getPitch(), readData += pitch)
				imemcpy(lockedDataWrite, readData, pitch);
		}
	}

	unlock(lockData);
}
// ======================================================================
