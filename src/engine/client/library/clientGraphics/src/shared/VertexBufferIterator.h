// ======================================================================
//
// VertexBufferIterator.h
// Copyright 2001-2004 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_VertexBufferIterator_H
#define INCLUDED_VertexBufferIterator_H

// ======================================================================

#include "sharedMath/PackedArgb.h"
#include "sharedMath/Vector.h"
#include "clientGraphics/VertexBuffer.h"

// ======================================================================

class VertexBufferReadWriteIterator;

class VertexBufferBaseIterator
{
public:

	bool                  operator ==(const VertexBufferBaseIterator &rhs) const;
	bool                  operator !=(const VertexBufferBaseIterator &rhs) const;

	const VertexBufferDescriptor *getDescriptor() const { return m_descriptor; }

protected:

	VertexBufferBaseIterator(const VertexBuffer &vertexBuffer, byte *data);
	VertexBufferBaseIterator(const VertexBufferBaseIterator &rhs);
	virtual ~VertexBufferBaseIterator();

	void checkLock() const;
	void assign(const VertexBufferBaseIterator &rhs);
	void increment();
	void decrement();

protected:

	const VertexBuffer             *m_vertexBuffer;
	const VertexBufferDescriptor   *m_descriptor;
	byte                           *m_data;

#ifdef _DEBUG
	int                             m_locks;
#endif

protected:

	// Disabled.
	VertexBufferBaseIterator();
};

class VertexBufferReadIterator : public VertexBufferBaseIterator
{
	friend class StaticVertexBuffer;
	friend class SystemVertexBuffer;
	friend class VertexBufferWriteIterator;

public:

	VertexBufferReadIterator(const VertexBufferReadIterator &rhs);
	VertexBufferReadIterator(const VertexBufferReadWriteIterator &rhs);
	virtual ~VertexBufferReadIterator();

	VertexBufferReadIterator &operator =(const VertexBufferReadIterator &rhs);
	VertexBufferReadIterator &operator =(const VertexBufferReadWriteIterator &rhs);
	VertexBufferReadIterator &operator ++();
	VertexBufferReadIterator &operator --();
	VertexBufferReadIterator &operator -=(int offset);
	VertexBufferReadIterator &operator +=(int offset);
	VertexBufferReadIterator  operator +(int offset) const;
	VertexBufferReadIterator  operator -(int offset) const;

	const Vector     &getPosition() const;
	float             getOoz() const;
	const Vector     &getNormal() const;
	float             getPointSize() const;
	const PackedArgb &getColor0() const;
	const PackedArgb &getColor1() const;
	int               getNumberOfTextureCoordinateSets() const;
	int               getTextureCoordinateSetDimension(int textureCoordinateSet) const;
	float             getTextureCoordinate(int setIndex, int coordinateIndex) const;
	void              getTextureCoordinates(int setIndex, float &tc0) const;
	void              getTextureCoordinates(int setIndex, float &tc0, float &tc1) const;
	void              getTextureCoordinates(int setIndex, float &tc0, float &tc1, float &tc2) const;
	void              getTextureCoordinates(int setIndex, float &tc0, float &tc1, float &tc2, float &tc3) const;

	const void       *getDataPointer() const;

protected:

	VertexBufferReadIterator(const VertexBuffer &vertexBuffer, byte *data);

private:

	// disabled
	VertexBufferReadIterator();
};
 
class VertexBufferWriteIterator : public  VertexBufferBaseIterator
{
	friend class DynamicVertexBuffer;
	friend class StaticVertexBuffer;
	friend class SystemVertexBuffer;

public: 
 
	VertexBufferWriteIterator(); 
	VertexBufferWriteIterator(const VertexBufferWriteIterator &rhs);
	VertexBufferWriteIterator(const VertexBufferReadWriteIterator &rhs);
	virtual ~VertexBufferWriteIterator();

	VertexBufferWriteIterator &operator =(const VertexBufferWriteIterator &rhs);
	VertexBufferWriteIterator &operator =(const VertexBufferReadWriteIterator &rhs);
	VertexBufferWriteIterator &operator ++();
	VertexBufferWriteIterator &operator --();
	VertexBufferWriteIterator &operator -=(int offset);
	VertexBufferWriteIterator &operator +=(int offset);
	VertexBufferWriteIterator  operator +(int offset) const;
	VertexBufferWriteIterator  operator -(int offset) const;
 
	void setPosition(const Vector &position);
	void setPosition(float x, float y, float z);
	void setPositionX(float x);
	void setPositionY(float y);
	void setPositionZ(float z);
	void setOoz(float ooz);
	void setNormal(const Vector &normal);
	void setNormal(float x, float y, float z);
	void setNormalX(float x);
	void setNormalY(float y);
	void setNormalZ(float z);
	void setPointSize(float size);
	void setColor0(uint32 color0);
	void setColor1(uint32 color1);
	void setColor0(const PackedArgb &color0);
	void setColor1(const PackedArgb &color1);
	void setTextureCoordinate(int setIndex, int coordinateIndex, float value);
	void setTextureCoordinates(int setIndex, float tc0);
	void setTextureCoordinates(int setIndex, float tc0, float tc1);
	void setTextureCoordinates(int setIndex, float tc0, float tc1, float tc2);
	void setTextureCoordinates(int setIndex, const Vector &tcs012);
	void setTextureCoordinates(int setIndex, float tc0, float tc1, float tc2, float tc3);

	void copy(const VertexBufferReadIterator &source, int count=1);

	void *getDataPointer();

protected:

	VertexBufferWriteIterator(const VertexBuffer &vertexBuffer, byte *data);
};

class VertexBufferReadWriteIterator : public VertexBufferBaseIterator
{
	friend class StaticVertexBuffer;
	friend class SystemVertexBuffer;

public:

	VertexBufferReadWriteIterator(const VertexBufferReadWriteIterator &rhs);
	virtual ~VertexBufferReadWriteIterator();

