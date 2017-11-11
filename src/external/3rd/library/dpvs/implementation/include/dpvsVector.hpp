#ifndef __DPVSVECTOR_HPP
#define __DPVSVECTOR_HPP
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
 * Description: 	Vector classes used in the public API
 *
 * $Archive: /dpvs/implementation/include/dpvsVector.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 12.02.02 15:02 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#if !defined (__DPVSBASEMATH_HPP)
#	include "dpvsBaseMath.hpp"
#endif

namespace DPVS
{

/******************************************************************************
 *
 * Class:			DPVS::Vector2i
 *
 * Description:		Two-component int32 vector
 *
 *****************************************************************************/

class Vector2i 
{
public:
	INT32 i,j;
DPVS_FORCE_INLINE				Vector2i	(void) : i(0),j(0)				{}
DPVS_FORCE_INLINE				Vector2i	(const Vector2i& s)				: i(s.i),j(s.j)	{ }
DPVS_FORCE_INLINE				Vector2i	(int ci, int cj)				: i(ci),j(cj)  {}
DPVS_FORCE_INLINE const INT32&	operator[]	(int n) const					{ DPVS_ASSERT (n>=0 && n < 2);  return ((const INT32*)this)[n]; }
DPVS_FORCE_INLINE INT32&		operator[]	(int n)							{ DPVS_ASSERT (n>=0 && n < 2);  return ((INT32*)this)[n]; }
DPVS_FORCE_INLINE bool			operator==	(const Vector2i& v) const		{ return (i == v.i && j == v.j);	}
DPVS_FORCE_INLINE bool			operator!=	(const Vector2i& v) const		{ return !(i == v.i && j == v.j);	}
//	Vector2i&		operator+=	(const Vector2i& v)				{ i += v.i; j += v.j; return *this; }
//	Vector2i&		operator-=	(const Vector2i& v)				{ i -= v.i; j -= v.j; return *this; }
};


/******************************************************************************
 *
 * Class:			DPVS::Vector3i
 *
 * Description:		Three-component int32 vector
 *
 *****************************************************************************/

class Vector3i
{
public:
	INT32 i,j,k;
DPVS_FORCE_INLINE				Vector3i	(void) : i(0),j(0),k(0)			{}
DPVS_FORCE_INLINE				Vector3i	(const Vector3i& s)				: i(s.i),j(s.j),k(s.k) {}
DPVS_FORCE_INLINE				Vector3i	(int ci, int cj, int ck)		: i(ci),j(cj),k(ck) {}
DPVS_FORCE_INLINE const INT32&	operator[]	(int n) const					{ DPVS_ASSERT (n>=0 && n < 3);  return ((const INT32*)this)[n]; }
DPVS_FORCE_INLINE INT32&		operator[]	(int n)							{ DPVS_ASSERT (n>=0 && n < 3);  return ((INT32*)this)[n]; }
DPVS_FORCE_INLINE Vector3i&		operator=	(const Vector3i& v)				{ i = v.i; j = v.j; k = v.k;  return *this; }
DPVS_FORCE_INLINE bool			operator==	(const Vector3i& v) const		{ return (i == v.i && j == v.j && k == v.k);	}
DPVS_FORCE_INLINE bool			operator!=	(const Vector3i& v) const		{ return !(i == v.i && j == v.j && k == v.k);	}
//	Vector3i&		operator+=	(const Vector3i& v)				{ i += v.i; j += v.j; k += v.k; return *this; }
//	Vector3i&		operator-=	(const Vector3i& v)				{ i -= v.i; j -= v.j; k -= v.k; return *this; }
};


/******************************************************************************
 *
 * Class:			DPVS::Vector4i
 *
 * Description:		Four-component int32 vector
 *
 *****************************************************************************/

class Vector4i 
{
public:
	INT32 i,j,k,l;
DPVS_FORCE_INLINE				Vector4i		(void)							: i(0),j(0),k(0),l(0)	{}
DPVS_FORCE_INLINE				Vector4i		(const Vector4i& s)				: i(s.i),j(s.j),k(s.k),l(s.l) {}
DPVS_FORCE_INLINE				Vector4i		(int ci, int cj, int ck, int cl): i(ci),j(cj),k(ck),l(cl) {}
DPVS_FORCE_INLINE const INT32&	operator[]		(int n) const					{ DPVS_ASSERT (n>=0 && n < 4);  return ((const INT32*)this)[n]; }
DPVS_FORCE_INLINE INT32&		operator[]		(int n)							{ DPVS_ASSERT (n>=0 && n < 4);  return ((INT32*)this)[n]; }
DPVS_FORCE_INLINE bool			operator==		(const Vector4i& v) const		{ return (i == v.i && j == v.j && k == v.k && l == v.l);	}
DPVS_FORCE_INLINE bool			operator!=		(const Vector4i& v) const		{ return !(i == v.i && j == v.j && k == v.k && l == v.l);	}
//	Vector4i&		operator+=	(const Vector4i& v)				{ i += v.i; j += v.j; k += v.k; l += v.l; return *this; }
//	Vector4i&		operator-=	(const Vector4i& v)				{ i -= v.i; j -= v.j; k -= v.k; l -= v.l; return *this; }
};


/******************************************************************************
 *
 * Class:			DPVS::Vector2
 *
 * Description:		Two-component vector 
 *
 *****************************************************************************/

class Vector2
{
public:
#if defined (DPVS_CPU_PS2) && defined (DPVS_ALIGN_VECTORS)
//PS2-BEGIN
	float DPVS_ALIGN8(x);														// make data 8-byte aligned..
//PS2-END
#else
	float x;
#endif
	float y;	

DPVS_FORCE_INLINE				Vector2			(void)							: x(0.0f),y(0.0f)		{}
DPVS_FORCE_INLINE explicit		Vector2 		(DisableConstructor)							{}
DPVS_FORCE_INLINE				Vector2			(float cx, float cy)			: x(cx),y(cy) {}
DPVS_FORCE_INLINE const float&	operator[]		(int i) const					{ DPVS_ASSERT (i>=0 && i < 2); return ((const float*)this)[i]; }
DPVS_FORCE_INLINE float&		operator[]		(int i)							{ DPVS_ASSERT (i>=0 && i < 2); return ((float*)this)[i]; }
DPVS_FORCE_INLINE bool			operator==		(const Vector2& s) const		{ return (x==s.x)&&(y==s.y); }
DPVS_FORCE_INLINE bool			operator!=		(const Vector2& s) const		{ return (x!=s.x)||(y!=s.y); }
};

DPVS_CT_ASSERT(sizeof(Vector2)==8);

/******************************************************************************
 *
 * Class:			DPVS::Vector3
 *
 * Description:		Three-component vector
 *
 *****************************************************************************/

class Vector3 
{
public:
	float	x,y,z;																		// data members

DPVS_FORCE_INLINE				Vector3		(void) : x(0.0f),y(0.0f),z(0.0f)		{}
DPVS_FORCE_INLINE  explicit		Vector3		(DisableConstructor)							{}
DPVS_FORCE_INLINE				Vector3		(const Vector3& s)						{ x = s.x; y = s.y; z = s.z; }
DPVS_FORCE_INLINE				Vector3		(float cx, float cy, float cz)			: x(cx),y(cy),z(cz) {}
DPVS_FORCE_INLINE  explicit		Vector3		(const Vector3d&);
DPVS_FORCE_INLINE  Vector3&		operator=	(const Vector3& s)						{ x = s.x; y = s.y; z = s.z; return *this; }
DPVS_FORCE_INLINE  const float&	operator[]	(int i) const							{ DPVS_ASSERT (i>=0 && i < 3); return ((const float*)this)[i]; }
DPVS_FORCE_INLINE  float&		operator[]	(int i)									{ DPVS_ASSERT (i>=0 && i < 3); return ((float*)this)[i]; }
DPVS_FORCE_INLINE  Vector3&		clear		(void)									{ x = 0.0f; y = 0.0f; z = 0.0f; return *this; }
DPVS_FORCE_INLINE  Vector3&		make		(float cx, float cy, float cz)			{ x = cx;  y = cy;  z = cz; return *this;	}
DPVS_FORCE_INLINE  Vector3&		operator+=	(const Vector3& v)						{ x += v.x, y += v.y, z += v.z; return *this; }
DPVS_FORCE_INLINE  Vector3&		operator-=	(const Vector3& v)						{ x -= v.x, y -= v.y, z -= v.z; return *this; }
DPVS_FORCE_INLINE  Vector3&		operator*=	(const Vector3& v)						{ x = (x*v.x), y = (y*v.y), z = (z*v.z); return *this; }
DPVS_FORCE_INLINE  Vector3&		operator/=	(const Vector3& v)						{ x = (x/v.x), y = (y/v.y), z = (z/v.z); return *this; }
DPVS_FORCE_INLINE  Vector3&		operator*=	(float s)								{ x = (x*s),   y = (y*s), z = (z*s); return *this; }
DPVS_FORCE_INLINE  Vector3&		operator/=	(float s)								{ s = (1.0f/s); x = (x*s), y = (y*s), z = (z*s); return *this; }
DPVS_FORCE_INLINE  bool			operator==	(const Vector3& v) const				{ return (x == v.x && y == v.y && z == v.z);	}
DPVS_FORCE_INLINE  bool			operator!=	(const Vector3& v) const				{ return !(x == v.x && y == v.y && z == v.z);	}
DPVS_FORCE_INLINE  float		length		(void) const							{ return Math::sqrt(x*x+y*y+z*z); }
DPVS_FORCE_INLINE  float		lengthSqr	(void) const							{ return (x*x+y*y+z*z); }
DPVS_FORCE_INLINE  Vector3&		normalize	(float len = 1.0f)						{ float l = x*x+y*y+z*z; if(l!=0.0f) { l = len / Math::sqrt(l); x*=l; y*=l; z*=l; } return *this; }
};

DPVS_FORCE_INLINE	Vector3			operator+			(const Vector3& v1,	const Vector3& v2)		{ return Vector3(v1.x+v2.x, v1.y+v2.y, v1.z+v2.z); }
DPVS_FORCE_INLINE	Vector3			operator-			(const Vector3& v1,	const Vector3& v2)		{ return Vector3(v1.x-v2.x, v1.y-v2.y, v1.z-v2.z); }
DPVS_FORCE_INLINE	Vector3			operator*			(const Vector3& v,	const float s)			{ return Vector3(v.x*s, v.y*s, v.z*s); }
DPVS_FORCE_INLINE	Vector3			operator*			(const float s,		const Vector3& v)		{ return Vector3(v.x*s, v.y*s, v.z*s); }
DPVS_FORCE_INLINE	Vector3			operator/			(const Vector3& v,	const float s)			{ return v*(1.0f/s); }
DPVS_FORCE_INLINE	Vector3			operator-			(const Vector3& v)							{ return Vector3(-v.x, -v.y, -v.z); }
DPVS_FORCE_INLINE	Vector3			cross				(const Vector3& v1,	const Vector3& v2)		{ return Vector3 ((v1.y*v2.z)-(v1.z*v2.y), (v1.z*v2.x)-(v1.x*v2.z), (v1.x*v2.y)-(v1.y*v2.x)); }
DPVS_FORCE_INLINE	Vector3			min					(const Vector3& v1, const Vector3& v2)		{ return Vector3 (Math::min(v1.x,v2.x),Math::min(v1.y,v2.y),Math::min(v1.z,v2.z)); }
DPVS_FORCE_INLINE	Vector3			max					(const Vector3& v1, const Vector3& v2)		{ return Vector3 (Math::max(v1.x,v2.x),Math::max(v1.y,v2.y),Math::max(v1.z,v2.z)); }
//PS2-BEGIN
/*
#if defined (DPVS_PS2_ASSEMBLY)
DPVS_FORCE_INLINE  float			dot					(const Vector3& v1, const Vector3& v2)		{ register float ret;asm ("lwc1	$f10, 0(%1); lwc1	$f11, 4(%1);lwc1	$f12, 8(%1);lwc1	$f20, 0(%2);lwc1	$f21, 4(%2);lwc1	$f22, 8(%2);mula.s	$f10, $f20;madda.s	$f11, $f21;madd.s	%0, $f12, $f22" : "=f"(ret) : "r"(&v1), "r"(&v2) : "$f10", "$f11", "$f12", "$f20", "$f21", "$f22" );return ret;}
#else
*/
//PS2-END
DPVS_FORCE_INLINE	float			dot					(const Vector3& v1,	const Vector3& v2) 		{ return (v1.x*v2.x + v1.y*v2.y + v1.z*v2.z); }
//#endif

DPVS_CT_ASSERT(sizeof(Vector3)==12);

/******************************************************************************
 *
 * Class:			DPVS::Vector3d
 *
 * Description:		Three-component double-precision vector
 *
 *****************************************************************************/

class Vector3d 
{
public:
	double	x,y,z;																		// data members

DPVS_FORCE_INLINE					Vector3d	(void) : x(0.0),y(0.0),z(0.0)		{}
DPVS_FORCE_INLINE					Vector3d	(double cx, double cy, double cz)		: x(cx),y(cy),z(cz) {}
DPVS_FORCE_INLINE 	explicit		Vector3d	(const Vector3& s)						{ x = s.x; y = s.y; z = s.z; }
DPVS_FORCE_INLINE	const double&	operator[]	(int i) const							{ DPVS_ASSERT (i>=0 && i < 3); return ((const double*)this)[i]; }
DPVS_FORCE_INLINE	double&			operator[]	(int i)									{ DPVS_ASSERT (i>=0 && i < 3); return ((double*)this)[i]; }
DPVS_FORCE_INLINE	Vector3d&		clear		(void)									{ x = 0.0f; y = 0.0f; z = 0.0f; return *this; }
DPVS_FORCE_INLINE	Vector3d&		make		(double cx, double cy, double cz)		{ x = cx;  y = cy;  z = cz; return *this;	}
DPVS_FORCE_INLINE	Vector3d&		operator+=	(const Vector3d& v)						{ x += v.x, y += v.y, z += v.z; return *this; }
DPVS_FORCE_INLINE	Vector3d&		operator-=	(const Vector3d& v)						{ x -= v.x, y -= v.y, z -= v.z; return *this; }
DPVS_FORCE_INLINE	Vector3d&		operator*=	(const Vector3d& v)						{ x = (x*v.x), y = (y*v.y), z = (z*v.z); return *this; }
DPVS_FORCE_INLINE	Vector3d&		operator/=	(const Vector3d& v)						{ x = (x/v.x), y = (y/v.y), z = (z/v.z); return *this; }
DPVS_FORCE_INLINE	Vector3d&		operator*=	(double s)								{ x = (x*s),   y = (y*s), z = (z*s); return *this; }
DPVS_FORCE_INLINE	Vector3d&		operator/=	(double s)								{ s = (1.0f/s); x = (x*s), y = (y*s), z = (z*s); return *this; }
DPVS_FORCE_INLINE	bool			operator==	(const Vector3d& v) const				{ return (x == v.x && y == v.y && z == v.z);	}
DPVS_FORCE_INLINE	bool			operator!=	(const Vector3d& v) const				{ return !(x == v.x && y == v.y && z == v.z);	}
DPVS_FORCE_INLINE	double			length		(void) const							{ return (double)sqrt(x*x+y*y+z*z); }
DPVS_FORCE_INLINE	double			lengthSqr	(void) const							{ return (x*x+y*y+z*z); }
DPVS_FORCE_INLINE	Vector3d&		normalize	(double len = 1.0f)						{ double l = x*x+y*y+z*z; if(l!=0.0) { l = len / ::sqrt(l); x = (double)(x*l); y = (double)(y*l); z = (double)(z*l); } return *this; }
};

DPVS_FORCE_INLINE					Vector3::Vector3		(const Vector3d& s)								{ x = (float)s.x; y = (float)s.y; z = (float)s.z; }
DPVS_FORCE_INLINE	Vector3d		operator+				(const Vector3d& v1,	const Vector3d& v2)		{ return Vector3d(v1.x+v2.x, v1.y+v2.y, v1.z+v2.z); }
DPVS_FORCE_INLINE	Vector3d		operator-				(const Vector3d& v1,	const Vector3d& v2)		{ return Vector3d(v1.x-v2.x, v1.y-v2.y, v1.z-v2.z); }
DPVS_FORCE_INLINE	Vector3d		operator*				(const Vector3d& v,		const double s)			{ return Vector3d(v.x*s, v.y*s, v.z*s); }
DPVS_FORCE_INLINE	Vector3d		operator*				(const double s,		const Vector3d& v)		{ return Vector3d(v.x*s, v.y*s, v.z*s); }
DPVS_FORCE_INLINE	Vector3d		operator/				(const Vector3d& v,		const double s)			{ return v*(1.0f/s); }
DPVS_FORCE_INLINE	Vector3d		operator-				(const Vector3d& v)								{ return Vector3d(-v.x, -v.y, -v.z); }
DPVS_FORCE_INLINE	double			dot						(const Vector3d& v1,	const Vector3d& v2) 	{ return (v1.x*v2.x + v1.y*v2.y + v1.z*v2.z); }
DPVS_FORCE_INLINE	Vector3d		cross					(const Vector3d& v1,	const Vector3d& v2)		{ return Vector3d ((v1.y*v2.z)-(v1.z*v2.y), (v1.z*v2.x)-(v1.x*v2.z), (v1.x*v2.y)-(v1.y*v2.x)); }

/******************************************************************************
 *
 * Class:			DPVS::Vector4
 *
 * Description:		Four-component vector 
 *
 *****************************************************************************/

class Vector4
{
public:
	float DPVS_VECTOR_ALIGN(x);	// x-component (makes sure vector4 is 16-byte aligned)
	float						y;
	float						z;
	float						w;		

DPVS_FORCE_INLINE					Vector4		(void)											: x(0),y(0),z(0),w(0)		{ DPVS_CHECK_ALIGN(this);}
DPVS_FORCE_INLINE	explicit		Vector4		(DisableConstructor)							{DPVS_CHECK_ALIGN(this);}
DPVS_FORCE_INLINE					Vector4		(float cx, float cy, float cz, float ch)		: x(cx),y(cy),z(cz),w(ch) { DPVS_CHECK_ALIGN(this);}
DPVS_FORCE_INLINE					Vector4		(const Vector3& v, float ch)		: x(v.x),y(v.y),z(v.z),w(ch) { DPVS_CHECK_ALIGN(this);}
DPVS_FORCE_INLINE	Vector4&		clear		(void)											{ x = (0); y = (0); z = (0); w = (0); return *this; }
DPVS_FORCE_INLINE	Vector4&		make		(float cx, float cy, float cz, float ch)		{ x = (cx), y = (cy), z = (cz), w = (ch); return *this; }
DPVS_FORCE_INLINE	Vector4&		make		(const Vector3& v, float ch)					{ x = v.x, y = v.y, z = v.z, w = (ch); return *this; }
DPVS_FORCE_INLINE	bool			operator==	(const Vector4& v) const						{ return (x == v.x && y == v.y && z == v.z && w == v.w);	}
DPVS_FORCE_INLINE	bool			operator!=	(const Vector4& v) const						{ return !(x == v.x && y == v.y && z == v.z && w == v.w);	}
DPVS_FORCE_INLINE	Vector4&		operator+=	(const Vector4& v)								{ x += v.x, y += v.y, z += v.z, w += v.w; return *this;		}
DPVS_FORCE_INLINE	Vector4&		operator-=	(const Vector4& v)								{ x -= v.x, y -= v.y, z -= v.z, w -= v.w; return *this;		}
DPVS_FORCE_INLINE	Vector4&		operator*=	(float s)										{ x = (x*s), y = (y*s), z = (z*s), w = (w*s); return *this; }
DPVS_FORCE_INLINE	Vector4&		operator*=	(const Vector4& v)								{ x = (x*v.x), y = (y*v.y), z = (z*v.z); w = (w*v.w); return *this; }
DPVS_FORCE_INLINE	float			length		(void) const									{ return Math::sqrt( x*x+y*y+z*z+w*w ); }
DPVS_FORCE_INLINE	float			lengthSqr	(void) const									{ return ( x*x+y*y+z*z+w*w ); }
DPVS_FORCE_INLINE	Vector4&		normalize	(float len = 1.0f)								{ float l = length();	if(l!=0.0f) *this *= (len/l); return *this; }
DPVS_FORCE_INLINE	const float&	operator[]	(int i) const									{ DPVS_ASSERT (i>=0 && i < 4); return ((const float*)this)[i]; }
DPVS_FORCE_INLINE	float&			operator[]	(int i)											{ DPVS_ASSERT (i>=0 && i < 4); return ((float*)this)[i]; }
};

DPVS_FORCE_INLINE 	Vector4			operator+	(const Vector4& v1, const Vector4& v2)	{ return Vector4(v1.x+v2.x, v1.y+v2.y, v1.z+v2.z, v1.w+v2.w); }
DPVS_FORCE_INLINE 	Vector4			operator-	(const Vector4& v1, const Vector4& v2)	{ return Vector4(v1.x-v2.x, v1.y-v2.y, v1.z-v2.z, v1.w-v2.w); }
DPVS_FORCE_INLINE	Vector4			operator*	(const Vector4& v,	float s)			{ return Vector4(v.x*s, v.y*s, v.z*s, v.w*s); }
DPVS_FORCE_INLINE	Vector4			operator*	(float s,			const Vector4& v)	{ return v*s; }
DPVS_FORCE_INLINE	Vector4			operator/	(const Vector4& v,	float s)			{ DPVS_ASSERT(s!=0.0f); float r = 1.0f/s; return v*r; }
DPVS_FORCE_INLINE	Vector4			operator-	(const Vector4& v)						{ return Vector4(-v.x, -v.y, -v.z, -v.w); }
DPVS_FORCE_INLINE	Vector4			min			(const Vector4& v1, const Vector4& v2)		{ return Vector4 (Math::min(v1.x,v2.x),Math::min(v1.y,v2.y),Math::min(v1.z,v2.z),Math::min(v1.w,v2.w)); }
DPVS_FORCE_INLINE	Vector4			max			(const Vector4& v1, const Vector4& v2)		{ return Vector4 (Math::max(v1.x,v2.x),Math::max(v1.y,v2.y),Math::max(v1.z,v2.z),Math::max(v1.w,v2.w)); }
//PS2-BEGIN
/*
#if defined (DPVS_PS2_ASSEMBLY)
DPVS_FORCE_INLINE  float			dot			(const Vector4& v1, const Vector4& v2)		{ register float ret; asm ("lwc1	$f10, 0(%1); lwc1	$f11, 4(%1); lwc1	$f12, 8(%1); lwc1	$f13, 12(%1); lwc1	$f20, 0(%2); lwc1	$f21, 4(%2); lwc1	$f22, 8(%2); lwc1	$f23, 12(%2); mula.s	$f10, $f20; madda.s	$f11, $f21; madda.s	$f12, $f22; madd.s	%0, $f13, $f23" : "=f"(ret) : "r"(&v1), "r"(&v2) : "$f10", "$f11", "$f12", "$f13", "$f20", "$f21", "$f22", "$f23" ); return ret;}
#else
*/
//PS2-END
DPVS_FORCE_INLINE	float			dot			(const Vector4& v1, const Vector4& v2) 		{ return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z + v1.w*v2.w; }
//#endif

DPVS_CT_ASSERT(sizeof(Vector4)==16);

/******************************************************************************
 *
 * Class:			DPVS::Vector4d
 *
 * Description:		Four-component double-precision vector (used only for
 *					storage -- no real math has been implemented).
 *
 *****************************************************************************/

class Vector4d
{
public:
	double x,y,z,w;	
DPVS_FORCE_INLINE					Vector4d	(void) : x(0.0),y(0.0),z(0.0),w(0.0)		{}
DPVS_FORCE_INLINE					Vector4d	(double cx, double cy, double cz, double cw) : x(cx),y(cy),z(cz),w(cw) {}
DPVS_FORCE_INLINE const double&		operator[]	(int i) const					{ DPVS_ASSERT (i>=0 && i < 4); return ((const double*)this)[i]; }
DPVS_FORCE_INLINE double&			operator[]	(int i)							{ DPVS_ASSERT (i>=0 && i < 4); return ((double*)this)[i]; }
};


} // namespace DPVS

//------------------------------------------------------------------------
#endif
