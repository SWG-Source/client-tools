// ======================================================================
//
// transform.java
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

package script;

import java.lang.Math;
import java.io.Serializable;

// ======================================================================

/**
 * The transform class represents a 4x4 matrix with only translation and orientation components.
 */
public final class transform implements Comparable, Serializable
{
	// ----------------------------------------------------------------------

	private final static long serialVersionUID = 2864167306712372792L;
	/** a 3x4 matrix representing a 4x4 with row 4 == (0, 0, 0, 1) */
	private final float[][] matrix;
	/** The identity transform (identity orientation, position at origin) */
	public final static transform identity = new transform(vector.unitX, vector.unitY, vector.unitZ, vector.zero);

	// ----------------------------------------------------------------------
	/**
	 * Construct a default transform.
	 */
	public transform()
	{
		matrix = identity.matrix;
	}

	// ----------------------------------------------------------------------
	/**
	 * Construct a transform given all elements of its matrix.
	 */
	public transform(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23)
	{
		matrix = new float[3][4];
		matrix[0][0] = m00;
		matrix[0][1] = m01;
		matrix[0][2] = m02;
		matrix[0][3] = m03;

		matrix[1][0] = m10;
		matrix[1][1] = m11;
		matrix[1][2] = m12;
		matrix[1][3] = m13;

		matrix[2][0] = m20;
		matrix[2][1] = m21;
		matrix[2][2] = m22;
		matrix[2][3] = m23;
	}

	// ----------------------------------------------------------------------
	/**
	 * Construct a matrix given its left handed orthonormal basis and translation vectors.
	 *
	 * @param i  unit vector along the X axis
	 * @param j  unit vector along the Y axis
	 * @param k  unit vector along the Z axis
	 * @param p  translation vector
	 */
	public transform(vector i, vector j, vector k, vector p)
	{
		matrix = new float[3][4];
		matrix[0][0] = i.x;
		matrix[0][1] = j.x;
		matrix[0][2] = k.x;
		matrix[0][3] = p.x;

		matrix[1][0] = i.y;
		matrix[1][1] = j.y;
		matrix[1][2] = k.y;
		matrix[1][3] = p.y;

		matrix[2][0] = i.z;
		matrix[2][1] = j.z;
		matrix[2][2] = k.z;
		matrix[2][3] = p.z;

		validate();
	}

	// ----------------------------------------------------------------------

	public transform(transform src)
	{
		matrix = src.matrix;
	}

	// ----------------------------------------------------------------------

	public Object clone()
	{
		return new transform(this);
	}

	// ----------------------------------------------------------------------

	public String toString()
	{
		return
			"[ " + matrix[0][0] + " " + matrix[0][1] + " " + matrix[0][2] + " " + matrix[0][3] + " ]\n" +
			"[ " + matrix[1][0] + " " + matrix[1][1] + " " + matrix[1][2] + " " + matrix[1][3] + " ]\n" +
			"[ " + matrix[2][0] + " " + matrix[2][1] + " " + matrix[2][2] + " " + matrix[2][3] + " ]";
	}

	// ----------------------------------------------------------------------

	public int compareTo(Object o) throws ClassCastException
	{
		return compareTo((transform)o);
	}

