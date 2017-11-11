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
 * Description: 	Intersection routines. The triangle-box intersection
 *					routine is a slightly modified version of a routine
 *					provided by Tomas Möller.
 *
 * $Archive: /dpvs/implementation/sources/dpvsIntersect.cpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 7.05.01 12:06 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#include "dpvsIntersect.hpp"
#include "dpvsArray.hpp"
#include "dpvsMesh.hpp"
#include "dpvsOBB.hpp"
#include "dpvsMath.hpp"

using namespace DPVS;

/*****************************************************************************
 *
 * Function:		DPVS::intersectOBBAABB()
 *
 * Description:		Determines whether an OBB intersects an AABB
 *
 * Parameters:		obb				= reference to OBB
 *					obbMtx			= object->cell transformation matrix
 *					aabb			= AABB in cell-space
 *
 * Returns:			true if OBB intersects AABB, false otherwise
 *
 *****************************************************************************/

bool DPVS::intersectOBBAABB (const OBB& obb, const Matrix4x3& obbMtx, const AABB& aabb)
{
	// We transform here the AABB into the OBB's space, then test whether the 
	// AABB vertices intersect the OBB.

	{
		Matrix4x3 obbToCell(NO_CONSTRUCTOR);
		Math::productFromLeft(obbToCell,obb.getMatrix(),obbMtx);	// object to cell-space matrix
		Math::invertMatrix(obbToCell);								// cell->obb matrix

		Vector3 mn(NO_CONSTRUCTOR);
		Vector3 mx(NO_CONSTRUCTOR);

		Math::transformAABBMinMax(mn,mx,aabb.getMin(),aabb.getMax(),obbToCell); // transform AABB into OBB space

		if (mx.x <= -1.0f ||	// If all vertices of the AABB are outside the OBB,
			mx.y <= -1.0f ||	// we know that the OBB and the AABB cannot overlap
			mx.z <= -1.0f ||
			mn.x >= +1.0f ||
			mn.y >= +1.0f ||
			mn.z >= +1.0f)
			return false;

	}
	
	// See if OBB is completely outside the AABB's planes
	{
		RawVector<Vector3,8> v;											// eight vertices of the OBB
		Math::transformOBBVertices (v, obb.getMatrix(), obbMtx);		// get vertices in cell-space
		Vector3 mn(NO_CONSTRUCTOR),mx(NO_CONSTRUCTOR);
		Math::minMax(mn,mx,v,8);

		if (mx.x <= aabb.getMin().x ||
			mx.y <= aabb.getMin().y ||
			mx.z <= aabb.getMin().z ||
			mn.x >= aabb.getMax().x ||
			mn.y >= aabb.getMax().y ||
			mn.z >= aabb.getMax().z)
			return false;
	}

	// OBB and AABB overlap
	return true;
}

/*****************************************************************************
 *
 * Function:		DPVS::intersectOBBFrustum()
 *
 * Description:		Determines whether an OBB intersects a view frustum
 *
 * Parameters:		obb				= reference to OBB
 *					objectToCell	= object->cell transformation matrix
 *					p				= array of clip planes (in cell-space)
 *					mask			= active plane mask
 *
 * Returns:			true if OBB intersects the frustum, false otherwise
 *
 * Notes:			The routine does not handle all cases; it can return
 *					'true' sometimes when the OBB doesn't really intersect
 *					the frustum.
 *
 *****************************************************************************/

bool DPVS::intersectOBBFrustum (const OBB& obb, const Matrix4x3& objectToCell, const Vector4* p, unsigned int mask)
{
	// DEBUG DEBUG TODO: HANDLE THE REMAINING DIFFICULT CASES?

	if (!mask)														// there's no active plane mask!
		return true;

	// transform OBB vertices into cell-space, then test
	// if all vertices are beyond any of the active planes
	RawVector<Vector3,8> v;											// eight vertices of the OBB
	Math::transformOBBVertices (v, obb.getMatrix(), objectToCell);	// get vertices in cell-space
	
	while (mask)
	{
		if (mask&1)
		{
			register float px = p->x, py = p->y, pz = p->z, pw = p->w;
			
			if ((v[0].x*px + v[0].y*py + v[0].z*pz + pw) <= 0.0f &&	// unrolled eight tests (w/ early exit)
				(v[1].x*px + v[1].y*py + v[1].z*pz + pw) <= 0.0f &&
				(v[2].x*px + v[2].y*py + v[2].z*pz + pw) <= 0.0f &&
				(v[3].x*px + v[3].y*py + v[3].z*pz + pw) <= 0.0f &&
				(v[4].x*px + v[4].y*py + v[4].z*pz + pw) <= 0.0f &&
				(v[5].x*px + v[5].y*py + v[5].z*pz + pw) <= 0.0f &&
				(v[6].x*px + v[6].y*py + v[6].z*pz + pw) <= 0.0f &&
				(v[7].x*px + v[7].y*py + v[7].z*pz + pw) <= 0.0f)
				return false;										// all 8 vertices outside
		}
		
		mask>>=1;
		p++;
	}

	return true;													// OBB intersects the frustum
}

//------------------------------------------------------------------------
