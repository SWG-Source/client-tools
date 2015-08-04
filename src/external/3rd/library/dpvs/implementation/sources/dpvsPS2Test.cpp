//PS2-BEGIN
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
 * Description:		PS2 test code
 *
 * $Archive: /dpvs/implementation/sources/dpvsPS2Test.cpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 27.08.02 14:48 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#include "dpvsMath.hpp"
#include "dpvsRectangle.hpp"

//#define JANITEST

#if !defined (DPVS_PS2_ASSEMBLY)
#	undef JANITEST
#endif

#if defined (JANITEST)
#	include "dpvsMatrixConverter.hpp"
#	include <stdio.h>
#	include <math.h>
#	include <string.h>
#	include <stdlib.h>
#	include <float.h>
#endif


namespace DPVS
{
namespace Math
{


#ifdef JANITEST
#define JANI_ASSERT( A ) if (A) {} else {printf( "janitest: Assertion %s failed, line %i\n", #A, __LINE__ );}

static const char* toString( const Matrix4x3& m )
{
	static char buff[1024];
	buff[0] = 0;
	for ( int i = 0 ; i < 3 ; ++i )
		sprintf( buff+strlen(buff), "%f %f %f %f\n", m[i][0], m[i][1], m[i][2], m[i][3] );
	return buff;
}

static Vector4 referenceTransformPlaneUniform( const Vector4& plEq, const Matrix4x3& invMtx )
{
	float  A	= (invMtx[0][0] * plEq.x + invMtx[0][1] * plEq.y + invMtx[0][2] * plEq.z);
	float  B	= (invMtx[1][0] * plEq.x + invMtx[1][1] * plEq.y + invMtx[1][2] * plEq.z);
	float  C	= (invMtx[2][0] * plEq.x + invMtx[2][1] * plEq.y + invMtx[2][2] * plEq.z);
	float  L2	= (A*A + B*B + C*C);
	float  L	= Math::reciprocalSqrt(L2);
	float  D	= plEq.w * L2 - (A*invMtx[0][3] + B*invMtx[1][3] + C*invMtx[2][3]);
	return Vector4(A*L,B*L,C*L,D*L);
}

static float frand()
{
	return float( rand() - RAND_MAX/2 ) / float( (RAND_MAX+1)/2 );
}

static void referenceTransform (Vector4* dest, const Vector3* src, int N, const Matrix4x4& mtx)
{
	DPVS_ASSERT (dest && src && N > 0);

	for (int i = 0; i < N; i++, dest++, src++)
	{
		float sx = src->x;
		float sy = src->y;
		float sz = src->z;
		 
		dest->x = mtx[0].x * sx + mtx[0].y * sy + mtx[0].z * sz + mtx[0].w;
		dest->y = mtx[1].x * sx + mtx[1].y * sy + mtx[1].z * sz + mtx[1].w;
		dest->z = mtx[2].x * sx + mtx[2].y * sy + mtx[2].z * sz + mtx[2].w;
		dest->w = mtx[3].x * sx + mtx[3].y * sy + mtx[3].z * sz + mtx[3].w;
	}
}

static void referenceTransform (Vector4* dest, const Vector4* src, int N, const Matrix4x4& mtx)
{
	DPVS_ASSERT (dest && src && N > 0);

	for (int i = 0; i < N; i++, dest++, src++)
	{
		register float sx = src->x;
		register float sy = src->y;
		register float sz = src->z;
		register float sw = src->w;
		 
		dest->x = mtx[0].x * sx + mtx[0].y * sy + mtx[0].z * sz + mtx[0].w * sw;
		dest->y = mtx[1].x * sx + mtx[1].y * sy + mtx[1].z * sz + mtx[1].w * sw;
		dest->z = mtx[2].x * sx + mtx[2].y * sy + mtx[2].z * sz + mtx[2].w * sw;
		dest->w = mtx[3].x * sx + mtx[3].y * sy + mtx[3].z * sz + mtx[3].w * sw;
	}
}

static void objectToRasterReference (Vector2* dest, const Vector3* src, int N, const Matrix4x4& mtx, const FloatRectangle& viewPort)
{
	DPVS_ASSERT (dest && src);

	float	xscale	=  0.5f * viewPort.width();
	float	xofs	=  0.5f * viewPort.width()  + viewPort.x0;
	float	yscale	= -0.5f * viewPort.height();
	float	yofs	=  0.5f * viewPort.height() + viewPort.y0;

	for (int i = 0; i < N; i++, dest++, src++)
	{
		float sx	= src->x;
		float sy	= src->y;
		float sz	= src->z;
		float oow   = Math::reciprocal(mtx[3].x * sx + mtx[3].y * sy + mtx[3].z * sz + mtx[3].w);

		dest->x = (mtx[0].x * sx + mtx[0].y * sy + mtx[0].z * sz + mtx[0].w) * oow * xscale + xofs;
		dest->y = (mtx[1].x * sx + mtx[1].y * sy + mtx[1].z * sz + mtx[1].w) * oow * yscale + yofs;
	}
}

void objectToRasterReference (Vector3* dest, const Vector3* src, int N, const Matrix4x4& mtx, const FloatRectangle& viewPort)
{
	DPVS_ASSERT (dest && src);

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

		dest->x = (mtx[0].x * sx + mtx[0].y * sy + mtx[0].z * sz + mtx[0].w) * oow * xscale + xofs;
		dest->y = (mtx[1].x * sx + mtx[1].y * sy + mtx[1].z * sz + mtx[1].w) * oow * yscale + yofs;
		dest->z = (mtx[2].x * sx + mtx[2].y * sy + mtx[2].z * sz + mtx[2].w) * oow * 0.5f + 0.5f;
	}
}

static int intFloorPS2( const float& f )
{
	return Math::intFloor(f);

	const INT32	almostOneInt32 = 0x3f800000-1;
	const float	almostOne = *reinterpret_cast<const float*>(&almostOneInt32);
	//int	c = intChop(f); 
	//float cf = c; 
	//float xf = cf-f+almostOne; 
	//int x = intChop(xf); 
	//return	c - x;
	int ret;
	asm (
	"	cvt.w.s	$f1, %1"
	"\n	mfc1		%0, $f1"
	"\n	cvt.s.w	$f2, $f1"
	"\n	sub.s		$f2, $f2, %1"
	"\n	add.s		$f2, $f2, %2"
	"\n	cvt.w.s	$f1, $f2"
	"\n	mfc1		$8, $f1"
	"\n	sub		%0, $8;"
		  : "=r"(ret) : "f"(f), "f"(almostOne) : "$8", "$f1", "$f2" );
	return ret;
}

static int intCeilPS2( const float& f )
{
	return Math::intCeil(f);

	const INT32	almostOneInt32 = 0x3f800000-1;
	const float	almostOne = *reinterpret_cast<const float*>(&almostOneInt32);
	//int	c = intChop(-f); 
	//float cf = c; 
	//float xf = cf+f+almostOne; 
	//int x = intChop(xf); 
	//return	x - c;
	int ret;
	asm (
		 "	 neg.s		$f2, %1" 
		 "\n cvt.w.s	$f1, $f2" 
		 "\n mfc1		$8, $f1" 
		 "\n cvt.s.w	$f2, $f1" 
		 "\n add.s		$f2, $f2, %1"
		 "\n add.s		$f2, $f2, %2" 
		 "\n cvt.w.s	$f1, $f2" 
		 "\n mfc1		%0, $f1" 
		 "\n sub		%0, %0, $8;"
		 : "=r"(ret) : "f"(f), "f"(almostOne) : "$8", "$f1", "$f2" );
	return ret;
}

static void minMaxTransformReference( Vector3& mn, Vector3& mx, const Vector3* src, const Matrix4x3& mtx,int N )
{
	mn.make(FLT_MAX,FLT_MAX,FLT_MAX);
	mx.make(-FLT_MAX,-FLT_MAX,-FLT_MAX);

	for(int i = 0; i < N; i++)
	{
		float x	= (mtx[0].x * src[i].x + mtx[0].y * src[i].y + mtx[0].z * src[i].z + mtx[0].w);
		float y	= (mtx[1].x * src[i].x + mtx[1].y * src[i].y + mtx[1].z * src[i].z + mtx[1].w);
		float z = (mtx[2].x * src[i].x + mtx[2].y * src[i].y + mtx[2].z * src[i].z + mtx[2].w);

		mn.x = Math::min(mn.x,x);
		mx.x = Math::max(mx.x,x);
		mn.y = Math::min(mn.y,y);
		mx.y = Math::max(mx.y,y);
		mn.z = Math::min(mn.z,z);
		mx.z = Math::max(mx.z,z);
	}
}

static int maxPS2( int a, int b )
{ 
	int ret;
	asm
	(
		" add %0, %1, $0"
		"\n slt $8, %1, %2"
		"\n movn %0, %2, $8"
		
		: "=r"(ret) : "r"(a), "r"(b) : "$8"
	);

	return ret;
}

static int minPS2( int a, int b )
{ 
	int ret;
	asm (
		" add %0, %1, $0"
		"\n slt $8, %2, %1"
		"\n movn %0, %2, $8"
	 : "=r"(ret) : "r"(a), "r"(b) : "$8" );
	return ret;
}

static float getMaxScaleReference( const Matrix4x4& m )
{
	float xs = (m[0][0]*m[0][0] + m[1][0]*m[1][0] + m[2][0]*m[2][0]); 
	float ys = (m[0][1]*m[0][1] + m[1][1]*m[1][1] + m[2][1]*m[2][1]); 
	float zs = (m[0][2]*m[0][2] + m[1][2]*m[1][2] + m[2][2]*m[2][2]); 

	float maxScale = Math::max(xs,ys);
	return Math::sqrt(Math::max(maxScale,zs)); 
}

static Vector3 getInverseTranslationReference (const Matrix4x3& src)
{
	register float a1 = src[0][0]; 
	register float b1 = src[0][1]; 
	register float c1 = src[0][2];
	register float d1 = src[0][3];
	register float a2 = src[1][0]; 
	register float b2 = src[1][1]; 
	register float c2 = src[1][2]; 
	register float d2 = src[1][3];
	register float a3 = src[2][0]; 
	register float b3 = src[2][1]; 
	register float c3 = src[2][2]; 
	register float d3 = src[2][3];

	register float b2c3_b3c2 = b2 * c3 - b3 * c2;
	register float a3c2_a2c3 = a3 * c2 - a2 * c3;
	register float a2b3_a3b2 = a2 * b3 - a3 * b2;

	register float rDet = -Math::reciprocal(a1 * (b2c3_b3c2) + b1 * (a3c2_a2c3) + c1 * (a2b3_a3b2));

	register float c1b3_b1c3 = c1 * b3 - b1 * c3;
	register float b1c2_c1b2 = b1 * c2 - c1 * b2;
	register float c1a2_a1c2 = c1 * a2 - a1 * c2;
	register float a1c3_c1a3 = a1 * c3 - c1 * a3;
	register float b1a3_a1b3 = b1 * a3 - a1 * b3;
	register float a1b2_b1a2 = a1 * b2 - b1 * a2;

	return Vector3
	(
		(d1 * (b2c3_b3c2) + d2 * (c1b3_b1c3) + d3 * (b1c2_c1b2))*rDet,
		(d1 * (a3c2_a2c3) + d2 * (a1c3_c1a3) + d3 * (c1a2_a1c2))*rDet,
		(d1 * (a2b3_a3b2) + d2 * (b1a3_a1b3) + d3 * (a1b2_b1a2))*rDet
	);
}

static Vector3 getInverseTranslationReference (const Matrix4x3& m, const Matrix4x3& n)
{
	register float a1 = n[0][0]*m[0][0] + n[0][1]*m[1][0] + n[0][2]*m[2][0];
	register float b1 = n[0][0]*m[0][1] + n[0][1]*m[1][1] + n[0][2]*m[2][1];
	register float c1 = n[0][0]*m[0][2] + n[0][1]*m[1][2] + n[0][2]*m[2][2];
	register float a2 = n[1][0]*m[0][0] + n[1][1]*m[1][0] + n[1][2]*m[2][0];
	register float b2 = n[1][0]*m[0][1] + n[1][1]*m[1][1] + n[1][2]*m[2][1];
	register float c2 = n[1][0]*m[0][2] + n[1][1]*m[1][2] + n[1][2]*m[2][2];
	register float a3 = n[2][0]*m[0][0] + n[2][1]*m[1][0] + n[2][2]*m[2][0];
	register float b3 = n[2][0]*m[0][1] + n[2][1]*m[1][1] + n[2][2]*m[2][1];
	register float c3 = n[2][0]*m[0][2] + n[2][1]*m[1][2] + n[2][2]*m[2][2];

	register float b2c3_b3c2 = b2 * c3 - b3 * c2;
	register float a3c2_a2c3 = a3 * c2 - a2 * c3;
	register float a2b3_a3b2 = a2 * b3 - a3 * b2;

	register float rDet = -Math::reciprocal(a1 * (b2c3_b3c2) + b1 * (a3c2_a2c3) + c1 * (a2b3_a3b2));

	register float c1b3_b1c3 = c1 * b3 - b1 * c3;
	register float b1c2_c1b2 = b1 * c2 - c1 * b2;
	register float c1a2_a1c2 = c1 * a2 - a1 * c2;
	register float a1c3_c1a3 = a1 * c3 - c1 * a3;
	register float b1a3_a1b3 = b1 * a3 - a1 * b3;
	register float a1b2_b1a2 = a1 * b2 - b1 * a2;

	register float d1 = n[0][0]*m[0][3] + n[0][1]*m[1][3] + n[0][2]*m[2][3] + n[0][3];
	register float d2 = n[1][0]*m[0][3] + n[1][1]*m[1][3] + n[1][2]*m[2][3] + n[1][3];
	register float d3 = n[2][0]*m[0][3] + n[2][1]*m[1][3] + n[2][2]*m[2][3] + n[2][3];

	return Vector3
	(
		(d1 * (b2c3_b3c2) + d2 * (c1b3_b1c3) + d3 * (b1c2_c1b2))*rDet,
		(d1 * (a3c2_a2c3) + d2 * (a1c3_c1a3) + d3 * (c1a2_a1c2))*rDet,
		(d1 * (a2b3_a3b2) + d2 * (b1a3_a1b3) + d3 * (a1b2_b1a2))*rDet
	);
}

static Vector4 getNormalizedPlaneEquationReference (const Vector3& a, const Vector3& b, const Vector3& c)
{
	register float	x1 = b.x - a.x;
	register float	y1 = b.y - a.y;
	register float	z1 = b.z - a.z;
	register float	x2 = c.x - a.x;
	register float	y2 = c.y - a.y;
	register float	z2 = c.z - a.z;
	register float	nx = (y1*z2)-(z1*y2);
	register float	ny = (z1*x2)-(x1*z2);
	register float	nz = (x1*y2)-(y1*x2);

	register float	d = nx*nx+ny*ny+nz*nz;

	if ( d >= FLT_MIN )
		d = Math::reciprocalSqrt(d);
	else
		d = 0.f;
	
	nx *= d;
	ny *= d;
	nz *= d;
	
	return Vector4(nx,ny,nz,-(a.x*nx+a.y*ny+a.z*nz));
}

static void transformAABBMinMaxReference (Vector3& dmn, Vector3& dmx, const Vector3& mn, const Vector3& mx, const Matrix4x3& m)
{
	// Perform incremental transformations and then local minmax operations

	register float x  = (mx.x-mn.x);
	register float y  = (mx.y-mn.y);
	register float z  = (mx.z-mn.z);

	{
		register float ax    = x * m[0].x; 
		register float bx    = z * m[0].z; 
		register float dst0x = mn.x * m[0].x + mn.y * m[0].y + mn.z * m[0].z + m[0].w;
		register float dst1x = dst0x + ax; 
		register float dst2x = dst0x + y * m[0].y; 
		register float dst3x = dst2x + ax; 
		register float dst4x = dst0x + bx; 
		register float dst5x = dst1x + bx; 
		register float dst6x = dst2x + bx; 
		register float dst7x = dst3x + bx; 

		register float dmnx = Math::min(dst0x,dst1x);
		register float dmxx = Math::max(dst0x,dst1x);

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
		register float by    = z * m[1].z; 
		register float ay    = x * m[1].x; 
		register float dst0y = mn.x * m[1].x + mn.y * m[1].y + mn.z * m[1].z + m[1].w;
		register float dst1y = dst0y + ay; 
		register float dst2y = dst0y + y * m[1].y;
		register float dst3y = dst2y + ay; 
		register float dst4y = dst0y + by; 
		register float dst5y = dst1y + by; 
		register float dst6y = dst2y + by; 
		register float dst7y = dst3y + by; 

		register float dmny = Math::min(dst0y,dst1y);
		register float dmxy = Math::max(dst0y,dst1y);

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
		register float bz	 = z * m[2].z; 
		register float az	 = x * m[2].x; 
		register float dst0z = mn.x * m[2].x + mn.y * m[2].y + mn.z * m[2].z + m[2].w;
		register float dst1z = dst0z + az; 
		register float dst2z = dst0z + y * m[2].y; 
		register float dst3z = dst2z + az; 
		register float dst4z = dst0z + bz; 
		register float dst5z = dst1z + bz; 
		register float dst6z = dst2z + bz; 
		register float dst7z = dst3z + bz; 

		register float dmnz = Math::min(dst0z,dst1z);
		register float dmxz = Math::max(dst0z,dst1z);

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

static void intFloorTest1()
{
	printf( "janitest: intFloor (test 1)\n" );
	float DPVS_VECTOR_ALIGN(floats[]  = { 1.2f, -1.4f, 2.7f, 0.1f, -0.7f, 0.f, 1.f, -0.99f, 3.2f, 100.1f }) ;
	INT32 DPVS_VECTOR_ALIGN(ints[ sizeof(floats)/sizeof(floats[0]) ]) ;
	const int n = sizeof(floats)/sizeof(floats[0]);
	Math::intFloor( ints, floats, n );
	for ( int i = 0 ; i < n ; ++i )
	{
		float f = floats[i];
		int asmint = ints[i];
		int refint = (int)floor(f);
		if ( asmint != refint )
			printf( "janitest: PS2 asm intFloor(%f)==%i, should be %i\n", f, asmint, refint );
	}
}

static void intFloorTest2()
{
	printf( "janitest: intFloor (test 2)\n" );
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
			printf( "janitest: PS2 asm intFloor(%f)==%i, should be %i\n", f, asmint, refint );
	}
}

static void intFloorTest3() 
{
	printf( "janitest: intFloor (test 3)\n" );
	float floats[103];
	const int n = sizeof(floats)/sizeof(floats[0]);
	for ( int j = 0 ; j < n ; ++j )
		floats[j] = frand() * 1e8f - 10.f * frand();
	for ( int i = 0 ; i < n ; ++i )
	{
		float f = floats[i];
		int refint = (int)floor(f);
		int asmint = intFloorPS2( f );
		if ( asmint != refint )
			printf( "janitest: intFloorPS2(%f)==%i, should be %i\n", f, asmint, refint );
	}
}

static void intCeilTest()
{
	printf( "janitest: intCeil\n" );
	float floats[103];
	const int n = sizeof(floats)/sizeof(floats[0]);
	for ( int j = 0 ; j < n ; ++j )
		floats[j] = frand() * 1e8f - 10.f * frand();
	for ( int i = 0 ; i < n ; ++i )
	{
		float f = floats[i];
		int refint = (int)ceil(f);
		int asmint = intCeilPS2( f );
		if ( asmint != refint )
			printf( "janitest: intCeilPS2(%f)==%i, should be %i\n", f, asmint, refint );
	}
}

static void invertMatrixTest()
{
	printf( "janitest: invertMatrix\n" );

	Matrix4x4 srcm;
	srcm.setColumn( 0, Vector4(0,0,-1,0) );
	srcm.setColumn( 1, Vector4(1,0,0,0) );
	srcm.setColumn( 2, Vector4(0,-1,0,0) );
	srcm.setColumn( 3, Vector4(1,2,3,1) );
	const Matrix4x4 srcm0 = srcm;

	for ( int multloop = 0 ; multloop < 3 ; ++multloop )
	{
		Matrix4x4 refm;
		invertMatrix4x4( refm, srcm );
		Matrix4x3 m;
		invertMatrix( m, reinterpret_cast<const Matrix4x3&>(srcm) );

		for ( int i = 0 ; i < 3 ; ++i )
		{
			const Vector4& row0 = refm[i];
			const Vector4& row1 = m[i];
			Vector4 diff = row0-row1;
			
			float err = diff.length();
			if ( err > 1e-3f )
			{
				printf( "janitest: PS2 asm invertMatrix returned\n" );
				printf( "%s", toString( m ) );
				printf( "but the matrix should be\n" );
				printf( "%s\n", toString( reinterpret_cast<Matrix4x3&>(refm) ) );
				break;
			}
		}

		srcm.productFromLeft( srcm0 );
	}
}

static void transformPlaneUniformTest()
{
	printf( "janitest: transformPlaneUniform\n" );

	const int n = 103;
	Vector4 pl[n];
	for ( int i = 0 ; i < n ; ++i )
	{
		Vector3 n( frand(), frand(), frand() );
		while ( n.length() < 1e-10f )
			n = Vector3( frand(), frand(), frand() );
		n.normalize();
		pl[i] = Vector4( n.x, n.y, n.z, 1e6f*frand()-frand()*10.f );
	}
	
	Matrix4x3 srcm;
	srcm.setColumn( 0, Vector3(0,0,-1) );
	srcm.setColumn( 1, Vector3(1,0,0) );
	srcm.setColumn( 2, Vector3(0,-1,0) );
	srcm.setColumn( 3, Vector3(1,2,3) );

	for ( int i = 0 ; i < n ; ++i )
	{
		Vector4 pl0 = pl[i];
		Vector4 refpl = referenceTransformPlaneUniform( pl0, srcm );
		Vector4 asmpl;
		::DPVS::Math::transformPlanes( &asmpl, &pl0, srcm, 1 );

		Vector4 diff = asmpl-refpl;
		float nerr = Vector3( diff.x, diff.y, diff.z ).length();
		float werr = fabs(diff.w);
		float werrRange = fabs(refpl.w);
		if ( werrRange < 1.f )
			werrRange = 1.f;
		if ( nerr > 1e-3f || werr/werrRange > 1e-3f )
			printf( "janitest: transformPlaneUniform(%f,%f,%f,%f)==(%f,%f,%f,%f), should be (%f,%f,%f,%f)\n", pl0[0], pl0[1], pl0[2], pl0[3], asmpl[0], asmpl[1], asmpl[2], asmpl[3], refpl[0], refpl[1], refpl[2], refpl[3] );
	}
}

static void minMaxTest1()
{
	printf( "janitest: minMax\n" );

	const int n = 103;
	Vector3  v[n];
	for ( int i = 0 ; i < n ; ++i )
	{
		for ( int j = 0 ; j < 3 ; ++j )
		{
			v[i][j] = frand() * 1e3f + frand() * 10.f;
		}
	}

	Vector3 mn, mx;
	Math::minMax( mn, mx, v, n );
	for ( int i = 0 ; i < n ; ++i )
	{
		for ( int j = 0 ; j < 3 ; ++j )
		{
			if ( v[i][j] > mx[j] )
			{
				printf( "janitest: minmax failed, mx=(%f,%f,%f) but found (%f,%f,%f)\n", mx.x, mx.y, mx.z, v[i].x, v[i].y, v[i].z );
				i = n;
				break;
			}
			else if ( v[i][j] < mn[j] )
			{
				printf( "janitest: minmax failed, mn=(%f,%f,%f) but found (%f,%f,%f)\n", mn.x, mn.y, mn.z, v[i].x, v[i].y, v[i].z );
				i = n;
				break;
			}
		}
	}
}

static void minMaxTest2()
{
	printf( "janitest: min/max(int,int)\n" );

	const int n = 103;
	int i1[n];
	int i2[n];
	for ( int i = 0 ; i < n ; ++i )
	{
		i1[i] = rand() - RAND_MAX/2;
		i2[i] = rand() - RAND_MAX/2;
	}

	for ( int i = 0 ; i < n ; ++i )
	{
		int mx = maxPS2( i1[i], i2[i] );
		int mn = minPS2( i1[i], i2[i] );

		if ( i1[i] > mx || i2[i] > mx || i1[i] < mn || i2[i] < mn )
		{
			printf( "janitest: min/max(int,int) failed:\n" );
			printf( "          i1[%i] == %i,\n", i, i1[i] );
			printf( "          i2[%i] == %i,\n", i, i2[i] );
			printf( "             min == %i,\n", mn );
			printf( "             max == %i,\n", mx );
		}
	}
}

static void transform3Test()
{
	printf( "janitest: transform (Vector3)\n" );

	Matrix4x4 srcm;
	srcm.setColumn( 0, Vector4(0,0,-1,0) );
	srcm.setColumn( 1, Vector4(1,0,0,0) );
	srcm.setColumn( 2, Vector4(0,-1,0,0) );
	srcm.setColumn( 3, Vector4(2,3,4,1) );

	const int n = 103;
	Vector3 v[n];
	for ( int i = 0 ; i < n ; ++i )
	{
		for ( int j = 0 ; j < 3 ; ++j )
		{
			v[i][j] = frand() * 1e3f + frand() * 10.f;
		}
	}

	Vector4 v1asm[n];
	Math::transform( v1asm, v, n, srcm );
	Vector4 v1ref[n];
	referenceTransform( v1ref, v, n, srcm );

	for ( int i = 0 ; i < n ; ++i )
	{
		Vector4 diff = v1asm[i] - v1ref[i];
		float dist = diff.length();
		if ( dist > 5.5f )
		{
			printf( "janitest: transform3 failed, v1asm (%f,%f,%f) and v1ref=(%f,%f,%f)\n", v1asm[i].x, v1asm[i].y, v1asm[i].z, v1ref[i].x, v1ref[i].y, v1ref[i].z );
		}
	}
}

static void transform4Test()
{
	printf( "janitest: transform (Vector4)\n" );

	Matrix4x4 srcm;
	srcm.setColumn( 0, Vector4(0,0,-1,0) );
	srcm.setColumn( 1, Vector4(1,0,0,0) );
	srcm.setColumn( 2, Vector4(0,-1,0,0) );
	srcm.setColumn( 3, Vector4(2,3,4,1) );

	const int n = 103;
	Vector4 v[n];
	for ( int i = 0 ; i < n ; ++i )
	{
		for ( int j = 0 ; j < 4 ; ++j )
		{
			v[i][j] = frand() * 1e3f + frand() * 10.f;
		}
	}

	Vector4 v1asm[n];
	Math::transform( v1asm, v, n, srcm );
	Vector4 v1ref[n];
	referenceTransform( v1ref, v, n, srcm );

	for ( int i = 0 ; i < n ; ++i )
	{
		Vector4 diff = v1asm[i] - v1ref[i];
		float dist = diff.length();
		if ( dist > 5.5f )
		{
			printf( "janitest: transform4 failed, v1asm (%f,%f,%f,%f) and v1ref=(%f,%f,%f,%f)\n", v1asm[i].x, v1asm[i].y, v1asm[i].z, v1asm[i].w, v1ref[i].x, v1ref[i].y, v1ref[i].z, v1ref[i].w );
		}

		Vector4 v1asmSingle = srcm.transform( v[i] );
		Vector4 diff2 = v1asmSingle - v1ref[i];
		float dist2 = diff2.length();
		if ( dist2 > 5.5f )
		{
			printf( "janitest: transform failed, v1asmSingle (%f,%f,%f,%f) and v1ref=(%f,%f,%f,%f)\n", v1asmSingle.x, v1asmSingle.y, v1asmSingle.z, v1asmSingle.w, v1ref[i].x, v1ref[i].y, v1ref[i].z, v1ref[i].w );
		}
	}
}

static void rasterToFixedTest()
{
	printf( "janitest: rasterToFixed\n" );

	const int n = 103;
	Vector2 DPVS_VECTOR_ALIGN(v[n]) ;
	for ( int i = 0 ; i < n ; ++i )
	{
		for ( int j = 0 ; j < 2 ; ++j )
		{
			v[i][j] = frand() * 4e3f + frand() * 10.f;
		}
	}

	Vector2 scale( frand()*4.f, frand()*4.f );

	Vector2i v1ref[n];
	for( int i = 0 ; i < n ; i++ )
	{
		v1ref[i][0] = Math::intFloor( scale.x * v[i].x );
		v1ref[i][1] = Math::intFloor( scale.y * v[i].y );
	}

	Vector2i DPVS_VECTOR_ALIGN(v1asm[n]);
	rasterToFixed( v1asm, v, scale, n );

	for( int i = 0 ; i < n ; i++ )
	{
		if ( v1ref[i] != v1asm[i] )
		{
			printf( "janitest: rasterToFixed failed.\n" );
			printf( "          v[%i]     = (%f, %f)\n", i, v[i][0], v[i][1] );
			printf( "          scale     = (%f, %f)\n", scale[0], scale[1] );
			printf( "          v1ref[%i] = (%i, %i)\n", i, v1ref[i][0], v1ref[i][1] );
			printf( "          v1asm[%i] = (%i, %i)\n", i, v1asm[i][0], v1asm[i][1] );
			break;
		}
	}
}

static void objectToRasterTest1()
{
	printf( "janitest: objectToRaster (Vector2)\n" );

	const int n = 103;
	Vector3 DPVS_VECTOR_ALIGN(v[n]) ;
	for ( int i = 0 ; i < n ; ++i )
	{
		for ( int j = 0 ; j < 3 ; ++j )
		{
			v[i][j] = frand() * 4e3f + frand() * 10.f;
		}
	}

	FloatRectangle viewPort;
	viewPort.x0 = 0.f;
	viewPort.y0 = 0.f;
	viewPort.x1 = 200.f;
	viewPort.y1 = 100.f;

	Matrix4x4 mtx;
	mtx.setColumn( 0, Vector4(0,0,-1,0) );
	mtx.setColumn( 1, Vector4(1,0,0,0) );
	mtx.setColumn( 2, Vector4(0,-1,0,0) );
	mtx.setColumn( 3, Vector4(2,3,4,1) );

	Vector2 v1ref[n];
	objectToRasterReference( v1ref, v, n, mtx, viewPort );

	Vector2 DPVS_VECTOR_ALIGN(v1asm[n]) ;
	objectToRaster( v1asm, v, n, mtx, viewPort );

	for( int i = 0 ; i < n ; i++ )
	{
		Vector2 diff; 
		diff[0] = v1ref[i][0] - v1asm[i][0];
		diff[1] = v1ref[i][1] - v1asm[i][1];
		float len = sqrtf( diff[0]*diff[0] + diff[1]*diff[1] );

		if ( len > 1.f )
		{
			printf( "janitest: objectToRaster (Vector2) failed.\n" );
			printf( "          v[%i]     = (%f, %f)\n", i, v[i][0], v[i][1] );
			printf( "          v1ref[%i] = (%f, %f)\n", i, v1ref[i][0], v1ref[i][1] );
			printf( "          v1asm[%i] = (%f, %f)\n", i, v1asm[i][0], v1asm[i][1] );
			break;
		}
	}
}

static void objectToRasterTest2()
{
	printf( "janitest: objectToRaster (Vector3)\n" );

	const int n = 103;
	Vector3 DPVS_VECTOR_ALIGN(v[n]) ;
	for ( int i = 0 ; i < n ; ++i )
	{
		for ( int j = 0 ; j < 3 ; ++j )
		{
			v[i][j] = frand() * 4e3f + frand() * 10.f;
		}
	}

	FloatRectangle viewPort;
	viewPort.x0 = 0.f;
	viewPort.y0 = 0.f;
	viewPort.x1 = 200.f;
	viewPort.y1 = 100.f;

	Matrix4x4 mtx;
	mtx.setColumn( 0, Vector4(0,0,-1,0) );
	mtx.setColumn( 1, Vector4(1,0,0,0) );
	mtx.setColumn( 2, Vector4(0,-1,0,0) );
	mtx.setColumn( 3, Vector4(2,3,4,1) );

	Vector3 v1ref[n];
	objectToRasterReference( v1ref, v, n, mtx, viewPort );

	Vector3 DPVS_VECTOR_ALIGN(v1asm[n]) ;
	objectToRaster( v1asm, v, n, mtx, viewPort );

	for( int i = 0 ; i < n ; i++ )
	{
		Vector3 diff; 
		diff[0] = v1ref[i][0] - v1asm[i][0];
		diff[1] = v1ref[i][1] - v1asm[i][1];
		float len = sqrtf( diff[0]*diff[0] + diff[1]*diff[1] );

		if ( len > 1.f )
		{
			printf( "janitest: objectToRaster (Vector3) failed.\n" );
			printf( "          v[%i]     = (%f, %f, %f)\n", i, v[i][0], v[i][1], v[i][2] );
			printf( "          v1ref[%i] = (%f, %f, %f)\n", i, v1ref[i][0], v1ref[i][1], v1ref[i][2] );
			printf( "          v1asm[%i] = (%f, %f, %f)\n", i, v1asm[i][0], v1asm[i][1], v1asm[i][2] );
			break;
		}
	}
}

static void minMaxTransformTest()
{
	printf( "janitest: minMaxTransform\n" );

	Matrix4x3 srcm;
	srcm.setColumn( 0, Vector3(0,0,-1) );
	srcm.setColumn( 1, Vector3(1,0,0) );
	srcm.setColumn( 2, Vector3(0,-1,0) );
	srcm.setColumn( 3, Vector3(2,3,4) );

	const int n = 103;
	Vector3  DPVS_VECTOR_ALIGN(v[n]);
	for ( int i = 0 ; i < n ; ++i )
	{
		for ( int j = 0 ; j < 3 ; ++j )
		{
			v[i][j] = frand() * 1e3f + frand() * 10.f;
		}
	}

	Vector3 asmMin, asmMax;
	Math::minMaxTransform( asmMin, asmMax, v, srcm, n );
	Vector3 refMin, refMax;
	minMaxTransformReference( refMin, refMax, v, srcm, n );

	float minDist = (asmMin - refMin).length();
	float maxDist = (asmMax - refMax).length();
	if ( minDist > 6.2f )
	{
		printf( "janitest: minMaxTransform failed, asmMin=(%f,%f,%f) and refMin=(%f,%f,%f), dist=%f\n", asmMin.x, asmMin.y, asmMin.z, refMin.x, refMin.y, refMin.z, minDist );
	}
	if ( maxDist > 6.2f )
	{
		printf( "janitest: minMaxTransform failed, asmMax=(%f,%f,%f) and refMax=(%f,%f,%f), dist=%f\n", asmMax.x, asmMax.y, asmMax.z, refMax.x, refMax.y, refMax.z, maxDist );
	}
}

static void matrixTests()
{
	// transpose 4x4
	{
		printf( "janitest: transpose 4x4\n" );

		Matrix4x4 mtx;
		for ( int j = 0 ; j < 4 ; ++j )
		{
			for ( int i = 0 ; i < 4 ; ++i )
			{
				mtx[i][j] = j*4+i;
			}
		}
		Matrix4x4 tmtx; 
		tmtx.transpose( mtx );
		for ( int j = 0 ; j < 4 ; ++j )
		{
			for ( int i = 0 ; i < 4 ; ++i )
			{
				JANI_ASSERT( tmtx[i][j] == mtx[j][i] );
			}
		}

		tmtx.transpose();
		for ( int j = 0 ; j < 4 ; ++j )
		{
			for ( int i = 0 ; i < 4 ; ++i )
			{
				JANI_ASSERT( tmtx[i][j] == mtx[i][j] );
			}
		}
	}

	// productFromLeft 4x3
	{
		printf( "janitest: productFromLeft 4x3\n" );

		Matrix4x3 m, im;
		m.setColumn( 0, Vector3(0,0,-1) );
		m.setColumn( 1, Vector3(1,0,0) );
		m.setColumn( 2, Vector3(0,-1,0) );
		m.setColumn( 3, Vector3(2,3,4) );
		invertMatrix( im, m );

		Matrix4x3 mr[8];
		int mri = 0;
		mr[mri] = m; mr[mri++].productFromLeft( im );
		Math::productFromLeft( mr[mri++], m, im );

		for ( int k = 0 ; k < mri ; ++k )
		{
			const Matrix4x3& m = mr[k];
			for ( int j = 0 ; j < 3 ; ++j )
			{
				for ( int i = 0 ; i < 4 ; ++i )
				{
					float v = (i == j ? 1.f : 0.f);
					JANI_ASSERT( fabsf(m[j][i]-v) < 0.001f );
				}
			}
		}
	}

	// productFromLeft 4x4
	{
		printf( "janitest: productFromLeft 4x4\n" );

		Matrix4x4 m, im;
		m.setColumn( 0, Vector4(0,0,-1,0) );
		m.setColumn( 1, Vector4(1,0,0,0) );
		m.setColumn( 2, Vector4(0,-1,0,0) );
		m.setColumn( 3, Vector4(2,3,4,1) );
		invertMatrix( im, m );

		Matrix4x4 mr[8];
		int mri = 0;
		mr[mri] = m; mr[mri++].productFromLeft( im );
		Math::productFromLeft( mr[mri++], m, im );
		Math::productFromLeft( mr[mri++], reinterpret_cast<const Matrix4x3&>(m), reinterpret_cast<const Matrix4x3&>(im) );
		Math::productFromLeft( mr[mri++], reinterpret_cast<const Matrix4x3&>(m), im );

		for ( int k = 0 ; k < mri ; ++k )
		{
			const Matrix4x4& m = mr[k];
			for ( int j = 0 ; j < 4 ; ++j )
			{
				for ( int i = 0 ; i < 4 ; ++i )
				{
					float v = (i == j ? 1.f : 0.f);
					JANI_ASSERT( fabsf(m[j][i]-v) < 0.001f );
				}
			}
		}
	}

	// identity 4x4
	{
		printf( "janitest: identity 4x4\n" );

		Matrix4x4 m;
		for ( int i = 0 ; i < 4 ; ++i )
			m.setColumn( i, Vector4(2,2,2,2) );
		m.ident();
		for ( int j = 0 ; j < 4 ; ++j )
		{
			for ( int i = 0 ; i < 4 ; ++i )
			{
				float v = (i == j ? 1.f : 0.f);
				JANI_ASSERT( m[j][i] == v );
			}
		}
	}

	// identity 4x3
	{
		printf( "janitest: identity 4x3\n" );

		Matrix4x3 m;
		for ( int i = 0 ; i < 4 ; ++i )
			m.setColumn( i, Vector3(2,2,2) );
		m.ident();
		for ( int j = 0 ; j < 3 ; ++j )
		{
			for ( int i = 0 ; i < 4 ; ++i )
			{
				float v = (i == j ? 1.f : 0.f);
				JANI_ASSERT( m[j][i] == v );
			}
		}
	}

	// getMaxScale test
	{
		printf( "janitest: getMaxScale\n" );

		Matrix4x4 m;
		for ( int j = 0 ; j < 4 ; ++j )
			for ( int i = 0 ; i < 4 ; ++i )
				m[j][i] = i+j*4;
		
		float asmMaxScale = m.getMaxScale();
		float refMaxScale = getMaxScaleReference( m );
		JANI_ASSERT( fabsf(refMaxScale-asmMaxScale) < 1e-2f );
	}

	// getInverseTranslation 4x3
	{
		printf( "janitest: getInverseTranslation 4x3\n" );

		Matrix4x3 m;
		m.setColumn( 0, Vector3(0,0,-1) );
		m.setColumn( 1, Vector3(1,0,0) );
		m.setColumn( 2, Vector3(0,-1,0) );
		m.setColumn( 3, Vector3(2,3,4) );

		Vector3 refInvT = getInverseTranslationReference( m );
		Vector3 asmInvT = getInverseTranslation( m );

		Vector3 diff = asmInvT - refInvT;
		if ( diff.length() > 1e-3f )
			printf( "janitest: getInverseTranslation(4x3,4x3) failed, asm=(%f, %f, %f), ref=(%f, %f, %f)\n", asmInvT.x, asmInvT.y, asmInvT.z, refInvT.x, refInvT.y, refInvT.z );
	}

	// getInverseTranslation 4x3, 4x3
	{
		printf( "janitest: getInverseTranslation 4x3,4x3\n" );

		Matrix4x3 m;
		m.setColumn( 0, Vector3(0,0,-1) );
		m.setColumn( 1, Vector3(1,0,0) );
		m.setColumn( 2, Vector3(0,-1,0) );
		m.setColumn( 3, Vector3(2,3,4) );
		Matrix4x3 m2;
		m2.setColumn( 0, Vector3(0,1,0) );
		m2.setColumn( 1, Vector3(0,0,1) );
		m2.setColumn( 2, Vector3(1,0,0) );
		m2.setColumn( 3, Vector3(7,-2,3) );

		Vector3 refInvT = getInverseTranslationReference( m, m2 );
		Vector3 asmInvT = getInverseTranslation( m, m2 );

		Vector3 diff = asmInvT - refInvT;
		if ( diff.length() > 1e-3f )
			printf( "janitest: getInverseTranslation(4x3) failed, asm=(%f, %f, %f), ref=(%f, %f, %f)\n", asmInvT.x, asmInvT.y, asmInvT.z, refInvT.x, refInvT.y, refInvT.z );
	}

	// unitBoxToScreen
	{
		printf( "janitest: unitBoxToScreen\n" );

		Matrix4x3 a;
		a.setColumn( 0, Vector3(0,0,-1) );
		a.setColumn( 1, Vector3(1,0,0) );
		a.setColumn( 2, Vector3(0,-1,0) );
		a.setColumn( 3, Vector3(2,3,4) );
		Matrix4x3 b;
		b.setColumn( 0, Vector3(0,1,0) );
		b.setColumn( 1, Vector3(0,0,1) );
		b.setColumn( 2, Vector3(1,0,0) );
		b.setColumn( 3, Vector3(7,-2,3) );
		Matrix4x4 c;
		c.setColumn( 0, Vector4(1,0,0,0) );
		c.setColumn( 1, Vector4(0,0,1,0) );
		c.setColumn( 2, Vector4(0,-1,0,0) );
		c.setColumn( 3, Vector4(-10,0,-1,1) );

		Matrix4x4 v;
		unitBoxToScreen( v, a, b, c );

		Matrix4x4 vref;
		productFromLeft( vref, a, b );
		vref.productFromLeft( c );

		for ( int j = 0 ; j < 4 ; ++j )
		{
			Vector4 diff = v[j] - vref[j];
			float dist = diff.length();
			if ( dist > 1e-3f )
			{
				printf( "janitest: unitBoxToScreen failed.\n" );
				printf( "          vasm[%i] = (%f, %f, %f, %f)\n", j, v[j][0], v[j][1], v[j][2], v[j][3] );
				printf( "          vref[%i] = (%f, %f, %f, %f)\n", j, vref[j][0], vref[j][1], vref[j][2], vref[j][3] );
			}
		}
	}

	// isUniform
	{
		printf( "janitest: isUniform\n" );

		Matrix4x4 m;
		m.setColumn( 0, Vector4(1,0,0,0) );
		m.setColumn( 1, Vector4(0,0,1,0) );
		m.setColumn( 2, Vector4(0,-1,0,0) );
		m.setColumn( 3, Vector4(-10,0,-1,1) );

		JANI_ASSERT( m.isUniform() );

		m[2][1] *= -3.f;
		JANI_ASSERT( !m.isUniform() );
	}
}

static void getNormalizedPlaneEquation()
{
	printf( "janitest: getNormalizedPlaneEquation\n" );

	Vector3 v0( 2,2,2 );
	Vector3 v1( 2,0,1 );
	Vector3 v2( -1,-1,-3 );
	Vector4 asmPl = getNormalizedPlaneEquation( v0, v1, v2 );
	Vector4 refPl = getNormalizedPlaneEquationReference( v0, v1, v2 );

	Vector4 diff = asmPl - refPl;
	float dist = diff.length();
	if ( dist > 1e-3f )
	{
		printf( "janitest: getNormalizedPlaneEquation failed, asmPl=(%f, %f, %f, %f) and refPl=(%f, %f, %f, %f)\n", asmPl[0], asmPl[1], asmPl[2], asmPl[3], refPl[0], refPl[1], refPl[2], refPl[3] );
	}
}

static void transformAABBMinMaxTest()
{
	printf( "janitest: transformAABBMinMax\n" );

	Matrix4x3 srcm;
	srcm.setColumn( 0, Vector3(0,0,-1) );
	srcm.setColumn( 1, Vector3(1,0,0) );
	srcm.setColumn( 2, Vector3(0,-1,0) );
	srcm.setColumn( 3, Vector3(2,3,4) );

	const int n = 501 * 2;
	JANI_ASSERT( 0 == (n&1) );
	Vector3 v[n];
	for ( int i = 0 ; i < n ; ++i )
	{
		for ( int j = 0 ; j < 3 ; ++j )
		{
			v[i][j] = frand() * 1e3f + frand() * 10.f;
		}
	}

	Vector3 v1asm[n];
	Vector3 v1ref[n];
	for ( int i = 0 ; i < n ; i += 2 )
	{
		transformAABBMinMax( v1asm[i], v1asm[i+1], v[i], v[i+1], srcm );
		transformAABBMinMaxReference( v1ref[i], v1ref[i+1], v[i], v[i+1], srcm );

		for ( int j = 0 ; j < 2 ; ++j )
		{
			int k = i + j;

			Vector3 diff = v1asm[k] - v1ref[k];
			float dist = diff.length();

			if ( dist > 5.5f )
			{
				printf( "janitest: transformAABBMinMax [%i] failed, asm (%f,%f,%f) and ref=(%f,%f,%f)\n", k, v1asm[k].x, v1asm[k].y, v1asm[k].z, v1ref[k].x, v1ref[k].y, v1ref[k].z );
			}
		}
	}
}

static void matrixConverterTest()
{
	printf( "janitest: MatrixConverter\n" );

	Matrix4x4 s44, t44;
	s44.setColumn( 0, Vector4(1,2,3,0) );
	s44.setColumn( 1, Vector4(5,6,7,0) );
	s44.setColumn( 2, Vector4(9,10,11,0) );
	s44.setColumn( 3, Vector4(0,0,0,1) );
	t44.transpose( s44 );

	Matrix4x4 d44;
	Matrix4x3 d43;

	MatrixConverter::setColumnMajor( true );
	MatrixConverter::convertMatrix( d44, s44 );
	MatrixConverter::convertMatrix( d43, s44 );
	JANI_ASSERT( d44 == t44 );
	JANI_ASSERT( d43 == reinterpret_cast<Matrix4x3&>(t44) );

	MatrixConverter::setColumnMajor( false );
	MatrixConverter::convertMatrix( d44, s44 );
	MatrixConverter::convertMatrix( d43, s44 );
	JANI_ASSERT( d44 == s44 );
	JANI_ASSERT( d43 == reinterpret_cast<Matrix4x3&>(s44) );
}

static float dotReference( const Vector4& v1, const Vector4& v2 )
{
	return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z + v1.w*v2.w; 
}

static float dotReference( const Vector3& v1, const Vector3& v2 )
{
	return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z; 
}

static float dotPS2( const Vector4& v1, const Vector4& v2 )
{
	register float ret;
	asm (
		"   lwc1	$f10, 0(%1); "
		"\n lwc1	$f11, 4(%1)"
		"\n lwc1	$f12, 8(%1)"
		"\n lwc1	$f13, 12(%1)"
		"\n lwc1	$f20, 0(%2)"
		"\n lwc1	$f21, 4(%2)"
		"\n lwc1	$f22, 8(%2)"
		"\n lwc1	$f23, 12(%2)"
		"\n mula.s	$f10, $f20"
		"\n madda.s	$f11, $f21"
		"\n madda.s	$f12, $f22"
		"\n madd.s	%0, $f13, $f23"
	 : "=f"(ret) : "r"(&v1), "r"(&v2) : "$f10", "$f11", "$f12", "$f13", "$f20", "$f21", "$f22", "$f23" );
	return ret;
}

static float dotPS2( const Vector3& v1, const Vector3& v2 )
{
	register float ret;
	asm (
		" lwc1	$f10, 0(%1); "
		"\n lwc1	$f11, 4(%1)"
		"\n lwc1	$f12, 8(%1)"
		"\n lwc1	$f20, 0(%2)"
		"\n lwc1	$f21, 4(%2)"
		"\n lwc1	$f22, 8(%2)"
		"\n mula.s	$f10, $f20"
		"\n madda.s	$f11, $f21"
		"\n madd.s	%0, $f12, $f22"
	 : "=f"(ret) : "r"(&v1), "r"(&v2) : "$f10", "$f11", "$f12", "$f20", "$f21", "$f22" );
	return ret;
}

static void vectorTests()
{
	const int n = 103;
	Vector4 v3[n];
	Vector4 v4[n];
	float fasm4[4*n];
	for ( int i = 0 ; i < n ; ++i )
	{
		for ( int j = 0 ; j < 4 ; ++j )
			v4[i][j] = frand() * 1e3f + frand() * 10.f;
		for ( int j = 0 ; j < 3 ; ++j )
			v3[i][j] = frand() * 1e3f + frand() * 10.f;
	}

	// dot (single)
	{
		printf( "janitest: dot (Vector4, Vector3)\n" );

		for ( int i = 0 ; i < n ; ++i )
		{
			for ( int j = 0 ; j < n ; ++j )
			{
				Vector4 a4 = v4[i];
				Vector4 b4 = v4[j];
				float d1 = dotPS2(a4,b4);
				float d2 = dotReference(a4,b4);
				float diff = fabsf( d1 - d2 );
				if ( diff > 1e-2f )
				{
					printf( "janitest: Vector4::dot failed:\n" );
					printf( "          a = (%f %f %f %f)\n", a4[0], a4[1], a4[2], a4[3] );
					printf( "          b = (%f %f %f %f)\n", b4[0], b4[1], b4[2], b4[3] );
					printf( "          dotPS2 = %f\n", d1 );
					printf( "          dotRef = %f\n", d2 );
				}

				Vector4 a3 = v3[i];
				Vector4 b3 = v3[j];
				d1 = dotPS2(a3,b3);
				d2 = dotReference(a3,b3);
				diff = fabsf( d1 - d2 );
				if ( diff > 1e-2f )
				{
					printf( "janitest: Vector3::dot failed:\n" );
					printf( "          a = (%f %f %f)\n", a3[0], a3[1], a3[2] );
					printf( "          b = (%f %f %f)\n", b3[0], b3[1], b3[2] );
					printf( "          dotPS2 = %f\n", d1 );
					printf( "          dotRef = %f\n", d2 );
				}
			}
		}
	}

	// dot (array)
	{
		printf( "janitest: dot (Vector4*)\n" );

		for ( int i = 0 ; i < n ; ++i )
		{
			dot( fasm4, v4, v4[i], n );

			for ( int j = 0 ; j < n ; ++j )
			{
				Vector4 a4 = v4[i];
				Vector4 b4 = v4[j];
				float d1 = fasm4[j];
				float d2 = dotReference(a4,b4);
				float diff = fabsf( d1 - d2 );
				if ( diff > 0.5f )
				{
					printf( "janitest: dot(Vector4*) failed:\n" );
					printf( "          a = (%f %f %f %f)\n", a4[0], a4[1], a4[2], a4[3] );
					printf( "          b = (%f %f %f %f)\n", b4[0], b4[1], b4[2], b4[3] );
					printf( "          asm = %f\n", d1 );
					printf( "          ref = %f\n", d2 );
				}
			}
		}
	}
}
#endif // JANITEST

void janiTest (void)
{
#ifdef JANITEST

	srand( 12345 );

	intFloorTest1();
	intFloorTest2();
	intFloorTest3();
	intCeilTest();
	invertMatrixTest();
	transformPlaneUniformTest();
	minMaxTest1();
	minMaxTest2();
	transform3Test();
	transform4Test();
	rasterToFixedTest();
	objectToRasterTest1();
	objectToRasterTest2();
	minMaxTransformTest();
	matrixTests();
	getNormalizedPlaneEquation();
	transformAABBMinMaxTest();
	matrixConverterTest();
	vectorTests();

#endif
}

} }

//PS2-END
//------------------------------------------------------------------------
