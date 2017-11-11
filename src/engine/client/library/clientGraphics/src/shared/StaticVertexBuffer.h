// ======================================================================
//
// StaticVertexBuffer.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_StaticVertexBuffer_H
#define INCLUDED_StaticVertexBuffer_H

// ======================================================================

class MemoryBlockManager;

#include "clientGraphics/HardwareVertexBuffer.h"
#include "clientGraphics/VertexBufferIterator.h"

// ======================================================================

class StaticVertexBufferGraphicsData
{
public:
	virtual DLLEXPORT ~StaticVertexBufferGraphicsData();

	virtual const VertexBufferDescriptor  &getDescriptor() const = 0;
	virtual int                            getSortKey() = 0;

	virtual void                          *lock(bool readOnly) = 0;
	virtual void                           unlock() = 0;
};


/**
 * Static vertex buffers are for rendering data that changes relatively
 * infrequently compared to use.
 */

class StaticVertexBuffer : public HardwareVertexBuffer
{
	friend class  Graphics;
	friend struct Gl_api;
	friend class  Direct3d8;
	friend class  Direct3d8_StaticVertexBufferData;
	friend class  Direct3d9;
	friend class  Direct3d9_StaticVertexBufferData;

public:

	static void install();
	static void *operator new(size_t);
	static void  operator delete(void *);

public:

	StaticVertexBuffer(const VertexBufferFormat &format, int numberOfVertices);
	StaticVertexBuffer(Iff &iff);
	virtual ~StaticVertexBuffer();

	virtual int                    getSortKey() const;

	int                            getNumberOfVertices() const;

	void                           lock();
	void                           lockReadOnly() const;
	void                           unlock() const;

	VertexBufferReadIterator       beginReadOnly() const;
	VertexBufferReadIterator       endReadOnly() const;

	VertexBufferReadWriteIterator  begin();
	VertexBufferReadWriteIterator  end();

protected:

	virtual VertexBufferWriteIterator preLoad(int numberOfVertices);
	virtual void                      postLoad();

private:

	static void remove();

	// Disabled
	StaticVertexBuffer();
	StaticVertexBuffer(const StaticVertexBuffer &);
	StaticVertexBuffer &operator =(const StaticVertexBuffer &);

private:

	static MemoryBlockManager *ms_memoryBlockManager;

private:

	StaticVertexBufferGraphicsData         *m_graphicsData;
	int                                     m_sortKey;
	int                                     m_numberOfVertices;

	mutable byte                           *m_data;
	mutable bool                            m_lockedReadOnly;

#ifdef _DEBUG
	int                                     m_locks;
	mutable int                             m_sets;
	mutable bool                            m_lockedTooFrequentlyWarning;
#endif
};

// ======================================================================
/**
 * Get the number of vertices in this VertexBuffer.
 * @return The number of vertices in the vertex buffer.
 */

inline int StaticVertexBuffer::getNumberOfVertices() const
{
	return m_numberOfVertices;
}

// ----------------------------------------------------------------------

inline int StaticVertexBuffer::getSortKey() const
{
	return m_sortKey;
}

// ----------------------------------------------------------------------
/**
 * Allow the vertex data to be modified.
 * This routine may not be called multiple times.  A VertexBuffer may not be used to render while it is locked.
 * @see unlock()
 */

inline void StaticVertexBuffer::lock()
{
	NOT_NULL(m_graphicsData);
	DEBUG_FATAL(m_data, ("VB already locked"));

#ifdef _DEBUG
	++m_debugIteratorLockCount;

	// detect a static VB being locked too frequently
	if (!m_lockedTooFrequentlyWarning && ++m_locks > 20 && (m_locks * 2 > m_sets))
	{
		DEBUG_WARNING(true, ("static VB is being locked extremely frequently.  Should this be a dynamic VB?"));
		m_lockedTooFrequentlyWarning = true;
	}
#endif

	m_lockedReadOnly = false;
	m_data = reinterpret_cast<byte *>(m_graphicsData->lock(false));
}

// ----------------------------------------------------------------------
/**
 * Allow the vertex data to be modified.
 * This routine may not be called multiple times.  A VertexBuffer may not be used to render while it is locked.
 * @see unlock()
 */

inline void StaticVertexBuffer::lockReadOnly() const
{
	NOT_NULL(m_graphicsData);

	DEBUG_FATAL(m_data, ("VB already locked"));

#ifdef _DEBUG
	++m_debugIteratorLockCount;
#endif
	m_lockedReadOnly = true;
	m_data = reinterpret_cast<byte *>(m_graphicsData->lock(true));
}

// ----------------------------------------------------------------------
/**
 * Let the VertexBuffer know it is done being modified.
 * @see lock()
 */

inline void StaticVertexBuffer::unlock() const
{
	DEBUG_FATAL(!m_data, ("VB not locked"));

#ifdef _DEBUG
	++m_debugIteratorLockCount;
#endif
	m_data = NULL;
	m_graphicsData->unlock();
}

// ----------------------------------------------------------------------
/**
 * return read-only iterator pointing to first vertex of the vertex buffer.
 */

inline VertexBufferReadIterator StaticVertexBuffer::beginReadOnly() const
{
	DEBUG_FATAL(!m_data, ("Vertex buffer is not locked"));
	return VertexBufferReadIterator(*this, m_data);
}

// ----------------------------------------------------------------------
/**
 * Return read-only iterator pointing to one past end of the vertex buffer.
 */

inline VertexBufferReadIterator StaticVertexBuffer::endReadOnly() const
{
	DEBUG_FATAL(!m_data, ("Vertex buffer is not locked"));
	return VertexBufferReadIterator(*this, m_data + m_numberOfVertices * m_descriptor->vertexSize);
}

// ----------------------------------------------------------------------
/**
 * Return a read-write iterator pointing to first vertex of the vertex buffer.
 */

inline VertexBufferReadWriteIterator StaticVertexBuffer::begin()
{
	DEBUG_FATAL(!m_data, ("Vertex buffer is not locked"));
	return VertexBufferReadWriteIterator(*this, m_data);
}

// ----------------------------------------------------------------------
/**
 * return read-write iterator pointing to one past end of the vertex buffer.
 */

inline VertexBufferReadWriteIterator StaticVertexBuffer::end()
{
	DEBUG_FATAL(!m_data, ("Vertex buffer is not locked"));
	return VertexBufferReadWriteIterator(*this, m_data + m_numberOfVertices * m_descriptor->vertexSize);
}

// ======================================================================

#endif
