/*****************************************************************************
 *
 * dPVS
 * -----------------------------------------
 *
 * (C) 1999-2001 Hybrid Holding, Ltd.
 * All Rights Reserved.
 *
 * This file consists of unpublished, proprietary source code of
 * Hybrid Holding, and is considered Confidential Information for
 * purposes of non-disclosure agreement. Disclosure outside the terms
 * outlined in signed agreement may result in irrepairable harm to
 * Hybrid Holding and legal action against the party in breach.
 *
 * Description: 	Matrix conversion class
 *
 * Notes:			This class performs conversions between single-precision
 *					and double precision matrices and handles the column/row
 *					major issues. These functions are called only from the
 *					API wrapper.
 *
 * $Archive: /dpvs/implementation/sources/dpvsMatrixConverter.cpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 27.08.02 14:20 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#include "dpvsMatrixConverter.hpp"
#include "dpvsMatrix.hpp"

using namespace DPVS;

bool MatrixConverter::s_columnMajor = false;

//=============================================================================
// matrix conversion into user-defined format
//=============================================================================

void MatrixConverter::convertMatrix	(Matrix4x3& dst, const Matrix4x4& src)
{
#if defined (DPVS_PS2_ASSEMBLY)
//PS2-BEGIN

	DPVS_ASSERT( is128Aligned(&dst) );
	DPVS_ASSERT( is128Aligned(&src) );

	asm __volatile__ (
	"	beq			%2, $0, convertMatrix4344_rowMajor"
	"\n	nop"
		/* column-major */
	"\n	lw $8, 0x00(%1)"
	"\n	sw $8, 0x00(%0)"
	"\n	lw $8, 0x10(%1)"
	"\n	sw $8, 0x04(%0)"
	"\n	lw $8, 0x20(%1)"
	"\n	sw $8, 0x08(%0)"
	"\n	lw $8, 0x30(%1)"
	"\n	sw $8, 0x0c(%0)"
	"\n	lw $8, 0x04(%1)"
	"\n	sw $8, 0x10(%0)"
	"\n	lw $8, 0x14(%1)"
	"\n	sw $8, 0x14(%0)"
	"\n	lw $8, 0x24(%1)"
	"\n	sw $8, 0x18(%0)"
	"\n	lw $8, 0x34(%1)"
	"\n	sw $8, 0x1c(%0)"
	"\n	lw $8, 0x08(%1)"
	"\n	sw $8, 0x20(%0)"
	"\n	lw $8, 0x18(%1)"
	"\n	sw $8, 0x24(%0)"
	"\n	lw $8, 0x28(%1)"
	"\n	sw $8, 0x28(%0)"
	"\n	lw $8, 0x38(%1)"
	"\n	sw $8, 0x2c(%0)"
	"\n	j convertMatrix4344_done"
	"\n	nop"
"\n	convertMatrix4344_rowMajor:"
	"\n	lq $8, 0x00(%1)"
	"\n	sq $8, 0x00(%0)"
	"\n	lq $8, 0x10(%1)"
	"\n	sq $8, 0x10(%0)"
	"\n	lq $8, 0x20(%1)"
	"\n	sq $8, 0x20(%0)"
"\n	convertMatrix4344_done:"
	 : : "r"(&dst), "r"(&src), "r"( (int)s_columnMajor ) : "$8" );
//PS2-END
#else

	if (s_columnMajor)
	{
		DPVS_API_ASSERT(src[0][3] == 0.0f && src[1][3] == 0.0f && src[2][3] == 0.0f && src[3][3] == 1.0f);
	
		dst[0][0] = src[0][0];
		dst[0][1] = src[1][0];
		dst[0][2] = src[2][0];
		dst[0][3] = src[3][0];
		dst[1][0] = src[0][1];
		dst[1][1] = src[1][1];
		dst[1][2] = src[2][1];
		dst[1][3] = src[3][1];
		dst[2][0] = src[0][2];
		dst[2][1] = src[1][2];
		dst[2][2] = src[2][2];
		dst[2][3] = src[3][2];
	}
	else
	{
		DPVS_API_ASSERT(src[3][0] ==  0.0f && src[3][1] == 0.0f && src[3][2] == 0.0f && src[3][3] == 1.0f);
		dst = (const Matrix4x3&)(src);
	}

#endif
}

