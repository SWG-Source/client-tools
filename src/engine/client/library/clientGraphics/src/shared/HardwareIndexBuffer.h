// ======================================================================
//
// HardwareIndexBuffer.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_HardwareIndexBuffer_H
#define INCLUDED_HardwareIndexBuffer_H

// ======================================================================

class HardwareIndexBuffer
{
public:

	enum Type
	{
		T_static,
		T_dynamic
	};

public:

	HardwareIndexBuffer(Type type);
	virtual ~HardwareIndexBuffer();

	Type getType() const;

private:

	HardwareIndexBuffer(const HardwareIndexBuffer &);
	HardwareIndexBuffer &operator =(const HardwareIndexBuffer &);

private:

	Type m_type;
};

// ======================================================================

inline HardwareIndexBuffer::HardwareIndexBuffer(Type type)
:
	m_type(type)
{
}


inline HardwareIndexBuffer::Type HardwareIndexBuffer::getType() const
{
	return m_type;
}

// ======================================================================

#endif
