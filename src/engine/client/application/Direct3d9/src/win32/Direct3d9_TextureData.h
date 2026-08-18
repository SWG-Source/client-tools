// ======================================================================
// Direct3d9_TextureData.h
// Todd Fiala
//
// copyright 1998 Bootprint Entertainment
// ======================================================================

#ifndef INCLUDED_Direct3d9_TextureData_H
#define INCLUDED_Direct3d9_TextureData_H

// ======================================================================

class MemoryBlockManager;

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"
#include "clientGraphics/Texture.h"
#include <d3d9.h>

// ======================================================================
// Own-impl replacement for D3DXLoadSurfaceFromSurface (Phase 33-03 / D-04a:
// d3dx9 has no x64 static lib, so every D3DX symbol must be removed before
// the x64 link). Point-copies a same-dims rect from src to dst via LockRect.
//
// D3D11-parity bridge (Option A): mirrors the proven, shipped-at-parity
// Direct3d11_TextureData lock-bridge design. Handles ONLY:
//   (a) same-D3DFORMAT copies (straight row-by-row memcpy honoring pitch), and
//   (b) the TF_RGB_888 (D3DFMT_R8G8B8, 24bpp) -> XRGB_8888/ARGB_8888
//       (D3DFMT_X8R8G8B8/A8R8G8B8, 32bpp) byte-order-identical stride
//       expansion (3->4 byte, 0xFF in byte 3).
// Loud-FATALs on every other format pair (D-06 loud-fail trip-wire) and on a
// src/dst rect size mismatch (mirrors Direct3d11_TextureData.cpp:979). The
// FATAL surfaces any unproven runtime pair during the Task-4 A/B rather than
// silently corrupting. srcRect/dstRect NULL == whole surface.
void Direct3d9_ownImplCopySurface(
	IDirect3DSurface9 *dst, RECT const *dstRect,
	IDirect3DSurface9 *src, RECT const *srcRect);

// ======================================================================

class Direct3d9_TextureData: public TextureGraphicsData
{
public:

	static void *operator new(size_t size);
	static void  operator delete(void *pointer);

	static void install(void);
	static void remove(void);

	static void releaseAllGlobalTextures();

	static D3DFORMAT getD3dFormat(TextureFormat textureFormat);

	static bool                                 isGlobalTexture(Tag textureTag);
	static Direct3d9_TextureData const * const *getGlobalTexture(Tag textureTag);
	static void                                 setGlobalTexture(Tag textureTag, const Texture &engineTexture);

	static IDirect3DTexture9 *create2dTexture(int width, int height, int mipmapLevelCount, TextureFormat textureFormat);

public:

	Direct3d9_TextureData(const Texture &newEngineTexture, const TextureFormat *runtimeFormats, int numberOfRuntimeFormats);
	virtual ~Direct3d9_TextureData(void);

	virtual TextureFormat   getNativeFormat() const;
	virtual void            lock(LockData &lockData);
	virtual void            unlock(LockData &lockData);

	virtual void            copyFrom(int surfaceLevel, TextureGraphicsData const & rhs, int srcX, int srcY, int srcWidth, int srcHeight, int dstX, int dstY, int dstWidth, int dstHeight);

	IDirect3DBaseTexture9  *getBaseTexture() const;
	TextureFormat           getTextureFormat() const;

#ifdef _DEBUG
	bool                    firstTimeUsedThisFrame() const;
	int                     getMemorySize() const;
#endif

private:

	/// disabled
	Direct3d9_TextureData(void);

	/// disabled
	Direct3d9_TextureData(const Direct3d9_TextureData&);

	/// disabled
	Direct3d9_TextureData &operator =(const Direct3d9_TextureData&);

private:

	struct PixelFormatInfo
	{
		bool           isSupported;
		D3DFORMAT      pixelFormat;
	};

	struct GlobalTextureInfo
	{
		Texture const *                        engineTexture;
		Direct3d9_TextureData const *          d3dTexture;
	};

private:

	typedef stdmap<Tag, GlobalTextureInfo>::fwd  GlobalTextureList;

	static PixelFormatInfo                   ms_pixelFormatInfoArray[TF_Count];
	static MemoryBlockManager *ms_memoryBlockManager;
	static GlobalTextureList                *ms_globalTextureList;

private:

	const Texture                           &m_engineTexture;
	IDirect3DBaseTexture9                   *m_d3dTexture;
	TextureFormat                            m_destFormat;

#ifdef _DEBUG
	mutable int                              m_lastUsedFrameNumber;
	int                                      m_memorySize;
#endif

	//lint -esym(1737, Direct3d9_TextureData::operator new) // local new hides global new
};

// ======================================================================

inline IDirect3DBaseTexture9 *Direct3d9_TextureData::getBaseTexture() const
{
	return m_d3dTexture;
}

// ----------------------------------------------------------------------

inline TextureFormat Direct3d9_TextureData::getTextureFormat() const
{
	return m_destFormat;
}

// ======================================================================

#endif
