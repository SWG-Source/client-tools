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
 * Description:		SphereModel and OBBModel source code...
 *
 * $Archive: /dpvs/implementation/sources/dpvsImpMiscModel.cpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 8.01.02 16:45 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#include "dpvsImpMiscModel.hpp"
#include "dpvsAABB.hpp"
#include "dpvsSphere.hpp"
#include "dpvsBounds.hpp"

using namespace DPVS;

/*****************************************************************************
 *
 * Function:		ImpOBBModel::ImpOBBModel()
 *
 * Description:		Constructor from a 4x3 matrix
 *
 *****************************************************************************/

ImpOBBModel::ImpOBBModel(const Matrix4x3& m) 
{
	OBB		obb;
	Sphere	sphere;

	obb.setMatrix	(m);
	calculateSphere	(sphere,obb);

	setOBB			(obb,true);
	setSphere		(sphere);
}

/*****************************************************************************
 *
 * Function:		ImpOBBModel::ImpOBBModel()
 *
 * Description:		Constructor from a set of vertices
 *
 *****************************************************************************/

ImpOBBModel::ImpOBBModel(const Vector3* vertices, int numVertices)
{
	OBB		obb;
	Sphere	sphere;

	if (vertices && numVertices > 0)					// just checking
	{
		calculateOBB	(obb, vertices, numVertices);
		calculateSphere	(sphere,obb);
	}

	setOBB			(obb, true);
	setSphere		(sphere);
}

/*****************************************************************************
 *
 * Function:		ImpOBBModel::~ImpOBBModel()
 *
 * Description:		Destructor
 *
 *****************************************************************************/

ImpOBBModel::~ImpOBBModel(void)
{
	// nada
}

/*****************************************************************************
 *
 * Function:		ImpSphereModel::ImpSphereModel()
 *
 * Description:		Constructor
 *
 *****************************************************************************/

ImpSphereModel::ImpSphereModel(const Vector3& center, float r)
{
	Sphere	sphere;

	r = Math::fabs(r);				// handle negative radii

	sphere.setCenter (center);
	sphere.setRadius (r);

	AABB	aabb;
	aabb.setMin(Vector3(center.x-r,center.y-r,center.z-r));
	aabb.setMax(Vector3(center.x+r,center.y+r,center.z+r));
	aabb.validateBounds();

	OBB		obb;
	calculateOBB (obb,aabb);
	
	setSphere (sphere);
	setOBB	  (obb, true);
}

/*****************************************************************************
 *
 * Function:		ImpSphereModel::~ImpSphereModel()
 *
 * Description:		Destructor
 *
 *****************************************************************************/

ImpSphereModel::~ImpSphereModel	(void)	
{
	// nada
}

//=============================================================================


