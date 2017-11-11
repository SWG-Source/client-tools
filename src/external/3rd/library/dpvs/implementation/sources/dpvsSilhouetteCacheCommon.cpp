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
 * Description: 	Silhouette cache common code shared by old and new
 *					caches
 *
 * $Archive: /dpvs/implementation/sources/dpvsSilhouetteCacheCommon.cpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 8.01.02 14:16 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#include "dpvsSilhouetteCache.hpp"
#include "dpvsHash.hpp"
#include "dpvsMath.hpp"
#include "dpvsAABB.hpp"
#include "dpvsBounds.hpp"
#include "dpvsRecycler.hpp"
#include "dpvsStatistics.hpp"
#include "dpvsTempArray.hpp"
#include "dpvsWeldHash.hpp"
#include "dpvsSort.hpp"

//#include "dpvsSystem.hpp" // DEBUG DEBUG DEBUG
#include <cstdio>// DEBUG DEBUG DEBUG

using namespace DPVS;

SilhouetteCache*			SilhouetteCache::s_silhouetteCache						= null;
int							SilhouetteCache::Client::Derived::s_derivedMemoryUsed	= 0;
int							SilhouetteCache::Client::Derived::s_numDerived			= 0;
SilhouetteCache::Client*	SilhouetteCache::Client::s_activeHead					= null;
SilhouetteCache::Client*	SilhouetteCache::Client::s_activeTail					= null;

// Structure used for sorting the triangles based on access order.. 
struct SortTri : public Vector3i
{
	DPVS_FORCE_INLINE bool operator> (const SortTri& s) const	
	{ 
		if (i > s.i) return true;
		if (i < s.i) return false;
		if (j > s.j) return true;
		if (j < s.j) return false;
		return (k > s.k);
	}
	DPVS_FORCE_INLINE bool operator< (const SortTri& s) const	
	{ 
		if (i < s.i) return true;
		if (i > s.i) return false;
		if (j < s.j) return true;
		if (j > s.j) return false;
		return (k < s.k);
	}
};
// structure used for sorting the edges based on plane access order
struct SortEdge : public SilhouetteCache::Client::Edge
{
private:
	DPVS_FORCE_INLINE UINT32 getSortValue (void) const
	{
		UINT32 p0 = (UINT32)(m_plane[0]);
		UINT32 p1 = (UINT32)(m_plane[1]);
		if (p0 < p1)
			swap(p0,p1);
		return (p0<<16)|p1;					// make 32-bit value
	}
public:
	DPVS_FORCE_INLINE bool operator> (const SortEdge& s) const { return getSortValue() > s.getSortValue(); }
	DPVS_FORCE_INLINE bool operator< (const SortEdge& s)  const { return getSortValue() < s.getSortValue();  }
};

/*****************************************************************************
 *
 * Function:		SilhouetteCache::init()
 *
 * Description:		
 *	
 *****************************************************************************/

void SilhouetteCache::init (void)
{
	DPVS_ASSERT(!s_silhouetteCache);
	s_silhouetteCache = NEW<SilhouetteCache>();
}

/*****************************************************************************
 *
 * Function:		SilhouetteCache::exit()
 *
 * Description:		
 *	
 *****************************************************************************/

void SilhouetteCache::exit (void)
{
	DELETE(s_silhouetteCache);
	s_silhouetteCache = null;
}

/*****************************************************************************
 *
 * Function:		SilhouetteCache::Client::setActive()
 *
 * Description:		Marks a client to be active (and sets it to the head of
 *					the active list)
 *	
 *****************************************************************************/

void SilhouetteCache::Client::setActive	(void) 
{
	// If client is already in the active list, we must remove it first..
	if (m_active)		
	{
		if (s_activeHead == this)						// if already head of active list..
			return;										// .. we're done...

		if (m_activePrev) 
			m_activePrev->m_activeNext = m_activeNext;
		else
		{
			DPVS_ASSERT(s_activeHead == this);
			s_activeHead = m_activeNext;
		}

		if (m_activeNext)
			m_activeNext->m_activePrev = m_activePrev;
		else
		{
			DPVS_ASSERT(s_activeTail == this);
			s_activeTail = m_activePrev;
		}
	} 

	// now link to head of the active list...

	m_active     = true;
	m_activePrev = null;
	m_activeNext = s_activeHead;
	
	if (m_activeNext)
		m_activeNext->m_activePrev = this;
	else
	{
		DPVS_ASSERT(!s_activeHead && !s_activeTail);
		s_activeTail = this;
	}

	s_activeHead = this;
}

/*****************************************************************************
 *
 * Function:		SilhouetteCache::Client::setInactive()
 *
 * Description:		Removes a client from the active list
 *	
 *****************************************************************************/

