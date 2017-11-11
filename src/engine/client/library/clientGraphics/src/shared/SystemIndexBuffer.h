// ======================================================================
//
// SystemIndexBuffer.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_SystemIndexBuffer_H
#define INCLUDED_SystemIndexBuffer_H

// ======================================================================

#include "clientGraphics/Graphics.def"

// ======================================================================

class SystemIndexBuffer
{
public:

	SystemIndexBuffer(int numberOfIndices);
	~SystemIndexBuffer();

	int           getNumberOfIndices() const;

	const Index  *beginReadOnly() const;
	const Index  *endReadOnly() const;

	Index        *begin();
	Index        *end();
	
	Index        &operator[](int index);
	Index         operator[](int index) const;

	void          copyIndices(int destinationIndex, Index const * sourceIndexBuffer, int sourceIndex, int numberOfIndices);

private:

	// Disabled
	SystemIndexBuffer();
	SystemIndexBuffer(const SystemIndexBuffer &);
	SystemIndexBuffer &operator =(const SystemIndexBuffer &);

private:

	int                m_numberOfIndices;
	Index             *m_data;
};

// ======================================================================
/**
 * Get the number of indices in this index buffer.
 * @return The number of indices in this index buffer.
 */

inline int SystemIndexBuffer::getNumberOfIndices() const
{
	return m_numberOfIndices;
}

// ----------------------------------------------------------------------
/**
 * Get a pointer to the start of the index memory.
 * @return Pointer to the start of the index memory.
 */

inline const Index *SystemIndexBuffer::beginReadOnly() const
{
	return m_data;
}

// ----------------------------------------------------------------------
/**
 * Get a pointer to one past the end of the index memory.
 * @return Pointer to one past the end of the index memory.
 */

inline const Index *SystemIndexBuffer::endReadOnly() const
{
	return m_data + m_numberOfIndices;
}

// ----------------------------------------------------------------------
/**
 * Get a pointer to the start of the index memory.
 * @return Pointer to the start of the index memory.
 */

inline Index *SystemIndexBuffer::begin()
{
	return m_data;
}

// ----------------------------------------------------------------------
/**
 * Get a pointer to one past the end of the index memory.
 * @return Pointer to one past the end of the index memory.
 */

inline Index *SystemIndexBuffer::end()
{
	return m_data + m_numberOfIndices;
}

// ----------------------------------------------------------------------
/**
 * Access a specific index.
 * @return Pointer to one past the end of the index memory.
 */

inline Index &SystemIndexBuffer::operator[](int index)
{
	DEBUG_FATAL(index < 0 || index >= m_numberOfIndices, ("Index out of range %d/%d", index, m_numberOfIndices));
	return m_data[index];
}

// ----------------------------------------------------------------------
/**
 * Access a specific index.
 * @return Pointer to one past the end of the index memory.
 */

inline Index SystemIndexBuffer::operator[](int index) const
{
	DEBUG_FATAL(index < 0 || index >= m_numberOfIndices, ("Index out of range %d/%d", index, m_numberOfIndices));
	return m_data[index];
}

// ======================================================================

#endif
