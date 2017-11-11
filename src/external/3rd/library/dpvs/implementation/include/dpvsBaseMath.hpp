#ifndef __DPVSBASEMATH_HPP
#define __DPVSBASEMATH_HPP
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
 * Description: 	Very basic math routines & inclusion of <cmath>.
 *					If non-inline, the routines are provided in dpvsMath.cpp
 *
 * $Archive: /dpvs/implementation/include/dpvsBaseMath.hpp $
 * $Author: wili $
 * $Revision: #6 $
 * $Modtime: 13.02.02 12:56 $
 * $Date: 2003/11/18 $
 *
 ******************************************************************************/

#if !defined (__DPVSPRIVATEDEFS_HPP)
#	include "dpvsPrivateDefs.hpp"
#endif

#if !defined (__FAST_MATH__)
#	define __FAST_MATH__
#endif

#include <cmath>										// C library math header

//------------------------------------------------------------------------
// Maximum single-precision floating point value (defined here so that
// we don't need to include <cfloat>).
//------------------------------------------------------------------------

#ifndef FLT_MAX
#	define FLT_MAX         3.402823466e+38F				/* max value */
#endif

#ifndef FLT_MIN
#	define FLT_MIN         1.175494351e-38F
#endif

#undef log2				// some GCC headers may define such a macro
#undef log10			// some GCC headers may define such a macro

