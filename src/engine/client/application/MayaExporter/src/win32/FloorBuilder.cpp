//===================================================================
//
// FloorBuilder.cpp
//
// copyright 2001, sony online entertainment
//
//===================================================================

#include "FirstMayaExporter.h"
#include "FloorBuilder.h"

#include "maya/MDagPath.h"
#include "maya/MFloatPointArray.h"
#include "maya/MFnMesh.h"
#include "maya/MIntArray.h"

#include "sharedFile/Iff.h"
#include "sharedMath/Vector.h"
#include "sharedMath/Line3d.h"
#include "sharedCollision/FloorMesh.h"
#include "sharedCollision/FloorTri.h"
#include "sharedCollision/FloorLocator.h"
#include "sharedObject/Hardpoint.h"
#include "sharedPathfinding/PathEdge.h"
#include "sharedPathfinding/PathNode.h"
#include "sharedPathfinding/SimplePathGraph.h"

#include "ExporterLog.h"
#include "Messenger.h"
#include "MeshBuilder.h"

#include <vector>
#include <algorithm>

#include "maya/MSelectionList.h"
#include "maya/MGlobal.h"
#include "maya/MFnSet.h"
#include "maya/MItDependencyNodes.h"
#include "maya/MFnSingleIndexedComponent.h"
#include "maya/MItMeshVertex.h"
#include "maya/MItMeshEdge.h"

// If a mesh has this many triangles or more, we build a boxtree for it
const int gs_minTrianglesForBoxtree = 10;

#define MESSENGER_REJECT_NORETURN(a,b)                  \
	if (a)                                       \
	{                                            \
		messenger->enableIndent(false);            \
		messenger->enableFileAndLineDisplay(true); \
		messenger->setFileName(__FILE__);          \
		messenger->setLineNumber(__LINE__);        \
		messenger->logErrorMessage b;              \
	}

extern Messenger * messenger;

typedef std::vector<IntPair> IntPairVector;
typedef std::vector<bool> BoolVector;

// Find the angle between (A,B) and (A,C)

float angleBetween ( Vector const & A, Vector const & B, Vector const & C )
{
	Vector dA = (B-A);
	Vector dB = (C-A);

	IGNORE_RETURN(dA.normalize());
	IGNORE_RETURN(dB.normalize());
	
	float dot = dA.dot(dB);

	return acos(dot);
}

// ----------------------------------------------------------------------

bool	FloorBuilder::findSetByName	( const char * searchName, MFnSet & outFnSet )
{
	MStatus status;

	MItDependencyNodes  setIterator(MFn::kSet, &status);
	if(!status) return false;

	bool done = setIterator.isDone(&status);
	if(!status) return false;

	while (!done)
	{
		MObject object = setIterator.item(&status);
		if(!status) return false;

		MFnDependencyNode  fnDependencyNode(object,&status);
		if(!status) return false;

		MString nodeName = fnDependencyNode.name(&status);
		if(!status) return false;

		if(nodeName == MString(searchName))
		{
			status = outFnSet.setObject(object);
			if(!status) return false;

			return true;
		}

		status = setIterator.next();
		if(!status) return false;

		done = setIterator.isDone(&status);
		if(!status) return false;
	}

	return false;
}


// ----------
// Given a selection set, find the component for the object named objectName.

bool FloorBuilder::findSetComponentByDagNode( MFnSet & fnSet, MObject & dagNode, MObject & outComponent )
{
	MStatus status;

	MSelectionList  setMembers;
	status = fnSet.getMembers(setMembers, true);

	const unsigned int selectionCount = setMembers.length();
	for (unsigned int i = 0; i < selectionCount; ++i)
	{
		MDagPath  dagPath;
		MObject   component;

		status = setMembers.getDagPath(i, dagPath, component);

		// ----------
		
		MObject node = dagPath.node(&status);
		if(!status) continue;

		if(node != dagNode) continue;

		// ----------

		outComponent = component;
		return true;
	}

	return false;
}

bool	FloorBuilder::getSetIndicesForObject( const char * setName, MObject & dagNode, MIntArray & indices )
{
	indices.clear();

	// ----------

	MFnSet fnSet;
	bool found = findSetByName(setName,fnSet);
	if(!found) return false;

	// ----------

	MObject component;
	found = findSetComponentByDagNode(fnSet,dagNode,component);
	if(!found) return false;

	// ----------

	MStatus status;

	MFnSingleIndexedComponent  fnComponent(component,&status);
	if(!status) return false;

	status = fnComponent.getElements(indices);
	if(!status) return false;

	// ----------

	return true;
}

