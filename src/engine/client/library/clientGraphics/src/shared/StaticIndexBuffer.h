// ======================================================================
//
// StaticIndexBuffer.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_StaticIndexBuffer_H
#define INCLUDED_StaticIndexBuffer_H

// ======================================================================

#include "clientGraphics/Graphics.def"
#include "clientGraphics/HardwareIndexBuffer.h"

// ======================================================================

class StaticIndexBufferGraphicsData
{
public:
	virtual DLLEXPORT ~StaticIndexBufferGraphicsData();

	virtual Index *lock(bool readOnly) = 0;
	virtual void   unlock() = 0;
};

class StaticIndexBuffer : public HardwareIndexBuffer
{
	friend class  Graphics;
	friend struct Gl_api;
	friend class  Direct3d8;
	friend class  Direct3d8_StaticIndexBufferData;
	friend class  Direct3d9;
	friend class  Direct3d9_StaticIndexBufferData;
	friend class  Direct3d11;
	friend class  Direct3d11_StaticIndexBufferData;

public:

	DLLEXPORT StaticIndexBuffer(int numberOfIndices);
	virtual ~StaticIndexBuffer();

	int               getNumberOfIndices() const;

	DLLEXPORT void    lock();
	void              lockReadOnly() const;
	DLLEXPORT void    unlock() const;

	const Index      *beginReadOnly() const;
	const Index      *endReadOnly() const;

	DLLEXPORT Index  *begin();
	Index            *end();

private:

	void lock(bool readOnly) const;

private:

	// Disabled
	StaticIndexBuffer();
	StaticIndexBuffer(const StaticIndexBuffer &);
	StaticIndexBuffer &operator =(const StaticIndexBuffer &);

private:

	int                             m_numberOfIndices;
	StaticIndexBufferGraphicsData * m_graphicsData;
	mutable Index *                 m_indexData;
	mutable bool                    m_readOnly;
};

// ======================================================================
/**
 * Get the number of indices in this index buffer.
 * @return The number of indices in this index buffer.
 */

inline int StaticIndexBuffer::getNumberOfIndices() const
{
	return m_numberOfIndices;
}

// ----------------------------------------------------------------------
/**
 * @internal
 */

inline void StaticIndexBuffer::lock(bool readOnly) const 
{
	DEBUG_FATAL(m_indexData, ("IB already locked"));
	m_readOnly = readOnly;
	m_indexData = m_graphicsData->lock(m_readOnly);
}

// ----------------------------------------------------------------------
/**
 * Lock the index buffer so it may be modified.
 * @see unlock()
 */

inline void StaticIndexBuffer::lock()
{
	lock(false);
}

// ----------------------------------------------------------------------
/**
 * Lock the index buffer so it may be read.
 * @see unlock()
 */

inline void StaticIndexBuffer::lockReadOnly() const
{
	lock(true);
}

// ----------------------------------------------------------------------
/**
 * Unlock the index buffer so it may be used to render.
 * @see lock()
 * @see lockReadOnly().
 */

inline void StaticIndexBuffer::unlock() const
{
	DEBUG_FATAL(!m_indexData, ("IB not locked"));
	m_graphicsData->unlock();
	m_indexData = NULL;
}

// ----------------------------------------------------------------------
/**
 * Get a pointer to the start of the index memory.
 * @return Pointer to the start of the index memory.
 */

inline Index *StaticIndexBuffer::begin()
{
	DEBUG_FATAL(!m_indexData, ("IB not locked"));
	DEBUG_FATAL(m_readOnly, ("IB locked read-only"));
	return m_indexData;
}

// ----------------------------------------------------------------------
/**
 * Get a pointer to the start of the index memory.
 * @return Pointer to the start of the index memory.
 */

inline Index *StaticIndexBuffer::end()
{
	DEBUG_FATAL(!m_indexData, ("IB not locked"));
	DEBUG_FATAL(m_readOnly, ("IB locked read-only"));
	return m_indexData + m_numberOfIndices;
}

// ----------------------------------------------------------------------
/**
 * Get a pointer to the start of the index memory.
 * @return Pointer to the start of the index memory.
 */

inline const Index *StaticIndexBuffer::beginReadOnly() const
{
	DEBUG_FATAL(!m_indexData, ("IB not locked"));
	return m_indexData;
}

// ----------------------------------------------------------------------
/**
 * Get a pointer to the start of the index memory.
 * @return Pointer to the start of the index memory.
 */

inline const Index *StaticIndexBuffer::endReadOnly() const
{
	DEBUG_FATAL(!m_indexData, ("IB not locked"));
	return m_indexData + m_numberOfIndices;
}

// ======================================================================

#endif