void SilhouetteCache::Client::setInactive (void)
{
	if (m_active)		// .. if in the active list
	{
		if (m_activePrev) 
			m_activePrev->m_activeNext = m_activeNext;
		else
		{
			DPVS_ASSERT(s_activeHead == this);
			s_activeHead = m_activeNext;
		}

		if (m_activeNext)
			m_activeNext->m_activePrev = m_activePrev; 
		else 
		{
			DPVS_ASSERT(s_activeTail == this);
			s_activeTail = m_activePrev;
		}

		m_active		= false;
		m_activePrev = null;
		m_activeNext = null;
	} else
	{
		DPVS_ASSERT(!m_activePrev);
		DPVS_ASSERT(!m_activeNext);		// WASS?
	}
}

/*****************************************************************************
 *
 * Function:		SilhouetteCache::Client::Derived::Derived()
 *
 * Description:		Constructor for derived arrays
 *	
 *****************************************************************************/

SilhouetteCache::Client::Derived::Derived (void)
{
	m_numSilhouetteEdges	= 0;
	m_numPlanes				= 0;
	m_plEq					= null;
	m_edges					= null;
	m_firstNode				= null;
	m_trianglePlanes        = null;
	m_alloc					= null;		// data allocation
}

/*****************************************************************************
 *
 * Function:		SilhouetteCache::Client::Derived::~Derived()
 *
 * Description:		Destructor for derived arrays
 *	
 *****************************************************************************/

SilhouetteCache::Client::Derived::~Derived (void)
{
	DPVS_ASSERT(!m_firstNode);
	DELETE_ARRAY(m_alloc);		// this frees the plane equations, edges and plane indices..
}

/*****************************************************************************
 *
 * Function:		SilhouetteCache::Client::releaseDerived()
 *
 * Description:		Releases the derived structure
 *
 *****************************************************************************/

void SilhouetteCache::Client::releaseDerived (void)
{
	if (m_derived)
	{
		SilhouetteCache::get()->deleteClientNodes(this);

		Derived::s_derivedMemoryUsed -= getDerivedMemoryUsed();
		Derived::s_numDerived--;
		DPVS_ASSERT(Derived::s_derivedMemoryUsed	>= 0);
		DPVS_ASSERT(Derived::s_numDerived			>= 0);

		DPVS_PROFILE(Statistics::setStatistic(Library::STAT_MODELDERIVEDMEMORYUSED,Derived::s_derivedMemoryUsed));
		DPVS_PROFILE(Statistics::setStatistic(Library::STAT_MODELDERIVED,	Derived::s_numDerived));

		DELETE(m_derived);
		m_derived	= null;
	}
}

/*****************************************************************************
 *
 * Function:		SilhouetteCache::setupDerived()
 *
 * Description:		Internal routine for setting up the "derived" structure for a client
 *
 * Notes:			The 'derived' structure contains plane equations for
 *					triangles, edges that can be silhouette edges etc.
 *
 * Optimize:		- store "counter info" (# of plEqs, #of edges) when
 *					  this function is called for the first time. This way we
 *				      can optimize subsequent calls considerably.
 *					- use the "planarity" bit calculated earlier
 *					- somehow avoid V4 constructors?
 *					- perform all allocations together?
 *
 *****************************************************************************/

