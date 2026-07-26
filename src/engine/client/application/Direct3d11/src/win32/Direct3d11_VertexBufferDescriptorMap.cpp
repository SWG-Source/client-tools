// ======================================================================
//
// Direct3d11_VertexBufferDescriptorMap.cpp
// copyright (c) 2026 Galaxies Reborn
//
// ======================================================================

#include "FirstDirect3d11.h"
#include "Direct3d11_VertexBufferDescriptorMap.h"

#include "clientGraphics/VertexBuffer.h"
#include "clientGraphics/VertexBufferDescriptor.h"
#include "clientGraphics/VertexBufferFormat.h"

#include <map>

// ======================================================================

namespace Direct3d11_VertexBufferDescriptorMapNamespace
{
	// std::map, not a vector or a hash table, and deliberately: the engine keeps
	// the address of a value, and std::map never moves its values on insert.
	typedef std::map<uint32, VertexBufferDescriptor> DescriptorMap;
	DescriptorMap *ms_descriptorMap;
}
using namespace Direct3d11_VertexBufferDescriptorMapNamespace;

// ======================================================================

void Direct3d11_VertexBufferDescriptorMap::install()
{
	DEBUG_FATAL(ms_descriptorMap, ("Direct3d11_VertexBufferDescriptorMap::install called twice"));
	ms_descriptorMap = new DescriptorMap;
}

// ----------------------------------------------------------------------

void Direct3d11_VertexBufferDescriptorMap::remove()
{
	delete ms_descriptorMap;
	ms_descriptorMap = NULL;
}

// ======================================================================
/**
 * Compute, cache and return the byte layout for a vertex format.
 *
 * The component order below IS the memory layout of every vertex in the game, and
 * it is the order Direct3d9_VertexBufferDescriptorMap uses: position, then the
 * reciprocal-w when the position is pre-transformed, then normal, point size,
 * colour 0, colour 1, then each texture coordinate set in order at its own
 * dimension. Changing the order would change where every existing vertex buffer's
 * components live.
 */

VertexBufferDescriptor const &Direct3d11_VertexBufferDescriptorMap::getDescriptor(VertexBufferFormat const &vertexFormat)
{
	NOT_NULL(ms_descriptorMap);

	DescriptorMap::iterator i = ms_descriptorMap->find(vertexFormat.getFlags());
	if (i != ms_descriptorMap->end())
		return i->second;

	VertexBufferDescriptor descriptor;

	if (vertexFormat.hasPosition())
	{
		descriptor.offsetPosition = descriptor.vertexSize;
		descriptor.vertexSize = static_cast<int8>(descriptor.vertexSize + (sizeof(float) * 3));

		// The reciprocal of w, present only for already-transformed positions.
		if (vertexFormat.isTransformed())
		{
			descriptor.offsetOoz = descriptor.vertexSize;
			descriptor.vertexSize = static_cast<int8>(descriptor.vertexSize + sizeof(float));
		}
		else
			descriptor.offsetOoz = -1;
	}
	else
	{
		DEBUG_FATAL(vertexFormat.isTransformed(), ("Transformed data requires XYZ as well"));
		descriptor.offsetPosition = -1;
	}

	if (vertexFormat.hasNormal())
	{
		descriptor.offsetNormal = descriptor.vertexSize;
		descriptor.vertexSize = static_cast<int8>(descriptor.vertexSize + (sizeof(float) * 3));
	}
	else
		descriptor.offsetNormal = -1;

	if (vertexFormat.hasPointSize())
	{
		descriptor.offsetPointSize = descriptor.vertexSize;
		descriptor.vertexSize = static_cast<int8>(descriptor.vertexSize + sizeof(float));
	}
	else
		descriptor.offsetPointSize = -1;

	if (vertexFormat.hasColor0())
	{
		descriptor.offsetColor0 = descriptor.vertexSize;
		descriptor.vertexSize = static_cast<int8>(descriptor.vertexSize + sizeof(uint32));
	}
	else
		descriptor.offsetColor0 = -1;

	if (vertexFormat.hasColor1())
	{
		descriptor.offsetColor1 = descriptor.vertexSize;
		descriptor.vertexSize = static_cast<int8>(descriptor.vertexSize + sizeof(uint32));
	}
	else
		descriptor.offsetColor1 = -1;

	int const numberOfTextureCoordinateSets = vertexFormat.getNumberOfTextureCoordinateSets();

	for (int set = 0; set < numberOfTextureCoordinateSets; ++set)
	{
		int const dimension = vertexFormat.getTextureCoordinateSetDimension(set);
		descriptor.offsetTextureCoordinateSet[set] = descriptor.vertexSize;
		descriptor.vertexSize = static_cast<int8>(descriptor.vertexSize + (sizeof(float) * dimension));
	}

	for (int set = numberOfTextureCoordinateSets; set < VertexBufferFormat::MAX_TEXTURE_COORDINATE_SETS; ++set)
		descriptor.offsetTextureCoordinateSet[set] = -1;

	// vertexSize is int8, so a vertex over 127 bytes wraps negative rather than
	// overflowing loudly. Fatal, because a negative stride produces garbage
	// geometry with nothing else reported.
	FATAL(descriptor.vertexSize <= 0, ("Direct3d11: vertex format 0x%08x computed a vertex size of %d. Either the format describes no data, or it exceeds the 127 bytes an int8 stride can hold.", vertexFormat.getFlags(), static_cast<int>(descriptor.vertexSize)));

	DescriptorMap::value_type entry(vertexFormat.getFlags(), descriptor);
	std::pair<DescriptorMap::iterator, bool> const result = ms_descriptorMap->insert(entry);
	DEBUG_FATAL(!result.second, ("insert() said the entry was already there, but find() did not locate it"));

	return result.first->second;
}

// ----------------------------------------------------------------------
/**
 * Look up a layout for flags whose format object is gone.
 *
 * Only legal once the flags have been seen: there is no way to recompute a layout
 * from flags alone without a VertexBufferFormat to interrogate.
 */

VertexBufferDescriptor const &Direct3d11_VertexBufferDescriptorMap::getDescriptor(uint32 formatFlags)
{
	NOT_NULL(ms_descriptorMap);

	DescriptorMap::const_iterator const i = ms_descriptorMap->find(formatFlags);
	FATAL(i == ms_descriptorMap->end(), ("Direct3d11: no vertex descriptor has been computed for format 0x%08x. It has to be created from a VertexBufferFormat before it can be looked up by flags.", formatFlags));

	return i->second;
}

// ----------------------------------------------------------------------

int Direct3d11_VertexBufferDescriptorMap::getDescriptorCount()
{
	return ms_descriptorMap ? static_cast<int>(ms_descriptorMap->size()) : 0;
}

// ======================================================================