// ----------------------------------------------------------------------

bool	FloorBuilder::extractPoints	( MFnMesh & fnMesh, VectorVector & out )
{
	out.clear();

	MFloatPointArray vertices;

	MStatus status;
	status = fnMesh.getPoints (vertices, MSpace::kObject);
	
	MESSENGER_REJECT(!status,("FloorBuilder::extractPoints - Couldn't get vertices for floor mesh\n"));

	// ----------

	out.clear();

	const uint n = vertices.length ();
	uint i;
	for (i = 0; i < n; ++i)
	{
		const MFloatPoint& point = vertices [i];

		//-- fixup rh to lh
		out.push_back (Vector (-point.x, point.y, point.z));
	}

	return true;
}

// ----------------------------------------------------------------------

bool	FloorBuilder::extractIndices	( MFnMesh & fnMesh, IntVector & out )
{
	out.clear();

	MStatus status;
	const int n = fnMesh.numPolygons (&status);

	MESSENGER_REJECT(!status,("FloorBuilder::extractIndices - Couldn't get the number of polygons in the floor mesh\n"));

	// ----------

	out.clear();

	MIntArray intArray;

	for( int i = 0; i < n; i++ )
	{
		intArray.clear ();

		status = fnMesh.getPolygonVertices (i, intArray);
		MESSENGER_REJECT(!status,("FloorBuilder::extractIndices - Couldn't extract polygon vertices\n"));

		uint nIndices = intArray.length();

		MESSENGER_REJECT(nIndices < 3, ("FloorBuilder::extractFloor - Floor has a degenerate polygon ( <3 vertices )\n"));
		MESSENGER_REJECT(nIndices > 3, ("FloorBuilder::extractFloor - Floor isn't tesselated into triangles, things will break\n"));

		for (uint j = 2; j < nIndices; ++j)
		{
			//-- invert winding order
			out.push_back (intArray [0]);
			out.push_back (intArray [j]);
			out.push_back (intArray [j-1]);
		}
	}

	return true;
}

// ----------------------------------------------------------------------

bool    FloorBuilder::extractPolyVerts  ( MFnMesh & fnMesh, int whichPoly, VectorVector & out )
{
    MStatus status;

    MFloatPointArray vertices;
    MIntArray indices;

    status = fnMesh.getPoints(vertices, MSpace::kWorld);
    if(!status) return false;

    status = fnMesh.getPolygonVertices(whichPoly, indices);
    if(!status) return false;

    out.clear();

    // invert winding order

    for( int i = indices.length() - 1; i >= 0; i--)
    {
	    MFloatPoint const & v = vertices[indices[i]];

        //-- fixup rh to lh
	    out.push_back( Vector( -v.x, v.y, v.z ) );
    }

    return true;
}










//===================================================================

FloorBuilder::FloorBuilder( MayaHierarchy::Node const * pNode )
{
	m_pNode = pNode;
}

// ----------------------------------------------------------------------

std::string FloorBuilder::getFilename ( void )
{
	MayaHierarchy const * hierarchy = m_pNode->getHierarchy();

	const char* const directory = hierarchy->getCollisionWriteDir();

	NOT_NULL (directory);

	std::string writeName = directory;

	writeName += m_pNode->getName ();
	writeName += hierarchy->getExtension (m_pNode->getType ());
	
	return writeName;
}

// ----------------------------------------------------------------------

bool FloorBuilder::unpackTriIndices ( MFnMesh & fnMesh, MIntArray & triIndices, IntVector & outIndices )
{
	UNREF(fnMesh);

	for (uint i = 0; i < triIndices.length(); i++)
	{
		outIndices.push_back( triIndices[i] );
	}

	return true;
}

// ----------

bool FloorBuilder::unpackEdgeIndices ( MFnMesh & fnMesh, MIntArray & edgeIndices, IntVector & outIndices )
{
	MStatus status;

	for (uint i = 0; i < edgeIndices.length(); i++)
	{
		int whichEdge = edgeIndices[i];
		int2 vertIndices;

		status = fnMesh.getEdgeVertices(whichEdge,vertIndices);

		if(!status)
		{
			outIndices.clear();
			return false;
		}

		outIndices.push_back(vertIndices[0]);
		outIndices.push_back(vertIndices[1]);
	}

	return true;
}

// ----------------------------------------------------------------------

