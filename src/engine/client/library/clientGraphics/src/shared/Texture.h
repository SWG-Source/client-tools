// ======================================================================
//
// Texture.h
//
// copyright 1998, 1999 Bootprint Entertainment
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef TEXTURE_H
#define TEXTURE_H

// ======================================================================

class Iff;
class AbstractFile;

#include "sharedFoundation/PersistentCrcString.h"
#include "clientGraphics/Texture.def"
#include "sharedMath/PackedArgb.h"

// ======================================================================

class TextureGraphicsData
{
	friend class Texture;

public:

	class LockData
	{
		friend class Texture;
		friend class Direct3d8_TextureData;
		friend class Direct3d9_TextureData;

	public:

		LockData(TextureFormat format, int level, int x, int y, int width, int height, bool discardContents);
		LockData(TextureFormat format, int level, int x, int y, int z, int width, int height, int depth, bool discardContents);
		LockData(TextureFormat format, CubeFace cubeFace, int level, int x, int y, int z, int width, int height, int depth, bool discardContents);


		TextureFormat  getFormat() const;
		int            getLevel() const;
		int            getX() const;
		int            getY() const;
		int            getZ() const;
		int            getWidth() const;
		int            getHeight() const;
		int            getDepth() const;
		bool           isReadOnly() const;
		bool           shouldDiscardContents() const;

		int            getPitch() const;
		int            getSlicePitch() const;
		void          *getPixelData();

	private:

		/// Disabled.
		LockData();

		/// Disabled.
		LockData(const LockData &);

		/// Disabled.
		LockData &operator =(const LockData &);

	private:

		TextureFormat  m_format;
		CubeFace       m_cubeFace;
		int            m_level;
		int            m_x;
		int            m_y;
		int            m_z;
		int            m_width;
		int            m_height;
		int            m_depth;
		bool           m_discardContents;
		bool           m_readOnly;

		int            m_pitch;
		int            m_slicePitch;
		void          *m_pixelData;
		void          *m_reserved;
	};

public:

	virtual DLLEXPORT  ~TextureGraphicsData();
	virtual void   copyFrom(int surfaceLevel, TextureGraphicsData const & rhs, int srcX, int srcY, int srcWidth, int srcHeight, int dstX, int dstY, int dstWidth, int dstHeight) = 0;

private:

	virtual TextureFormat getNativeFormat() const = 0;
	virtual void        lock(LockData &lockData) = 0;
	virtual void        unlock(LockData &lockData) = 0;
};

// ----------------------------------------------------------------------

class Texture
{
	friend class Direct3d8;
	friend class Direct3d8_RenderTarget;
	friend class Direct3d9;
	friend class Direct3d9_RenderTarget;
	friend class TextureList;
	friend class Graphics;

public:

	typedef TextureGraphicsData::LockData LockData;

private:

	mutable int                              m_referenceCount;
	PersistentCrcString                      m_crcString;

	bool                                     m_renderTarget;
	bool                                     m_cube;
	bool                                     m_volume;
	bool                                     m_dynamic;
	bool                                     m_cursorHack;
	int                                      m_width;
	int                                      m_height;
	int                                      m_depth;
	int                                      m_mipmapLevelCount;
	TextureGraphicsData                     *m_graphicsData;

	mutable uint8                            m_representativeColorComputed;
	mutable PackedArgb                       m_representativeColor;

private:

	// disabled
	Texture(void);
	Texture(const Texture&);
	Texture &operator =(const Texture&);

private:

	static void           remove(void);

	static void          *operator new(size_t size);
	static void           operator delete(void *pointer);

	explicit Texture(CrcString const &fileName);
	Texture(int newAlpha, int newRed, int newGreen, int newBlue);
	Texture(const void *pixelData, TextureFormat sourceFormat, int width, int height, int depth, const TextureFormat *runtimeFormats, int numberOfRuntimeFormats);
	Texture(int textureCreationFlags, int width, int height, int numberOfMipMapLevels, const TextureFormat *runtimeTextureFormats, int textureFormatCount);

	void                  loadSurface(TextureFormat format, CubeFace face, AbstractFile *fileInterface, int numberOfHighestMipmapLevelsToDiscard, int numberOfLowestMipmapLevelsToDiscard);
	void                  load(const char * fileName);

	~Texture(void);

	PackedArgb            computeRepresentativeColor() const;

public:

	static void                     install(void);

public:

	void DLLEXPORT        fetch() const;
	void DLLEXPORT        release() const;

	const CrcString      &getCrcString(void) const;
	const char           *getName(void) const;

	bool                  isRenderTarget() const;
	bool                  isCubeMap() const;
	bool                  isVolumeMap() const;
	bool                  isDynamic() const;
	int                   getWidth(void) const;
	int                   getHeight(void) const;
	int                   getDepth(void) const;
	int                   getMipmapLevelCount(void) const;
	TextureFormat         getNativeFormat() const;

	void                  lock(LockData &lockData);
	void                  lockReadOnly(LockData &lockData) const;
	void                  unlock(LockData &lockData) const;

	int                   getReferenceCount() const;

	void				  copyPixels(const void *pixelData, TextureFormat sourceFormat, int width, int height);

	TextureGraphicsData  const *         getGraphicsData() const;
	TextureGraphicsData  const * const * getGraphicsDataAddress() const;

	PackedArgb                           getRepresentativeColor() const;

	void   copyFrom(int surfaceLevel, Texture const & rhs, int srcX, int srcY, int srcWidth, int srcHeight, int dstX, int dstY, int dstWidth, int dstHeight);

	//lint -esym(1737, Texture::operator new) // Texture::operator new hides global operator new
};

