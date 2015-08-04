// ======================================================================
//
// DoubleVector.h
// Portions copyright 1998 Bootprint Entertainment.
// Portions copyright 2000-2001 Sony Online Entertainment.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_DoubleVector_H
#define INCLUDED_DoubleVector_H

// ======================================================================
#include "sharedMath/Vector.h"

class DoubleVector
{
public:

	// Vector 1,0,0
	static const DoubleVector unitX;

	// Vector 0,1,0
	static const DoubleVector unitY;

	// Vector 0,0,1
	static const DoubleVector unitZ;

	// Vector -1,0,0
	static const DoubleVector negativeUnitX;

	// Vector 0,-1,0
	static const DoubleVector negativeUnitY;

	// Vector 0,0,-1
	static const DoubleVector negativeUnitZ;

	// Vector 0,0,0
	static const DoubleVector zero;

	// Vector 1,1,1
	static const DoubleVector xyz111;

	// Vector max, max, max
	static const DoubleVector maxXYZ;

	// Vector -maxXYZ
	static const DoubleVector negativeMaxXYZ;

	// minimum vector magnitude to normalize
	static const double   NORMALIZE_THRESHOLD;

	// A vector is normalized if its magnitude is within NORMALIZED_EPSILON of 1.
	static const double   NORMALIZED_EPSILON;

public:

	// x-component of the 3d vector
	double x;

	// y-component of the 3d vector
	double y;

	// z-component of the 3d vector
	double z;

public:

	DoubleVector(void);
	DoubleVector(double newX, double newY, double newZ);
	DoubleVector(const Vector &i_floatVector);

	void                 debugPrint(const char *header) const;

	void                 set(double newX, double newY, double newZ);
	void                 makeZero(void);

	bool                 normalize(void);
	bool                 approximateNormalize(void);
	bool                 isNormalized(void) const;

	const DoubleVector         findClosestPointOnLine(const DoubleVector &line0, const DoubleVector &line1) const;
	const DoubleVector         findClosestPointOnLine(const DoubleVector &line0, const DoubleVector &line1, double *t) const;
	const DoubleVector         findClosestPointOnLineSegment(const DoubleVector & startPoint, const DoubleVector & endPoint) const;
	double                 distanceToLine(const DoubleVector &line0, const DoubleVector &line1) const;
	double                 distanceToLineSegment(const DoubleVector &line0, const DoubleVector &line1) const;

	double                 theta(void) const;
	double                 phi(void) const;

	double                 dot(const DoubleVector &vector) const;
	const DoubleVector         cross(const DoubleVector &rhs) const;

	double                 magnitudeSquared(void) const;
	double                 approximateMagnitude(void) const;
	double                 magnitude(void) const;
	double                 magnitudeBetween(const DoubleVector &vector) const;
	double                 magnitudeBetweenSquared(const DoubleVector &vector) const;

	const DoubleVector         operator -(void) const;

	DoubleVector              &operator -=(const DoubleVector &rhs);
	DoubleVector              &operator +=(const DoubleVector &rhs);
	DoubleVector              &operator /=(double scalar);
	DoubleVector              &operator *=(double scalar);

	const DoubleVector         operator +(const DoubleVector &rhs) const;
	const DoubleVector         operator -(const DoubleVector &rhs) const;
	const DoubleVector         operator *(double scalar) const;
	const DoubleVector         operator /(double scalar) const;
	friend const DoubleVector  operator *(double scalar, const DoubleVector &vector);

	bool                 operator ==(const DoubleVector &rhs) const;
	bool                 operator !=(const DoubleVector &rhs) const;
	bool                 withinEpsilon(const DoubleVector &rhs, double epsilon) const;

	const DoubleVector         reflectIncoming(const DoubleVector &incident) const;
	const DoubleVector         reflectOutgoing(const DoubleVector &incident) const;

	// const DoubleVector      refract(const DoubleVector &normal, double n1, double n2);

	bool                 inPolygon (const DoubleVector& v0, const DoubleVector& v1, const DoubleVector& v2) const;
	bool                 inPolygon (const stdvector<DoubleVector>::fwd &convexPolygonVertices) const;

	operator Vector() const { return Vector(float(x), float(y), float(z)); }

public:

	static const DoubleVector  midpoint(const DoubleVector &vector1, const DoubleVector &vector2);
	static const DoubleVector  linearInterpolate(const DoubleVector &begin, const DoubleVector &end, double t);

	static const DoubleVector  randomUnit(void);

