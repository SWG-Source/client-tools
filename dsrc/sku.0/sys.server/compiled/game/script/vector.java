// ======================================================================
//
// vector.java
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

package script;

import java.lang.Math;
import java.io.Serializable;
import script.random;

// ======================================================================

public final class vector implements Comparable, Serializable
{
	// ----------------------------------------------------------------------

	private final static long serialVersionUID = 9014437847284915252L;

	private final static float NORMALIZE_THRESHOLD = 0.00001f;
	private final static float NORMALIZED_EPSILON = 0.00001f;
	private final static float NORMALIZED_RANGE_SQUARED_MIN = (1.0f - (2.0f * NORMALIZED_EPSILON)) + NORMALIZED_EPSILON*NORMALIZED_EPSILON;
	private final static float NORMALIZED_RANGE_SQUARED_MAX = (1.0f + (2.0f * NORMALIZED_EPSILON)) + NORMALIZED_EPSILON*NORMALIZED_EPSILON;

	/** The zero vector */
	public final static vector zero = new vector();
	/** The unit vector along the X axis. */
	public final static vector unitX = new vector(1.0f, 0.0f, 0.0f);
	/** The unit vector along the Y axis. */
	public final static vector unitY = new vector(0.0f, 1.0f, 0.0f);
	/** The unit vector along the Z axis. */
	public final static vector unitZ = new vector(0.0f, 0.0f, 1.0f);

	final public float x;
	final public float y;
	final public float z;

	// ----------------------------------------------------------------------
	/**
	 * Construct a zero vector.
	 */
	public vector()
	{
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
	}

	// ----------------------------------------------------------------------
	/**
	 * Construct a (newX, newY, newZ) vector
	 */
	public vector(float newX, float newY, float newZ)
	{
		x = newX;
		y = newY;
		z = newZ;
	}

	// ----------------------------------------------------------------------
	/**
	 * Construct a vector from another vector
	 */
	public vector(vector src)
	{
		x = src.x;
		y = src.y;
		z = src.z;
	}

	// ----------------------------------------------------------------------

	public Object clone()
	{
		return new vector(this);
	}

	// ----------------------------------------------------------------------

	public String toString()
	{
		return "[ " + x + " " + y + " " + z + " ]";
	}

	// ----------------------------------------------------------------------

	public int compareTo(Object o) throws ClassCastException
	{
		return compareTo((vector)o);
	}

	// ----------------------------------------------------------------------
	/**
	 * Determine how the magnitude of this vector compares to another vector
	 *
	 * @return -1 if this is shorter than v, 0 if of equal length, 1 if this is longer than v
	 */
	public int compareTo(vector v)
	{
		float mag1 = magnitudeSquared();
		float mag2 = v.magnitudeSquared();
		if (mag1 < mag2)
			return -1;
		if (mag2 > mag1)
			return 1;
		return 0;
	}

	// ----------------------------------------------------------------------
	/**
	 * Determine whether this vector is equal to an object.
	 */
	public boolean equals(Object o)
	{
		if (o != null)
		{
			try
			{
				vector v = (vector)o;
				return x == v.x && y == v.y && z == v.z;
			}
			catch (ClassCastException err)
			{
			}
		}
		return false;
	}

	// ----------------------------------------------------------------------
	/**
	 * Normalize a vector to a length of 1.
	 *
	 * @return the normalized vector, or the unit Z vector if not appropriate for normalization.
	 */
	public vector normalize()
	{
		float mag = magnitude();

		if (mag < NORMALIZE_THRESHOLD)
			return unitZ;

		return divide(mag);
	}

	// ----------------------------------------------------------------------
	/**
	 * Normalize a vector to a length of approximately 1.
	 *
	 * @return the approximately normalized vector, or the unit Z vector if not appropariate for normalization.
	 */
	public vector approximateNormalize()
	{
		float mag = approximateMagnitude();

		if (mag < NORMALIZE_THRESHOLD)
			return unitZ;

		return divide(mag);
	}

	// ----------------------------------------------------------------------
	/**
	 * Determine whether a vector is normalized.
	 *
	 * @return true if normalized, false if not.
	 */
	public boolean isNormalized()
	{
		float magSq = magnitudeSquared();
		return magSq >= NORMALIZED_RANGE_SQUARED_MIN && magSq <= NORMALIZED_RANGE_SQUARED_MAX;
	}