	VertexBufferReadWriteIterator &operator =(const VertexBufferReadWriteIterator &rhs);
	VertexBufferReadWriteIterator &operator ++();
	VertexBufferReadWriteIterator &operator --();
	VertexBufferReadWriteIterator &operator -=(int offset);
	VertexBufferReadWriteIterator &operator +=(int offset);
	VertexBufferReadWriteIterator  operator +(int offset) const;
	VertexBufferReadWriteIterator  operator -(int offset) const;

	const Vector     &getPosition() const;
	float             getOoz() const;
	const Vector     &getNormal() const;
	float             getPointSize() const;
	const PackedArgb &getColor0() const;
	const PackedArgb &getColor1() const;
	int               getNumberOfTextureCoordinateSets() const;
	int               getTextureCoordinateSetDimension(int textureCoordinateSet) const;
	float             getTextureCoordinate(int setIndex, int coordinateIndex) const;
	void              getTextureCoordinates(int setIndex, float &tc0) const;
	void              getTextureCoordinates(int setIndex, float &tc0, float &tc1) const;
	void              getTextureCoordinates(int setIndex, float &tc0, float &tc1, float &tc2) const;
	void              getTextureCoordinates(int setIndex, float &tc0, float &tc1, float &tc2, float &tc3) const;

	void              setPosition(const Vector &position);
	void              setPosition(float x, float y, float z);
	void              setPositionX(float x);
	void              setPositionY(float y);
	void              setPositionZ(float z);
	void              setOoz(float ooz);
	void              setNormal(const Vector &normal);
	void              setNormal(float x, float y, float z);
	void              setNormalX(float x);
	void              setNormalY(float y);
	void              setNormalZ(float z);
	void              setPointSize(float size);
	void              setColor0(uint32 color0);
	void              setColor1(uint32 color1);
	void              setColor0(const PackedArgb &color0);
	void              setColor1(const PackedArgb &color1);
	void              setTextureCoordinate(int setIndex, int coordinateIndex, float value);
	void              setTextureCoordinates(int setIndex, float tc0);
	void              setTextureCoordinates(int setIndex, float tc0, float tc1);
	void              setTextureCoordinates(int setIndex, float tc0, float tc1, float tc2);
	void              setTextureCoordinates(int setIndex, const Vector &tcs012);
	void              setTextureCoordinates(int setIndex, float tc0, float tc1, float tc2, float tc3);

	void              copy(const VertexBufferReadIterator &source, int count=1);

	const void       *getDataPointer() const;
	void             *getDataPointer();

private:

	VertexBufferReadWriteIterator(const VertexBuffer &vertexBuffer, byte *data);

private:

	// disabled
	VertexBufferReadWriteIterator(); 
};

// ======================================================================

inline VertexBufferBaseIterator::VertexBufferBaseIterator()
:
	m_vertexBuffer(NULL),
	m_descriptor(NULL),
	m_data(NULL)
#ifdef _DEBUG
	,
	m_locks(0)
#endif
{
}

// ----------------------------------------------------------------------

inline VertexBufferBaseIterator::VertexBufferBaseIterator(const VertexBuffer &vertexBuffer, byte *data)
:
	m_vertexBuffer(&vertexBuffer),
	m_descriptor(vertexBuffer.m_descriptor),
	m_data(data)
#ifdef _DEBUG
	,
	m_locks(m_vertexBuffer->m_debugIteratorLockCount)
#endif
{
	NOT_NULL(m_data);
}

// ----------------------------------------------------------------------

inline VertexBufferBaseIterator::VertexBufferBaseIterator(const VertexBufferBaseIterator &rhs)
:
	m_vertexBuffer(rhs.m_vertexBuffer),
	m_descriptor(rhs.m_descriptor),
	m_data(rhs.m_data)
#ifdef _DEBUG
	,
	m_locks(rhs.m_locks)
#endif
{
	rhs.checkLock();
	NOT_NULL(m_data);
}

// ----------------------------------------------------------------------

inline bool VertexBufferBaseIterator::operator ==(const VertexBufferBaseIterator &rhs) const
{
	checkLock();
	rhs.checkLock();
	return m_data == rhs.m_data;
}

// ----------------------------------------------------------------------

inline bool VertexBufferBaseIterator::operator !=(const VertexBufferBaseIterator &rhs) const
{
	return !(*this == rhs);
}

// ----------------------------------------------------------------------

inline void VertexBufferBaseIterator::checkLock() const
{
#ifdef _DEBUG
	DEBUG_FATAL(m_locks != m_vertexBuffer->m_debugIteratorLockCount, ("Attempt to use old iterator from previous lock"));
#endif
}

// ----------------------------------------------------------------------

inline void VertexBufferBaseIterator::assign(const VertexBufferBaseIterator &rhs)
{
	rhs.checkLock();
	if (this != &rhs)
	{
		m_vertexBuffer = rhs.m_vertexBuffer;
		m_descriptor = rhs.m_descriptor;
		m_data = rhs.m_data;
#ifdef _DEBUG
		m_locks = rhs.m_locks;
#endif
	}
}

// ----------------------------------------------------------------------

inline void VertexBufferBaseIterator::increment()
{
	checkLock();
	m_data += m_descriptor->vertexSize;
}

// ----------------------------------------------------------------------

inline void VertexBufferBaseIterator::decrement()
{
	checkLock();
	m_data -= m_descriptor->vertexSize;
}

// ======================================================================

inline VertexBufferReadIterator::VertexBufferReadIterator(const VertexBuffer &vertexBuffer, byte *data)
:	VertexBufferBaseIterator(vertexBuffer, data)
{
}

// ----------------------------------------------------------------------

inline VertexBufferReadIterator::VertexBufferReadIterator(const VertexBufferReadIterator &rhs)
:	VertexBufferBaseIterator(rhs)
{
}

// ----------------------------------------------------------------------