	static const DoubleVector perpendicular(DoubleVector const & direction);
};

// ======================================================================
// Construct a vector
//
// Remarks:
//
//   Initializes the components to 0.

inline DoubleVector::DoubleVector(void)
: x(CONST_REAL(0)), y(CONST_REAL(0)), z(CONST_REAL(0))
{
}

// ----------------------------------------------------------------------
/**
 * Construct a vector.
 * 
 * Initializes the components to the specified values
 * 
 * @param newX  Value for the X component
 * @param newY  Value for the Y component
 * @param newZ  Value for the Z component
 */

inline DoubleVector::DoubleVector(double newX, double newY, double newZ)
: x(newX), y(newY), z(newZ)
{
}

inline DoubleVector::DoubleVector(const Vector &i_floatVector)
: x(i_floatVector.x), y(i_floatVector.y), z(i_floatVector.z)
{
}

// ----------------------------------------------------------------------
/**
 * Set the vector components to new values.
 * 
 * @param newX  Value for the X component
 * @param newY  Value for the Y component
 * @param newZ  Value for the Z component
 */

inline void DoubleVector::set(double newX, double newY, double newZ)
{
	x = newX;
	y = newY;
	z = newZ;
}

// ----------------------------------------------------------------------
/**
 * Set all the vector components to zero.
 */

inline void DoubleVector::makeZero(void)
{
	x = CONST_REAL(0);
	y = CONST_REAL(0);
	z = CONST_REAL(0);
}

// ----------------------------------------------------------------------
/**
 * Return the rotation of the vector around the Y plane.
 * 
 * The result is undefined if both the x and z values of the vector are zero.
 * 
 * This routine uses atan2() so it is not particularly fast.
 * 
 * @return The rotation of the vector around the Y plane
 */

inline double DoubleVector::theta(void) const
{
	return atan2(x, z);
}

// ----------------------------------------------------------------------
/**
 * Calculate the angle of the vector from the X-Z plane.
 * 
 * This routine uses sqrt() and atan2() so it is not particularly fast.
 * 
 * @return The angle of the vector from the X-Z plane
 */

inline double DoubleVector::phi(void) const
{
	return atan2(-y, sqrt(sqr(x) + sqr(z)));
}

// ----------------------------------------------------------------------
/**
 * Compute the dot product between this vector and another vector.
 * 
 * The dot product value is equal to the cosine of the angle between
 * the two vectors multiplied by the sum of the lengths of the vectors.
 * 
 * @param vector  DoubleVector to compute the dot product against
 */

inline double DoubleVector::dot(const DoubleVector &vec) const
{
	return (x * vec.x) + (y * vec.y) + (z * vec.z);
}

// ----------------------------------------------------------------------
/**
 * Calculate the square of the magnitude of this vector.
 * 
 * This routine is much faster than magnitude().
 * 
 * @return The square of the magnitude of the vector
 * @see DoubleVector::magnitude()
 */

inline double DoubleVector::magnitudeSquared(void) const
{
	return (sqr(x) + sqr(y) + sqr(z));
}

// ----------------------------------------------------------------------
/**
 * Calculate the approximate magnitude of this vector.
 * 
 * The implementation of this routine has +/- 8% error.
 * 
 * @return The approximate magnitude of the vector
 */


inline double DoubleVector::approximateMagnitude(void) const
{
	double minc = fabs(x);
	double midc = fabs(y);
	double maxc = fabs(z);

	// sort the vectors
	// we do our own swapping to avoid heavy-weight includes in such a low-level class
	if (midc < minc)
	{
		const double temp = midc;
		midc = minc;
		minc = temp;
	}

	if (maxc < minc)
	{
		const double temp = maxc;
		maxc = minc;
		minc = temp;
	}

	if (maxc < midc)
	{
		const double temp = maxc;
		maxc = midc;
		midc = temp;
	}

	return (maxc + CONST_REAL(11.0f / 32.0f) * midc + CONST_REAL(0.25f) * minc);
}

// ----------------------------------------------------------------------
/**
 * Calculate the magnitude of this vector.
 * 
 * This routine is slow because it requires a square root operation.
 * 
 * @return The magnitude of the vector
 * @see DoubleVector::magnitudeSquared()
 */

inline double DoubleVector::magnitude(void) const
{
	return sqrt(magnitudeSquared());
}

