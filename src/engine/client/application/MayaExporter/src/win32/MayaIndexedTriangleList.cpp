//===================================================================
//
// MayaIndexedTriangleList.cpp
// asommers
//
// copyright 2001, sony online entertainment
//
//===================================================================

#include "FirstMayaExporter.h"
#include "MayaIndexedTriangleList.h"

#include "Messenger.h"

#include "maya/MDagPath.h"
#include "maya/MFloatPointArray.h"
#include "maya/MFnMesh.h"
#include "maya/MFnTransform.h"
#include "maya/MIntArray.h"
#include "maya/MMatrix.h"

#include "sharedMath/IndexedTriangleList.h"
#include "sharedMath/Vector.h"

#include <vector>

//===================================================================

static MStatus extractScale (const MObject& object, Vector& scale)
{
	MStatus status;
	MFnTransform transform (object, &status);
	if (status)
	{
		MMatrix matrix = transform.transformation (&status).asMatrix ();
		if (status)
		{
			scale.x = static_cast<float> (matrix [0][0]);
			scale.y = static_cast<float> (matrix [1][1]);
			scale.z = static_cast<float> (matrix [2][2]);
		}
	}

	return status;
}

//===================================================================

MayaIndexedTriangleList::MayaIndexedTriangleList (const MDagPath& meshDagPath, bool const triangulatedOnly) :
	m_indexedTriangleList (NON_NULL (new IndexedTriangleList ())),
	m_scale (Vector::xyz111)
{
	MStatus status;

	status = extractScale (meshDagPath.transform (&status), m_scale);
	if (!status)
		return;

	MFnMesh fnMesh (meshDagPath, &status);
	if (!status)
		return;

	m_indexedTriangleList->allowVertexMerging(true);
	m_indexedTriangleList->setVertexMergeEpsilon(0.01f);

	//-- get vertices
	std::vector<Vector> vertices;

	{
		MFloatPointArray pointArray;

		status = fnMesh.getPoints (pointArray, MSpace::kObject);
		if (!status)
			return;

		const uint n = pointArray.length ();
		uint i;
		for (i = 0; i < n; ++i)
		{
			const MFloatPoint& point = pointArray [i];

			//-- fixup rh to lh
			vertices.push_back (Vector (-point.x * m_scale.x, point.y * m_scale.y, point.z * m_scale.z));
		}
	}

	//-- get indices
	std::vector<int> indices;

	{
		const int n = fnMesh.numPolygons (&status);
		if (!status)
			return;

		MIntArray intArray;

		int i;
		for (i = 0; i < n; ++i)
		{
			intArray.clear ();

			status = fnMesh.getPolygonVertices (i, intArray);
			if (!status)
				return;

			//-- Only allow triangulated geometry
			if (triangulatedOnly && intArray.length() != 3)
				return;

			for (uint j = 2; j < intArray.length(); ++j)
			{
				//-- invert winding order
				indices.push_back (intArray [0]);
				indices.push_back (intArray [j]);
				indices.push_back (intArray [j-1]);
			}
		}
	}

	//-- 
	m_indexedTriangleList->addIndexedTriangleList (&vertices [0], static_cast<int> (vertices.size ()), &indices [0], static_cast<int> (indices.size ()));
}

//-------------------------------------------------------------------

MayaIndexedTriangleList::~MayaIndexedTriangleList ()
{
	delete m_indexedTriangleList;
	m_indexedTriangleList = 0;
}

//-------------------------------------------------------------------

IndexedTriangleList* MayaIndexedTriangleList::createIndexedTriangleList () const
{
	NOT_NULL (m_indexedTriangleList);

	return m_indexedTriangleList->clone ();
}

//-------------------------------------------------------------------

const Vector& MayaIndexedTriangleList::getScale () const
{
	return m_scale;
}

//===================================================================

