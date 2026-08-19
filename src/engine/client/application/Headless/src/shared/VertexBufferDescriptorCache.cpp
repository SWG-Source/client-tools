// ======================================================================
//
// VertexBufferDescriptorCache.cpp (Headless stub)
//
// The Headless module's original header declared this class but shipped
// no implementation. We mirror Direct3d9_VertexBufferDescriptorMap so the
// engine's vertex-buffer code path sees consistent stride/offset values
// even though nothing actually renders.
//
// ======================================================================

#include "FirstHeadless.h"
#include "VertexBufferDescriptorCache.h"

#include "clientGraphics/VertexBufferDescriptor.h"
#include "clientGraphics/VertexBufferFormat.h"

#include <map>

// ======================================================================

namespace VertexBufferDescriptorCacheNamespace
{
	typedef std::map<uint32, VertexBufferDescriptor> Cache;
	Cache *s_cache;
} // namespace VertexBufferDescriptorCacheNamespace

using namespace VertexBufferDescriptorCacheNamespace;

// ======================================================================

void VertexBufferDescriptorCache::install()
{
	s_cache = new Cache;
}

// ----------------------------------------------------------------------

void VertexBufferDescriptorCache::shutdown()
{
	delete s_cache;
	s_cache = NULL;
}

// ----------------------------------------------------------------------

VertexBufferDescriptor const &VertexBufferDescriptorCache::getDescriptor(VertexBufferFormat const &vertexFormat)
{
	Cache::iterator it = s_cache->find(vertexFormat.getFlags());
	if (it != s_cache->end())
		return it->second;

	VertexBufferDescriptor descriptor;

	if (vertexFormat.hasPosition())
	{
		descriptor.offsetPosition = descriptor.vertexSize;
		descriptor.vertexSize += sizeof(float) * 3;

		if (vertexFormat.isTransformed())
		{
			descriptor.offsetOoz = descriptor.vertexSize;
			descriptor.vertexSize += sizeof(float);
		}
	}

	if (vertexFormat.hasNormal())
	{
		descriptor.offsetNormal = descriptor.vertexSize;
		descriptor.vertexSize += sizeof(float) * 3;
	}

	if (vertexFormat.hasPointSize())
	{
		descriptor.offsetPointSize = descriptor.vertexSize;
		descriptor.vertexSize += sizeof(float);
	}

	if (vertexFormat.hasColor0())
	{
		descriptor.offsetColor0 = descriptor.vertexSize;
		descriptor.vertexSize += sizeof(uint32);
	}

	if (vertexFormat.hasColor1())
	{
		descriptor.offsetColor1 = descriptor.vertexSize;
		descriptor.vertexSize += sizeof(uint32);
	}

	int const numberOfTextureCoordinateSets = vertexFormat.getNumberOfTextureCoordinateSets();
	for (int i = 0; i < numberOfTextureCoordinateSets; ++i)
	{
		int const dimension = vertexFormat.getTextureCoordinateSetDimension(i);
		descriptor.offsetTextureCoordinateSet[i] = descriptor.vertexSize;
		descriptor.vertexSize = static_cast<int8>(descriptor.vertexSize + (sizeof(float) * dimension));
	}

	std::pair<Cache::iterator, bool> result = s_cache->insert(std::make_pair(vertexFormat.getFlags(), descriptor));
	return result.first->second;
}

// ======================================================================
