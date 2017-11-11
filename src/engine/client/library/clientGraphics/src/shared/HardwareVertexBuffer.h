// ======================================================================
//
// HardwareVertexBuffer.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_HardwareVertexBuffer_H
#define INCLUDED_HardwareVertexBuffer_H

// ======================================================================

#include "clientGraphics/VertexBuffer.h"

// ======================================================================

class HardwareVertexBuffer : public VertexBuffer
{
public:

	enum Type
	{
		T_static,
		T_dynamic
	};

public:

	virtual ~HardwareVertexBuffer();
	
	Type getType() const;

	virtual int getSortKey() const;

protected:

	HardwareVertexBuffer(Type type, const VertexBufferFormat &format);
	HardwareVertexBuffer(Type type);

private:

	// Disabled.
	HardwareVertexBuffer();
	HardwareVertexBuffer(const HardwareVertexBuffer &);
	HardwareVertexBuffer &operator =(const HardwareVertexBuffer &);

private:

	const Type m_type;
};

// ======================================================================

inline HardwareVertexBuffer::Type HardwareVertexBuffer::getType() const
{
	return m_type;
}

// ----------

inline int HardwareVertexBuffer::getSortKey() const
{
	return -1;
}

// ======================================================================

#endif
