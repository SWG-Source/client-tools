#ifndef __DPVSMATRIX_HPP
#define __DPVSMATRIX_HPP
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
 * Description: 	Matrix class
 *
 * $Archive: /dpvs/implementation/include/dpvsMatrix.hpp $
 * $Author: otso $ 
 * $Revision: #2 $
 * $Modtime: 27.08.02 14:15 $
 * $Date: 2004/11/08 $
 * 
 ******************************************************************************/

#if !defined (__DPVSVECTOR_HPP)
#	include "dpvsVector.hpp"
#endif

namespace DPVS
{

/******************************************************************************
 *
 * Class:			DPVS::Matrix4x3
 *
 * Description:		4x3 Matrix class used for passing data into some classes
 *
 *****************************************************************************/

class Matrix4x3
{
private:
	float DPVS_VECTOR_ALIGN(m[3][4]);	
public:
DPVS_FORCE_INLINE 	void				ident			(void);
DPVS_FORCE_INLINE						Matrix4x3		(void)										{ DPVS_CHECK_ALIGN(this); ident(); }
DPVS_FORCE_INLINE	explicit			Matrix4x3		(DisableConstructor)						{ DPVS_CHECK_ALIGN(this); }
DPVS_FORCE_INLINE						Matrix4x3		(const Matrix4x3& n)						{ DPVS_CHECK_ALIGN(&n); *this = n; }
DPVS_FORCE_INLINE	Matrix4x3&			operator=		(const Matrix4x3& n);
DPVS_FORCE_INLINE	const Vector4&		operator[]		(int i) const								{ DPVS_ASSERT (i>=0 && i < 3);  return (const Vector4&)(m[i][0]); }
DPVS_FORCE_INLINE	Vector4&			operator[]		(int i)										{ DPVS_ASSERT (i>=0 && i < 3);  return (Vector4&)(m[i][0]); }
DPVS_FORCE_INLINE	bool				operator==		(const Matrix4x3& n) const;
DPVS_FORCE_INLINE	bool				operator!=		(const Matrix4x3& n) const					{ return !(*this == n); }
DPVS_FORCE_INLINE	void				clear			(void);
DPVS_FORCE_INLINE	Vector3				getColumn		(int i)	const								{ DPVS_ASSERT (i>=0 && i < 4);  return Vector3(m[0][i], m[1][i], m[2][i]);	}
DPVS_FORCE_INLINE	Vector3				getDof			(void) const								{ return Vector3(m[0][2], m[1][2], m[2][2]); }
					void				flushToZero		(void);
DPVS_FORCE_INLINE	float				getMaxScale		(void) const;
DPVS_FORCE_INLINE	Vector3				getRight		(void) const								{ return Vector3(m[0][0], m[1][0], m[2][0]); }
DPVS_FORCE_INLINE	Vector3				getTranslation	(void) const								{ return getColumn(3);		}
DPVS_FORCE_INLINE	Vector3				getUp			(void) const								{ return Vector3(m[0][1], m[1][1], m[2][1]); }
DPVS_FORCE_INLINE	void				productFromLeft	(const Matrix4x3&);
DPVS_FORCE_INLINE	Matrix4x3&			setColumn		(int i, const Vector3& v)					{ DPVS_ASSERT (i>=0 && i < 4);  m[0][i] = v.x; m[1][i] = v.y; m[2][i] = v.z; return *this; }
DPVS_FORCE_INLINE	Matrix4x3&			setTranslation	(const Vector3& v)							{ return setColumn(3,v);	}
DPVS_FORCE_INLINE	Vector3				transform		(const Vector3& src) const;
DPVS_FORCE_INLINE	Vector4				transform		(const Vector4& src) const;
DPVS_FORCE_INLINE	Vector4				transformT		(const Vector4& src) const;
};
DPVS_CT_ASSERT(sizeof(Matrix4x3)==48);

/******************************************************************************
 *
 * Class:			DPVS::Matrix4x4d
 *
 * Description:		Double-precision 4x4 Matrix class used for passing data into 
 *					the public API functions. Not used internally in the library
 *					except for converting into Matrix4x4's
 *
 *****************************************************************************/

class Matrix4x4d
{
private:
	double m[4][4];
public:
	DPVS_FORCE_INLINE	Matrix4x4d&		operator=			(const Matrix4x4& s);
	DPVS_FORCE_INLINE	const Vector4d&	operator[]			(int i) const								{ DPVS_ASSERT (i>=0 && i < 4);  return (const Vector4d&)(m[i][0]); }
	DPVS_FORCE_INLINE	Vector4d&		operator[]			(int i)										{ DPVS_ASSERT (i>=0 && i < 4);  return (Vector4d&)(m[i][0]); }
};

/******************************************************************************
 *
 * Class:			DPVS::Matrix4x4
 *
 * Description:		4x4 Matrix class used for passing data into the
 *					public API functions
 *
 *****************************************************************************/

class Matrix4x4
{
private:
	float DPVS_VECTOR_ALIGN(m[4][4]);	
public:
DPVS_FORCE_INLINE	void				ident				(void);
DPVS_FORCE_INLINE						Matrix4x4			(void)										{ DPVS_CHECK_ALIGN(this); ident(); }
DPVS_FORCE_INLINE	explicit			Matrix4x4			(DisableConstructor)						{ DPVS_CHECK_ALIGN(this); }
DPVS_FORCE_INLINE						Matrix4x4			(const Matrix4x4& n)						{ *this = n; }
DPVS_FORCE_INLINE	Matrix4x4&			operator=			(const Matrix4x4d&);
DPVS_FORCE_INLINE	Matrix4x4&			operator=			(const Matrix4x4&);
DPVS_FORCE_INLINE	const Vector4&		operator[]			(int i) const								{ DPVS_ASSERT (i>=0 && i < 4);  return (const Vector4&)(m[i][0]); }
DPVS_FORCE_INLINE	Vector4&			operator[]			(int i)										{ DPVS_ASSERT (i>=0 && i < 4);  return (Vector4&)(m[i][0]); }
DPVS_FORCE_INLINE	bool				operator==			(const Matrix4x4& n) const;
DPVS_FORCE_INLINE	bool				operator!=			(const Matrix4x4& n) const					{ return !(*this == n); }
DPVS_FORCE_INLINE	void				clear				(void);
					void				flushToZero			(void);
DPVS_FORCE_INLINE	Vector4				getColumn			(int i)	const								{ DPVS_ASSERT (i>=0 && i < 4);  return Vector4(m[0][i], m[1][i], m[2][i],m[3][i]);	}
DPVS_FORCE_INLINE	Vector3				getDof				(void) const								{ return Vector3(m[0][2], m[1][2], m[2][2]); }
DPVS_FORCE_INLINE	float				getMaxScale			(void) const;
DPVS_FORCE_INLINE	Vector3				getRight			(void) const								{ return Vector3(m[0][0], m[1][0], m[2][0]); }
DPVS_FORCE_INLINE	Vector3				getTranslation		(void) const								{ return Vector3(m[0][3], m[1][3], m[2][3]); }
DPVS_FORCE_INLINE	Vector3				getUp				(void) const								{ return Vector3(m[0][1], m[1][1], m[2][1]); }
DPVS_FORCE_INLINE	bool				is4x3Matrix			(void) const								{ return Math::isZero(m[3][0]) && Math::isZero(m[3][1]) && Math::isZero(m[3][2]) && m[3][3] == 1.0f; }
DPVS_FORCE_INLINE	bool				isUniform			(void) const;
DPVS_FORCE_INLINE	void				productFromLeft		(const Matrix4x4& n);
DPVS_FORCE_INLINE	void				setColumn			(int i, const Vector4& v)					{ DPVS_ASSERT (i>=0 && i < 4);  m[0][i] = v.x; m[1][i] = v.y; m[2][i] = v.z; m[3][i] = v.w; }
DPVS_FORCE_INLINE	void				setTranslation		(const Vector3& v)							{ m[0][3] = v[0]; m[1][3] = v[1]; m[2][3] = v[2]; }
DPVS_FORCE_INLINE	Vector4				transform			(const Vector3& src) const;
DPVS_FORCE_INLINE	Vector4				transform			(const Vector4& src) const;
DPVS_FORCE_INLINE	Vector3		 		transformDivByW		(const Vector3& src) const;
DPVS_FORCE_INLINE	Vector4				transformT			(const Vector3& src) const;
DPVS_FORCE_INLINE	Vector4				transformT			(const Vector4& src) const;
DPVS_FORCE_INLINE	void				transpose			(void);
DPVS_FORCE_INLINE	void				transpose			(const Matrix4x4& src);

};

DPVS_CT_ASSERT(sizeof(Matrix4x4)==64);

//------------------------------------------------------------------------
// Implementation
//------------------------------------------------------------------------

DPVS_FORCE_INLINE void Matrix4x4::ident (void)
{
#if defined (DPVS_PS2_VU_ASSEMBLY)
//PS2-BEGIN
	DPVS_CHECK_ALIGN(&m[0][0]);
	asm ("sqc2	vf0, 0x30(%0);vmr32.xyzw vf1, vf0; sqc2	vf1, 0x20(%0); vmr32.xyzw vf2, vf1; sqc2 vf2, 0x10(%0); vmr32.xyzw vf1, vf2; sqc2 vf1, 0x0(%0) " : : "r"(&m[0][0]) );
//PS2-END
#else

	m[0][0] = 1.0f; m[0][1] = 0.0f; m[0][2] = 0.0f; m[0][3] = 0.0f;
	m[1][0] = 0.0f; m[1][1] = 1.0f; m[1][2] = 0.0f; m[1][3] = 0.0f;
	m[2][0] = 0.0f; m[2][1] = 0.0f; m[2][2] = 1.0f; m[2][3] = 0.0f;
	m[3][0] = 0.0f; m[3][1] = 0.0f; m[3][2] = 0.0f; m[3][3] = 1.0f;

#endif // DPVS_PS2_ASSEMBLY
}

DPVS_FORCE_INLINE void Matrix4x3::ident(void)
{
#if defined (DPVS_PS2_VU_ASSEMBLY)
//PS2-BEGIN
	DPVS_CHECK_ALIGN(&m[0][0]);
	asm  ("vmr32.xyzw	vf1, vf0;sqc2		vf1, 0x20(%0);vmr32.xyzw	vf2, vf1;sqc2		vf2, 0x10(%0);vmr32.xyzw	vf1, vf2;sqc2		vf1, 0x0(%0)" : : "r"(&m[0][0]) );
//PS2-END
#else
	m[0][0] = 1.0f; m[0][1] = 0.0f; m[0][2] = 0.0f; m[0][3] = 0.0f;
	m[1][0] = 0.0f; m[1][1] = 1.0f; m[1][2] = 0.0f; m[1][3] = 0.0f;
	m[2][0] = 0.0f; m[2][1] = 0.0f; m[2][2] = 1.0f; m[2][3] = 0.0f;
#endif
}

DPVS_FORCE_INLINE Vector3 Matrix4x3::transform (const Vector3& s)  const
{
	return Vector3(
	s.x * m[0][0] + s.y * m[0][1] + s.z * m[0][2] + m[0][3],
	s.x * m[1][0] + s.y * m[1][1] + s.z * m[1][2] + m[1][3],
	s.x * m[2][0] + s.y * m[2][1] + s.z * m[2][2] + m[2][3]);
}

DPVS_FORCE_INLINE Vector4 Matrix4x3::transform	(const Vector4& s)  const
{
	return Vector4(
	s.x * m[0][0] + s.y * m[0][1] + s.z * m[0][2] + s.w * m[0][3],
	s.x * m[1][0] + s.y * m[1][1] + s.z * m[1][2] + s.w * m[1][3],
	s.x * m[2][0] + s.y * m[2][1] + s.z * m[2][2] + s.w * m[2][3],
	s.w);
}

DPVS_FORCE_INLINE Vector4 Matrix4x3::transformT	(const Vector4& s) const			// transform transposed
{
	return Vector4(
	s.x * m[0][0] + s.y * m[1][0] + s.z * m[2][0],
	s.x * m[0][1] + s.y * m[1][1] + s.z * m[2][1],
	s.x * m[0][2] + s.y * m[1][2] + s.z * m[2][2],
	s.x * m[0][3] + s.y * m[1][3] + s.z * m[2][3] + s.w);
}

DPVS_FORCE_INLINE Matrix4x3& Matrix4x3::operator= (const Matrix4x3& ss)
{
#if defined (DPVS_PS2_ASSEMBLY)
//PS2-BEGIN

	DPVS_ASSERT( is128Aligned(this) );
	DPVS_ASSERT( is128Aligned(&ss) );

	asm (
	"	lq		$8, 0x00(%1)"
	"\n	lq		$9, 0x10(%1)"
	"\n	lq		$10,0x20(%1)"
	"\n	sq		$8, 0x00(%0)"
	"\n	sq		$9, 0x10(%0)"
	"\n	sq		$10,0x20(%0)"
	 : : "r"(&m[0][0]), "r"(&ss.m[0][0]) : "$8", "$9", "$10" );

	return *this;
//PS2-END
#else

	float*			d = &(*this)[0][0];
	const float*	s = &ss[0][0];
	for (int i = 0 ; i < 12; i++)
		d[i] = s[i];
	return *this;

#endif // DPVS_PS2_ASSEMBLY
}

DPVS_FORCE_INLINE Matrix4x4& Matrix4x4::operator= (const Matrix4x4& ss)
{
#if defined (DPVS_PS2_ASSEMBLY)
//PS2-BEGIN

	DPVS_ASSERT( is128Aligned(this) );
	DPVS_ASSERT( is128Aligned(&ss) );

	asm (
	"	lq		$8, 0x00(%1)"
	"\n	lq		$9, 0x10(%1)"
	"\n	lq		$10,0x20(%1)"
	"\n	lq		$11,0x30(%1)"
	"\n	sq		$8, 0x00(%0)"
	"\n	sq		$9, 0x10(%0)"
	"\n	sq		$10,0x20(%0)"
	"\n	sq		$11,0x30(%0)"
	 : : "r"(&m[0][0]), "r"(&ss.m[0][0]) : "$8", "$9", "$10", "$11" );

	return *this;
//PS2-END
#else

	float*			d = &(*this)[0][0];
	const float*	s = &ss[0][0];
	for (int i = 0 ; i < 16; i++)
		d[i] = s[i];
	return *this;

#endif // DPVS_PS2_ASSEMBLY
}

DPVS_FORCE_INLINE Vector4 Matrix4x4::transform	(const Vector4& s)  const
{
#if defined (DPVS_PS2_ASSEMBLY)
//PS2-BEGIN

	Vector4 DPVS_VECTOR_ALIGN(s1(NO_CONSTRUCTOR));

	asm __volatile__ (
	"	lwc1	$f10, 0(%0)"
	"\n	lwc1	$f11, 4(%0)"
	"\n	lwc1	$f12, 8(%0)"
	"\n	lwc1	$f13, 12(%0)"
	"\n	lwc1	$f14, 16(%0)"
	"\n	lwc1	$f15, 20(%0)"
	"\n	lwc1	$f16, 24(%0)"
	"\n	lwc1	$f17, 28(%0)"
	"\n	lwc1	$f18, 32(%0)"
	"\n	lwc1	$f19, 36(%0)"
	"\n	lwc1	$f20, 40(%0)"
	"\n	lwc1	$f21, 44(%0)"
	"\n	lwc1	$f22, 48(%0)"
	"\n	lwc1	$f23, 52(%0)"
	"\n	lwc1	$f24, 56(%0)"
	"\n	lwc1	$f25, 60(%0)"
	"\n	lwc1	$f4, 0(%1)"
	"\n	lwc1	$f5, 4(%1)"
	"\n	lwc1	$f6, 8(%1)"
	"\n	lwc1	$f7, 12(%1)"
		/*	s	= f4-f7
			m	= f10-f25 */

	"\n	mula.s	$f4, $f10"
	"\n	madda.s	$f5, $f11"
	"\n	madda.s	$f6, $f12"
	"\n	madd.s	$f1, $f13, $f7"
	"\n	swc1	$f1, 0(%2)"

	"\n	mula.s	$f4, $f14"
	"\n	madda.s	$f5, $f15"
	"\n	madda.s	$f6, $f16"
	"\n	madd.s	$f1, $f17, $f7"
	"\n	swc1	$f1, 4(%2)"

	"\n	mula.s	$f4, $f18"
	"\n	madda.s	$f5, $f19"
	"\n	madda.s	$f6, $f20"
	"\n	madd.s	$f1, $f21, $f7"
	"\n	swc1	$f1, 8(%2)"

	"\n	mula.s	$f4, $f22"
	"\n	madda.s	$f5, $f23"
	"\n	madda.s	$f6, $f24"
	"\n	madd.s	$f1, $f25, $f7"
	"\n	swc1	$f1, 12(%2)"
	 : : "r"(&m[0][0]), "r"(&s), "r"(&s1) : "$f10", "$f11", "$f12", "$f13", "$f14", "$f15", "$f16", "$f17", "$f18", "$f19", "$f20", "$f21", "$f22", "$f23", "$f24", "$f25", "$f4", "$f5", "$f6", "$f7", "$f1" );

	return s1;
//PS2-END
#else

	return Vector4(
	s.x * m[0][0] + s.y * m[0][1] + s.z * m[0][2] + s.w * m[0][3],
	s.x * m[1][0] + s.y * m[1][1] + s.z * m[1][2] + s.w * m[1][3],
	s.x * m[2][0] + s.y * m[2][1] + s.z * m[2][2] + s.w * m[2][3],
	s.x * m[3][0] + s.y * m[3][1] + s.z * m[3][2] + s.w * m[3][3]);

#endif // DPVS_PS2_ASSEMBLY
}

DPVS_FORCE_INLINE Vector4 Matrix4x4::transformT	(const Vector3& s)  const			// transform transposed
{
	return Vector4(
	s.x * m[0][0] + s.y * m[1][0] + s.z * m[2][0] + m[3][0],
	s.x * m[0][1] + s.y * m[1][1] + s.z * m[2][1] + m[3][1],
	s.x * m[0][2] + s.y * m[1][2] + s.z * m[2][2] + m[3][2],
	s.x * m[0][3] + s.y * m[1][3] + s.z * m[2][3] + m[3][3]);
}

DPVS_FORCE_INLINE Vector4 Matrix4x4::transformT	(const Vector4& s) const			// transform transposed
{
	return Vector4(
	s.x * m[0][0] + s.y * m[1][0] + s.z * m[2][0] + s.w * m[3][0],
	s.x * m[0][1] + s.y * m[1][1] + s.z * m[2][1] + s.w * m[3][1],
	s.x * m[0][2] + s.y * m[1][2] + s.z * m[2][2] + s.w * m[3][2],
	s.x * m[0][3] + s.y * m[1][3] + s.z * m[2][3] + s.w * m[3][3]);
}

DPVS_FORCE_INLINE Vector4 Matrix4x4::transform (const Vector3& s)  const
{
	// the non-existing W component of the source vector is regarded to be 1.0
	return Vector4(
	s.x * m[0][0] + s.y * m[0][1] + s.z * m[0][2] + m[0][3],
	s.x * m[1][0] + s.y * m[1][1] + s.z * m[1][2] + m[1][3],
	s.x * m[2][0] + s.y * m[2][1] + s.z * m[2][2] + m[2][3],
	s.x * m[3][0] + s.y * m[3][1] + s.z * m[3][2] + m[3][3]);
}

DPVS_FORCE_INLINE void Matrix4x4::transpose (void)
{
#if defined (DPVS_PS2_ASSEMBLY)
//PS2-BEGIN

	asm (
	"	lw $8, 0x04(%0)"
	"\n	lw $9, 0x10(%0)"
	"\n	sw $8, 0x10(%0)"
	"\n	sw $9, 0x04(%0)"

	"\n	lw $8, 0x08(%0)"
	"\n	lw $9, 0x20(%0)"
	"\n	sw $8, 0x20(%0)"
	"\n	sw $9, 0x08(%0)"

	"\n	lw $8, 0x0c(%0)" 
	"\n	lw $9, 0x30(%0)"
	"\n	sw $8, 0x30(%0)"
	"\n	sw $9, 0x0c(%0)"

	"\n	lw $8, 0x18(%0)"
	"\n	lw $9, 0x24(%0)"
	"\n	sw $8, 0x24(%0)"
	"\n	sw $9, 0x18(%0)"

	"\n	lw $8, 0x1c(%0)"
	"\n	lw $9, 0x34(%0)"
	"\n	sw $8, 0x34(%0)"
	"\n	sw $9, 0x1c(%0)"

	"\n	lw $8, 0x2c(%0)"
	"\n	lw $9, 0x38(%0)"
	"\n	sw $8, 0x38(%0)"
	"\n	sw $9, 0x2c(%0)"
	 : : "r"(&m[0][0]) : "$8", "$9" );
//PS2-END
#else

	float tmp;
	tmp = m[0][1]; m[0][1] = m[1][0]; m[1][0] = tmp;
	tmp = m[0][2]; m[0][2] = m[2][0]; m[2][0] = tmp;
	tmp = m[0][3]; m[0][3] = m[3][0]; m[3][0] = tmp;
	tmp = m[1][2]; m[1][2] = m[2][1]; m[2][1] = tmp;
	tmp = m[1][3]; m[1][3] = m[3][1]; m[3][1] = tmp;
	tmp = m[2][3]; m[2][3] = m[3][2]; m[3][2] = tmp;

#endif // DPVS_PS2_ASSEMBLY
}

DPVS_FORCE_INLINE Vector3 Matrix4x4::transformDivByW (const Vector3& s)  const
{
	float  oow = Math::reciprocal(s.x * m[3][0] + s.y * m[3][1] + s.z * m[3][2] + m[3][3]);
	// the non-existing W component of the source vector is regarded to be 1.0
	return Vector3(
	(s.x * m[0][0] + s.y * m[0][1] + s.z * m[0][2] + m[0][3])*oow,
	(s.x * m[1][0] + s.y * m[1][1] + s.z * m[1][2] + m[1][3])*oow,
	(s.x * m[2][0] + s.y * m[2][1] + s.z * m[2][2] + m[2][3])*oow);
}

DPVS_FORCE_INLINE void Matrix4x3::clear(void)
{
#if defined (DPVS_PS2_ASSEMBLY)
//PS2-BEGIN

	DPVS_ASSERT( is128Aligned(this) );

	asm (
	"	sq		$0, 0x00(%0)"
	"\n	sq		$0, 0x10(%0)"
	"\n	sq		$0, 0x20(%0)"
	 : : "r"(&m[0][0]) );
//PS2-END
#else

	float* t = &m[0][0];
	for (int i = 0; i < 12; i++)
		t[i] = 0.0f;

#endif // DPVS_PS2_ASSEMBLY
}

DPVS_FORCE_INLINE void Matrix4x4::clear (void)
{
#if defined (DPVS_PS2_ASSEMBLY)
//PS2-BEGIN

	DPVS_ASSERT( is128Aligned(this) );

	asm (
	"	sq		$0, 0x00(%0)"
	"\n	sq		$0, 0x10(%0)"
	"\n	sq		$0, 0x20(%0)"
	"\n	sq		$0, 0x30(%0)"
	 : : "r"(&m[0][0]) );
//PS2-END
#else 


	float* t = &m[0][0];
	for (int i = 0; i < 16; i++)
		t[i] = 0.0f;

#endif // DPVS_PS2_ASSEMBLY
}

DPVS_FORCE_INLINE float Matrix4x4::getMaxScale (void) const
{
#if defined (DPVS_PS2_VU_ASSEMBLY)
//PS2-BEGIN
	DPVS_ASSERT( is128Aligned(this) );
	register float ret;
	asm __volatile__ ("lqc2		vf11, 0(%1);lqc2 vf12, 0x10(%1);lqc2 vf13, 0x20(%1); vmula.xyz	ACC, vf11, vf11; vmadda.xyz	ACC, vf12, vf12; vmadd.xyz vf1, vf13, vf13; vmaxy.x	vf2, vf1, vf1; vmaxz.x vf2, vf2, vf1;qmfc2	%1, vf2;mtc1 %1, %0; sqrt.s	%0, %0" : "=f"(ret) : "r"(&m[0][0]) );
	return ret;
//PS2-END
#else

	float xs = (m[0][0]*m[0][0] + m[1][0]*m[1][0] + m[2][0]*m[2][0]); 
	float ys = (m[0][1]*m[0][1] + m[1][1]*m[1][1] + m[2][1]*m[2][1]); 
	float zs = (m[0][2]*m[0][2] + m[1][2]*m[1][2] + m[2][2]*m[2][2]); 

	float maxScale = Math::max(xs,ys);
	return Math::sqrt(Math::max(maxScale,zs)); 

#endif // DPVS_PS2_VU_ASSEMBLY
}

DPVS_FORCE_INLINE void Matrix4x3::productFromLeft (const Matrix4x3& n)
{
	DPVS_ASSERT(&n != this);

#if defined (DPVS_PS2_VU_ASSEMBLY)
//PS2-BEGIN

	DPVS_ASSERT( is128Aligned(&n) );
	DPVS_ASSERT( is128Aligned(this) );

/*
	m1[0][0] = n.m[0][0]*m[0][0] + n.m[0][1]*m[1][0] + n.m[0][2]*m[2][0];
	m1[0][1] = n.m[0][0]*m[0][1] + n.m[0][1]*m[1][1] + n.m[0][2]*m[2][1];
	m1[0][2] = n.m[0][0]*m[0][2] + n.m[0][1]*m[1][2] + n.m[0][2]*m[2][2];
	m1[0][3] = n.m[0][0]*m[0][3] + n.m[0][1]*m[1][3] + n.m[0][2]*m[2][3] + n.m[0][3];

	m1[1][0] = n.m[1][0]*m[0][0] + n.m[1][1]*m[1][0] + n.m[1][2]*m[2][0];
	m1[1][1] = n.m[1][0]*m[0][1] + n.m[1][1]*m[1][1] + n.m[1][2]*m[2][1];
	m1[1][2] = n.m[1][0]*m[0][2] + n.m[1][1]*m[1][2] + n.m[1][2]*m[2][2];
	m1[1][3] = n.m[1][0]*m[0][3] + n.m[1][1]*m[1][3] + n.m[1][2]*m[2][3] + n.m[1][3];

	m1[2][0] = n.m[2][0]*m[0][0] + n.m[2][1]*m[1][0] + n.m[2][2]*m[2][0];
	m1[2][1] = n.m[2][0]*m[0][1] + n.m[2][1]*m[1][1] + n.m[2][2]*m[2][1];
	m1[2][2] = n.m[2][0]*m[0][2] + n.m[2][1]*m[1][2] + n.m[2][2]*m[2][2];
	m1[2][3] = n.m[2][0]*m[0][3] + n.m[2][1]*m[1][3] + n.m[2][2]*m[2][3] + n.m[2][3];
	*/

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
	"\n	sqc2			vf1, 0x0(%0)"
	"\n	vmulax.xyzw		ACC, vf11, vf16"
	"\n	vmadday.xyzw	ACC, vf12, vf16"
	"\n	vmaddaz.xyzw	ACC, vf13, vf16"
	"\n	vmaddw.xyzw		vf1, vf0, vf16"
	"\n	sqc2			vf1, 0x10(%0)"
	"\n	vmulax.xyzw		ACC, vf11, vf17"
	"\n	vmadday.xyzw	ACC, vf12, vf17"
	"\n	vmaddaz.xyzw	ACC, vf13, vf17"
	"\n	vmaddw.xyzw		vf1, vf0, vf17"
	"\n	sqc2			vf1, 0x20(%0)"
	 : : "r"(&m[0][0]), "r"(&n.m[0][0]) );
//PS2-END
#else

