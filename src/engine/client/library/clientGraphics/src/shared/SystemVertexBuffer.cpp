// ======================================================================
//
// SystemVertexBuffer.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/SystemVertexBuffer.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedSynchronization/Mutex.h"

#include <map>

// ======================================================================

MemoryBlockManager    *SystemVertexBuffer::ms_memoryBlockManager;
SystemVertexBuffer::DescriptorMap  *SystemVertexBuffer::ms_descriptorMap;
Mutex                               SystemVertexBuffer::ms_criticalSection;

// ======================================================================

void SystemVertexBuffer::install()
{
	DEBUG_FATAL(ms_memoryBlockManager, ("Already installed"));

	ms_memoryBlockManager = new MemoryBlockManager("SystemVertexBuffer", true, sizeof(SystemVertexBuffer), 0, 0, 0);
	ms_descriptorMap = new DescriptorMap;

	ExitChain::add(remove, "SystemVertexBuffer::remove()");
}

// ----------------------------------------------------------------------

void SystemVertexBuffer::remove()
{
	NOT_NULL(ms_memoryBlockManager);
	delete ms_memoryBlockManager;
	ms_memoryBlockManager = 0;

	NOT_NULL(ms_descriptorMap);
	delete ms_descriptorMap;
	ms_descriptorMap = 0;
}

// ----------------------------------------------------------------------

void *SystemVertexBuffer::operator new(size_t size)
{
	UNREF(size);	
	NOT_NULL(ms_memoryBlockManager);
	DEBUG_FATAL(size != sizeof(SystemVertexBuffer), ("Looks like a decendent class is trying to use our new routine"));
	DEBUG_FATAL(size != static_cast<size_t> (ms_memoryBlockManager->getElementSize()), ("installed with bad size"));

	return ms_memoryBlockManager->allocate();
}

// ----------------------------------------------------------------------

void  SystemVertexBuffer::operator delete(void *pointer)
{
	NOT_NULL(ms_memoryBlockManager);
	ms_memoryBlockManager->free(pointer);
}

// ----------------------------------------------------------------------

int SystemVertexBuffer::getDescriptorVertexSize(const VertexBufferFormat &vertexFormat)
{
	return lookupDescriptor(vertexFormat).vertexSize;
}

// ----------------------------------------------------------------------
/**
 * It is desired that this format matches whatever the hardware is using.
 */
const VertexBufferDescriptor &SystemVertexBuffer::lookupDescriptor(const VertexBufferFormat &vertexFormat)
{
	ms_criticalSection.enter();

		DescriptorMap::iterator i = ms_descriptorMap->find(vertexFormat.getFlags());

		// if one wasn't found, add it
		if (i == ms_descriptorMap->end())
		{
			VertexBufferDescriptor descriptor;

			descriptor.vertexSize = 0;

			// position
			if (vertexFormat.hasPosition())
			{
				descriptor.offsetPosition = descriptor.vertexSize;
				descriptor.vertexSize += isizeof(float) * 3;
			}

			// rhw
			if (vertexFormat.isTransformed())
			{
				descriptor.offsetOoz = descriptor.vertexSize;
				descriptor.vertexSize += isizeof(float);
			}
			else
			{
				descriptor.offsetOoz = -1;
			}

			// normal
			if (vertexFormat.hasNormal())
			{
				descriptor.offsetNormal = descriptor.vertexSize;
				descriptor.vertexSize += isizeof(float) * 3;
			}
			else
			{
				descriptor.offsetNormal = -1;
			}

			// diffuse color
			if (vertexFormat.hasColor0())
			{
				descriptor.offsetColor0 = descriptor.vertexSize;
				descriptor.vertexSize += isizeof(uint32);
			}
			else
			{
				descriptor.offsetColor0 = -1;
			}

			// diffuse color
			if (vertexFormat.hasColor1())
			{
				descriptor.offsetColor1 = descriptor.vertexSize;
				descriptor.vertexSize += isizeof(uint32);
			}
			else
			{
				descriptor.offsetColor1 = -1;
			}

			// texture coordinate sets
			const int numberOfTextureCoordinateSets = vertexFormat.getNumberOfTextureCoordinateSets();
			{
				for (int j = 0; j < numberOfTextureCoordinateSets; ++j)
				{
					descriptor.offsetTextureCoordinateSet[j] = descriptor.vertexSize;
					descriptor.vertexSize = static_cast<int8>(descriptor.vertexSize + (isizeof(float) * vertexFormat.getTextureCoordinateSetDimension(j)));
				}
			}
			{
				for (int j = numberOfTextureCoordinateSets; j < VertexBufferDescriptor::MAX_TEXTURE_COORDINATE_SETS; ++j)
					descriptor.offsetTextureCoordinateSet[j] = -1;
			}

			DEBUG_FATAL(descriptor.vertexSize == 0, ("Vertex has no data"));

			// insert the new descriptor
			DescriptorMap::value_type entry(vertexFormat.getFlags(), descriptor);
			std::pair<DescriptorMap::iterator, bool> result = ms_descriptorMap->insert(entry);
			DEBUG_FATAL(!result.second, ("insert() said entry was already there, but find() didn't locate it"));

			// and we need the iterator to it to return the stored descriptor
			i = result.first;
		}

	ms_criticalSection.leave();

	return (*i).second;
}