// ----------------------------------------------------------------------
/**
 * Calculate the square of the magnitude of the vector between this vector and the specified vector.
 * 
 * This routine is much faster than magnitudeBetween().
 * 
 * @param vector  The other endpoint of the delta vector
 * @return The square of the magnitude of the delta vector
 * @see DoubleVector::magnitudeBetween()
 */

inline double DoubleVector::magnitudeBetweenSquared(const DoubleVector &vec) const
{
	return (sqr(x - vec.x) + sqr(y - vec.y) + sqr(z - vec.z));
}

// ----------------------------------------------------------------------
/**
 * Calculate the magnitude of the vector between this vector and the specified vector.
 * 
 * This routine is much slower than magnitudeBetweenSquared().
 * 
 * @param vector  The other endpoint of the delta vector
 * @return The magnitude of the delta vector
 * @see DoubleVector::magnitudeBetweenSquared()
 */

inline double DoubleVector::magnitudeBetween(const DoubleVector &vec) const
{
	return sqrt(magnitudeBetweenSquared(vec));
}

// ----------------------------------------------------------------------
/**
 * Reverse the direction of the vector.
 * 
 * This routine simple negates each component of the vector
 */

inline const DoubleVector DoubleVector::operator -(void) const
{
	return DoubleVector(-x, -y, -z);
}

// ----------------------------------------------------------------------
/**
 * Subtract a vector from this vector and store the result back in this vector.
 * 
 * This is the basic obvious -= operator overloaded for vectors
 * 
 * @param rhs  The vector to subtract from this vector
 * @return A reference to this modified vector
 */

inline DoubleVector &DoubleVector::operator -=(const DoubleVector &rhs)
{
	x -= rhs.x;
	y -= rhs.y;
	z -= rhs.z;
	return *this;
}

// ----------------------------------------------------------------------
/**
 * Add a vector from this vector and store the result back in this vector.
 * 
 * This is the basic obvious += operator overloaded for vectors.
 * 
 * @param rhs  The vector to add to this vector
 * @return A reference to this modified vector
 */

inline DoubleVector &DoubleVector::operator +=(const DoubleVector &rhs)
{
	x += rhs.x;
	y += rhs.y;
	z += rhs.z;
	return *this;
}

// ----------------------------------------------------------------------
/**
 * Multiply this vector by a scalar.
 * 
 * This is the basic obvious *= operator overloaded for vectors and scalars.
 * 
 * @param scalar  The vector to subtract from this vector
 * @return A reference to this modified vector
 */

inline DoubleVector &DoubleVector::operator *=(double scalar)
{
	x *= scalar;
	y *= scalar;
	z *= scalar;
	return *this;
}

// ----------------------------------------------------------------------
/**
 * Divide this vector by a scalar.
 * 
 * This is the basic obvious /= operator overloaded for vectors and scalars.
 * 
 * @param scalar  The vector to subtract from this vector
 * @return A reference to this modified vector
 */

inline DoubleVector &DoubleVector::operator /=(double scalar)
{
	*this *= (CONST_REAL(1.0) / scalar);
	return *this;
}

// ----------------------------------------------------------------------
/**
 * Calculate the cross product between two vectors.
 * 
 * This routine returns a temporary.
 * 
 * Cross products are not communitive.
 * 
 * @param rhs  The right-hand size of the expression
 * @return A vector that is the result of the cross product of the two vectors.
 */

inline const DoubleVector DoubleVector::cross(const DoubleVector &rhs) const
{
	return DoubleVector(y * rhs.z - z * rhs.y, z * rhs.x - x * rhs.z, x * rhs.y - y * rhs.x);
}

// ----------------------------------------------------------------------
/**
 * Add two vectors.
 * 
 * This routine returns a temporary.
 * 
 * @param rhs  The right-hand size of the expression
 * @return A vector that is the sum of the two arguments.
 */

inline const DoubleVector DoubleVector::operator +(const DoubleVector &rhs) const
{
	return DoubleVector(x + rhs.x, y + rhs.y, z + rhs.z);
}

// ----------------------------------------------------------------------
/**
 * Subtract two vectors.
 * 
 * This routine returns a temporary.
 * 
 * @param rhs  The right-hand size of the expression
 * @return A vector that is the result of the left-hand-side minus the right-hand-side
 */

inline const DoubleVector DoubleVector::operator -(const DoubleVector &rhs) const
{
	return DoubleVector(x - rhs.x, y - rhs.y, z - rhs.z);
}

// ----------------------------------------------------------------------
/**
 * Multiply a vector by a scalar.
 * 
 * This routine returns a temporary.
 * 
 * @param scalar  The scalar to multiply by
 * @return The source vector multiplied by the scalar
 */