namespace DPVS
{
namespace Math
{

//------------------------------------------------------------------------
// Global constants
//------------------------------------------------------------------------

extern const float PI;	// the value 'pi'

#if defined (DPVS_BUILD_CW) && defined (DPVS_CPU_PS2)                    // need to turn profiling off here..
#   pragma cats off
#endif

#if defined (DPVS_PS2_ASSEMBLY)	// PS2 "intrinsic" functions
//PS2-BEGIN

//------------------------------------------------------------------------
// Some single-precision float functions + integer functions for PS2
//------------------------------------------------------------------------

#if !defined (DPVS_BUILD_CW) // GCC version..
	DPVS_FORCE_INLINE  float   fabs            (float a)								{ float ret; asm ("abs.s %0, %1" : "=f"(ret) : "f"(a) ); return ret; }
#else // CodeWarrior stuff
	DPVS_FORCE_INLINE  float   fabs            (float f)								{ return ::fabsf(f);    }
#endif //

DPVS_FORCE_INLINE	int			intChop         (float a)								{ int ret; asm ("cvt.w.s %1, %1; mfc1 %0,%1" : "=r"(ret) : "f"(a) ); return ret;}
DPVS_FORCE_INLINE	float		max             (float a, float b)                      { float ret; asm ("max.s %0, %1, %2" : "=f"(ret) : "f"(a), "f"(b) ); return ret; }
DPVS_FORCE_INLINE	float		min             (float a, float b)                      { float ret; asm ("min.s %0, %1, %2" : "=f"(ret) : "f"(a), "f"(b) ); return ret; }
DPVS_FORCE_INLINE	int			max				(int a, int b)							{ int ret; asm ("add %0, %1, $0; slt $8, %1, %2;	movn %0, %2, $8" : "=r"(ret) : "r"(a), "r"(b) : "$8" );	return ret;}
DPVS_FORCE_INLINE	int			min				(int a, int b)							{ int ret; asm ("add %0, %1, $0; slt $8, %2, %1; movn %0, %2, $8" : "=r"(ret) : "r"(a), "r"(b) : "$8" ); return ret;}
DPVS_FORCE_INLINE	float		sqrt            (float a)                               { float ret; asm ("sqrt.s %0, %1" : "=f"(ret) : "f"(a) ); return ret; }
DPVS_FORCE_INLINE	float		reciprocalSqrt  (float a)                               { float ret; const float one = 1.0f; asm ("rsqrt.s %0, %1, %2" : "=f"(ret) : "f"(one) , "f"(a) ); return ret; }
DPVS_FORCE_INLINE	int			intFloor		(const float& f )						{const INT32 almostOneInt32 = 0x3f800000-1; const float almostOne = *reinterpret_cast<const float*>(&almostOneInt32); int ret; asm ("cvt.w.s $f1, %1; mfc1 %0, $f1; cvt.s.w $f2, $f1; sub.s $f2, $f2, %1; add.s $f2, $f2, %2; cvt.w.s $f1, $f2; mfc1 $8, $f1; sub %0, $8" : "=r"(ret) : "f"(f), "f"(almostOne) : "$8", "$f1", "$f2" ); return ret;}
DPVS_FORCE_INLINE	int			intCeil			(const float& f)						{const INT32	almostOneInt32 = 0x3f800000-1; const float almostOne = *reinterpret_cast<const float*>(&almostOneInt32); int ret; asm ("neg.s $f2, %1; cvt.w.s $f1, $f2; mfc1 $8, $f1; cvt.s.w $f2, $f1; add.s $f2, $f2, %1; add.s $f2, $f2, %2; cvt.w.s $f1, $f2; mfc1 %0, $f1; sub %0, %0, $8" : "=r"(ret) : "f"(f), "f"(almostOne) : "$8", "$f1", "$f2" ); return ret;}

//------------------------------------------------------------------------
// Nintendo GameCube versions of some basemath routines
//------------------------------------------------------------------------
//PS2-END
#elif defined (DPVS_GAMECUBE_ASSEMBLY)
//NGC-BEGIN
DPVS_FORCE_INLINE	float		fabs            (float a)								{ return ::fabsf(a); }
DPVS_FORCE_INLINE	int			intChop         (float a)								{ return (int)(a);	}
DPVS_FORCE_INLINE	float		max				(register float a, register float b)
{
	register float retVal = a - b;
	asm
	{
//		fsubs	fp3,a,b					// fp3 = a - b  (>= 0 if a > b)
		fsel	retVal,retVal,a,b		// ret = (fp3 >= 0) ? a : b
	}

	DPVS_ASSERT(retVal == ((a>b) ? a : b));
	return retVal;
}
DPVS_FORCE_INLINE	float		min				(register float a, register float b)
{
	register float retVal = a - b;
	asm
	{
//		fsubs	fp3,a,b					// fp3 = a - b  (>= 0 if a > b)
		fsel	retVal,retVal,b,a		// ret = (fp3 >= 0) ? b : a
	}
	DPVS_ASSERT(retVal == ((a<b) ? a : b));
	return retVal;
}
DPVS_FORCE_INLINE	int			max				(int a, int b)							{ return a > b ? a : b; }
DPVS_FORCE_INLINE	int			min				(int a, int b)							{ return a < b ? a : b; }
DPVS_FORCE_INLINE	int			intFloor		(const float& f )
{

	const INT32	almostOneInt32 = 0x3f800000-1;
	const float	almostOne = *reinterpret_cast<const float*>(&almostOneInt32);
	int	c = intChop(f);
	float cf = c;
	float xf = cf-f+almostOne;
	int x = intChop(xf);
	return	c - x;
}

DPVS_FORCE_INLINE	int			intCeil			(const float& f)
{
	const INT32	almostOneInt32 = 0x3f800000-1;
	const float	almostOne = *reinterpret_cast<const float*>(&almostOneInt32);
	int	c = intChop(-f);
	float cf = c;
	float xf = cf+f+almostOne;
	int x = intChop(xf);
	return	x - c;
}

DPVS_FORCE_INLINE  float	sqrt			(float f)								{ return ::sqrtf(f);	}
DPVS_FORCE_INLINE  float	reciprocalSqrt	(float f)								{ DPVS_ASSERT(f>0.0f);  return 1.0f / (::sqrtf(f)); }

//------------------------------------------------------------------------
// Some single-precision float functions + integer functions for non-PS2
// platforms
//------------------------------------------------------------------------

//NGC-END
#else // non-PS2/NGC routines

DPVS_FORCE_INLINE  float	max				(float a, float b)						{ return a > b ? a : b; }
DPVS_FORCE_INLINE  float	min				(float a, float b)						{ return a < b ? a : b; }
DPVS_FORCE_INLINE  int		max				(int a, int b)							{ return a > b ? a : b; }
DPVS_FORCE_INLINE  int		min				(int a, int b)							{ return a < b ? a : b; }

DPVS_FORCE_INLINE int intChop (const float& f)						// generic non-FPU implementation of intChop()
{
	INT32 a			= *reinterpret_cast<const INT32*>(&f);			// take bit pattern of float into a register
	INT32 sign		= (a>>31);										// sign = 0xFFFFFFFF if original value is negative, 0 if positive
	INT32 mantissa	= (a&((1<<23)-1))|(1<<23);						// extract mantissa and add the hidden bit
	INT32 exponent	= ((a&0x7fffffff)>>23)-127;						// extract the exponent
	INT32 r			= ((UINT32)(mantissa)<<8)>>(31-exponent);		// ((1<<exponent)*mantissa)>>24 -- (we know that mantissa > (1<<24))
	return ((r ^ (sign)) - sign ) &~ (exponent>>31);				// add original sign. If exponent was negative, make return value 0.
}


#endif // DPVS_PS2_ASSEMBLY

//------------------------------------------------------------------------
// double <-> float conversion code (may be fully emulated on some
// platforms)
//------------------------------------------------------------------------

#if defined (DPVS_EMULATED_DOUBLE)
	void		doubleToFloat	(float& f, const double& d);
	void		floatToDouble	(double& d, const float& f);
#else
	DPVS_FORCE_INLINE  void	doubleToFloat	(float& f, const double& d )			{ f = (float)d; }
	DPVS_FORCE_INLINE  void	floatToDouble	(double& d, const float& f )			{ d = (double)f; }
#endif

//------------------------------------------------------------------------
// Some other useful single-precision floating point / integer routines
//------------------------------------------------------------------------

DPVS_FORCE_INLINE int abs (int a)									
{ 
#if defined (DPVS_BUILD_CW) && defined (DPVS_OS_MAC)
	return __abs(a); 
#else
	return (a<0) ? -a : a;
#endif
}

DPVS_FORCE_INLINE	UINT32	bitPattern		(const float& f)						{ return *((const UINT32*)&f);	}
DPVS_FORCE_INLINE	void    clamp			(float& a, float mn, float mx)			{ DPVS_ASSERT(mn <= mx); a = max(a,mn); a = min(a,mx); }
DPVS_FORCE_INLINE	void    clamp			(int& a, int mn, int mx)				{ DPVS_ASSERT(mn <= mx); a = max(a,mn); a = min(a,mx); }
DPVS_FORCE_INLINE	int		clampMinZero	(INT32 i)								{  /* clamps value to zero, i.e. x = (x<0) ? 0 : x; */ return i & ~(i>>31);  }
DPVS_FORCE_INLINE	void	flushToZero		(float& f)								{ /* If fabs(f) < 1e-15 f = 0.0f; */ if ((bitPattern(f) & 0x7fffffff) <= 0x26901d7d) f = 0.0f; }
					int		intFloor		(const float&);
					int		intCeil 		(const float&);
DPVS_FORCE_INLINE	bool	isFinite		(float f)								{ /* return f>=-FLT_MAX && f<=FLT_MAX */ UINT32 pattern = (*(UINT32*)(&f)) & 0x7FFFFFFF; return (pattern <= 0x7F7FFFFF); }
DPVS_FORCE_INLINE	bool	isValidFloat	(float f)								{ /* return f>=-FLT_MAX && f<=FLT_MAX */ UINT32 pattern = (*(UINT32*)(&f)) & 0x7FFFFFFF; return (pattern <= 0x7F800000); }


DPVS_FORCE_INLINE	bool	isZero			(const float& f)						{ return (((*((const UINT32*)(&f)))&0x7FFFFFFF)==0); }
DPVS_FORCE_INLINE	float	reciprocal		(float f)								{ DPVS_ASSERT(f!=0.0f); return 1.0f / f; }
DPVS_FORCE_INLINE	void	sort3			(float& a, float& b, float& c)			{ /* sorts values a,b,c so that a<=b<=c */ if (a > b) swap(a,b); if (b > c) swap(b,c); if (a > b) swap(a,b);			 }
DPVS_FORCE_INLINE	float	sqr				(float x)								{ return x*x; }

//------------------------------------------------------------------------
// If <cmath> routines only have double-precision variants, we need to
// have separate code here.. otherwise use the single-precision variants
//------------------------------------------------------------------------

#if defined (DPVS_DOUBLE_MATH_H)
DPVS_FORCE_INLINE	float   ceil            (float f)								{ return (float)::ceil((float)f);			}
DPVS_FORCE_INLINE	float   cos				(float f)								{ return (float)::cos((float)f);			}
#	if defined (DPVS_BUILD_CW) && defined (DPVS_OS_MAC)
		DPVS_FORCE_INLINE	float	fabs			(float f)						{ return __fabsf(f);	}
#	else
		DPVS_FORCE_INLINE	float	fabs			(float f)						{ return (float)::fabs((float)f);	}
#endif

DPVS_FORCE_INLINE	float   floor           (float f)								{ return (float)::floor((float)f);			}
DPVS_FORCE_INLINE	float	reciprocalSqrt	(float f)								{ DPVS_ASSERT(f>0.0f);  return 1.0f / ((float)::sqrt((float)f)); }
DPVS_FORCE_INLINE	float   sin				(float f)								{ return (float)::sin((float)f);			}
DPVS_FORCE_INLINE	float	sqrt			(float f)								{ return (float)::sqrt((float)f);	}
DPVS_FORCE_INLINE	float   pow				(float x, float y)						{ return (float)::pow((float)x,(float)y);	}
#else
DPVS_FORCE_INLINE	float   ceil            (float f)								{ return ceilf(f);	}
DPVS_FORCE_INLINE	float   cos				(float f)								{ return cosf(f);		}
DPVS_FORCE_INLINE	float   floor           (float f)								{ return floorf(f);   }
DPVS_FORCE_INLINE	float   sin				(float f)								{ return sinf(f);		}
DPVS_FORCE_INLINE	float   pow				(float x, float y)						{ return powf(x,y);	}
#	if !defined (DPVS_PS2_ASSEMBLY) && !defined(DPVS_GAMECUBE_ASSEMBLY)
DPVS_FORCE_INLINE  float	fabs			(float f)								{ return fabsf(f);	}
DPVS_FORCE_INLINE  float	sqrt			(float f)								{ return sqrtf(f);	}
DPVS_FORCE_INLINE  float	reciprocalSqrt	(float f)								{ DPVS_ASSERT(f>0.0f);  return 1.0f / (sqrtf(f)); }
#	endif
#endif // DPVS_DOUBLE_MATH_H

#if defined (DPVS_BUILD_CW) && defined (DPVS_CPU_PS2)                   // turn profiling back on
#   pragma cats on
#endif

} // Math
} // DPVS

//------------------------------------------------------------------------
#endif // __DPVSBASEMATH_HPP