	for (int i =0; i < 4; i++)
	{
		register float a = m[0][i], b = m[1][i], c = m[2][i];
		m[0][i] = n.m[0][0]*a + n.m[0][1]*b + n.m[0][2]*c;
		m[1][i] = n.m[1][0]*a + n.m[1][1]*b + n.m[1][2]*c;
		m[2][i] = n.m[2][0]*a + n.m[2][1]*b + n.m[2][2]*c;
	}
	m[0][3] += n[0][3];
	m[1][3] += n[1][3];
	m[2][3] += n[2][3];

#endif
}

DPVS_FORCE_INLINE bool Matrix4x3::operator== (const Matrix4x3& n) const
{
	return ((*this)[0]==n[0] && (*this)[1]==n[1] && (*this)[2]==n[2]);
}

DPVS_FORCE_INLINE float Matrix4x3::getMaxScale (void) const
{
	register float xs = (m[0][0]*m[0][0] + m[1][0]*m[1][0] + m[2][0]*m[2][0]); 
	register float ys = (m[0][1]*m[0][1] + m[1][1]*m[1][1] + m[2][1]*m[2][1]); 
	register float zs = (m[0][2]*m[0][2] + m[1][2]*m[1][2] + m[2][2]*m[2][2]); 
	return Math::sqrt(Math::max(Math::max(xs,ys),zs)); 
}