void SilhouetteCache::setupDerived (const Client* c)
{
	//--------------------------------------------------------------------
	// Mark as active..
	//--------------------------------------------------------------------

	const_cast<Client*>(c)->setActive();	// sorry for the cast.. but a man's gotta do what a man's gotta do...
	
	//--------------------------------------------------------------------
	// First check if we _can_ create a derived structure for the data
	// specified. If we cannot, return immediately.
	//--------------------------------------------------------------------

	if (c->getTriangleCount() > Client::MAX_TRIANGLES || c->getVertexCount() > Client::MAX_VERTICES)
		return;

	//--------------------------------------------------------------------
	// Allocate a Derived structure. We _know_ now that the triangle and
	// vertex counts are bounded, so we can compress the Derived data
	// somewhat.
	//--------------------------------------------------------------------

	DPVS_ASSERT (!c->m_derived);
	DPVS_PROFILE(Statistics::incStatistic(Library::STAT_MODELTOPOLOGYCOMPUTED,1));

	int								numTriangles	= c->getTriangleCount();
	const Vector3i*					triangles		= c->getTriangleVertices();
	const Vector3*					vertices		= c->getVertices();
	int								numPlEq			= 0;				// number of unique plane equations
	int								edgeCount		= 0;				// number of edges	
	TempArray<Vector4,false>		tmpPlEq(numTriangles);				// temporary plane equations	
	TempArray<UINT16,false>			tmpTrianglePlanes(numTriangles);	// temporary triangle plane indices
	TempArray<Client::Edge,false>	tmpEdges(numTriangles*3);			// temporary edge array
	TempArray<Vector3i,false>		tmpTriEdges(numTriangles);			// temporary triangle edge indices

	// DEBUG DEBUG TODO OPTIMIZATIONS (STORE THIS DATA INTO THE CLIENT STRUCTURE):
	// 1) IF M_PLANAR, THEN JUST COPY FIRST PLANE EQUATION
	// 2) USE ONE BIT TO INDICATE IF ANY EDGES CAN BE REMOVED (i.e. planar edges)
	// 3) USE ONE BIT TO INDICATE IF ANY PLANE EQUATIONS CAN BE REMOVED 

	//--------------------------------------------------------------------
	// If the "optimize planarity" bit is set to the mesh, we attempt
	// to find shared plane equations. If none (or too few) were found,
	// the bit is disabled so that subsequent setupDerived() calls for the
	// same mesh can avoid unnecessary hashing.
	//--------------------------------------------------------------------

	if (c->m_optimizePlanarity)	// routine that attempts to optimize the plEqs using a hash
	{
		m_plEqWeldHash.reset (&tmpPlEq[0],numTriangles);					// reset plane equation hash
		for (int i = 0; i < numTriangles; i++)
		{
			Vector4 plEq(Math::getNormalizedPlaneEquation(vertices[triangles[i].i],vertices[triangles[i].j],vertices[triangles[i].k]));
			tmpTrianglePlanes[i] = (UINT16)(m_plEqWeldHash.getIndex(plEq));
		}
		numPlEq = m_plEqWeldHash.getOffset();

		if (numPlEq == numTriangles)								// optimization failed, so turn off the optimize bit..
		{
			const_cast<Client*>(c)->m_optimizePlanarity = false;	// ..to avoid future work..
			const_cast<Client*>(c)->m_optimizeEdges     = false;	// edges cannot be co-planar either...
		}

	} else	// routine that just computes the plEqs without wleding
	{
		numPlEq = numTriangles;
		for (int i = 0; i < numTriangles; i++)
		{
			tmpTrianglePlanes[i]	= (UINT16)(i);
			tmpPlEq[i]				= Math::getNormalizedPlaneEquation(vertices[triangles[i].i],vertices[triangles[i].j],vertices[triangles[i].k]);
		}
	}

	//--------------------------------------------------------------------
	// Now find all shared edges. We allow sharing of an edge if
	// a) the winding is different for the two sharing triangles (i.e. other one has A-B, other one B-A)
	// b) the shared edge has not been already shared, i.e. if three
	//    triangles share an edge, we won't accept the last share.
	//
	// These rules are very important as the silhouette rasterization
	// would fail if triple-shared edges were allowed.
	//--------------------------------------------------------------------

	{

		if (!m_edgeHash)
			m_edgeHash = NEW< Hash<Vector2i,int> >();

		for (int i = 0; i < numTriangles; i++)
		for (int j = 0; j < 3; j++)
		{
			int a = triangles[i][j];									// first vertex
			int b = (j==2) ? triangles[i][0] : triangles[i][j+1];		// second vertex

			if (m_edgeHash->exists(Vector2i(b,a)))
			{
				tmpTriEdges[i][j] = m_edgeHash->get(Vector2i(b,a));		// B-A edge
				m_edgeHash->remove(Vector2i(b,a));						// don't all triple-sharing etc...
			} else														// new edge
			{
				m_edgeHash->insert(Vector2i(a,b),edgeCount);			// insert as A-B
				tmpTriEdges[i][j] = edgeCount;
				tmpEdges[edgeCount].m_vertex[0] = (UINT16)a;
				tmpEdges[edgeCount].m_vertex[1] = (UINT16)b;
				tmpEdges[edgeCount].m_plane[0]  = Client::Edge::NOT_CONNECTED;
				tmpEdges[edgeCount].m_plane[1]  = Client::Edge::NOT_CONNECTED;
				edgeCount++;
			}
		}

		m_edgeHash->removeAll();										// remove but don't release memory

	}

	//----------------------------------------------------------------
	// At this point we know which edges are used by each triangle... 
	// time to remove all "planar" edges, then remap triangles 
	// correspondingly. This optimization is done only if the
	// "optimizeEdges" bit has been set.
	//----------------------------------------------------------------

	int outEdgeCount = edgeCount;

	if (c->m_optimizeEdges)
	{
		outEdgeCount = 0;

		for (int i = 0; i < numTriangles; i++)							// temporary stuff..
		for (int j = 0; j < 3; j++)
		{
			int edge = tmpTriEdges[i][j];								
			if (tmpEdges[edge].m_plane[0] == Client::Edge::NOT_CONNECTED)
				tmpEdges[edge].m_plane[0] = (UINT16)i;					// first triangle
			else	
				tmpEdges[edge].m_plane[1] = (UINT16)i;					// second triangle
		}

		for (int i = 0; i < edgeCount; i++)
		{
			const Client::Edge& e = tmpEdges[i];						// reference to input edge

			int t0 = e.m_plane[0];										// first triangle
			int t1 = e.m_plane[1];										// second triangle (may be Edge::NOT_CONNECTED)

			DPVS_ASSERT(t0 != Client::Edge::NOT_CONNECTED);									

			bool planar = false;

			//----------------------------------------------------------------
			// If the edge is connected to two triangles, we test if the
			// edge can be classified as "planar" (and moved to the very end
			// of the edge list). This classification saves run-time costs
			// when objects have many planar surfaces.
			//----------------------------------------------------------------

			if (t1 != Client::Edge::NOT_CONNECTED)										// if not a "single plane" edge
			{
				planar = true;

				int t0Plane = tmpTrianglePlanes[t0];
				int t1Plane = tmpTrianglePlanes[t1];

				if (t0Plane != t1Plane)													// if not same plane equation...
				{
					const Vector4&	A	= tmpPlEq[t0Plane];								// get plane equations for the triangles
					const Vector4&	B	= tmpPlEq[t1Plane];

					if ((A.x*B.x + A.y*B.y+A.z*B.z) >= 0.99999f)						// facing test..
					{
						float	edgeLength	= (vertices[e.m_vertex[0]] - vertices[e.m_vertex[1]]).length();		// length of the edge
						float	threshold	= edgeLength * 0.000001f;									// our comparison threshold

						for (int j = 0; j < 3; j++)
						{
							const Vector3& p = vertices[triangles[t1][j]];				// get vertex from second triangle
							const Vector3& q = vertices[triangles[t0][j]];				// get vertex from first triangle
							if ( Math::fabs(A.x*p.x + A.y*p.y +A.z*p.z+ A.w)>threshold ||	// too far away from plane?
								 Math::fabs(B.x*q.x + B.y*q.y +B.z*q.z+ B.w)>threshold)	
							{
								planar = false;
								break;
							}
						}
					} else
						planar = false;										// dot products differ too much
				}
			}

			if (planar)														// discard edge...
				continue;

			//----------------------------------------------------------------
			// Copy edge into output structure
			//----------------------------------------------------------------

			Client::Edge& o = tmpEdges[outEdgeCount++];
			o.m_vertex[0] = e.m_vertex[0];									// copy vertices
			o.m_vertex[1] = e.m_vertex[1];									// copy vertices
			o.m_plane[0]  = tmpTrianglePlanes[t0];							// direct to plane rather than triangle
			o.m_plane[1]  = (t1==Client::Edge::NOT_CONNECTED) ? 
								(UINT16)Client::Edge::NOT_CONNECTED : 
								tmpTrianglePlanes[t1];						// direct to plane rather than triangle
		}

		//----------------------------------------------------------------
		// If the optimization failed, store this information into the
		// "optimizeEdges" bit of the client. This way we don't attempt
		// the same optimization when the setupDerived() routine is called
		// again for the same mesh. DEBUG DEBUG TODO: here we could have
		// some ratio (say 95%??) instead of an absolute value.
		//----------------------------------------------------------------

		if (outEdgeCount == edgeCount)
			const_cast<Client*>(c)->m_optimizeEdges     = false;	
	} else // non-optimizing version just sets up the edge plane indices correctly.
	{
		for (int i = 0; i < numTriangles; i++)							// temporary stuff..
		{
			UINT16 plane = tmpTrianglePlanes[i];						// plane equation index of the triangle

			for (int j = 0; j < 3; j++)
			{
				int edge = tmpTriEdges[i][j];								
				DPVS_ASSERT(edge>=0 && edge < outEdgeCount);
				if (tmpEdges[edge].m_plane[0] == Client::Edge::NOT_CONNECTED)
					tmpEdges[edge].m_plane[0] = plane;					// first plane?
				else	
					tmpEdges[edge].m_plane[1] = plane;					// second plane
			}
		}
	}

	//----------------------------------------------------------------
	// Optimize the edge access order by sorting the edge array based
	// on the plane equation access order. The sorting rules ensures
	// that all double-connected edges come first, then the single-
	// connected edges. Inside these two groups the edges are sorted
	// to maximize coherence in plEq data accesses.
	//----------------------------------------------------------------

	quickSort (reinterpret_cast<SortEdge*>(&tmpEdges[0]), outEdgeCount);

	//--------------------------------------------------------------------
	// Allocate the Derived data and memory for the arrays. Then copy
	// the temporary arrays there. We allocate all three arrays using
	// only a single memory alloc -> this improves cache coherence later
	// and minimizes allocation overhead.
	//--------------------------------------------------------------------

	c->m_derived = NEW<Client::Derived>();									// allocate the derived structure
	
	c->m_derived->m_alloc = NEW_ARRAY<char>(								// we know that all allocations are 16-byte aligned..
		numPlEq*sizeof(Vector4) + 
		outEdgeCount*sizeof(Client::Edge) +
		numTriangles*sizeof(UINT16));

	c->m_derived->m_numPlanes			= numPlEq;							// number of planes..
	c->m_derived->m_numSilhouetteEdges	= outEdgeCount;						// just the silhouette edges

	// we know c->m_derived->m_alloc is at least 16 byte aligned, so alloc the vector4s
	// here first to maintain the alignment...
	c->m_derived->m_plEq				= reinterpret_cast<Vector4*>(c->m_derived->m_alloc);
	c->m_derived->m_edges				= reinterpret_cast<Client::Edge*>(c->m_derived->m_plEq + numPlEq);
	c->m_derived->m_trianglePlanes		= reinterpret_cast<UINT16*>(c->m_derived->m_edges + outEdgeCount);

	::memcpy (c->m_derived->m_plEq,				&tmpPlEq[0], numPlEq * sizeof(Vector4));
	::memcpy (c->m_derived->m_edges,			&tmpEdges[0],outEdgeCount*sizeof(Client::Edge));
	::memcpy (c->m_derived->m_trianglePlanes,	&tmpTrianglePlanes[0], numTriangles*sizeof(UINT16));

	//--------------------------------------------------------------------
	// Update statistics
	//--------------------------------------------------------------------

	Client::Derived::s_derivedMemoryUsed += c->getDerivedMemoryUsed();				
	Client::Derived::s_numDerived++;

	DPVS_PROFILE(Statistics::setStatistic(Library::STAT_MODELDERIVEDMEMORYUSED,Client::Derived::s_derivedMemoryUsed));
	DPVS_PROFILE(Statistics::setStatistic(Library::STAT_MODELDERIVED,Client::Derived::s_numDerived));

	//--------------------------------------------------------------------
	// In debug build cross-validate our data structures
	//--------------------------------------------------------------------

#if defined (DPVS_DEBUG)

	for (int i = 0; i < c->m_derived->m_numSilhouetteEdges; i++)
	{
		Client::Edge& e = c->m_derived->m_edges[i];

		DPVS_ASSERT(e.m_vertex[0] < c->getVertexCount());
		DPVS_ASSERT(e.m_vertex[1] < c->getVertexCount());
		DPVS_ASSERT(e.m_plane[0]  < c->m_derived->m_numPlanes);
		DPVS_ASSERT(e.m_plane[1]  < c->m_derived->m_numPlanes ||
			        e.m_plane[1] == Client::Edge::NOT_CONNECTED);
	}

	for (int i = 0; i < numTriangles; i++)
	{
		int plane = c->m_derived->m_trianglePlanes[i];
		DPVS_ASSERT (plane >= 0 && plane < c->m_derived->m_numPlanes);				// wasss?
	}
#endif // DPVS_DEBUG

	//--------------------------------------------------------------------
	// Move mesh into compressed state as we don't need the triangle indices
	// for a while..
	//--------------------------------------------------------------------

	c->m_mesh.compress();
}

