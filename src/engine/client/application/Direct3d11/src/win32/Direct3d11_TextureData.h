// ======================================================================
//
// Direct3d11_TextureData.h
// copyright (c) 2026 Galaxies Reborn
//
// One texture: a 2D image, a cube map, a volume, or a render target. Also the
// global-texture registry, which lives here because it lives here in DX9 and
// because its values are backend texture pointers.
//
// The class name is not a preference. TextureGraphicsData::LockData names
// Direct3d11_TextureData as a friend, and that friendship is the only route to
// writing the pitch and pixel-data fields a lock has to report, so the name and the
// global namespace are both fixed by the engine.
//
// ----------------------------------------------------------------------
// Uploads go through a per-lock scratch buffer, not a mapped resource
//
//   Cube maps are locked and written face by face at runtime while NOT being
//   flagged dynamic, so a DYNAMIC resource -- which can only map its single
//   subresource -- cannot be the general upload path. The first texture the client
//   ever creates is a 128x128 DXT5 cube map with eight mip levels, which is 48
//   (face, level) locks before the first frame.
//
//   The engine's pitch contract is its own: a lock reports the pitch of the format
//   the CALLER asked for, which is frequently not the format the resource is in,
//   and the loader collapses a whole mip level into a single file read when that
//   pitch matches the file's. A tight scratch buffer honours that exactly; a mapped
//   staging row pitch cannot, and would silently take the slow row-by-row branch.
//
// ----------------------------------------------------------------------
// Three formats, not one
//
//   m_nativeFormat   what getNativeFormat() reports. The engine compares a lock's
//                    requested format against this to decide whether it is getting
//                    raw access, and Graphics::setMouseCursor asserts on it.
//
//   m_storageFormat  the engine format whose row layout the resource actually has.
//                    Differs from the native format only for TF_L_8, which is
//                    stored expanded to TF_ARGB_8888 because DXGI has no L8 that
//                    samples as (L, L, L, 1).
//
//   m_dxgiFormat     what the resource was created with.
//
// Any lock whose requested format is not m_storageFormat goes through
// Direct3d11_TextureConverter in whichever direction the lock needs. DX9 did the
// same thing with a scratch surface and D3DXLoadSurfaceFromSurface; there is no
// D3DX in D3D11, so the conversion is ours.
//
// ----------------------------------------------------------------------
// Read-back is tracked, not assumed
//
// A lock that does not discard has to present the subresource's current contents:
// the mouse-cursor alpha hack read-modify-writes its texture, and
// computeRepresentativeColor reads a pixel out of one. But loadSurface ALSO passes
// discardContents = isDynamic(), which is false for every .dds, so a naive
// implementation would stage-copy and stall on every mip level of every texture at
// load time. DX9 pays nothing there, because a MANAGED surface locked without
// D3DLOCK_DISCARD just hands back its system-memory copy.
//
// So this tracks which subresources have ever been written. Contents of one that
// never has are undefined -- exactly as they are for a freshly created D3D9 MANAGED
// surface -- so the read-back is skipped and the scratch is zeroed. Every load
// therefore costs nothing extra, and every genuine read-modify-write is served
// correctly. The read-backs that do happen are counted, so they cannot hide.
//
// Every creation failure is fatal. The prior DX11 attempt fell back to a blank
// B8G8R8A8 texture with the comment that the data may be wrong but the client
// boots, which converts a missing asset into a wrong image with nothing in the log.
//
// ======================================================================

#ifndef INCLUDED_Direct3d11_TextureData_H
#define INCLUDED_Direct3d11_TextureData_H

// ======================================================================

#include "clientGraphics/Texture.h"
#include "sharedFoundation/Tag.h"

#include <d3d11_1.h>

class MemoryBlockManager;

// ======================================================================

class Direct3d11_TextureData : public TextureGraphicsData
{
public:

	static void install();
	static void remove();

	// The global texture registry.
	//
	// getGlobalTexture returns the ADDRESS of the stored backend pointer, not the
	// pointer. A static shader caches that address once when it is built and
	// dereferences it at every draw, which is how a later setGlobalTexture reaches
	// shaders that already exist. Two consequences that are contracts, not details:
	// the container must never relocate its values, and a tag must be registered
	// before the first shader that samples it is constructed.
	static bool                              isGlobalTexture(Tag tag);
	static void                              setGlobalTexture(Tag tag, Texture const &texture);
	static Direct3d11_TextureData const *const *getGlobalTexture(Tag tag);
	static void                              releaseAllGlobalTextures();

