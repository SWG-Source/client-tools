// ======================================================================
//
// Direct3d9_VertexBufferDescriptorMap.cpp
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstDirect3d9.h"
#include "Direct3d9_VertexBufferDescriptorMap.h"

#include "Direct3d9.h"

#include "clientGraphics/VertexBufferFormat.h"
#include "clientGraphics/VertexBufferDescriptor.h"

#include <map>

// ======================================================================

namespace Direct3d9_VertexBufferDescriptorMapNamespace
{
	typedef std::map<uint32, VertexBufferDescriptor> DescriptorMap;

	const int TextureCoordinateSetDimensionLookup[] =
	{
		D3DFVF_TEXTUREFORMAT1,
		D3DFVF_TEXTUREFORMAT2,
		D3DFVF_TEXTUREFORMAT3,
		D3DFVF_TEXTUREFORMAT4
	};

	DescriptorMap  *ms_descriptorMap;
}
using namespace Direct3d9_VertexBufferDescriptorMapNamespace;

// ======================================================================

void Direct3d9_VertexBufferDescriptorMap::install()
{
	ms_descriptorMap = new DescriptorMap;
}

// ----------------------------------------------------------------------

void Direct3d9_VertexBufferDescriptorMap::remove()
{
	delete ms_descriptorMap;
}

// ----------------------------------------------------------------------

const VertexBufferDescriptor &Direct3d9_VertexBufferDescriptorMap::getDescriptor(const VertexBufferFormat &vertexFormat)
{
	DescriptorMap::iterator i = ms_descriptorMap->find(vertexFormat.getFlags());

	// if one wasn't found, add it
	if (i == ms_descriptorMap->end())
	{
		VertexBufferDescriptor descriptor;

		// position
		if (vertexFormat.hasPosition())
		{
			descriptor.offsetPosition = descriptor.vertexSize;
			descriptor.vertexSize += sizeof(float) * 3;

			// rhw
			if (vertexFormat.isTransformed())
			{
				descriptor.offsetOoz = descriptor.vertexSize;
				descriptor.vertexSize += sizeof(float);
			}
			else
			{
				descriptor.offsetOoz = -1;
			}
		}
		else
		{
			DEBUG_FATAL(vertexFormat.isTransformed(), ("Transformed data requires XYZ as well"));
			descriptor.offsetPosition = -1;
		}

		// normal
		if (vertexFormat.hasNormal())
		{
			descriptor.offsetNormal = descriptor.vertexSize;
			descriptor.vertexSize += sizeof(float) * 3;
		}
		else
		{
			descriptor.offsetNormal = -1;
		}

		// point size

		if (vertexFormat.hasPointSize())
		{
			descriptor.offsetPointSize = descriptor.vertexSize;
			descriptor.vertexSize += sizeof(float);
		}
		else
		{
			descriptor.offsetPointSize = -1;
		}

		// color0
		if (vertexFormat.hasColor0())
		{
			descriptor.offsetColor0 = descriptor.vertexSize;
			descriptor.vertexSize += sizeof(uint32);
		}
		else
		{
			descriptor.offsetColor0 = -1;
		}

		// color1
		if (vertexFormat.hasColor1())
		{
			descriptor.offsetColor1 = descriptor.vertexSize;
			descriptor.vertexSize += sizeof(uint32);
		}
		else
		{
			descriptor.offsetColor1 = -1;
		}

		// texture coordinate sets
		const int numberOfTextureCoordinateSets = vertexFormat.getNumberOfTextureCoordinateSets();
		{
			for (int i = 0; i < numberOfTextureCoordinateSets; ++i)
			{
				const int dimension = vertexFormat.getTextureCoordinateSetDimension(i);
				descriptor.offsetTextureCoordinateSet[i] = descriptor.vertexSize;
				descriptor.vertexSize = static_cast<int8>(descriptor.vertexSize + (sizeof(float) * dimension));
			}
		}
		{
			for (int i = numberOfTextureCoordinateSets; i < VertexBufferFormat::MAX_TEXTURE_COORDINATE_SETS; ++i)
				descriptor.offsetTextureCoordinateSet[i] = -1;
		}

		DEBUG_FATAL(descriptor.vertexSize == 0, ("Vertex has no data"));

		// insert the new descriptor
		DescriptorMap::value_type entry(vertexFormat.getFlags(), descriptor);
		std::pair<DescriptorMap::iterator, bool> result = ms_descriptorMap->insert(entry);
		DEBUG_FATAL(!result.second, ("insert() said entry was already there, but find() didn't locate it"));

		// and we need the iterator to it to return the descriptor
		i = result.first;
	}

	return i->second;
}

// ----------------------------------------------------------------------

const VertexBufferDescriptor &Direct3d9_VertexBufferDescriptorMap::getDescriptor(uint32 formatFlags)
{
	DescriptorMap::iterator i = ms_descriptorMap->find(formatFlags);
	DEBUG_FATAL(i == ms_descriptorMap->end(), ("Descriptor not found"));
	return i->second;
}

// ======================================================================