/*****************************************************************************
 *
 * Function:		SilhouetteCache::Client::deleteMesh()
 *
 * Description:		Frees mesh data and deletes all cached silhouettes of
 *					the mesh
 *
 *****************************************************************************/

void SilhouetteCache::Client::deleteMesh (void)						
{ 
	releaseDerived();
	m_mesh.set(0,0,null,null);
	m_planar	= false;
}

/*****************************************************************************
 *
 * Function:		SilhouetteCache::Client::Client()
 *
 * Description:		Constructor
 *	
 *****************************************************************************/

SilhouetteCache::Client::Client	(void)	:
	m_mesh(),
	m_derived(null),
	m_activePrev(null),
	m_activeNext(null),
	m_planar(false),
	m_active(false),
	m_optimizePlanarity(true),
	m_optimizeEdges(true)
{ 
	// nada
}

/*****************************************************************************
 *
 * Function:		SilhouetteCache::Client::~Client()
 *
 * Description:		Destructor
 *
 *****************************************************************************/

SilhouetteCache::Client::~Client	(void)						
{ 
	deleteMesh();
	setInactive();		// mark client as inactive
}

/*****************************************************************************
 *
 * Function:		SilhouetteCache::Client::testPlanarity()
 *
 * Description:		Finds out whether the current mesh is planar or not
 *
 * Returns:			true if the mesh is planar, false otherwise
 *
 * Notes:			This function is called only by setMesh() and the return
 *					value is then cached to the m_planar member variable.
 *
 *****************************************************************************/