	static int  getLiveInstanceCount();

public:

	Direct3d11_TextureData(Texture const &texture, TextureFormat const *runtimeFormats, int numberOfRuntimeFormats);
	virtual ~Direct3d11_TextureData();

	static void *operator new(size_t size);
	static void  operator delete(void *pointer);

	virtual void            copyFrom(int surfaceLevel, TextureGraphicsData const &rhs, int srcX, int srcY, int srcWidth, int srcHeight, int dstX, int dstY, int dstWidth, int dstHeight);

	// Reached through class Direct3d11, which is the friend of Texture.
	ID3D11ShaderResourceView *getShaderResourceView() const;
	// The dimension the shader resource view was created with, which follows from the engine
	// texture's own kind. Free to ask, so a pixel program's declared dimension can be checked
	// against it every time a texture is bound.
	D3D_SRV_DIMENSION       getViewDimension() const;

	// The engine's name for this texture, for diagnostics that have to say WHICH texture.
	char const             *getTextureName() const;
	ID3D11Resource           *getResource() const;
	DXGI_FORMAT               getDxgiFormat() const;
	Texture const            &getEngineTexture() const;

	// A stable, non-truncating identity for sort-by-texture batching. DX9's
	// equivalent casts the D3D pointer straight to int, which on x64 discards the top
	// 32 bits and makes distinct textures compare equal.
	int                       getSortKey() const;

private:

	Direct3d11_TextureData();
	Direct3d11_TextureData(Direct3d11_TextureData const &);
	Direct3d11_TextureData &operator =(Direct3d11_TextureData const &);

	// The three private virtuals. Private on the base too; Texture is its friend.
	virtual TextureFormat   getNativeFormat() const;
	virtual void            lock(LockData &lockData);
	virtual void            unlock(LockData &lockData);

	void                    chooseFormat(TextureFormat const *runtimeFormats, int numberOfRuntimeFormats);
	void                    createResource();
	int                     getSubresource(CubeFace cubeFace, int level) const;

	int                     getSubresourceCount() const;
	void                    getLevelSize(int level, int &width, int &height, int &depth) const;
	bool                    isSubresourceInitialised(int subresource) const;
	void                    markSubresourceInitialised(int subresource);

	// Copy a whole mip level out of the resource into a tight buffer in the STORAGE
	// format. Whole level, not the locked region: a compressed resource can only be
	// copied on block boundaries, and the region a caller locks need not be one --
	// computeRepresentativeColor asks for a single pixel of whatever level is
	// smallest, which for a texture shipped without a mip chain is the full-size
	// level. Returns false when the staging surface could not be produced.
	bool                    stageLevel(int subresource, int levelWidth, int levelHeight, int levelDepth, uint8 *destination, int pitch, int slicePitch);

	// Fill a lock's buffer with the region's current contents, converting from the
	// storage format if the lock asked for a different one.
	void                    readBackRegion(LockData const &lockData, uint8 *destination, int pitch, int slicePitch);

private:

	Texture const          &m_texture;

	TextureFormat           m_nativeFormat;
	TextureFormat           m_storageFormat;
	DXGI_FORMAT             m_dxgiFormat;

	ID3D11Resource           *m_resource;
	ID3D11ShaderResourceView *m_shaderResourceView;

	int                     m_mipmapLevelCount;
	int                     m_faceCount;

	// One bit per subresource, so a non-discarding lock knows whether there is
	// anything to read back. Sized from the actual subresource count rather than
	// capped at a word: a cube map's count is six times its level count, so a
	// 1024-square cube map already needs 66 bits and any fixed word would be a
	// silent correctness cliff at some texture size.
	uint8                  *m_initialisedSubresources;

	static MemoryBlockManager *ms_memoryBlockManager;
};

// ======================================================================

inline Texture const &Direct3d11_TextureData::getEngineTexture() const
{
	return m_texture;
}

// ----------------------------------------------------------------------

inline ID3D11ShaderResourceView *Direct3d11_TextureData::getShaderResourceView() const
{
	return m_shaderResourceView;
}

// ----------------------------------------------------------------------

inline ID3D11Resource *Direct3d11_TextureData::getResource() const
{
	return m_resource;
}

// ----------------------------------------------------------------------

inline DXGI_FORMAT Direct3d11_TextureData::getDxgiFormat() const
{
	return m_dxgiFormat;
}

// ======================================================================

#endif
