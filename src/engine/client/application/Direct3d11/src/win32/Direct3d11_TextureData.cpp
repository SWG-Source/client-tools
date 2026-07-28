// ======================================================================
//
// Direct3d11_TextureData.cpp
// copyright (c) 2026 Galaxies Reborn
//
// ======================================================================

#include "FirstDirect3d11.h"
#include "Direct3d11_TextureData.h"

#include "Direct3d11_Device.h"
#include "Direct3d11_Metrics.h"
#include "Direct3d11_RenderTarget.h"
#include "Direct3d11_StateCache.h"
#include "Direct3d11_TextureConverter.h"
#include "Direct3d11_TextureFormatMap.h"

#include "clientGraphics/TextureFormatInfo.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/Os.h"

#include <map>

// ======================================================================

MemoryBlockManager *Direct3d11_TextureData::ms_memoryBlockManager;

// ======================================================================

namespace Direct3d11_TextureDataNamespace
{
	Tag const TAG_ENVM = TAG(E,N,V,M);

	// The registry's value. Both halves are needed: the engine texture owns the
	// reference count, the backend texture is what gets bound.
	struct GlobalTextureInfo
	{
		Texture const                *engineTexture;
		Direct3d11_TextureData const *backendTexture;
	};

	// std::map, and the node stability is the contract rather than a preference --
	// getGlobalTexture hands out the address of the backendTexture field and shaders
	// keep it. Any container that relocates its values (a vector, a flat map, a
	// value-storing hash map that rehashes) would dangle every cached pointer the
	// first time it grew.
	typedef std::map<Tag, GlobalTextureInfo> GlobalTextureList;
	GlobalTextureList *ms_globalTextureList;

	int ms_liveInstanceCount;

	// Said once. A partial write to a compressed level that is not block aligned is refused by the
	// runtime, so it is a texel-correctness problem worth one line rather than one per texture.
	bool ms_reportedUnalignedBlockWrite;

	// ----------------------------------------------------------------------

	char const *describeTexture(Texture const &texture)
	{
		char const * const name = texture.getName();
		return (name && *name) ? name : "<unnamed>";
	}

	char const *describeFormat(TextureFormat format)
	{
		if (static_cast<int>(format) < 0 || static_cast<int>(format) >= static_cast<int>(TF_Count))
			return "<out of range>";

		char const * const name = TextureFormatInfo::getInfo(format).name;
		return name ? name : "<unnamed>";
	}

	// Tight row and slice strides for a region, in the format the CALLER asked for.
	// The engine's loader collapses a level into a single read when the pitch it is
	// given matches the file's, so these have to describe a contiguous block.
	void computeTightPitches(TextureFormat format, int width, int height, int &pitch, int &rows)
	{
		TextureFormatInfo const &info = TextureFormatInfo::getInfo(format);

		if (info.compressed)
		{
			pitch = (width + info.blockWidth - 1) / info.blockWidth * info.blockSize;
			rows  = (height + info.blockHeight - 1) / info.blockHeight;
		}
		else
		{
			pitch = width * info.pixelByteCount;
			rows  = height;
		}
	}
}
using namespace Direct3d11_TextureDataNamespace;

// ======================================================================

void Direct3d11_TextureData::install()
{
	DEBUG_FATAL(ms_memoryBlockManager, ("Direct3d11_TextureData already installed"));

	ms_memoryBlockManager = new MemoryBlockManager("Direct3d11_TextureData::ms_memoryBlockManager", true, sizeof(Direct3d11_TextureData), 0, 0, 0);
	ms_globalTextureList = new GlobalTextureList;

	// Publishes TextureFormatInfo::supported for all 17 formats. Nothing else in the
	// engine ever writes that flag, and every entry starts false, so without this the
	// first createTextureData finds no usable format at all.
	Direct3d11_TextureFormatMap::install();
}

// ----------------------------------------------------------------------
/**
 * Tear down, in the one order that is safe.
 *
 * The list is drained BEFORE the block manager dies. Draining releases engine
 * textures, the last release of one deletes it, deleting it deletes its graphics data,
 * and that runs this class's operator delete -- which needs the manager. DX9 does
 * these two in the opposite order and gets away with it only because ExitChain
 * ordering guarantees the list is already empty by then. Relying on that is a trap for
 * whoever next changes an install order, so this does not.
 */

void Direct3d11_TextureData::remove()
{
	if (ExitChain::isFataling())
		return;

	if (ms_globalTextureList)
	{
		releaseAllGlobalTextures();
		delete ms_globalTextureList;
		ms_globalTextureList = NULL;
	}

	DEBUG_WARNING(ms_liveInstanceCount != 0, ("Direct3d11: %d texture(s) are still alive at shutdown.", ms_liveInstanceCount));

	if (ms_memoryBlockManager)
	{
		delete ms_memoryBlockManager;
		ms_memoryBlockManager = NULL;
	}
}

// ----------------------------------------------------------------------

int Direct3d11_TextureData::getLiveInstanceCount()
{
	return ms_liveInstanceCount;
}

// ======================================================================

void *Direct3d11_TextureData::operator new(size_t size)
{
	NOT_NULL(ms_memoryBlockManager);
	DEBUG_FATAL(size != sizeof(Direct3d11_TextureData), ("Direct3d11: texture data allocation of %d bytes, expected %d.", static_cast<int>(size), static_cast<int>(sizeof(Direct3d11_TextureData))));
	DEBUG_FATAL(size != static_cast<size_t>(ms_memoryBlockManager->getElementSize()), ("Direct3d11: the texture data block manager was installed with the wrong element size."));
	UNREF(size);

	++ms_liveInstanceCount;
	return ms_memoryBlockManager->allocate();
}

