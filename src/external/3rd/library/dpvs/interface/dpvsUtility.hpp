#ifndef __DPVSUTILITY_HPP
#define __DPVSUTILITY_HPP
/*****************************************************************************
 *
 * dPVS
 * -----------------------------------------
 *
 * (C) 1999-2004 Hybrid Graphics, Ltd.
 * All Rights Reserved.
 *
 * Dynamic PVS and dPVS are trademarks of Criterion Software Ltd.
 *
 * This file consists of unpublished, proprietary source code of
 * Hybrid Graphics, and is considered Confidential Information for
 * purposes of non-disclosure agreement. Disclosure outside the terms
 * outlined in signed agreement may result in irrepairable harm to
 * Hybrid Graphics and legal action against the party in breach.
 *
 * Description:     Several auxiliary functions and classes for making
 *					the use of dPVS easier
 *
 * $Id: //depot/products/dpvs/interface/dpvsUtility.hpp#5 $
 * $Date: 2004/11/12 $
 * $Author: wili $
 * 
 ******************************************************************************/

#if !defined (__DPVSDEFS_HPP)
#   include "dpvsDefs.hpp"
#endif

namespace DPVS
{

class MeshModel;
/*****************************************************************************
 *
 * Class:			DPVS::ObjectSplitter
 *
 * Description:		Class for splitting a triangle soup into a number of
 *					"objects"
 *
 * Notes:			The splitter never clips triangles. A separate tesselator
 *					should be run externally to partition very large triangles
 *					(large relative to scene size) to enchance the results
 *					of the object splitter.
 *
 *					The splitter expects to receive the "entire scene" as
 *					its input - do not pass in individual models to it.
 *
 *****************************************************************************/

/******************************************************************************
 *
 * Function:		DPVS::ObjectSplitter::split()
 *
 * Description:		Splits a scene into "objects"
 *
 * Parameters:		objectIDs		= output object ID array
 *					vertices		= input vertex positions
 *					triangles		= input triangle vertex indices
 *					triMaterials	= input triangle materials (may be NULL)
 *					numVertices		= number of vertices
 *					numTriangles	= number of triangles
 *
 * Returns:			number of objects the input object was split into
 *
 * Notes:			The output array will contain object IDs for each triangle.
 *					The object IDs are in range [0,numObjects[, where numObjects
 *					is the return value of the function.
 *
 *					The 'triMaterials' array is optional (i.e. NULL may be
 *					submitted). If a triangle material array is provided, arbitrary
 *					UINT32 'material tags' can be used.
 *
 ******************************************************************************/

class ObjectSplitter
{
public:
	static DPVSDEC int		split				(UINT32 objectIDs[], const DPVS::Vector3* vertices, const DPVS::Vector3i* triangles, const UINT32* triMaterials, int numVertices,int numTriangles, float minObjectSize=0.0f);
	static DPVSDEC int		splitEx				(UINT32 objectIDs[], const Vector3* vertices, const Vector3i* triangles, const UINT32* triMaterials, int numVertices,int numTriangles, const Vector3& sceneScale, float minObjectSize=0.0f, int targetObjects = 0);
	static DPVSDEC int		splitEx				(MeshModel** models, const Vector3* vertices, const Vector3i* triangles, const UINT32* triMaterials, int numVertices,int numTriangles, bool clockwise, const Vector3& sceneScale, float minObjectSize=0.0f, int targetObjects = 0);
private:
							ObjectSplitter		(void);											// no constructor exists
							ObjectSplitter		(const ObjectSplitter&);						// no constructor exists
};

} // DPVS
                                            
//------------------------------------------------------------------------
#endif // __DPVSUTILITY_HPP
