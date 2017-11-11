// ======================================================================
//
// ShadowVolumeStreamerDetector.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_ShadowVolumeStreamerDetector_H
#define INCLUDED_ShadowVolumeStreamerDetector_H

// ======================================================================

class IndexedTriangleList;
class StaticIndexBuffer;
class StaticVertexBuffer;
class Vector;

// ======================================================================

class ShadowVolumeStreamerDetector
{
public:

	explicit ShadowVolumeStreamerDetector (char const * debugName);
	~ShadowVolumeStreamerDetector ();

	void addPrimitive (StaticVertexBuffer const & vertexBuffer, StaticIndexBuffer const & indexBuffer);
	void addPrimitive (IndexedTriangleList const & indexedTriangleList);

	void detectAndReport ();

private:

	struct Edge
	{
	public:

		int m_i0;
		int m_i1;

		int m_numberOfFaces;
	};

private:

	ShadowVolumeStreamerDetector ();
	ShadowVolumeStreamerDetector (ShadowVolumeStreamerDetector const & rhs);
	ShadowVolumeStreamerDetector & operator= (ShadowVolumeStreamerDetector const & rhs);

	void addEdge (int i0, int i1);

private:

	char * const m_debugName;

	typedef stdvector<Edge>::fwd EdgeList;
	EdgeList * const m_edgeList;

	IndexedTriangleList * const m_indexedTriangleList;
};

// ======================================================================

#endif