bool SilhouetteCache::Client::testPlanarity	(void) const
{
	int	triCount = getTriangleCount();

	if (triCount <= 1)				// zero or one triangle meshes are always planar...
		return true;

	DPVS_ASSERT(getVertexCount()>0);

	//----------------------------------------------------------------
	// Check if all vertices are on the same plane..
	//----------------------------------------------------------------

	const Vector3i*	ti			= getTriangleVertices();
	const Vector3*	vl			= getVertices();

	Vector4			plEq		= Math::getNormalizedPlaneEquation(vl[ti[0].i],vl[ti[0].j],vl[ti[0].k]);
	int				vertexCount	= getVertexCount();
	float			threshold   = (vl[ti[0].i]-vl[ti[0].j]).length() * 0.00001f;

	for (int i = vertexCount-1; i >= 0; --i)
	{
		float dist = Math::fabs(vl[i].x * plEq.x + vl[i].y * plEq.y + vl[i].z * plEq.z + plEq.w);
		if (dist > threshold)	// too much variation...
			return false;
	}

	//----------------------------------------------------------------
	// Then make sure that all the triangle normals have the same
	// facing... Otherwise two-sided meshes might be incorrectly
	// classified as planar..
	//----------------------------------------------------------------

	Vector3	normal = cross(vl[ti[0].j]-vl[ti[0].i], vl[ti[0].k]-vl[ti[0].i]).normalize();	// normal of the first vertex
	for(int i = triCount-1; i > 0; --i)
	{
		Vector3 n = cross(vl[ti[i].j]-vl[ti[i].i], vl[ti[i].k]-vl[ti[i].i]).normalize();
		if(dot(n,normal) < 0.9999f)	// too much variation...														// too much variation to be classified as co-planar...
			return false;
	}

	return true;			// yes, the mesh is planar...
}