DPVS_FORCE_INLINE void Matrix4x4::transpose (const Matrix4x4& src)
{
#if defined (DPVS_PS2_ASSEMBLY)
//PS2-BEGIN
	// This version survives if &src==this
	asm __volatile__ (
	"	lq		$8, 0(%0)"
	"\n	lq		$9, 0x10(%0)"
	"\n	lq		$10, 0x20(%0)"
	"\n	lq		$11, 0x30(%0)"

	"\n	pextlw	$12, $9, $8"
	"\n	pextuw	$13, $9, $8"
	"\n	pextlw	$14, $11, $10"
	"\n	pextuw	$15, $11, $10"

	"\n	pcpyld	$8, $14, $12"
	"\n	pcpyud	$9, $12, $14"
	"\n	pcpyld	$10, $15, $13"
	"\n	pcpyud	$11, $13, $15"

	"\n	sq		$8, 0(%1)"
	"\n	sq		$9, 0x10(%1)"
	"\n	sq		$10, 0x20(%1)"
	"\n	sq		$11, 0x30(%1)"
	 : : "r"(&src.m[0][0]), "r"(&m[0][0]) : "$8", "$9", "$10", "$11", "$12", "$13", "$14", "$15" );
//PS2-END
#else

	if (&src == this)
	{
		this->transpose();
		return;
	}

	m[0][0] = src[0][0];
	m[0][1] = src[1][0];
	m[0][2] = src[2][0];
	m[0][3] = src[3][0];
	m[1][0] = src[0][1];
	m[1][1] = src[1][1];
	m[1][2] = src[2][1];
	m[1][3] = src[3][1];
	m[2][0] = src[0][2];
	m[2][1] = src[1][2];
	m[2][2] = src[2][2];
	m[2][3] = src[3][2];
	m[3][0] = src[0][3];
	m[3][1] = src[1][3];
	m[3][2] = src[2][3];
	m[3][3] = src[3][3];

#endif // DPVS_PS2_ASSEMBLY
}

