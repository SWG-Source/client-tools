//===================================================================
//
// MayaIndexedTriangleList.h
// asommers
//
// copyright 2001, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_MayaIndexedTriangleList_H
#define INCLUDED_MayaIndexedTriangleList_H

//===================================================================

#include "sharedMath/Vector.h"

class IndexedTriangleList;
class MDagPath;

//===================================================================

class MayaIndexedTriangleList
{
public:

	explicit MayaIndexedTriangleList (const MDagPath &meshDagPath, bool triangulatedOnly = false);
	~MayaIndexedTriangleList ();

	IndexedTriangleList* createIndexedTriangleList () const;
	const Vector& getScale () const;

private:

	MayaIndexedTriangleList ();
	MayaIndexedTriangleList (const MayaIndexedTriangleList&);
	MayaIndexedTriangleList& operator= (const MayaIndexedTriangleList&);

private:

	IndexedTriangleList* m_indexedTriangleList;
	Vector               m_scale;
};

//===================================================================

#endif