// ----------------------------------------------------------------------

void Direct3d11_TextureData::operator delete(void *pointer)
{
	NOT_NULL(ms_memoryBlockManager);

	--ms_liveInstanceCount;
	ms_memoryBlockManager->free(pointer);
}

// ======================================================================
/**
 * Is this tag one the registry answers for?
 *
 * Either the exact tag ENVM, or any tag whose first character is an underscore. Tag
 * packs its four characters most significant byte first, so the top byte is character
 * zero. In the shipped shader corpus exactly two tags satisfy this: ENVM and _SKY.
 */

bool Direct3d11_TextureData::isGlobalTexture(Tag tag)
{
	return (tag == TAG_ENVM) || (((tag >> 24) & 0xff) == '_');
}

// ----------------------------------------------------------------------
/**
 * Register or re-point a global texture.
 *
 * The new reference is taken BEFORE the old one is dropped. That ordering is what
 * makes re-pointing a tag at a texture whose only other reference is this registry
 * safe -- reversed, the release would destroy the texture the fetch was about to
 * claim. SetupClientGraphics leans on the fetch existing at all: it registers the
 * default cube map under two tags and then drops its own reference immediately.
 *
 * An existing node is updated in place and never erased and reinserted, because
 * getGlobalTexture has already handed its address to every shader built since.
 */

void Direct3d11_TextureData::setGlobalTexture(Tag tag, Texture const &texture)
{
	char tagName[5];
	ConvertTagToString(tag, tagName);

	FATAL(!ms_globalTextureList, ("Direct3d11: global texture '%s' was set before the texture subsystem was installed.", tagName));
	FATAL(!isGlobalTexture(tag), ("Direct3d11: tag '%s' is not a global texture tag.", tagName));

	GlobalTextureList::iterator i = ms_globalTextureList->find(tag);

	if (i != ms_globalTextureList->end())
	{
		// Setting a tag to what it already holds is a no-op, deliberately: the backend
		// pointer is unchanged, and re-fetching would leak a reference.
		if (i->second.engineTexture == &texture)
			return;

		texture.fetch();
		i->second.engineTexture->release();
		i->second.engineTexture = &texture;
		i->second.backendTexture = static_cast<Direct3d11_TextureData const *>(texture.getGraphicsData());
		return;
	}

	GlobalTextureInfo info;
	info.engineTexture = &texture;
	info.backendTexture = static_cast<Direct3d11_TextureData const *>(texture.getGraphicsData());

	bool const inserted = ms_globalTextureList->insert(GlobalTextureList::value_type(tag, info)).second;
	FATAL(!inserted, ("Direct3d11: global texture '%s' was already present after a failed lookup.", tagName));
	UNREF(inserted);

	texture.fetch();
}

// ----------------------------------------------------------------------
/**
 * The address of a tag's backend texture pointer.
 *
 * Fatal on a tag that has not been registered, and that is the useful behaviour: the
 * only alternative is a pointer-to-pointer the caller then has to test on every single
 * draw, and the actual bug this catches -- a shader sampling a global tag that install
 * never registered -- is one nobody would ever find from a black texture.
 */

Direct3d11_TextureData const *const *Direct3d11_TextureData::getGlobalTexture(Tag tag)
{
	char tagName[5];
	ConvertTagToString(tag, tagName);

	FATAL(!ms_globalTextureList, ("Direct3d11: global texture '%s' was requested before the texture subsystem was installed.", tagName));
	FATAL(!isGlobalTexture(tag), ("Direct3d11: tag '%s' is not a global texture tag.", tagName));

	GlobalTextureList::const_iterator i = ms_globalTextureList->find(tag);
	FATAL(i == ms_globalTextureList->end(), ("Direct3d11: global texture '%s' has not been registered. It must be set before the first shader that samples it is built.", tagName));

	return &i->second.backendTexture;
}

// ----------------------------------------------------------------------
/**
 * Drop every reference the registry holds.
 *
 * The node is erased before its texture is released, and backendTexture is never
 * read: the release that takes a texture to zero deletes its graphics data, so
 * touching the node afterwards would be a use-after-free.
 */

void Direct3d11_TextureData::releaseAllGlobalTextures()
{
	if (!ms_globalTextureList)
		return;

	while (!ms_globalTextureList->empty())
	{
		GlobalTextureList::iterator i = ms_globalTextureList->begin();
		Texture const * const engineTexture = i->second.engineTexture;
		ms_globalTextureList->erase(i);

		if (engineTexture)
			engineTexture->release();
	}
}

// ======================================================================

Direct3d11_TextureData::Direct3d11_TextureData(Texture const &texture, TextureFormat const *runtimeFormats, int numberOfRuntimeFormats)
:
	TextureGraphicsData(),
	m_texture(texture),
	m_nativeFormat(TF_ARGB_8888),
	m_storageFormat(TF_ARGB_8888),
	m_dxgiFormat(DXGI_FORMAT_UNKNOWN),
	m_resource(NULL),
	m_shaderResourceView(NULL),
	m_mipmapLevelCount(texture.getMipmapLevelCount()),
	m_faceCount(texture.isCubeMap() ? 6 : 1),
	m_initialisedSubresources(NULL)
{
	// Kept from DX9. D3D11 resource creation is free-threaded, but this runs with the
	// texture list's own state in flux and the engine assumes the main thread.
	DEBUG_FATAL(!Os::isMainThread(), ("Direct3d11: a texture was created from a thread other than the main thread."));

	chooseFormat(runtimeFormats, numberOfRuntimeFormats);
	createResource();

	int const bytes = (getSubresourceCount() + 7) / 8;
	m_initialisedSubresources = new uint8[bytes];
	memset(m_initialisedSubresources, 0, static_cast<size_t>(bytes));
}

