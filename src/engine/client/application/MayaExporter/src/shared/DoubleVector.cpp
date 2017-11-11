// ======================================================================
//
// DoubleVector.cpp
// Copyright 2005 Sony Online Entertainment.
// All Rights Reserved.
//
// ======================================================================

#include "FirstMayaExporter.h"
#include "DoubleVector.h"

#include "sharedRandom/Random.h"

#include <vector>

// ======================================================================

#define CONST_DOUBLE(_x_) double(_x_)

const DoubleVector DoubleVector::unitX(CONST_DOUBLE(1), CONST_DOUBLE(0), CONST_DOUBLE(0));
const DoubleVector DoubleVector::unitY(CONST_DOUBLE(0), CONST_DOUBLE(1), CONST_DOUBLE(0));
const DoubleVector DoubleVector::unitZ(CONST_DOUBLE(0), CONST_DOUBLE(0), CONST_DOUBLE(1));

const DoubleVector DoubleVector::negativeUnitX(CONST_DOUBLE(-1), CONST_DOUBLE( 0), CONST_DOUBLE( 0));
const DoubleVector DoubleVector::negativeUnitY(CONST_DOUBLE( 0), CONST_DOUBLE(-1), CONST_DOUBLE( 0));
const DoubleVector DoubleVector::negativeUnitZ(CONST_DOUBLE( 0), CONST_DOUBLE( 0), CONST_DOUBLE(-1));

const DoubleVector DoubleVector::zero(CONST_DOUBLE(0), CONST_DOUBLE(0), CONST_DOUBLE(0));
const DoubleVector DoubleVector::xyz111(CONST_DOUBLE(1), CONST_DOUBLE(1), CONST_DOUBLE(1));

const DoubleVector DoubleVector::maxXYZ(REAL_MAX, REAL_MAX, REAL_MAX);
const DoubleVector DoubleVector::negativeMaxXYZ(-REAL_MAX, -REAL_MAX, -REAL_MAX);

const double   DoubleVector::NORMALIZE_THRESHOLD(CONST_DOUBLE(0.0000001));

const double   DoubleVector::NORMALIZED_EPSILON(CONST_DOUBLE(0.0000001));

// If M is between 1-e and 1+e, then M^2 is between 1-2e+e^2 and 1+2e+e^2.

static const double   NORMALIZED_RANGE_SQUARED_MIN = (1.0f - (2.0f * DoubleVector::NORMALIZED_EPSILON)) + (DoubleVector::NORMALIZED_EPSILON * DoubleVector::NORMALIZED_EPSILON);
static const double   NORMALIZED_RANGE_SQUARED_MAX = (1.0f + (2.0f * DoubleVector::NORMALIZED_EPSILON)) + (DoubleVector::NORMALIZED_EPSILON * DoubleVector::NORMALIZED_EPSILON);

// ======================================================================
// Normalize a vector to a length of 1
//
// Return value:
//
//   True if the vector has been normalized, otherwise false.
//
// Remarks:
//
//   If the vector is too small, it cannot be normalized.

bool DoubleVector::normalize(void)
{
	double mag = magnitude();

	if (mag < NORMALIZE_THRESHOLD)
		return false;

	*this /= mag;

	return true;
}

// ----------------------------------------------------------------------
/**
 * Normalize a vector to a length of approximately 1.
 * 
 * If the vector is too small, it cannot be normalized.
 * 
 * @return True if the vector has been approximately normalized, otherwise false.
 */

bool DoubleVector::approximateNormalize(void)
{
	double mag = approximateMagnitude();

	if (mag < NORMALIZE_THRESHOLD)
		return false;

	*this /= mag;

	return true;
}

// ----------------------------------------------------------------------
/**
 * Return true if a vector is of length 1 (within some tolerance)
 *
 * DoubleVectors normalized with DoubleVector::normalize() should always be of unit
 * length within tolerance, vectors normalized with DoubleVector::approximate
 * Normalize will not.
 *
 * @return True if the vector's length is within NORMALIZED_TOLERANCE of 1
 */

bool DoubleVector::isNormalized(void) const
{
	double mag2 = magnitudeSquared();

	return WithinRangeInclusiveInclusive(NORMALIZED_RANGE_SQUARED_MIN,mag2,NORMALIZED_RANGE_SQUARED_MAX);
}

// ----------------------------------------------------------------------
/**
 * Find the point on the specified line that is as close to this point as possible.
 * 
 * The line is treated as an infinite line, not a line segment.
 * 
 * @param line0  First point on the line
 * @param line1  Second point on the line
 * @param t  Parameteric time along the line that is closest to this vector
 */

const DoubleVector DoubleVector::findClosestPointOnLine(const DoubleVector &line0, const DoubleVector &line1, double *t) const
{
	DEBUG_FATAL(!t, ("t arg is null"));

    NOT_NULL(t);

	DoubleVector delta(line1 - line0);
	const double r  = (*this - line0).dot(delta) / delta.magnitudeSquared();
	*t = r;
	return line0 + delta * r;
}

// ----------------------------------------------------------------------
/**
 * Find the point on the specified line that is as close to this point as possible.
 * 
 * The line is treated as an infinite line, not a line segment.
 * 
 * @param line0  First point on the line
 * @param line1  Second point on the line
 */

const DoubleVector DoubleVector::findClosestPointOnLine(const DoubleVector &line0, const DoubleVector &line1) const
{
	double t;
	return findClosestPointOnLine(line0, line1, &t);
}

//-----------------------------------------------------------------------