bool FloorBuilder::flagPortalEdges ( FloorMesh & floorMesh )
{
	MStatus status;

    const MayaHierarchy::Node * cellNode = NULL;
    const MayaHierarchy::Node * listNode = NULL;

    // Find the portal list that's under the same cell as this floor.

    if(!m_pNode->findParentByType(MayaHierarchy::T_cel, cellNode))
	{
		// This node is not a cell floor, since it has no cell parent. We can't flag portal edges for it.

		return true;
	}

    if(!cellNode->findChildByType(MayaHierarchy::T_pls, listNode))
	{
        MESSENGER_REJECT(true,("FloorBuilder::flagPortalEdges - Cell %s has no portal list\n",cellNode->getName()));
	}


    // For each portal in the list, extract the portal polygon and use it
    // to mark portal edges in the floor. Skip portal nodes with the "impassible"
    // attribute set.

    std::vector<Vector> portalVerts;

    floorMesh.clearPortalEdges();

	bool success = true;
    
	for(int i = 0; i < listNode->getNumberOfChildren(); i++)
	{
        MayaHierarchy::Node const * portalNode = listNode->getChild(i);

		MFnDagNode dagNode;
		if(!portalNode->getMayaDagNodeFn(dagNode)) 
		{
            MESSENGER_REJECT_NORETURN(true,("FloorBuilder::flagPortalEdges - Could not get dag node for portal %s\n",portalNode->getName()));
			success = false;
		}

		MFnMesh mesh(portalNode->getMayaDagPath(), &status);
		if(!status)
		{
            MESSENGER_REJECT_NORETURN(true,("FloorBuilder::flagPortalEdges - Could not get mesh node for portal %s\n",portalNode->getName()));
			success = false;
		}

		dagNode.attribute ("impassable", &status);
		if (status == MS::kSuccess) continue;

		bool matchedFloorToPortal = false;
		int const meshNumPolygons = mesh.numPolygons();
		for (int polyIndex = 0; polyIndex < meshNumPolygons; polyIndex++)
		{
			if(!extractPolyVerts(mesh,polyIndex,portalVerts)) 
			{
				MESSENGER_REJECT_NORETURN(true,("FloorBuilder::flagPortalEdges - Could not get portal poly %d for %s\n", polyIndex, portalNode->getName()));
				success = false;
			}

			//check for identical verts as they break flagPortalEdges
			int const numVerts = portalVerts.size();
			for(int vertIndex = 0; vertIndex < numVerts - 1; ++vertIndex)
			{
				for(int vertIndex2 = vertIndex +1; vertIndex2 < numVerts; ++vertIndex2)
				{
					if(portalVerts[vertIndex] == portalVerts[vertIndex2])
					{
						MESSENGER_REJECT_NORETURN(true,("FloorBuilder::flagPortalEdges - Identical verts found in portal poly %d for %s\n", polyIndex, portalNode->getName()));
						success = false;
					}
				}
			}

			if(floorMesh.flagPortalEdges(portalVerts,i))
			{
				matchedFloorToPortal = true;
				// don't break - keeping check all portal faces and flagging floor edges
			}
		}
		
		if (!matchedFloorToPortal)
		{
			MESSENGER_REJECT_NORETURN(true,("FloorBuilder::flagPortalEdges - Could not match floor edge to portal %s\n",portalNode->getName()));
			success = false;
		}
    }

    return success;
}

// ----------------------------------------------------------------------

bool FloorBuilder::extractFloor ( MObject nodeObject, FloorMesh & outMesh )
{
	outMesh.clear();
	
	// ----------

	MStatus status;

	MFnMesh fnMesh(nodeObject,&status);
	if (!status) return false;

	MDagPath path;
	status = MDagPath::getAPathTo(nodeObject,path);
	if(!status) return false;

	// ----------

	VectorVector	vertices;
	IntVector		indices;

	if(!extractPoints(fnMesh,vertices)) return false;
	if(!extractIndices(fnMesh,indices)) return false;

	// ----------

	MObject mayaTransform = path.transform (&status);
	if (status == MS::kFailure) return false;

	Transform  transform;

	if( !MayaHierarchy::MayaTransform_to_Transform(mayaTransform,transform) )
		return false;

	// ----------

	uint const verticesSize = vertices.size();
	for(uint i = 0; i < verticesSize; i++)
	{
		vertices[i] = transform.rotateTranslate_l2p(vertices[i]);
	}

	outMesh.build(vertices,indices);

	// ----------
	// If this floor has any edges marked as crossable, add those to the floor.

	{
		MIntArray edges;
		IntVector indices;

		getSetIndicesForObject("crossable",nodeObject,edges);

		unpackEdgeIndices(fnMesh,edges,indices);

		outMesh.addCrossableEdges(indices);
	}

	{
		MIntArray edges;
		IntVector indices;

		getSetIndicesForObject("ramp",nodeObject,edges);

		unpackEdgeIndices(fnMesh,edges,indices);

		outMesh.addRampEdges(indices);
	}

	// ----------
	// If this floor has any floortris marked as fallthrough, add those to the floor.

	{
		MIntArray tris;
		IntVector indices;

		getSetIndicesForObject("fallthrough",nodeObject,tris);

		unpackTriIndices(fnMesh,tris,indices);

		outMesh.addFallthroughTris(indices);
	}

	// ----------

	outMesh.compile();

	return true;
}