/*****************************************************************************
 *
 * Function:		SilhouetteCache::Client::setMesh()
 *
 * Description:		Creates internal mesh structures for specified triangle/
 *					vertex set.
 *
 * Parameters:		vertices	= vertex locations in object space
 *					nVertices	= number of vertices	
 *					triangles	= triangle vertex indices
 *					nTriangles  = number of triangles
 *
 * Notes:			The function welds the input mesh and takes a copy of
 *					the welded mesh data. The original arrays can be deleted
 *					after this call. The derived data (edge tables etc.) are not 
 *					allocated yet -- they are alloced/computed only when the object 
 *					is used as an occluder for the first time. This reduces
 *					startup costs considerably.
 *
 *****************************************************************************/

void SilhouetteCache::Client::setMesh (const Vector3* loc, int nVertices, const Vector3i* tris, int nTriangles)
{
	deleteMesh();																		// delete existing mesh..
	if (!loc || !tris)																	// no input data??
		return;
	SilhouetteCache::get()->weldMesh(m_mesh,loc,nVertices,tris,nTriangles);				// copy welded mesh
	m_planar = testPlanarity();															// test planarity
	m_mesh.compress();																	// compress the mesh data..
}

/*****************************************************************************
 *
 * Function:		SilhouetteCache::SilhouetteCache()
 *
 * Description:		Constructor
 *
 *****************************************************************************/

SilhouetteCache::SilhouetteCache (void) :
	m_nodeMemoryUsed(0),
	m_maxCacheSizeInBytes(0),
	m_nodeHead(null),
	m_nodeTail(null),
	m_nodeCount(0),
	m_nodeTimeStamp(0),
	m_nodeVertexCacheSize(0),
	m_nodeVertexCacheTimeStamp(0),
	m_edgeHash(null)
{
	// nada (everything handled in initializer lists)
}

/*****************************************************************************
 *
 * Function:		SilhouetteCache::~SilhouetteCache()
 *
 * Description:		Destructor
 *
 *****************************************************************************/

SilhouetteCache::~SilhouetteCache (void)
{
	invalidateCache();
	DELETE(m_edgeHash);

	DPVS_ASSERT ("Some MeshModels are leaking!" && !Client::s_activeTail && !Client::s_activeHead);

	while (Client::s_activeHead)
		Client::s_activeHead->setInactive();	// .. just make sure..
}


