// ======================================================================
//
// DynamicIndexBuffer.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_DynamicIndexBuffer_H
#define INCLUDED_DynamicIndexBuffer_H

// ======================================================================

#include "clientGraphics/Graphics.def"
#include "clientGraphics/HardwareIndexBuffer.h"

// ======================================================================

class DynamicIndexBufferGraphicsData
{
public:
	virtual DLLEXPORT ~DynamicIndexBufferGraphicsData();

	virtual Index *lock(int numberOfIndices) = 0;
	virtual void   unlock() = 0;
};

class DynamicIndexBuffer : public HardwareIndexBuffer
{
	friend class  Graphics;
	friend struct Gl_api;
	friend class  Direct3d8;
	friend class  Direct3d8_StaticIndexBufferData;
	friend class  Direct3d9;
	friend class  Direct3d9_StaticIndexBufferData;

public:

#ifdef _DEBUG
	static int        getDynamicGlobalId();
#endif

public:

	DynamicIndexBuffer();
	virtual ~DynamicIndexBuffer();

	void          lock(int numberOfIndices);
	void          unlock() const;

#ifdef _DEBUG
	int           getDynamicId() const;
#endif

	Index        *begin();
	Index        *end();

	void          copyIndices(int destinationIndex, const Index *sourceIndexBuffer, int sourceIndex, int numberOfIndices);

private:

	DynamicIndexBuffer(const DynamicIndexBuffer &);
	DynamicIndexBuffer &operator =(const DynamicIndexBuffer &);

private:

#ifdef _DEBUG
	static int                 ms_dynamicId;
	static bool                ms_dynamicLocked;
#endif

private:

	DynamicIndexBufferGraphicsData * m_graphicsData;
	mutable int                      m_numberOfIndices;
	mutable Index *                  m_indexData;
#ifdef _DEBUG
	int                              m_dynamicId;
#endif
};

// ======================================================================
/**
 * Used to catch invalid use of a dynamic index buffer.
 * @internal
 */

#ifdef _DEBUG

inline int DynamicIndexBuffer::getDynamicGlobalId()
{
	return ms_dynamicId;
}

#endif

// ----------------------------------------------------------------------

/**
 * Used to catch invalid use of a dynamic index buffer.
 * @internal
 */

#ifdef _DEBUG

inline int DynamicIndexBuffer::getDynamicId(void) const
{
	return m_dynamicId;
}

#endif

// ----------------------------------------------------------------------
/**
 * Lock the index buffer so it may be modified.
 * @see unlock()
 */

inline void DynamicIndexBuffer::lock(int numberOfIndices)
{
	DEBUG_FATAL(m_indexData, ("IB already locked"));
	DEBUG_FATAL(ms_dynamicLocked, ("Locking two dynamic IBs at the same time"));
#ifdef _DEBUG
	ms_dynamicLocked = true;
	m_dynamicId = ++ms_dynamicId;
#endif
	m_numberOfIndices = numberOfIndices;
	m_indexData = m_graphicsData->lock(m_numberOfIndices);
}


// ----------------------------------------------------------------------
/**
 * Unlock the index buffer so it may be used to render.
 * @see lock()
 * @see lockReadOnly().
 */

inline void DynamicIndexBuffer::unlock() const
{
	DEBUG_FATAL(!m_indexData, ("IB not locked"));
#ifdef _DEBUG
	ms_dynamicLocked = false;
#endif
	m_graphicsData->unlock();
	m_indexData = NULL;
}

// ----------------------------------------------------------------------
/**
 * Get a pointer to the start of the index memory.
 * This pointer should only be written to, never read from.
 * @return Pointer to the start of the index memory.
 */

inline Index *DynamicIndexBuffer::begin()
{
	DEBUG_FATAL(!m_indexData, ("IB not locked"));
	return m_indexData;
}

// ----------------------------------------------------------------------
/**
 * Get a pointer to the start of the index memory.
 * This pointer should only be written to, never read from.
 * @return Pointer to the start of the index memory.
 */

inline Index *DynamicIndexBuffer::end()
{
	DEBUG_FATAL(!m_indexData, ("IB not locked"));
	return m_indexData + m_numberOfIndices;
}

// ======================================================================

#endif