inline VertexBufferReadIterator::VertexBufferReadIterator(const VertexBufferReadWriteIterator &rhs)
:	VertexBufferBaseIterator(rhs)
{
}

// ----------------------------------------------------------------------

inline VertexBufferReadIterator &VertexBufferReadIterator::operator =(const VertexBufferReadIterator &rhs)
{
	assign(rhs);
	return *this;
}

// ----------------------------------------------------------------------

inline VertexBufferReadIterator &VertexBufferReadIterator::operator =(const VertexBufferReadWriteIterator &rhs)
{
	assign(rhs);
	return *this;
}

// ----------------------------------------------------------------------

inline VertexBufferReadIterator &VertexBufferReadIterator::operator ++()
{
	increment();
	return *this;
}

// ----------------------------------------------------------------------

inline VertexBufferReadIterator &VertexBufferReadIterator::operator --()
{
	decrement();
	return *this;
}

// ----------------------------------------------------------------------

inline VertexBufferReadIterator &VertexBufferReadIterator::operator +=(int offset)
{
	checkLock();
	m_data += m_descriptor->vertexSize * offset;
	return *this;
}

// ----------------------------------------------------------------------

inline VertexBufferReadIterator &VertexBufferReadIterator::operator -=(int offset)
{
	checkLock();
	m_data -= m_descriptor->vertexSize * offset;
	return *this;
}

// ----------------------------------------------------------------------

inline VertexBufferReadIterator VertexBufferReadIterator::operator +(int offset) const
{
	checkLock();
	return VertexBufferReadIterator(*m_vertexBuffer, m_data + (offset * m_descriptor->vertexSize));
}

// ----------------------------------------------------------------------

inline VertexBufferReadIterator VertexBufferReadIterator::operator -(int offset) const
{
	checkLock();
	return VertexBufferReadIterator(*m_vertexBuffer, m_data - (offset * m_descriptor->vertexSize));
}

// ----------------------------------------------------------------------

inline const Vector &VertexBufferReadIterator::getPosition() const
{
	checkLock();
	DEBUG_FATAL(!m_vertexBuffer->hasPosition(), ("VB does not have position"));
	return *reinterpret_cast<const Vector*>(m_data + m_descriptor->offsetPosition);
}

// ----------------------------------------------------------------------

inline float VertexBufferReadIterator::getOoz() const
{
	checkLock();
	DEBUG_FATAL(!m_vertexBuffer->isTransformed(), ("VB is not transformed"));
	return *reinterpret_cast<const float*>(m_data + m_descriptor->offsetOoz);
}

// ----------------------------------------------------------------------

inline const Vector &VertexBufferReadIterator::getNormal() const
{
	checkLock();
	DEBUG_FATAL(!m_vertexBuffer->hasNormal(), ("VB does not have normal"));
	return *reinterpret_cast<const Vector*>(m_data + m_descriptor->offsetNormal);
}

// ----------------------------------------------------------------------

inline float VertexBufferReadIterator::getPointSize() const
{
	checkLock();
	DEBUG_FATAL(!m_vertexBuffer->hasPointSize(), ("VB does not have point size"));
	return *reinterpret_cast<const float*>(m_data + m_descriptor->offsetPointSize);
}

// ----------------------------------------------------------------------

inline const PackedArgb &VertexBufferReadIterator::getColor0() const
{
	checkLock();
	DEBUG_FATAL(!m_vertexBuffer->hasColor0(), ("VB does not have color 0"));
	return *reinterpret_cast<const PackedArgb *>(m_data + m_descriptor->offsetColor0);
}

// ----------------------------------------------------------------------

inline const PackedArgb &VertexBufferReadIterator::getColor1() const
{
	checkLock();
	DEBUG_FATAL(!m_vertexBuffer->hasColor1(), ("VB does not have color 1"));
	return *reinterpret_cast<const PackedArgb *>(m_data + m_descriptor->offsetColor1);
}

// ----------------------------------------------------------------------

inline int VertexBufferReadIterator::getNumberOfTextureCoordinateSets() const
{
	return m_vertexBuffer->getNumberOfTextureCoordinateSets();
}

// ----------------------------------------------------------------------

inline int VertexBufferReadIterator::getTextureCoordinateSetDimension(int textureCoordinateSet) const
{
	return m_vertexBuffer->getTextureCoordinateSetDimension(textureCoordinateSet);
}

// ----------------------------------------------------------------------

inline float VertexBufferReadIterator::getTextureCoordinate(int setIndex, int coordinateIndex) const
{
	checkLock();
	DEBUG_FATAL(setIndex >= m_vertexBuffer->getNumberOfTextureCoordinateSets(), ("VB does not have enough texture coordinates %d/%d", setIndex, m_vertexBuffer->getNumberOfTextureCoordinateSets()));
	DEBUG_FATAL(coordinateIndex >= m_vertexBuffer->getTextureCoordinateSetDimension(setIndex), ("VB texture coordinate set %d is too low dimension %d/%d", setIndex, coordinateIndex, m_vertexBuffer->getTextureCoordinateSetDimension(setIndex)));
	return reinterpret_cast<const float*>(m_data + m_descriptor->offsetTextureCoordinateSet[setIndex])[coordinateIndex];
}

// ----------------------------------------------------------------------

inline void VertexBufferReadIterator::getTextureCoordinates(int setIndex, float &tc0) const
{
	checkLock();
	DEBUG_FATAL(setIndex >= m_vertexBuffer->getNumberOfTextureCoordinateSets(), ("VB does not have enough texture coordinates %d/%d", setIndex, m_vertexBuffer->getNumberOfTextureCoordinateSets()));
	DEBUG_FATAL(m_vertexBuffer->getTextureCoordinateSetDimension(setIndex) != 1, ("VB texture coordinate set %d is not of proper dimension 1/%d", setIndex, m_vertexBuffer->getTextureCoordinateSetDimension(setIndex)));
	const float * const tcs = reinterpret_cast<const float*>(m_data + m_descriptor->offsetTextureCoordinateSet[setIndex]);
	tc0 = tcs[0];
}