inline const DoubleVector DoubleVector::operator *(double scalar) const
{
	return DoubleVector(x * scalar, y * scalar, z * scalar);
}

// ----------------------------------------------------------------------
/**
 * Divide a vector by a scalar.
 * 
 * This routine returns a temporary.
 * 
 * @param scalar  The scalar to multiply by
 * @return The source vector divided by the scalar
 */

inline const DoubleVector DoubleVector::operator /(double scalar) const
{
	const double multiplier(CONST_REAL(1.0) / scalar);
	return DoubleVector(x * multiplier, y * multiplier, z * multiplier);
}

// ----------------------------------------------------------------------
/**
 * Multiply a vector by a scalar.
 * 
 * This routine returns a temporary.
 * 
 * @return The source vector multiplied by the scalar
 */

inline const DoubleVector operator *(double scalar, const DoubleVector &vec)
{
	return DoubleVector(vec.x * scalar, vec.y * scalar, vec.z * scalar);
}

// ----------------------------------------------------------------------
/**
 * Test two vectors for equality.
 * 
 * Floating-point math make may two very similiar expressions end up slightly
 * different, thus showing inequality when the result is very, very close.
 * 
 * @param rhs  The right-hand size of the expression
 * @return True if the vectors are exactly equal, otherwise false.
 */

inline bool DoubleVector::operator ==(const DoubleVector &rhs) const
{
	return (x == rhs.x && y == rhs.y && z == rhs.z);  //lint !e777 // Testing floats for equality
}

// ----------------------------------------------------------------------
/**
 * Test two vectors for inequality.
 * 
 * Floating-point math make may two very similiar expressions end up slightly
 * different, thus showing inequality when the result is very, very close.
 * 
 * @param rhs  The right-hand size of the expression
 * @return True if the vectors are not equal, otherwise false.
 */

inline bool DoubleVector::operator !=(const DoubleVector &rhs) const
{
	return (x != rhs.x || y != rhs.y || z != rhs.z); //lint !e777 // Testing floats for equality
}

// ----------------------------------------------------------------------
/**
 * Reflect an outgoing vector around this normal.
 * 
 * This routine assumes that both this vector and the normal around which it
 * is being reflected have the same origin (the dot product of the normal and
 * the vertex is positive)
 * 
 * @param incident  Normal to reflect this vector around
 * @return The reflected vector
 */

inline const DoubleVector DoubleVector::reflectOutgoing(const DoubleVector &incident) const
{
	const DoubleVector &normal = *this;
	return normal * (2 * normal.dot(incident)) - incident;
}

// ----------------------------------------------------------------------
/**
 * Reflect an incoming vector around this normal.
 * 
 * This routine assumes that the vector terminates at the normal
 * (the dot product of the normal and the vertex is negative).
 * 
 * @param incident  Normal to reflect this vector around
 * @return The reflected vector
 */

inline const DoubleVector DoubleVector::reflectIncoming(const DoubleVector &incident) const
{
	return reflectOutgoing(-incident);
}

// ----------------------------------------------------------------------
/**
 * Compute the midpoint of two vectors.
 * 
 * This routine just averages the three components separately.
 * 
 * @param vector1  First endpoint
 * @param vector2  Second endpoint
 */

inline const DoubleVector DoubleVector::midpoint(const DoubleVector &vector1, const DoubleVector &vector2)
{
	return DoubleVector((vector1.x + vector2.x) * CONST_REAL(0.5), (vector1.y + vector2.y) * CONST_REAL(0.5), (vector1.z + vector2.z) * CONST_REAL(0.5));
}

// ----------------------------------------------------------------------
/**
 * Linearly interpolate between two vectors.
 * 
 * The time parameter should be between 0.0 and 1.0 inclusive in order to have
 * the result be between the two endpoints.  At time 0.0 the result will be
 * vector1, and at time 1.0 the result will be vector2.
 * 
 * @param vector1  Starting endpoint
 * @param vector2  Terminating endpoint
 * @param time   
 */

inline const DoubleVector DoubleVector::linearInterpolate(const DoubleVector &vector1, const DoubleVector &vector2, double time)
{
	return DoubleVector(vector1.x + (vector2.x - vector1.x) * time, vector1.y + (vector2.y - vector1.y) * time, vector1.z + (vector2.z - vector1.z) * time);
}

// ======================================================================

#endif