#if 0
void testos (int nVertices,int nTris, const Vector3* vertices, const Vector3i* tris)
{
	static int nv = 0;
	static int nb = 0;
	static int rep = 0;
	static int old = 0;
	static int total  = 0;

	int bits = 0;

	const float* f = (const float*)(vertices);

	const int BUF_SIZE = 16;
	float buf[BUF_SIZE];
	int bufIndex=0;
	for (int i = 0; i < BUF_SIZE; i++)
	{
		buf[i] = i;							// initialize to _something_ ..
	}

	for (int j = 0; j < 3; j++)
	{
		if (j == 2)
			continue;

		float delta = 0.0f;

		for (int i = 0; i < nVertices; i++)
		{
			nv += 4;											// four bytes (uncompressed)
			total++;

	/*		bits++;
			if (i > 0 && vertices[i][j] == vertices[i-1][j])	// repetition...
			{
				rep++;
				continue;
			}

			bits++;	// either look-back buffer or a new vertex
	*/
			for (int k = 0; k < BUF_SIZE; k++)
			{
				if (vertices[i][j] == buf[k])
				{
					bits+=4;
					goto next;
				}
			}

			buf[(bufIndex++)%BUF_SIZE] = vertices[i][j];
			bits+=32;
	next:;
		}

	}

	nb += (bits+7)/8;						// compressed bytes

	printf ("in = %d out = %d comp = %.3f\n",nv,nb,(float)(nb)/(nv));
	printf ("old = %.3f%%\n",(100.0f*old)/(total));
	printf ("rep = %.3f%%\n",(100.0f*rep)/(total));
	

//	printf ("size = %d bytes\n",nVertices*sizeof(Vector3) + nTris * sizeof(Vector3i));
/*
	// first attempt to compress the indices

	Array<int> remap(nVertices);

	for (int i = 0; i < nVertices; i++)
		remap[i] = -1;

	int outIndex = 0;
	int bits     = 0;
	int ixBits   = 0;

	const int LOOKBACK_SIZE = 8;
	int lookBack[LOOKBACK_SIZE];
	int lookIndex = 0;
	for (int i = 0; i < LOOKBACK_SIZE; i++)
		lookBack[i] = -1;

	static int ol = 0;
	static int on = 0;
	static int oi = 0;
	static int of = 0;
	static int ov = 0;

	for (int i = 0; i < nTris; i++)
	{
		int newLook[3];
		int newLooks = 0;

		for (int j = 0; j < 3; j++)
		{
			ov++;
			int index = tris[i][j];					// vertex index

			bits+=2;

			for (int j = 0; j < LOOKBACK_SIZE; j++)
			{
				if (lookBack[j] == index)
				{
					bits += 2;
					ol++;
					goto done;
				}
			}

			newLook[newLooks++] = index;

			if (remap[index]==-1)				// not yet specified...
			{
				remap[index] = outIndex++;		// new remap value
				on++;
				continue;						// we're done...
			}

			oi++;
			// variable-size indexing..
			bits   += getHighestSetBit(outIndex)+1;		// woo!

		done:;
		}

		// update lookback buffer
		for (int j = 0; j < newLooks; j++)
		{
			lookBack[(lookIndex++)&(LOOKBACK_SIZE-1)] = newLook[j];
		}

	}

	int inSize  = nTris*sizeof(Vector3i);
	int outSize = (bits+7)/8;


	static int is = 0;
	static int os = 0;
	static int ot = 0;

	is += inSize;
	os += outSize;	// should be outSize!!
	ot += nTris;

	printf ("in = %d, out = %d (out%% = %.3f) bitspertri = %.3f\n",is,os,(100.0f*os)/is,(float)(os*8)/ot);
	printf ("lookback = %.3f%% new = %.3f%% fixed = %.3f%% index = %.3f%%\n",(100.0f*ol)/ov,(100.0f*on)/ov,(100.0f*of)/ov,(100.0f*oi)/ov);
*/
}
#endif // 0


/*****************************************************************************
 *
 * Function:		SilhouetteCache::weldMesh()
 *
 * Description:		Welds a mesh (i.e. finds out all shared vertices in
 *					an input mesh and produces only unique vertices)
 *
 * Parameters:		loc			= vertex positions
 *					nVertices	= number of input vertices
 *					tris		= input triangle vertex indices
 *					nTriangles	= number of input triangles
 *
 * Returns:			pointer to Mesh structure
 *
 * Notes:			The routine also removes degenerate and duplicate
 *					triangles.
 *
 *****************************************************************************/

