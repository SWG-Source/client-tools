// ======================================================================
//
// MayaLightMeshReader.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "FirstMayaExporter.h"
#include "MayaLightMeshReader.h"

#include "maya/MFnMesh.h"
#include "maya/MIntArray.h"
#include "maya/MItMeshPolygon.h"
#include "Messenger.h"

// ======================================================================

namespace
{
	Messenger *messenger;
}

// ======================================================================

void MayaLightMeshReader::install(Messenger *newMessenger)
{
	messenger = newMessenger;
}

// ----------------------------------------------------------------------

void MayaLightMeshReader::remove()
{
	messenger = 0;
}

// ----------------------------------------------------------------------

bool MayaLightMeshReader::enumerateTriangles(const MFnMesh &mesh, void *context, TriangleCallback callback)
{
	NOT_NULL(callback);

	MStatus  status;
	int      polyVertexIndex[3];

	//-- Retrieve the polygon to shader mapping.
	MObjectArray  shaderGroupArray;
	MIntArray     shaderPolyMapping;

	// NOTE: we do not support more than a single instance of each mesh being used.
	const int     instanceIndex = 0;

	status = mesh.getConnectedShaders(instanceIndex, shaderGroupArray, shaderPolyMapping);
	STATUS_REJECT(status, "fnMesh.getConnectedShaders() failed");

	//-- Retrieve a polygon iterator for the mesh.
	MItMeshPolygon m_polygonIterator(mesh.object(), &status);
	STATUS_REJECT(status, "enumerateTriangles(): MItMeshPolygon construction error");

	bool isDone = m_polygonIterator.isDone(&status);
	STATUS_REJECT(status, "m_polygonIterator.isDone() failed");

	//-- Loop over all polygons.
	int polyIndex = 0;

	while (!isDone)
	{
		//-- Get polygon data.
		const int polyVertexCount = static_cast<int>(m_polygonIterator.polygonVertexCount(&status));
		STATUS_REJECT(status, "m_polygonIterator.polygonVertexCount() failed");

		const int shaderIndex = shaderPolyMapping[static_cast<unsigned int>(polyIndex)];

		// Maya puts in CCW-is-visible culling order.  We need to put in CW order, so flip order of v1 and v2.
		// Fan the triangles --- note this is not always valid.  We want artists to triangulate meshes
		// before the exporter takes a crack at the data.
		polyVertexIndex[0] = 0;

		for (int lastIndex = 2; lastIndex < polyVertexCount; ++lastIndex)
		{
			// Setup fan, pivoting around face-relative poly index 0.
			polyVertexIndex[1] = lastIndex;
			polyVertexIndex[2] = lastIndex - 1;

			const bool callbackResult = (*callback)(context, shaderIndex, polyIndex, polyVertexIndex);
			MESSENGER_REJECT(!callbackResult, ("enumerateTriangles(): callback failed.\n"));
		}

		//-- Increment loop.
		status = m_polygonIterator.next();
		STATUS_REJECT(status, "m_polygonIterator.next() failed");

		isDone = m_polygonIterator.isDone(&status);
		STATUS_REJECT(status, "m_polygonIterator.isDone() failed");

		++polyIndex;
	}
	
	//-- Indicate success.
	return true;
}

// ======================================================================