// ======================================================================

inline TextureGraphicsData::LockData::LockData(TextureFormat format, int level, int x, int y, int width, int height, bool discardContents)
: //lint !e578 // Declaration of 'width' hides 'Texture::width'
	m_format(format),
	m_cubeFace(CF_none),
	m_level(level),
	m_x(x),
	m_y(y),
	m_z(0),
	m_width(width),
	m_height(height),
	m_depth(1),
	m_discardContents(discardContents),
	m_readOnly(false),
	m_pitch(0),
	m_slicePitch(0),
	m_pixelData(0),
	m_reserved(NULL)
{
}

// ----------------------------------------------------------------------

inline TextureGraphicsData::LockData::LockData(TextureFormat format, int level, int x, int y, int z, int width, int height, int depth, bool discardContents)
: //lint !e578 // Declaration of 'width' hides 'Texture::width'
	m_format(format),
	m_cubeFace(CF_none),
	m_level(level),
	m_x(x),
	m_y(y),
	m_z(z),
	m_width(width),
	m_height(height),
	m_depth(depth),
	m_discardContents(discardContents),
	m_readOnly(false),
	m_pitch(0),
	m_slicePitch(0),
	m_pixelData(0),
	m_reserved(NULL)
{
}

// ----------------------------------------------------------------------

inline TextureGraphicsData::LockData::LockData(TextureFormat format, CubeFace cubeFace, int level, int x, int y, int z, int width, int height, int depth, bool discardContents)
: //lint !e578 // Declaration of 'width' hides 'Texture::width'
	m_format(format),
	m_cubeFace(cubeFace),
	m_level(level),
	m_x(x),
	m_y(y),
	m_z(z),
	m_width(width),
	m_height(height),
	m_depth(depth),
	m_discardContents(discardContents),
	m_readOnly(false),
	m_pitch(0),
	m_slicePitch(0),
	m_pixelData(0),
	m_reserved(NULL)
{
}

// ----------------------------------------------------------------------

inline TextureFormat Texture::LockData::getFormat() const
{
	return m_format;
}

// ----------------------------------------------------------------------

inline int Texture::LockData::getLevel() const
{
	return m_level;
}

// ----------------------------------------------------------------------

inline int Texture::LockData::getX() const
{
	return m_x;
}

// ----------------------------------------------------------------------

inline int Texture::LockData::getY() const
{
	return m_y;
}

// ----------------------------------------------------------------------

inline int Texture::LockData::getZ() const
{
	return m_z;
}

// ----------------------------------------------------------------------

inline int Texture::LockData::getWidth() const
{
	return m_width;
}

// ----------------------------------------------------------------------

inline int Texture::LockData::getHeight() const
{
	return m_height;
}

// ----------------------------------------------------------------------

inline int Texture::LockData::getDepth() const
{
	return m_depth;
}

// ----------------------------------------------------------------------

inline bool Texture::LockData::isReadOnly() const
{
	return m_readOnly;
}

// ----------------------------------------------------------------------

inline bool Texture::LockData::shouldDiscardContents() const
{
	return m_discardContents;
}

// ----------------------------------------------------------------------

inline int Texture::LockData::getPitch() const
{
	DEBUG_FATAL(!m_pixelData, ("not locked"));
	return m_pitch;
}

// ----------------------------------------------------------------------

inline int Texture::LockData::getSlicePitch() const
{
	DEBUG_FATAL(!m_pixelData, ("not locked"));
	return m_slicePitch;
}

// ----------------------------------------------------------------------

inline void *Texture::LockData::getPixelData()
{
	DEBUG_FATAL(!m_pixelData, ("not locked"));
	return m_pixelData;
}

// ======================================================================

inline bool Texture::isRenderTarget() const
{
	return m_renderTarget;
}

// ----------------------------------------------------------------------
/**
 * Check whether this texture is a cube map or not.
 * @return true if the texture is a cube map, otherwise false.
 */

inline bool Texture::isCubeMap() const
{
	return m_cube;
}

// ----------------------------------------------------------------------

inline bool Texture::isVolumeMap() const
{
	return m_volume;
}

// ----------------------------------------------------------------------

inline bool Texture::isDynamic() const
{
	return m_dynamic;
}

// ----------------------------------------------------------------------

inline int Texture::getWidth(void) const
{
	return m_width;
}

// ----------------------------------------------------------------------

inline int Texture::getHeight(void) const
{
	return m_height;
}

// ----------------------------------------------------------------------

inline int Texture::getDepth(void) const
{
	return m_depth;
}

// ----------------------------------------------------------------------

inline int Texture::getMipmapLevelCount(void) const
{
	return m_mipmapLevelCount;
}

// ----------------------------------------------------------------------
/**
 * Return a read-only version of the texture's name.
 * 
 * A texture may not have a name, in which case the returned value
 * will be NULL.
 * 
 * @return A non-null CrcString pointer if the texture was named, otherwise null.
 */

inline const CrcString &Texture::getCrcString(void) const
{
	return m_crcString;
}

// ----------------------------------------------------------------------

inline const char *Texture::getName(void) const
{
	return m_crcString.getString ();
}

// ----------------------------------------------------------------------

inline int Texture::getReferenceCount() const
{
	return m_referenceCount;
}

// ----------------------------------------------------------------------

inline TextureGraphicsData  const * Texture::getGraphicsData() const
{
	return m_graphicsData;
}

// ----------------------------------------------------------------------

inline TextureGraphicsData  const * const * Texture::getGraphicsDataAddress() const
{
	return &m_graphicsData;
}

// ======================================================================

#endif