// ----------------------------------------------------------------------

Direct3d11_TextureData::~Direct3d11_TextureData()
{
	// Unbind before releasing. A view left in the state cache's shadow would be
	// compared against a future texture the allocator happens to place at the same
	// address, and that bind would then be skipped as redundant.
	if (m_shaderResourceView)
	{
		Direct3d11_StateCache::destroyShaderResource(m_shaderResourceView);

	// And any render target view onto this resource, for the same reason: a cached view
	// outliving its resource is a dangling pointer that the next allocation makes look valid.
	Direct3d11_RenderTarget::releaseViewsFor(m_resource);
		m_shaderResourceView->Release();
		m_shaderResourceView = NULL;
	}

	if (m_resource)
	{
		m_resource->Release();
		m_resource = NULL;
	}

	delete [] m_initialisedSubresources;
	m_initialisedSubresources = NULL;
}

// ======================================================================
/**
 * Pick the first format in the caller's preference list the device supports.
 *
 * The list is ordered by the engine's preference and normally terminates in something
 * universally available, so the first supported entry wins. A list with nothing
 * supported in it is fatal and names every format tried -- there is no honest
 * substitute, and choosing a format other than the one getNativeFormat then reports
 * would make every subsequent lock convert in the wrong direction.
 */

void Direct3d11_TextureData::chooseFormat(TextureFormat const *runtimeFormats, int numberOfRuntimeFormats)
{
	NOT_NULL(runtimeFormats);

	for (int i = 0; i < numberOfRuntimeFormats; ++i)
	{
		TextureFormat const candidate = runtimeFormats[i];

		if (TextureFormatInfo::getInfo(candidate).supported)
		{
			m_nativeFormat = candidate;
			m_storageFormat = Direct3d11_TextureFormatMap::getStorageFormat(candidate);
			m_dxgiFormat = Direct3d11_TextureFormatMap::getDxgiFormat(candidate);
			return;
		}
	}

	// Name every format tried. The single-entry lists are the interesting failures:
	// TF_A_8, TF_L_8 and TF_P_8 have no fallback at all, so for those this message is
	// the whole diagnosis.
	char formats[512];
	formats[0] = 0;
	for (int i = 0; i < numberOfRuntimeFormats; ++i)
	{
		if (strlen(formats) + 20 >= sizeof(formats))
			break;

		if (i)
			strcat(formats, ", ");
		strcat(formats, describeFormat(runtimeFormats[i]));
	}

	FATAL(true, ("Direct3d11: texture '%s' asked for one of [%s] and this device supports none of them.", describeTexture(m_texture), formats));
}

// ----------------------------------------------------------------------
/**
 * Create the resource and its shader view.
 *
 * Dimensions and mip count are used exactly as given. No rounding to a power of two,
 * no clamping, and never a mip count of zero to mean "generate a chain": short chains
 * are legal in the asset set, the engine only warns about them, and generating the
 * missing tail would both differ from DX9 and be impossible for the block-compressed
 * formats most of these are.
 *
 * USAGE_DEFAULT throughout, including for the textures the engine flags dynamic. A
 * DYNAMIC resource can only map its single subresource, which rules it out for cube
 * maps and for anything with mip levels -- and having one upload path rather than two
 * is worth more than the marginal gain on the few textures that would qualify.
 */

