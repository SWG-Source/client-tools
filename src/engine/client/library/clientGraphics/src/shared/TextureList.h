// ======================================================================
//
// TextureList.h
// Portions copyright 1998, 1999 Bootprint Entertainment
// Portions copyright 2000-2002 Sony Online Entertainment
// All Rights Reserved
//
// ======================================================================

#ifndef INCLUDED_TextureList_H
#define INCLUDED_TextureList_H

// ======================================================================

class CrcString;
class Iff;
class Texture;

#include "clientGraphics/Texture.def"

// ======================================================================

class TextureList
{
	friend class Texture;

public:

	static void install(void);
	static void assignAsynchronousLoaderFunctions();

	static void reloadTextures();

	static void setBuildingAsynchronousLoaderData(bool buildingAsynchronousLoaderData);

	static const char    *getDefaultTextureName();
	static const Texture *fetchDefaultTexture();

	static char const * getDefaultNormalTextureName();
	static Texture const * fetchDefaultNormalTexture();

	static const char    *getDefaultEnvironmentTextureName();
	static const Texture *fetchDefaultEnvironmentTexture();

	static const Texture *fetchDefaultWhiteTexture();
	static const Texture *fetchDefaultBlackTexture();

	// texture list reading functionality
	static const Texture *fetch(CrcString const &fileName);
	static const Texture *fetch(const char *filename);
	static const Texture *fetch(Iff *iff);
	static const Texture *fetch(int alpha, int red, int green, int blue);
	static const Texture *fetch(const void *pixelData, TextureFormat format, int width, int height, const TextureFormat *runtimeFormats, int runtimeFormatCount);
	static Texture       *fetch(int textureCreationFlags, int width, int height, int numberOfMipMapLevels, const TextureFormat *runtimeTextureFormats, int textureFormatCount);

	//-- These functions are called at the beginning of TextureList::remove to clean up any existing texture resources
	typedef void (*CleanupCallbackFunction)();
	static void addCleanupCallbackFunction(CleanupCallbackFunction cleanupCallbackFunction);

private:

	static void           remove();
	static void           removeFromList(const Texture *texture);
	static const Texture *fetch(CrcString const & fileName, bool createTexture);
	static const void    *asynchronousLoaderFetchNoCreate(const char *fileName);
	static void           asynchronousLoaderRelease(const void *texture);
	static Texture const * create(CrcString const & filename, bool createTexture);
};

// ======================================================================

#endif