DPVS_FORCE_INLINE bool Matrix4x4::operator== (const Matrix4x4& n) const
{
	const float* d = (const float*)&(m[0][0]);
	const float* s = (const float*)&(n[0][0]);
	for (int i = 0; i < 16; i++)
	if (d[i]!=s[i])
		return false;
	return true;
}

DPVS_FORCE_INLINE void Matrix4x4::productFromLeft (const Matrix4x4& n)
{
	DPVS_ASSERT(&n != this);

#if defined (DPVS_PS2_VU_ASSEMBLY)
//PS2-BEGIN

	DPVS_ASSERT( is128Aligned(&n) );
	DPVS_ASSERT( is128Aligned(this) );

	/*
	m1[0][0] = n.m[0][0]*m[0][0] + n.m[0][1]*m[1][0] + n.m[0][2]*m[2][0];
	m1[0][1] = n.m[0][0]*m[0][1] + n.m[0][1]*m[1][1] + n.m[0][2]*m[2][1];
	m1[0][2] = n.m[0][0]*m[0][2] + n.m[0][1]*m[1][2] + n.m[0][2]*m[2][2];
	m1[0][3] = n.m[0][0]*m[0][3] + n.m[0][1]*m[1][3] + n.m[0][2]*m[2][3] + n.m[0][3];

	m1[1][0] = n.m[1][0]*m[0][0] + n.m[1][1]*m[1][0] + n.m[1][2]*m[2][0];
	m1[1][1] = n.m[1][0]*m[0][1] + n.m[1][1]*m[1][1] + n.m[1][2]*m[2][1];
	m1[1][2] = n.m[1][0]*m[0][2] + n.m[1][1]*m[1][2] + n.m[1][2]*m[2][2];
	m1[1][3] = n.m[1][0]*m[0][3] + n.m[1][1]*m[1][3] + n.m[1][2]*m[2][3] + n.m[1][3];

	m1[2][0] = n.m[2][0]*m[0][0] + n.m[2][1]*m[1][0] + n.m[2][2]*m[2][0];
	m1[2][1] = n.m[2][0]*m[0][1] + n.m[2][1]*m[1][1] + n.m[2][2]*m[2][1];
	m1[2][2] = n.m[2][0]*m[0][2] + n.m[2][1]*m[1][2] + n.m[2][2]*m[2][2];
	m1[2][3] = n.m[2][0]*m[0][3] + n.m[2][1]*m[1][3] + n.m[2][2]*m[2][3] + n.m[2][3];

	m1[3][0] = n.m[3][0]*m[0][0] + n.m[3][1]*m[1][0] + n.m[3][2]*m[2][0];
	m1[3][1] = n.m[3][0]*m[0][1] + n.m[3][1]*m[1][1] + n.m[3][2]*m[2][1];
	m1[3][2] = n.m[3][0]*m[0][2] + n.m[3][1]*m[1][2] + n.m[3][2]*m[2][2];
	m1[3][3] = n.m[3][0]*m[0][3] + n.m[3][1]*m[1][3] + n.m[3][2]*m[2][3] + n.m[3][3];
	*/

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
	"\n	sqc2			vf1, 0x0(%0)"
	"\n	vmulax.xyzw		ACC, vf11, vf16"
	"\n	vmadday.xyzw	ACC, vf12, vf16"
	"\n	vmaddaz.xyzw	ACC, vf13, vf16"
	"\n	vmaddw.xyzw		vf1, vf14, vf16"
	"\n	sqc2			vf1, 0x10(%0)"
	"\n	vmulax.xyzw		ACC, vf11, vf17"
	"\n	vmadday.xyzw	ACC, vf12, vf17"
	"\n	vmaddaz.xyzw	ACC, vf13, vf17"
	"\n	vmaddw.xyzw		vf1, vf14, vf17"
	"\n	sqc2			vf1, 0x20(%0)"
	"\n	vmulax.xyzw		ACC, vf11, vf18"
	"\n	vmadday.xyzw	ACC, vf12, vf18"
	"\n	vmaddaz.xyzw	ACC, vf13, vf18"
	"\n	vmaddw.xyzw		vf1, vf14, vf18"
	"\n	sqc2			vf1, 0x30(%0)"
	 : : "r"(&m[0][0]), "r"(&n.m[0][0]) );
