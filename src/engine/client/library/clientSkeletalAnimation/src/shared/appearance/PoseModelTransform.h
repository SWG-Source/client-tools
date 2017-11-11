// ======================================================================
//
// PoseModelTransform.h
// Copyright 2005 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_PoseModelTransform_H
#define INCLUDED_PoseModelTransform_H

#include "sharedMath/Vector.h"
#include "sharedMath/Transform.h"

//-----------------------------------
#undef POINTER_ALIGN_16
#define POINTER_ALIGN_16(x) ((byte *)((UINT_PTR(x)+15) & -16))

#undef STACK_ALLOC_ALIGN_16
#define STACK_ALLOC_ALIGN_16(_size_) POINTER_ALIGN_16(_alloca(_size_+15))

#undef POINTER_ALIGN_32
#define POINTER_ALIGN_32(x) ((byte *)((UINT_PTR(x)+31) & -32))

#undef STACK_ALLOC_ALIGN_32
#define STACK_ALLOC_ALIGN_32(_size_) POINTER_ALIGN_32(_alloca(_size_+31))

#undef POINTER_BYTE_DISTANCE
#define POINTER_BYTE_DISTANCE(_x_, _y_) (((uint8 *)_y_) - ((uint8 *)_x_))
//-----------------------------------

// ===========================================================================
// Matrix is stored in column-major form.
//
// This is sort of a temporary "quickie" implementation of a real column-major
// 4x4 homogenous matrix class.  
//
// align(16) is to help with SSE compatibility
class __declspec(align(16)) PoseModelTransform
{
public:

	PoseModelTransform() {}
	PoseModelTransform(const Transform &x) 
	{
		const Transform::matrix_t &xm = x.getMatrix();

		// row 0
		matrix[0][0] = xm[0][0];
		matrix[1][0] = xm[0][1];
		matrix[2][0] = xm[0][2];
		matrix[3][0] = xm[0][3];

		// row 1
		matrix[0][1] = xm[1][0];
		matrix[1][1] = xm[1][1];
		matrix[2][1] = xm[1][2];
		matrix[3][1] = xm[1][3];

		// row 2
		matrix[0][2] = xm[2][0];
		matrix[1][2] = xm[2][1];
		matrix[2][2] = xm[2][2];
		matrix[3][2] = xm[2][3];

		// row 3
		matrix[0][3] = 0;
		matrix[1][3] = 0;
		matrix[2][3] = 0;
		matrix[3][3] = 1;
	}

	void makeIdentity()
	{
		// row 0
		matrix[0][0] = 1;
		matrix[1][0] = 0;
		matrix[2][0] = 0;
		matrix[3][0] = 0;

		// row 1
		matrix[0][1] = 0;
		matrix[1][1] = 1;
		matrix[2][1] = 0;
		matrix[3][1] = 0;

		// row 2
		matrix[0][2] = 0;
		matrix[1][2] = 0;
		matrix[2][2] = 1;
		matrix[3][2] = 0;

		// row 3
		matrix[0][3] = 0;
		matrix[1][3] = 0;
		matrix[2][3] = 0;
		matrix[3][3] = 1;
	}

	const Vector rotateTranslate_l2p(const Vector &i_vec3) const
	{
		return Vector (
			matrix[0][0] * i_vec3.x + matrix[1][0] * i_vec3.y + matrix[2][0] * i_vec3.z + matrix[3][0],
			matrix[0][1] * i_vec3.x + matrix[1][1] * i_vec3.y + matrix[2][1] * i_vec3.z + matrix[3][1],
			matrix[0][2] * i_vec3.x + matrix[1][2] * i_vec3.y + matrix[2][2] * i_vec3.z + matrix[3][2]
			);
	}

	const Vector rotate_l2p(const Vector &i_vec3) const
	{
		return Vector (
			matrix[0][0] * i_vec3.x + matrix[1][0] * i_vec3.y + matrix[2][0] * i_vec3.z,
			matrix[0][1] * i_vec3.x + matrix[1][1] * i_vec3.y + matrix[2][1] * i_vec3.z,
			matrix[0][2] * i_vec3.x + matrix[1][2] * i_vec3.y + matrix[2][2] * i_vec3.z
			);
	}

	float matrix[4][4];
};

#endif