// ----------------------------------------------------------------------
// Create path nodes for all the artist-placed locators

// We're using the userId field of the path nodes to help speed up node
// connection (saves us a drop test)

PathNode createCellWaypointPathNode( FloorMesh & mesh, FloorLocator & loc )
{
	UNREF(mesh);

	PathNode node;

	node.setType( PNT_CellWaypoint );
	node.setPosition_p( loc.getPosition_p() );
	node.setRadius( loc.getRadius() );
	node.setUserId( loc.getId() );

	return node;
}

PathNode createCellPortalPathNode( FloorMesh & mesh, FloorLocator & loc, int key )
{
	UNREF(mesh);

	PathNode node;

	node.setType( PNT_CellPortal );
	node.setPosition_p( loc.getPosition_p() );
	node.setRadius( loc.getRadius() );
	node.setUserId( loc.getId() );
	node.setKey(key);

	return node;
}

bool FloorBuilder::createHardpointPathNodes( FloorMesh & mesh, HardpointVec & pathHardpoints, PathNodeList & nodes )
{
	bool addedHardpoint = false;

	for(uint iNode = 0; iNode < pathHardpoints.size(); iNode++)
	{
		Hardpoint * h = pathHardpoints[iNode];

		Vector pos = h->getTransform().getPosition_p();

		// Jitter the locations we get from the locators slightly so they don't land
		// exactly on floor edges

		Vector jitter(0.007f,0.0f,0.003f);

		pos += jitter;

		Line3d line(pos,-Vector::unitY);

		FloorLocator closestAbove;
		FloorLocator closestBelow;

		if( mesh.findClosestPair(line,-1,closestBelow,closestAbove) )
		{
			float distBelow = abs(closestBelow.getOffset());
			float distAbove = abs(closestAbove.getOffset());

			if(closestBelow.isAttached() && (distBelow < 3.0f))
			{
				addedHardpoint = true;

				closestBelow.setRadius(0.5f);

				nodes.push_back( createCellWaypointPathNode(mesh,closestBelow) );
			}
			else if(closestAbove.isAttached() && (distAbove < 1.0f))
			{
				addedHardpoint = true;

				closestAbove.setRadius(0.5f);

				nodes.push_back( createCellWaypointPathNode(mesh,closestAbove) );
			}
		}
	}

	return addedHardpoint;
}

// ----------------------------------------------------------------------
// Create path nodes for every portal-adjacent edge in the mesh

void FloorBuilder::createPortalPathNodes ( MayaHierarchy::Node const * node, FloorMesh & mesh, PathNodeList & nodes )
{
	int tricount = static_cast<int>(mesh.getTriCount());

	for(int i = 0; i < tricount; i++)
	{
		FloorTri const & F = mesh.getFloorTri(i);

		for(int j = 0; j < 3; j++)
		{
			int cellPortalIndex = F.getPortalId(j);

			if(cellPortalIndex != -1)
			{
				int buildingPortalIndex;

				if(MayaHierarchy::convertCellPortalIndexToBuildingPortalIndex(node,cellPortalIndex,buildingPortalIndex))
				{
					Vector A = mesh.getVertex(F.getCornerIndex(j));
					Vector B = mesh.getVertex(F.getCornerIndex(j+1));

					Vector D = B - A;

					// Don't create path nodes for degenerate edges in the floor

					if(D.magnitude() < 0.01f) 
					{
						continue;
					}

					Vector C = (B + A) / 2.0f;

					Vector jitter(0.007f,0.0f,0.003f);

					C += jitter;

					FloorLocator loc(&mesh,C,i,0.0f,0.0f);

					nodes.push_back( createCellPortalPathNode(mesh,loc,buildingPortalIndex) );
				}
			}
		}
	}
}