// ======================================================================

SystemVertexBuffer::SystemVertexBuffer(const VertexBufferFormat &format, int numberOfVertices, MemoryBlockManager* const memoryBlockManager)
: VertexBuffer(format),
	m_dataMemoryBlockManager(memoryBlockManager),
	m_numberOfVertices(numberOfVertices),
	m_data(0)
{
	m_descriptor = &lookupDescriptor(format);

	if (m_dataMemoryBlockManager)
	{
		DEBUG_FATAL(m_dataMemoryBlockManager->getElementSize() != m_descriptor->vertexSize * m_numberOfVertices, ("memoryBlockManager %s is wrong size (%i != %i)", m_dataMemoryBlockManager->getName(), m_dataMemoryBlockManager->getElementSize(), m_descriptor->vertexSize * m_numberOfVertices));
		m_data = reinterpret_cast<byte*>(m_dataMemoryBlockManager->allocate());
	}
	else
		m_data = new byte[static_cast<size_t>(m_descriptor->vertexSize * m_numberOfVertices)];
}

// ----------------------------------------------------------------------

SystemVertexBuffer::~SystemVertexBuffer()
{
	if (m_dataMemoryBlockManager)
		m_dataMemoryBlockManager->free(m_data);
	else
		delete [] m_data;

	m_data = 0;  //lint !e672  // possible memory leak -- deleted above
}  //lint !e1740  // m_dataMemoryBlockManager not freed or zeroed -- it's a const pointer

// ----------------------------------------------------------------------

VertexBufferWriteIterator SystemVertexBuffer::preLoad(int numberOfVertices)
{
	DEBUG_FATAL(true, ("Cannot load into a dynamic vertex buffer"));
	UNREF(numberOfVertices);  //lint !e527  // unreachable -- this function should not be called, but it needs to return an object

	return VertexBufferWriteIterator(*this, NULL);
}

// ----------------------------------------------------------------------

void SystemVertexBuffer::postLoad()
{
	DEBUG_FATAL(true, ("Cannot load into a dynamic vertex buffer"));
}

// ----------------------------------------------------------------------
/**
 * Retrieve the min and max extents for the vertex array.
 * 
 * This function runs through all vertices in the buffer, finding
 * the minimum x,y,z and maximum x,y,z.  The minExtent and maxExtent
 * vectors thus form the minimum local-space-aligned bounding box for
 * the vertex data.
 * 
 * If there are no vertices in the vertex array, the min and max are
 * all set to zero.
 * 
 * @param minExtent  [OUT] specifies the minimum x,y,z found in the vertex array
 * @param maxExtent  [OUT] specifies the maximum x,y,z found in the vertex array
 */

void SystemVertexBuffer::getMinMax(Vector &minExtent, Vector &maxExtent) const
{
	if (m_numberOfVertices == 0)
	{
		minExtent = Vector::zero;
		maxExtent = Vector::zero;
		return;
	}

	// start off with the first vertex as the whole min/max 
	VertexBufferReadIterator v = beginReadOnly();
	minExtent = v.getPosition();
	maxExtent = minExtent;

	// check all the other vertices
	const VertexBufferReadIterator e = endReadOnly();
	for (++v; v != e; ++v)
	{
		const Vector &pos = v.getPosition();

		minExtent.x = std::min(minExtent.x, pos.x);
		minExtent.y = std::min(minExtent.y, pos.y);
		minExtent.z = std::min(minExtent.z, pos.z);

		maxExtent.x = std::max(maxExtent.x, pos.x);
		maxExtent.y = std::max(maxExtent.y, pos.y);
		maxExtent.z = std::max(maxExtent.z, pos.z);
	}
}

// ----------------------------------------------------------------------
/*
 * Save the vertex buffer to an Iff.
 * @param iff The iff file to store the vertex buffer data into.
 */

void SystemVertexBuffer::write(Iff &iff) const
{
	VertexBuffer::write(iff, beginReadOnly(), m_numberOfVertices);
}

// ======================================================================
