#ifndef __DPVSMATH_HPP
#define __DPVSMATH_HPP
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
 * Description: 	Some auxiliary math routines used by all the
 *					modules
 *
 * $Archive: /dpvs/implementation/include/dpvsMath.hpp $
 * $Author: wili $ 
 * $Revision: #2 $
 * $Modtime: 13.02.02 12:56 $
 * $Date: 2003/11/18 $
 * 
 ******************************************************************************/

#if !defined (__DPVSMATRIX_HPP)
#	include "dpvsMatrix.hpp"				
#endif

namespace DPVS
{

class FloatRectangle;

namespace Math
{
	//void		testFloatStack				(void);

	int			divFixed32_16				(int a, int b);
	void		dot							(float* dst, const Vector4* src, const Vector4& p, int N);
	int			extractSigns				(const Vector3&);
	Vector3		getInverseTranslation		(const Matrix4x3&);
	Vector3		getInverseTranslation		(const Matrix4x3& a, const Matrix4x3& b);
	Vector4		getNormalizedPlaneEquation	(const Vector3& a, const Vector3& b, const Vector3& c);
	void		intFloor					(INT32* dst, const float* src, int N);
	bool		isInvertible				(const Matrix4x3&);
	bool		isUniformRotationMatrix		(const Matrix4x3&);
	bool		isUniformRotationMatrix		(const Matrix4x4&);
	bool		isPerspectiveProjectionMatrix(const Matrix4x4&);
	void		invertMatrix				(Matrix4x3& dst, const Matrix4x3& src);
	void		invertMatrix				(Matrix4x4& dst, const Matrix4x4& src);
	void		invertMatrix				(Matrix4x3& dst);
	void		invertMatrix				(Matrix4x4& dst);
	bool		isFinite					(const Vector3&);
	bool		isFinite					(const Vector4&);
	bool		isFinite					(const Matrix4x3&);
	bool		isFinite					(const Matrix4x4&);
	float		maxDist						(const Vector3& pt, const Vector3* src, int N);
	void		minMax						(Vector2& mn, Vector2& mx, const Vector2* src, int N);
	void		minMax						(Vector3& mn, Vector3& mx, const Vector3* src, int N);
	void		minMaxTransform				(Vector3& mn, Vector3& mx, const Vector3* src, const Matrix4x3& m, int N);
	bool		minMaxTransformDivByW		(Vector3& mn, Vector3& mx, const Vector3* src, const Matrix4x4& m, int N);
	void		objectToRaster				(Vector2* dest, const Vector3* src, int N, const Matrix4x4& mtx, const FloatRectangle& viewPort);
	void		objectToRaster				(Vector3* dest, const Vector3* src, int N, const Matrix4x4& mtx, const FloatRectangle& viewPort);
	void		productFromLeft				(Matrix4x3& d, const Matrix4x3& m, const Matrix4x3& n);
	void		productFromLeft				(Matrix4x4& d, const Matrix4x3& m, const Matrix4x3& n);
	void		productFromLeft				(Matrix4x4& d, const Matrix4x3& m, const Matrix4x4& n);
	void		productFromLeftProjection	(Matrix4x4& d, const Matrix4x3& a, const Matrix4x4& b);
	void		productFromLeft				(Matrix4x4& d, const Matrix4x4& m, const Matrix4x4& n);
	void		screenToRaster				(Vector2* dst, int N, const FloatRectangle& raster);
	void		screenToRaster				(Vector3* dst, int N, const FloatRectangle& raster);
	void		screenToRasterDivByW		(Vector2* dst, const Vector4* src, int N, const FloatRectangle& raster);
	void		rasterToFixed				(Vector2i* dst, const Vector2* src, const Vector2& scale, int N);
	void		transform					(Vector4* dest, const Vector3* src, int N, const Matrix4x4& mtx);
	void		transform					(Vector4* dest, const Vector4* src, int N, const Matrix4x4& mtx);
//	void		transformAABB				(Vector4 dst[8], const Vector3& mn, const Vector3& mx, const Matrix4x4& m);
	void		transformAABBMinMax			(Vector3& dmn, Vector3& dmx, const Vector3& mn, const Vector3& mx, const Matrix4x3& m);
	bool		transformAndDivByW			(Vector3& d, const Vector3& s, const Matrix4x4& m);
	void		transformAndDivByW			(Vector3* dest, const Vector3* src, int N, const Matrix4x4& mtx);
	void		transformOBBVertices		(Vector3 dst[8], const Matrix4x3& obb, const Matrix4x3& objectToCell);
	Vector4		transformPerspective		(const Vector4& s, const Matrix4x4& m);
	Vector4		transformPerspective		(const Vector3& s, const Matrix4x4& m);
	void		transformUnitBox			(Vector3 dst[8], const Matrix4x3& m);
	void		transformUnitBoxXYDivByW	(Vector2 dst[8], const Matrix4x4& m);
	bool		transformUnitBoxXYZDivByW	(Vector3 dst[8], const Matrix4x4& m);
	void		transformPlanes				(Vector4* dst, const Vector4* src, const Matrix4x3& invMtx, int N);
	void		unitBoxToScreen				(Matrix4x4& v, const Matrix4x3& a, const Matrix4x3& b, const Matrix4x4& c);