// ----------------------------------------------------------------------
// Do a brute-force o(n^2) search to try and connect every pair of 
// (non-portal) path nodes in the mesh.

void FloorBuilder::createInteriorPathEdges ( FloorMesh & mesh, PathNodeList & nodes, PathEdgeList & edges )
{
	int nodecount = nodes.size();

	for(int i = 0; i < nodecount - 1; i++)
	{
		if(nodes[i].getType() == PNT_CellPortal) continue;

		for(int j = i+1; j < nodecount; j++)
		{
			if(nodes[j].getType() == PNT_CellPortal) continue;

			FloorLocator A( &mesh, nodes[i].getPosition_p(), nodes[i].getUserId(), 0.0f, 0.0f );
			FloorLocator B( &mesh, nodes[j].getPosition_p(), nodes[j].getUserId(), 0.0f, 0.0f );

			if(mesh.testConnectable(A,B))
			{
				edges.push_back( PathEdge(i,j) );
				edges.push_back( PathEdge(j,i) );
			}
		}
	}
}

// ----------------------------------------------------------------------
// Connect each portal node to the closest connectable non-portal node

void FloorBuilder::createPortalPathEdges ( FloorMesh & mesh, PathNodeList & nodes, PathEdgeList & edges )
{
	int nodecount = nodes.size();

	for(int i = 0; i < nodecount; i++)
	{
		if(nodes[i].getType() != PNT_CellPortal) continue;

		int minId = -1;
		float minDist = REAL_MAX;

		for(int j = 0; j < nodecount; j++)
		{
			if(nodes[j].getType() == PNT_CellPortal) continue;

			FloorLocator A( &mesh, nodes[i].getPosition_p(), nodes[i].getUserId(), 0.0f, 0.0f );
			FloorLocator B( &mesh, nodes[j].getPosition_p(), nodes[j].getUserId(), 0.0f, 0.0f );

			if(mesh.testConnectable(A,B))
			{
				float dist = nodes[i].getPosition_p().magnitudeBetween( nodes[j].getPosition_p() );

				if(dist < minDist)
				{
					minDist = dist;
					minId = j;
				}
			}
		}

		if(minId != -1)
		{
			edges.push_back( PathEdge(i,minId) );
			edges.push_back( PathEdge(minId,i) );
		}
	}
}

// ----------------------------------------------------------------------