void Direct3d11_TextureData::createResource()
{
	Direct3d11_Metrics::ScopedTimer timer(Direct3d11_Metrics::textureCreateTicks);

	ID3D11Device1 * const device = Direct3d11_Device::getDevice();
	NOT_NULL(device);

	FATAL(m_dxgiFormat == DXGI_FORMAT_UNKNOWN, ("Direct3d11: texture '%s' chose engine format %s, which has no DXGI representation. The format support sweep and the format map disagree.", describeTexture(m_texture), describeFormat(m_nativeFormat)));

	int const width  = m_texture.getWidth();
	int const height = m_texture.getHeight();

	FATAL(width <= 0 || height <= 0 || m_mipmapLevelCount <= 0, ("Direct3d11: texture '%s' is %dx%d with %d mip level(s).", describeTexture(m_texture), width, height, m_mipmapLevelCount));

	HRESULT hresult = S_OK;

	if (m_texture.isVolumeMap())
	{
		D3D11_TEXTURE3D_DESC description;
		Zero(description);
		description.Width     = static_cast<UINT>(width);
		description.Height    = static_cast<UINT>(height);
		description.Depth     = static_cast<UINT>(m_texture.getDepth());
		description.MipLevels = static_cast<UINT>(m_mipmapLevelCount);
		description.Format    = m_dxgiFormat;
		description.Usage     = D3D11_USAGE_DEFAULT;
		description.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		ID3D11Texture3D *volume = NULL;
		hresult = device->CreateTexture3D(&description, NULL, &volume);
		m_resource = volume;
	}
	else
	{
		D3D11_TEXTURE2D_DESC description;
		Zero(description);
		description.Width     = static_cast<UINT>(width);
		description.Height    = static_cast<UINT>(height);
		description.MipLevels = static_cast<UINT>(m_mipmapLevelCount);

		// Six array slices in the DXGI cube ordering, which is the same order as the
		// engine's CubeFace enumeration: +X, -X, +Y, -Y, +Z, -Z.
		description.ArraySize = static_cast<UINT>(m_faceCount);
		description.Format    = m_dxgiFormat;
		description.SampleDesc.Count = 1;
		description.Usage     = D3D11_USAGE_DEFAULT;
		description.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		// A render target in this engine is always sampled as well -- that is the
		// entire point of every one of them -- so both bind flags, always.
		if (m_texture.isRenderTarget())
			description.BindFlags |= D3D11_BIND_RENDER_TARGET;

		if (m_texture.isCubeMap())
		{
			DEBUG_WARNING(width != height, ("Direct3d11: cube map '%s' is %dx%d, and cube maps must be square.", describeTexture(m_texture), width, height));
			description.MiscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;
		}

		ID3D11Texture2D *plane = NULL;
		hresult = device->CreateTexture2D(&description, NULL, &plane);
		m_resource = plane;
	}

	FATAL(FAILED(hresult) || !m_resource, ("Direct3d11: texture '%s' (%dx%d, %d mip(s), %s%s%s%s) could not be created (%s).",
		describeTexture(m_texture), width, height, m_mipmapLevelCount,
		describeFormat(m_nativeFormat),
		m_texture.isCubeMap() ? ", cube" : "",
		m_texture.isVolumeMap() ? ", volume" : "",
		m_texture.isRenderTarget() ? ", render target" : "",
		Direct3d11_Device::describeHresult(hresult)));

	++Direct3d11_Metrics::textureCreations;

	// Named so that a capture says which texture this is. Without it every texture in a RenderDoc
	// capture is a number, and "which texture is bound where the skin renders green" becomes a
	// correlation exercise instead of a glance.
	Direct3d11_Device::setDebugNameFormatted(m_resource, "%s (%s%s%s)",
		describeTexture(m_texture),
		describeFormat(m_nativeFormat),
		m_texture.isCubeMap() ? ", cube" : "",
		m_texture.isRenderTarget() ? ", render target" : "");

	// A default view covers exactly the levels and slices that exist, and for a
	// resource created with MISC_TEXTURECUBE it is a cube view rather than an array
	// view. Both are what we want, so the description is left NULL.
	hresult = device->CreateShaderResourceView(m_resource, NULL, &m_shaderResourceView);

	Direct3d11_Device::setDebugNameFormatted(m_shaderResourceView, "%s view", describeTexture(m_texture));
	FATAL(FAILED(hresult) || !m_shaderResourceView, ("Direct3d11: the shader view for texture '%s' could not be created (%s).", describeTexture(m_texture), Direct3d11_Device::describeHresult(hresult)));
}

// ======================================================================

int Direct3d11_TextureData::getSubresourceCount() const
{
	return m_faceCount * m_mipmapLevelCount;
}

// ----------------------------------------------------------------------

void Direct3d11_TextureData::getLevelSize(int level, int &width, int &height, int &depth) const
{
	width  = m_texture.getWidth() >> level;
	height = m_texture.getHeight() >> level;
	depth  = m_texture.getDepth() >> level;

	// Halving stops at one, which is how the engine's own loader computes it.
	if (width < 1)
		width = 1;
	if (height < 1)
		height = 1;
	if (depth < 1)
		depth = 1;
}

// ----------------------------------------------------------------------

int Direct3d11_TextureData::getSubresource(CubeFace cubeFace, int level) const
{
	DEBUG_FATAL(level < 0 || level >= m_mipmapLevelCount, ("Direct3d11: mip level %d is out of range 0..%d for texture '%s'.", level, m_mipmapLevelCount - 1, describeTexture(m_texture)));

	if (m_faceCount == 1)
		return level;

	// CF_none reaches here for a cube map from the callers that use the 2D lock
	// constructor -- computing a representative colour, for one. DX9 passes it straight
	// to D3D and gets an error; face zero is the more useful answer, and is what those
	// callers are actually asking for.
	int face = static_cast<int>(cubeFace);
	if (face < 0 || face > 5)
		face = 0;

	return (face * m_mipmapLevelCount) + level;
}

// ----------------------------------------------------------------------

bool Direct3d11_TextureData::isSubresourceInitialised(int subresource) const
{
	if (!m_initialisedSubresources || subresource < 0 || subresource >= getSubresourceCount())
		return false;

	return (m_initialisedSubresources[subresource / 8] & (1 << (subresource & 7))) != 0;
}

// ----------------------------------------------------------------------

void Direct3d11_TextureData::markSubresourceInitialised(int subresource)
{
	if (!m_initialisedSubresources || subresource < 0 || subresource >= getSubresourceCount())
		return;

	m_initialisedSubresources[subresource / 8] = static_cast<uint8>(m_initialisedSubresources[subresource / 8] | (1 << (subresource & 7)));
}

// ======================================================================

TextureFormat Direct3d11_TextureData::getNativeFormat() const
{
	return m_nativeFormat;
}

// ----------------------------------------------------------------------

