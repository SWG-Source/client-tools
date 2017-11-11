#ifndef __DPVSBOUNDS_HPP
#define __DPVSBOUNDS_HPP
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
 * Desc:	Routines for constructing tight bounding volumes for meshes
 *
 * $Archive: /dpvs/implementation/include/dpvsBounds.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 7.05.01 12:10 $
 * $Date: 2003/01/22 $
 * 
 *****************************************************************************/

#if !defined (__DPVSPRIVATEDEFS_HPP)
#	include "dpvsPrivateDefs.hpp"
#endif

namespace DPVS
{
	class OBB;
	class AABB;
	class Sphere;

	void calculateOBB			(OBB& obb, const Vector3* vertices, int N);			// this in dpvsOBB.cpp
	void calculateOBB			(OBB& obb, const AABB& aabb);						// dpvsOBB.cpp
	
	void calculateAABB			(AABB& d, const Sphere& s);
	void calculateAABB			(AABB& d, const OBB& obb);
	void calculateAABB			(AABB& d, const Vector3* vertices, int N);
	void calculateAABB			(AABB& d, const Vector3* vertices, const Matrix4x3& mtx, int N);
	
	void calculateSphere		(Sphere& d, const AABB& s);
	void calculateSphere		(Sphere& d, const OBB& s);
	void calculateSphere		(Sphere& s, const Vector3* vertices, int N);
	void calculateSphereAndAABB (Sphere& s, AABB& box, const Vector3* vertices, int N);
	void calculateSphereAndAABB (Sphere& s, AABB& box, const Vector3* vertices, const Matrix4x3& mtx, int N);
} 

//------------------------------------------------------------------------
#endif // __DPVSBOUNDS_HPP

 