	// ----------------------------------------------------------------------
	/**
	 * Find the closest point to this along a given (infinite) line.
	 *
	 * @param line0  a reference point on the line
	 * @param line1  another reference point on the line
	 * @return  the point on the line through line0 and line1 which is closest to this point.
	 */
	public vector findClosestPointOnLine(vector line0, vector line1)
	{
		vector delta = line1.subtract(line0);
		float r = subtract(line0).dot(delta) / delta.magnitudeSquared();
		return line0.add(delta.multiply(r));
	}

	// ----------------------------------------------------------------------
	/**
	 * Find the closest point to this along a given line segment.
	 *
	 * @param startPoint  one endpoint of the line segment
	 * @param endPoint    the other endpoint of the line segment
	 * @return  the point on the line segment from startPoint to endPoint which is closest to this point
	 */
	public vector findClosestPointOnLineSegment(vector startPoint, vector endPoint)
	{
		vector delta = endPoint.subtract(startPoint);
		float deltaMagnitudeSquared = delta.magnitudeSquared();
		if (deltaMagnitudeSquared < NORMALIZE_THRESHOLD)
			return startPoint;
		float r = subtract(startPoint).dot(delta) / deltaMagnitudeSquared;
		if (r < 0.0f)
			r = 0.0f;
		else if (r > 1.0f)
			r = 1.0f;
		return startPoint.add(delta.multiply(r));
	}


	// ----------------------------------------------------------------------
	/**
	 * Determine if this point is within a given right conical frustum
	 *
	 * @param startPoint  one endpoint of the frustum
	 * @param endPoint    the other endpoint of the frustum
	 * @param startRadius the radius of the frustum at startPoint
	 * @param endRadius   the radius of the frustum at endPoint
	 * @return  true if the point is within the frustum, false otherwise
	 */
	public boolean isWithinConicalFrustum(vector startPoint, vector endPoint, float startRadius, float endRadius)
	{
		vector delta = endPoint.subtract(startPoint);
		float deltaMagnitudeSquared = delta.magnitudeSquared();
		if (deltaMagnitudeSquared < NORMALIZE_THRESHOLD)
		{
			return false;
		}
		float t = subtract(startPoint).dot(delta) / deltaMagnitudeSquared;
		if (t < 0.0f || t > 1.0f) // off the end of the axis
		{
			return false;
		}
		
		vector closestPointOnAxis = startPoint.add(delta.multiply(t));
		float distanceFromAxisSquared = magnitudeBetweenSquared(closestPointOnAxis);
		float frustumRadius = (endRadius - startRadius) * t + startRadius;

		return (frustumRadius*frustumRadius) > distanceFromAxisSquared;
	}
	
	// ----------------------------------------------------------------------
	/**
	 * Determine if this point is within a cone
	 *
	 * @param startPoint  The tip of the cone
	 * @param endPoint    A point along the cone axis
	 * @param range       The max distance allowed
	 * @param halfAngle   The allowed angle from the axis to be in the cone
	 * @return  true if the point is within the cone, false otherwise
	 */
	public boolean isWithinCone(vector startPoint, vector endPoint, float range, float halfAngle)
	{
		vector coneTestPoint = subtract(startPoint);
		
		float distSquared = coneTestPoint.magnitudeSquared();
		
		if(distSquared > range*range)
		{
			return false;
		}
		else if (distSquared < NORMALIZE_THRESHOLD)
		{
			return true;
		}
		
		vector coneAxis = endPoint.subtract(startPoint);
		
		if(coneAxis.magnitudeSquared() < NORMALIZE_THRESHOLD)
		{
			return false;
		}
		
		coneAxis = coneAxis.normalize();
		coneTestPoint = coneTestPoint.normalize();
		
		float cosAngle = (float)Math.cos(Math.toRadians(halfAngle));
		float dot = coneTestPoint.dot(coneAxis);
		boolean withinCone = (dot >= cosAngle);
		
		return withinCone;
	}
	
	// ----------------------------------------------------------------------
	/**
	 * Calculate the distance from this point to a specified (infinite) line.
	 *
	 * @param line0  a reference point on the line
	 * @param line1  another reference point on the line
	 * @return  the distance from this point to the specified line
	 */
	public float distanceToLine(vector line0, vector line1)
	{
		return magnitudeBetween(findClosestPointOnLine(line0, line1));
	}

	// ----------------------------------------------------------------------
	/**
	 * Calculate the distance from this point to a specified line segment.
	 *
	 * @param startPoint  one endpoint of the line segment
	 * @param endPoint    the other endpoint of the line segment
	 * @return  the distance from this point to the specified line segment
	 */
	public float distanceToLineSegment(vector startPoint, vector endPoint)
	{
		return magnitudeBetween(findClosestPointOnLineSegment(startPoint, endPoint));
	}