int Direct3d11_TextureData::getSortKey() const
{
	// Fold the whole pointer into 31 bits. DX9 casts it straight to int and loses the
	// top half on x64, which makes two distinct textures sort as one whenever they
	// differ only above bit 31 -- a batching defect that costs draw calls silently.
	uint64 const value = reinterpret_cast<uint64>(m_resource);
	uint32 const folded = static_cast<uint32>(value ^ (value >> 32));

	return static_cast<int>(folded & 0x7fffffff);
}

// ======================================================================
/**
 * Copy one whole mip level out of the resource, in the storage format.
 *
 * A whole level, never the locked region, and that is not laziness. A copy out of a
 * block-compressed resource has to start and end on block boundaries, and the region a
 * caller locks need not: computeRepresentativeColor asks for a single pixel of whatever
 * level is smallest, which for one of the textures shipped WITHOUT a mip chain is the
 * full-size level. Staging the level and slicing on the CPU is correct for every case
 * and needs no alignment reasoning at all.
 *
 * This stalls -- it copies on the GPU timeline and then maps without DO_NOT_WAIT -- so it
 * is counted. It only ever runs for a level that has actually been written and is being
 * locked without discard: the mouse cursor's alpha pass, a representative-colour read,
 * the ground environment strip.
 */

bool Direct3d11_TextureData::stageLevel(int subresource, int levelWidth, int levelHeight, int levelDepth, uint8 *destination, int pitch, int slicePitch)
{
	ID3D11Device1 * const device = Direct3d11_Device::getDevice();
	ID3D11DeviceContext1 * const context = Direct3d11_Device::getContext();
	if (!device || !context || !m_resource)
		return false;

	ID3D11Resource *readback = NULL;
	HRESULT hresult = E_FAIL;

	if (m_texture.isVolumeMap())
	{
		D3D11_TEXTURE3D_DESC staging;
		Zero(staging);
		staging.Width          = static_cast<UINT>(levelWidth);
		staging.Height         = static_cast<UINT>(levelHeight);
		staging.Depth          = static_cast<UINT>(levelDepth);
		staging.MipLevels      = 1;
		staging.Format         = m_dxgiFormat;
		staging.Usage          = D3D11_USAGE_STAGING;
		staging.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

		ID3D11Texture3D *volume = NULL;
		hresult = device->CreateTexture3D(&staging, NULL, &volume);
		readback = volume;
	}
	else
	{
		D3D11_TEXTURE2D_DESC staging;
		Zero(staging);
		staging.Width          = static_cast<UINT>(levelWidth);
		staging.Height         = static_cast<UINT>(levelHeight);
		staging.MipLevels      = 1;
		staging.ArraySize      = 1;
		staging.Format         = m_dxgiFormat;
		staging.SampleDesc.Count = 1;
		staging.Usage          = D3D11_USAGE_STAGING;
		staging.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

		ID3D11Texture2D *plane = NULL;
		hresult = device->CreateTexture2D(&staging, NULL, &plane);
		readback = plane;
	}

	if (FAILED(hresult) || !readback)
	{
		WARNING(true, ("Direct3d11: a readback surface for texture '%s' could not be created (%s), so a non-discarding lock is seeing zeroes.", describeTexture(m_texture), Direct3d11_Device::describeHresult(hresult)));
		return false;
	}

	// The whole subresource, so no box and no alignment question.
	context->CopySubresourceRegion(readback, 0, 0, 0, 0, m_resource, static_cast<UINT>(subresource), NULL);

	++Direct3d11_Metrics::blockingStagingMaps;

	bool staged = false;

	D3D11_MAPPED_SUBRESOURCE mapped;
	Zero(mapped);
	if (SUCCEEDED(context->Map(readback, 0, D3D11_MAP_READ, 0, &mapped)))
	{
		int rows = 0;
		int tightPitch = 0;
		computeTightPitches(m_storageFormat, levelWidth, levelHeight, tightPitch, rows);

		int copyBytes = (pitch < tightPitch) ? pitch : tightPitch;
		if (static_cast<int>(mapped.RowPitch) < copyBytes)
			copyBytes = static_cast<int>(mapped.RowPitch);

		for (int slice = 0; slice < levelDepth; ++slice)
		{
			uint8 const * const source = static_cast<uint8 const *>(mapped.pData) + (static_cast<size_t>(slice) * mapped.DepthPitch);
			uint8 * const target = destination + (static_cast<size_t>(slice) * slicePitch);

			for (int row = 0; row < rows; ++row)
				memcpy(target + (row * pitch), source + (static_cast<size_t>(row) * mapped.RowPitch), static_cast<size_t>(copyBytes));
		}

		context->Unmap(readback, 0);
		staged = true;
	}
	else
	{
		WARNING(true, ("Direct3d11: the readback surface for texture '%s' could not be mapped, so a non-discarding lock is seeing zeroes.", describeTexture(m_texture)));
	}

	readback->Release();
	return staged;
}

// ----------------------------------------------------------------------
/**
 * Present a region's current contents in the format the lock asked for.
 *
 * Stage the level in the storage format, convert the level if the formats differ, then
 * copy out the sub-rectangle. Converting the level rather than the region costs work for
 * a small region of a large level, which is exactly the representative-colour case; that
 * is once per texture and cached by the engine, and the alternative is block-alignment
 * arithmetic in three places instead of none.
 */