// ----------------------------------------------------------------------

inline void VertexBufferReadIterator::getTextureCoordinates(int setIndex, float &tc0, float &tc1) const
{
	checkLock();
	DEBUG_FATAL(setIndex >= m_vertexBuffer->getNumberOfTextureCoordinateSets(), ("VB does not have enough texture coordinates %d/%d", setIndex, m_vertexBuffer->getNumberOfTextureCoordinateSets()));
	DEBUG_FATAL(m_vertexBuffer->getTextureCoordinateSetDimension(setIndex) != 2, ("VB texture coordinate set %d is not of proper dimension 2/%d", setIndex, m_vertexBuffer->getTextureCoordinateSetDimension(setIndex)));
	const float * const tcs = reinterpret_cast<const float*>(m_data + m_descriptor->offsetTextureCoordinateSet[setIndex]);
	tc0 = tcs[0];
	tc1 = tcs[1];
}

// ----------------------------------------------------------------------

inline void VertexBufferReadIterator::getTextureCoordinates(int setIndex, float &tc0, float &tc1, float &tc2) const
{
	checkLock();
	DEBUG_FATAL(setIndex >= m_vertexBuffer->getNumberOfTextureCoordinateSets(), ("VB does not have enough texture coordinates %d/%d", setIndex, m_vertexBuffer->getNumberOfTextureCoordinateSets()));
	DEBUG_FATAL(m_vertexBuffer->getTextureCoordinateSetDimension(setIndex) != 3, ("VB texture coordinate set %d is not of proper dimension 3/%d", setIndex, m_vertexBuffer->getTextureCoordinateSetDimension(setIndex)));
	const float * const tcs = reinterpret_cast<const float*>(m_data + m_descriptor->offsetTextureCoordinateSet[setIndex]);
	tc0 = tcs[0];
	tc1 = tcs[1];
	tc2 = tcs[2];
}

// ----------------------------------------------------------------------

inline void VertexBufferReadIterator::getTextureCoordinates(int setIndex, float &tc0, float &tc1, float &tc2, float &tc3) const
{
	checkLock();
	DEBUG_FATAL(setIndex >= m_vertexBuffer->getNumberOfTextureCoordinateSets(), ("VB does not have enough texture coordinates %d/%d", setIndex, m_vertexBuffer->getNumberOfTextureCoordinateSets()));
	DEBUG_FATAL(m_vertexBuffer->getTextureCoordinateSetDimension(setIndex) != 4, ("VB texture coordinate set %d is not of proper dimension 4/%d", setIndex, m_vertexBuffer->getTextureCoordinateSetDimension(setIndex)));
	const float * const tcs = reinterpret_cast<const float*>(m_data + m_descriptor->offsetTextureCoordinateSet[setIndex]);
	tc0 = tcs[0];
	tc1 = tcs[1];
	tc2 = tcs[2];
	tc3 = tcs[3];
}

// ----------------------------------------------------------------------
/**
 * Retrieve a pointer to the beginning of raw data for this vertex.
 *
 * Do not use this function unless you know exactly what you are doing
 * and require access to data through a different interface.
 */

inline const void *VertexBufferReadIterator::getDataPointer() const
{
	return m_data;
}

// ======================================================================

inline VertexBufferWriteIterator::VertexBufferWriteIterator()
:	VertexBufferBaseIterator()
{
}

// ----------------------------------------------------------------------

inline VertexBufferWriteIterator::VertexBufferWriteIterator(const VertexBuffer &vertexBuffer, byte *data)
:	VertexBufferBaseIterator(vertexBuffer, data)
{
}

// ----------------------------------------------------------------------

inline VertexBufferWriteIterator::VertexBufferWriteIterator(const VertexBufferWriteIterator &rhs)
:	VertexBufferBaseIterator(rhs)
{
}

// ----------------------------------------------------------------------

inline VertexBufferWriteIterator::VertexBufferWriteIterator(const VertexBufferReadWriteIterator &rhs)
:	VertexBufferBaseIterator(rhs)
{
}

// ----------------------------------------------------------------------

inline VertexBufferWriteIterator &VertexBufferWriteIterator::operator =(const VertexBufferWriteIterator &rhs)
{
	assign(rhs);
	return *this;
}

// ----------------------------------------------------------------------

inline VertexBufferWriteIterator &VertexBufferWriteIterator::operator =(const VertexBufferReadWriteIterator &rhs)
{
	assign(rhs);
	return *this;
}

// ----------------------------------------------------------------------

inline VertexBufferWriteIterator &VertexBufferWriteIterator::operator ++()
{
	increment();
	return *this;
}

// ----------------------------------------------------------------------

inline VertexBufferWriteIterator &VertexBufferWriteIterator::operator --()
{
	decrement();
	return *this;
}

// ----------------------------------------------------------------------

inline VertexBufferWriteIterator &VertexBufferWriteIterator::operator +=(int offset)
{
	checkLock();
	m_data += m_descriptor->vertexSize * offset;
	return *this;
}

// ----------------------------------------------------------------------

inline VertexBufferWriteIterator &VertexBufferWriteIterator::operator -=(int offset)
{
	checkLock();
	m_data -= m_descriptor->vertexSize * offset;
	return *this;
}

// ----------------------------------------------------------------------

inline VertexBufferWriteIterator VertexBufferWriteIterator::operator +(int offset) const
{
	checkLock();
	return VertexBufferWriteIterator(*m_vertexBuffer, m_data + (offset * m_descriptor->vertexSize));
}

// ----------------------------------------------------------------------

inline VertexBufferWriteIterator VertexBufferWriteIterator::operator -(int offset) const
{
	checkLock();
	return VertexBufferWriteIterator(*m_vertexBuffer, m_data - (offset * m_descriptor->vertexSize));
}

