// ======================================================================
// TextureFormatInfo.cpp
//
// copyright 1998 Bootprint Entertainment
// copyright 2001 Sony Online Entertainment
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/TextureFormatInfo.h"

// ======================================================================

static TextureFormatInfo info[TF_Count] =
{
	// supported  compressed bytes  a,  r   g   b  a mask       r mask,     g mask,     b mask,     blockWidth  blockHeight  blockSize     name             
	{  false,     false,     4,     8,  8,  8,  8, 0xff000000,  0x00ff0000, 0x0000ff00, 0x000000ff, 0,          0,           0,            "TF_ARGB_8888",  }, // TF_ARGB_8888
	{  false,     false,     2,     4,  4,  4,  4, 0x0000f000,  0x00000f00, 0x000000f0, 0x0000000f, 0,          0,           0,            "TF_ARGB_4444",  }, // TF_ARGB_4444
	{  false,     false,     2,     1,  5,  5,  5, 0x00008000,  0x00007c00, 0x000003e0, 0x0000001f, 0,          0,           0,            "TF_ARGB_1555",  }, // TF_ARGB_1555
	{  false,     false,     4,     0,  8,  8,  8, 0x00000000,  0x00ff0000, 0x0000ff00, 0x000000ff, 0,          0,           0,            "TF_XRGB_8888",  }, // TF_XRGB_8888
	{  false,     false,     3,     0,  8,  8,  8, 0x00000000,  0x00ff0000, 0x0000ff00, 0x000000ff, 0,          0,           0,            "TF_RGB_888",    }, // TF_RGB_888
	{  false,     false,     2,     0,  5,  6,  5, 0x00000000,  0x0000f800, 0x000007e0, 0x0000001f, 0,          0,           0,            "TF_RGB_565",    }, // TF_RGB_565
	{  false,     false,     2,     0,  5,  5,  5, 0x00000000,  0x00007c00, 0x000003e0, 0x0000001f, 0,          0,           0,            "TF_RGB_555",    }, // TF_RGB_555
	{  false,     true,      0,     0,  0,  0,  0, 0x00000000,  0x00000000, 0x00000000, 0x00000000, 4,          4,           8,            "TF_DXT1",       }, // TF_DXT1
	{  false,     true,      0,     0,  0,  0,  0, 0x00000000,  0x00000000, 0x00000000, 0x00000000, 4,          4,          16,            "TF_DXT2",       }, // TF_DXT2
	{  false,     true,      0,     0,  0,  0,  0, 0x00000000,  0x00000000, 0x00000000, 0x00000000, 4,          4,          16,            "TF_DXT3",       }, // TF_DXT3
	{  false,     true,      0,     0,  0,  0,  0, 0x00000000,  0x00000000, 0x00000000, 0x00000000, 4,          4,          16,            "TF_DXT4",       }, // TF_DXT4
	{  false,     true,      0,     0,  0,  0,  0, 0x00000000,  0x00000000, 0x00000000, 0x00000000, 4,          4,          16,            "TF_DXT5",       }, // TF_DXT5
	{  false,     false,     1,     8,  0,  0,  0, 0x000000ff,  0x00000000, 0x00000000, 0x00000000, 0,          0,           0,            "TF_A_8",        }, // TF_A_8,
	{  false,     false,     1,     0,  8,  0,  0, 0x00000000,  0x000000ff, 0x00000000, 0x00000000, 0,          0,           0,            "TF_L_8",        }, // TF_L_8,
	{  false,     false,     1,     0,  8,  8,  8, 0x00000000,  0x00000000, 0x00000000, 0x00000000, 1,          1,           1,            "TF_P_8",        }, // TF_P_8,
};

// ======================================================================
/**
 * Get information about a texture format.
 *
 * This routine will return a struct of information about the specified texture format.  
 * @param format The texture format for which information is desired.
 */
const TextureFormatInfo &TextureFormatInfo::getInfo(TextureFormat format)
{
	DEBUG_FATAL(static_cast<int>(format) < 0 || static_cast<int>(format) >= static_cast<int>(TF_Count), ("format arg out of range: value/max %d/%d", static_cast<int>(format), static_cast<int>(TF_Count)-1));
	return info[format];
}

// ----------------------------------------------------------------------
/**
 * Set whether a texture is supported as a 2D texture or not.
 * @internal
 * @param format    The texture format to modify.
 * @param supported True if the texture format is supported, otherwise false.
 */

void TextureFormatInfo::setSupported(TextureFormat format, bool supported)
{
	DEBUG_FATAL(static_cast<int>(format) < 0 || static_cast<int>(format) >= static_cast<int>(TF_Count), ("format arg out of range: value/max %d/%d", static_cast<int>(format), static_cast<int>(TF_Count)-1));
	info[format].supported = supported;
}

// ======================================================================