void Direct3d11_TextureData::readBackRegion(LockData const &lockData, uint8 *destination, int pitch, int slicePitch)
{
	int const level = lockData.getLevel();

	int levelWidth = 0;
	int levelHeight = 0;
	int levelDepth = 0;
	getLevelSize(level, levelWidth, levelHeight, levelDepth);

	int storagePitch = 0;
	int storageRows = 0;
	computeTightPitches(m_storageFormat, levelWidth, levelHeight, storagePitch, storageRows);
	int const storageSlicePitch = storagePitch * storageRows;

	uint8 * const levelBytes = new uint8[storageSlicePitch * levelDepth];
	memset(levelBytes, 0, static_cast<size_t>(storageSlicePitch * levelDepth));

	int const subresource = getSubresource(lockData.m_cubeFace, level);

	if (stageLevel(subresource, levelWidth, levelHeight, levelDepth, levelBytes, storagePitch, storageSlicePitch))
	{
		TextureFormat const format = lockData.m_format;

		int const x = lockData.getX();
		int const y = lockData.getY();
		int const z = lockData.getZ();
		int const width  = lockData.getWidth();
		int const height = lockData.getHeight();
		int const depth  = (lockData.getDepth() > 0) ? lockData.getDepth() : 1;

		if (format == m_storageFormat)
		{
			TextureFormatInfo const &info = TextureFormatInfo::getInfo(format);

			// A compressed region can only be sliced on block boundaries. Every engine
			// caller that locks in the native format locks the whole level, so this
			// never bites -- but a silently misaligned slice would produce a plausible
			// wrong image, so it is stated rather than assumed.
			FATAL(info.compressed && (x != 0 || y != 0 || width != levelWidth || height != levelHeight),
				("Direct3d11: texture '%s' is stored compressed and a non-discarding lock asked for the sub-region (%d,%d %dx%d) of a %dx%d level, which cannot be sliced on block boundaries.",
				describeTexture(m_texture), x, y, width, height, levelWidth, levelHeight));

			int rows = 0;
			int regionPitch = 0;
			computeTightPitches(format, width, height, regionPitch, rows);

			int const rowOffset = info.compressed ? ((y / info.blockHeight) * storagePitch) : (y * storagePitch);
			int const columnOffset = info.compressed ? ((x / info.blockWidth) * info.blockSize) : (x * info.pixelByteCount);

			for (int slice = 0; slice < depth; ++slice)
			{
				uint8 const * const source = levelBytes + (static_cast<size_t>(z + slice) * storageSlicePitch) + rowOffset + columnOffset;
				uint8 * const target = destination + (static_cast<size_t>(slice) * slicePitch);

				for (int row = 0; row < rows; ++row)
					memcpy(target + (row * pitch), source + (static_cast<size_t>(row) * storagePitch), static_cast<size_t>(regionPitch));
			}
		}
		else
		{
			int levelRows = 0;
			int levelPitch = 0;
			computeTightPitches(format, levelWidth, levelHeight, levelPitch, levelRows);

			uint8 * const converted = new uint8[levelPitch * levelRows];

			// Slice by slice, because the converter works on one 2D image at a time.
			// Only the requested slices are converted.
			int const bytesPerPixel = TextureFormatInfo::getInfo(format).pixelByteCount;
			int const regionPitch = width * bytesPerPixel;

			for (int slice = 0; slice < depth; ++slice)
			{
				Direct3d11_TextureConverter::convert(
					m_storageFormat, levelBytes + (static_cast<size_t>(z + slice) * storageSlicePitch), storagePitch,
					format, converted, levelPitch,
					levelWidth, levelHeight);

				uint8 * const target = destination + (static_cast<size_t>(slice) * slicePitch);

				for (int row = 0; row < height; ++row)
					memcpy(target + (row * pitch), converted + (static_cast<size_t>(y + row) * levelPitch) + (x * bytesPerPixel), static_cast<size_t>(regionPitch));
			}

			delete [] converted;
		}
	}

	delete [] levelBytes;
}

// ----------------------------------------------------------------------
/**
 * Hand the caller a buffer to read or write the requested region through.
 *
 * The format the caller asks for is frequently not the resource's -- Texture::load
 * locks in the source FILE's format, and computeRepresentativeColor always asks for
 * TF_ARGB_8888 whatever the texture is stored as. TF_Native means "whatever it
 * actually is", and has to be resolved into the lock before anything consults the
 * format table, because TF_Native sits past the end of that table.
 */