	// ----------------------------------------------------------------------
	/**
	 * Calculate the rotation of the vector around the Y plane.
	 *
	 * @return  the rotation of the vector around the Y plane, in radians - result is undefined if x == z == 0
	 */
	public float theta()
	{
		return (float)Math.atan2(x, z);
	}

	// ----------------------------------------------------------------------
	/**
	 * Calculate the angle of the vector from the XZ plane.
	 *
	 * @return  the angle of the vector from the XZ plane, in radians.
	 */
	public float phi()
	{
		return (float)Math.atan2(-y, Math.sqrt(x*x+z*z));
	}

	// ----------------------------------------------------------------------
	/**
	 * Compute the dot product between this vector and another vector.
	 *
	 * The dot product value is equal to the cosine of the angle between the two vectors multiplied by the sum of the lengths of the vectors.
	 *
	 * @param vec  vector to compute the dot product against.
	 * @return  the dot product of this vector and vec.
	 */
	public float dot(vector vec)
	{
		return x*vec.x + y*vec.y + z*vec.z;
	}

	// ----------------------------------------------------------------------
	/**
	 * Calculate the cross product between this vector and another vector.
	 *
	 * @param rhs  The right hand side of the cross product operation
	 * @return  the vector which is ((this) cross (rhs)).
	 */
	public vector cross(vector rhs)
	{
		return new vector(y*rhs.z - z*rhs.y, z*rhs.x - x*rhs.z, x*rhs.y - y*rhs.x);
	}

	// ----------------------------------------------------------------------
	/**
	 * Calculate the magnitude squared of this vector.
	 *
	 * In situations where squared magnitudes can be dealt with rather than magnitudes, doing so is preferable as it avoids square root operations.
	 *
	 * @return  the magnitude squared of this vector
	 */
	public float magnitudeSquared()
	{
		return x*x + y*y + z*z;
	}

	// ----------------------------------------------------------------------
	/**
	 * Calculate the approximate magnitude of this vector, within about 8%.
	 *
	 * When efficiency is needed, and squared magnitudes cannot be used, but accuracy need not be perfect, this is the preferred method to use.
	 *
	 * @return  the approximate magnitude of this vector
	 */
	public float approximateMagnitude()
	{
		float minc = Math.abs(x);
		float midc = Math.abs(y);
		float maxc = Math.abs(z);

		if (midc < minc)
		{
			float temp = midc;
			midc = minc;
			minc = temp;
		}

		if (maxc < minc)
		{
			float temp = maxc;
			maxc = minc;
			minc = temp;
		}

		if (maxc < midc)
		{
			float temp = maxc;
			maxc = midc;
			midc = temp;
		}

		return maxc + 11.0f/32.0f*midc + 0.25f*minc;
	}

	// ----------------------------------------------------------------------
	/**
	 * Calculate the magnitude of this vector.
	 *
	 * This requires a square root operation, so should only be used when necessary.  See magnitudeSquared() and approximateMagnitude().
	 *
	 * @return  the magnitude of this vector
	 */
	public float magnitude()
	{
		return (float)Math.sqrt(magnitudeSquared());
	}

	// ----------------------------------------------------------------------
	/**
	 * Calculate the magnitude between this vector and another vector.
	 *
	 * This requires a square root operation, so should only be used when necessary.  See magnitudeBetweenSquared().
	 *
	 * @param vec  the vector to compare against
	 * @return  the magnitude between this and vec
	 */
	public float magnitudeBetween(vector vec)
	{
		return (float)Math.sqrt(magnitudeBetweenSquared(vec));
	}

	// ----------------------------------------------------------------------
	/**
	 * Calculate the square of the magnitude between this vector and another vector.
	 *
	 * @param vec  the vector to compare against
	 * @return  the square of the magnitude between this and vec
	 */
	public float magnitudeBetweenSquared(vector vec)
	{
		float dx = x-vec.x;
		float dy = y-vec.y;
		float dz = z-vec.z;
		return dx*dx + dy*dy + dz*dz;
	}

	// ----------------------------------------------------------------------
	/**
	 * Determine if two vectors are within an epsilon distance.
	 *
	 * The epsilon distance is tested for all three dimensions, not a distance between the points.
	 *
	 * @param vec  the vector to compare this against
	 * @param epsilon  the epsilon distance
	 * @return whether this and vec are within the epsilon distance of each other
	 */
	public boolean withinEpsilon(vector vec, float epsilon)
	{
		return Math.abs(x-vec.x) < epsilon && Math.abs(y-vec.y) < epsilon && Math.abs(z-vec.z) < epsilon;
	}

