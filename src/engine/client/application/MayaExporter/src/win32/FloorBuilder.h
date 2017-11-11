//===================================================================
//
// FloorBuilder.h
// aappleby
//
// copyright 2001, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_FloorBuilder_H
#define INCLUDED_FloorBuilder_H

#include "MayaHierarchy.h"

#include <string>
#include <utility>

//===================================================================

class IndexedTriangleList;
class MDagPath;
class Iff;
class MFnSet;
class MObject;
class MIntArray;
class MFnMesh;
class FloorMesh;
class Hardpoint;
class PathNode;
class PathEdge;

typedef std::pair<int,int> IntPair;
typedef stdvector<int>::fwd IntVector;
typedef stdvector<Vector>::fwd VectorVector;
typedef stdvector<IntPair>::fwd IntPairVector;
typedef stdvector<bool>::fwd BoolVector;
typedef stdvector<Hardpoint*>::fwd HardpointVec;

//===================================================================

class FloorBuilder
{
public:
	
	explicit FloorBuilder( MayaHierarchy::Node const * pNode );
	
	bool            writeFloor                  ( bool bPublish );
	
	// these are used by meshbuilder too
	
	static bool     extractPoints               ( MFnMesh & fnMesh, VectorVector & out );
	static bool     extractIndices              ( MFnMesh & fnMesh, IntVector & out );
	
protected:
	
	typedef stdvector<PathEdge>::fwd PathEdgeList;
	typedef stdvector<PathNode>::fwd PathNodeList;

	std::string     getFilename                 ( void );
	bool            writeFloor                  ( Iff & iff );
	bool            flagPortalEdges             ( FloorMesh & floorMesh );
	
	static bool     findSetByName               ( const char * searchName, MFnSet & outFnSet );
	static bool     findSetComponentByDagNode   ( MFnSet & fnSet, MObject & dagNode, MObject & outComponent );
	static bool     getSetIndicesForObject      ( const char * setName, MObject & dagNode, MIntArray & indices );
	static bool     extractPolyVerts            ( MFnMesh & fnMesh, int whichPoly, VectorVector & out );
	static bool     unpackTriIndices            ( MFnMesh & fnMesh, MIntArray & triIndices, IntVector & outIndices );
	static bool     unpackEdgeIndices           ( MFnMesh & fnMesh, MIntArray & edgeIndices, IntVector & outIndices );
	static bool     extractFloor                ( MObject nodeObject, FloorMesh & outMesh );

	static bool     createHardpointPathNodes    ( FloorMesh & mesh, HardpointVec & pathHardpoints, PathNodeList & nodes );
	static void     createPortalPathNodes       ( MayaHierarchy::Node const * node, FloorMesh & mesh, PathNodeList & nodes );
	static void     createInteriorPathEdges     ( FloorMesh & mesh, PathNodeList & nodes, PathEdgeList & edges );
	static void     createPortalPathEdges       ( FloorMesh & mesh, PathNodeList & nodes, PathEdgeList & edges );
	static void     prunePathEdges              ( PathNodeList & nodes, PathEdgeList & edges );
	static void     markRedundantEdges          ( PathNodeList & nodes, PathEdgeList & edges, BoolVector & flags );

	static void     buildPathGraph              ( MayaHierarchy::Node const * node, FloorMesh & mesh );
	
	// ----------
	
	MayaHierarchy::Node const * m_pNode;
	
private:

	FloorBuilder();
};

// ----------------------------------------------------------------------


//===================================================================

#endif