void MatrixConverter::convertMatrix	(Matrix4x3& dst, const Matrix4x4d& src)
{
	if (s_columnMajor)
	{
		DPVS_API_ASSERT(src[0][3] == 0.0 && src[1][3] == 0.0 && src[2][3] == 0.0 && src[3][3] == 1.0);
	
		Math::doubleToFloat( dst[0][0], src[0][0] );
		Math::doubleToFloat( dst[0][1], src[1][0] );
		Math::doubleToFloat( dst[0][2], src[2][0] );
		Math::doubleToFloat( dst[0][3], src[3][0] );
		Math::doubleToFloat( dst[1][0], src[0][1] );
		Math::doubleToFloat( dst[1][1], src[1][1] );
		Math::doubleToFloat( dst[1][2], src[2][1] );
		Math::doubleToFloat( dst[1][3], src[3][1] );
		Math::doubleToFloat( dst[2][0], src[0][2] );
		Math::doubleToFloat( dst[2][1], src[1][2] );
		Math::doubleToFloat( dst[2][2], src[2][2] );
		Math::doubleToFloat( dst[2][3], src[3][2] );
	}
	else
	{
		DPVS_API_ASSERT(src[3][0] ==  0.0 && src[3][1] ==  0.0 && src[3][2] ==  0.0 && src[3][3] ==  1.0);

		Math::doubleToFloat( dst[0][0], src[0][0] );
		Math::doubleToFloat( dst[0][1], src[0][1] );
		Math::doubleToFloat( dst[0][2], src[0][2] );
		Math::doubleToFloat( dst[0][3], src[0][3] );
		Math::doubleToFloat( dst[1][0], src[1][0] );
		Math::doubleToFloat( dst[1][1], src[1][1] );
		Math::doubleToFloat( dst[1][2], src[1][2] );
		Math::doubleToFloat( dst[1][3], src[1][3] );
		Math::doubleToFloat( dst[2][0], src[2][0] );
		Math::doubleToFloat( dst[2][1], src[2][1] );
		Math::doubleToFloat( dst[2][2], src[2][2] );
		Math::doubleToFloat( dst[2][3], src[2][3] );
	
	}
}

void MatrixConverter::convertMatrix	(Matrix4x4& dst, const Matrix4x3& src)
{
#if defined (DPVS_PS2_ASSEMBLY)
//PS2-BEGIN

	DPVS_ASSERT( is128Aligned(&dst) );
	DPVS_ASSERT( is128Aligned(&src) );

	asm __volatile__ (
	"	beq			%2, $0, convertMatrix4443_rowMajor"
	"\n	nop"
		/* column-major */
	"\n	lw $8, 0x00(%1)"
	"\n	sw $8, 0x00(%0) "
	"\n	lw $8, 0x10(%1)"
	"\n	sw $8, 0x04(%0) "
	"\n	lw $8, 0x20(%1)"
	"\n	sw $8, 0x08(%0) "
	"\n	lw $8, 0x30(%1)"
	"\n	sw $8, 0x0c(%0) "
	"\n	lw $8, 0x04(%1)"
	"\n	sw $8, 0x10(%0) "
	"\n	lw $8, 0x14(%1)"
	"\n	sw $8, 0x14(%0) "
	"\n	lw $8, 0x24(%1)"
	"\n	sw $8, 0x18(%0) "
	"\n	lw $8, 0x34(%1)"
	"\n	sw $8, 0x1c(%0) "
	"\n	lw $8, 0x08(%1)"
	"\n	sw $8, 0x20(%0) "
	"\n	lw $8, 0x18(%1)"
	"\n	sw $8, 0x24(%0) "
	"\n	lw $8, 0x28(%1)"
	"\n	sw $8, 0x28(%0) "
	"\n	lw $8, 0x38(%1)"
	"\n	sw $8, 0x2c(%0)"
	"\n	j convertMatrix4443_done"
	"\n	nop"
"\n	convertMatrix4443_rowMajor:"
	"\n	lq $8, 0x00(%1)"
	"\n	sq $8, 0x00(%0)"
	"\n	lq $8, 0x10(%1)"
	"\n	sq $8, 0x10(%0)"
	"\n	lq $8, 0x20(%1)"
	"\n	sq $8, 0x20(%0)"
	"\n	convertMatrix4443_done:"
	"\n	sqc2 vf0, 0x30(%0)"
	 : : "r"(&dst), "r"(&src), "r"( (int)s_columnMajor ) : "$8" );
//PS2-END
#else

	if (s_columnMajor)
	{
		dst[0][0] = src[0][0];
		dst[0][1] = src[1][0];
		dst[0][2] = src[2][0];
		dst[0][3] = 0.0f;
		dst[1][0] = src[0][1];
		dst[1][1] = src[1][1];
		dst[1][2] = src[2][1];
		dst[1][3] = 0.0f;
		dst[2][0] = src[0][2];
		dst[2][1] = src[1][2];
		dst[2][2] = src[2][2];
		dst[2][3] = 0.0f;
		dst[3][0] = src[0][3];
		dst[3][1] = src[1][3];
		dst[3][2] = src[2][3];
		dst[3][3] = 1.0f;
	}
	else
	{
		dst[0] = src[0];
		dst[1] = src[1];
		dst[2] = src[2];
		dst[3].make(0.0f,0.0f,0.0f,1.0f);
	}

#endif
}

