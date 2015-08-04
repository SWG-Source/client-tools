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
 * Description:		PowerPC test code
 *
 * $Archive: /dpvs/implementation/sources/dpvsPPCTest.cpp $
 * $Author: wili $ 
 * $Revision: #2 $
 * $Modtime: 6/28/01 5:38p $
 * $Date: 2003/02/20 $
 * 
 ******************************************************************************/

#if 0
//#define DPVS_PPC_TEST

#if defined (DPVS_PPC_TEST)
#	include "dpvsMatrixConverter.hpp"
#	include "dpvsVector.hpp"
#	include "dpvsMath.hpp"
#	include "dpvsBitMath.hpp"
#	include "dpvsRectangle.hpp"
#	include "dpvsRandom.hpp"

#	include <cstdlib>
#	include <cstdio>
#	include <cmath>

#	if defined (DPVS_OS_MAC) && !defined (DPVS_PPC_ASSEMBLY)
#		error PowerPC assembly not supported!
#	endif
#endif

namespace DPVS
{
namespace Math
{

#if defined (DPVS_PPC_TEST)

class PPCTest
{
public:
	static void			test				(void);

private:
	//--------------------------------------------------------------------
	// ppc implementations
	//--------------------------------------------------------------------

	static int			getHighestSetBitPPC	(unsigned int value);
	static unsigned int	getNextPowerOfTwoPPC(unsigned int value);

	static int			intFloorPPC			(float f);
	static int			intCeilPPC			(const float& f);

	static float		dotPPC				(const Vector3& a, const Vector3& b);
	static float		dotPPC				(const Vector4& a, const Vector4& b);
	static void			dotArrayPPC			(float* dst, const Vector4* src, const Vector4& p, int N);

	//--------------------------------------------------------------------
	// test functions
	//--------------------------------------------------------------------

	static void			bitMathTests		(void);

	static void			dotTestArray		(void);
	static void			dotTestSingle		(void);

	static void			intFloorTest1		(void);
	static void			intFloorTest2		(void);
	static void			intFloorTest3		(void);
	static void			intCeilTest			(void);