// ----------------------------------------------------------------------

inline void VertexBufferWriteIterator::setPosition(const Vector &position)
{
	checkLock();
	DEBUG_FATAL(!m_vertexBuffer->hasPosition(), ("VB does not have position"));
	*reinterpret_cast<Vector*>(m_data + m_descriptor->offsetPosition) = position;
}

// ----------------------------------------------------------------------

inline void VertexBufferWriteIterator::setPosition(float x, float y, float z)
{
	checkLock();
	DEBUG_FATAL(!m_vertexBuffer->hasPosition(), ("VB does not have position"));
	float * const pos = reinterpret_cast<float*>(m_data + m_descriptor->offsetPosition);
	pos[0] = x;
	pos[1] = y;
	pos[2] = z;
}

// ----------------------------------------------------------------------

inline void VertexBufferWriteIterator::setPositionX(float x)
{
	checkLock();
	DEBUG_FATAL(!m_vertexBuffer->hasPosition(), ("VB does not have position"));
	reinterpret_cast<float*>(m_data + m_descriptor->offsetPosition)[0] = x;
}

// ----------------------------------------------------------------------

inline void VertexBufferWriteIterator::setPositionY(float y)
{
	checkLock();
	DEBUG_FATAL(!m_vertexBuffer->hasPosition(), ("VB does not have position"));
	reinterpret_cast<float*>(m_data + m_descriptor->offsetPosition)[1] = y;
}

// ----------------------------------------------------------------------

inline void VertexBufferWriteIterator::setPositionZ(float z)
{
	checkLock();
	DEBUG_FATAL(!m_vertexBuffer->hasPosition(), ("VB does not have position"));
	reinterpret_cast<float*>(m_data + m_descriptor->offsetPosition)[2] = z;
}

// ----------------------------------------------------------------------

inline void VertexBufferWriteIterator::setOoz(float ooz)
{
	checkLock();
	DEBUG_FATAL(!m_vertexBuffer->isTransformed(), ("VB does is not transformed"));
	*reinterpret_cast<float*>(m_data + m_descriptor->offsetOoz) = ooz;
}

// ----------------------------------------------------------------------

inline void VertexBufferWriteIterator::setNormal(const Vector &normal)
{
	checkLock();
	DEBUG_FATAL(!m_vertexBuffer->hasNormal(), ("VB does not have normal"));
	*reinterpret_cast<Vector*>(m_data + m_descriptor->offsetNormal) = normal;
}

// ----------------------------------------------------------------------

inline void VertexBufferWriteIterator::setNormal(float x, float y, float z)
{
	checkLock();
	DEBUG_FATAL(!m_vertexBuffer->hasNormal(), ("VB does not have normal"));
	float * const n = reinterpret_cast<float*>(m_data + m_descriptor->offsetNormal);
	n[0] = x;
	n[1] = y;
	n[2] = z;
}

// ----------------------------------------------------------------------

inline void VertexBufferWriteIterator::setNormalX(float x)
{
	checkLock();
	DEBUG_FATAL(!m_vertexBuffer->hasNormal(), ("VB does not have normal"));
	reinterpret_cast<float*>(m_data + m_descriptor->offsetNormal)[0] = x;
}

// ----------------------------------------------------------------------

inline void VertexBufferWriteIterator::setNormalY(float y)
{
	checkLock();
	DEBUG_FATAL(!m_vertexBuffer->hasNormal(), ("VB does not have normal"));
	reinterpret_cast<float*>(m_data + m_descriptor->offsetNormal)[1] = y;
}

// ----------------------------------------------------------------------

inline void VertexBufferWriteIterator::setNormalZ(float z)
{
	checkLock();
	DEBUG_FATAL(!m_vertexBuffer->hasNormal(), ("VB does not have normal"));
	reinterpret_cast<float*>(m_data + m_descriptor->offsetNormal)[2] = z;
}

// ----------------------------------------------------------------------

inline void VertexBufferWriteIterator::setPointSize(float size)
{
	checkLock();
	DEBUG_FATAL(!m_vertexBuffer->hasPointSize(), ("VB does not have point size"));
	*reinterpret_cast<float*>(m_data + m_descriptor->offsetPointSize) = size;
}

// ----------------------------------------------------------------------

inline void VertexBufferWriteIterator::setColor0(uint32 color0)
{
	checkLock();
	DEBUG_FATAL(!m_vertexBuffer->hasColor0(), ("VB does not have color 0"));
	*reinterpret_cast<uint32 *>(m_data + m_descriptor->offsetColor0) = color0;
}

// ----------------------------------------------------------------------

inline void VertexBufferWriteIterator::setColor1(uint32 color1)
{
	checkLock();
	DEBUG_FATAL(!m_vertexBuffer->hasColor1(), ("VB does not have color 1"));
	*reinterpret_cast<uint32 *>(m_data + m_descriptor->offsetColor1) = color1;
}

// ----------------------------------------------------------------------

inline void VertexBufferWriteIterator::setColor0(const PackedArgb &color0)
{
	checkLock();
	DEBUG_FATAL(!m_vertexBuffer->hasColor0(), ("VB does not have color 0"));
	*reinterpret_cast<PackedArgb *>(m_data + m_descriptor->offsetColor0) = color0;
}

// ----------------------------------------------------------------------

inline void VertexBufferWriteIterator::setColor1(const PackedArgb &color1)
{
	checkLock();
	DEBUG_FATAL(!m_vertexBuffer->hasColor1(), ("VB does not have color 1"));
	*reinterpret_cast<PackedArgb *>(m_data + m_descriptor->offsetColor1) = color1;
}

// ----------------------------------------------------------------------