void Direct3d11_TextureData::lock(LockData &lockData)
{
	Direct3d11_Metrics::ScopedTimer timer(Direct3d11_Metrics::textureUploadTicks);

	DEBUG_FATAL(lockData.m_pixelData, ("Direct3d11: this texture region is already locked."));
	DEBUG_FATAL(lockData.getWidth() <= 0 || lockData.getHeight() <= 0, ("Direct3d11: a lock of texture '%s' asked for a %dx%d region.", describeTexture(m_texture), lockData.getWidth(), lockData.getHeight()));

	// Resolve TF_Native first: getInfo would DEBUG_FATAL on it.
	if (lockData.m_format == TF_Native)
		lockData.m_format = m_nativeFormat;

	TextureFormat const format = lockData.m_format;

	int const width  = lockData.getWidth();
	int const height = lockData.getHeight();
	int const depth  = (lockData.getDepth() > 0) ? lockData.getDepth() : 1;

	int pitch = 0;
	int rows = 0;
	computeTightPitches(format, width, height, pitch, rows);

	int const slicePitch = pitch * rows;
	int const bytes = slicePitch * depth;

	FATAL(bytes <= 0, ("Direct3d11: a lock of texture '%s' computed %d bytes for a %dx%dx%d region in format %s.",
		describeTexture(m_texture), bytes, width, height, depth, describeFormat(format)));

	// If the caller's format is not the one the resource holds, say so now rather than
	// at unlock, and name both -- an unconvertible pair means either the format map or
	// a caller has changed, and the message is the whole diagnosis.
	bool const needsConversion = (format != m_storageFormat);
	if (needsConversion)
	{
		FATAL(!Direct3d11_TextureConverter::canConvert(m_storageFormat, format) || !Direct3d11_TextureConverter::canConvert(format, m_storageFormat),
			("Direct3d11: texture '%s' is stored as %s and was locked as %s, and that pair cannot be converted in both directions.",
			describeTexture(m_texture), describeFormat(m_storageFormat), describeFormat(format)));
	}

	uint8 * const scratch = new uint8[bytes];

	// A lock that is not discarding has to show what is already there -- but only if
	// there IS anything. Texture::loadSurface passes discardContents = isDynamic(),
	// which is false for every .dds in the game, so without the initialised test every
	// mip level of every texture would stage-copy and stall at load time for contents
	// it is about to overwrite completely. A subresource that has never been written
	// has undefined contents, exactly as a freshly created D3D9 MANAGED surface does,
	// so zero is a legitimate answer for it.
	memset(scratch, 0, static_cast<size_t>(bytes));

	int const subresource = getSubresource(lockData.m_cubeFace, lockData.getLevel());

	if (!lockData.shouldDiscardContents() && isSubresourceInitialised(subresource))
		readBackRegion(lockData, scratch, pitch, slicePitch);

	lockData.m_pitch      = pitch;
	lockData.m_slicePitch = slicePitch;
	lockData.m_pixelData  = scratch;

	// The backend's private per-lock slot. DX9 parks a scratch D3D surface here; this
	// parks the allocation, and unlock() recovers it from here rather than recomputing
	// the size, so the two can never disagree.
	lockData.m_reserved   = scratch;
}

// ----------------------------------------------------------------------

void Direct3d11_TextureData::unlock(LockData &lockData)
{
	Direct3d11_Metrics::ScopedTimer timer(Direct3d11_Metrics::textureUploadTicks);

	uint8 * const scratch = static_cast<uint8 *>(lockData.m_reserved);

	if (!scratch)
	{
		DEBUG_WARNING(true, ("Direct3d11: a region of texture '%s' was unlocked without being locked.", describeTexture(m_texture)));
		return;
	}

	ID3D11DeviceContext1 * const context = Direct3d11_Device::getContext();

	if (context && m_resource && !lockData.isReadOnly())
	{
		TextureFormat const format = lockData.m_format;

		int const width  = lockData.getWidth();
		int const height = lockData.getHeight();
		int const depth  = (lockData.getDepth() > 0) ? lockData.getDepth() : 1;

		int const subresource = getSubresource(lockData.m_cubeFace, lockData.getLevel());

		// The destination box is in the resource's own texels, and for a block-compressed format
		// D3D requires it to be block aligned.
		//
		// An earlier version of this comment said every writer locks a whole level so the box "is
		// naturally aligned". That is wrong, and the D3D11 debug layer said so 24 times per run:
		//
		//   UpdateSubresource: ... *pDstBox = {{0,0,0},{2,2,1}}. BC2_UNORM requires alignment of
		//   coodinates to multiples of {4,4,1}.
		//
		// A WHOLE level can be 1x1 or 2x2 -- the tail of any mip chain is smaller than one 4x4
		// block -- so "whole level" and "block aligned" are different properties, and every
		// compressed texture with a full mip chain hits it on its last two or three levels.
		//
		// A whole-level write therefore passes no box at all. That is not a workaround: it is what
		// the API wants, and it lets the runtime size the write from the subresource itself rather
		// than being told a size it has to reject.
		int levelWidth = 0;
		int levelHeight = 0;
		int levelDepth = 0;
		getLevelSize(lockData.getLevel(), levelWidth, levelHeight, levelDepth);

		bool const wholeLevel =
			lockData.getX() == 0 && lockData.getY() == 0 && lockData.getZ() == 0 &&
			width == levelWidth && height == levelHeight && depth == levelDepth;

		D3D11_BOX box;
		box.left   = static_cast<UINT>(lockData.getX());
		box.top    = static_cast<UINT>(lockData.getY());
		box.front  = static_cast<UINT>(lockData.getZ());
		box.right  = static_cast<UINT>(lockData.getX() + width);
		box.bottom = static_cast<UINT>(lockData.getY() + height);
		box.back   = static_cast<UINT>(lockData.getZ() + depth);

		// A partial write into a compressed level still needs aligning, and cannot simply be
		// rounded: writing a whole block when the caller supplied part of one would put the wrong
		// texels in the rest of it. Reported rather than corrupted, once, with the numbers.
		TextureFormatInfo const &storageInfo = TextureFormatInfo::getInfo(m_storageFormat);

		if (!wholeLevel && storageInfo.compressed)
		{
			bool const aligned =
				(box.left   % static_cast<UINT>(storageInfo.blockWidth))  == 0 &&
				(box.top    % static_cast<UINT>(storageInfo.blockHeight)) == 0 &&
				((box.right  % static_cast<UINT>(storageInfo.blockWidth))  == 0 || box.right  == static_cast<UINT>(levelWidth)) &&
				((box.bottom % static_cast<UINT>(storageInfo.blockHeight)) == 0 || box.bottom == static_cast<UINT>(levelHeight));

			if (!aligned && !ms_reportedUnalignedBlockWrite)
			{
				ms_reportedUnalignedBlockWrite = true;
				WARNING(true, ("Direct3d11: a partial write to compressed level %d of '%s' covers texels (%u,%u)-(%u,%u), which is not a multiple of the %dx%d block. D3D11 will reject it and those texels will keep their old contents. Reported once.",
					lockData.getLevel(), m_texture.getName() ? m_texture.getName() : "<unnamed>",
					box.left, box.top, box.right, box.bottom,
					storageInfo.blockWidth, storageInfo.blockHeight));
			}
		}

		D3D11_BOX const * const destinationBox = wholeLevel ? NULL : &box;

		if (format == m_storageFormat)
		{
			context->UpdateSubresource(m_resource, static_cast<UINT>(subresource), destinationBox, scratch, static_cast<UINT>(lockData.m_pitch), static_cast<UINT>(lockData.m_slicePitch));
		}
		else
		{
			int storagePitch = 0;
			int storageRows = 0;
			computeTightPitches(m_storageFormat, width, height, storagePitch, storageRows);

			int const storageSlicePitch = storagePitch * storageRows;

			// Every slice, converted one 2D image at a time, into a single buffer laid
			// out the way UpdateSubresource expects. Sizing this for one slice would
			// silently truncate a volume write.
			uint8 * const converted = new uint8[storageSlicePitch * depth];

			for (int slice = 0; slice < depth; ++slice)
			{
				Direct3d11_TextureConverter::convert(
					format, scratch + (static_cast<size_t>(slice) * lockData.m_slicePitch), lockData.m_pitch,
					m_storageFormat, converted + (static_cast<size_t>(slice) * storageSlicePitch), storagePitch,
					width, height);
			}

			context->UpdateSubresource(m_resource, static_cast<UINT>(subresource), destinationBox, converted, static_cast<UINT>(storagePitch), static_cast<UINT>(storageSlicePitch));

			delete [] converted;
		}

		markSubresourceInitialised(subresource);
	}

	delete [] scratch;

	// Cleared on every path, read-only included: the engine's own already-locked and
	// not-locked assertions test the pixel data pointer, and so do its accessors.
	lockData.m_pitch      = 0;
	lockData.m_slicePitch = 0;
	lockData.m_pixelData  = NULL;
	lockData.m_reserved   = NULL;
}

