// ======================================================================
//
// VertexBuffer.h
//
// copyright 1998 Bootprint Entertainment
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_VertexBuffer_H
#define INCLUDED_VertexBuffer_H

// ======================================================================

class Iff;
class VertexBufferWriteIterator;
class VertexBufferReadIterator;

// ======================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"
#include "clientGraphics/VertexBufferFormat.h"
#include "clientGraphics/VertexBufferDescriptor.h"

// ======================================================================

/**
 * Provide access to vertex data with various data components.
 *
 * The actual layout of the vertex data in memory is determined by the Graphics layer.
 */

class VertexBuffer
{
	friend class VertexBufferBaseIterator;

public:

	virtual ~VertexBuffer();

	const VertexBufferFormat     &getFormat() const;
	const VertexBufferDescriptor &getVertexDescriptor() const;

	int                         getVertexSize() const;
	bool                        hasPosition() const;
	bool                        isTransformed() const;
	bool                        hasNormal() const;
	bool                        hasPointSize() const;
	bool                        hasColor0() const;
	bool                        hasColor1() const;
	int                         getNumberOfTextureCoordinateSets() const;
	int                         getTextureCoordinateSetDimension(int textureCoordinateSet) const;

protected:

	VertexBuffer(const VertexBufferFormat &format);
	VertexBuffer();
	
	void                              load(Iff &iff);
	virtual VertexBufferWriteIterator preLoad(int numberOfVertices) = 0;
	virtual void                      postLoad() = 0;

	void         write(Iff &iff, VertexBufferReadIterator v, int numberOfVertices) const;

private:

	// Disabled.
	VertexBuffer(const VertexBuffer &);
	VertexBuffer &operator =(const VertexBuffer &);

	void        load_0001(Iff &iff);
	void        load_0002(Iff &iff);
	void        load_0003(Iff &iff);

protected:

	VertexBufferFormat               m_format;
	const VertexBufferDescriptor    *m_descriptor;

#ifdef _DEBUG
	mutable int                      m_debugIteratorLockCount;
#endif
};

// ======================================================================
/**
 * Get the size of an individual vertex.
 */

inline int VertexBuffer::getVertexSize() const
{
	NOT_NULL(m_descriptor);
	return m_descriptor->vertexSize;
}

// ======================================================================
/**
 * Get a pointer to the vertex buffer's descriptor structure.
 */

inline const VertexBufferDescriptor &VertexBuffer::getVertexDescriptor() const
{
	NOT_NULL(m_descriptor);
	return *m_descriptor;
}

// ----------------------------------------------------------------------
/**
 * Get the Format of the vertexBuffer.
 */

inline const VertexBufferFormat &VertexBuffer::getFormat() const
{
	return m_format;
}

// ----------------------------------------------------------------------
/**
 * Test whether the VertexBuffer contains positional data.
 */

inline bool VertexBuffer::hasPosition() const
{
	return m_format.hasPosition();
}

// ----------------------------------------------------------------------
/**
 * Test whether the VertexBuffer contains transformed vertex data.
 */

inline bool VertexBuffer::isTransformed() const
{
	return m_format.isTransformed();
}

// ----------------------------------------------------------------------
/**
 * Test whether the VertexBuffer contains a normal per vertex.
 */

inline bool VertexBuffer::hasNormal() const
{
	return m_format.hasNormal();
}

// ----------------------------------------------------------------------
/**
 * Test whether the VertexBuffer contains a point size per vertex
 */

inline bool VertexBuffer::hasPointSize() const
{
	return m_format.hasPointSize();
}

// ----------------------------------------------------------------------
/**
 * Test whether the VertexBuffer contains a ARGB color0 data per vertex.
 */

inline bool VertexBuffer::hasColor0() const
{
	return m_format.hasColor0();
}

// ----------------------------------------------------------------------
/**
 * Test whether the VertexBuffer contains a ARGB color1 data per vertex.
 */

inline bool VertexBuffer::hasColor1() const
{
	return m_format.hasColor1();
}

// ----------------------------------------------------------------------
/**
 * Get the number of texture coordinate sets per vertex in this VertexBuffer.
 */

inline int VertexBuffer::getNumberOfTextureCoordinateSets() const
{
	return m_format.getNumberOfTextureCoordinateSets();
}

// ----------------------------------------------------------------------
/**
 * Get the number of UV sets per vertex in this VertexBuffer.
 */

inline int VertexBuffer::getTextureCoordinateSetDimension(int textureCoordinateSet) const
{
	return m_format.getTextureCoordinateSetDimension(textureCoordinateSet);
}

// ======================================================================

#endif