//PS2-END
#else

	for (int i = 0; i < 4; i++)
	{
		register float a = m[0][i], b = m[1][i], c = m[2][i], d = m[3][i];
		m[0][i] = n.m[0][0]*a + n.m[0][1]*b + n.m[0][2]*c + n.m[0][3]*d;
		m[1][i] = n.m[1][0]*a + n.m[1][1]*b + n.m[1][2]*c + n.m[1][3]*d;
		m[2][i] = n.m[2][0]*a + n.m[2][1]*b + n.m[2][2]*c + n.m[2][3]*d;
		m[3][i] = n.m[3][0]*a + n.m[3][1]*b + n.m[3][2]*c + n.m[3][3]*d;
	}

#endif
}

DPVS_FORCE_INLINE bool Matrix4x4::isUniform (void) const
{
#if defined (DPVS_PS2_VU_ASSEMBLY)
//PS2-BEGIN

	register float err = 0.0001f;
	register int ret = 0;
	asm __volatile__ (
	"	addi		%0, $0, 1"
	"\n	vmr32		vf1, vf0"
	"\n	vmr32		vf2, vf1"
	"\n	qmfc2		$9, vf2"
	"\n	ld			$8, 0x30(%1)"
	"\n	bne			$8, $0, isUniform_notUniform"
	"\n	nop"
	"\n	ld			$8, 0x38(%1)"
	"\n	bne			$8, $9, isUniform_notUniform"
	"\n	nop"
		
	"\n	lqc2		vf11, 0x0(%1)"
	"\n	lqc2		vf12, 0x10(%1)"
	"\n	lqc2		vf13, 0x20(%1)"
	"\n	vmul.xyz	vf1, vf11, vf11"
	"\n	vmul.xyz	vf2, vf12, vf12"
	"\n	vmul.xyz	vf3, vf13, vf13"
	"\n	vadd.xyz	vf2, vf2, vf3"
	"\n	vadd.xyz	vf1, vf1, vf2"
		
	"\n	qmfc2		$8, vf1"
	"\n	vmr32.xyzw	vf2, vf1"
	"\n	mtc1		$8, $f1"
	"\n	qmfc2		$8, vf2"
	"\n	vmr32.xyzw	vf1, vf2"
	"\n	mtc1		$8, $f2"
	"\n	qmfc2		$8, vf1"
	"\n	mtc1		$8, $f3"
	"\n	sqrt.s		$f1, $f1"
	"\n	sqrt.s		$f2, $f2"
	"\n	sqrt.s		$f3, $f3"

	"\n	mul.s		$f4, $f1, %2"
	"\n	sub.s		$f5, $f1, $f2"
	"\n	abs.s		$f5, $f5"
	"\n	c.le.s		$f5, $f4"
	"\n	bc1f		isUniform_notUniform"
	"\n	nop"
	"\n	sub.s		$f5, $f1, $f3"
	"\n	abs.s		$f5, $f5"
	"\n	c.le.s		$f5, $f4"
	"\n	bc1f		isUniform_notUniform"
	"\n	nop"
	"\n	addi		%0, $0, 0"
"\n	isUniform_notUniform:"
	 : "=r"(ret) : "r"(&m[0][0]), "f"(err) : "$8", "$9", "$f1", "$f2", "$f3", "$f4", "$f5" );
	return 0 == ret;
//PS2-END
#else

	if (!is4x3Matrix())
		return false;	// not 4x3
	float xs = Math::sqrt(m[0][0]*m[0][0] + m[1][0]*m[1][0] + m[2][0]*m[2][0]); 
	float ys = Math::sqrt(m[0][1]*m[0][1] + m[1][1]*m[1][1] + m[2][1]*m[2][1]); 
	float zs = Math::sqrt(m[0][2]*m[0][2] + m[1][2]*m[1][2] + m[2][2]*m[2][2]); 
	return (Math::fabs(xs-ys) <= xs*0.0001f && Math::fabs(xs-zs) <= xs*0.0001f);

#endif
}

DPVS_FORCE_INLINE Matrix4x4d& Matrix4x4d::operator= (const Matrix4x4& ss)
{
	double*			d = &(*this)[0][0];
	const float*	s = &ss[0][0];
	for (int i = 0 ; i < 16; i++)
		Math::floatToDouble(d[i],s[i]);
	return *this;
}

DPVS_FORCE_INLINE Matrix4x4& Matrix4x4::operator= (const Matrix4x4d& ss)
{
	float*			d = &(*this)[0][0];
	const double*	s = &ss[0][0];
	for (int i = 0 ; i < 16; i++)
		Math::doubleToFloat(d[i],s[i]);
	return *this;
}

} // DPVS

//------------------------------------------------------------------------
#endif // __DPVSMATRIX_HPP