void MatrixConverter::convertMatrix	(Matrix4x4d& dst, const Matrix4x3& src)
{
	if (s_columnMajor)
	{
		Math::floatToDouble( dst[0][0], src[0][0] );
		Math::floatToDouble( dst[0][1], src[1][0] );
		Math::floatToDouble( dst[0][2], src[2][0] );
		Math::floatToDouble( dst[0][3], 0.f );
		Math::floatToDouble( dst[1][0], src[0][1] );
		Math::floatToDouble( dst[1][1], src[1][1] );
		Math::floatToDouble( dst[1][2], src[2][1] );
		Math::floatToDouble( dst[1][3], 0.0f );
		Math::floatToDouble( dst[2][0], src[0][2] );
		Math::floatToDouble( dst[2][1], src[1][2] );
		Math::floatToDouble( dst[2][2], src[2][2] );
		Math::floatToDouble( dst[2][3], 0.0f );
		Math::floatToDouble( dst[3][0], src[0][3] );
		Math::floatToDouble( dst[3][1], src[1][3] );
		Math::floatToDouble( dst[3][2], src[2][3] );
		Math::floatToDouble( dst[3][3], 1.0f );
	}
	else
	{
		Math::floatToDouble( dst[0][0], src[0][0] );
		Math::floatToDouble( dst[0][1], src[0][1] );
		Math::floatToDouble( dst[0][2], src[0][2] );
		Math::floatToDouble( dst[0][3], src[0][3] );
		Math::floatToDouble( dst[1][0], src[1][0] );
		Math::floatToDouble( dst[1][1], src[1][1] );
		Math::floatToDouble( dst[1][2], src[1][2] );
		Math::floatToDouble( dst[1][3], src[1][3] );
		Math::floatToDouble( dst[2][0], src[2][0] );
		Math::floatToDouble( dst[2][1], src[2][1] );
		Math::floatToDouble( dst[2][2], src[2][2] );
		Math::floatToDouble( dst[2][3], src[2][3] );
		Math::floatToDouble( dst[3][0], 0.0f );
		Math::floatToDouble( dst[3][1], 0.0f );
		Math::floatToDouble( dst[3][2], 0.0f );
		Math::floatToDouble( dst[3][3], 1.0f );
	}
}

void MatrixConverter::convertMatrix	(Matrix4x4& dst)
{
	if (s_columnMajor)
		dst.transpose();
}

void MatrixConverter::convertMatrix	(Matrix4x4& dst, const Matrix4x4& src)
{
	if (s_columnMajor)
		dst.transpose(src);
	else
		dst = src;
}

void MatrixConverter::convertMatrix	(Matrix4x4& dst, const Matrix4x4d& src)
{
	if (s_columnMajor)
	{
		Math::doubleToFloat( dst[0][0], src[0][0] );
		Math::doubleToFloat( dst[0][1], src[1][0] );
		Math::doubleToFloat( dst[0][2], src[2][0] );
		Math::doubleToFloat( dst[0][3], src[3][0] );
		Math::doubleToFloat( dst[1][0], src[0][1] );
		Math::doubleToFloat( dst[1][1], src[1][1] );
		Math::doubleToFloat( dst[1][2], src[2][1] );
		Math::doubleToFloat( dst[1][3], src[3][1] );
		Math::doubleToFloat( dst[2][0], src[0][2] );
		Math::doubleToFloat( dst[2][1], src[1][2] );
		Math::doubleToFloat( dst[2][2], src[2][2] );
		Math::doubleToFloat( dst[2][3], src[3][2] );
		Math::doubleToFloat( dst[3][0], src[0][3] );
		Math::doubleToFloat( dst[3][1], src[1][3] );
		Math::doubleToFloat( dst[3][2], src[2][3] );
		Math::doubleToFloat( dst[3][3], src[3][3] );
	}
	else
		dst = src;
}

void MatrixConverter::convertMatrix	(Matrix4x4d& dst, const Matrix4x4& src)
{
	if (s_columnMajor)
	{
		Math::floatToDouble( dst[0][0], src[0][0] );
		Math::floatToDouble( dst[0][1], src[1][0] );
		Math::floatToDouble( dst[0][2], src[2][0] );
		Math::floatToDouble( dst[0][3], src[3][0] );

		Math::floatToDouble( dst[1][0], src[0][1] );
		Math::floatToDouble( dst[1][1], src[1][1] );
		Math::floatToDouble( dst[1][2], src[2][1] );
		Math::floatToDouble( dst[1][3], src[3][1] );
		Math::floatToDouble( dst[2][0], src[0][2] );
		Math::floatToDouble( dst[2][1], src[1][2] );
		Math::floatToDouble( dst[2][2], src[2][2] );
		Math::floatToDouble( dst[2][3], src[3][2] );
		Math::floatToDouble( dst[3][0], src[0][3] );
		Math::floatToDouble( dst[3][1], src[1][3] );
		Math::floatToDouble( dst[3][2], src[2][3] );
		Math::floatToDouble( dst[3][3], src[3][3] );
	}
	else												
		dst = src;
}

//------------------------------------------------------------------------