inline void VertexBufferWriteIterator::setTextureCoordinate(int setIndex, int coordinateIndex, float value)
{
	checkLock();
	DEBUG_FATAL(setIndex >= m_vertexBuffer->getNumberOfTextureCoordinateSets(), ("VB does not have enough texture coordinates %d/%d", setIndex, m_vertexBuffer->getNumberOfTextureCoordinateSets()));
	DEBUG_FATAL(coordinateIndex >= m_vertexBuffer->getTextureCoordinateSetDimension(setIndex), ("VB texture coordinate set %d is too low dimension %d/%d", setIndex, coordinateIndex, m_vertexBuffer->getTextureCoordinateSetDimension(setIndex)));
	reinterpret_cast<float*>(m_data + m_descriptor->offsetTextureCoordinateSet[setIndex])[coordinateIndex] = value;
}

// ----------------------------------------------------------------------

inline void VertexBufferWriteIterator::setTextureCoordinates(int setIndex, float tc0)
{
	checkLock();
	DEBUG_FATAL(setIndex >= m_vertexBuffer->getNumberOfTextureCoordinateSets(), ("VB does not have enough texture coordinates %d/%d", setIndex, m_vertexBuffer->getNumberOfTextureCoordinateSets()));
	DEBUG_FATAL(m_vertexBuffer->getTextureCoordinateSetDimension(setIndex) != 1, ("VB texture coordinate set %d is not of proper dimension 1/%d", setIndex, m_vertexBuffer->getTextureCoordinateSetDimension(setIndex)));
	float * const tcs = reinterpret_cast<float*>(m_data + m_descriptor->offsetTextureCoordinateSet[setIndex]);
	tcs[0] = tc0;
}

// ----------------------------------------------------------------------

inline void VertexBufferWriteIterator::setTextureCoordinates(int setIndex, float tc0, float tc1)
{
	checkLock();
	DEBUG_FATAL(setIndex >= m_vertexBuffer->getNumberOfTextureCoordinateSets(), ("VB does not have enough texture coordinates %d/%d", setIndex, m_vertexBuffer->getNumberOfTextureCoordinateSets()));
	DEBUG_FATAL(m_vertexBuffer->getTextureCoordinateSetDimension(setIndex) != 2, ("VB texture coordinate set %d is not of proper dimension 2/%d", setIndex, m_vertexBuffer->getTextureCoordinateSetDimension(setIndex)));
	float * const tcs = reinterpret_cast<float*>(m_data + m_descriptor->offsetTextureCoordinateSet[setIndex]);
	tcs[0] = tc0;
	tcs[1] = tc1;
}

// ----------------------------------------------------------------------

inline void VertexBufferWriteIterator::setTextureCoordinates(int setIndex, float tc0, float tc1, float tc2)
{
	checkLock();
	DEBUG_FATAL(setIndex >= m_vertexBuffer->getNumberOfTextureCoordinateSets(), ("VB does not have enough texture coordinates %d/%d", setIndex, m_vertexBuffer->getNumberOfTextureCoordinateSets()));
	DEBUG_FATAL(m_vertexBuffer->getTextureCoordinateSetDimension(setIndex) != 3, ("VB texture coordinate set %d is not of proper dimension 3/%d", setIndex, m_vertexBuffer->getTextureCoordinateSetDimension(setIndex)));
	float * const tcs = reinterpret_cast<float*>(m_data + m_descriptor->offsetTextureCoordinateSet[setIndex]);
	tcs[0] = tc0;
	tcs[1] = tc1;
	tcs[2] = tc2;
}

// ----------------------------------------------------------------------

inline void VertexBufferWriteIterator::setTextureCoordinates(int setIndex, const Vector &tcs012)
{
	checkLock();
	DEBUG_FATAL(setIndex >= m_vertexBuffer->getNumberOfTextureCoordinateSets(), ("VB does not have enough texture coordinates %d/%d", setIndex, m_vertexBuffer->getNumberOfTextureCoordinateSets()));
	DEBUG_FATAL(m_vertexBuffer->getTextureCoordinateSetDimension(setIndex) != 3, ("VB texture coordinate set %d is not of proper dimension 3/%d", setIndex, m_vertexBuffer->getTextureCoordinateSetDimension(setIndex)));
	float * const tcs = reinterpret_cast<float*>(m_data + m_descriptor->offsetTextureCoordinateSet[setIndex]);
	tcs[0] = tcs012.x;
	tcs[1] = tcs012.y;
	tcs[2] = tcs012.z;
}

// ----------------------------------------------------------------------

inline void VertexBufferWriteIterator::setTextureCoordinates(int setIndex, float tc0, float tc1, float tc2, float tc3)
{
	checkLock();
	DEBUG_FATAL(setIndex >= m_vertexBuffer->getNumberOfTextureCoordinateSets(), ("VB does not have enough texture coordinates %d/%d", setIndex, m_vertexBuffer->getNumberOfTextureCoordinateSets()));
	DEBUG_FATAL(m_vertexBuffer->getTextureCoordinateSetDimension(setIndex) != 4, ("VB texture coordinate set %d is not of proper dimension 4/%d", setIndex, m_vertexBuffer->getTextureCoordinateSetDimension(setIndex)));
	float * const tcs = reinterpret_cast<float*>(m_data + m_descriptor->offsetTextureCoordinateSet[setIndex]);
	tcs[0] = tc0;
	tcs[1] = tc1;
	tcs[2] = tc2;
	tcs[3] = tc3;
}

// ----------------------------------------------------------------------

inline void VertexBufferWriteIterator::copy(const VertexBufferReadIterator &source, int count)
{
	checkLock();
	source.checkLock();	
	DEBUG_FATAL(m_vertexBuffer == source.m_vertexBuffer, ("Cannot copy from a VB back to iteself"));
	DEBUG_FATAL(*m_descriptor != *source.m_descriptor, ("VertexBufferDescriptors differ, can't copy"));
	memcpy(m_data, source.m_data, m_descriptor->vertexSize * count);
}