const DoubleVector DoubleVector::findClosestPointOnLineSegment(const DoubleVector & startPoint, const DoubleVector & endPoint) const
{
	DoubleVector delta(endPoint - startPoint);

	// if these vectors describe a point instead of a line-segment, return the startpoint
	// rather than returning an invalid vector
	const double deltaMagnitudeSquared = delta.magnitudeSquared();
	if(deltaMagnitudeSquared <NORMALIZE_THRESHOLD)
		return startPoint;
	
	const double r = clamp(CONST_DOUBLE(0), (*this - startPoint).dot(delta) / deltaMagnitudeSquared, CONST_DOUBLE(1));
	return startPoint + delta * r;
}

// ----------------------------------------------------------------------
/**
 * Calculate the distance from this point to the specified line.
 * 
 * The line is treated as an infinite line, not a line segment.
 * 
 * @param line0  First point on the line
 * @param line1  Second point on the line
 */

double DoubleVector::distanceToLine(const DoubleVector &line0, const DoubleVector &line1) const
{
	return magnitudeBetween(findClosestPointOnLine(line0, line1));
}

// ----------------------------------------------------------------------
/**
 * Calculate the distance from this point to the specified line segment.
 * 
 * @param line0  First point on the line
 * @param line1  Second point on the line
 */

double DoubleVector::distanceToLineSegment(const DoubleVector &line0, const DoubleVector &line1) const
{
	return magnitudeBetween(findClosestPointOnLineSegment(line0, line1));
}

// ----------------------------------------------------------------------
/**
 * Send this vector to the DebugPrint system.
 * 
 * The header parameter may be NULL.
 * 
 * @param header  Header for the vector
 */

void DoubleVector::debugPrint(const char *header) const
{
	if (header)
		DEBUG_REPORT_PRINT(true, ("%s: ", header));

	DEBUG_REPORT_PRINT(true, ("  %-8.2f %-8.2f %-8.2f\n", x, y, z));
}

// ----------------------------------------------------------------------
/**
 * Create a random unit vector.
 * 
 * This routine will return an evenly distributed random vector on the
 * unit sphere.
 */

const DoubleVector DoubleVector::randomUnit(void)
{
	// from the comp.graphics.algorithm FAQ
	const double lz = cos(Random::randomReal(0.0f, PI));
	const double t = Random::randomReal(0.f, PI_TIMES_2);
	const double r = sqrt(1.0f - sqr(lz));
	return DoubleVector(r * cos(t), r * sin(t), lz);
}

// ----------------------------------------------------------------------
/**
 * Test whether a point lies within a triangle.
 * 
 * This should be on at least the plane for the test to work. See Plane::findIntersection ()
 */

namespace {
	// adapated from http://www.blackpawn.com/texts/pointinpoly/default.html
	inline bool sameSide(const DoubleVector &p1, const DoubleVector &p2, const DoubleVector &a, const DoubleVector &b)
	{
		DoubleVector const ba(b - a);
		DoubleVector const cp1(ba.cross(p1 - a));
		DoubleVector const cp2(ba.cross(p2 - a));
		return cp1.dot(cp2) >= 0.0f;
	}
}

bool DoubleVector::inPolygon (const DoubleVector& v0, const DoubleVector& v1, const DoubleVector& v2) const
{
	return sameSide(*this, v0, v1, v2) && sameSide(*this, v1, v2, v0) && sameSide(*this, v2, v0, v1);
}

// ----------------------------------------------------------------------
/**
 * Test whether a point lies within a convex n-sided polygon.
 * 
 * This should be on at least the plane for the test to work. See Plane::findIntersection ()
 */

bool DoubleVector::inPolygon(const std::vector<DoubleVector> &convexPolygonVertices) const
{
	// @todo optimize this
	const uint numberOfConvexPolygonVertices = convexPolygonVertices.size();
	for (uint i = 1; i < numberOfConvexPolygonVertices-1; ++i)
		if (inPolygon(convexPolygonVertices[0], convexPolygonVertices[i], convexPolygonVertices[i+1]))
			return true;

	return false;
}

// ----------------------------------------------------------------------
/**
 * Determine if two vectors are within an epsilon distance.
 * The epsilon distance is tested for all three dimensions, not a
 * distance between the points.
 *
 * @param rhs The second vector to compare.
 * @param epsilon The epsilon distance.
 */
bool DoubleVector::withinEpsilon(const DoubleVector &rhs, double epsilon) const
{
	return (fabs(x - rhs.x) < epsilon) && (fabs(y - rhs.y) < epsilon) && (fabs(z - rhs.z) < epsilon);
}


// ----------------------------------------------------------------------
/**
 * Find a direction which is perpendicular to the vector passed in.
 * NOTE: The result is NOT guaranteed to be normalized.
 */

const DoubleVector DoubleVector::perpendicular(DoubleVector const & direction)
{
	// Measure the projection of "direction" onto each of the axes
	double const id = fabs(direction.dot(DoubleVector::unitX));
	double const jd = fabs(direction.dot(DoubleVector::unitY));
	double const kd = fabs(direction.dot(DoubleVector::unitZ));

	DoubleVector result;

	if (id <= jd && id <= kd)
		// Projection onto i was the smallest
		result = direction.cross(DoubleVector::unitX);
	else if (jd <= id && jd <= kd)
		// Projection onto j was the smallest
		result = direction.cross(DoubleVector::unitY);
	else
		// Projection onto k was the smallest
		result = direction.cross(DoubleVector::unitZ);

	result.normalize();

	return result;
}

// ======================================================================
