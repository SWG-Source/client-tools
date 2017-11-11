// ======================================================================
//
// SystemVertexBuffer.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_SystemVertexBuffer_H
#define INCLUDED_SystemVertexBuffer_H

// ======================================================================

class Mutex;
class MemoryBlockManager;
class MemoryBlockManager;

#include "clientGraphics/VertexBuffer.h"
#include "clientGraphics/VertexBufferIterator.h"

// ======================================================================

/**
 * System vertex buffers can be accessed in the same manner as other
 * vertex buffers, but they may not be used for rendering.
 */

class SystemVertexBuffer : public VertexBuffer
{
public:

	static void  install();
	static int   getDescriptorVertexSize(const VertexBufferFormat &format);

	static void *operator new(size_t);
	static void  operator delete(void *);

public:

	SystemVertexBuffer(const VertexBufferFormat &format, int numberOfVertices, MemoryBlockManager *memoryBlockManager=0);
	virtual ~SystemVertexBuffer();

	int               getNumberOfVertices() const;
	void              getMinMax(Vector &minExtent, Vector &maxExtent) const;

	void              write(Iff &iff) const;

	VertexBufferReadIterator        beginReadOnly() const;
	VertexBufferReadIterator        endReadOnly() const;

	VertexBufferWriteIterator       beginWriteOnly();
	VertexBufferWriteIterator       endWriteOnly();

	VertexBufferReadWriteIterator   begin();
	VertexBufferReadWriteIterator   end();

protected:

	virtual VertexBufferWriteIterator preLoad(int numberOfVertices);
	virtual void                      postLoad();

private:

	// Disabled
	SystemVertexBuffer();
	SystemVertexBuffer(const SystemVertexBuffer &);
	SystemVertexBuffer &operator =(const SystemVertexBuffer &);

private:

	static void                          remove();
	static const VertexBufferDescriptor &lookupDescriptor(const VertexBufferFormat &vertexFormat);

private:

	typedef stdmap<uint32, VertexBufferDescriptor>::fwd DescriptorMap;

	static MemoryBlockManager    *ms_memoryBlockManager;
	static DescriptorMap                      *ms_descriptorMap;
	static Mutex                               ms_criticalSection;

private:

	MemoryBlockManager *const  m_dataMemoryBlockManager;
	int                        m_numberOfVertices;
	byte                      *m_data;
};

// ======================================================================
/**
 * Get the number of vertices in this VertexBuffer.
 * @return The number of vertices in the vertex buffer.
 */

inline int SystemVertexBuffer::getNumberOfVertices() const
{
	return m_numberOfVertices;
}

// ----------------------------------------------------------------------
/**
 * Return read-only iterator pointing to first vertex of the vertex buffer.
 */

inline VertexBufferReadIterator SystemVertexBuffer::beginReadOnly() const
{
	return VertexBufferReadIterator(*this, m_data);
}

// ----------------------------------------------------------------------
/**
 * Return read-only iterator pointing to one past end of the vertex buffer.
 */

inline VertexBufferReadIterator SystemVertexBuffer::endReadOnly() const
{
	return VertexBufferReadIterator(*this, m_data + m_numberOfVertices * m_descriptor->vertexSize);
}

// ----------------------------------------------------------------------
/**
 * Return a write iterator pointing to first vertex of the vertex buffer.
 */

inline VertexBufferWriteIterator SystemVertexBuffer::beginWriteOnly()
{
	return VertexBufferWriteIterator(*this, m_data);
}

// ----------------------------------------------------------------------
/**
 * Return write iterator pointing to one past end of the vertex buffer.
 */

inline VertexBufferWriteIterator SystemVertexBuffer::endWriteOnly()
{
	return VertexBufferWriteIterator(*this, m_data + m_numberOfVertices * m_descriptor->vertexSize);
}

// ----------------------------------------------------------------------
/**
 * Return a read-write iterator pointing to first vertex of the vertex buffer.
 */

inline VertexBufferReadWriteIterator SystemVertexBuffer::begin()
{
	return VertexBufferReadWriteIterator(*this, m_data);
}

// ----------------------------------------------------------------------
/**
 * Return read-write iterator pointing to one past end of the vertex buffer.
 */

inline VertexBufferReadWriteIterator SystemVertexBuffer::end()
{
	return VertexBufferReadWriteIterator(*this, m_data + m_numberOfVertices * m_descriptor->vertexSize);
}

// ======================================================================

#endif