// ----------------------------------------------------------------------
/**
 * Retrieve a pointer to the beginning of raw data for this vertex.
 *
 * Do not use this function unless you know exactly what you are doing
 * and require access to data through a different interface.
 */

inline void *VertexBufferWriteIterator::getDataPointer()
{
	return m_data;
}

// ======================================================================

inline VertexBufferReadWriteIterator ::VertexBufferReadWriteIterator(const VertexBufferReadWriteIterator &rhs)
: VertexBufferBaseIterator(rhs)
{
}

// ----------------------------------------------------------------------

inline VertexBufferReadWriteIterator ::VertexBufferReadWriteIterator(const VertexBuffer &vertexBuffer, byte *data)
: VertexBufferBaseIterator(vertexBuffer, data)
{
}

// ----------------------------------------------------------------------

inline VertexBufferReadWriteIterator &VertexBufferReadWriteIterator::operator =(const VertexBufferReadWriteIterator &rhs)
{
	assign(rhs);
	return *this;
}

// ----------------------------------------------------------------------

inline VertexBufferReadWriteIterator &VertexBufferReadWriteIterator::operator ++()
{
	increment();
	return *this;
}

// ----------------------------------------------------------------------

inline VertexBufferReadWriteIterator &VertexBufferReadWriteIterator::operator --()
{
	decrement();
	return *this;
}

// ----------------------------------------------------------------------

inline VertexBufferReadWriteIterator &VertexBufferReadWriteIterator::operator +=(int offset)
{
	checkLock();
	m_data += m_descriptor->vertexSize * offset;
	return *this;
}

// ----------------------------------------------------------------------

inline VertexBufferReadWriteIterator &VertexBufferReadWriteIterator::operator -=(int offset)
{
	checkLock();
	m_data -= m_descriptor->vertexSize * offset;
	return *this;
}

// ----------------------------------------------------------------------

inline VertexBufferReadWriteIterator  VertexBufferReadWriteIterator::operator +(int offset) const
{
	checkLock();
	return VertexBufferReadWriteIterator(*m_vertexBuffer, m_data + (offset * m_descriptor->vertexSize));
}

// ----------------------------------------------------------------------

inline VertexBufferReadWriteIterator  VertexBufferReadWriteIterator::operator -(int offset) const
{
	checkLock();
	return VertexBufferReadWriteIterator(*m_vertexBuffer, m_data - (offset * m_descriptor->vertexSize));
}

// ----------------------------------------------------------------------

inline const Vector &VertexBufferReadWriteIterator::getPosition() const
{
	return reinterpret_cast<const VertexBufferReadIterator*>(this)->getPosition();
}

// ----------------------------------------------------------------------

inline float VertexBufferReadWriteIterator::getOoz() const
{
	return reinterpret_cast<const VertexBufferReadIterator*>(this)->getOoz();
}

// ----------------------------------------------------------------------

inline const Vector &VertexBufferReadWriteIterator::getNormal() const
{
	return reinterpret_cast<const VertexBufferReadIterator*>(this)->getNormal();
}

// ----------------------------------------------------------------------

inline float VertexBufferReadWriteIterator::getPointSize() const
{
	return reinterpret_cast<const VertexBufferReadIterator*>(this)->getPointSize();
}

// ----------------------------------------------------------------------

inline const PackedArgb &VertexBufferReadWriteIterator::getColor0() const
{
	return reinterpret_cast<const VertexBufferReadIterator*>(this)->getColor0();
}

// ----------------------------------------------------------------------

inline const PackedArgb &VertexBufferReadWriteIterator::getColor1() const
{
	return reinterpret_cast<const VertexBufferReadIterator*>(this)->getColor1();
}

// ----------------------------------------------------------------------

inline int VertexBufferReadWriteIterator::getNumberOfTextureCoordinateSets() const
{
	return reinterpret_cast<const VertexBufferReadIterator*>(this)->getNumberOfTextureCoordinateSets();
}

// ----------------------------------------------------------------------

inline int VertexBufferReadWriteIterator::getTextureCoordinateSetDimension(int textureCoordinateSet) const
{
	return reinterpret_cast<const VertexBufferReadIterator*>(this)->getTextureCoordinateSetDimension(textureCoordinateSet);
}

// ----------------------------------------------------------------------

inline float VertexBufferReadWriteIterator::getTextureCoordinate(int setIndex, int coordinateIndex) const
{
	return reinterpret_cast<const VertexBufferReadIterator*>(this)->getTextureCoordinate(setIndex, coordinateIndex);
}

// ----------------------------------------------------------------------

inline void VertexBufferReadWriteIterator::getTextureCoordinates(int setIndex, float &tc0) const
{
	reinterpret_cast<const VertexBufferReadIterator*>(this)->getTextureCoordinates(setIndex, tc0);
}

// ----------------------------------------------------------------------

inline void VertexBufferReadWriteIterator::getTextureCoordinates(int setIndex, float &tc0, float &tc1) const
{
	reinterpret_cast<const VertexBufferReadIterator*>(this)->getTextureCoordinates(setIndex, tc0, tc1);
}

// ----------------------------------------------------------------------

inline void VertexBufferReadWriteIterator::getTextureCoordinates(int setIndex, float &tc0, float &tc1, float &tc2) const
{
	reinterpret_cast<const VertexBufferReadIterator*>(this)->getTextureCoordinates(setIndex, tc0, tc1, tc2);
}

// ----------------------------------------------------------------------

inline void VertexBufferReadWriteIterator::getTextureCoordinates(int setIndex, float &tc0, float &tc1, float &tc2, float &tc3) const
{
	reinterpret_cast<const VertexBufferReadIterator*>(this)->getTextureCoordinates(setIndex, tc0, tc1, tc2, tc3);
}

// ----------------------------------------------------------------------

inline void VertexBufferReadWriteIterator::setPosition(const Vector &position)
{
	reinterpret_cast<VertexBufferWriteIterator*>(this)->setPosition(position);
}

// ----------------------------------------------------------------------