void FloorBuilder::markRedundantEdges ( PathNodeList & nodes, PathEdgeList & edges, BoolVector & flags )
{
	float angleTolerance = (20.0f / 360.0f) * (2.0f * PI);

	int nEdges = edges.size();

	for(int i = 0; i < nEdges - 1; i++)
	{
		for(int j = i+1; j < nEdges; j++)
		{
			PathEdge const & edgeA = edges[i];
			PathEdge const & edgeB = edges[j];

			int AA = edgeA.getIndexA();
			int AB = edgeA.getIndexB();
			int BA = edgeB.getIndexA();
			int BB = edgeB.getIndexB();

			// If the two edges go between the same nodes, don't prune either

			if((AA == BA) && (AB == BB)) continue;
			if((AA == BB) && (AB == BA)) continue;

			Vector A;
			Vector B;
			Vector C;

			// Find the three points we'll use to check the angle between
			// the two edges

			if(AA == BA)
			{
				A = nodes[AA].getPosition_p();
				B = nodes[AB].getPosition_p();

				C = nodes[BB].getPosition_p();
			}
			else if(AA == BB)
			{
				A = nodes[AA].getPosition_p();
				B = nodes[AB].getPosition_p();

				C = nodes[BA].getPosition_p();
			}
			else if(AB == BA)
			{
				A = nodes[AB].getPosition_p();
				B = nodes[AA].getPosition_p();

				C = nodes[BB].getPosition_p();
			}
			else if(AB == BB)
			{
				A = nodes[AB].getPosition_p();
				B = nodes[AA].getPosition_p();

				C = nodes[BA].getPosition_p();
			}
			else
			{
				// The edges don't share a vertex, so skip them.

				continue;
			}

			if(angleBetween(A,B,C) < angleTolerance)
			{
				float magAB = (B-A).magnitudeSquared();
				float magAC = (C-A).magnitudeSquared();

				if(magAB > magAC)
				{
					// The A-B edge is longer, so mark edgeA as redundant.

					flags[i] = true;
				}
				else
				{
					// The A-C edge is longer, so mark edgeB as redundant.

					flags[j] = true;
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

void FloorBuilder::prunePathEdges ( PathNodeList & nodes, PathEdgeList & edges )
{
	int edgeCount = edges.size();

	BoolVector flags(edgeCount,false);

	markRedundantEdges(nodes,edges,flags);

	PathEdgeList newEdges;

	for(int i = 0; i < edgeCount; i++)
	{
		if(!flags[i])
		{
			newEdges.push_back(edges[i]);
		}
	}

	edges.swap(newEdges);
}

// ----------------------------------------------------------------------

void FloorBuilder::buildPathGraph ( MayaHierarchy::Node const * node, FloorMesh & mesh )
{
	HardpointVec pathHardpoints;

	IGNORE_RETURN( MeshBuilder::collectHardpoints(0, pathHardpoints,"hp_path", false) );

	PathNodeList * pathNodes = new PathNodeList();
	PathEdgeList * pathEdges = new PathEdgeList();

	if(!pathHardpoints.empty())
	{
		IGNORE_RETURN( createHardpointPathNodes(mesh,pathHardpoints,*pathNodes) );
	}

	createPortalPathNodes(node,mesh,*pathNodes);

	if(pathNodes->size() == 0)
	{
		delete pathNodes;
		delete pathEdges;

		return;
	}

	for(uint i = 0; i < pathNodes->size(); i++)
	{
		pathNodes->at(i).setIndex(i);
	}

	createInteriorPathEdges(mesh,*pathNodes,*pathEdges);

	prunePathEdges(*pathNodes,*pathEdges);

	// Portal edges don't get pruned (it's OK if we have a few redundant ones)

	createPortalPathEdges(mesh,*pathNodes,*pathEdges);

	SimplePathGraph * newGraph = new SimplePathGraph( pathNodes, pathEdges );

	mesh.attachPathGraph(newGraph);
}

// ----------------------------------------------------------------------

bool FloorBuilder::writeFloor ( bool bPublish )
{
    MESSENGER_REJECT(!m_pNode,("FloorBuilder::writeFloor - No floor node specified\n"));

	Iff iff(1024);

	if(!writeFloor(iff))
	{
        MESSENGER_REJECT(true,("FloorBuilder::writeFloor - Couldn't write floor for node %s\n",m_pNode->getName()));
	}

	if(bPublish) 
	{
		std::string filename = getFilename();
		
		ExporterLog::addSharedDestinationFile(filename.c_str());

		iff.write (filename.c_str());
	}

	return true;
}

bool FloorBuilder::writeFloor ( Iff & iff )
{
    MESSENGER_REJECT(!m_pNode,("FloorBuilder::writeFloor - No floor node specified\n"));

	MStatus status;
	MDagPath path = m_pNode->getMayaDagPath();

	MObject nodeObject = path.node(&status);

	if(!status)
	{
        MESSENGER_REJECT(true,("FloorBuilder::writeFloor - Could not get MObject for node %s\n",m_pNode->getName()));
	}

    // FloorMesh is a DataResource, and has to be constructed with a string for a filename.
    // Since this is just a temporary, use the empty string as its filename.

	FloorMesh mesh("");

	if(!extractFloor(nodeObject,mesh))
	{
		return false;
	}
 
    if(!flagPortalEdges(mesh))
	{
		return false;
	}

	buildPathGraph(m_pNode,mesh);

	if(mesh.getTriCount() >= gs_minTrianglesForBoxtree)
	{
		mesh.buildBoxTree();
	}

	mesh.write(iff);

	// Hacky stuff - Find the average of the positions of the path nodes on the floor and
	// use that as the position of the cell path node in the building path graph

	{
		PathGraph const * graph = safe_cast<PathGraph const *>(mesh.getPathGraph());

		if(graph)
		{
			Vector accum = Vector::zero;

			int nodeCount = graph->getNodeCount();

			for(int i = 0; i < nodeCount; i++)
			{
				accum += graph->getNode(i)->getPosition_p();
			}

			accum /= (float)nodeCount;

			const MayaHierarchy::Node * cellNode = NULL;

			if(m_pNode->findParentByType(MayaHierarchy::T_cel, cellNode))
			{
				cellNode->setCellPathNodePosition(accum);
			}
		}
	}

	return true;
}

