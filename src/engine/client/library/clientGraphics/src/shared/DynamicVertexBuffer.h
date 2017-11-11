// ======================================================================
//
// DynamicVertexBuffer.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_DynamicVertexBuffer_H
#define INCLUDED_DynamicVertexBuffer_H

// ======================================================================

class MemoryBlockManager;

#include "clientGraphics/HardwareVertexBuffer.h"
#include "clientGraphics/VertexBufferIterator.h"

// ======================================================================

class DynamicVertexBufferGraphicsData
{
public:
	virtual DLLEXPORT ~DynamicVertexBufferGraphicsData();

	virtual void                          *lock(int numberOfVertices, bool forceDiscard) = 0;
	virtual void                           unlock() = 0;
	virtual void                           unlock(int numberOfVertices) = 0;
	virtual const VertexBufferDescriptor  &getDescriptor() const = 0;
	virtual int                            getNumberOfLockableDynamicVertices(bool withDiscard) = 0;
	virtual int                            getSortKey() = 0;
};

/**
 * Dynamic vertex buffers are for rendering data that changes frequently.
 */

class DynamicVertexBuffer : public HardwareVertexBuffer
{
	friend class  Graphics;
	friend struct Gl_api;
	friend class  Direct3d8;
	friend class  Direct3d8_DynamicVertexBufferData;
	friend class  Direct3d9;
	friend class  Direct3d9_DynamicVertexBufferData;

public:

	static void install();
	static void *operator new(size_t);
	static void  operator delete(void *);

public:

#ifdef _DEBUG
	static int        getDynamicGlobalId();
#endif

public:

	DynamicVertexBuffer(const VertexBufferFormat &format);
	virtual ~DynamicVertexBuffer();

	virtual int           getSortKey() const;
#ifdef _DEBUG
	int                   getDynamicId() const;
#endif

	int                   getNumberOfLockableDynamicVertices(bool withDiscard=true);

	void                  lock(int numberOfVertices, bool forceDiscard=false);
	void                  unlock();
	void                  unlock(int numberOfVertices);

	VertexBufferWriteIterator  begin();
	VertexBufferWriteIterator  end();

protected:

	virtual VertexBufferWriteIterator preLoad(int numberOfVertices);
	virtual void                      postLoad();

private:

	static void remove();

	// Disabled
	DynamicVertexBuffer();
	DynamicVertexBuffer(const DynamicVertexBuffer &);
	DynamicVertexBuffer &operator =(const DynamicVertexBuffer &);

private:
	
	static MemoryBlockManager  *ms_memoryBlockManager;

#ifdef _DEBUG
	static int                               ms_dynamicGlobalId;
	static bool                              ms_dynamicGlobalLocked;
#endif

private:

	DynamicVertexBufferGraphicsData * m_graphicsData;
	int                               m_sortKey;

	byte *                            m_data;
	int                               m_numberOfVertices;

#ifdef _DEBUG
	int                               m_dynamicId;
#endif
};

// ======================================================================
/**
 * Used to catch invalid use of a Dynamic vertex buffer.
 * @internal
 */

#ifdef _DEBUG
inline int DynamicVertexBuffer::getDynamicGlobalId()
{
	return ms_dynamicGlobalId;
}
#endif

// ======================================================================
/**
 * Get a unique key for sorting by vertex buffer.
 * @return a unique key for this vertex buffer
 */
inline int DynamicVertexBuffer::getSortKey() const
{
	return m_sortKey;
}

// ----------------------------------------------------------------------
/**
 * Used to catch invalid use of a Dynamic vertex buffer.
 * @internal
 */

#ifdef _DEBUG
inline int DynamicVertexBuffer::getDynamicId() const
{
	return m_dynamicId;
}
#endif

// ----------------------------------------------------------------------
/**
 * Get the number of lockable vertices in a dynamic VertexBuffer.
 *
 * If the withDiscard parameter is true, then this routine will return
 * the maximum number of vertices for this vertex type that may ever
 * be locked in a dynamic vertex buffer.  If withDiscard is false, it will
 * return the number of vertices that there is currently room for in the
 * backing vertex buffer without discarding the contents.
 *
 * @param  withDiscard Whether or not to discard existing vertices.  See remarks.
 * @return             The number of vertices that may be locked.  See remarks.
 */

inline int DynamicVertexBuffer::getNumberOfLockableDynamicVertices(bool withDiscard)
{
	return m_graphicsData->getNumberOfLockableDynamicVertices(withDiscard);
}

// ----------------------------------------------------------------------
/**
 * Allow the vertex data to be modified.
 * 
 * A VertexBuffer may not be used to render while it is locked.
 * Data is not preserved between separate locks of a dynamic vertex buffer.
 * Locking a dynamic vertex buffer makes it illegal to try to render with any
 * previously locked dynamic vertex buffer.
 * @param numberOfVertices The number of vertices the application will write into the DVB.
 * @param forceDiscard Force the flush of any existing data and start from vertex 0.  Should be used for multistreaming with dynamic VBs only.
 * @see unlock()
 */

inline void DynamicVertexBuffer::lock(int numberOfVertices, bool forceDiscard)
{
	NOT_NULL(m_graphicsData);
	DEBUG_FATAL(m_data, ("VB already locked"));
	DEBUG_FATAL(ms_dynamicGlobalLocked, ("Locking two dynamic VBs at the same time"));

#ifdef _DEBUG
	++m_debugIteratorLockCount;
	ms_dynamicGlobalLocked = true;
	m_dynamicId = ++ms_dynamicGlobalId;
#endif
	m_numberOfVertices = numberOfVertices;
	m_data = reinterpret_cast<byte *>(m_graphicsData->lock(numberOfVertices, forceDiscard));
}

// ----------------------------------------------------------------------
/**
 * Let the DynamicVertexBuffer know it is done being modified.
 * @see lock()
 */

inline void DynamicVertexBuffer::unlock()
{
	DEBUG_FATAL(!m_data, ("VB not locked"));
#ifdef _DEBUG
	++m_debugIteratorLockCount;
	ms_dynamicGlobalLocked = false;
#endif
	m_graphicsData->unlock();
	m_data = NULL;
}

// ----------------------------------------------------------------------
/**
 * Let the DynamicVertexBuffer know it is done being modified.
 * @see lock()
 */

inline void DynamicVertexBuffer::unlock(int numberOfVertices)
{
	DEBUG_FATAL(!m_data, ("VB not locked"));
#ifdef _DEBUG
	++m_debugIteratorLockCount;
	ms_dynamicGlobalLocked = false;
#endif
	m_graphicsData->unlock(numberOfVertices);
	m_data = NULL;
}

// ----------------------------------------------------------------------
/**
 * Get a write-only iterator pointing to first vertex of the vertex buffer.
 * @return A write-only iterator pointing to first vertex of the vertex buffer.
 */

inline VertexBufferWriteIterator DynamicVertexBuffer::begin()
{
	DEBUG_FATAL(!m_data, ("Vertex buffer is not locked"));
	return VertexBufferWriteIterator(*this, m_data);
}

// ----------------------------------------------------------------------
/**
 * Return a write-only iterator pointing to one past end of the vertex buffer.
 * @return A write-only iterator pointing to one past end of the vertex buffer.
 */

inline VertexBufferWriteIterator DynamicVertexBuffer::end()
{
	DEBUG_FATAL(!m_data, ("Vertex buffer is not locked"));
	return VertexBufferWriteIterator(*this, m_data + m_numberOfVertices * m_descriptor->vertexSize);
}

// ======================================================================

#endif