	// testing testing... 
	void		ppcTest						(void);
	void		janiTest					(void);
	void		invertMatrix4x4				(Matrix4x4&, const Matrix4x4&);


inline void invertMatrix (Matrix4x4& d)
{
	invertMatrix (d,d);
}

inline void invertMatrix (Matrix4x3& d)
{
	invertMatrix (d,d);
}

DPVS_FORCE_INLINE void validateMatrix (const Matrix4x4& s)
{
	s;
#if defined (DPVS_DEBUG)
	int i,j;
	for (j = 0; j < 4; j++)
	for (i = 0; i < 4; i++)
		DPVS_ASSERT (isValidFloat(s[j][i]));
#endif
}

DPVS_FORCE_INLINE void validateMatrix (const Matrix4x3& s)
{
	s;
#if defined (DPVS_DEBUG)
	int i,j;
	for (j = 0; j < 3; j++)
	for (i = 0; i < 4; i++)
		DPVS_ASSERT (isValidFloat(s[j][i]));
#endif
}

DPVS_FORCE_INLINE bool transformAndDivByW (Vector3& d, const Vector3& s, const Matrix4x4& m)
{
	float oow = (s.x * m[3].x + s.y * m[3].y + s.z * m[3].z + m[3].w);
	if (oow <= 0.0f)
		return false;
	oow = 1.0f / oow;
	d.x = (s.x*m[0].x+s.y*m[0].y+s.z*m[0].z+m[0].w) * oow;
	d.y = (s.x*m[1].x+s.y*m[1].y+s.z*m[1].z+m[1].w) * oow;
	d.z = (s.x*m[2].x+s.y*m[2].y+s.z*m[2].z+m[2].w) * oow;
	return true;
}

// Classifies a vector (such as a normal) into one of eight
// categories based on the components' signs. This is a common
// operation used in many AABB and octree-related algorithms.
// The routine uses bit mathematics to avoid comparisons
// (approx. 3.5 times faster on Intel machines than the floating
// point version).
//
// 0 = negative x, negative y, negative z
// 1 = positive x, negative y, negative z
// 2 = negative x, positive y, negative z
// 3 = positive x, positive y, negative z
// 4 = negative x, negative y, positive z
// 5 = positive x, negative y, positive z
// 6 = negative x, positive y, positive z
// 7 = positive x, positive y, positive z

inline int extractSigns (const Vector3& v)
{
	const UINT32* src = (const UINT32*)(&v);
	UINT32 index = (src[0]>>31);		// if (v.x < 0.0f) index |= 1; else index = 0;
	index |= (src[1]&0x80000000)>>30;	// if (v.y < 0.0f) index |= 2;
	index |= (src[2]&0x80000000)>>29;	// if (v.z < 0.0f) index |= 4
	index ^= 7;							// flip all three bits using xor to reverse comparisons
	return (int)(index);				// return value
/*
	// here's corresponding floating point comparison code
	int index = 0;
	if (v.x >= 0.0f) index |= 1;
	if (v.y >= 0.0f) index |= 2;
	if (v.z >= 0.0f) index |= 4;
	return index;

*/
}

// transforms planes when inverse matrix 'm' has been computed. If you don't have the inverse matrix,
// then use the API call transformPlanes() (it will internally compute the inverse matrix).

DPVS_FORCE_INLINE void transformPlanesWithInverseMatrix (Vector4* dst, const Vector4* src, const Matrix4x3& n, int N)
{
	DPVS_ASSERT (dst && src && N>=0);

	for (int i = 0; i < N; i++)
	{
		float sx   = src[i].x;
		float sy   = src[i].y;
		float sz   = src[i].z;
		float sw   = src[i].w;

		float x		= sx * n[0][0] + sy * n[1][0] + sz * n[2][0];
		float y		= sx * n[0][1] + sy * n[1][1] + sz * n[2][1];
		float z		= sx * n[0][2] + sy * n[1][2] + sz * n[2][2];
		float len	= 1.0f / Math::sqrt(x*x+y*y+z*z);				
		float w		= len * (sw * (sx*sx+sy*sy+sz*sz) + sx * n[0][3] + sy * n[1][3] + sz * n[2][3]);

		dst[i].x = x * len;
		dst[i].y = y * len;
		dst[i].z = z * len;
		dst[i].w = w;			// already multiplied
	}
}

} // DPVS::Math
} // DPVS

//------------------------------------------------------------------------
#endif