	// ----------------------------------------------------------------------
	/**
	 * Compare this transform against another transform.
	 *
	 * @param tr  the transform to compare this against
	 * @return  0 if all elements are equal, -1 if the first non-equal element of this encountered traversing the matricies is less than the corresponding element of tr, or else 1
	 */
	public int compareTo(transform tr)
	{
		for (int i = 0; i < 3; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				if (matrix[i][j] < tr.matrix[i][j])
					return -1;
				else if (matrix[i][j] > tr.matrix[i][j])
					return 1;
			}
		}
		return 0;
	}

	// ----------------------------------------------------------------------

	public boolean equals(Object o)
	{
		if (o != null)
		{
			try
			{
				transform tr = (transform)o;
				for (int i = 0; i < 3; ++i)
					for (int j = 0; j < 4; ++j)
						if (matrix[i][j] != tr.matrix[i][j])
							return false;
				return true;
			}
			catch (ClassCastException err)
			{
			}
		}
		return false;
	}

	// ----------------------------------------------------------------------

	public void validate() throws java.lang.ArithmeticException
	{
		vector i = getLocalFrameI_p();
		vector j = getLocalFrameJ_p();
		vector k = getLocalFrameK_p();
		if (   Math.abs(i.magnitudeSquared()-1.0f) > 0.0005f
		    || Math.abs(j.magnitudeSquared()-1.0f) > 0.0005f
		    || Math.abs(k.magnitudeSquared()-1.0f) > 0.0005f
		    || Math.abs(i.dot(j)) > 0.0001f
		    || Math.abs(j.dot(k)) > 0.0001f
		    || Math.abs(i.dot(k)) > 0.0001f)
			throw new ArithmeticException("transform basis is not orthonormal (i="+i+", j="+j+", k="+k+")");
	}

	// ----------------------------------------------------------------------
	/**
	 * Reorthogonalize a transform.
	 *
	 * Repeated rotations will introduce numerical error into the transform,
	 * which will cause the upper 3x3 matrix to become non-orthonormal.  If
	 * enough error is introduced, weird errors will begin to occur when using
	 * the transform.  This routine attempts to reduce the numerical error by
	 * reorthonormalizing the upper 3x3 matrix.
	 *
	 * @return the reorthonormalized transform.
	 */
	public transform reorthonormalize()
	{
		vector k = getLocalFrameK_p().normalize();
		vector i = getLocalFrameJ_p().normalize().cross(k);
		vector j = k.cross(i);

		return new transform(i, j, k, getPosition_p());
	}

	// ----------------------------------------------------------------------
	/**
	 * Multiply two transforms together.
	 *
	 * The matrices and the multiply are defined as follows:
	 * a b c d     m n o p      am+bq+cu an+br+cv ao+bs+cw ap+bt+cx+d
	 * e f g h     q r s t      em+fq+gu en+fr+gv eo+fs+gw ep+ft+gx+h
	 * i j k l  *  u v w x  =   im+jq+ku in+jr+kv io+js+kw ip+jt+kx+l
	 * 0 0 0 1     0 0 0 1             0        0        0          1
	 * where the bottom row of the matrix is not stored.
	 *
	 * @param tr  the transform to multiply this transform by
	 * @return  the product of this and tr
	 */
	public transform multiply(transform tr)
	{
		return new transform(
			matrix[0][0] * tr.matrix[0][0] + matrix[0][1] * tr.matrix[1][0] + matrix[0][2] * tr.matrix[2][0],
			matrix[0][0] * tr.matrix[0][1] + matrix[0][1] * tr.matrix[1][1] + matrix[0][2] * tr.matrix[2][1],
			matrix[0][0] * tr.matrix[0][2] + matrix[0][1] * tr.matrix[1][2] + matrix[0][2] * tr.matrix[2][2],
			matrix[0][0] * tr.matrix[0][3] + matrix[0][1] * tr.matrix[1][3] + matrix[0][2] * tr.matrix[2][3] + matrix[0][3],
			matrix[1][0] * tr.matrix[0][0] + matrix[1][1] * tr.matrix[1][0] + matrix[1][2] * tr.matrix[2][0],
			matrix[1][0] * tr.matrix[0][1] + matrix[1][1] * tr.matrix[1][1] + matrix[1][2] * tr.matrix[2][1],
			matrix[1][0] * tr.matrix[0][2] + matrix[1][1] * tr.matrix[1][2] + matrix[1][2] * tr.matrix[2][2],
			matrix[1][0] * tr.matrix[0][3] + matrix[1][1] * tr.matrix[1][3] + matrix[1][2] * tr.matrix[2][3] + matrix[1][3],
			matrix[2][0] * tr.matrix[0][0] + matrix[2][1] * tr.matrix[1][0] + matrix[2][2] * tr.matrix[2][0],
			matrix[2][0] * tr.matrix[0][1] + matrix[2][1] * tr.matrix[1][1] + matrix[2][2] * tr.matrix[2][1],
			matrix[2][0] * tr.matrix[0][2] + matrix[2][1] * tr.matrix[1][2] + matrix[2][2] * tr.matrix[2][2],
			matrix[2][0] * tr.matrix[0][3] + matrix[2][1] * tr.matrix[1][3] + matrix[2][2] * tr.matrix[2][3] + matrix[2][3]);
	}

	// ----------------------------------------------------------------------
	/**
	 * Calculate the inverse of this transform.
	 *
	 * @return  the inverse of this transform.
	 */
	public transform invert()
	{
		// transpose the upper 3x3 matrix and invert the translation
		return new transform(
			matrix[0][0], matrix[1][0], matrix[2][0], -(matrix[0][0] * matrix[0][3] + matrix[0][1] * matrix[1][3] + matrix[0][2] * matrix[2][3]),
			matrix[0][1], matrix[1][1], matrix[2][1], -(matrix[1][1] * matrix[0][3] + matrix[1][1] * matrix[1][3] + matrix[1][2] * matrix[2][3]),
			matrix[0][2], matrix[1][2], matrix[2][2], -(matrix[2][0] * matrix[0][3] + matrix[2][1] * matrix[1][3] + matrix[2][2] * matrix[2][3]));
	}

	// ----------------------------------------------------------------------
	/**
	 * Move this transform in its own local space.
	 *
	 * This routine moves the transform according to its current frame of reference.
	 * Therefore, moving along the Z axis will move the transform forward in the direction in which it is pointed.
	 *
	 * @param vec  vector to rotate and translate
	 * @return  the resulting translated transform
	 */
	public transform move_l(vector vec)
	{
		return move_p(rotate_l2p(vec));
	}

	// ----------------------------------------------------------------------
	/**
	 * Move this transform in its parent space.
	 *
	 * This routine moves the transform in its parent space, or the world space if the transform has no parent.
	 * Therefore, moving along the Z axis will move the transform forward along the Z axis of its parent space, not forward in the direction in which it is pointed.
	 *
	 * @param vec  Displacement to move in parent space
	 * @return  the resulting translated transform
	 */
	public transform move_p(vector vec)
	{
		return new transform(
			matrix[0][0], matrix[0][1], matrix[0][2], matrix[0][3] + vec.x,
			matrix[1][0], matrix[1][1], matrix[1][2], matrix[1][3] + vec.y,
			matrix[2][0], matrix[2][1], matrix[2][2], matrix[2][3] + vec.z);
	}

	// ----------------------------------------------------------------------
	/**
	 * Yaw this transform.
	 *
	 * This routine will rotate the transform around the Y axis by the specified number of radians.
	 * Positive rotations are clockwise when viewed looking at the origin from the positive side of the axis around which the transform is being rotated.
	 *
	 * @param radians  amount to rotate, in radians
	 * @return  the resulting yawed transform
	 */
	public transform yaw_l(float radians)
	{
		float sine   = (float)Math.sin(radians);
		float cosine = (float)Math.cos(radians);

		return new transform(
			matrix[0][0]*cosine + matrix[0][2]*(-sine), matrix[0][1], matrix[0][0]*sine  + matrix[0][2]*cosine, matrix[0][3],
			matrix[1][0]*cosine + matrix[1][2]*(-sine), matrix[1][1], matrix[1][0]*sine  + matrix[1][2]*cosine, matrix[1][3],
			matrix[2][0]*cosine + matrix[2][2]*(-sine), matrix[2][1], matrix[2][0]*sine  + matrix[2][2]*cosine, matrix[2][3]);
	}

	// ----------------------------------------------------------------------
	/**
	 * Pitch this transform.
	 *
	 * This routine will rotate the transform around the X axis by the specified number of radians.
	 * Positive rotations are clockwise when viewed looking at the origin from the positive side of the axis around which the transform is being rotated.
	 *
	 * @param radians  amount to rotate, in radians
	 * @return  the resulting pitched transform
	 */
	public transform pitch_l(float radians)
	{
		float sine   = (float)Math.sin(radians);
		float cosine = (float)Math.cos(radians);

		return new transform(
			matrix[0][0], matrix[0][1]*cosine + matrix[0][2]*sine, matrix[0][1]*(-sine) + matrix[0][2]*cosine, matrix[0][3],
			matrix[1][0], matrix[1][1]*cosine + matrix[1][2]*sine, matrix[1][1]*(-sine) + matrix[1][2]*cosine, matrix[1][3],
			matrix[2][0], matrix[2][1]*cosine + matrix[2][2]*sine, matrix[2][1]*(-sine) + matrix[2][2]*cosine, matrix[2][3]);
	}

	// ----------------------------------------------------------------------
	/**
	 * Roll this transform.
	 *
	 * This routine will rotate the transform around the Z axis by the specified number of radians.
	 * Positive rotations are clockwise when viewed looking at the origin from the positive side of the axis around which the transform is being rotated.
	 *
	 * @param radians  amount to rotate, in radians
	 * @return  the resulting rolled transform
	 */
	public transform roll_l(float radians)
	{
		float sine   = (float)Math.sin(radians);
		float cosine = (float)Math.cos(radians);

		return new transform(
			matrix[0][0]*cosine + matrix[0][1]*sine, matrix[0][0]*(-sine) + matrix[0][1]*cosine, matrix[0][2], matrix[0][3],
			matrix[1][0]*cosine + matrix[1][1]*sine, matrix[1][0]*(-sine) + matrix[1][1]*cosine, matrix[1][2], matrix[1][3],
			matrix[2][0]*cosine + matrix[2][1]*sine, matrix[2][0]*(-sine) + matrix[2][1]*cosine, matrix[2][2], matrix[2][3]);
	}

	// ----------------------------------------------------------------------
	/**
	 * Set the orientation to the identity orientation, leaving the position intact.
	 *
	 * @return  the resulting transform, with the identity orientation but the position of the original transform.
	 */
	public transform resetRotate_l2p()
	{
		return new transform(
			1.0f, 0.0f, 0.0f, matrix[0][3],
			0.0f, 1.0f, 0.0f, matrix[1][3],
			0.0f, 0.0f, 1.0f, matrix[2][3]);
	}

	// ----------------------------------------------------------------------
	/**
	 * Get the position (translation component) from the transform.
	 *
	 * @return  the position vector
	 */
	public vector getPosition_p()
	{
		return new vector(matrix[0][3], matrix[1][3], matrix[2][3]);
	}

	// ----------------------------------------------------------------------
	/**
	 * Set the position (translation component) of the transform.
	 *
	 * @param vec  the new position vector
	 * @return  the transform with its translation component set to the new position
	 */
	public transform setPosition_p(vector vec)
	{
		return new transform(
			matrix[0][0], matrix[0][1], matrix[0][2], vec.x,
			matrix[1][0], matrix[1][1], matrix[1][2], vec.y,
			matrix[2][0], matrix[2][1], matrix[2][2], vec.z);
	}

	// ----------------------------------------------------------------------
	/**
	 * Set the position (translation component) of the transform.
	 *
	 * @param x  the x component of the new position vector
	 * @param y  the y component of the new position vector
	 * @param z  the z component of the new position vector
	 * @return  the transform with its translation component set to the new position
	 */
	public transform setPosition_p(float x, float y, float z)
	{
		return new transform(
			matrix[0][0], matrix[0][1], matrix[0][2], x,
			matrix[1][0], matrix[1][1], matrix[1][2], y,
			matrix[2][0], matrix[2][1], matrix[2][2], z);
	}

	// ----------------------------------------------------------------------
	/**
	 * Get the parent-space vector pointing along the X axis of this frame of reference.
	 *
	 * The vector returned is in parent space, which is world space if the transform has no parent.
	 *
	 * @return  the vector pointing along the X axis of the frame in parent space
	 */
	public vector getLocalFrameI_p()
	{
		return new vector(matrix[0][0], matrix[1][0], matrix[2][0]);
	}

	// ----------------------------------------------------------------------
	/**
	 * Get the parent-space vector pointing along the Y axis of this frame of reference.
	 *
	 * The vector returned is in parent space, which is world space if the transform has no parent.
	 *
	 * @return  the vector pointing along the Y axis of the frame in parent space
	 */
	public vector getLocalFrameJ_p()
	{
		return new vector(matrix[0][1], matrix[1][1], matrix[2][1]);
	}

	// ----------------------------------------------------------------------
	/**
	 * Get the parent-space vector pointing along the Z axis of this frame of reference.
	 *
	 * The vector returned is in parent space, which is world space if the transform has no parent.
	 *
	 * @return  the vector pointing along the Z axis of the frame in parent space
	 */
	public vector getLocalFrameK_p()
	{
		return new vector(matrix[0][2], matrix[1][2], matrix[2][2]);
	}

	// ----------------------------------------------------------------------
	/**
	 * Set the transform matrix from the i, j, and k vectors.
	 *
	 * This routine assumes that i, j, and k are a left-handed orthonormal basis.
	 * If they are not, the transform must be reorthonormalized after this routine.
	 *
	 * @param i  unit vector along the X axis
	 * @param j  unit vector along the Y axis
	 * @param k  unit vector along the Z axis
	 * @return  the resulting vector with the newly set orientation and original translation
	 */
	public transform setLocalFrameIJK_p(vector i, vector j, vector k)
	{
		return new transform(i, j, k, getPosition_p());
	}

	// ----------------------------------------------------------------------
	/**
	 * Set the transform matrix from the k and j vectors.
	 *
	 * This routine assumes that k and j are part of a left-handed orthonormal basis.
	 * If they are not, the transform must be reorthonormalized after this routine.
	 *
	 * @param k  unit vector along the Z axis
	 * @param j  unit vector along the Y axis
	 * @return  the resulting vector with the newly set orientation and original translation
	 */
	public transform setLocalFrameKJ_p(vector k, vector j)
	{
		return new transform(j.cross(k), j, k, getPosition_p());
	}

	// ----------------------------------------------------------------------
	/**
	 * Get the transform-space vector pointing along the X axis of the parent of reference.
	 *
	 * @return  the vector pointing along the X axis of the parent's frame in local space
	 */
	public vector getParentFrameI_l()
	{
		return new vector(matrix[0][0], matrix[0][1], matrix[0][2]);
	}

	// ----------------------------------------------------------------------
	/**
	 * Get the transform-space vector pointing along the Y axis of the parent of reference.
	 *
	 * @return  the vector pointing along the Y axis of the parent's frame in local space
	 */
	public vector getParentFrameJ_l()
	{
		return new vector(matrix[1][0], matrix[1][1], matrix[1][2]);
	}

	// ----------------------------------------------------------------------
	/**
	 * Get the transform-space vector pointing along the Z axis of the parent of reference.
	 *
	 * @return  the vector pointing along the Z axis of the parent's frame in local space
	 */
	public vector getParentFrameK_l()
	{
		return new vector(matrix[2][0], matrix[2][1], matrix[2][2]);
	}

	// ----------------------------------------------------------------------
	/**
	 * Rotate a vector from the transform's current frame to the parent frame.
	 *
	 * Pure rotation is most useful for vectors that are orientational, such as normals.
	 *
	 * @param vec  the vector to rotate
	 * @return  the vector in parent space
	 */
	public vector rotate_l2p(vector vec)
	{
		return new vector(
			matrix[0][0] * vec.x + matrix[0][1] * vec.y + matrix[0][2] * vec.z,
			matrix[1][0] * vec.x + matrix[1][1] * vec.y + matrix[1][2] * vec.z,
			matrix[2][0] * vec.x + matrix[2][1] * vec.y + matrix[2][2] * vec.z);
	}

	// ----------------------------------------------------------------------
	/**
	 * Transform a vector from the transform's current frame to the parent frame.
	 *
	 * Rotation and translation is most useful for vectors that are position, such as vertex data.
	 *
	 * @param vec  vector to rotate and translate
	 * @return  the vector in parent space
	 */
	public vector rotateTranslate_l2p(vector vec)
	{
		return new vector(
			matrix[0][0] * vec.x + matrix[0][1] * vec.y + matrix[0][2] * vec.z + matrix[0][3],
			matrix[1][0] * vec.x + matrix[1][1] * vec.y + matrix[1][2] * vec.z + matrix[1][3],
			matrix[2][0] * vec.x + matrix[2][1] * vec.y + matrix[2][2] * vec.z + matrix[2][3]);
	}

	// ----------------------------------------------------------------------
	/**
	 * Rotate a vector from the parent space to the local transform space.
	 *
	 * Pure rotation is most useful for vectors that are orientational, such as normals.
	 *
	 * @param vec  the vector to rotate
	 * @return  the vector in local space
	 */
	public vector rotate_p2l(vector vec)
	{
		return new vector(
			matrix[0][0] * vec.x + matrix[1][0] * vec.y + matrix[2][0] * vec.z,
			matrix[0][1] * vec.x + matrix[1][1] * vec.y + matrix[2][1] * vec.z,
			matrix[0][2] * vec.x + matrix[1][2] * vec.y + matrix[2][2] * vec.z);
	}

	// ----------------------------------------------------------------------
	/**
	 * Transform a vector from the parent space to to the local transform space.
	 *
	 * Rotation and translation is most useful for vectors that are position, such as vertex data.
	 *
	 * @param vec  vector to rotate and translate
	 * @return  the vector in local space
	 */
	public vector rotateTranslate_p2l(vector vec)
	{
		float x = vec.x - matrix[0][3];
		float y = vec.y - matrix[1][3];
		float z = vec.z - matrix[2][3];

		return new vector(
		  matrix[0][0] * x + matrix[1][0] * y + matrix[2][0] * z,
		  matrix[0][1] * x + matrix[1][1] * y + matrix[2][1] * z,
		  matrix[0][2] * x + matrix[1][2] * y + matrix[2][2] * z);
	}

	// ----------------------------------------------------------------------
	/**
	 * Transform a transform from local transform space to parent transform space.
	 *
	 * @param tr  the transform to change from local space to parent space
	 * @return  the transform in parent space
	 */
	public transform rotateTranslate_l2p(transform tr)
	{
		return multiply(tr).reorthonormalize();
	}

	// ----------------------------------------------------------------------
	/**
	 * Transform a transform from parent transform space to the local transform space.
	 *
	 * @param tr  the transform to change from parent space to local space
	 * @return  the transform in local space
	 */
	public transform rotateTranslate_p2l(transform tr)
	{
		return tr.multiply(this).reorthonormalize();
	}

	// ----------------------------------------------------------------------
	/**
	 * Determine whether this transform approximates another, within given tolerances.
	 *
	 * @param tr  the transform to compare this against
	 * @param rotDelta  the tolerance for the rotational comparison
	 * @param posDelta  the tolerance for the positional comparison
	 * @return  whether tr approximates this transform, within the given tolerances.
	 */
	public boolean approximates(transform tr, float rotDelta, float posDelta)
	{
		float rotDeltaCheck = 1-rotDelta;

		for (int i = 0; i < 3; ++i)
			if (matrix[0][i]*tr.matrix[0][i] + matrix[1][i]*tr.matrix[1][i]+matrix[2][i]*tr.matrix[2][i] < rotDeltaCheck)
				return false;

		float dx = matrix[0][3]-tr.matrix[0][3];
		float dy = matrix[1][3]-tr.matrix[1][3];
		float dz = matrix[2][3]-tr.matrix[2][3];
		return dx*dx+dy*dy+dz*dz <= posDelta*posDelta;
	}

	// ----------------------------------------------------------------------

}

// ======================================================================