	// ----------------------------------------------------------------------
	/**
	 * Reflect an incoming vector around this normal.
	 *
	 * This routine assumes that the incident vector terminates at this normal (the dot product of the normal and the vertex is negative).
	 *
	 * @param incident  Normal to reflect this vector around
	 * @return  the reflected vector
	 */
	public vector reflectIncoming(vector incident)
	{
		return reflectOutgoing(incident.negate());
	}

	// ----------------------------------------------------------------------
	/**
	 * Reflect an outgoing vector around this normal.
	 *
	 * This routine assumes that both the incident vector and the normal around which it is being reflected have the same origin (the dot product of the normal and the vertex is positive).
	 *
	 * @param incident  Normal to reflect this vector around
	 * @return  the reflected vector
	 */
	public vector reflectOutgoing(vector incident)
	{
		return multiply(2*dot(incident)).subtract(incident);
	}

	// ----------------------------------------------------------------------
	/**
	 * Negate this vector.
	 *
	 * @return  the negated vector (this * -1)
	 */
	public vector negate()
	{
		return new vector(-x, -y, -z);
	}

	// ----------------------------------------------------------------------
	/**
	 * Add this and another vector.
	 *
	 * @param rhs  the vector to add to this
	 * @return  the sum of this and rhs
	 */
	public vector add(vector rhs)
	{
		return new vector(x+rhs.x, y+rhs.y, z+rhs.z);
	}

	// ----------------------------------------------------------------------
	/**
	 * Subtract another vector from this vector.
	 *
	 * @param rhs  the vector to subtract to this
	 * @return  the difference of this and rhs
	 */
	public vector subtract(vector rhs)
	{
		return new vector(x-rhs.x, y-rhs.y, z-rhs.z);
	}

	// ----------------------------------------------------------------------
	/**
	 * Multiply this vector by a scalar.
	 *
	 * @param scalar  the scalar value to multiply this by
	 * @return  the product of this vector and the scalar
	 */
	public vector multiply(float scalar)
	{
		return new vector(x*scalar, y*scalar, z*scalar);
	}

	// ----------------------------------------------------------------------
	/**
	 * Divide this vector by a scalar.
	 *
	 * @param scalar  the scalar value to divide this by
	 * @return  the resulting divided vector
	 */
	public vector divide(float scalar)
	{
		return new vector(x/scalar, y/scalar, z/scalar);
	}

	// ----------------------------------------------------------------------
	/**
	 * Determine the midpoint of the line segment between 2 points.
	 *
	 * @param v1  one endpoint of the line segment
	 * @param v2  the other endpoint of the line segment
	 * @return  the midpoint of the line segment between v1 and v2
	 */
	public static vector midpoint(vector v1, vector v2)
	{
		return new vector((v1.x+v2.x)*0.5f, (v1.y+v2.y)*0.5f, (v1.z+v2.z)*.05f);
	}

	// ----------------------------------------------------------------------
	/**
	 * Linearly interpolate between two vectors.
	 *
	 * @param begin  the starting endpoint
	 * @param end    the terminating endpoint
	 * @param t      the time for the interpolation, from 0.0 (begin) to 1.0 (end)
	 * @return  the point along the line segment from begin to end at time t
	 */
	public static vector linearInterpolate(vector begin, vector end, float t)
	{
		return new vector(begin.x+(end.x-begin.x)*t, begin.y+(end.y-begin.y)*t, begin.z+(end.z-begin.z)*t);
	}

	// ----------------------------------------------------------------------
	/**
	 * Create a random unit vector.
	 *
	 * @return  a random, evenly distributed vector on the unit sphere.
	 */
	public static vector randomUnit()
	{
		float lz = random.rand(-1.0f, 1.0f);
		float t = random.rand(0.0f, (float)Math.PI*2.0f);
		double r = Math.sqrt(1.0f-lz*lz);
		return new vector((float)(r*Math.cos(t)), (float)(r*Math.sin(t)), lz);
	}

	// ----------------------------------------------------------------------
	/**
	 * Create a random vector within a cube of specified size.
	 *
	 * @param halfSideLength  half the length of a side of the cube
	 * @return  a random vector within an axially aligned cube centered at the origin with sides of length halfSideLength*2
	 */
	public static vector randomCube(float halfSideLength)
	{
		return new vector(random.rand(-halfSideLength, halfSideLength), random.rand(-halfSideLength, halfSideLength), random.rand(-halfSideLength, halfSideLength));
	}

	// ----------------------------------------------------------------------
}

// ======================================================================