void SilhouetteCache::weldMesh (Mesh& mesh, const Vector3* loc, int nVertices, const Vector3i* tris, int nTriangles)
{
	int maxVertices = nTriangles*3;											// there cannot be more than this real vertices
	if (nVertices < maxVertices)
		maxVertices = nVertices;
	if (maxVertices == 0)
		maxVertices = 1;													// special hack fix for empty data

	TempArray<Vector3,false>	outVertices(maxVertices);					// output vertex data array
	TempArray<Vector3i,false>	outTris(nTriangles > 1 ? nTriangles : 1);	// output triangle data array

	{
		m_triangleWeldHash.reset(&outTris[0],nTriangles);					// reset the triangle weld hash
		m_vertexWeldHash.reset(&outVertices[0],maxVertices);				// reset the weld hash

		TempArray<INT32,false>		vRemap(nVertices);						// vertex remap table
		fillDWord((UINT32*)(&vRemap[0]),0xFFFFFFFF,nVertices);				// 0xFFFFFFFF = not used yet

		for (int i = 0; i < nTriangles; i++)								// loop through all input triangles
		{
			const Vector3i& t = tris[i];

			DPVS_ASSERT(t.i >=0 && t.i < nVertices);
			DPVS_ASSERT(t.j >=0 && t.j < nVertices);
			DPVS_ASSERT(t.k >=0 && t.k < nVertices);

			//----------------------------------------------------------------
			// Skip degenerate triangles (these could have been introduced
			// by strip generation in modeling/importing phases). A degenerate
			// triangle has two or three vertices with exactly the same position.
			// Such triangles cannot affect the rasterization output in any
			// way, so we can safely remove them.
			//----------------------------------------------------------------

			if (loc[t.i] == loc[t.j] || loc[t.i] == loc[t.k] || loc[t.j] == loc[t.k])	
				continue;

			//----------------------------------------------------------------
			// Now, add all new vertices (use hash to find identical vertex
			// positions and weld such vertices together).
			//----------------------------------------------------------------

			Vector3i tri;											// output triangle with re-mapped vertex indices

			for (int j = 0; j < 3; j++)
			{
				int vertex = t[j];
				int index  = vRemap[vertex];
				if (index==-1) /* 0xFFFFFFFF */						// if not processed yet..
				{
					index = m_vertexWeldHash.getIndex(loc[vertex]);
					vRemap[vertex] = index;
				}
				tri[j] = index;
			}

			//----------------------------------------------------------------
			// Rotate the output triangle so that the smallest index always
			// comes first (this is needed to properly recognize duplicate
			// triangles that differ only by rotation)..
			//----------------------------------------------------------------

			if (tri.j < tri.i && tri.j < tri.k)						// second index is smallest
			{
				swap(tri.i,tri.j);									// BAC
				swap(tri.j,tri.k);									// BCA
			} else
			if (tri.k < tri.i && tri.k < tri.j)
			{
				swap(tri.i,tri.j);									// BAC
				swap(tri.i,tri.k);									// CAB
			}

			//----------------------------------------------------------------
			// Insert the rotated triangle into the hash (this will automatically
			// add the triangle into the output list, if its unique)
			//----------------------------------------------------------------

			m_triangleWeldHash.getIndex(tri);						// there we go..
		}

	}

	int nOutVertices = m_vertexWeldHash.getOffset();				// # of output vertices
	int nOutTris     = m_triangleWeldHash.getOffset();				// # of output triangles

	//----------------------------------------------------------------
	// Fix for a fully degenerate mesh (the remaining pipeline assumes
	// we have at least one triangle and one vertex, so let's make
	// it happy by generating an "empty" triangle at the first 
	// referenced vertex
	//----------------------------------------------------------------

	if (!nOutTris)
	{
		outTris[0].i = 0;														// triangle (0,0,0)
		outTris[0].j = 0;
		outTris[0].k = 0;

		outVertices[0] = nTriangles ? loc[tris[0].i] : Vector3(0.0f,0.0f,0.0f);	// choose first vertex of the mesh (or empty vector if there were no triangles)
		nOutVertices = 1;
		nOutTris     = 1;
	} else
	{
		//----------------------------------------------------------------
		// Here we sort the triangles based on their vertex access order. 
		// The purpose of this is to improve the cache coherence later.
		//----------------------------------------------------------------

		quickSort ((SortTri*)(&outTris[0]), nOutTris);					// sort the triangles based on access order
	}

	//----------------------------------------------------------------
	// Copy data into output mesh
	//----------------------------------------------------------------

//	testos (nOutVertices,nOutTris, &outVertices[0], &outTris[0]);

	mesh.set(nOutVertices,nOutTris, &outVertices[0], &outTris[0]);
}

/*****************************************************************************
 *
 * Function:		SilhouetteCache::freeRecyclers()
 *
 * Description:		Frees all data in recycler structures
 *
 *****************************************************************************/

void SilhouetteCache::freeRecyclers (void)
{
	m_recycler.free();					
	m_nodeVertexCache.reset(0);
	m_nodeVertexCacheSize = 0;			// MUST BE DONE HERE IF VERTEX CACHE IS FREED
	
	m_triangleWeldHash.reset(null,0);	// release memory used
	m_vertexWeldHash.reset(null,0);		// release memory used
	m_plEqWeldHash.reset(null,0);		// release memory used

	DELETE(m_edgeHash);
	m_edgeHash		= null;
}

/*****************************************************************************
 *
 * Function:		SilhouetteCache::invalidateCache()
 *
 * Description:		Invalidates contents of the silhouette cache
 *
 *****************************************************************************/

void SilhouetteCache::invalidateCache (void)
{
	int size = getMaxCacheSize();	// take a copy

	setMaxCacheSize(0);				// switch temporarily to 0 bytes
	setMaxCacheSize(size);			// return back to normal
	
	freeRecyclers();				// free the recyclers
}

/*****************************************************************************
 *
 * Function:		SilhouetteCache::setMaxCacheSize()
 *
 * Description:		Sets maximum cache memory consumption
 *
 * Parameters:		bytes = number of bytes of memory to use
 *
 * Notes:			True memory consumption can exceed this value by
 *					the memory usage of a single silhouette (in case
 *					that max size < memory needed by a single silhouette).
 *
 *					The cache can be flushed by setting the size first to
 *					zero and then returning the size to old value.
 *
 *****************************************************************************/

void SilhouetteCache::setMaxCacheSize (int bytes)
{
	DPVS_ASSERT (bytes >= 0);
	m_maxCacheSizeInBytes = bytes;
	keepCacheSizeInBounds();
}


//------------------------------------------------------------------------
