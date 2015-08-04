// ======================================================================
//
// PaddedVector.h
// Copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_PaddedVector_H
#define INCLUDED_PaddedVector_H

// ======================================================================

#include "sharedMath/Vector.h"

// ======================================================================

class PaddedVector : public Vector
{
public:

	PaddedVector();
	PaddedVector(Vector const &vector);
	PaddedVector(real newX, real newY, real newZ);

private:

	float  m_pad;
};

// ======================================================================

inline PaddedVector::PaddedVector()
: Vector(),
	m_pad(0.0f)
{
}

// ----------------------------------------------------------------------

inline PaddedVector::PaddedVector(Vector const &vector)
: Vector(vector),
	m_pad(0.0f)
{
}

// ----------------------------------------------------------------------

inline PaddedVector::PaddedVector(real newX, real newY, real newZ)
:
	Vector(newX, newY, newZ),
	m_pad(0.0f)
{
}

// ======================================================================

#endif
