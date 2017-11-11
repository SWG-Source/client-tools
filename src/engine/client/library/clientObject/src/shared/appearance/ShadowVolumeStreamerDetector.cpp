// ======================================================================
//
// ShadowVolumeStreamerDetector.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "clientObject/FirstClientObject.h"
#include "clientObject/ShadowVolumeStreamerDetector.h"

#include "sharedMath/IndexedTriangleList.h"
#include "clientGraphics/StaticVertexBuffer.h"
#include "clientGraphics/StaticIndexBuffer.h"

#include <vector>

// ======================================================================

namespace ShadowVolumeStreamerDetectorNamespace
{
	int findOrAdd (std::vector<Vector> & vertices, Vector const & vector)
	{
		int const numberOfVertices = static_cast<int> (vertices.size ());
		for (int i = 0; i < numberOfVertices; ++i)
			if (vertices [i] == vector)
				return i;

		vertices.push_back (vector);

		return numberOfVertices;
	}
}

using namespace ShadowVolumeStreamerDetectorNamespace;

// ======================================================================
// PUBLIC ShadowVolumeStreamerDetector
// ======================================================================

ShadowVolumeStreamerDetector::ShadowVolumeStreamerDetector (char const * const debugName) :
	m_debugName (DuplicateString (debugName)),
	m_edgeList (new EdgeList),
	m_indexedTriangleList (new IndexedTriangleList)
{
}

// ----------------------------------------------------------------------

ShadowVolumeStreamerDetector::~ShadowVolumeStreamerDetector ()
{
	delete [] m_debugName;
	delete m_edgeList;
	delete m_indexedTriangleList;
}

// ----------------------------------------------------------------------

void ShadowVolumeStreamerDetector::addPrimitive (StaticVertexBuffer const & vertexBuffer, StaticIndexBuffer const & indexBuffer)
{
	IndexedTriangleList indexedTriangleList;

	{
		vertexBuffer.lockReadOnly ();

			VertexBufferReadIterator vi = vertexBuffer.beginReadOnly ();
			int const numberOfVertices = vertexBuffer.getNumberOfVertices ();

			std::vector<Vector> & vertices = indexedTriangleList.getVertices ();
			vertices.reserve (numberOfVertices);

			for (int i = 0; i < numberOfVertices; ++i, ++vi)
				vertices.push_back (vi.getPosition ());

		vertexBuffer.unlock ();
	}

	{
		indexBuffer.lockReadOnly ();

			Index const * ii = indexBuffer.beginReadOnly ();
			int const numberOfIndices = indexBuffer.getNumberOfIndices ();

			std::vector<int> & indices = indexedTriangleList.getIndices ();
			indices.reserve (numberOfIndices);

			for (int i = 0; i < numberOfIndices; ++i, ++ii)
				indices.push_back (*ii);
		
		indexBuffer.unlock ();
	}

	addPrimitive (indexedTriangleList);
}

// ----------------------------------------------------------------------

void ShadowVolumeStreamerDetector::addPrimitive (IndexedTriangleList const & indexedTriangleList)
{
	std::vector<Vector> const & sourceVertices = indexedTriangleList.getVertices ();
	
	std::vector<int> const & sourceIndices = indexedTriangleList.getIndices ();
	int const numberOfSourceIndices = static_cast<int> (sourceIndices.size ());

	std::vector<Vector> & destinationVertices = m_indexedTriangleList->getVertices ();
	std::vector<int> & destinationIndices = m_indexedTriangleList->getIndices ();

	for (int i = 0; i < numberOfSourceIndices; ++i)
	{
		int const index = findOrAdd (destinationVertices, sourceVertices [sourceIndices [i]]);
		destinationIndices.push_back (index);
	}
}

// ----------------------------------------------------------------------

void ShadowVolumeStreamerDetector::detectAndReport ()
{
	{
		std::vector<int> const & indices = m_indexedTriangleList->getIndices ();
		int const numberOfIndices = static_cast<int> (indices.size ());
		int const numberOfFaces = numberOfIndices / 3;
		for (int i = 0; i < numberOfFaces; ++i)
		{
			int const i0 = indices [3 * i + 0];
			int const i1 = indices [3 * i + 1];
			int const i2 = indices [3 * i + 2];

			addEdge (i0, i1);
			addEdge (i1, i2);
			addEdge (i2, i0);
		}
	}

	{
		int const m_numberOfEdges = static_cast<int> (m_edgeList->size ());
		for (int i = 0; i < m_numberOfEdges; ++i)
		{
			Edge const & edge = (*m_edgeList) [i];
			if (edge.m_numberOfFaces > 2)
			{
#ifdef _DEBUG
				Vector const & v0 = m_indexedTriangleList->getVertices () [edge.m_i0];
				Vector const & v1 = m_indexedTriangleList->getVertices () [edge.m_i1];
				DEBUG_REPORT_LOG (true, ("ShadowVolumeStreamerDetector [%s]: edge has %i faces and may cause streamer at v0=<%1.2f, %1.2f, %1.2f>, v1=<%1.2f, %1.2f, %1.2f>\n", m_debugName, edge.m_numberOfFaces, v0.x, v0.y, v0.z, v1.x, v1.y, v1.z));
#endif
			}
		}
	}
}

// ======================================================================
// PRIVATE ShadowVolumeStreamerDetector
// ======================================================================

void ShadowVolumeStreamerDetector::addEdge (int const i0, int const i1)
{
	if (i0 == i1)
	{
#ifdef _DEBUG
		Vector const & v0 = m_indexedTriangleList->getVertices () [i0];
		DEBUG_REPORT_LOG (true, ("ShadowVolumeStreamerDetector [%s]: found zero-length edge, v=<%1.2f, %1.2f, %1.2f>\n", m_debugName, v0.x, v0.y, v0.z));
#endif
		return;
	}

	int const numberOfEdges = static_cast<int> (m_edgeList->size ());
	int i = 0;
	for (i = 0; i < numberOfEdges; ++i)
	{
		if ((*m_edgeList) [i].m_i0 == i0 && (*m_edgeList) [i].m_i1 == i1)
			break;

		if ((*m_edgeList) [i].m_i0 == i1 && (*m_edgeList) [i].m_i1 == i0)
			break;
	}

	if (i == numberOfEdges)
	{
		Edge edge;
		edge.m_i0 = i0;
		edge.m_i1 = i1;
		edge.m_numberOfFaces = 0;
		m_edgeList->push_back (edge);
	}

	++(*m_edgeList) [i].m_numberOfFaces;
}

// ======================================================================
