#ifndef __DPVSINTERSECT_HPP
#define __DPVSINTERSECT_HPP
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
 *
 * Desc:	Intersection testing code for basic bounding volumes
 *
 * $Archive: /dpvs/implementation/include/dpvsIntersect.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 13.02.02 12:56 $
 * $Date: 2003/01/22 $
 * 
 *****************************************************************************/

#if !defined(__DPVSMATH_HPP)
#	include "dpvsMath.hpp"
#endif

#if !defined(__DPVSAABB_HPP)
#	include "dpvsAABB.hpp"
#endif

#if !defined(__DPVSSPHERE_HPP)
#	include "dpvsSphere.hpp"
#endif

namespace DPVS
{
class Mesh;
class OBB;

//--------------------------------------------------------------------
// Intersection function prototypes
//--------------------------------------------------------------------

bool intersectOBBAABB					(const OBB& obb, const Matrix4x3& obbMtx, const AABB& aabb);
bool intersectOBBFrustum				(const OBB& obb, const Matrix4x3& mtx, const Vector4*, unsigned int mask);
bool intersectHullPlane					(const Vector3*, int, const Vector4&, const Matrix4x3&);
bool intersectFrustum					(const Vector3&, const Vector4*, unsigned int);
bool intersect							(const Sphere&, const Vector3&);
bool intersect							(const Sphere&, const Sphere&);
bool intersect							(const AABB& a, const AABB& b);
bool intersect							(const AABB& a, const Vector3& point);
bool intersect							(const AABB& a, const Vector3* points, unsigned int pointMask);
bool intersectPointUnitBox				(const Vector3& v);

//--------------------------------------------------------------------
// Inline implementation
//--------------------------------------------------------------------

// test if a single vertex 'v' is inside frustum defined by planes in 'p' (mask indicates
// which planes are active). Assumes that vertex and planes are in the same space.
DPVS_FORCE_INLINE bool intersectFrustum (const Vector3& v, const Vector4* p, unsigned int mask)
{
	DPVS_ASSERT(p);
	while (mask)
	{
		if ((mask&1) && ((v.x*p->x + v.y*p->y + v.z*p->z + p->w) <= 0.0f))	
			return false;
		mask>>=1;
		p++;
	}
	return true;													
}


// Routine for determining if any of the vertices in 'v' are 'inside' the plane 'p' (early
// exits as soon as even one vertex is inside). The obToWorld matrix is used to transform the vertices 
// in 'v' to world space.

DPVS_FORCE_INLINE bool intersectHullPlane (const Vector3* v, int N, const Vector4& p, const Matrix4x3& obToWorld)
{
	float base = -(p.x * obToWorld[0][3] + p.y * obToWorld[1][3] + p.z * obToWorld[2][3] + p.w);
	for (int i = 0; i < N; i++)
	if ((p.x * (v[i].x * obToWorld[0][0] + v[i].y * obToWorld[0][1] + v[i].z * obToWorld[0][2]) +
		 p.y * (v[i].x * obToWorld[1][0] + v[i].y * obToWorld[1][1] + v[i].z * obToWorld[1][2]) +
		 p.z * (v[i].x * obToWorld[2][0] + v[i].y * obToWorld[2][1] + v[i].z * obToWorld[2][2])) > base)
		 return true;
	return false;
}

// Test if sphere intersects any planes defined by 'frustumMask'
// Update frustumMask as well...

/*
DPVS_FORCE_INLINE bool intersectSphereFrustum (const Sphere& s, const Vector4* p, unsigned int& frustumMask)
{
	DPVS_ASSERT(p);

	#define ISECT(P) { float d = (p[(P)].x*x)+(p[(P)].y*y)+(p[(P)].z*z)+(p[(P)].w); if (d < r) { if (d <= -r) return false;	frustumMask |= (1<<(P)); }}									

	unsigned long mask = (unsigned long)frustumMask;		// input mask
	frustumMask = 0;						// clear output mask

	const float		r		= s.getRadius();
	const float&	x		= s.getCenter().x;
	const float&	y		= s.getCenter().y;
	const float&	z		= s.getCenter().z;
	int				plane	= 0;

	while (mask)
	{
		if (mask&1)							// if plane is active, perform intersection test
			ISECT(plane);					
		mask>>=1;							// remove one bit from the mask
		plane++;							// increase plane
	}

	return true;							// sphere is visible 
	
	#undef ISECT
}
*/

/*
#if defined (DPVS_GAMECUBE_ASSEMBLY)
//NGC-BEGIN
DPVS_FORCE_INLINE float floatSelect(register float s, register float a, register float b)
{
	register float retVal;
	asm
	{
		fsel	retVal,s,a,b
	}
	return retVal;
}
//NGC-END
#endif

*/

DPVS_FORCE_INLINE bool intersect (const AABB& a, const AABB& b)
{
#if defined (DPVS_GAMECUBE_ASSEMBLY) && defined (not_defined)
//NGC-BEGIN
	// DEBUG DEBUG GameCube - not properly tested!!
	float retVal		= -1.0f;		// true (in sign bit) by default
	float floatFalse	= 0.0f;			// false (in sign bit)

	retVal = floatSelect(a.getMax().x - b.getMin().x, retVal, floatFalse);
	retVal = floatSelect(a.getMin().x - b.getMax().x, retVal, floatFalse);
	retVal = floatSelect(a.getMax().y - b.getMin().y, retVal, floatFalse);
	retVal = floatSelect(a.getMin().y - b.getMax().y, retVal, floatFalse);
	retVal = floatSelect(a.getMax().z - b.getMin().z, retVal, floatFalse);
	retVal = floatSelect(a.getMin().z - b.getMax().z, retVal, floatFalse);

	return Math::bitPattern(retVal) >> 31;
//NGC-END
#else
	if (a.getMax().x < b.getMin().x) return false;
	if (a.getMin().x > b.getMax().x) return false;
	if (a.getMax().y < b.getMin().y) return false;
	if (a.getMin().y > b.getMax().y) return false;
	if (a.getMax().z < b.getMin().z) return false;
	if (a.getMin().z > b.getMax().z) return false;
	return true;
#endif
}

DPVS_FORCE_INLINE bool intersect (const AABB& a, const Vector3& pt)
{
	return a.contains(pt);
}

// Intersection of AABB and a frustum. The frustum may contain 0-32 planes (active planes are defined
// by inClipMask). Returns boolean value indicating whether AABB intersects the view frustum or not.
// If AABB intersects the frustum, an output clip mask is returned as well (indicating which
// planes are crossed by the AABB). This information can be used to optimize testing of
// child nodes or objects inside the nodes (pass value as 'inClipMask').

// DEBUG DEBUG TODO: outClipMask aliases pretty much with everything...
DPVS_FORCE_INLINE bool intersectAABBFrustum (const AABB& a, const Vector4* p, unsigned int& outClipMask, unsigned int inClipMask)
{
	DPVS_ASSERT(p);
	DPVS_CHECK_ALIGN(p);
	Vector3	m(a.getCenter());	// center of AABB
	Vector3	d(a.getMax() - m);	// half-diagonal
	unsigned int mk	= 1;				
	outClipMask		= 0;				// init outclip mask
	while (mk <= inClipMask){
		if (inClipMask&mk){				// if clip plane is active...
			float NP = d.x*Math::fabs(p->x)+d.y*Math::fabs(p->y)+d.z*Math::fabs(p->z);		
			float MP = m.x*p->x + m.y*p->y + m.z*p->z + p->w;						
			if ((MP+NP) < 0.0f) return false;	// behind clip plane									
			if ((MP-NP) < 0.0f) outClipMask |= mk;										
		}
		mk+=mk;							// mk = (1<<iter)
		p++;							// next plane
	}
	return true;						// AABB intersects frustum
}

// tests if at least one point is inside the AABB. Routine complements the 
// AABB+plane list intersection routine.
DPVS_FORCE_INLINE bool	intersect (const AABB& a, const Vector3* p, int N)
{
	DPVS_ASSERT (p && N>=0);
	for (int i = 0; i < N; i++)
	{
		if (p[i].x >= a.getMin().x && p[i].x <= a.getMax().x &&
			p[i].y >= a.getMin().y && p[i].y <= a.getMax().y &&
			p[i].z >= a.getMin().z && p[i].z <= a.getMax().z)
			return true;
	}
	return false;
}

DPVS_FORCE_INLINE bool intersect (const Sphere& a, const Sphere& b)
{
	const Vector3& A = a.getCenter();
	const Vector3& B = b.getCenter();

	float x = A.x-B.x;
	float y = A.y-B.y;
	float z = A.z-B.z;

	float R = a.getRadius()+b.getRadius();

	return ((x*x+y*y+z*z) < (R*R));	// distance < radius^2
}

DPVS_FORCE_INLINE bool intersect (const Sphere& a, const Vector3& pt)
{
	return ( (a.getCenter()-pt).lengthSqr() < Math::sqr(a.getRadius()) );
}

// returns true if point is inside the axis-aligned box formed by coordinates (-1,-1,-1) and (+1,+1,+1)
DPVS_FORCE_INLINE bool intersectPointUnitBox	(const Vector3& v)
{
	const UINT32* p = (const UINT32*)&v;
	return ((p[0]&0x7fffffff)<=0x3f800000 &&			// 0x3f800000 is bit pattern for 1.0
		    (p[1]&0x7fffffff)<=0x3f800000 &&
			(p[2]&0x7fffffff)<=0x3f800000);
}

} // DPVS

//------------------------------------------------------------------------
#endif // __DPVSINTERSECT_HPP