	static inline float	frand				(void)	{ return float( rand() - RAND_MAX/2 ) / float( (RAND_MAX+1)/2 ); }

private:
	Random				m_random;
};

//--------------------------------------------------------------------
// ppc implementations
//--------------------------------------------------------------------

int PPCTest::getHighestSetBitPPC(unsigned int value)
{
#if defined (DPVS_PPC_ASSEMBLY)
	register int retVal;

	asm
	(
		"li r11,31\n"
		"cntlzw r12,%1\n"
		"sub %0,r11,r12\n"
		: "=r"(retVal) : "r"(value) : "r12","r11"
	);

	return retVal;
#else
	return getHighestSetBit(value);
#endif
}

unsigned int PPCTest::getNextPowerOfTwoPPC(unsigned int value)
{
#if defined (DPVS_PPC_ASSEMBLY)
	register int retVal;

	if(value == 0) return 1;	// can we somehow include this into the computations??

	asm
	(
	   "li		r10,32\n"
	"	subi	r11,%1,1\n"
	"	cntlzw	r11,r11\n"
	"	sub		r10,r10,r11\n"
	"	li		r12,1\n"
	"	slw		%0,r12,r10\n"
		: "=r"(retVal) : "r"(value) : "r12","r11"
	);

	return retVal;
#else
	return getNextPowerOfTwo(value);
#endif
}

float PPCTest::dotPPC(const Vector3& v1, const Vector3& v2)
{
#if defined (DPVS_PPC_ASSEMBLY)

//	asm
//	(
//	   "lfs f1,0(%1)\n"
//	"	lfs f2,4(%1)\n"
//	"	lfs f3,8(%1)\n"
//	"	lfs f4,0(%2)\n"
//	"	lfs f5,4(%2)\n"
//	"	lfs f6,8(%2)\n"
//	"	fmul f7,f1,f4\n"
//	"	fmadd f7,f2,f5,f7\n"
//	"	fmadd f7,f3,f6,f7\n"
//	"	stfs f7,0(%0)"
//	: : "b"(&ret), "b"(&v1), "b"(&v2) : "f1", "f2", "f3", "f4", "f5", "f6", "f7"
//	);

	float retVal;

	asm
	(
	"	lfs f1,0(%1)\n"
	"	lfs f2,4(%1)\n"
	"	lfs f3,8(%1)\n"
	"	lfs f4,0(%2)\n"
	"	lfs f5,4(%2)\n"
	"	lfs f6,8(%2)\n"
	"	fmuls %0,f1,f4\n"
	"	fmadds %0,f2,f5,%0\n"
	"	fmadds %0,f3,f6,%0\n"
	: "=f"(retVal) : "b"(&v1), "b"(&v2) : "f1", "f2","f3", "f4", "f5", "f6"
	);

	return retVal;
#else
	return DPVS::dot(v1, v2);
#endif
}

float PPCTest::dotPPC(const Vector4& a, const Vector4& b)
{
#if defined (DPVS_PPC_ASSEMBLY)
	float ret;

	asm
	(
	"	lfs f1, 0(%1)\n"
	"	lfs f2, 4(%1)\n"
	"	lfs f3, 8(%1)\n"
	"	lfs f4,12(%1)\n"
	"	lfs f5, 0(%2)\n"
	"	lfs f6, 4(%2)\n"
	"	lfs f7, 8(%2)\n"
	"	lfs f8,12(%2)\n"
	"	fmuls %0,f1,f5\n"
	"	fmadds %0,f2,f6,%0\n"
	"	fmadds %0,f3,f7,%0\n"
	"	fmadds %0,f4,f8,%0\n"
	: "=f"(ret) : "b"(&a), "b"(&b) : "f1", "f2","f3", "f4", "f5", "f6", "f7", "f8"
	);

	return ret;
#else
	return DPVS::dot(a, b);
#endif
}

void PPCTest::dotArrayPPC(float* dst, const Vector4* src, const Vector4& p, int N)
{
#if defined (DPVS_PPC_ASSEMBLY) && defined (not_defined)
	// %0 = dst
	// %1 = src
	// %2 = &p
	// %3 = N
	// f4-f7 = p
	asm
	(
	"	lfs 	f6, 0(%2)		\n"
	"	lfs 	f7, 4(%2)		\n"
	"	lfs 	f8, 8(%2)		\n"
	"	lfs 	f9,12(%2)		\n"
	"	mtctr	%3				\n"

	"	addi	%1,%1,-4		\n"
	"	addi	%0,%0,-4		\n"

"1:								\n"
	"	lfsu	f10,4(%1)		\n"
	"	lfsu	f11,4(%1)		\n"
	"	lfsu	f12,4(%1)		\n"
	"	lfsu	f13,4(%1)		\n"

	"	fmul	f5,f6,f10		\n"
	"	fmadd	f5,f7,f11,f5	\n"
	"	fmadd	f5,f8,f12,f5	\n"
	"	fmadd	f5,f9,f13,f5	\n"

	"	stfsu	f5,4(%0)		\n"

	"	bdnz+	1b				\n"

	: /* no output */
	: "b"(dst), "b"(src), "b"(&p), "r"(N)
	: "f5", "f6", "f7", "f8", "f9", "f10", "f11", "f12", "f13"
	);
#else
	register float px = p.x, py = p.y, pz = p.z, pw = p.w;

	for (int i = 0; i < N; i++)
		dst[i] = src[i].x * px + src[i].y * py + src[i].z * pz + src[i].w * pw;
#endif
}

int PPCTest::intFloorPPC(float f)
{
#if defined (DPVS_PPC_ASSEMBLY)
	const INT32	almostOneInt32 = 0x3f800000-1;
	const float	almostOne = *reinterpret_cast<const float*>(&almostOneInt32);
	//int		c  = intChop(f);
	//float		cf = c;
	//float		xf = cf-f+almostOne;
	//int		x  = intChop(xf);
	//return	c - x;
//	int ret;

/*
	int tmp[2];
	asm volatile
	(
		"
		fctiwz	f8,%2
		stfd	f8,0(%1)

		lwz		r8,4(%1)		// r1 = intChop(c)
		lwz		%0,4(%1)		// ret = intChop(c)

		lfs		f5,4(%1)		// f8 = intChop(c) = [cf]
		fsub	f6,f5,%2		// f9 = cf-f
		fadd	f6,f6,%2		// f9 = cf-f+almostOne = [xf]

		fctiwz	f6,f6
		stfd	f6,0(%1)
		"
	: "=r"(ret) : "b"(&tmp[0]), "f"(f), "f"(almostOne) : "f8", "f9", "r8", "r9"
	);
*/
/*
		lwz		r8,4(%1)		// r1 = intChop(c)
		lfs		f8,4(%1)		// f8 = intChop(c) = [cf]
		fsub	f9,f8,%2		// f9 = cf-f
		fadd	f9,f9,%2		// f9 = cf-f+almostOne = [xf]
		fctiwz	f9,f9
		stfd	f9,0(%1)
		lwz		r9,4(%1)
		subf	%0,r8,r9		// ret = c - x
*/

//	int		c	= ret;
//	int		x	= tmp[1];

//	float q=1.0f,w=1.0f;
//	asm volatile ( "fctiwz %0,%1 // FLOOR!!!" : "=f"(q) : "f"(w) );

	int		c	= (int)f;
	float	cf	= (float)c;
	float	xf	= cf - f + almostOne;
	int		x	= (int)xf;
	int		rv	= c - x;

//	asm volatile ( "fctiwz %0,%1 // END FLOOR!!!" : "=f"(q) : "f"(w) );

	return rv;

//	return ret;
#else
	return Math::intFloor(f);
#endif
}

int PPCTest::intCeilPPC(const float& f)
{
//#if defined (DPVS_PPC_ASSEMBLY)
//	const INT32	almostOneInt32 = 0x3f800000-1;
//	const float	almostOne = *reinterpret_cast<const float*>(&almostOneInt32);
//	//int	c = intChop(-f); 
//	//float cf = c; 
//	//float xf = cf+f+almostOne; 
//	//int x = intChop(xf); 
//	//return	x - c;
//	int ret;

//	return ret;
//#else
	return Math::intCeil(f);
//#endif
}

//--------------------------------------------------------------------
// test functions
//--------------------------------------------------------------------

void PPCTest::bitMathTests(void)
{
	printf("ppctest: testing getHighestSetBit()\n");

	Random random;

	for(int i = 0; i < 1000000; i++)
	{
		int value		= random.getI() & random.getI();
		int result		= getHighestSetBitPPC(value);
		int reference	= getHighestSetBit(value);

		if(result != reference)
		{
			printf("ppctest: #%d: getHighestSetBitPPC(0x%x) == 0x%x, should be 0x%x\n", i, value, result, reference);
			break;
		}
	}

	for(int i = 0; i < 1000000; i++)
	{
		int value		= random.getI() & random.getI();
		int result		= getNextPowerOfTwoPPC(value);
		int reference	= getNextPowerOfTwo(value);

		if(result != reference)
		{
			printf("ppctest: #%d: getNextPowerOfTwoPPC(0x%x) == 0x%x, should be 0x%x\n", i, value, result, reference);
			break;
		}
	}

}

void PPCTest::dotTestArray(void)
{
	printf("ppctest: dotTestArray\n");

	const int		NUM_VECTORS = 119;
	Vector4			vectors[NUM_VECTORS];
	float			results[NUM_VECTORS];
	float			references[NUM_VECTORS];
	const Vector4	dotWith(1.3f, 0.3f, -1.1f, 0.5f);

	for(int i = 0; i < NUM_VECTORS; i++)
	{
//		vectors[i] = Vector4(frand(), frand(), frand(), frand());
		vectors[i] = Vector4(1.0f, 0.0f, 0.0f, 0.0f);
	}

	dotArrayPPC(results, vectors, dotWith, NUM_VECTORS);
	Math::dot(references, vectors, dotWith, NUM_VECTORS);

	for(int i = 0; i < NUM_VECTORS; i++)
	{
		if(fabs(results[i] - references[i]) > 0.00001)
		{
			printf("ppctest: array dot failed at %d of %d, %f instead of %f\n", i, NUM_VECTORS, results[i], references[i]);
			printf("ppctest: (%f, %f, %f, %f) . ", vectors[i].x, vectors[i].y, vectors[i].z, vectors[i].w);
			printf("(%f, %f, %f, %f)\n", dotWith.x, dotWith.y, dotWith.z, dotWith.w);
			printf("ppctest: huoh: %f\n", DPVS::dot(vectors[i], dotWith));
			break;
		}
	}

	printf("ppctest: result[0]: %f\n", references[0]);
}

void PPCTest::dotTestSingle(void)
{
//	printf("ppctest: dotTestSingle\n");

//	const Vector3	a(1.3f, 0.3f, -1.1f);
//	const Vector3	b(-0.5f, 1.3f, 0.1f);

	const Vector3	a3(1.0f, 0.0f, 0.0f);
	const Vector3	b3(0.5f, 0.0f, 0.0f);

	float result	= dotPPC(a3, b3);
	float reference	= DPVS::dot(a3, b3);

	if(fabs(result - reference) > 0.000001)
	{
		printf("ppctest: PPC asm dot3(..)==%f, should be %f\n", result, reference);
	}

	const Vector4	a4(1.3f, 0.3f, -1.1f, 0.5f);
	const Vector4	b4(-0.5f, 1.3f, 0.1f, 1.0f);

	result		= dotPPC(a4, b4);
	reference	= DPVS::dot(a4, b4);

	if(fabs(result - reference) > 0.000001)
	{
		printf("ppctest: PPC asm dot4(..)==%f, should be %f\n", result, reference);
	}
}

void PPCTest::intFloorTest1()
{
	printf( "ppctest: intFloor (test 1)\n" );
	float DPVS_VECTOR_ALIGN(floats[])  = { 1.2f, -1.4f, 2.7f, 0.1f, -0.7f, 0.f, 1.f, -0.99f, 3.2f, 100.1f } ;
	INT32 DPVS_VECTOR_ALIGN(ints[ sizeof(floats)/sizeof(floats[0]) ]) ;
	const int n = sizeof(floats)/sizeof(floats[0]);
	Math::intFloor( ints, floats, n );
	for ( int i = 0 ; i < n ; ++i )
	{
		float f = floats[i];
//		int asmint = ints[i];
		int refint = (int)floor(f);

		int ppcFloored = intFloorPPC(floats[i]);
		printf("floor(%f) == %i (0x%x)\n", floats[i], ppcFloored, ppcFloored);
		if ( ppcFloored != refint )
			printf( "ppctest: PPC asm intFloor(%f)==%i, should be %i\n", f, ppcFloored, refint );
	}
}

void PPCTest::intFloorTest2()
{
	printf( "ppctest: intFloor (test 2)\n" );
	float DPVS_VECTOR_ALIGN(floats[103]);
	INT32 DPVS_VECTOR_ALIGN(ints[ sizeof(floats)/sizeof(floats[0]) ]);
	const int n = sizeof(floats)/sizeof(floats[0]);
	for ( int j = 0 ; j < n ; ++j )
		floats[j] = frand() * 1e8f - 10.f * frand();
	Math::intFloor( ints, floats, n );
	for ( int i = 0 ; i < n ; ++i )
	{
		float f = floats[i];
		int asmint = ints[i];
		int refint = (int)floor(f);
		if ( asmint != refint )
			printf( "ppctest: PPC asm intFloor(%f)==%i, should be %i\n", f, asmint, refint );
	}
}

void PPCTest::intFloorTest3() 
{
	printf( "ppctest: intFloor (test 3)\n" );
	float floats[103];
	const int n = sizeof(floats)/sizeof(floats[0]);
	for ( int j = 0 ; j < n ; ++j )
		floats[j] = frand() * 1e8f - 10.f * frand();
	for ( int i = 0 ; i < n ; ++i )
	{
		float f = floats[i];
		int refint = (int)floor(f);
		int asmint = intFloorPPC(f);
		if ( asmint != refint )
			printf( "ppctest: intFloorPPC(%f)==%i, should be %i\n", f, asmint, refint );
	}
}

void PPCTest::intCeilTest()
{
	printf( "ppctest: intCeil\n" );
	float floats[103];
	const int n = sizeof(floats)/sizeof(floats[0]);
	for ( int j = 0 ; j < n ; ++j )
		floats[j] = frand() * 1e8f - 10.f * frand();
	for ( int i = 0 ; i < n ; ++i )
	{
		float f = floats[i];
		int refint = (int)ceil(f);
		int asmint = intCeilPPC(f);
		if ( asmint != refint )
			printf( "ppctest: intCeilPPC(%f)==%i, should be %i\n", f, asmint, refint );
	}
}

void PPCTest::test(void)
{
	bitMathTests();

	dotTestSingle();
	dotTestArray();

	intFloorTest1();
//	intFloorTest2();
//	intFloorTest3();
//	intCeilTest();
}

#endif // DPVS_PPC_TEST

void ppcTest(void)
{
#if defined (DPVS_PPC_TEST)
	PPCTest::test();
#endif
}

} // Math
} // DPVS

#endif
//--------------------------------------------------------------------
