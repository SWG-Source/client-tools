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
 * Description:		Math non-inline code
 *
 * $Archive: /dpvs/implementation/sources/dpvsMath.cpp $
 * $Author: wili $
 * $Revision: #1 $
 * $Modtime: 27.08.02 14:15 $
 * $Date: 2003/01/22 $
 *
 ******************************************************************************/

#include "dpvsMath.hpp"
#include "dpvsRectangle.hpp"

#if defined (DPVS_X86_ASSEMBLY)
#	include "dpvsSystem.hpp"										// on X86
#endif

namespace DPVS
{
namespace Math
{
#if defined (DPVS_DEBUG)

static DPVS_FORCE_INLINE bool isApproximatelyEqual(float a, float b)
{
	if (Math::fabs(a-b) <= 0.001f)
		return true;

	return Math::fabs(a-b) <= 0.001f * fabs(a) ||
		   Math::fabs(a-b) <= 0.001f * fabs(b);

}

static DPVS_FORCE_INLINE bool isApproximatelyEqual(const Vector3& a, const Vector3& b)
{
	return	isApproximatelyEqual(a.x,b.x) &&
			isApproximatelyEqual(a.y,b.y) &&
			isApproximatelyEqual(a.z,b.z);
}

static DPVS_FORCE_INLINE bool isApproximatelyEqual(const Vector4& a, const Vector4& b)
{
	return	isApproximatelyEqual(a.x,b.x) &&
			isApproximatelyEqual(a.y,b.y) &&
			isApproximatelyEqual(a.z,b.z) &&
			isApproximatelyEqual(a.w,b.w);
}

#endif

const float PI = 3.14159265358979323846f;						// the constant pi

// On other platforms these routines are inline in dpvsBaseMath.hpp
#if defined (DPVS_EMULATED_DOUBLE)
void floatToDouble( double& d, const float& f )
{

	UINT32* di		= (UINT32*)(&d);
	UINT32  fi		= *(const UINT32*)(&f);

	UINT32  sign	 = fi & 0x80000000;					// copy sign
	fi &= 0x7FFFFFFF;									// remove sign
	int     exponent = (fi>>23)-127+1023;				// exponent
	UINT32  mantissa = fi & ((1<<23)-1);				// mantissa

	di[DPVS_SELECT_ENDIAN(1,0)] = sign | (exponent<<20) | (mantissa>>3);
	di[DPVS_SELECT_ENDIAN(0,1)] = (mantissa & 7)<<29;
}

void doubleToFloat( float& f, const double& d )
{

	const UINT32* id = (const UINT32*)(&d);

	UINT32 upper = DPVS_SELECT_ENDIAN(id[1],id[0]);	// upper dword
	UINT32 lower = DPVS_SELECT_ENDIAN(id[0],id[1]);	// lower dword

	UINT32 sign     = upper&0x80000000;					// copy sign bit
	upper &= 0x7FFFFFFF;								// remove sign bit

	int exponent = (upper>>20)-1023+127;				// convert exponent

	exponent = exponent & ~(exponent>>31);				// if exponent < -128 exponent = -128

	UINT32 mantissa = ((upper & ((1<<20)-1))<<3) | (lower>>29);	// 20 bits from upper, 3 bits from lower

	*((UINT32*)(&f)) = sign | (exponent<<23) | mantissa;
}
#endif // DPVS_EMULATED_DOUBLE

/******************************************************************************
 *
 * Function:		DPVS::Math::invertMatrix4x4()
 *
 * Description:		Inverts a 4x4 matrix
 *
 * Parameters:		m	= reference to destination matrix
 *					src	= reference to source matrix (may equal 'm')
 *
 * Notes:			This function is not as time-critical as the 4x3 variant.
 *
 ******************************************************************************/

void invertMatrix4x4 (Matrix4x4& m, const Matrix4x4& src)
{
	register const float a4 = src[3][0];
	register const float b4 = src[3][1];
	register const float c4 = src[3][2];
	register const float d4 = src[3][3];
	register const float a1 = src[0][0];
	register const float b1 = src[0][1];
	register const float c1 = src[0][2];
	register const float d1 = src[0][3];
	register const float a2 = src[1][0];
	register const float b2 = src[1][1];
	register const float c2 = src[1][2];
	register const float d2 = src[1][3];
	register const float a3 = src[2][0];
	register const float b3 = src[2][1];
	register const float c3 = src[2][2];
	register const float d3 = src[2][3];

	register float a3b4_a4b3 = a3 * b4 - a4 * b3;
	register float a3c4_a4c3 = a3 * c4 - a4 * c3;
	register float a3d4_a4d3 = a3 * d4 - a4 * d3;
	register float b3c4_b4c3 = b3 * c4 - b4 * c3;
	register float b3d4_b4d3 = b3 * d4 - b4 * d3;
	register float c3d4_c4d3 = c3 * d4 - c4 * d3;

	register float m00	= (b2 * c3d4_c4d3 - c2 * b3d4_b4d3 + d2 * b3c4_b4c3);
	register float m10	=-(a2 * c3d4_c4d3 - c2 * a3d4_a4d3 + d2 * a3c4_a4c3);
	register float m20	= (a2 * b3d4_b4d3 - b2 * a3d4_a4d3 + d2 * a3b4_a4b3);
	register float m30	=-(a2 * b3c4_b4c3 - b2 * a3c4_a4c3 + c2 * a3b4_a4b3);

	register float det	= Math::reciprocal(a1 * m00 + b1 * m10 + c1 * m20 + d1 * m30);

	m[0][0] = m00 * det;
	m[1][0] = m10 * det;
	m[2][0] = m20 * det;
	m[3][0] = m30 * det;

	m[0][1]	=-(b1 * c3d4_c4d3 - c1 * b3d4_b4d3 + d1 * b3c4_b4c3)*det;
	m[1][1]	= (a1 * c3d4_c4d3 - c1 * a3d4_a4d3 + d1 * a3c4_a4c3)*det;
	m[2][1]	=-(a1 * b3d4_b4d3 - b1 * a3d4_a4d3 + d1 * a3b4_a4b3)*det;
	m[3][1]	= (a1 * b3c4_b4c3 - b1 * a3c4_a4c3 + c1 * a3b4_a4b3)*det;

	register float a2d4_a4d2 = a2 * d4 - a4 * d2;
	register float a2b4_a4b2 = a2 * b4 - a4 * b2;
	register float a2c4_a4c2 = a2 * c4 - a4 * c2;
	register float b2c4_b4c2 = b2 * c4 - b4 * c2;
	register float b2d4_b4d2 = b2 * d4 - b4 * d2;
	register float c2d4_c4d2 = c2 * d4 - c4 * d2;

	m[0][2]	= (b1 * c2d4_c4d2 - c1 * b2d4_b4d2 + d1 * b2c4_b4c2)*det;
	m[1][2]	=-(a1 * c2d4_c4d2 - c1 * a2d4_a4d2 + d1 * a2c4_a4c2)*det;
	m[2][2]	= (a1 * b2d4_b4d2 - b1 * a2d4_a4d2 + d1 * a2b4_a4b2)*det;
	m[3][2]	=-(a1 * b2c4_b4c2 - b1 * a2c4_a4c2 + c1 * a2b4_a4b2)*det;

	register float a2b3_a3b2 = a2 * b3 - a3 * b2;
	register float a2c3_a3c2 = a2 * c3 - a3 * c2;
	register float a2d3_a3d2 = a2 * d3 - a3 * d2;
	register float b2c3_b3c2 = b2 * c3 - b3 * c2;
	register float b2d3_b3d2 = b2 * d3 - b3 * d2;
	register float c2d3_c3d2 = c2 * d3 - c3 * d2;

	m[0][3]	=-(b1 * c2d3_c3d2 - c1 * b2d3_b3d2 + d1 * b2c3_b3c2)*det;
	m[1][3]	= (a1 * c2d3_c3d2 - c1 * a2d3_a3d2 + d1 * a2c3_a3c2)*det;
	m[2][3]	=-(a1 * b2d3_b3d2 - b1 * a2d3_a3d2 + d1 * a2b3_a3b2)*det;
	m[3][3]	= (a1 * b2c3_b3c2 - b1 * a2c3_a3c2 + c1 * a2b3_a3b2)*det;
}

/******************************************************************************
 *
 * Function:		DPVS::Math::isFinite()
 *
 * Description:
 *
 * Parameters:
 *
 * Returns:
 *
 ******************************************************************************/

bool isFinite (const Vector3&   s)
{
	return isFinite(s.x) && isFinite(s.y) && isFinite(s.z);
}

/******************************************************************************
 *
 * Function:		DPVS::Math::isFinite()
 *
 * Description:
 *
 * Parameters:
 *
 * Returns:
 *
 ******************************************************************************/

bool isFinite (const Vector4&   s)
{
	return isFinite(s.x) && isFinite(s.y) && isFinite(s.z);
}

/******************************************************************************
 *
 * Function:		DPVS::Math::isFinite()
 *
 * Description:
 *
 * Parameters:
 *
 * Returns:
 *
 ******************************************************************************/

bool isFinite (const Matrix4x4& m)
{
	return isFinite(m[0]) && isFinite(m[1]) && isFinite(m[2]) && isFinite(m[3]);
}

/******************************************************************************
 *
 * Function:		DPVS::Math::isFinite()
 *
 * Description:
 *
 * Parameters:
 *
 * Returns:
 *
 ******************************************************************************/

bool isFinite (const Matrix4x3& m)
{
	return isFinite(m[0]) && isFinite(m[1]) && isFinite(m[2]);
}

/******************************************************************************
 *
 * Function:		DPVS::Math::intChop()
 *
 * Description:		Calculates (int)(float)
 *
 * Parameters:		f = floating point value
 *
 * Returns:			(int)(float)
 *
 * Notes:			This function is implemented in the header dpvsBaseMath.hpp
 *
 ******************************************************************************/

/******************************************************************************
 *
 * Function:		DPVS::Math::intFloor()
 *
 * Description:		Calculates (int)(floor(float))
 *
 * Parameters:		f = floating point value
 *
 * Returns:			(int)(floor(float))
 *
 ******************************************************************************/
/******************************************************************************
 *
 * Function:		DPVS::Math::intCeil()
 *
 * Description:		Calculates (int)(ceil(float))
 *
 * Parameters:		f = floating point value
 *
 * Returns:			(int)(ceil(float))
 *
 * Notes:			The basic idea is that we just calculate intFloor() with
 *					the original value negated, then negate the result.
 *
 ******************************************************************************/

#if !defined (DPVS_PS2_ASSEMBLY) && !defined (DPVS_GAMECUBE_ASSEMBLY)						// PS2 and GameCube versions already in the header file

int intFloor ( const float& f )
{
	INT32 a			= *reinterpret_cast<const INT32*>(&f);									// take bit pattern of float into a register
	INT32 sign		= (a>>31);																// sign = 0xFFFFFFFF if original value is negative, 0 if positive

	a&=0x7fffffff;																			// we don't need the sign any more

	INT32 exponent	= (a>>23)-127;															// extract the exponent
	INT32 expsign   = ~(exponent>>31);														// 0xFFFFFFFF if exponent is positive, 0 otherwise
	INT32 imask		= ( (1<<(31-(exponent))))-1;											// mask for true integer values
	INT32 mantissa	= (a&((1<<23)-1));														// extract mantissa (without the hidden bit)
	INT32 r			= ((UINT32)(mantissa|(1<<23))<<8)>>(31-exponent);						// ((1<<exponent)*(mantissa|hidden bit))>>24 -- (we know that mantissa > (1<<24))
	r = ((r & expsign) ^ (sign)) + ((!((mantissa<<8)&imask)&(expsign^((a-1)>>31)))&sign);	// if (fabs(value)<1.0) value = 0; copy sign; if (value < 0 && value==(int)(value)) value++;
	return r;
}

int intCeil (const float& f)
{
	INT32 a			= *reinterpret_cast<const INT32*>(&f) ^ 0x80000000;						// take bit pattern of float into a register
	INT32 sign		= (a>>31);																// sign = 0xFFFFFFFF if original value is negative, 0 if positive

	a&=0x7fffffff;																			// we don't need the sign any more

	INT32 exponent	= (a>>23)-127;															// extract the exponent
	INT32 expsign   = ~(exponent>>31);														// 0xFFFFFFFF if exponent is positive, 0 otherwise
	INT32 imask		= ( (1<<(31-(exponent))))-1;											// mask for true integer values
	INT32 mantissa	= (a&((1<<23)-1));														// extract mantissa (without the hidden bit)
	INT32 r			= ((UINT32)(mantissa|(1<<23))<<8)>>(31-exponent);						// ((1<<exponent)*(mantissa|hidden bit))>>24 -- (we know that mantissa > (1<<24))
	r = ((r & expsign) ^ (sign)) + ((!((mantissa<<8)&imask)&(expsign^((a-1)>>31)))&sign);	// if (fabs(value)<1.0) value = 0; copy sign; if (value < 0 && value==(int)(value)) value++;

	return -r;
}
#endif // !DPVS_PS2_ASSEMBLY && !defined (DPVS_GAMECUBE_ASSEMBLY)

/******************************************************************************
 *
 * Function:		DPVS::Math::isInvertible()
 *
 * Description:		Debug routine for checking whether we can invert a matrix or not
 *
 * Returns:			boolean indicating whether the matrix can be inverted
 *
 ******************************************************************************/

bool isInvertible (const Matrix4x3& src)
{
	// In debug build first check for NaNs etc (otherwise we would raise a
	// floating point exception)

#if defined (DPVS_DEBUG)
	if (!Math::isFinite(src))
		return false;
#endif

	register float a1 = src[0][0];
	register float b1 = src[0][1];
	register float c1 = src[0][2];
	register float a2 = src[1][0];
	register float b2 = src[1][1];
	register float c2 = src[1][2];
	register float a3 = src[2][0];
	register float b3 = src[2][1];
	register float c3 = src[2][2];

	register float b2c3_b3c2 = b2 * c3 - b3 * c2;
	register float a3c2_a2c3 = a3 * c2 - a2 * c3;
	register float a2b3_a3b2 = a2 * b3 - a3 * b2;

	return (a1 * (b2c3_b3c2) + b1 * (a3c2_a2c3) + c1 * (a2b3_a3b2)) != 0.0f;
}


/******************************************************************************
 *
 * Function:		DPVS::Math::getInverseTranslation()
 *
 * Description:		Inverts a 4x3 matrix and returns the translation of
 *					the inverted matrix
 *
 * Parameters:		src = reference to source matrix
 *
 * Returns:			vector containing translation of the inverse matrix
 *
 * Notes:			The routine performs internally only a partial inversion..
 *
 ******************************************************************************/


static DPVS_FORCE_INLINE Vector3 getInverseTranslationReal (const Matrix4x3& src)
{
	float a1 = src[0][0];
	float b1 = src[0][1];
	float c1 = src[0][2];
	float d1 = src[0][3];
	float a2 = src[1][0];
	float b2 = src[1][1];
	float c2 = src[1][2];
	float d2 = src[1][3];
	float a3 = src[2][0];
	float b3 = src[2][1];
	float c3 = src[2][2];
	float d3 = src[2][3];

	float b2c3_b3c2 = b2 * c3 - b3 * c2;
	float a3c2_a2c3 = a3 * c2 - a2 * c3;
	float a2b3_a3b2 = a2 * b3 - a3 * b2;

	float rDet = -Math::reciprocal(a1 * (b2c3_b3c2) + b1 * (a3c2_a2c3) + c1 * (a2b3_a3b2));

	float c1b3_b1c3 = c1 * b3 - b1 * c3;
	float b1c2_c1b2 = b1 * c2 - c1 * b2;
	float c1a2_a1c2 = c1 * a2 - a1 * c2;
	float a1c3_c1a3 = a1 * c3 - c1 * a3;
	float b1a3_a1b3 = b1 * a3 - a1 * b3;
	float a1b2_b1a2 = a1 * b2 - b1 * a2;

	return Vector3
	(
		(d1 * (b2c3_b3c2) + d2 * (c1b3_b1c3) + d3 * (b1c2_c1b2))*rDet,
		(d1 * (a3c2_a2c3) + d2 * (a1c3_c1a3) + d3 * (c1a2_a1c2))*rDet,
		(d1 * (a2b3_a3b2) + d2 * (b1a3_a1b3) + d3 * (a1b2_b1a2))*rDet
	);
}

Vector3 getInverseTranslation (const Matrix4x3& src)
{
#if defined (DPVS_PS2_VU_ASSEMBLY)
//PS2-BEGIN

	float DPVS_VECTOR_ALIGN(invtrans[4]);

	DPVS_ASSERT( is128Aligned(&src) );
	DPVS_ASSERT( is128Aligned(invtrans) );

	asm __volatile__ (

		/* src -> vf11,12,13 */
	"	lqc2		vf11, 0x0(%0)"
	"\n	lqc2		vf12, 0x10(%0)"
	"\n	lqc2		vf13, 0x20(%0)"

		/* x2y3_x3y2 = x2 * y3 - x3 * y2; */
	"\n	vmuly.x		vf1, vf12, vf13"
	"\n	vmuly.x		vf2, vf13, vf12"
	"\n	vsub.x		vf4, vf1, vf2"
		/* x2y3_x3y2 = vf4.x */

		/* x2z3_x3z2 = x2 * z3 - x3 * z2; */
	"\n	vmulz.x		vf1, vf12, vf13"
	"\n	vmulz.x		vf2, vf13, vf12"
	"\n	vsub.x		vf5, vf1, vf2"
		/* x2z3_x3z2 = vf5.x */

		/* x2w3_x3w2 = x2 * w3 - x3 * w2; */
	"\n	vmulw.x		vf1, vf12, vf13"
	"\n	vmulw.x		vf2, vf13, vf12"
	"\n	vsub.x		vf6, vf1, vf2"
		/* x2w3_x3w2 = vf6.x */

		/* y2z3_y3z2 = y2 * z3 - y3 * z2; */
	"\n	vmulz.y		vf1, vf12, vf13"
	"\n	vmulz.y		vf2, vf13, vf12"
	"\n	vsub.y		vf7, vf1, vf2"
		/*  y2z3_y3z2 = vf7.y */

		/* y2w3_y3w2 = y2 * w3 - y3 * w2; */
	"\n	vmulw.y		vf1, vf12, vf13"
	"\n	vmulw.y		vf2, vf13, vf12"
	"\n	vsub.y		vf8, vf1, vf2"
		/* y2w3_y3w2 = vf8.y */

		/* z2w3_z3w2 = z2 * w3 - z3 * w2; */
	"\n	vmulw.z		vf1, vf12, vf13"
	"\n	vmulw.z		vf2, vf13, vf12"
	"\n	vsub.z		vf9, vf1, vf2"
		/* z2w3_z3w2 = vf9.z*/

		/* m[0][0]	= (y2 * z3 - y3 * z2); */
	"\n	vmulz.y		vf1, vf12, vf13"
	"\n	vmulz.y		vf2, vf13, vf12"
	"\n	vsub.y		vf3, vf1, vf2"
	"\n	vaddy.x		vf21, vf0, vf3"
		/* m[0][0] = vf21.x */

		/* m[1][0]	= (x3 * z2 - x2 * z3); */
	"\n	vmulz.x		vf1, vf13, vf12"
	"\n	vmulz.x		vf2, vf12, vf13"
	"\n	vsub.x		vf22, vf1, vf2"
		/* m[1][0] = vf22.x */

		/* m[2][0]	= (x2 * y3 - x3 * y2); */
	"\n	vmuly.x		vf1, vf12, vf13"
	"\n	vmuly.x		vf2, vf13, vf12"
	"\n	vsub.x		vf23, vf1, vf2"
		/* m[2][0] = vf23.x */

		/* m[0][3]	= (z1 * y2w3_y3w2 - z2w3_z3w2 * y1 - w1 * y2z3_y3z2);
					= (vf11.z * vf8.y - vf9.z * vf11.y - vf11.w * vf7.y); */
	"\n	vmuly.z		vf1, vf11, vf8"
	"\n	vmuly.z		vf2, vf9, vf11"
	"\n	vsub.z		vf3, vf1, vf2"
	"\n	vmuly.w		vf1, vf11, vf7"
	"\n	vsubw.z		vf2, vf3, vf1"
	"\n	vsub.w		vf1, vf0, vf0"
	"\n	vaddz.w		vf21, vf1, vf2"
		/* m[0][3] = vf21.w */

		/* m[1][3]	= (x1 * z2w3_z3w2 - x2w3_x3w2 * z1 + x2z3_x3z2 * w1);
					= (vf11.x * vf9.z - vf6.x * vf11.z + vf5.x * vf11.w); */
	"\n	vmulz.x		vf1, vf11, vf9"
	"\n	vmulz.x		vf2, vf6, vf11"
	"\n	vsub.x		vf3, vf1, vf2"
	"\n	vmulw.x		vf1, vf5, vf11"
	"\n	vadd.x		vf2, vf1, vf3"
	"\n	vsub.w		vf1, vf0, vf0"
	"\n	vaddx.w		vf22, vf1, vf2"
		/* m[1][3] = vf22.w */

		/* m[2][3]	= (y1 * x2w3_x3w2 - y2w3_y3w2 * x1 - x2y3_x3y2 * w1);
					= (vf11.y * vf6.x - vf8.y * vf11.x - vf4.x * vf11.w); */
	"\n	vmulx.y		vf1, vf11, vf6"
	"\n	vmulx.y		vf2, vf8, vf11"
	"\n	vsub.y		vf3, vf1, vf2"
	"\n	vmulw.x		vf1, vf4, vf11"
	"\n	vsubx.y		vf2, vf3, vf1"
	"\n	vsub.w		vf1, vf0, vf0"
	"\n	vaddy.w		vf23, vf1, vf2"
		/* m[2][3] = vf23.w */

		/*
		m[0][0] = vf21.x
		m[1][0] = vf22.x
		m[2][0] = vf23.x
		m[0][3] = vf21.w
		m[1][3] = vf22.w
		m[2][3] = vf23.w
		*/

		/* det		= x1 * m[0][0]		+ m[1][0] * y1		+ m[2][0] * z1;
					= vf11.x * vf21.x	+ vf22.x * vf11.y	+ vf23.x * vf11.z; */
	"\n	vmul.x		vf1, vf11, vf21"
	"\n	vmuly.x		vf2, vf22, vf11"
	"\n	vadd.x		vf3, vf1, vf2"
	"\n	vmulz.x		vf1, vf23, vf11"
	"\n	vadd.x		vf2, vf3, vf1"
		/* det = vf2.x */

		/* invdet = 1.f / det; */
	"\n	vdiv		Q, vf0w, vf2x"
		/* invdet = Q */

		/* invdet -> {invdet, invdet, invdet, invdet} */
	"\n	vsub		vf1, vf0, vf0"
	"\n	vwaitq"
	"\n	vaddq		vf2, vf1, Q"
		/* {invdet, invdet, invdet, invdet} = vf2 */

		/* for ( int j = 0 ; j < 3 ; ++j ) m[j] *= invdet; */
	"\n	vmulw.x		vf1, vf2, vf21"
	"\n	vmulw.y		vf1, vf2, vf22"
	"\n	vmulw.z		vf1, vf2, vf23"

		/*
		m[0][3] = vf1.x
		m[1][3] = vf1.y
		m[2][3] = vf1.z
		*/

		/* vf11,12,13 -> invtrans */
	"\n	sqc2		vf1, 0x0(%1)"

		: : "r"(&src), "r"(invtrans));

	// validate results
	#if defined (DPVS_DEBUG)
		Vector3 res( invtrans[0], invtrans[1], invtrans[2] );
		return res;
		DPVS_ASSERT(isApproximatelyEqual(res,getInverseTranslationReal(src)));
	#else
		return Vector3( invtrans[0], invtrans[1], invtrans[2] );
	#endif // DPVS_DEBUG
//PS2-END
#else
	return getInverseTranslationReal(src);
#endif
}

/******************************************************************************
 *
 * Function:		DPVS::Math::getInverseTranslation()
 *
 * Description:		Calculates inverse translation of (a*b)
 *
 * Parameters:		m = reference to first matrix
 *					n = reference to second matrix
 *
 * Returns:			inverse translation vector
 *
 * Notes:			This is the optimized version for:
 *					Matrix4x3 d = m;
 *					d.productFromLeft(n);
 *					return Math::getInverseTranslation(d);
 *
 *					The routine is used for transforming cameras into
 *					the local space of an OBB (i.e. OBB matrix is multiplied
 *					together with ObjectToCamera matrix).
 *
 ******************************************************************************/

Vector3	getInverseTranslation (const Matrix4x3& m, const Matrix4x3& n)
{
#if defined (DPVS_PS2_VU_ASSEMBLY)
//PS2-BEGIN
	float DPVS_VECTOR_ALIGN(invtrans[4]);

	DPVS_ASSERT( is128Aligned(&n) );
	DPVS_ASSERT( is128Aligned(&m) );
	DPVS_ASSERT( is128Aligned(invtrans) );

	asm __volatile__ (

	"   lqc2			vf11, 0x0(%0)"
	"\n	lqc2			vf12, 0x10(%0)"
	"\n	lqc2			vf13, 0x20(%0)"
	"\n	lqc2			vf15, 0x0(%1)"
	"\n	lqc2			vf16, 0x10(%1)"
	"\n	lqc2			vf17, 0x20(%1)"
	"\n	vmulax.xyzw		ACC, vf11, vf15"
	"\n	vmadday.xyzw	ACC, vf12, vf15"
	"\n	vmaddaz.xyzw	ACC, vf13, vf15"
	"\n	vmaddw.xyzw		vf21, vf0, vf15"
	"\n	vmulax.xyzw		ACC, vf11, vf16"
	"\n	vmadday.xyzw	ACC, vf12, vf16"
	"\n	vmaddaz.xyzw	ACC, vf13, vf16"
	"\n	vmaddw.xyzw		vf22, vf0, vf16"
	"\n	vmulax.xyzw		ACC, vf11, vf17"
	"\n	vmadday.xyzw	ACC, vf12, vf17"
	"\n	vmaddaz.xyzw	ACC, vf13, vf17"
	"\n	vmaddw.xyzw		vf13, vf0, vf17"
	"\n	vmove.xyzw		vf12, vf22"
	"\n	vmove.xyzw		vf11, vf21"
	"\n	vmuly.x		vf1, vf12, vf13"
	"\n	vmuly.x		vf2, vf13, vf12"
	"\n	vsub.x		vf4, vf1, vf2"
	"\n	vmulz.x		vf1, vf12, vf13"
	"\n	vmulz.x		vf2, vf13, vf12"
	"\n	vsub.x		vf5, vf1, vf2"
	"\n	vmulw.x		vf1, vf12, vf13"
	"\n	vmulw.x		vf2, vf13, vf12"
	"\n	vsub.x		vf6, vf1, vf2"
	"\n	vmulz.y		vf1, vf12, vf13"
	"\n	vmulz.y		vf2, vf13, vf12"
	"\n	vsub.y		vf7, vf1, vf2"
	"\n	vmulw.y		vf1, vf12, vf13"
	"\n	vmulw.y		vf2, vf13, vf12"
	"\n	vsub.y		vf8, vf1, vf2"
	"\n	vmulw.z		vf1, vf12, vf13"
	"\n	vmulw.z		vf2, vf13, vf12"
	"\n	vsub.z		vf9, vf1, vf2"
	"\n	vmulz.y		vf1, vf12, vf13"
	"\n	vmulz.y		vf2, vf13, vf12"
	"\n	vsub.y		vf3, vf1, vf2"
	"\n	vaddy.x		vf21, vf0, vf3"
	"\n	vmulz.x		vf1, vf13, vf12"
	"\n	vmulz.x		vf2, vf12, vf13"
	"\n	vsub.x		vf22, vf1, vf2"
	"\n	vmuly.x		vf1, vf12, vf13"
	"\n	vmuly.x		vf2, vf13, vf12"
	"\n	vsub.x		vf23, vf1, vf2"
	"\n	vmuly.z		vf1, vf11, vf8"
	"\n	vmuly.z		vf2, vf9, vf11"
	"\n	vsub.z		vf3, vf1, vf2"
	"\n	vmuly.w		vf1, vf11, vf7"
	"\n	vsubw.z		vf2, vf3, vf1"
	"\n	vsub.w		vf1, vf0, vf0"
	"\n	vaddz.w		vf21, vf1, vf2"
	"\n	vmulz.x		vf1, vf11, vf9"
	"\n	vmulz.x		vf2, vf6, vf11"
	"\n	vsub.x		vf3, vf1, vf2"
	"\n	vmulw.x		vf1, vf5, vf11"
	"\n	vadd.x		vf2, vf1, vf3"
	"\n	vsub.w		vf1, vf0, vf0"
	"\n	vaddx.w		vf22, vf1, vf2"
	"\n	vmulx.y		vf1, vf11, vf6"
	"\n	vmulx.y		vf2, vf8, vf11"
	"\n	vsub.y		vf3, vf1, vf2"
	"\n	vmulw.x		vf1, vf4, vf11"
	"\n	vsubx.y		vf2, vf3, vf1"
	"\n	vsub.w		vf1, vf0, vf0"
	"\n	vaddy.w		vf23, vf1, vf2"
	"\n	vmul.x		vf1, vf11, vf21"
	"\n	vmuly.x		vf2, vf22, vf11"
	"\n	vadd.x		vf3, vf1, vf2"
	"\n	vmulz.x		vf1, vf23, vf11"
	"\n	vadd.x		vf2, vf3, vf1"
	"\n	vdiv		Q, vf0w, vf2x"
	"\n	vsub		vf1, vf0, vf0"
	"\n	vwaitq"
	"\n	vaddq		vf2, vf1, Q"
	"\n	vmulw.x		vf1, vf2, vf21"
	"\n	vmulw.y		vf1, vf2, vf22"
	"\n	vmulw.z		vf1, vf2, vf23"
	"\n	sqc2		vf1, 0x0(%2)"
	 : : "r"(&m), "r"(&n), "r"(invtrans) );

	Vector3 res( invtrans[0], invtrans[1], invtrans[2] );
	return res;
//PS2-END
#else
	float a1 = n[0][0]*m[0][0] + n[0][1]*m[1][0] + n[0][2]*m[2][0];
	float b1 = n[0][0]*m[0][1] + n[0][1]*m[1][1] + n[0][2]*m[2][1];
	float c1 = n[0][0]*m[0][2] + n[0][1]*m[1][2] + n[0][2]*m[2][2];
	float a2 = n[1][0]*m[0][0] + n[1][1]*m[1][0] + n[1][2]*m[2][0];
	float b2 = n[1][0]*m[0][1] + n[1][1]*m[1][1] + n[1][2]*m[2][1];
	float c2 = n[1][0]*m[0][2] + n[1][1]*m[1][2] + n[1][2]*m[2][2];
	float a3 = n[2][0]*m[0][0] + n[2][1]*m[1][0] + n[2][2]*m[2][0];
	float b3 = n[2][0]*m[0][1] + n[2][1]*m[1][1] + n[2][2]*m[2][1];
	float c3 = n[2][0]*m[0][2] + n[2][1]*m[1][2] + n[2][2]*m[2][2];
	float d1 = n[0][0]*m[0][3] + n[0][1]*m[1][3] + n[0][2]*m[2][3] + n[0][3];
	float d2 = n[1][0]*m[0][3] + n[1][1]*m[1][3] + n[1][2]*m[2][3] + n[1][3];
	float d3 = n[2][0]*m[0][3] + n[2][1]*m[1][3] + n[2][2]*m[2][3] + n[2][3];

	float b2c3_b3c2 = b2 * c3 - b3 * c2;
	float a3c2_a2c3 = a3 * c2 - a2 * c3;
	float a2b3_a3b2 = a2 * b3 - a3 * b2;

	float rDet = -Math::reciprocal(a1 * (b2c3_b3c2) + b1 * (a3c2_a2c3) + c1 * (a2b3_a3b2));

	float c1b3_b1c3 = c1 * b3 - b1 * c3;
	float b1c2_c1b2 = b1 * c2 - c1 * b2;
	float c1a2_a1c2 = c1 * a2 - a1 * c2;
	float a1c3_c1a3 = a1 * c3 - c1 * a3;
	float b1a3_a1b3 = b1 * a3 - a1 * b3;
	float a1b2_b1a2 = a1 * b2 - b1 * a2;

	return Vector3
	(
		(d1 * (b2c3_b3c2) + d2 * (c1b3_b1c3) + d3 * (b1c2_c1b2))*rDet,
		(d1 * (a3c2_a2c3) + d2 * (a1c3_c1a3) + d3 * (c1a2_a1c2))*rDet,
		(d1 * (a2b3_a3b2) + d2 * (b1a3_a1b3) + d3 * (a1b2_b1a2))*rDet
	);
#endif
}

/******************************************************************************
 *
 * Function:		DPVS::Math::dot()
 *
 * Description:		Computes dot products of array 'src' against vector 'p' and
 *					stores results into float array 'dst'
 *
 * Parameters:		dst = destination float array (results of the dots)
 *					src = source vector4 array
 *					p	= source vector4
 *					N	= number of vectors (N >= 0)
 *
 * Notes:			We expect that the input array 'src' is _not_ in L1 cache,
 *					because this routine is often called for large arrays of
 *					plane equations.
 *
 ******************************************************************************/

void dot (float* dst, const Vector4* src, const Vector4& p, int N)
{
	DPVS_ASSERT (dst && src && N >= 0);

#if defined (DPVS_PS2_ASSEMBLY)
//PS2-BEGIN

	if ( N < 65536 )
	{
		asm __volatile__ (
		"	add			$8, %3, $0"
		"\n andi		$8, $8, 0xFFFC"
		"\n sub			%3, %3, $8"
		"\n lwc1		$f10, 0(%2)"
		"\n lwc1		$f11, 4(%2)"
		"\n lwc1		$f12, 8(%2)"
		"\n lwc1		$f13, 12(%2)"
			/*	$f10-$f13	= p
				%3			= count & 3
				$8			= count & ~3
				%0			= dst
				%1			= src*/
"\n dot_loop4:"
		"\n beq			$8, $0, dot_padding"
		"\n nop"

		"\n lwc1		$f4, 0x00(%1)"
		"\n lwc1		$f5, 0x04(%1)"
		"\n lwc1		$f6, 0x08(%1)"
		"\n lwc1		$f7, 0x0C(%1)"
		"\n mula.s		$f4, $f10"
		"\n madda.s		$f5, $f11"
		"\n madda.s		$f6, $f12"
		"\n madd.s		$f1, $f7, $f13"
		"\n swc1		$f1, 0(%0)"

		"\n lwc1		$f4, 0x10(%1)"
		"\n lwc1		$f5, 0x14(%1)"
		"\n lwc1		$f6, 0x18(%1)"
		"\n lwc1		$f7, 0x1C(%1)"
		"\n mula.s		$f4, $f10"
		"\n madda.s		$f5, $f11"
		"\n madda.s		$f6, $f12"
		"\n madd.s		$f1, $f7, $f13"
		"\n swc1		$f1, 4(%0)"

		"\n lwc1		$f4, 0x20(%1)"
		"\n lwc1		$f5, 0x24(%1)"
		"\n lwc1		$f6, 0x28(%1)"
		"\n lwc1		$f7, 0x2C(%1)"
		"\n mula.s		$f4, $f10"
		"\n madda.s		$f5, $f11"
		"\n madda.s		$f6, $f12"
		"\n madd.s		$f1, $f7, $f13"
		"\n swc1		$f1, 8(%0)"

		"\n lwc1		$f4, 0x30(%1)"
		"\n lwc1		$f5, 0x34(%1)"
		"\n lwc1		$f6, 0x38(%1)"
		"\n lwc1		$f7, 0x3C(%1)"
		"\n mula.s		$f4, $f10"
		"\n madda.s		$f5, $f11"
		"\n madda.s		$f6, $f12"
		"\n madd.s		$f1, $f7, $f13"
		"\n swc1		$f1, 12(%0)"

		"\n addi		$8, $8, -4"
		"\n addiu		%0, %0, 16"
		"\n addiu		%1, %1, 64"
		"\n j			dot_loop4"
		"\n nop"
"\n dot_padding:"
		"\n beq			%3, $0, dot_done"
		"\n nop"

		"\n lwc1		$f4, 0x00(%1)"
		"\n lwc1		$f5, 0x04(%1)"
		"\n lwc1		$f6, 0x08(%1)"
		"\n lwc1		$f7, 0x0C(%1)"
		"\n mula.s		$f4, $f10"
		"\n madda.s		$f5, $f11"
		"\n madda.s		$f6, $f12"
		"\n madd.s		$f1, $f7, $f13"
		"\n swc1		$f1, 0(%0)"

		"\n addi		%3, %3, -1"
		"\n addiu		%0, %0, 4"
		"\n addiu		%1, %1, 16"
		"\n j			dot_loop4"
"\n dot_done:"
		 : : "r"(dst), "r"(src), "r"(&p), "r"(N) : "$8" );

	// validate results
	#if defined (DPVS_DEBUG)
		register float px = p.x, py = p.y, pz = p.z, pw = p.w;
		for (int i = 0; i < N; i++)
			DPVS_ASSERT(isApproximatelyEqual(dst[i], src[i].x * px + src[i].y * py + src[i].z * pz + src[i].w * pw));
	#endif

		return;
	}

	// FALLTHRU!
//PS2-END
#endif

#if defined (DPVS_X86_ASSEMBLY)
	//--------------------------------------------------------------------
	// The routine is unrolled by two. It performs pre-fetching of source
	// data to maximize performance on large arrays (i.e. when source
	// data is not in L1 cache)
	//--------------------------------------------------------------------

	int cnt = (N > 5) ? ((N-5)&~1) : 0;

	__asm
	{
		mov		eax, p
		fld		dword ptr[eax+0]
		fld		dword ptr[eax+4]
		fld		dword ptr[eax+8]
		fld		dword ptr[eax+12]
		push	edi
		push	esi
		mov		ecx,cnt
		cmp		ecx,0
		jle		eol
		mov		edi,dst
		mov		esi,src
		shl		ecx,2
		lea		esi,[esi+ecx*4]
		add		edi,ecx
		neg		ecx
		mov		ebx,[esi+ecx*4+32]					// prefetch
		jmp		lps
		align	16

		lps:
			fld		dword ptr[esi+ecx*4]
			fmul	st,st(4)
			fld		dword ptr[esi+ecx*4+4]
			fmul	st,st(4)
			fld		dword ptr[esi+ecx*4+8]
			fmul	st,st(4)
			fld		dword ptr[esi+ecx*4+12]
			fmul	st,st(4)				// W Z Y X
			fxch	st(3)					// X Z Y W
			faddp	st(2),st				// Z X+Y W
			faddp	st(2),st				// X+Y Z+W
			faddp	st(1),st
			mov		eax,[esi+ecx*4+64]		// prefetch
			fstp	dword ptr[edi+ecx]
			fld		dword ptr[esi+ecx*4 + 16]
			fmul	st,st(4)
			fld		dword ptr[esi+ecx*4+4 + 16]
			fmul	st,st(4)
			fld		dword ptr[esi+ecx*4+8 + 16]
			fmul	st,st(4)
			fld		dword ptr[esi+ecx*4+12 + 16]
			fmul	st,st(4)				// W Z Y X
			fxch	st(3)					// X Z Y W
			faddp	st(2),st				// Z X+Y W
			faddp	st(2),st				// X+Y Z+W
			faddp	st(1),st
			fstp	dword ptr[edi+ecx + 4]
			add		ecx,8
			js		lps
eol:
		mov		ecx,N
		sub		ecx,cnt		// ecx = n-cnt
		jz		skip
		mov		edx,cnt
		shl		edx,2
		mov		edi,dst
		mov		esi,src
		lea		esi,[esi+edx*4]
		add		edi,edx
		shl		ecx,2
		lea		esi,[esi+ecx*4]
		add		edi,ecx
		neg		ecx
lp2:
		fld		dword ptr[esi+ecx*4]
		fmul	st,st(4)
		fld		dword ptr[esi+ecx*4+4]
		fmul	st,st(4)
		fld		dword ptr[esi+ecx*4+8]
		fmul	st,st(4)
		fld		dword ptr[esi+ecx*4+12]
		fmul	st,st(4)				// W Z Y X
		fxch	st(3)					// X Z Y W
		faddp	st(2),st				// Z X+Y W
		faddp	st(2),st				// X+Y Z+W
		faddp	st(1),st
		fstp	dword ptr[edi+ecx]
		add		ecx,4
		js		lp2
skip:
		pop		esi
		pop		edi
		fcompp
		fcompp
	}
#else // DPVS_X86_ASSEMBLY
	register float px = p.x, py = p.y, pz = p.z, pw = p.w;
	for (int i = 0; i < N; i++)
		dst[i] = src[i].x * px + src[i].y * py + src[i].z * pz + src[i].w * pw;
#endif
}

/******************************************************************************
 *
 * Function:		DPVS::Math::getNormalizedPlaneEquation()
 *
 * Description:		Calculates normalized plane equation of triangle
 *					formed by vertices a,b,c
 *
 * Parameters:		a = first  vertex
 *					b = second vertex
 *					c = third  vertex
 *
 * Returns:			normalized plane equation
 *
 ******************************************************************************/

Vector4 getNormalizedPlaneEquation (const Vector3& a, const Vector3& b, const Vector3& c)
{
 	float	x1 = b.x - a.x;
	float	y1 = b.y - a.y;
	float	z1 = b.z - a.z;
	float	x2 = c.x - a.x;
	float	y2 = c.y - a.y;
	float	z2 = c.z - a.z;
	float	nx = (y1*z2)-(z1*y2);
	float	ny = (z1*x2)-(x1*z2);
	float	nz = (x1*y2)-(y1*x2);

	float	d = nx*nx+ny*ny+nz*nz;

	if ( d < FLT_MIN)								// can't take reciprocal sqrt
		return Vector4(0.0f,0.0f,0.0f,-0.0f);

	d = Math::reciprocalSqrt(d);

	nx *= d;
	ny *= d;
	nz *= d;

	return Vector4(nx,ny,nz,-(a.x*nx+a.y*ny+a.z*nz));
}


/******************************************************************************
 *
 * Function:		DPVS::Math::isUniformRotationMatrix()
 *
 * Description:		Determines where matrix is a uniform-scaled rotation matrix
 *
 * Parameters:		m = reference to input 4x3 matrix
 *
 * Returns:			boolean value indicating whether is a uniform-scaled rotation matrix
 *
 ******************************************************************************/

bool isUniformRotationMatrix (const Matrix4x3& m)
{
	float x = (m[0][0]*m[0][0] + m[1][0]*m[1][0] + m[2][0]*m[2][0]);
	float y = (m[0][1]*m[0][1] + m[1][1]*m[1][1] + m[2][1]*m[2][1]);

	if (Math::fabs(x-y) <= (x*0.001f))
	{
		float z = (m[0][2]*m[0][2] + m[1][2]*m[1][2] + m[2][2]*m[2][2]);

		if (Math::fabs(x-z) <= (x*0.001f))
			return true;
	}
	return false;
}

/******************************************************************************
 *
 * Function:		DPVS::Math::isUniformRotationMatrix()
 *
 * Description:		Determines where matrix is a uniform-scaled rotation matrix
 *
 * Parameters:		m = reference to input 4x4 matrix
 *
 * Returns:			boolean value indicating whether matrix is uniformly scaled
 *
 ******************************************************************************/

bool isUniformRotationMatrix (const Matrix4x4& m)
{
	if (!isZero(m[3][0]) || !isZero(m[3][1]) || !isZero(m[3][2]) || m[3][3] != 1.0f)	// contains projective terms
		return false;
	return isUniformRotationMatrix((const Matrix4x3&)m);
}

/******************************************************************************
 *
 * Function:		DPVS::Math::transformPlanes()
 *
 * Description:		Transforms clip planes with specified matrix
 *
 * Parameters:		dst	 = destination array of clip planes
 *					src  = source array of clip planes
 *					mtx	 = transformation matrix
 *					N    = number of clip planes
 *
 * Notes:			DO NOT CODE THIS IN SIMD (ACCURACY CONSIDERATIONS!!)
 *
 *					This function can properly transform plane equations that
 *					have _not_ been normalized. It also handles extremely
 *					degenerate matrices (although they must be invertible!)
 *
 ******************************************************************************/

void transformPlanes (Vector4* dst, const Vector4* src, const Matrix4x3& m, int N)
{
	DPVS_ASSERT (dst && src && N>=0);
	Matrix4x3	n(NO_CONSTRUCTOR);
	Math::invertMatrix(n,m);				// invert the 4x3 matrix
	transformPlanesWithInverseMatrix(dst,src,n,N);
}

/******************************************************************************
 *
 * Function:		DPVS::Math::intFloor()
 *
 * Description:		Converts an array of floating point values into integers
 *
 * Parameters:		dst = array of 32-bit integers
 *					src = source floating point array
 *					N	= number of floats to convert
 *
 * Notes:			The float->int conversion is performed by first flooring
 *					the floating point value
 *
 ******************************************************************************/

void intFloor (INT32* dst, const float* src, int N)
{
	DPVS_ASSERT( dst && src && N > 0);

#if defined (DPVS_PS2_VU_ASSEMBLY)
//PS2-BEGIN

	if ( is128Aligned(dst) && is128Aligned(src) && N < 65536)
	{
		static const INT32					almostOneInt32	= 0x3f800000-1;
		static const float					almostOne		= *reinterpret_cast<const float*>( &almostOneInt32 );
		static const float DPVS_VECTOR_ALIGN(almostOne4[4]) = { almostOne, almostOne, almostOne, almostOne };

		INT32	convBuff4[4];

		DPVS_ASSERT( is128Aligned(src) );
		DPVS_ASSERT( is128Aligned(dst) );
		DPVS_ASSERT( is128Aligned(almostOne4) );
		DPVS_ASSERT( is128Aligned(convBuff4) );

		asm __volatile__ (

		"\n	lqc2		vf10, 0(%3)"
		"\n	andi		%2, %2, 0xFFFF"
		"\n	andi		$8, %2, 0xFFFC"
		"\n	andi		%2, %2, 3"
"\n	intFloor_loop4:"
		"\n	beq			$8, $0, intFloor_padding"
		"\n	nop"

		"\n	lqc2		vf1, 0(%1)"
			/* int	c = chop(f); */
		"\n	vftoi0.xyzw	vf2, vf1"
		"\n	sqc2		vf2, 0(%0)"
			/* float cf = c; */
		"\n	vitof0.xyzw	vf3, vf2"
			/* float xf = cf-f+almostOne; */
		"\n	vsub.xyzw	vf4, vf3, vf1"
		"\n	vadd.xyzw	vf5, vf4, vf10"
			/* int x = xf; */
		"\n	vftoi0.xyzw	vf6, vf5"
			/* return	c - x;*/
		"\n	sqc2		vf6, 0(%4)"
		"\n	lq			$9, 0(%0)"
		"\n	lq			$10, 0(%4)"
		"\n	psubw		$9, $9, $10"
		"\n	sq			$9, 0(%0)"

		"\n	addiu		%0, %0, 16"
		"\n	addiu		%1, %1, 16"
		"\n	addi		$8, $8, -4"
		"\n	j			intFloor_loop4"
		"\n	nop"
"\n	intFloor_padding:"
		"\n	beq			%2, $0, intFloor_end"
		"\n	nop"

		"\n	lw			$9, 0(%1)"
		"\n	qmtc2		$9, vf1"
			/* int	c = chop(f); */
		"\n	vftoi0.x	vf2, vf1"
		"\n	qmfc2		$10, vf2"
			/* float cf = c; */
		"\n	vitof0.x	vf3, vf2"
			/* float xf = cf-f+almostOne; */
		"\n	vsub.x		vf4, vf3, vf1"
		"\n	vadd.x		vf5, vf4, vf10"
			/* int x = xf; */
		"\n	vftoi0.x	vf6, vf5"
			/* return	c - x;*/
		"\n	qmfc2		$9,  vf6"
		"\n	sub			$10, $10, $9"
		"\n	sw			$10, 0(%0)"

		"\n	addiu		%0, %0, 4"
		"\n	addiu		%1, %1, 4"
		"\n	addi		%2, %2, -1"
		"\n	j			intFloor_padding"
		"\n	nop"
"\n	intFloor_end:"

		: : "r"(dst), "r"(src), "r"(N), "r"(almostOne4), "r"(convBuff4) : "$8", "$9", "$10" );

		#if defined (DPVS_DEBUG)
			for (int i = 0; i < N; i++)
				DPVS_ASSERT(dst[i]==Math::intFloor(src[i]));
		#endif

		return;
	}
	// FALLTHRU!
//PS2-END
#endif

#if defined (DPVS_X86_ASSEMBLY)

	//------------------------------------------------------------------------
	// This routine is ripped of from SurRender x86 VP. The loop is unrolled
	// four times and the conversion is handled by fistp (the rounding
	// mode is initially changed).
	//------------------------------------------------------------------------

	X86::FPUControl foo(X86::FPUControl::ROUND_DOWN);	// change rounding mode to near-infinity (floor)

	DPVS_ASM
	{
		mov		ecx,N
		mov		eax,src
		mov		ebx,dst
		and		ecx,~3
		jz		padding
		shr		ecx,2
		dec		ecx
		shl		ecx,4
		jmp		loopie
		align 16
loopie:
		fld		dword ptr [eax + ecx+0]
		fld		dword ptr [eax + ecx+4]
		fld		dword ptr [eax + ecx+8]
		fld		dword ptr [eax + ecx+12]
		fxch	st(3)							// save one cycle
		fistp	dword ptr [ebx + ecx+0]
		fistp	dword ptr [ebx + ecx+8]
		fistp	dword ptr [ebx + ecx+4]
		fistp	dword ptr [ebx + ecx+12]
		sub		ecx,16
		jge		loopie
padding:
		mov		ecx,N
		mov		edx,ecx
		and		edx,~3						// edx = count&~3
		and		ecx,3						// ecx = count&3
		jz		outta
		lea		eax,[eax + edx*4]
		lea		ebx,[ebx + edx*4]
		cmp		ecx,2
		jl		skip2
		fld		dword ptr [eax]
		fld		dword ptr [eax+4]
		fxch	st(1)
		sub		ecx,2
		add		eax,8
		fistp	dword ptr [ebx]
		fistp	dword ptr [ebx+4]
		add		ebx,8
skip2:
		cmp		ecx,1
		jl		outta
		fld		dword ptr [eax]
		fistp	dword ptr [ebx]
outta:
	}

	return;

#else
	for (int i = 0; i < N; i++)
		dst[i] = Math::intFloor(src[i]);
#endif // DPVS_X86_ASSEMBLY
}

/******************************************************************************
 *
 * Function:		DPVS::Math::rasterToFixed()
 *
 * Description:		Converts raster-space coordinates into fixed point
 *
 * Parameters:		dst		= destination fixed point array
 *					src		= source raster-space coordinates
 *					scale	= scale factor prior to int-casting
 *					N		= number of coordinates
 *
 * Notes:			the formula is dst[i].x = (int)(floor(src[i].x*scale.x))) etc.
 *
 ******************************************************************************/

void rasterToFixed (Vector2i* dst, const Vector2* src, const Vector2& scale, int N)
{
	DPVS_ASSERT( dst && src && N > 0);

#if defined (DPVS_PS2_VU_ASSEMBLY)
//PS2-BEGIN

	if ( is128Aligned(dst) && is128Aligned(src) && N < 65536)
	{
		static const INT32						almostOneInt32	= 0x3f800000-1;
		static const float						almostOne		= *reinterpret_cast<const float*>( &almostOneInt32 );
		static const float DPVS_VECTOR_ALIGN(almostOne4[4])  = { almostOne, almostOne, almostOne, almostOne };

		INT32						convBuff4[4];
		float DPVS_VECTOR_ALIGN(scale4[4]);

		DPVS_ASSERT( is128Aligned(src) );
		DPVS_ASSERT( is128Aligned(dst) );
		DPVS_ASSERT( is128Aligned(almostOne4) );
		DPVS_ASSERT( is128Aligned(convBuff4) );

		asm __volatile__ (
		"	lw			$8, 0(%6)"
		"\n lw			$9, 4(%6)"
		"\n sw			$8, 0(%5)"
		"\n sw			$9, 4(%5)"
		"\n sw			$8, 8(%5)"
		"\n sw			$9, 12(%5)"

		"\n lqc2		vf10, 0(%3)"
		"\n lqc2		vf11, 0(%5)"
		"\n andi		%2, %2, 0xFFFF"
		"\n andi		$8, %2, 0xFFFE"
		"\n andi		%2, %2, 1"
			/*
			dst				= %0
			src				= %1
			count & 1		= %2
			count & ~1		= $8
			&convBuff4[0]	= %4
			almostOne4		= vf10
			scale4			= vf11
			*/
"\n rasterToFixed_loop2:"
		"\n beq			$8, $0, rasterToFixed_padding"
		"\n nop"

		"\n lqc2		vf2, 0(%1)"
		"\n vmul.xyzw	vf1, vf2, vf11"
			/* int	c = chop(f); */
		"\n vftoi0.xyzw	vf2, vf1"
		"\n sqc2		vf2, 0(%0)"
			/* float cf = c; */
		"\n vitof0.xyzw	vf3, vf2"
			/* float xf = cf-f+almostOne; */
		"\n vsub.xyzw	vf4, vf3, vf1"
		"\n vadd.xyzw	vf5, vf4, vf10"
			/* int x = xf; */
		"\n vftoi0.xyzw	vf6, vf5"
			/* return	c - x;*/
		"\n sqc2		vf6, 0(%4)"
		"\n lq			$9, 0(%0)"
		"\n lq			$10, 0(%4)"
		"\n psubw		$9, $9, $10"
		"\n sq			$9, 0(%0)"

		"\n addiu		%0, %0, 16"
		"\n addiu		%1, %1, 16"
		"\n addi		$8, $8, -2"
		"\n j			rasterToFixed_loop2"
		"\n nop"
"\n rasterToFixed_padding:"
		"\n beq			%2, $0, rasterToFixed_end"
		"\n nop"

		"\n ld			$9, 0(%1)"
		"\n sd			$9, 0(%4)"
		"\n lqc2		vf2, 0(%4)"
		"\n vmul.xy		vf1, vf2, vf11"
			/* int	c = chop(f); */
		"\n vftoi0.xy	vf2, vf1"
		"\n sqc2		vf2, 0(%4)"
		"\n lw			$10, 0(%4)"
		"\n lw			$11, 4(%4)"
			/* float cf = c; */
		"\n vitof0.xy	vf3, vf2"
			/* float xf = cf-f+almostOne; */
		"\n vsub.xy		vf4, vf3, vf1"
		"\n vadd.xy		vf5, vf4, vf10"
			/* int x = xf; */
		"\n vftoi0.xy	vf6, vf5"
			/* return	c - x;*/
		"\n sqc2		vf6, 0(%4)"
		"\n lw			$8, 0(%4)"
		"\n lw			$9, 4(%4)"
		"\n sub			$10, $10, $8"
		"\n sw			$10, 0(%0)"
		"\n sub			$11, $11, $9"
		"\n sw			$11, 4(%0)"

		"\n addiu		%0, %0, 8"
		"\n addiu		%1, %1, 8"
		"\n addi		%2, %2, -1"
		"\n j			rasterToFixed_padding"
		"\n nop"
"\n rasterToFixed_end:"

		 : : "r"(dst), "r"(src), "r"(N), "r"(almostOne4), "r"(convBuff4),
			"r"(scale4), "r"(&scale) : "$8", "$9", "$10", "$11" );

		// DEBUG DEBUG CANNOT BE VALIDATED PROPERLY (SEEMS LIKE ROUNDING GOES OFF SOMEHOW?)

		// validate
/*		#if defined (DPVS_DEBUG)
			for(int i=0;i<N;i++)
			{
				DPVS_ASSERT(dst[i][0] == Math::intFloor(scale.x*src[i].x));
				DPVS_ASSERT(dst[i][1] == Math::intFloor(scale.y*src[i].y));
			}
		#endif
*/
		return;
	}
	// FALLTHRU!
//PS2-END
#endif

#if defined (DPVS_X86_ASSEMBLY)

	//------------------------------------------------------------------------
	// On X87 we just change the rounding mode of the FPU once at the
	// beginning of the loop, then perform fistp instructions (handles the
	// intFloor() operation correctly). This is quite a bit faster than
	// using any other approach.
	//------------------------------------------------------------------------
	{
		X86::FPUControl foo(X86::FPUControl::ROUND_DOWN);	// change rounding mode

		DPVS_CT_ASSERT(sizeof(Vector2i)==8);
		DPVS_CT_ASSERT(sizeof(Vector2)==8);

		DPVS_ASM
		{
			mov		eax,dst
			mov		ebx,src
			mov		ecx,N
			mov		edx,scale
			fld		dword ptr [edx+0]		// st(2) (scale.x)
			fld		dword ptr [edx+4]		// st(1) (scale.y)
	loopie:
			fld		dword ptr [ebx]			// st(0) (src.x)
			fmul	st,st(2)				// scale.x
			fistp	dword ptr [eax]			// dst.x
			fld		dword ptr [ebx+4]       // src.y
			fmul	st,st(1)				// scale.y
			fistp	dword ptr [eax+4]		// dst.y
			add		ebx,8					// sizeof(Vector2)
			add		eax,8					// sizeof(Vector2i)
			dec		ecx
			jnz		loopie
			fcompp
		}

		// FPU rounding mode is returned here
	}

	return;
/*
	// make sure our routine works a-ok
#if defined (DPVS_DEBUG)
	for(int i=0;i<N;i++)
	{
		DPVS_ASSERT(dst[i][0] == Math::intFloor(scale.x*src[i].x));
		DPVS_ASSERT(dst[i][1] == Math::intFloor(scale.y*src[i].y));
	}

#endif
*/

#else
	float scaleX = scale.x;
	float scaleY = scale.y;

	for(int i=0;i<N;i++)
	{
		dst[i][0] = Math::intFloor(scaleX*src[i].x); 
		dst[i][1] = Math::intFloor(scaleY*src[i].y);
	}
#endif
}

/******************************************************************************
 *
 * Function:		DPVS::Math::screenToRaster()
 *
 * Description:		Performs screen-space into raster-space mapping of vectors
 *
 * Parameters:		dst		= source/destination array (performed in-place)
 *					N		= number of vectors
 *					raster	= reference to viewport rectangle structure
 *
 ******************************************************************************/

void screenToRaster (Vector2* dst, int N, const FloatRectangle& raster)
{
	DPVS_ASSERT(dst);
	float xs = 0.5f * raster.width();
	float ys = -0.5f * raster.height();
	float xo = raster.x0 + xs;
	float yo = raster.y0 - ys;

	for (int i = 0; i < N; i++)
	{
		dst[i].x = dst[i].x * xs + xo;
		dst[i].y = dst[i].y * ys + yo;
	}
}

/******************************************************************************
 *
 * Function:		DPVS::Math::screenToRasterDivByW()
 *
 * Description:		Performs division by W and screen-space into raster-space
 *					mapping of vectors
 *
 * Parameters:		dst		= destination array
 *					src		= source homogenous vector array
 *					N		= number of vectors
 *					raster	= reference to viewport rectangle structure
 *
 ******************************************************************************/

void screenToRasterDivByW (Vector2* dst, const Vector4* src, int N, const FloatRectangle& raster)
{
	DPVS_ASSERT(dst);
	float xs = 0.5f * raster.width();
	float ys = -0.5f * raster.height();
	float xo = raster.x0 + xs;
	float yo = raster.y0 - ys;

	for (int i = 0; i < N; i++)
	{
		float oow = Math::reciprocal(src[i].w);
		dst[i].x = src[i].x * oow * xs + xo;
		dst[i].y = src[i].y * oow * ys + yo;
	}
}

/******************************************************************************
 *
 * Function:		DPVS::Math::screenToRaster()
 *
 * Description:		Performs screen-space into raster-space mapping of vectors
 *
 * Parameters:		dst		= source/destination array (performed in-place)
 *					N		= number of vectors
 *					raster	= reference to viewport rectangle structure
 *
 ******************************************************************************/

void screenToRaster (Vector3* dst, int N, const FloatRectangle& raster)
{
	DPVS_ASSERT(dst);

	float xs = 0.5f * raster.width();
	float ys = -0.5f * raster.height();
	float xo = raster.x0 + xs;
	float yo = raster.y0 - ys;

	for (int i = 0; i < N; i++)
	{
		dst[i].x = dst[i].x * xs + xo;
		dst[i].y = dst[i].y * ys + yo;
		dst[i].z = dst[i].z * 0.5f + 0.5f;		// map to [0,1] range
	}
}

/******************************************************************************
 *
 * Function:		DPVS::Math::objectToRaster()
 *
 * Description:		Transforms using 'mtx', then performs divByW and viewport mapping
 *
 * Parameters:
 *
 * Notes:			MUST BE NON-FRONT-CLIPPING
 *
 ******************************************************************************/

void objectToRaster (Vector3* dest, const Vector3* src, int N, const Matrix4x4& mtx, const FloatRectangle& viewPort)
{
	DPVS_ASSERT (dest && src);

#if defined (DPVS_PS2_VU_ASSEMBLY)
//PS2-BEGIN

	if (0 == (7 & ((UPTR)dest)))
	{
		DPVS_ASSERT( 0 == (7 & ((UPTR)dest)) );
		DPVS_ASSERT( is128Aligned(&mtx) );
		DPVS_ASSERT( &viewPort.x0 + 1 == &viewPort.x1 );
		DPVS_ASSERT( &viewPort.x1 + 1 == &viewPort.y0 );
		DPVS_ASSERT( &viewPort.y0 + 1 == &viewPort.y1 );

		const float*				mtxp3 = &mtx[3][0]; // Codewarrior doesn't accept (&mtx[3]) as asm param
		float DPVS_VECTOR_ALIGN(scale[4]);
		float DPVS_VECTOR_ALIGN(ofs[4]);
		float DPVS_VECTOR_ALIGN(conv4[4]) ;
		Matrix4x4					tmtx(NO_CONSTRUCTOR); tmtx.transpose( mtx );

		asm __volatile__ (
		"	mov.s		$f10, %9"
			/* 	&viewPort	= %8
				0.5f		= $f10 */
		"\n	lwc1		$f1, 0(%8)"
		"\n	lwc1		$f2, 4(%8)"
		"\n	sub.s		$f3, $f2, $f1"
		"\n	mul.s		$f4, $f3, $f10"
		"\n	swc1		$f4, 0(%5)"
		"\n	add.s		$f4, $f1"
		"\n	swc1		$f4, 0(%6)"

		"\n	lwc1		$f1, 8(%8)"
		"\n	lwc1		$f2, 12(%8)"
		"\n	sub.s		$f3, $f2, $f1"
		"\n	mul.s		$f4, $f3, $f10"
		"\n	neg.s		$f4, $f4"
		"\n	swc1		$f4, 4(%5)"
		"\n	sub.s		$f1, $f4"
		"\n	swc1		$f1, 4(%6)"

		"\n	swc1		$f10, 8(%5)"
		"\n	swc1		$f10, 8(%6)"

		"\n	lqc2		vf11, 0x00(%3)"
		"\n	lqc2		vf12, 0x10(%3)"
		"\n	lqc2		vf13, 0x20(%3)"
		"\n	lqc2		vf14, 0x30(%3)"
		"\n	lqc2		vf18, 0(%4)"
		"\n	lqc2		vf19, 0(%5)"
		"\n	lqc2		vf20, 0(%6)"
			/* tmtx = vf11-14 */
			/* mtx3 = vf18 */
			/* scale = vf19 */
			/* ofs = vf20 */
	"\n	objectToRaster_loop:"
		"\n	beq			%2, $0, objectToRaster_done"
		"\n	nop"

		"\n	lw			$8,  0(%1)"
		"\n	lw			$9,  4(%1)"
		"\n	lw			$10, 8(%1)"
		"\n	sw			$8,  0(%7)"
		"\n	sw			$9,  4(%7)"
		"\n	sw			$10, 8(%7)"
		"\n	lqc2		vf5, 0(%7)"
		"\n	vaddx.w		vf5, vf0, vf0"
			/* (sx,sy,sz,1) = vf5 */

			/* float oow = Math::reciprocal(mtx[3].x * sx + mtx[3].y * sy + mtx[3].z * sz + mtx[3].w); */
		"\n	vmul.xyz	vf1, vf18, vf5"
		"\n	vadd.x		vf2, vf0, vf1"
		"\n	vaddy.x		vf3, vf2, vf1"
		"\n	vaddz.x		vf4, vf3, vf1"
		"\n	vaddw.x		vf1, vf4, vf18"
		"\n	vdiv		Q, vf0w, vf1x"
			/* oow = Q */

			/*
			dest->x = (tmtx.x[0] * sx + tmtx.y[0] * sy + tmtx.z[0] * sz + tmtx.w[0]) * xscale * oow + xofs;
			dest->y = (tmtx.x[1] * sx + tmtx.y[1] * sy + tmtx.z[1] * sz + tmtx.w[1]) * yscale * oow + yofs;
			dest->z = (tmtx.x[2] * sx + tmtx.y[2] * sy + tmtx.z[2] * sz + tmtx.w[2]) * 0.5f * oow + 0.5f;
			*/
		"\n	vmulax.xyz	ACC, vf11, vf5"
		"\n	vmadday.xyz	ACC, vf12, vf5"
		"\n	vmaddaz.xyz	ACC, vf13, vf5"
		"\n	vmaddw.xyz	vf2, vf14, vf0"
		"\n	vmul.xyz	vf3, vf2, vf19"
		"\n	vwaitq"
		"\n	vmulq.xyz	vf2, vf3, Q"
		"\n	vadd.xyz	vf1, vf2, vf20"
			/* dest = vf1 */

		"\n	qmfc2		$8, vf1"
		"\n	vmr32.xyzw	vf2, vf1"
		"\n	sw			$8, 0(%0)"
		"\n	qmfc2		$8, vf2"
		"\n	vmr32.xyzw	vf1, vf2"
		"\n	sw			$8, 4(%0)"
		"\n	qmfc2		$8, vf1"
		"\n	sw			$8, 8(%0)"

		"\n	addi		%0, %0, 12"
		"\n	addi		%1, %1, 12"
		"\n	addi		%2, %2, -1"
		"\n	j			objectToRaster_loop"
		"\n	nop"
	"\n	objectToRaster_done:"
		 : : "r"(dest), "r"(src), "r"(N), "r"(&tmtx), "r"(mtxp3),
			"r"(scale), "r"(ofs), "r"(conv4), "r"(&viewPort), "f"(0.5f) : "$8", "$9", "$10", "$f1", "$f2", "$f3", "$f4", "$f10" );

	// DEBUG DEBUG CANNOT BE VALIDATED (PLEASE CHECK AGAIN!)
/*
		#if defined (DPVS_DEBUG)
		{
			register float	xscale	=  0.5f * viewPort.width();
			register float	xofs	=  0.5f * viewPort.width()  + viewPort.x0;
			register float	yscale	= -0.5f * viewPort.height();
			register float	yofs	=  0.5f * viewPort.height() + viewPort.y0;

			for (int i = 0; i < N; i++, dest++, src++)
			{
				register float sx	= src->x;
				register float sy	= src->y;
				register float sz	= src->z;
				register float oow   = Math::reciprocal(mtx[3].x * sx + mtx[3].y * sy + mtx[3].z * sz + mtx[3].w);

				DPVS_ASSERT(isApproximatelyEqual(dest->x,(mtx[0].x * sx + mtx[0].y * sy + mtx[0].z * sz + mtx[0].w) * oow * xscale + xofs));
				DPVS_ASSERT(isApproximatelyEqual(dest->y,(mtx[1].x * sx + mtx[1].y * sy + mtx[1].z * sz + mtx[1].w) * oow * yscale + yofs));
				DPVS_ASSERT(isApproximatelyEqual(dest->z,(mtx[2].x * sx + mtx[2].y * sy + mtx[2].z * sz + mtx[2].w) * oow * 0.5f + 0.5f));
			}
		}
		#endif
*/
		return;
	}

	// FALLTHRU!!!!
//PS2-END
#endif

	float	xscale	=  0.5f * viewPort.width();
	float	xofs	=  0.5f * viewPort.width()  + viewPort.x0;
	float	yscale	= -0.5f * viewPort.height();
	float	yofs	=  0.5f * viewPort.height() + viewPort.y0;

	float m00 = mtx[0][0], m10 = mtx[1][0], m20 = mtx[2][0], m30 = mtx[3][0];
	float m01 = mtx[0][1], m11 = mtx[1][1], m21 = mtx[2][1], m31 = mtx[3][1];
	float m02 = mtx[0][2], m12 = mtx[1][2], m22 = mtx[2][2], m32 = mtx[3][2];
	float m03 = mtx[0][3], m13 = mtx[1][3], m23 = mtx[2][3], m33 = mtx[3][3];

	for (int i = 0; i < N; i++, dest++, src++)
	{
		float sx	= src->x;
		float sy	= src->y;
		float sz	= src->z;

//		float oow   = Math::reciprocal(mtx[3].x * sx + mtx[3].y * sy + mtx[3].z * sz + mtx[3].w);
		float oow   = Math::reciprocal(m30 * sx + m31 * sy + m32 * sz + m33);

		dest->x = (m00 * sx + m01 * sy + m02 * sz + m03) * oow * xscale + xofs;
		dest->y = (m10 * sx + m11 * sy + m12 * sz + m13) * oow * yscale + yofs;
		dest->z = (m20 * sx + m21 * sy + m22 * sz + m23) * oow * 0.5f + 0.5f;

//		dest->x = (mtx[0].x * sx + mtx[0].y * sy + mtx[0].z * sz + mtx[0].w) * oow * xscale + xofs;
//		dest->y = (mtx[1].x * sx + mtx[1].y * sy + mtx[1].z * sz + mtx[1].w) * oow * yscale + yofs;
//		dest->z = (mtx[2].x * sx + mtx[2].y * sy + mtx[2].z * sz + mtx[2].w) * oow * 0.5f + 0.5f;
	}
}

/******************************************************************************
 *
 * Function:		DPVS::Math::objectToRaster()
 *
 * Description:		Transforms using 'mtx', then performs divByW and viewport mapping
 *
 * Parameters:
 *
 * Notes:			MUST BE NON-FRONT-CLIPPING
 *
 ******************************************************************************/

void objectToRaster (Vector2* dest, const Vector3* src, int N, const Matrix4x4& mtx, const FloatRectangle& viewPort)
{
	DPVS_ASSERT (dest && src);

#if defined (DPVS_PS2_VU_ASSEMBLY)
//PS2-BEGIN

	DPVS_ASSERT( 0 == (7 & ((UPTR)dest)) );
	DPVS_ASSERT( is128Aligned(&mtx) );
	DPVS_ASSERT( (&viewPort.x0 + 1) == &viewPort.x1 );
	DPVS_ASSERT( (&viewPort.x1 + 1) == &viewPort.y0 );
	DPVS_ASSERT( (&viewPort.y0 + 1) == &viewPort.y1 );

	const float*				mtxp3 = &mtx[3][0];
	float DPVS_VECTOR_ALIGN(scale[4]);
	float DPVS_VECTOR_ALIGN(ofs[4]);
	float DPVS_VECTOR_ALIGN(conv4[4]);
	Matrix4x4					tmtx(NO_CONSTRUCTOR); tmtx.transpose( mtx );

	asm __volatile__ (
	"	mov.s		$f10, %9"
		/* 	&viewPort	= %8
			0.5f		= $f10 */
	"\n	lwc1		$f1, 0(%8)"
	"\n	lwc1		$f2, 4(%8)"
	"\n	sub.s		$f3, $f2, $f1"
	"\n	mul.s		$f4, $f3, $f10"
	"\n	swc1		$f4, 0(%5)"
	"\n	add.s		$f4, $f1"
	"\n	swc1		$f4, 0(%6)"

	"\n	lwc1		$f1, 8(%8)"
	"\n	lwc1		$f2, 12(%8)"
	"\n	sub.s		$f3, $f2, $f1"
	"\n	mul.s		$f4, $f3, $f10"
	"\n	neg.s		$f4, $f4"
	"\n	swc1		$f4, 4(%5)"
	"\n	sub.s		$f1, $f4"
	"\n	swc1		$f1, 4(%6)"

	"\n	lqc2		vf11, 0x00(%3)"
	"\n	lqc2		vf12, 0x10(%3)"
	"\n	lqc2		vf13, 0x20(%3)"
	"\n	lqc2		vf14, 0x30(%3)"
	"\n	lqc2		vf18, 0(%4)"
	"\n	lqc2		vf19, 0(%5)"
	"\n	lqc2		vf20, 0(%6)"
		/* tmtx = vf11-14 */
		/* mtx3 = vf18 */
		/* scale = vf19 */
		/* ofs = vf20 */
"\n	objectToRaster_loop2:"
	"\n	beq			%2, $0, objectToRaster_done2"
	"\n	nop"

	"\n	lw			$8,  0(%1)"
	"\n	lw			$9,  4(%1)"
	"\n	lw			$10, 8(%1)"
	"\n	sw			$8,  0(%7)"
	"\n	sw			$9,  4(%7)"
	"\n	sw			$10, 8(%7)"
	"\n	lqc2		vf5, 0(%7)"
		/* (sx,sy,sz) = vf5 */

		/* float oow = Math::reciprocal(mtx[3].x * sx + mtx[3].y * sy + mtx[3].z * sz + mtx[3].w); */
	"\n	vmul.xyz	vf1, vf18, vf5"
	"\n	vadd.x		vf2, vf0, vf1"
	"\n	vaddy.x		vf3, vf2, vf1"
	"\n	vaddz.x		vf4, vf3, vf1"
	"\n	vaddw.x		vf1, vf4, vf18"
	"\n	vdiv		Q, vf0w, vf1x"
		/* oow = Q */

		/*
		dest->x = (tmtx.x[0] * sx + tmtx.y[0] * sy + tmtx.z[0] * sz + tmtx.w[0]) * xscale * oow + xofs;
		dest->y = (tmtx.x[1] * sx + tmtx.y[1] * sy + tmtx.z[1] * sz + tmtx.w[1]) * yscale * oow + yofs;
		*/
	"\n	vmulax.xy	ACC, vf11, vf5"
	"\n	vmadday.xy	ACC, vf12, vf5"
	"\n	vmaddaz.xy	ACC, vf13, vf5"
	"\n	vmaddw.xy	vf2, vf14, vf0"
	"\n	vmul.xy		vf3, vf2, vf19"
	"\n	vwaitq"
	"\n	vmulq.xy	vf2, vf3, Q"
	"\n	vadd.xy		vf1, vf2, vf20"
		/* dest = vf1 */

	"\n	qmfc2		$8, vf1"
	"\n	vmr32.xyzw	vf2, vf1"
	"\n	sw			$8, 0(%0)"
	"\n	qmfc2		$8, vf2"
	"\n	sw			$8, 4(%0)"

	"\n	addi		%0, %0, 8"
	"\n	addi		%1, %1, 12"
	"\n	addi		%2, %2, -1"
	"\n	j			objectToRaster_loop2"
	"\n	nop"
"\n	objectToRaster_done2:"
	 : : "r"(dest), "r"(src), "r"(N), "r"(&tmtx), "r"(mtxp3),
		"r"(scale), "r"(ofs), "r"(conv4), "r"(&viewPort), "f"(0.5f) : "$8", "$9", "$10", "$f1", "$f2", "$f3", "$f4", "$f10" );

	#if defined (DPVS_DEBUG)
	{
		register float	xscale	=  0.5f * viewPort.width();
		register float	xofs	=  0.5f * viewPort.width()  + viewPort.x0;
		register float	yscale	= -0.5f * viewPort.height();
		register float	yofs	=  0.5f * viewPort.height() + viewPort.y0;

		for (int i = 0; i < N; i++, dest++, src++)
		{
			register float sx	= src->x;
			register float sy	= src->y;
			register float sz	= src->z;
			register float oow  = Math::reciprocal(mtx[3].x * sx + mtx[3].y * sy + mtx[3].z * sz + mtx[3].w);

			DPVS_ASSERT(isApproximatelyEqual(dest->x,(mtx[0].x * sx + mtx[0].y * sy + mtx[0].z * sz + mtx[0].w) * oow * xscale + xofs));
			DPVS_ASSERT(isApproximatelyEqual(dest->y,(mtx[1].x * sx + mtx[1].y * sy + mtx[1].z * sz + mtx[1].w) * oow * yscale + yofs));
		}
	}
	#endif
//PS2-END
#else

	float	xscale	=  0.5f * viewPort.width();
	float	xofs	=  0.5f * viewPort.width()  + viewPort.x0;
	float	yscale	= -0.5f * viewPort.height();
	float	yofs	=  0.5f * viewPort.height() + viewPort.y0;

	float m00 = mtx[0][0], m10 = mtx[1][0], m30 = mtx[3][0];
	float m01 = mtx[0][1], m11 = mtx[1][1], m31 = mtx[3][1];
	float m02 = mtx[0][2], m12 = mtx[1][2], m32 = mtx[3][2];
	float m03 = mtx[0][3], m13 = mtx[1][3], m33 = mtx[3][3];

	for (int i = 0; i < N; i++, dest++, src++)
	{
		float sx	= src->x;
		float sy	= src->y;
		float sz	= src->z;

		float oow	= Math::reciprocal(m30 * sx + m31 * sy + m32 * sz + m33);
//		float oow	= Math::reciprocal(mtx[3].x * sx + mtx[3].y * sy + mtx[3].z * sz + mtx[3].w);

		dest->x = (m00 * sx + m01 * sy + m02 * sz + m03) * oow * xscale + xofs;
		dest->y = (m10 * sx + m11 * sy + m12 * sz + m13) * oow * yscale + yofs;
//		dest->x = (mtx[0].x * sx + mtx[0].y * sy + mtx[0].z * sz + mtx[0].w) * oow * xscale + xofs;
//		dest->y = (mtx[1].x * sx + mtx[1].y * sy + mtx[1].z * sz + mtx[1].w) * oow * yscale + yofs;
	}

#endif
}

/******************************************************************************
 *
 * Function:		DPVS::Math::transformAndDivByW()
 *
 * Description:		Transforms using 'mtx', then performs divByW
 *
 * Parameters:
 *
 * Notes:			MUST BE NON-FRONT-CLIPPING
 *
 ******************************************************************************/

void transformAndDivByW (Vector3* dest, const Vector3* src, int N, const Matrix4x4& mtx)
{
	DPVS_ASSERT (dest && src && N > 0);

	float m00 = mtx[0][0], m10 = mtx[1][0], m20 = mtx[2][0], m30 = mtx[3][0];
	float m01 = mtx[0][1], m11 = mtx[1][1], m21 = mtx[2][1], m31 = mtx[3][1];
	float m02 = mtx[0][2], m12 = mtx[1][2], m22 = mtx[2][2], m32 = mtx[3][2];
	float m03 = mtx[0][3], m13 = mtx[1][3], m23 = mtx[2][3], m33 = mtx[3][3];

	for (int i = 0; i < N; i++, dest++, src++)
	{
		float sx = src->x;
		float sy = src->y;
		float sz = src->z;
		float oow = Math::reciprocal(m30 * sx + m31 * sy + m32 * sz + m33);

		dest->x = (m00 * sx + m01 * sy + m02 * sz + m03) * oow;
		dest->y = (m10 * sx + m11 * sy + m12 * sz + m13) * oow;
		dest->z = (m20 * sx + m21 * sy + m22 * sz + m23) * oow;
	}
}

/******************************************************************************
 *
 * Function:		DPVS::Math::transform()
 *
 * Description:		Transforms an array of vectors
 *
 * Parameters:		dest = destination vector array (homogenous)
 *					src  = source vector array
 *					N	 = number of vector
 *					mtx  = reference to 4x4 transformation matrix
 *
 ******************************************************************************/

void transform (Vector4* dest, const Vector3* src, int N, const Matrix4x4& mtx)
{
	DPVS_ASSERT (dest && src && N > 0);

#if defined (DPVS_PS2_VU_ASSEMBLY)
//PS2-BEGIN

	float DPVS_VECTOR_ALIGN(src4[4]);
	Matrix4x4 mtxTransposed(NO_CONSTRUCTOR); mtxTransposed.transpose( mtx );

	DPVS_ASSERT( is128Aligned(dest) );
	DPVS_ASSERT( is128Aligned(src4) );
	DPVS_ASSERT( is128Aligned(&mtxTransposed) );

	asm __volatile__ (
		/* mtx -> vf11-14 */
	"	lqc2		vf11, 0(%3)"
	"\n	lqc2		vf12, 0x10(%3)"
	"\n	lqc2		vf13, 0x20(%3)"
	"\n	lqc2		vf14, 0x30(%3)"
"\n	transform3_loop:"
	"\n	beq			%2, $0, transform3_done"
	"\n	nop"

		/* src -> src4 */
	"\n	lw			$8, 0(%1)"
	"\n	lw			$9, 4(%1)"
	"\n	lw			$10, 8(%1)"
	"\n	sw			$8, 0(%4)"
	"\n	sw			$9, 4(%4)"
	"\n	sw			$10, 8(%4)"

		/* src4 -> vf1 */
	"\n	lqc2		vf1, 0(%4)"

		/*
		dest->x = mtxT.x[0] * sx + mtxT.y[0] * sy + mtxT.z[0] * sz + mtxT.w[0];
		dest->y = mtxT.x[1] * sx + mtxT.y[1] * sy + mtxT.z[1] * sz + mtxT.w[1];
		dest->z = mtxT.x[2] * sx + mtxT.y[2] * sy + mtxT.z[2] * sz + mtxT.w[2];
		dest->w = mtxT.x[3] * sx + mtxT.y[3] * sy + mtxT.z[3] * sz + mtxT.w[3];
		*/
	"\n	vmulax.xyzw		ACC, vf11, vf1"
	"\n	vmadday.xyzw	ACC, vf12, vf1"
	"\n	vmaddaz.xyzw	ACC, vf13, vf1"
	"\n	vmaddw.xyzw		vf2, vf14, vf0"

	"\n	sqc2		vf2, 0(%0)"

	"\n	addi		%0, %0, 16"
	"\n	addi		%1, %1, 12"
	"\n	addi		%2, %2, -1"
	"\n	j			transform3_loop"
	"\n	nop"
	"\n	transform3_done:"
	 : : "r"(dest), "r"(src), "r"(N), "r"(&mtxTransposed), "r"(src4) : "$8", "$9", "$10" );

	#if defined (DPVS_DEBUG)
	for (int i = 0; i < N; i++, dest++, src++)
	{
		register float sx = src->x;
		register float sy = src->y;
		register float sz = src->z;

		DPVS_ASSERT(isApproximatelyEqual(dest->x,mtx[0].x * sx + mtx[0].y * sy + mtx[0].z * sz + mtx[0].w));
		DPVS_ASSERT(isApproximatelyEqual(dest->y,mtx[1].x * sx + mtx[1].y * sy + mtx[1].z * sz + mtx[1].w));
		DPVS_ASSERT(isApproximatelyEqual(dest->z,mtx[2].x * sx + mtx[2].y * sy + mtx[2].z * sz + mtx[2].w));
		DPVS_ASSERT(isApproximatelyEqual(dest->w,mtx[3].x * sx + mtx[3].y * sy + mtx[3].z * sz + mtx[3].w));
	}
	#endif

	return;
//PS2-END
#else

	float m00 = mtx[0][0], m10 = mtx[1][0], m20 = mtx[2][0], m30 = mtx[3][0];
	float m01 = mtx[0][1], m11 = mtx[1][1], m21 = mtx[2][1], m31 = mtx[3][1];
	float m02 = mtx[0][2], m12 = mtx[1][2], m22 = mtx[2][2], m32 = mtx[3][2];
	float m03 = mtx[0][3], m13 = mtx[1][3], m23 = mtx[2][3], m33 = mtx[3][3];

	for (int i = 0; i < N; i++, dest++, src++)
	{
		float sx = src->x;
		float sy = src->y;
		float sz = src->z;

		dest->x = m00 * sx + m01 * sy + m02 * sz + m03;
		dest->y = m10 * sx + m11 * sy + m12 * sz + m13;
		dest->z = m20 * sx + m21 * sy + m22 * sz + m23;
		dest->w = m30 * sx + m31 * sy + m32 * sz + m33;
	}
#endif
}

/******************************************************************************
 *
 * Function:		DPVS::Math::transform()
 *
 * Description:		Transforms an array of vectors
 *
 * Parameters:		dest = destination vector array (homogenous)
 *					src  = source vector array
 *					N	 = number of vector
 *					mtx  = reference to 4x4 transformation matrix
 *
 ******************************************************************************/

void transform (Vector4* dest, const Vector4* src, int N, const Matrix4x4& mtx)
{
	DPVS_ASSERT (dest && src && N > 0);

#if defined (DPVS_PS2_VU_ASSEMBLY)
//PS2-BEGIN

	Matrix4x4 mtxTransposed(NO_CONSTRUCTOR); mtxTransposed.transpose( mtx );

	DPVS_ASSERT( is128Aligned(dest) );
	DPVS_ASSERT( is128Aligned(src) );
	DPVS_ASSERT( is128Aligned(&mtxTransposed) );

	asm __volatile__ (
	"	lqc2		vf11, 0(%3)"
	"\n	lqc2		vf12, 0x10(%3)"
	"\n	lqc2		vf13, 0x20(%3)"
	"\n	lqc2		vf14, 0x30(%3)"
"\n	transform4_loop:"
	"\n	beq			%2, $0, transform4_done"
	"\n	nop"

	"\n	lqc2			vf1, 0(%1)"
	"\n	vmulax.xyzw		ACC, vf11, vf1"
	"\n	vmadday.xyzw	ACC, vf12, vf1"
	"\n	vmaddaz.xyzw	ACC, vf13, vf1"
	"\n	vmaddw.xyzw		vf2, vf14, vf1"
	"\n	sqc2			vf2, 0(%0)"

	"\n	addi		%0, %0, 16"
	"\n	addi		%1, %1, 16"
	"\n	addi		%2, %2, -1"
	"\n	j			transform4_loop"
	"\n	nop"
"\n	transform4_done:"
	 : : "r"(dest), "r"(src), "r"(N), "r"(&mtxTransposed) : "$8", "$9", "$10" );

	#if defined (DPVS_DEBUG)
	for (int i = 0; i < N; i++, dest++, src++)
	{
		float sx = src->x;
		float sy = src->y;
		float sz = src->z;
		float sw = src->z;

		DPVS_ASSERT(isApproximatelyEqual(dest->x,mtx[0].x * sx + mtx[0].y * sy + mtx[0].z * sz + mtx[0].w * sw));
		DPVS_ASSERT(isApproximatelyEqual(dest->y,mtx[1].x * sx + mtx[1].y * sy + mtx[1].z * sz + mtx[1].w * sw));
		DPVS_ASSERT(isApproximatelyEqual(dest->z,mtx[2].x * sx + mtx[2].y * sy + mtx[2].z * sz + mtx[2].w * sw));
		DPVS_ASSERT(isApproximatelyEqual(dest->w,mtx[3].x * sx + mtx[3].y * sy + mtx[3].z * sz + mtx[3].w * sw));
	}
	#endif // DPVS_DEBUG
//PS2-END
#else

	float m00 = mtx[0][0], m10 = mtx[1][0], m20 = mtx[2][0], m30 = mtx[3][0];
	float m01 = mtx[0][1], m11 = mtx[1][1], m21 = mtx[2][1], m31 = mtx[3][1];
	float m02 = mtx[0][2], m12 = mtx[1][2], m22 = mtx[2][2], m32 = mtx[3][2];
	float m03 = mtx[0][3], m13 = mtx[1][3], m23 = mtx[2][3], m33 = mtx[3][3];

	for (int i = 0; i < N; i++, dest++, src++)
	{
		float sx = src->x;
		float sy = src->y;
		float sz = src->z;
		float sw = src->z;

		dest->x = m00 * sx + m01 * sy + m02 * sz + m03 + m03 * sw;
		dest->y = m10 * sx + m11 * sy + m12 * sz + m13 + m13 * sw;
		dest->z = m20 * sx + m21 * sy + m22 * sz + m23 + m23 * sw;
		dest->w = m30 * sx + m31 * sy + m32 * sz + m33 + m33 * sw;
//		dest->x = mtx[0].x * sx + mtx[0].y * sy + mtx[0].z * sz + mtx[0].w * sw;
//		dest->y = mtx[1].x * sx + mtx[1].y * sy + mtx[1].z * sz + mtx[1].w * sw;
//		dest->z = mtx[2].x * sx + mtx[2].y * sy + mtx[2].z * sz + mtx[2].w * sw;
//		dest->w = mtx[3].x * sx + mtx[3].y * sy + mtx[3].z * sz + mtx[3].w * sw;
	}
#endif
}

/******************************************************************************
 *
 * Function:		DPVS::Math::minMax()
 *
 * Description:		Returns component-wise minimum and maximum vectors
 *
 * Parameters:		mn	= resulting minimum values
 *					mx	= resulting maximum values
 *					src = array of source vectors
 *					N	= number of source vectors (MUST BE > 0!!)
 *
 ******************************************************************************/

void minMax	(Vector2& mn, Vector2& mx, const Vector2* src, int N)
{
	// HEEBO
	DPVS_ASSERT(N>0 && src);
/*
#if defined (DPVS_X86_ASSEMBLY)

	// This piece of code has been taken from SurRender Vector Processor (tested by srVPTest)

	if (X86::getInstructionSets() & X86::S_CMOV)	// if X86-conditional moves supported by processor
	{
		__asm
		{
			mov		eax,src
			mov		ecx,N

			shl		ecx,2
			jz		_ulos
			lea		ecx,[ecx+ecx*2]
			add		eax,ecx
			neg		ecx

			fld		dword ptr[eax+ecx]
			fld		st(0)
			fld		dword ptr[eax+ecx+4]
			fld		st(0)
			fld		dword ptr[eax+ecx+8]
			fld		st(0)
			add		ecx,12
			jns		_store

			jmp		_lp
			align	16

			_lp:
				fld		dword ptr[eax+ecx]		//minZ maxZ minY maxY minX maxX
				FUCOMI	(5)
				fxch	st(5)
				FCMOVB	(5)
				fxch	st(5)
				FUCOMI	(6)
				fxch	st(6)
				FCMOVNBE(6)
				fstp	st(6)
				fld		dword ptr[eax+ecx+4]	//minZ maxZ minY maxY minX maxX
				FUCOMI	(3)
				fxch	st(3)
				FCMOVB	(3)
				fxch	st(3)
				FUCOMI	(4)
				fxch	st(4)
				FCMOVNBE(4)
				fstp	st(4)
				fld		dword ptr[eax+ecx+8]	//minZ maxZ minY maxY minX maxX
				FUCOMI	(1)
				fxch	st(1)
				FCMOVB	(1)
				fxch	st(1)
				FUCOMI	(2)
				fxch	st(2)
				FCMOVNBE(2)
				fstp	st(2)
				add		ecx,12
				js		_lp

		_store:
			mov		eax,mn
			mov		ecx,mx
			fstp	dword ptr[eax+8]
			fstp	dword ptr[ecx+8]
			fstp	dword ptr[eax+4]
			fstp	dword ptr[ecx+4]
			fstp	dword ptr[eax]
			fstp	dword ptr[ecx]
		_ulos:
		}
		return;
	}

	// else: fallthru!
#endif
*/
	//--------------------------------------------------------------------
	// C version used by other platforms and non-CMOV x86 platforms..
	//--------------------------------------------------------------------

	register float mnx = src[0].x;
	register float mny = src[0].y;
	register float mxx = mnx;
	register float mxy = mny;

	for (int i = 1; i < N; i++)
	{
		float x = src[i].x, y = src[i].y;
		mnx = Math::min(mnx,x);
		mxx = Math::max(mxx,x);
		mny = Math::min(mny,y);
		mxy = Math::max(mxy,y);
	}

	mn.x = mnx;
	mn.y = mny;
	mx.x = mxx;
	mx.y = mxy;
}

/******************************************************************************
 *
 * Function:		DPVS::Math::minMax()
 *
 * Description:		Returns component-wise minimum and maximum vectors
 *
 * Parameters:		mn	= resulting minimum values
 *					mx	= resulting maximum values
 *					src = array of source vectors
 *					N	= number of source vectors (MUST BE > 0!!)
 *
 ******************************************************************************/

void minMax	(Vector3& mn, Vector3& mx, const Vector3* src, int N)
{
	DPVS_ASSERT(N>0 && src);

#if defined (DPVS_PS2_VU_ASSEMBLY)
//PS2-BEGIN

	float DPVS_VECTOR_ALIGN(src4[4]);
	float DPVS_VECTOR_ALIGN(min4[4]);
	float DPVS_VECTOR_ALIGN(max4[4]);

	asm __volatile__ (
	"	swc1		%5, 0(%0);	swc1		%5, 4(%0);	swc1		%5, 8(%0)"
	"\n	neg.s		%5, %5"
	"\n	swc1		%5, 0(%1);	swc1		%5, 4(%1);	swc1		%5, 8(%1)"

	"\n	lqc2		vf10, 0(%0)"
	"\n	lqc2		vf11, 0(%1)"
"\n	minMax_loop:"
	"\n	beq			%3, $0, minMax_done"
	"\n	nop"

		/* src -> src4 */
	"\n	lw			$8, 0(%2)"
	"\n	lw			$9, 4(%2)"
	"\n	lw			$10, 8(%2)"
	"\n	sw			$8, 0(%4)"
	"\n	sw			$9, 4(%4)"
	"\n	sw			$10, 8(%4)"
	"\n	lqc2		vf8, 0(%4)"
		/* src4 = vf8 */

		/* vf10 = min(src4,vf10) */
	"\n	vadd.xyz	vf1, vf10, vf0"
	"\n	vmini.xyz	vf10, vf1, vf8"

		/* vf11 = max(src4,vf11) */
	"\n	vadd.xyz	vf1, vf11, vf0"
	"\n	vmax.xyz	vf11, vf1, vf8"

	"\n	addi		%3, %3, -1"
	"\n	addi		%2, %2, 12"
	"\n	j			minMax_loop"
	"\n	nop"
"\n	minMax_done:"
	"\n	sqc2		vf10, 0(%0)"
	"\n	sqc2		vf11, 0(%1)"
	 : : "r"(min4), "r"(max4), "r"(src), "r"(N), "r"(src4),
	"f"(FLT_MAX) : "$8", "$9", "$10" );

	mn.x = min4[0];
	mn.y = min4[1];
	mn.z = min4[2];
	mx.x = max4[0];
	mx.y = max4[1];
	mx.z = max4[2];

	#if defined (DPVS_DEBUG)
	{
		float mnx = src[0].x;
		float mny = src[0].y;
		float mnz = src[0].z;
		float mxx = src[0].x;
		float mxy = src[0].y;
		float mxz = src[0].z;

		for (int i = 1; i < N; i++)
		{
			float x = src[i].x, y = src[i].y, z = src[i].z;
			mnx = Math::min(mnx,x);
			mxx = Math::max(mxx,x);
			mny = Math::min(mny,y);
			mxy = Math::max(mxy,y);
			mnz = Math::min(mnz,z);
			mxz = Math::max(mxz,z);
		}
		DPVS_ASSERT(mn.x == mnx);
		DPVS_ASSERT(mn.y == mny);
		DPVS_ASSERT(mn.z == mnz);
		DPVS_ASSERT(mx.x == mxx);
		DPVS_ASSERT(mx.y == mxy);
		DPVS_ASSERT(mx.z == mxz);
	}
	#endif

	return;
//PS2-END
#elif defined (DPVS_X86_ASSEMBLY)

	// This piece of code has been taken from SurRender Vector Processor (tested by srVPTest)

	if (X86::getInstructionSets() & X86::S_CMOV)	// if X86-conditional moves supported by processor
	{
		__asm
		{
			mov		eax,src
			mov		ecx,N

			shl		ecx,2
			jz		_ulos
			lea		ecx,[ecx+ecx*2]
			add		eax,ecx
			neg		ecx

			fld		dword ptr[eax+ecx]
			fld		st(0)
			fld		dword ptr[eax+ecx+4]
			fld		st(0)
			fld		dword ptr[eax+ecx+8]
			fld		st(0)
			add		ecx,12
			jns		_store

			jmp		_lp
			align	16

			_lp:
				fld		dword ptr[eax+ecx]		//minZ maxZ minY maxY minX maxX
				FUCOMI	(5)
				fxch	st(5)
				FCMOVB	(5)
				fxch	st(5)
				FUCOMI	(6)
				fxch	st(6)
				FCMOVNBE(6)
				fstp	st(6)
				fld		dword ptr[eax+ecx+4]	//minZ maxZ minY maxY minX maxX
				FUCOMI	(3)
				fxch	st(3)
				FCMOVB	(3)
				fxch	st(3)
				FUCOMI	(4)
				fxch	st(4)
				FCMOVNBE(4)
				fstp	st(4)
				fld		dword ptr[eax+ecx+8]	//minZ maxZ minY maxY minX maxX
				FUCOMI	(1)
				fxch	st(1)
				FCMOVB	(1)
				fxch	st(1)
				FUCOMI	(2)
				fxch	st(2)
				FCMOVNBE(2)
				fstp	st(2)
				add		ecx,12
				js		_lp

		_store:
			mov		eax,mn
			mov		ecx,mx
			fstp	dword ptr[eax+8]
			fstp	dword ptr[ecx+8]
			fstp	dword ptr[eax+4]
			fstp	dword ptr[ecx+4]
			fstp	dword ptr[eax]
			fstp	dword ptr[ecx]
		_ulos:
		}
		return;
	}

	// else: fallthru!
#endif

	//--------------------------------------------------------------------
	// C version used by other platforms and non-CMOV x86 platforms..
	//--------------------------------------------------------------------

	float mnx = src[0].x;
	float mny = src[0].y;
	float mnz = src[0].z;
	float mxx = src[0].x;
	float mxy = src[0].y;
	float mxz = src[0].z;

	for (int i = 1; i < N; i++)
	{
		float x = src[i].x, y = src[i].y, z = src[i].z;
		mnx = Math::min(mnx,x);
		mxx = Math::max(mxx,x);
		mny = Math::min(mny,y);
		mxy = Math::max(mxy,y);
		mnz = Math::min(mnz,z);
		mxz = Math::max(mxz,z);
	}

	mn.x = mnx;
	mn.y = mny;
	mn.z = mnz;
	mx.x = mxx;
	mx.y = mxy;
	mx.z = mxz;
}

/******************************************************************************
 *
 * Function:		DPVS::Math::minMaxTransform()
 *
 * Description:		Transforms vectors in src[] by matrix 'mtx', then finds
 *					min/max for all three components.
 *
 * Parameters:		mn	= resulting minimum values
 *					mx	= resulting maximum values
 *					src = array of source vectors
 *					mtx = transformation matrix
 *					N	= number of source vectors (MUST BE > 0!!)
 *
 * Notes:			This function is called when we need to find the exact cell-space
 *					AABB of an object. It is therefore time-critical if new objects
 *					are created often, and it constitutes the majority of the
 *					startup time for large databases.
 *
 ******************************************************************************/

void minMaxTransform (Vector3& mn, Vector3& mx, const Vector3* src, const Matrix4x3& mtx,int N)
{
	DPVS_ASSERT(N>0 && src);

	if (N > 0)
	{
		float mnx =  FLT_MAX;
		float mny =  FLT_MAX;
		float mnz =  FLT_MAX;
		float mxx = -FLT_MAX;
		float mxy = -FLT_MAX;
		float mxz = -FLT_MAX;
		float m00 = mtx[0][0], m10 = mtx[1][0], m20 = mtx[2][0];
		float m01 = mtx[0][1], m11 = mtx[1][1], m21 = mtx[2][1];
		float m02 = mtx[0][2], m12 = mtx[1][2], m22 = mtx[2][2];

		for(int i = 0; i < N; i++)
		{
			float	sx  = src[i].x;
			float	sy	= src[i].y;
			float	sz	= src[i].z;

			float	x	= (m00 * sx + m01 * sy + m02 * sz);
			float	y	= (m10 * sx + m11 * sy + m12 * sz);
			float	z	= (m20 * sx + m21 * sy + m22 * sz);

			mnx = Math::min(mnx,x);
			mxx = Math::max(mxx,x);
			mny = Math::min(mny,y);
			mxy = Math::max(mxy,y);
			mnz = Math::min(mnz,z);
			mxz = Math::max(mxz,z);
		}

		mn.x = mnx + mtx[0].w;
		mx.x = mxx + mtx[0].w;
		mn.y = mny + mtx[1].w;
		mx.y = mxy + mtx[1].w;
		mn.z = mnz + mtx[2].w;
		mx.z = mxz + mtx[2].w;
	}
}

/******************************************************************************
 *
 * Function:		DPVS::Math::minMaxTransformDivByW()
 *
 * Description:		Transforms vectors in 'src' by 'm', divides by W component, then finds
 *					component-wise min & max
 *
 * Parameters:		mn	= resulting minimum values
 *					mx	= resulting maximum values
 *					src = array of source vectors
 *					mtx = transformation matrix
 *					N	= number of source vectors (MUST BE > 0!!)
 *
 * Returns:			boolean value indicating if the vector set is front-clipping (and thus invalid)
 *
 ******************************************************************************/

bool minMaxTransformDivByW (Vector3& mn, Vector3& mx, const Vector3* src, const Matrix4x4& mtx,int N)
{
	mn.make(FLT_MAX,FLT_MAX,FLT_MAX);
	mx.make(-FLT_MAX,-FLT_MAX,-FLT_MAX);

	float m00 = mtx[0][0], m10 = mtx[1][0], m20 = mtx[2][0], m30 = mtx[3][0];
	float m01 = mtx[0][1], m11 = mtx[1][1], m21 = mtx[2][1], m31 = mtx[3][1];
	float m02 = mtx[0][2], m12 = mtx[1][2], m22 = mtx[2][2], m32 = mtx[3][2];
	float m03 = mtx[0][3], m13 = mtx[1][3], m23 = mtx[2][3], m33 = mtx[3][3];

	for(int i = 0; i < N; i++, src++)
	{
		float z = m20 * src->x + m21 * src->y + m22 * src->z + m23;
		float w = m30 * src->x + m31 * src->y + m32 * src->z + m33;

		if(z <= -w)	// near-clipping
			return true;

		float oow	= Math::reciprocal(w);
		float x		= (m00 * src->x + m01 * src->y + m02 * src->z + m03) * oow;
		float y		= (m10 * src->x + m11 * src->y + m12 * src->z + m13) * oow;

		z*=oow;

		mn.x = Math::min(mn.x,x);
		mx.x = Math::max(mx.x,x);
		mn.y = Math::min(mn.y,y);
		mx.y = Math::max(mx.y,y);
		mn.z = Math::min(mn.z,z);
		mx.z = Math::max(mx.z,z);
	}

	return false;
}

/******************************************************************************
 *
 * Function:		DPVS::Math::maxDist()
 *
 * Description:		Returns maximum distance of point set to specified point
 *
 * Parameters:		pt	= point
 *					src = input point set
 *					N	= number of input points
 *
 * Returns:			maximum distance to 'pt'
 *
 ******************************************************************************/

float maxDist (const Vector3& pt, const Vector3* src, int N)
{
	float md = 0.0f;

	float x = pt.x;
	float y = pt.y;
	float z = pt.z;

	for (int i = 0; i < N; i++)
	{
		float d = Math::sqr(src[i].x - x) + Math::sqr(src[i].y - y) + Math::sqr(src[i].z - z);
		md = Math::max(d,md);
	}

	return Math::sqrt(md);
}


/******************************************************************************
 *
 * Function:		DPVS::Math::productFromLeft()
 *
 * Description:
 *
 * Parameters:		d = reference to destination matrix
 *					m = reference to first source matrix
 *					n = reference to second source matrix
 *
 ******************************************************************************/

void productFromLeft(Matrix4x3& d, const Matrix4x3& m, const Matrix4x3& n)
{
	DPVS_ASSERT ((&d != &n) && (&d != &m));

#if defined (DPVS_PS2_VU_ASSEMBLY)
//PS2-BEGIN
	DPVS_ASSERT( is128Aligned(&n) );
	DPVS_ASSERT( is128Aligned(&m) );
	DPVS_ASSERT( is128Aligned(&d) );

	asm __volatile__ (
	"	lqc2			vf11, 0x0(%0)"
	"\n	lqc2			vf12, 0x10(%0)"
	"\n	lqc2			vf13, 0x20(%0)"
	"\n	lqc2			vf15, 0x0(%1)"
	"\n	lqc2			vf16, 0x10(%1)"
	"\n	lqc2			vf17, 0x20(%1)"
	"\n	vmulax.xyzw		ACC, vf11, vf15"
	"\n	vmadday.xyzw	ACC, vf12, vf15"
	"\n	vmaddaz.xyzw	ACC, vf13, vf15"
	"\n	vmaddw.xyzw		vf1, vf0, vf15"
	"\n	sqc2			vf1, 0x0(%2)"
	"\n	vmulax.xyzw		ACC, vf11, vf16"
	"\n	vmadday.xyzw	ACC, vf12, vf16"
	"\n	vmaddaz.xyzw	ACC, vf13, vf16"
	"\n	vmaddw.xyzw		vf1, vf0, vf16"
	"\n	sqc2			vf1, 0x10(%2)"
	"\n	vmulax.xyzw		ACC, vf11, vf17"
	"\n	vmadday.xyzw	ACC, vf12, vf17"
	"\n	vmaddaz.xyzw	ACC, vf13, vf17"
	"\n	vmaddw.xyzw		vf1, vf0, vf17"
	"\n	sqc2			vf1, 0x20(%2)"
	 : : "r"(&m), "r"(&n), "r"(&d) );
//PS2-END
#else

	float n00 = n[0][0], n01 = n[0][1], n02 = n[0][2];
	float m00 = m[0][0], m10 = m[1][0], m20 = m[2][0];
	
	d[0].x = n00*m00 + n01*m10 + n02*m20;

	float m01 = m[0][1], m11 = m[1][1], m21 = m[2][1];

	d[0].y = n00*m01 + n01*m11 + n02*m21;

	float m02 = m[0][2], m12 = m[1][2], m22 = m[2][2];

	d[0].z = n00*m02 + n01*m12 + n02*m22;

	float m03 = m[0][3], m13 = m[1][3], m23 = m[2][3];

	d[0].w = n00*m03 + n01*m13 + n02*m23 + n[0][3];

	float n10 = n[1][0], n11 = n[1][1], n12 = n[1][2];

	d[1].x = n10*m00 + n11*m10 + n12*m20;
	d[1].y = n10*m01 + n11*m11 + n12*m21;
	d[1].z = n10*m02 + n11*m12 + n12*m22;
	d[1].w = n10*m03 + n11*m13 + n12*m23 + n[1][3];

	float n20 = n[2][0], n21 = n[2][1], n22 = n[2][2];

	d[2].x = n20*m00 + n21*m10 + n22*m20;
	d[2].y = n20*m01 + n21*m11 + n22*m21;
	d[2].z = n20*m02 + n21*m12 + n22*m22;
	d[2].w = n20*m03 + n21*m13 + n22*m23 + n[2][3];

#endif
}
/******************************************************************************
 *
 * Function:		DPVS::Math::productFromLeft()
 *
 * Description:
 *
 * Parameters:		d = reference to destination 4x4 matrix
 *					m = reference to first source 4x3 matrix
 *					n = reference to second source 4x3 matrix
 *
 ******************************************************************************/

void productFromLeft(Matrix4x4& d, const Matrix4x3& m, const Matrix4x3& n)
{
#if defined (DPVS_PS2_VU_ASSEMBLY)
//PS2-BEGIN

	DPVS_ASSERT( is128Aligned(&n) );
	DPVS_ASSERT( is128Aligned(&m) );
	DPVS_ASSERT( is128Aligned(&d) );

	asm __volatile__ (
	"	lqc2			vf11, 0x0(%0)"
	"\n	lqc2			vf12, 0x10(%0)"
	"\n	lqc2			vf13, 0x20(%0)"
	"\n	lqc2			vf15, 0x0(%1)"
	"\n	lqc2			vf16, 0x10(%1)"
	"\n	lqc2			vf17, 0x20(%1)"
	"\n	vmulax.xyzw		ACC, vf11, vf15"
	"\n	vmadday.xyzw	ACC, vf12, vf15"
	"\n	vmaddaz.xyzw	ACC, vf13, vf15"
	"\n	vmaddw.xyzw		vf1, vf0, vf15"
	"\n	sqc2			vf1, 0x0(%2)"
	"\n	vmulax.xyzw		ACC, vf11, vf16"
	"\n	vmadday.xyzw	ACC, vf12, vf16"
	"\n	vmaddaz.xyzw	ACC, vf13, vf16"
	"\n	vmaddw.xyzw		vf1, vf0, vf16"
	"\n	sqc2			vf1, 0x10(%2)"
	"\n	vmulax.xyzw		ACC, vf11, vf17"
	"\n	vmadday.xyzw	ACC, vf12, vf17"
	"\n	vmaddaz.xyzw	ACC, vf13, vf17"
	"\n	vmaddw.xyzw		vf1, vf0, vf17"
	"\n	sqc2			vf1, 0x20(%2)"
	"\n	sqc2			vf0, 0x30(%2)"
	 : : "r"(&m), "r"(&n), "r"(&d) );
//PS2-END
#else

	d[3].x = 0.0f;
	d[3].y = 0.0f;
	d[3].z = 0.0f;
	d[3].w = 1.0f;

	float n00 = n[0][0], n01 = n[0][1], n02 = n[0][2];
	float m00 = m[0][0], m01 = m[0][1], m02 = m[0][2], m03 = m[0][3];
	float m10 = m[1][0], m11 = m[1][1], m12 = m[1][2], m13 = m[1][3];
	float m20 = m[2][0], m21 = m[2][1], m22 = m[2][2], m23 = m[2][3];
//	float m30 = m[3][0], m31 = m[3][1], m32 = m[3][2], m33 = m[3][3];

	d[0].x = n00*m00 + n01*m10 + n02*m20;
	d[0].y = n00*m01 + n01*m11 + n02*m21;
	d[0].z = n00*m02 + n01*m12 + n02*m22;
	d[0].w = n00*m03 + n01*m13 + n02*m23 + n[0][3];

	float n10 = n[1][0], n11 = n[1][1], n12 = n[1][2];

	d[1].x = n10*m00 + n11*m10 + n12*m20;
	d[1].y = n10*m01 + n11*m11 + n12*m21;
	d[1].z = n10*m02 + n11*m12 + n12*m22;
	d[1].w = n10*m03 + n11*m13 + n12*m23 + n[1][3];

	float n20 = n[2][0], n21 = n[2][1], n22 = n[2][2];

	d[2].x = n20*m00 + n21*m10 + n22*m20;
	d[2].y = n20*m01 + n21*m11 + n22*m21;
	d[2].z = n20*m02 + n21*m12 + n22*m22;
	d[2].w = n20*m03 + n21*m13 + n22*m23 + n[2][3];


/*	old version
	d[0].x = n[0][0]*m[0][0] + n[0][1]*m[1][0] + n[0][2]*m[2][0];
	d[0].y = n[0][0]*m[0][1] + n[0][1]*m[1][1] + n[0][2]*m[2][1];
	d[0].z = n[0][0]*m[0][2] + n[0][1]*m[1][2] + n[0][2]*m[2][2];
	d[0].w = n[0][0]*m[0][3] + n[0][1]*m[1][3] + n[0][2]*m[2][3] + n[0][3];
	d[1].x = n[1][0]*m[0][0] + n[1][1]*m[1][0] + n[1][2]*m[2][0];
	d[1].y = n[1][0]*m[0][1] + n[1][1]*m[1][1] + n[1][2]*m[2][1];
	d[1].z = n[1][0]*m[0][2] + n[1][1]*m[1][2] + n[1][2]*m[2][2];
	d[1].w = n[1][0]*m[0][3] + n[1][1]*m[1][3] + n[1][2]*m[2][3] + n[1][3];
	d[2].x = n[2][0]*m[0][0] + n[2][1]*m[1][0] + n[2][2]*m[2][0];
	d[2].y = n[2][0]*m[0][1] + n[2][1]*m[1][1] + n[2][2]*m[2][1];
	d[2].z = n[2][0]*m[0][2] + n[2][1]*m[1][2] + n[2][2]*m[2][2];
	d[2].w = n[2][0]*m[0][3] + n[2][1]*m[1][3] + n[2][2]*m[2][3] + n[2][3];
	d[3].x = 0.0f;
	d[3].y = 0.0f;
	d[3].z = 0.0f;
	d[3].w = 1.0f;
*/
#endif
}

/******************************************************************************
 *
 * Function:		DPVS::Math::productFromLeft()
 *
 * Description:
 *
 * Parameters:		d = reference to destination matrix
 *					m = reference to first source matrix  (may not be d)
 *					n = reference to second source matrix (may not be d)
 *
 ******************************************************************************/

void productFromLeft(Matrix4x4& d, const Matrix4x4& m, const Matrix4x4& n)
{
	DPVS_ASSERT ((&d != &n) && (&d != &m));

#if defined (DPVS_PS2_VU_ASSEMBLY)
//PS2-BEGIN
	DPVS_ASSERT( is128Aligned(&n) );
	DPVS_ASSERT( is128Aligned(&m) );
	DPVS_ASSERT( is128Aligned(&d) );

	asm __volatile__ (
	"	lqc2			vf11, 0x0(%0)"
	"\n	lqc2			vf12, 0x10(%0)"
	"\n	lqc2			vf13, 0x20(%0)"
	"\n	lqc2			vf14, 0x30(%0)"
	"\n	lqc2			vf15, 0x0(%1)"
	"\n	lqc2			vf16, 0x10(%1)"
	"\n	lqc2			vf17, 0x20(%1)"
	"\n	lqc2			vf18, 0x30(%1)"
	"\n	vmulax.xyzw		ACC, vf11, vf15"
	"\n	vmadday.xyzw	ACC, vf12, vf15"
	"\n	vmaddaz.xyzw	ACC, vf13, vf15"
	"\n	vmaddw.xyzw		vf1, vf14, vf15"
	"\n	sqc2			vf1, 0x0(%2)"
	"\n	vmulax.xyzw		ACC, vf11, vf16"
	"\n	vmadday.xyzw	ACC, vf12, vf16"
	"\n	vmaddaz.xyzw	ACC, vf13, vf16"
	"\n	vmaddw.xyzw		vf1, vf14, vf16"
	"\n	sqc2			vf1, 0x10(%2)"
	"\n	vmulax.xyzw		ACC, vf11, vf17"
	"\n	vmadday.xyzw	ACC, vf12, vf17"
	"\n	vmaddaz.xyzw	ACC, vf13, vf17"
	"\n	vmaddw.xyzw		vf1, vf14, vf17"
	"\n	sqc2			vf1, 0x20(%2)"
	"\n	vmulax.xyzw		ACC, vf11, vf18"
	"\n	vmadday.xyzw	ACC, vf12, vf18"
	"\n	vmaddaz.xyzw	ACC, vf13, vf18"
	"\n	vmaddw.xyzw		vf1, vf14, vf18"
	"\n	sqc2			vf1, 0x30(%2)"
	 : : "r"(&m), "r"(&n), "r"(&d) );
//PS2-END
#else

	float m00 = m[0][0], m01 = m[0][1], m02 = m[0][2], m03 = m[0][3];
	float m10 = m[1][0], m11 = m[1][1], m12 = m[1][2], m13 = m[1][3];
	float m20 = m[2][0], m21 = m[2][1], m22 = m[2][2], m23 = m[2][3];
	float m30 = m[3][0], m31 = m[3][1], m32 = m[3][2], m33 = m[3][3];

	for (int i = 0; i < 4; i++)
	{
		float n00 = n[i][0], n01 = n[i][1], n02 = n[i][2], n03 = n[i][3];

		d[i].x = n00*m00 + n01*m10 + n02*m20 + n03*m30;
		d[i].y = n00*m01 + n01*m11 + n02*m21 + n03*m31;
		d[i].z = n00*m02 + n01*m12 + n02*m22 + n03*m32;
		d[i].w = n00*m03 + n01*m13 + n02*m23 + n03*m33;
	}

#endif
}

/******************************************************************************
 *
 * Function:		DPVS::Math::productFromLeft()
 *
 * Description:
 *
 * Parameters:		d = reference to destination matrix
 *					m = reference to first source matrix  (may not be d)
 *					n = reference to second source matrix (may not be d)
 *
 ******************************************************************************/

void productFromLeft(Matrix4x4& d, const Matrix4x3& m, const Matrix4x4& n)
{
	DPVS_ASSERT ((&d != &n));

#if defined (DPVS_PS2_VU_ASSEMBLY)
//PS2-BEGIN

	DPVS_ASSERT( is128Aligned(&n) );
	DPVS_ASSERT( is128Aligned(&m) );
	DPVS_ASSERT( is128Aligned(&d) );

	asm __volatile__ (
	"	lqc2			vf11, 0x0(%0)"
	"\n	lqc2			vf12, 0x10(%0)"
	"\n	lqc2			vf13, 0x20(%0)"
	"\n	vmove.xyzw		vf14, vf0"
	"\n	lqc2			vf15, 0x0(%1)"
	"\n	lqc2			vf16, 0x10(%1)"
	"\n	lqc2			vf17, 0x20(%1)"
	"\n	lqc2			vf18, 0x30(%1)"
	"\n	vmulax.xyzw		ACC, vf11, vf15"
	"\n	vmadday.xyzw	ACC, vf12, vf15"
	"\n	vmaddaz.xyzw	ACC, vf13, vf15"
	"\n	vmaddw.xyzw		vf1, vf14, vf15"
	"\n	sqc2			vf1, 0x0(%2)"
	"\n	vmulax.xyzw		ACC, vf11, vf16"
	"\n	vmadday.xyzw	ACC, vf12, vf16"
	"\n	vmaddaz.xyzw	ACC, vf13, vf16"
	"\n	vmaddw.xyzw		vf1, vf14, vf16"
	"\n	sqc2			vf1, 0x10(%2)"
	"\n	vmulax.xyzw		ACC, vf11, vf17"
	"\n	vmadday.xyzw	ACC, vf12, vf17"
	"\n	vmaddaz.xyzw	ACC, vf13, vf17"
	"\n	vmaddw.xyzw		vf1, vf14, vf17"
	"\n	sqc2			vf1, 0x20(%2)"
	"\n	vmulax.xyzw		ACC, vf11, vf18"
	"\n	vmadday.xyzw	ACC, vf12, vf18"
	"\n	vmaddaz.xyzw	ACC, vf13, vf18"
	"\n	vmaddw.xyzw		vf1, vf14, vf18"
	"\n	sqc2			vf1, 0x30(%2)"
	 : : "r"(&m), "r"(&n), "r"(&d) );
//PS2-END
#else

	float n00 = n[0][0], n01 = n[0][1], n02 = n[0][2];
	float m00 = m[0][0], m01 = m[0][1], m02 = m[0][2], m03 = m[0][3];
	float m10 = m[1][0], m11 = m[1][1], m12 = m[1][2], m13 = m[1][3];
	float m20 = m[2][0], m21 = m[2][1], m22 = m[2][2], m23 = m[2][3];
//	float m30 = m[3][0], m31 = m[3][1], m32 = m[3][2], m33 = m[3][3];

	d[0].x = n00*m00 + n01*m10 + n02*m20;
	d[0].y = n00*m01 + n01*m11 + n02*m21;
	d[0].z = n00*m02 + n01*m12 + n02*m22;
	d[0].w = n00*m03 + n01*m13 + n02*m23 + n[0][3];

	float n10 = n[1][0], n11 = n[1][1], n12 = n[1][2];

	d[1].x = n10*m00 + n11*m10 + n12*m20;
	d[1].y = n10*m01 + n11*m11 + n12*m21;
	d[1].z = n10*m02 + n11*m12 + n12*m22;
	d[1].w = n10*m03 + n11*m13 + n12*m23 + n[1][3];

	float n20 = n[2][0], n21 = n[2][1], n22 = n[2][2];

	d[2].x = n20*m00 + n21*m10 + n22*m20;
	d[2].y = n20*m01 + n21*m11 + n22*m21;
	d[2].z = n20*m02 + n21*m12 + n22*m22;
	d[2].w = n20*m03 + n21*m13 + n22*m23 + n[2][3];

	float n30 = n[3][0], n31 = n[3][1], n32 = n[3][2];

	d[3].x = n30*m00 + n31*m10 + n32*m20;
	d[3].y = n30*m01 + n31*m11 + n32*m21;
	d[3].z = n30*m02 + n31*m12 + n32*m22;
	d[3].w = n30*m03 + n31*m13 + n32*m23 + n[3][3];
#endif
}

/******************************************************************************
 *
 * Function:		DPVS::Math::productFromLeftProjection()
 *
 * Description:		Performs product-from-left when we know that b
 *					is a projection matrix
 *
 * Parameters:		d = reference to destination matrix
 *					a = reference to first source matrix
 *					b = reference to second source matrix (projection matrix!)
 *
 * Notes:			Assumes d != b
 *
 *					The routine works for both perspective and ortho projection
 *					matrices.
 *
 ******************************************************************************/
// assumes b = perspective projection matrix (see assertions below)
// assumes d != b

void productFromLeftProjection(Matrix4x4& d, const Matrix4x3& a, const Matrix4x4& b)
{
	DPVS_ASSERT(&d != &b);

#if defined (DPVS_PS2_VU_ASSEMBLY)
//PS2-BEGIN

	DPVS_ASSERT( is128Aligned(&d) );
	DPVS_ASSERT( is128Aligned(&a) );
	DPVS_ASSERT( is128Aligned(&b) );

	asm __volatile__ (
	"   lqc2			vf11, 0x0(%0)"
	"\n	lqc2			vf12, 0x10(%0)"
	"\n	lqc2			vf13, 0x20(%0)"
	"\n	vmove.xyzw		vf14, vf0"
	"\n	lqc2			vf15, 0x0(%1)"
	"\n	lqc2			vf16, 0x10(%1)"
	"\n	lqc2			vf17, 0x20(%1)"
	"\n	lqc2			vf18, 0x30(%1)"
	"\n	vmulax.xyzw		ACC, vf11, vf15"
	"\n	vmadday.xyzw	ACC, vf12, vf15"
	"\n	vmaddaz.xyzw	ACC, vf13, vf15"
	"\n	vmaddw.xyzw		vf1, vf14, vf15"
	"\n	sqc2			vf1, 0x0(%2)"
	"\n	vmulax.xyzw		ACC, vf11, vf16"
	"\n	vmadday.xyzw	ACC, vf12, vf16"
	"\n	vmaddaz.xyzw	ACC, vf13, vf16"
	"\n	vmaddw.xyzw		vf1, vf14, vf16"
	"\n	sqc2			vf1, 0x10(%2)"
	"\n	vmulax.xyzw		ACC, vf11, vf17"
	"\n	vmadday.xyzw	ACC, vf12, vf17"
	"\n	vmaddaz.xyzw	ACC, vf13, vf17"
	"\n	vmaddw.xyzw		vf1, vf14, vf17"
	"\n	sqc2			vf1, 0x20(%2)"
	"\n	vmulax.xyzw		ACC, vf11, vf18"
	"\n	vmadday.xyzw	ACC, vf12, vf18"
	"\n	vmaddaz.xyzw	ACC, vf13, vf18"
	"\n	vmaddw.xyzw		vf1, vf14, vf18"
	"\n	sqc2			vf1, 0x30(%2)"
	 : : "r"(&a), "r"(&b), "r"(&d) );
//PS2-END
#else

	if (isZero(b[3][3]))	// perspective projection
	{
		DPVS_ASSERT(Math::isPerspectiveProjectionMatrix(b));

		float a00 = a[0][0], a01 = a[0][1], a02 = a[0][2], a03 = a[0][3];
		float a10 = a[1][0], a11 = a[1][1], a12 = a[1][2], a13 = a[1][3];
		float a20 = a[2][0], a21 = a[2][1], a22 = a[2][2], a23 = a[2][3];
//		float a30 = a[3][0], a31 = a[3][1], a32 = a[3][2], a33 = a[3][3];

		float b00 = b[0][0], b02 = b[0][2];

		d[0].x = b00*a00 + b02*a20;
		d[0].y = b00*a01 + b02*a21;
		d[0].z = b00*a02 + b02*a22;
		d[0].w = b00*a03 + b02*a23;

		float b11 = b[1][1], b12 = b[1][2];

		d[1].x = b11*a10 + b12*a20;
		d[1].y = b11*a11 + b12*a21;
		d[1].z = b11*a12 + b12*a22;
		d[1].w = b11*a13 + b12*a23;

		float b22 = b[2][2];

		d[2].x = b22*a20;
		d[2].y = b22*a21;
		d[2].z = b22*a22;
		d[2].w = b22*a23 + b[2][3];

//		float b30 = b[3][0], b31 = b[3][1], b32 = b[3][2];

		d[3].x = a20;
		d[3].y = a21;
		d[3].z = a22;
		d[3].w = a23;
/*
		d[0][0] = b[0][0]*a[0][0] + b[0][2]*a[2][0];
		d[0][1] = b[0][0]*a[0][1] + b[0][2]*a[2][1];
		d[0][2] = b[0][0]*a[0][2] + b[0][2]*a[2][2];
		d[0][3] = b[0][0]*a[0][3] + b[0][2]*a[2][3];

		d[1][0] = b[1][1]*a[1][0] + b[1][2]*a[2][0];
		d[1][1] = b[1][1]*a[1][1] + b[1][2]*a[2][1];
		d[1][2] = b[1][1]*a[1][2] + b[1][2]*a[2][2];
		d[1][3] = b[1][1]*a[1][3] + b[1][2]*a[2][3];

		d[2][0] = b[2][2]*a[2][0];
		d[2][1] = b[2][2]*a[2][1];
		d[2][2] = b[2][2]*a[2][2];
		d[2][3] = b[2][2]*a[2][3] + b[2][3];

		d[3][0] = a[2][0];
		d[3][1] = a[2][1];
		d[3][2] = a[2][2];
		d[3][3] = a[2][3];
*/
/*
#if defined (DPVS_DEBUG)
		Matrix4x4 v;
		productFromLeft(v,a,b);
		DPVS_ASSERT( v == d);
#endif
*/
	} else
	{
		// DEBUG DEBUG HERE WE COULD ENSURE IT IS A VALID ORTHO PROJECTION MATRIX

		productFromLeft(d,a,b);		// use generic productFromLeft
	}

#endif
}

/******************************************************************************
 *
 * Function:		DPVS::Math::unitBoxToScreen()
 *
 * Description:
 *
 *					The routine works for both perspective and ortho projection
 *					matrices.
 *
 ******************************************************************************/

// assumes c == projection matrix (see assertions below)
void unitBoxToScreen (Matrix4x4& v, const Matrix4x3& a, const Matrix4x3& b, const Matrix4x4& c)
{
#if defined (DPVS_PS2_VU_ASSEMBLY)
//PS2-BEGIN

	DPVS_ASSERT( is128Aligned(&v) );
	DPVS_ASSERT( is128Aligned(&a) );
	DPVS_ASSERT( is128Aligned(&b) );
	DPVS_ASSERT( is128Aligned(&c) );

	// c * (b * a)
	asm __volatile__ (
	"	lqc2			vf11, 0x0(%0)"
	"\n	lqc2			vf12, 0x10(%0)"
	"\n	lqc2			vf13, 0x20(%0)"
	"\n	lqc2			vf15, 0x0(%1)"
	"\n	lqc2			vf16, 0x10(%1)"
	"\n	lqc2			vf17, 0x20(%1)"
	"\n	vmulax.xyzw		ACC, vf11, vf15"
	"\n	vmadday.xyzw	ACC, vf12, vf15"
	"\n	vmaddaz.xyzw	ACC, vf13, vf15"
	"\n	vmaddw.xyzw		vf21, vf0, vf15"
	"\n	vmulax.xyzw		ACC, vf11, vf16"
	"\n	vmadday.xyzw	ACC, vf12, vf16"
	"\n	vmaddaz.xyzw	ACC, vf13, vf16"
	"\n	vmaddw.xyzw		vf22, vf0, vf16"
	"\n	vmulax.xyzw		ACC, vf11, vf17"
	"\n	vmadday.xyzw	ACC, vf12, vf17"
	"\n	vmaddaz.xyzw	ACC, vf13, vf17"
	"\n	vmaddw.xyzw		vf23, vf0, vf17"
	"\n	vmove.xyzw		vf24, vf0"
	"\n	lqc2			vf15, 0x0(%3)"
	"\n	lqc2			vf16, 0x10(%3)"
	"\n	lqc2			vf17, 0x20(%3)"
	"\n	lqc2			vf18, 0x30(%3)"
	"\n	vmulax.xyzw		ACC, vf21, vf15"
	"\n	vmadday.xyzw	ACC, vf22, vf15"
	"\n	vmaddaz.xyzw	ACC, vf23, vf15"
	"\n	vmaddw.xyzw		vf1, vf24, vf15"
	"\n	sqc2			vf1, 0x0(%2)"
	"\n	vmulax.xyzw		ACC, vf21, vf16"
	"\n	vmadday.xyzw	ACC, vf22, vf16"
	"\n	vmaddaz.xyzw	ACC, vf23, vf16"
	"\n	vmaddw.xyzw		vf1, vf24, vf16"
	"\n	sqc2			vf1, 0x10(%2)"
	"\n	vmulax.xyzw		ACC, vf21, vf17"
	"\n	vmadday.xyzw	ACC, vf22, vf17"	
	"\n	vmaddaz.xyzw	ACC, vf23, vf17"
	"\n	vmaddw.xyzw		vf1, vf24, vf17"
	"\n	sqc2			vf1, 0x20(%2)"
	"\n	vmulax.xyzw		ACC, vf21, vf18"
	"\n	vmadday.xyzw	ACC, vf22, vf18"
	"\n	vmaddaz.xyzw	ACC, vf23, vf18"
	"\n	vmaddw.xyzw		vf1, vf24, vf18"
	"\n	sqc2			vf1, 0x30(%2)"
	: : "r"(&a), "r"(&b), "r"(&v), "r"(&c) );
//PS2-END
#else

	if (isZero(c[3][3]))
	{
		DPVS_ASSERT(Math::isPerspectiveProjectionMatrix(c));

		float a00 = a[0][0], a01 = a[0][1], a02 = a[0][2], a03 = a[0][3];
		float a10 = a[1][0], a11 = a[1][1], a12 = a[1][2], a13 = a[1][3];
		float a20 = a[2][0], a21 = a[2][1], a22 = a[2][2], a23 = a[2][3];

		float u00 = b[0][0]*a00 + b[0][1]*a10 + b[0][2]*a20;
		float u01 = b[0][0]*a01 + b[0][1]*a11 + b[0][2]*a21;
		float u02 = b[0][0]*a02 + b[0][1]*a12 + b[0][2]*a22;
		float u03 = b[0][0]*a03 + b[0][1]*a13 + b[0][2]*a23 + b[0][3];
		float u10 = b[1][0]*a00 + b[1][1]*a10 + b[1][2]*a20;
		float u11 = b[1][0]*a01 + b[1][1]*a11 + b[1][2]*a21;
		float u12 = b[1][0]*a02 + b[1][1]*a12 + b[1][2]*a22;
		float u13 = b[1][0]*a03 + b[1][1]*a13 + b[1][2]*a23 + b[1][3];

		float b20 = b[2][0], b21 = b[2][1], b22 = b[2][2];

		float v30 = b20*a00 + b21*a10 + b22*a20;
		float v31 = b20*a01 + b21*a11 + b22*a21;
		float v32 = b20*a02 + b21*a12 + b22*a22;
		float v33 = b20*a03 + b21*a13 + b22*a23 + b[2][3];

		v[3][0] = v30;
		v[3][1] = v31;
		v[3][2] = v32;
		v[3][3] = v33;

		float c00 = c[0][0], c02 = c[0][2];

		v[0][0] = c00*u00 + c02*v30;
		v[0][1] = c00*u01 + c02*v31;
		v[0][2] = c00*u02 + c02*v32;
		v[0][3] = c00*u03 + c02*v33;

		float c11 = c[1][1], c12 = c[1][2];

		v[1][0] = c11*u10 + c12*v30;
		v[1][1] = c11*u11 + c12*v31;
		v[1][2] = c11*u12 + c12*v32;
		v[1][3] = c11*u13 + c12*v33;

		float c22 = c[2][2];

		v[2][0] = c22*v30;
		v[2][1] = c22*v31;
		v[2][2] = c22*v32;
		v[2][3] = c22*v33 + c[2][3];

/*		DEBUG DEBUG GameCube - original version
		v[3][0] = b[2][0]*a[0][0] + b[2][1]*a[1][0] + b[2][2]*a[2][0];
		v[3][1] = b[2][0]*a[0][1] + b[2][1]*a[1][1] + b[2][2]*a[2][1];
		v[3][2] = b[2][0]*a[0][2] + b[2][1]*a[1][2] + b[2][2]*a[2][2];
		v[3][3] = b[2][0]*a[0][3] + b[2][1]*a[1][3] + b[2][2]*a[2][3] + b[2][3];
		v[0][0] = c[0][0]*u00     + c[0][2]*v[3][0];
		v[0][1] = c[0][0]*u01	  + c[0][2]*v[3][1];
		v[0][2] = c[0][0]*u02	  + c[0][2]*v[3][2];
		v[0][3] = c[0][0]*u03	  + c[0][2]*v[3][3];
		v[1][0] = c[1][1]*u10	  + c[1][2]*v[3][0];
		v[1][1] = c[1][1]*u11	  + c[1][2]*v[3][1];
		v[1][2] = c[1][1]*u12	  + c[1][2]*v[3][2];
		v[1][3] = c[1][1]*u13	  + c[1][2]*v[3][3];
		v[2][0] = c[2][2]*v[3][0];
		v[2][1] = c[2][2]*v[3][1];
		v[2][2] = c[2][2]*v[3][2];
		v[2][3] = c[2][2]*v[3][3] + c[2][3];
*/
/*
#if defined (DPVS_DEBUG)
		Matrix4x4 d;
		productFromLeft(d,a,b);
		d.productFromLeft(c);
		DPVS_ASSERT( d == v);
#endif
*/
	} else
	{
		// DEBUG DEBUG HERE WE COULD ENSURE IT IS A VALID ORTHO PROJECTION MATRIX
		productFromLeft(v,a,b);
		v.productFromLeft(c);
	}

#endif
}

/******************************************************************************
 *
 * Function:		DPVS::Math::divFixed32_16()
 *
 * Description:		Performs fixed point division of values 'a', 'b' returns
 *					result in 16.16 fixed point
 *
 * Parameters:		a = first integer
 *					b = second integer
 *
 * Returns:			(a/b) in 16.16 fixed point
 *
 * Notes:			This function isn't called anymore that often. It is only
 *					needed for very large screen resolutions (>=2048 pixels).
 *
 ******************************************************************************/

int divFixed32_16 (int a, int b)
{
	DPVS_ASSERT(b!=0);										// division by zero

#if defined (DPVS_X86_ASSEMBLY)

	int retval;

	DPVS_ASM												// perform 64-bit division
	{
		mov		eax,[a]
		mov		ebx,[b]
		mov		edx,eax
		sar		edx,31
		shld	edx,eax,16
		shl		eax,16
		idiv	ebx
		mov		[retval],eax
	}
#if defined (DPVS_DEBUG)
	int check = (INT32)(((__int64)(a)*65536)/b);			// just make sure our asm code works
	DPVS_ASSERT( check == retval);
#endif
	return retval;

#elif defined (DPVS_UINT64_DEFINED)						// if machine supports 64-bit instructions?
	return (INT32)(((INT64)(a)*65536)/b);
#else														// d'oh - must use double-precision math
	return (int)((double)(a*65536.0)/(double)(b));
#endif
}

/******************************************************************************
 *
 * Function:		DPVS::Math::transformAABB()
 *
 * Description:		Transforms an axis-aligned bounding box with specified matrix
 *
 * Parameters:		dst = destination vectors (homogenous)
 *					mn	= AABB top-left corner
 *					mx  = AABB bottom-right corner
 *					m	= transformation matrix
 *
 * Notes:			Output order:
 *					X   Y   Z
 *					0 = neg,neg,neg
 *					1 = pos,neg,neg
 *					2 = neg,pos,neg
 *					3 = pos,pos,neg
 *					4 = neg,neg,pos
 *					5 = pos,neg,pos
 *					6 = neg,pos,pos
 *					7 = pos,pos,pos
 *
 ******************************************************************************/
/*
void transformAABB (Vector4 dst[8], const Vector3& mn, const Vector3& mx, const Matrix4x4& m)
{
	register float a0,a1,a2,a3;

	dst[0].x = mn.x * m[0].x + mn.y * m[0].y + mn.z * m[0].z + m[0].w;
	dst[0].y = mn.x * m[1].x + mn.y * m[1].y + mn.z * m[1].z + m[1].w;
	dst[0].z = mn.x * m[2].x + mn.y * m[2].y + mn.z * m[2].z + m[2].w;
	dst[0].w = mn.x * m[3].x + mn.y * m[3].y + mn.z * m[3].z + m[3].w;

	register float y = (mx.y-mn.y);

	dst[2].x = dst[0].x + y * m[0].y;
	dst[2].y = dst[0].y + y * m[1].y;
	dst[2].z = dst[0].z + y * m[2].y;
	dst[2].w = dst[0].w + y * m[3].y;

	register float x = (mx.x-mn.x);
	a0 = x * m[0].x;
	a1 = x * m[1].x;
	a2 = x * m[2].x;
	a3 = x * m[3].x;

	dst[1].x = dst[0].x + a0;
	dst[1].y = dst[0].y + a1;
	dst[1].z = dst[0].z + a2;
	dst[1].w = dst[0].w + a3;

	dst[3].x = dst[2].x + a0;
	dst[3].y = dst[2].y + a1;
	dst[3].z = dst[2].z + a2;
	dst[3].w = dst[2].w + a3;

	register float z = (mx.z-mn.z);
	a0 = z * m[0].z;
	a1 = z * m[1].z;
	a2 = z * m[2].z;
	a3 = z * m[3].z;

	dst[4].x = dst[0].x + a0;
	dst[4].y = dst[0].y + a1;
	dst[4].z = dst[0].z + a2;
	dst[4].w = dst[0].w + a3;

	dst[5].x = dst[1].x + a0;
	dst[5].y = dst[1].y + a1;
	dst[5].z = dst[1].z + a2;
	dst[5].w = dst[1].w + a3;

	dst[6].x = dst[2].x + a0;
	dst[6].y = dst[2].y + a1;
	dst[6].z = dst[2].z + a2;
	dst[6].w = dst[2].w + a3;

	dst[7].x = dst[3].x + a0;
	dst[7].y = dst[3].y + a1;
	dst[7].z = dst[3].z + a2;
	dst[7].w = dst[3].w + a3;
}
*/

/******************************************************************************
 *
 * Function:		DPVS::Math::transformAABBMinMax()
 *
 * Description:		Transforms AABB with matrix 'm', then returns the
 *					extents of the transformed box into dmn and dmx
 *
 * Parameters:		dmn = minimum vector of output AABB
 *					dmx = maximum vector of output AABB
 *					mn  = minimum vector of input AABB
 *					mx	= maximum vector of input AABB
 *					m	= transformation matrix
 *
 ******************************************************************************/

static DPVS_FORCE_INLINE void transformAABBMinMaxReal (Vector3& dmn, Vector3& dmx, const Vector3& mn, const Vector3& mx, const Matrix4x3& m)
{
	float x  = (mx.x-mn.x);
	float y  = (mx.y-mn.y);
	float z  = (mx.z-mn.z);

	{
		float ax    = x * m[0].x;
		float bx    = z * m[0].z;
		float dst0x = mn.x * m[0].x + mn.y * m[0].y + mn.z * m[0].z + m[0].w;
		float dst1x = dst0x + ax;
		float dst2x = dst0x + y * m[0].y;
		float dst3x = dst2x + ax;
		float dst4x = dst0x + bx;
		float dst5x = dst1x + bx;
		float dst6x = dst2x + bx;
		float dst7x = dst3x + bx;

		float dmnx = Math::min(dst0x,dst1x);
		float dmxx = Math::max(dst0x,dst1x);

		dmnx = Math::min(dmnx,dst2x);
		dmxx = Math::max(dmxx,dst2x);
		dmnx = Math::min(dmnx,dst3x);
		dmxx = Math::max(dmxx,dst3x);
		dmnx = Math::min(dmnx,dst4x);
		dmxx = Math::max(dmxx,dst4x);
		dmnx = Math::min(dmnx,dst5x);
		dmxx = Math::max(dmxx,dst5x);
		dmnx = Math::min(dmnx,dst6x);
		dmxx = Math::max(dmxx,dst6x);
		dmnx = Math::min(dmnx,dst7x);
		dmxx = Math::max(dmxx,dst7x);

		dmn.x = dmnx;
		dmx.x = dmxx;
	}

	{
		float by    = z * m[1].z;
		float ay    = x * m[1].x;
		float dst0y = mn.x * m[1].x + mn.y * m[1].y + mn.z * m[1].z + m[1].w;
		float dst1y = dst0y + ay;
		float dst2y = dst0y + y * m[1].y;
		float dst3y = dst2y + ay;
		float dst4y = dst0y + by;
		float dst5y = dst1y + by;
		float dst6y = dst2y + by;
		float dst7y = dst3y + by;

		float dmny = Math::min(dst0y,dst1y);
		float dmxy = Math::max(dst0y,dst1y);

		dmny = Math::min(dmny,dst2y);
		dmxy = Math::max(dmxy,dst2y);
		dmny = Math::min(dmny,dst3y);
		dmxy = Math::max(dmxy,dst3y);
		dmny = Math::min(dmny,dst4y);
		dmxy = Math::max(dmxy,dst4y);
		dmny = Math::min(dmny,dst5y);
		dmxy = Math::max(dmxy,dst5y);
		dmny = Math::min(dmny,dst6y);
		dmxy = Math::max(dmxy,dst6y);
		dmny = Math::min(dmny,dst7y);
		dmxy = Math::max(dmxy,dst7y);

		dmn.y = dmny;
		dmx.y = dmxy;

	}
	{
		float bz	 = z * m[2].z;
		float az	 = x * m[2].x;
		float dst0z = mn.x * m[2].x + mn.y * m[2].y + mn.z * m[2].z + m[2].w;
		float dst1z = dst0z + az;
		float dst2z = dst0z + y * m[2].y;
		float dst3z = dst2z + az;
		float dst4z = dst0z + bz;
		float dst5z = dst1z + bz;
		float dst6z = dst2z + bz;
		float dst7z = dst3z + bz;

		float dmnz = Math::min(dst0z,dst1z);
		float dmxz = Math::max(dst0z,dst1z);

		dmnz = Math::min(dmnz,dst2z);
		dmxz = Math::max(dmxz,dst2z);
		dmnz = Math::min(dmnz,dst3z);
		dmxz = Math::max(dmxz,dst3z);
		dmnz = Math::min(dmnz,dst4z);
		dmxz = Math::max(dmxz,dst4z);
		dmnz = Math::min(dmnz,dst5z);
		dmxz = Math::max(dmxz,dst5z);
		dmnz = Math::min(dmnz,dst6z);
		dmxz = Math::max(dmxz,dst6z);
		dmnz = Math::min(dmnz,dst7z);
		dmxz = Math::max(dmxz,dst7z);

		dmn.z = dmnz;
		dmx.z = dmxz;

	}
}

/* "foobar box" */
void transformAABBMinMax (Vector3& dmn, Vector3& dmx, const Vector3& mn, const Vector3& mx, const Matrix4x3& m)
{
	// Perform incremental transformations and then local minmax operations

#if defined (DPVS_PS2_VU_ASSEMBLY)
//PS2-BEGIN

	DPVS_ASSERT( is128Aligned(&m) );

	float DPVS_VECTOR_ALIGN(mn4[4]);
	float DPVS_VECTOR_ALIGN(mx4[4]);
	float DPVS_VECTOR_ALIGN(tmtx[16]);

	asm __volatile__ (
	"	lw $8, 0(%6)"
	"\n	sw $8, 0(%2)"
	"\n	lw $8, 4(%6)"
	"\n	sw $8, 4(%2)"
	"\n	lw $8, 8(%6)"
	"\n	sw $8, 8(%2)"
	"\n	swc1 %8, 12(%2)"
	"\n	lw $8, 0(%7)"
	"\n	sw $8, 0(%3)"
	"\n	lw $8, 4(%7)"
	"\n	sw $8, 4(%3)"
	"\n	lw $8, 8(%7)"
	"\n	sw $8, 8(%3)"
	"\n	swc1 %8, 12(%3)"
	"\n	lw $8, 0x00(%4)"
	"\n	sw $8, 0x00(%5)"
	"\n	lw $8, 0x10(%4)"
	"\n	sw $8, 0x04(%5)"
	"\n	lw $8, 0x20(%4)"
	"\n	sw $8, 0x08(%5)"
	"\n	addi $8, $0, 0"
	"\n	sw $8, 0x0c(%5)"
	"\n	lw $8, 0x04(%4)"
	"\n	sw $8, 0x10(%5)"
	"\n	lw $8, 0x14(%4)"
	"\n	sw $8, 0x14(%5)"
	"\n	lw $8, 0x24(%4)"
	"\n	sw $8, 0x18(%5)"
	"\n	addi $8, $0, 0"
	"\n	sw $8, 0x1c(%5)"
	"\n	lw $8, 0x08(%4)"
	"\n	sw $8, 0x20(%5)"
	"\n	lw $8, 0x18(%4)"
	"\n	sw $8, 0x24(%5)"
	"\n	lw $8, 0x28(%4)"
	"\n	sw $8, 0x28(%5)"
	"\n	addi $8, $0, 0"
	"\n	sw $8, 0x2c(%5)"
	"\n	lw $8, 0x0c(%4)"
	"\n	sw $8, 0x30(%5)"
	"\n	lw $8, 0x1c(%4)"
	"\n	sw $8, 0x34(%5)"
	"\n	lw $8, 0x2c(%4)"
	"\n	sw $8, 0x38(%5)"
	"\n	addi $8, $0, 1"
	"\n	sw $8, 0x3c(%5)"
	"\n	lqc2		vf11, 0(%5)"
	"\n	lqc2		vf12, 0x10(%5)"
	"\n	lqc2		vf13, 0x20(%5)"
	"\n	lqc2		vf14, 0x30(%5)"
	"\n	lqc2		vf15, 0(%2)"
	"\n	lqc2		vf16, 0(%3)"
	"\n	vmulax.xyzw		ACC, vf11, vf15"
	"\n	vmadday.xyzw	ACC, vf12, vf15"
	"\n	vmaddaz.xyzw	ACC, vf13, vf15"
	"\n	vmaddw.xyzw		vf25, vf14, vf15"
	"\n	vmulax.xyzw		ACC, vf11, vf16"
	"\n	vmadday.xyzw	ACC, vf12, vf16"
	"\n	vmaddaz.xyzw	ACC, vf13, vf16"
	"\n	vmaddw.xyzw		vf26, vf14, vf16"
	"\n	vmini.xyz		vf21, vf25, vf26"
	"\n	vmax.xyz		vf22, vf25, vf26"
	"\n	vmr32.xyzw		vf1, vf21"
	"\n	vmr32.xyzw		vf2, vf1"
	"\n	qmfc2			$8, vf21"
	"\n	sw				$8, 0(%0)"
	"\n	qmfc2			$8, vf1"
	"\n	sw				$8, 4(%0)"
	"\n	qmfc2			$8, vf2"
	"\n	sw				$8, 8(%0)"
	"\n	vmr32.xyzw		vf1, vf22"
	"\n	vmr32.xyzw		vf2, vf1"
	"\n	qmfc2			$8, vf22"
	"\n	sw				$8, 0(%1)"
	"\n	qmfc2			$8, vf1"
	"\n	sw				$8, 4(%1)"
	"\n	qmfc2			$8, vf2"
	"\n	sw				$8, 8(%1)"
	 : : "r"(&dmn), "r"(&dmx), "r"(mn4), "r"(mx4), "r"(&m), "r"(tmtx), "r"(&mn), "r"(&mx), "f"(1.f) : "$8" );

	/* foobar */
/*
	#if defined (DPVS_DEBUG)
		Vector3 cmn,cmx;
		transformAABBMinMaxReal (cmn,cmx,mn,mx,m);
		DPVS_ASSERT(isApproximatelyEqual(dmn,cmn));
		DPVS_ASSERT(isApproximatelyEqual(dmx,cmx));
	#endif
*/
//PS2-END
#else
	transformAABBMinMaxReal (dmn,dmx,mn,mx,m);
#endif
}


/******************************************************************************
 *
 * Function:		DPVS::Math::transformUnitBox()
 *
 * Description:		Transforms unit box (i.e. (-1,-1,-1),(+1,+1,+1)) with
 *					specified matrix
 *
 * Parameters:		dst = destination vectors (homogenous)
 *					m	= transformation matrix
 *
 * Notes:			Output order:
 *					X   Y   Z
 *					0 = neg,neg,neg
 *					1 = pos,neg,neg
 *					2 = neg,pos,neg
 *					3 = pos,pos,neg
 *					4 = neg,neg,pos
 *					5 = pos,neg,pos
 *					6 = neg,pos,pos
 *					7 = pos,pos,pos
 *
 *					The routine performs the transformation incrementally,
 *					without any multiplications.
 *
 ******************************************************************************/

void transformUnitBox (Vector3 dst[8], const Matrix4x3& m)
{
	float dst0x = m[0].w - (m[0].x + m[0].y + m[0].z);
	float dst0y = m[1].w - (m[1].x + m[1].y + m[1].z);
	float dst0z = m[2].w - (m[2].x + m[2].y + m[2].z);

	float a0 = m[0].x + m[0].x;
	float a1 = m[1].x + m[1].x;
	float a2 = m[2].x + m[2].x;

	float dst2x = dst0x + m[0].y + m[0].y;
	float dst2y = dst0y + m[1].y + m[1].y;
	float dst2z = dst0z + m[2].y + m[2].y;

	float dst1x = dst0x + a0;
	float dst1y = dst0y + a1;
	float dst1z = dst0z + a2;

	float dst3x = dst2x + a0;
	float dst3y = dst2y + a1;
	float dst3z = dst2z + a2;

	float b0 = m[0].z + m[0].z;
	float b1 = m[1].z + m[1].z;
	float b2 = m[2].z + m[2].z;

	dst[0].x = dst0x; dst[0].y = dst0y; dst[0].z = dst0z;
	dst[1].x = dst1x; dst[1].y = dst1y; dst[1].z = dst1z;
	dst[2].x = dst2x; dst[2].y = dst2y; dst[2].z = dst2z;
	dst[3].x = dst3x; dst[3].y = dst3y; dst[3].z = dst3z;

	dst[4].x = dst0x + b0;
	dst[4].y = dst0y + b1;
	dst[4].z = dst0z + b2;

	dst[5].x = dst1x + b0;
	dst[5].y = dst1y + b1;
	dst[5].z = dst1z + b2;

	dst[6].x = dst2x + b0;
	dst[6].y = dst2y + b1;
	dst[6].z = dst2z + b2;

	dst[7].x = dst3x + b0;
	dst[7].y = dst3y + b1;
	dst[7].z = dst3z + b2;
}

/******************************************************************************
 *
 * Function:		DPVS::Math::reciprocal4()
 *
 * Description:		Calculates reciprocal of a 4-component vector
 *
 * Parameters:		dst = destination vector
 *					src = source vector
 *
 * Notes:			Default implementation uses only a single division. On
 *					SIMD processors this is implemented using a SIMD instruction.
 *
 ******************************************************************************/

static DPVS_FORCE_INLINE void reciprocal4 (float& w0, float& w1, float& w2, float& w3)
{
#if defined (DPVS_CPU_GAMECUBE) || defined (DPVS_CPU_X86)
	float	a		= w0;
	float	b		= w1;
	float	c		= w2;
	float	d		= w3;
	double	bc		= b  * c;
	double	bcd		= bc * d;
	double	rcp		= 1.0 / double(a*bcd);
	w0				= (float)(rcp * bcd);
	double	rcpBCD	= rcp * a;
	w3				= (float)(rcpBCD * bc);
	double	rcpBC	= rcpBCD * d;
	w1				= (float)(rcpBC * c);
	w2				= (float)(rcpBC * b);				// 9 muls and 1 div
#else
	w0 = Math::reciprocal(w0);
	w1 = Math::reciprocal(w1);
	w2 = Math::reciprocal(w2);
	w3 = Math::reciprocal(w3);
#endif
}

/******************************************************************************
 *
 * Function:		DPVS::Math::transformUnitBoxXYDivByW()
 *
 * Description:		Transforms unit box (-1,-1,-1) - (+1,+1,+1) by matrix 'm' and
 *					stores XY coordinates of the vertices (divided by W) into array 'dst'
 *
 * Parameters:		dst = destination vertices
 *					m	= matrix
 *
 * Notes:			Assumes that the box is not front-clipping!
 *
 ******************************************************************************/

void transformUnitBoxXYDivByW (Vector2 dst[8], const Matrix4x4& m)
{
//	float DPVS_VECTOR_ALIGN(w[8]);

	float a3	= 2.0f * m[3].x;
	float b3	= 2.0f * m[3].z;

	float w0	= -m[3].x -m[3].y -m[3].z + m[3].w;
	float w4	= w0+b3;
	float w1	= w0+a3;
	float w5	= w1+b3;

	float w2	= w0+2.0f*m[3].y;
	float w3	= w2+a3;
	float w6	= w2+b3;
	float w7	= w3+b3;

	reciprocal4 (w0, w4, w1, w5);

	float a0	= 2.0f * m[0].x;
	float a1	= 2.0f * m[1].x;
	float b0	= 2.0f * m[0].z;
	float b1	= 2.0f * m[1].z;
	float dst0x	= m[0].w-(m[0].x + m[0].y + m[0].z);
	float dst0y	= m[1].w-(m[1].x + m[1].y + m[1].z);
	float dst1x	= dst0x + a0;
	float dst1y	= dst0y + a1;
	float dst2x	= dst0x + 2.0f * m[0].y;
	float dst2y	= dst0y + 2.0f * m[1].y;
	float dst3x	= dst2x + a0;
	float dst3y	= dst2y + a1;

	dst[0].x	= dst0x*w0;
	dst[0].y	= dst0y*w0;
	dst[4].x	= (dst0x + b0)*w4;
	dst[4].y	= (dst0y + b1)*w4;

	dst[1].x	= dst1x*w1;
	dst[1].y	= dst1y*w1;
	dst[5].x	= (dst1x + b0)*w5;
	dst[5].y	= (dst1y + b1)*w5;

	reciprocal4 (w2, w6, w3, w7);

	dst[2].x	= dst2x*w2;
	dst[2].y	= dst2y*w2;
	dst[6].x	= (dst2x + b0)*w6;
	dst[6].y	= (dst2y + b1)*w6;

	dst[3].x	= dst3x*w3;
	dst[3].y	= dst3y*w3;
	dst[7].x	= (dst3x + b0)*w7;
	dst[7].y	= (dst3y + b1)*w7;
}

/******************************************************************************
 *
 * Function:		DPVS::Math::transformUnitBoxXYZDivByW()
 *
 * Description:		Transforms unit box (-1,-1,-1) - (+1,+1,+1) by matrix 'm' and
 *					stores XYZ coordinates of the vertices (divided by W) into array 'dst'
 *
 * Parameters:		dst = destination vertices
 *					m	= matrix
 *
 ******************************************************************************/

// returns false if object is front-clipping NOTE NOTE THE VECTORS ARE NOT CORRECT THEN!!!!
bool transformUnitBoxXYZDivByW (Vector3 dst[8], const Matrix4x4& m)
{
	//--------------------------------------------------------------------
	// Calculate OOW array
	//--------------------------------------------------------------------

//	float DPVS_VECTOR_ALIGN (w[8]);
	float w0,w1,w2,w3,w4,w5,w6,w7;

	float w0a	= 2.0f * m[3].x;
	float w1a	= 2.0f * m[3].z;

	w0		= m[3].w - m[3].x - m[3].y - m[3].z;
	w1		= w0+w0a;
	w2		= w0+2.0f*m[3].y;
	w3		= w2+w0a;
	w4		= w0+w1a;
	w5		= w1+w1a;
	w6		= w2+w1a;
	w7		= w3+w1a;

//	const INT32* iw = (const INT32*)(w);
//	if(((iw[0]-1) | (iw[1]-1) | (iw[2]-1) | (iw[3]-1) | (iw[4]-1) | (iw[5]-1) | (iw[6]-1) | (iw[7]-1)) < 0)
//		return false;

	// check if there are any negative or zero values...

	float mn = min(w0,w1);
	mn = min(mn,w2);
	mn = min(mn,w3);
	mn = min(mn,w4);
	mn = min(mn,w5);
	mn = min(mn,w6);
	mn = min(mn,w7);
	if (mn <= 0.0f)
		return false;

	reciprocal4(w0, w1, w2, w3);
	reciprocal4(w4, w5, w6, w7);

	//--------------------------------------------------------------------
	// calculate xyz
	//--------------------------------------------------------------------

	register float dst0x	= m[0].w - m[0].x - m[0].y - m[0].z;
	register float dst0y	= m[1].w - m[1].x - m[1].y - m[1].z;
	register float dst0z	= m[2].w - m[2].x - m[2].y - m[2].z;

	register float b0	= 2.0f * m[0].z;
	register float b1	= 2.0f * m[1].z;
	register float b2	= 2.0f * m[2].z;

	dst[0].x	= dst0x*w0;
	dst[0].y	= dst0y*w0;
	dst[0].z	= dst0z*w0;
	dst[4].x	= (dst0x + b0)*w4;
	dst[4].y	= (dst0y + b1)*w4;
	dst[4].z	= (dst0z + b2)*w4;

	register float a0	= 2.0f * m[0].x;
	register float a1	= 2.0f * m[1].x;
	register float a2	= 2.0f * m[2].x;

	register float dst1x	= dst0x + a0;
	register float dst1y	= dst0y + a1;
	register float dst1z	= dst0z + a2;

	dst[1].x	= dst1x*w1;
	dst[1].y	= dst1y*w1;
	dst[1].z	= dst1z*w1;
	dst[5].x	= (dst1x + b0)*w5;
	dst[5].y	= (dst1y + b1)*w5;
	dst[5].z	= (dst1z + b2)*w5;

	register float dst2x	= dst0x + (2.0f * m[0].y);
	register float dst2y	= dst0y + (2.0f * m[1].y);
	register float dst2z	= dst0z + (2.0f * m[2].y);

	dst[2].x	= dst2x*w2;
	dst[2].y	= dst2y*w2;
	dst[2].z	= dst2z*w2;
	dst[6].x	= (dst2x + b0)*w6;
	dst[6].y	= (dst2y + b1)*w6;
	dst[6].z	= (dst2z + b2)*w6;

	register float dst3x	= dst2x + a0;
	register float dst3y	= dst2y + a1;
	register float dst3z	= dst2z + a2;

	dst[3].x	= dst3x*w3;
	dst[3].y	= dst3y*w3;
	dst[3].z	= dst3z*w3;
	dst[7].x	= (dst3x + b0)*w7;
	dst[7].y	= (dst3y + b1)*w7;
	dst[7].z	= (dst3z + b2)*w7;

	return true;
}

/******************************************************************************
 *
 * Function:		DPVS::Math::transformOBBVertices()
 *
 * Description:		Transforms eight vertices of an OBB into space determined by 'mtx'
 *
 * Parameters:		dst = destination vertex array
 *					obb = OBB transformation matrix 'm'
 *					objectToCamera = object->camera transformation matrix 'n'
 *
 ******************************************************************************/

void transformOBBVertices (Vector3 dst[8], const Matrix4x3& m, const Matrix4x3& n)
{
//	Matrix4x3 d(NO_CONSTRUCTOR);
//	Math::productFromLeft(d,m,n);
//	Math::transformUnitBox (dst,d);		// transform OBB vertices into camera-space

	float d0x = n[0][0]*m[0][0] + n[0][1]*m[1][0] + n[0][2]*m[2][0];
	float d0y = n[0][0]*m[0][1] + n[0][1]*m[1][1] + n[0][2]*m[2][1];
	float d0z = n[0][0]*m[0][2] + n[0][1]*m[1][2] + n[0][2]*m[2][2];
	float d0w = n[0][0]*m[0][3] + n[0][1]*m[1][3] + n[0][2]*m[2][3] + n[0][3];
	float d1x = n[1][0]*m[0][0] + n[1][1]*m[1][0] + n[1][2]*m[2][0];
	float d1y = n[1][0]*m[0][1] + n[1][1]*m[1][1] + n[1][2]*m[2][1];
	float d1z = n[1][0]*m[0][2] + n[1][1]*m[1][2] + n[1][2]*m[2][2];
	float d1w = n[1][0]*m[0][3] + n[1][1]*m[1][3] + n[1][2]*m[2][3] + n[1][3];
	float d2x = n[2][0]*m[0][0] + n[2][1]*m[1][0] + n[2][2]*m[2][0];
	float d2y = n[2][0]*m[0][1] + n[2][1]*m[1][1] + n[2][2]*m[2][1];
	float d2z = n[2][0]*m[0][2] + n[2][1]*m[1][2] + n[2][2]*m[2][2];
	float d2w = n[2][0]*m[0][3] + n[2][1]*m[1][3] + n[2][2]*m[2][3] + n[2][3];

	float a0 = d0x + d0x;
	float a1 = d1x + d1x;
	float a2 = d2x + d2x;
	float b0 = d0z + d0z;
	float b1 = d1z + d1z;
	float b2 = d2z + d2z;

	float dst0x = d0w - (d0x + d0y + d0z);
	float dst0y = d1w - (d1x + d1y + d1z);
	float dst0z = d2w - (d2x + d2y + d2z);

	dst[0].x = dst0x; dst[0].y = dst0y; dst[0].z = dst0z;

	dst[4].x = dst0x + b0;
	dst[4].y = dst0y + b1;
	dst[4].z = dst0z + b2;

	float dst1x = dst0x + a0;
	float dst1y = dst0y + a1;
	float dst1z = dst0z + a2;

	dst[1].x = dst1x; dst[1].y = dst1y; dst[1].z = dst1z;

	dst[5].x = dst1x + b0;
	dst[5].y = dst1y + b1;
	dst[5].z = dst1z + b2;

	float dst2x = dst0x + d0y + d0y;
	float dst2y = dst0y + d1y + d1y;
	float dst2z = dst0z + d2y + d2y;

	dst[2].x = dst2x; dst[2].y = dst2y; dst[2].z = dst2z;

	dst[6].x = dst2x + b0;
	dst[6].y = dst2y + b1;
	dst[6].z = dst2z + b2;

	float dst3x = dst2x + a0;
	float dst3y = dst2y + a1;
	float dst3z = dst2z + a2;

	dst[3].x = dst3x; dst[3].y = dst3y; dst[3].z = dst3z;

	dst[7].x = dst3x + b0;
	dst[7].y = dst3y + b1;
	dst[7].z = dst3z + b2;

}

/******************************************************************************
 *
 * Function:		DPVS::Math::invertMatrix4x3()
 *
 * Description:		Inverts a 4x3 matrix
 *
 * Parameters:		m	= reference to destination matrix
 *					src	= reference to source matrix (may equal 'm')
 *
 * Notes:			m may be equal to src...
 *
 ******************************************************************************/

void invertMatrix (Matrix4x3& m, const Matrix4x3& src)
{
	float a2 = src[1][0];
	float b2 = src[1][1];
	float c2 = src[1][2];
	float a3 = src[2][0];
	float b3 = src[2][1];
	float c3 = src[2][2];

	float b2c3_b3c2 = b2 * c3 - b3 * c2;
	float a3c2_a2c3 = a3 * c2 - a2 * c3;
	float a2b3_a3b2 = a2 * b3 - a3 * b2;

	float a1 = src[0][0];
	float b1 = src[0][1];
	float c1 = src[0][2];

	float rDet	= Math::reciprocal(a1 * (b2c3_b3c2) + b1 * (a3c2_a2c3) + c1 * (a2b3_a3b2));

	b2c3_b3c2	*= rDet;
	a3c2_a2c3	*= rDet;
	a2b3_a3b2	*= rDet;
	a1			*= rDet;
	b1			*= rDet;
	c1			*= rDet;

	float d1 = -src[0][3];
	float d2 =  src[1][3];
	float d3 =  src[2][3];

	{
		float c1b3_b1c3 = c1 * b3 - b1 * c3;
		float b1c2_c1b2 = b1 * c2 - c1 * b2;

		m[0][0]	= b2c3_b3c2;
		m[0][1]	= c1b3_b1c3;
		m[0][2]	= b1c2_c1b2;
		m[0][3]	= d1 * b2c3_b3c2 - d2 * c1b3_b1c3 - d3 * b1c2_c1b2;
	}

	{
		float a1c3_c1a3 = a1 * c3 - c1 * a3;
		float c1a2_a1c2 = c1 * a2 - a1 * c2;

		m[1][0]	= a3c2_a2c3;
		m[1][1]	= a1c3_c1a3;
		m[1][2]	= c1a2_a1c2;
		m[1][3]	= d1 * a3c2_a2c3 - d2 * a1c3_c1a3 - d3 * c1a2_a1c2;
	}

	{
		float b1a3_a1b3 = b1 * a3 - a1 * b3;
		float a1b2_b1a2 = a1 * b2 - b1 * a2;

		m[2][0]	= a2b3_a3b2;
		m[2][1]	= b1a3_a1b3;
		m[2][2]	= a1b2_b1a2;
		m[2][3]	= d1 * a2b3_a3b2 - d2 * b1a3_a1b3 - d3 * a1b2_b1a2;
	}
}


/******************************************************************************
 *
 * Function:		DPVS::Math::isPerspectiveProjectionMatrix()
 *
 * Description:		Returns boolean indicating whether matrix is a
 *					perspective projection matrix
 *
 * Parameters:		m	= reference to source 4x4 matrix
 *
 * Returns:			true if perspective projection matrix
 *
 * Notes:			This function is mainly called in debug build and isn't
 *					thus time-critical (so don't go and optimize these into
 *					integer comparisons or something stupid like that).
 *
 ******************************************************************************/

bool isPerspectiveProjectionMatrix (const Matrix4x4& c)
{
	if (c[3][2] != 1.0f)
		return false;
	const UINT32* s = reinterpret_cast<const UINT32*>(&c);
	UINT32 t =          s[1]  |		s[3] |
				s[4]          |		s[7] |
				s[8]  | s[9]  |
				s[12] | s[13] |     s[15];

	return ((t&0x7FFFFFFF)==0);		// if all tested elements were zeroes (signed or unsigned)...

}

/******************************************************************************
 *
 * Function:		DPVS::Math::invertMatrix()
 *
 * Description:		Inverts a 4x4 matrix
 *
 * Parameters:		m	= reference to destination matrix
 *					src	= reference to source matrix (may equal 'm')
 *
 * Notes:			Detects internally if matrix is really 4x3, then calls
 *					appropriate function.
 *
 ******************************************************************************/

void invertMatrix (Matrix4x4& m, const Matrix4x4& src)
{
	if (isZero(src[3][0]) && isZero(src[3][1]) && isZero(src[3][2]) && src[3][3] == 1.0f)
	{
		invertMatrix ((Matrix4x3&)m,(const Matrix4x3&)src);
		m[3].make(0.0f,0.0f,0.0f,1.0f);			// clear last row
		return;
	}

	invertMatrix4x4(m,src);
}

} // DPVS::Math
} // DPVS

//------------------------------------------------------------------------
