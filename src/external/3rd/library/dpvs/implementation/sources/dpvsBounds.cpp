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
 * $Archive: /dpvs/implementation/sources/dpvsBounds.cpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 14.06.01 12:42 $
 * $Date: 2003/01/22 $
 * 
 *****************************************************************************/

#include "dpvsBounds.hpp"
#include "dpvsAABB.hpp"
#include "dpvsSphere.hpp"
#include "dpvsOBB.hpp"
#include "dpvsMatrix.hpp"

namespace DPVS
{

/*****************************************************************************
 *
 * Function:		DPVS::calculateAABB()
 *
 * Description:		Constructs an AABB from a sphere
 *
 * Parameters:		d = reference to output AABB
 *					s = reference to input AABB
 *
 *****************************************************************************/

void calculateAABB (AABB& d, const Sphere& s)
{
	float	r	= s.getRadius();
	Vector3	rr(r,r,r);
	d.setMin (s.getCenter() - rr);
	d.setMax (s.getCenter() + rr);
}

/*****************************************************************************
 *
 * Function:		DPVS::calculateAABB()
 *
 * Description:		Constructs an AABB from a set of vertices
 *
 * Parameters:		d			= reference to output AABB
 *					vertices	= input vertices
 *					N			= number of input vertices
 *
 *****************************************************************************/

void calculateAABB (AABB& d, const Vector3* vertices, int N)
{
	DPVS_ASSERT (N>0 && vertices);
	Vector3 mn(NO_CONSTRUCTOR);
	Vector3 mx(NO_CONSTRUCTOR);
	Math::minMax(mn,mx,vertices,N);			// use Math library minmax routine
	d.setMin(mn);
	d.setMax(mx);
}

/*****************************************************************************
 *
 * Function:		DPVS::calculateAABB()
 *
 * Description:		Constructs an AABB from a set of vertices that are transformed with specified matrix
 *
 * Parameters:		d			= reference to output AABB
 *					vertices	= input vertices
 *					mtx			= reference to 4x3 transformation matrix
 *					N			= number of input vertices
 *
 *****************************************************************************/

void calculateAABB (AABB& d, const Vector3* vertices, const Matrix4x3& mtx, int N)
{
	DPVS_ASSERT(vertices && N > 0);
	Vector3 mn(NO_CONSTRUCTOR);
	Vector3 mx(NO_CONSTRUCTOR);
	Math::minMaxTransform(mn,mx,vertices,mtx,N);
	d.setMin(mn);
	d.setMax(mx);
}

/*****************************************************************************
 *
 * Function:		DPVS::calculateSphere()
 *
 * Description:		Constructs a bounding sphere from an AABB
 *
 * Parameters:		d			= reference to output sphere
 *					s			= reference to input AABB
 *
 *****************************************************************************/

void calculateSphere (Sphere& d, const AABB& s)
{
	d.setCenter (s.getCenter());
	d.setRadius (s.getDiagonalLength()*0.5f);
}

/*****************************************************************************
 *
 * Function:		DPVS::calculateSphereAndAABB()
 *
 * Description:		Constructs a bounding sphere and an AABB from a set of vertices
 *
 * Parameters:		s			= reference to output sphere
 *					box			= reference to output AABB
 *					vertices	= input vertices
 *					N			= number of input vertices
 *
 *****************************************************************************/

void calculateSphereAndAABB (Sphere& s, AABB& box, const Vector3* vertices, int N)
{
	DPVS_ASSERT (N>0 && vertices);

	calculateAABB(box,vertices,N);						// start by calculating AABB

	Vector3 center(box.getCenter());
	
	s.setCenter (center);
	s.setRadius (1.000001f * Math::maxDist(center,vertices,N));

#if defined (DPVS_DEBUG)

	// here self-debugging test 
	for (int i = 0; i < N; i++)
	{
		float d = (vertices[i]-s.getCenter()).length();
		DPVS_ASSERT (d <= s.getRadius());
	}
#endif

}

/*****************************************************************************
 *
 * Function:		DPVS::calculateSphere()
 *
 * Description:		Constructs a bounding sphere from a set of vertices
 *
 * Parameters:		s			= reference to output sphere
 *					vertices	= input vertices
 *					N			= number of input vertices
 *
 *****************************************************************************/

void calculateSphere (Sphere& s, const Vector3* vertices, int N)
{
	AABB box;									// dummy box (as the ..andAABB routine needs a box)..
	calculateSphereAndAABB(s,box,vertices,N);
}

/*****************************************************************************
 *
 * Function:		DPVS::calculateSphereAndAABB()
 *
 * Description:		Constructs a bounding sphere and an AABB from a set of vertices that
 *					are transformed by specified matrix
 *
 * Parameters:		s			= reference to output sphere
 *					box			= reference to output AABB
 *					vertices	= input vertices
 *					mtx			= reference to input transformation matrix
 *					N			= number of input vertices
 *
 *****************************************************************************/

void calculateSphereAndAABB (Sphere& s, AABB& box, const Vector3* vertices, const Matrix4x3& mtx, int N)
{
	DPVS_ASSERT (N>0 && vertices);

	calculateAABB(box,vertices,mtx,N);			// calculate AABB

	Vector3	cen		= (box.getCenter());
	float   rad_sq	= 0.0f;

	for (int i = 0; i < N; i++)
	{
		Vector3 w = mtx.transform(vertices[i]);
		float   d = (w-cen).lengthSqr();		// squared distance to center
		if (d > rad_sq)
			rad_sq = d;
	}

	float rad = Math::sqrt(rad_sq) * 1.000001f;	// to dispel floating point inaccuracies..

	s.setRadius (rad);
	s.setCenter (cen);

#if defined (DPVS_DEBUG)

	// here self-debugging test 
	for (int i = 0; i < N; i++)
	{
		Vector3 w = mtx.transform(vertices[i]);
		float d = (w-s.getCenter()).length();
		DPVS_ASSERT (d <= s.getRadius());
	}
#endif
}

} // DPVS

//------------------------------------------------------------------------