inline void VertexBufferReadWriteIterator::setPosition(float x, float y, float z)
{
	reinterpret_cast<VertexBufferWriteIterator*>(this)->setPosition(x, y, z);
}

// ----------------------------------------------------------------------

inline void VertexBufferReadWriteIterator::setPositionX(float x)
{
	reinterpret_cast<VertexBufferWriteIterator*>(this)->setPositionX(x);
}

// ----------------------------------------------------------------------

inline void VertexBufferReadWriteIterator::setPositionY(float y)
{
	reinterpret_cast<VertexBufferWriteIterator*>(this)->setPositionY(y);
}

// ----------------------------------------------------------------------

inline void VertexBufferReadWriteIterator::setPositionZ(float z)
{
	reinterpret_cast<VertexBufferWriteIterator*>(this)->setPositionZ(z);
}

// ----------------------------------------------------------------------

inline void VertexBufferReadWriteIterator::setOoz(float ooz)
{
	reinterpret_cast<VertexBufferWriteIterator*>(this)->setOoz(ooz);
}

// ----------------------------------------------------------------------

inline void VertexBufferReadWriteIterator::setNormal(const Vector &normal)
{
	reinterpret_cast<VertexBufferWriteIterator*>(this)->setNormal(normal);
}

// ----------------------------------------------------------------------

inline void VertexBufferReadWriteIterator::setNormal(float x, float y, float z)
{
	reinterpret_cast<VertexBufferWriteIterator*>(this)->setNormal(x, y, z);
}

// ----------------------------------------------------------------------

inline void VertexBufferReadWriteIterator::setNormalX(float x)
{
	reinterpret_cast<VertexBufferWriteIterator*>(this)->setNormalX(x);
}

// ----------------------------------------------------------------------

inline void VertexBufferReadWriteIterator::setNormalY(float y)
{
	reinterpret_cast<VertexBufferWriteIterator*>(this)->setNormalY(y);
}

// ----------------------------------------------------------------------

inline void VertexBufferReadWriteIterator::setNormalZ(float z)
{
	reinterpret_cast<VertexBufferWriteIterator*>(this)->setNormalZ(z);
}

// ----------------------------------------------------------------------

inline void VertexBufferReadWriteIterator::setPointSize(float size)
{
	reinterpret_cast<VertexBufferWriteIterator*>(this)->setPointSize(size);
}

// ----------------------------------------------------------------------

inline void VertexBufferReadWriteIterator::setColor0(uint32 color)
{
	reinterpret_cast<VertexBufferWriteIterator*>(this)->setColor0(color);
}

// ----------------------------------------------------------------------

inline void VertexBufferReadWriteIterator::setColor1(uint32 color)
{
	reinterpret_cast<VertexBufferWriteIterator*>(this)->setColor1(color);
}

// ----------------------------------------------------------------------

inline void VertexBufferReadWriteIterator::setColor0(const PackedArgb &color)
{
	reinterpret_cast<VertexBufferWriteIterator*>(this)->setColor0(color);
}

// ----------------------------------------------------------------------

inline void VertexBufferReadWriteIterator::setColor1(const PackedArgb &color)
{
	reinterpret_cast<VertexBufferWriteIterator*>(this)->setColor1(color);
}

// ----------------------------------------------------------------------

inline void VertexBufferReadWriteIterator::setTextureCoordinate(int setIndex, int coordinateIndex, float value)
{
	reinterpret_cast<VertexBufferWriteIterator*>(this)->setTextureCoordinate(setIndex, coordinateIndex, value);
}

// ----------------------------------------------------------------------

inline void VertexBufferReadWriteIterator::setTextureCoordinates(int setIndex, float tc0)
{
	reinterpret_cast<VertexBufferWriteIterator*>(this)->setTextureCoordinates(setIndex, tc0);
}

// ----------------------------------------------------------------------

inline void VertexBufferReadWriteIterator::setTextureCoordinates(int setIndex, float tc0, float tc1)
{
	reinterpret_cast<VertexBufferWriteIterator*>(this)->setTextureCoordinates(setIndex, tc0, tc1);
}

// ----------------------------------------------------------------------

inline void VertexBufferReadWriteIterator::setTextureCoordinates(int setIndex, float tc0, float tc1, float tc2)
{
	reinterpret_cast<VertexBufferWriteIterator*>(this)->setTextureCoordinates(setIndex, tc0, tc1, tc2);
}

// ----------------------------------------------------------------------

inline void VertexBufferReadWriteIterator::setTextureCoordinates(int setIndex, const Vector &tcs012)
{
	reinterpret_cast<VertexBufferWriteIterator*>(this)->setTextureCoordinates(setIndex, tcs012);
}

// ----------------------------------------------------------------------

inline void VertexBufferReadWriteIterator::setTextureCoordinates(int setIndex, float tc0, float tc1, float tc2, float tc3)
{
	reinterpret_cast<VertexBufferWriteIterator*>(this)->setTextureCoordinates(setIndex, tc0, tc1, tc2, tc3);
}

// ----------------------------------------------------------------------

inline void VertexBufferReadWriteIterator::copy(const VertexBufferReadIterator &source, int count)
{
	reinterpret_cast<VertexBufferWriteIterator*>(this)->copy(source, count);
}

// ----------------------------------------------------------------------
/**
 * Retrieve a pointer to the beginning of raw data for this vertex.
 *
 * Do not use this function unless you know exactly what you are doing
 * and require access to data through a different interface.
 */

inline const void *VertexBufferReadWriteIterator::getDataPointer() const
{
	return m_data;
}

// ----------------------------------------------------------------------
/**
 * Retrieve a pointer to the beginning of raw data for this vertex.
 *
 * Do not use this function unless you know exactly what you are doing
 * and require access to data through a different interface.
 */

inline void *VertexBufferReadWriteIterator::getDataPointer()
{
	return m_data;
}

// ======================================================================

#endif