// ======================================================================
/**
 * Copy a rectangle from another texture into a level of this one.
 *
 * Only a straight copy, not a stretch: D3D11 has no StretchRect on the immediate
 * context, and a scaling copy would need a full-screen shader pass. The one caller in
 * the engine -- the heat-distortion composite -- copies render target to render target
 * at level zero with identical rectangles, so a size or format mismatch is reported
 * rather than silently point sampled.
 */

void Direct3d11_TextureData::copyFrom(int surfaceLevel, TextureGraphicsData const &rhs, int srcX, int srcY, int srcWidth, int srcHeight, int dstX, int dstY, int dstWidth, int dstHeight)
{
	Direct3d11_TextureData const &source = static_cast<Direct3d11_TextureData const &>(rhs);

	FATAL(this == &source, ("Direct3d11: copyFrom cannot copy texture '%s' onto itself.", describeTexture(m_texture)));

	ID3D11DeviceContext1 * const context = Direct3d11_Device::getContext();
	if (!context || !m_resource || !source.m_resource)
		return;

	FATAL(srcWidth != dstWidth || srcHeight != dstHeight, ("Direct3d11: copyFrom was asked to scale a %dx%d region of '%s' into %dx%d of '%s', which needs a shader pass this backend does not have.", srcWidth, srcHeight, describeTexture(source.m_texture), dstWidth, dstHeight, describeTexture(m_texture)));

	FATAL(source.m_dxgiFormat != m_dxgiFormat, ("Direct3d11: copyFrom was asked to copy '%s' (%s) into '%s' (%s), and a copy cannot convert formats.", describeTexture(source.m_texture), describeFormat(source.m_nativeFormat), describeTexture(m_texture), describeFormat(m_nativeFormat)));

	D3D11_BOX box;
	box.left   = static_cast<UINT>(srcX);
	box.top    = static_cast<UINT>(srcY);
	box.front  = 0;
	box.right  = static_cast<UINT>(srcX + srcWidth);
	box.bottom = static_cast<UINT>(srcY + srcHeight);
	box.back   = 1;

	int const destinationSubresource = getSubresource(CF_none, surfaceLevel);

	context->CopySubresourceRegion(m_resource,
		static_cast<UINT>(destinationSubresource),
		static_cast<UINT>(dstX), static_cast<UINT>(dstY), 0,
		source.m_resource,
		static_cast<UINT>(source.getSubresource(CF_none, surfaceLevel)),
		&box);

	markSubresourceInitialised(destinationSubresource);
}

// ----------------------------------------------------------------------

char const *Direct3d11_TextureData::getTextureName() const
{
	return describeTexture(m_texture);
}

// ----------------------------------------------------------------------

D3D_SRV_DIMENSION Direct3d11_TextureData::getViewDimension() const
{
	if (m_texture.isCubeMap())
		return D3D_SRV_DIMENSION_TEXTURECUBE;

	if (m_texture.isVolumeMap())
		return D3D_SRV_DIMENSION_TEXTURE3D;

	return D3D_SRV_DIMENSION_TEXTURE2D;
}

// ======================================================================
