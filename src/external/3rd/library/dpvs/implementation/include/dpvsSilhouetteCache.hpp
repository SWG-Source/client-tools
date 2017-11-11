#ifndef __DPVSSILHOUETTECACHE_HPP
#define __DPVSSILHOUETTECACHE_HPP
/*****************************************************************************
 *
 * dPVS
 * -----------------------------------------
 *
 * (C) 1999-2004 Hybrid Graphics, Ltd.
 * All Rights Reserved.
 *
 * This file consists of unpublished, proprietary source code of
 * Hybrid Graphics, and is considered Confidential Information for
 * purposes of non-disclosure agreement. Disclosure outside the terms
 * outlined in signed agreement may result in irreparable harm to
 * Hybrid Graphics and legal action against the party in breach.
 *
 * Description: 	Silhouette cache header file
 *
 * $Archive: /dpvs/implementation/include/dpvsSilhouetteCache.hpp $
 * $Author: wili $ 
 * $Revision: #2 $
 * $Modtime: 28.01.02 11:29 $
 * $Date: 2004/02/06 $
 * 
 ******************************************************************************/

#if !defined (__DPVSRECYCLER_HPP)
#	include "dpvsRecycler.hpp"
#endif

#if !defined (__DPVSARRAY_HPP)
#	include "dpvsArray.hpp"
#endif

#if !defined (__DPVSMESH_HPP)
#	include "dpvsMesh.hpp"
#endif

#if !defined (__DPVSWELDHASH_HPP)
#	include "dpvsWeldHash.hpp"
#endif

namespace DPVS
{
class Sphere;

/******************************************************************************
 *
 * Class:			DPVS::SilhouetteCache
 *
 * Description:		Class for managing write silhouettes
 *
 *****************************************************************************/

class SilhouetteCache
{
public:
	class  Client;
	struct EdgeSilhouetteInfo;
	struct Node;

	//----------------------------------------------------------------
	// Public API functions
	//----------------------------------------------------------------

	static void				init					(void);
	static void				exit					(void);
	static SilhouetteCache* get						(void)			{ DPVS_ASSERT(s_silhouetteCache); return s_silhouetteCache; }

	int						getMaxCacheSize			(void) const	{ return m_maxCacheSizeInBytes; }
	void					setMaxCacheSize			(int bytes);
	int						getMemoryUsed			(void) const	{ return m_nodeMemoryUsed; }
	void					invalidateCache			(void);
	void					updateTimeStamp			(void);
	void					weldMesh				(Mesh& mesh, const Vector3* loc, int nVertices, const Vector3i* tris, int nTriangles);

	void					getSilhouette			(Client*, EdgeSilhouetteInfo&, const Vector4& camInObjectSpace);
	void					deleteClientNodes		(Client*);	// used semi-internally
	void					setupDerived			(const Client*);

	//----------------------------------------------------------------
	// SilhouetteCache::Client class (ImpMeshModel inherits this)
	//----------------------------------------------------------------

	class Client
	{	
	public:
		enum
		{
			MAX_TRIANGLES = 65535,				// max triangles we can use for an occluder
			MAX_VERTICES  = 65535				// max vertices we can use for an occluder
		};

		struct Edge
		{
			enum
			{
				NOT_CONNECTED = 0xFFFF			// symbolic value for a "non-connected" edge (should be outside [0,MAX_TRIANGLES[ range)
			};

			UINT16	m_vertex[2];				// two vertex indices (both must be valid)
			UINT16	m_plane[2];					// two indices to plane equation array (second one may be NOT_CONNECTED)
		};

		DPVS_CT_ASSERT(sizeof(Edge)==8);

		// this structure should really be protected, but different compilers seem to have
		// different views on its visibility, so the only way to get all of them to accept
		// it is to make it public -- dooooh...
		struct Derived
		{
						Derived		(void);
						~Derived	(void);

			int			m_numPlanes;			// number of entries in plane equation array (must be <= MAX_TRIANGLES)
			Vector4*	m_plEq;					// array of plane equations (m_numPlanes entries)
			int			m_numSilhouetteEdges;	// number of (potential) silhouette edges (i.e. non-planar)
			Edge*		m_edges;				// array of edges (total # == m_numSilhouetteEdges) -- does not have _planar_ edges!!
			Node*		m_firstNode;			// pointer to first Node
			UINT16*		m_trianglePlanes;		// array of indices (for each triangle) into the m_plEq array (numTriangles entries)
			char*		m_alloc;				// all data is allocated with this (so don't go and delete m_plEq etc.)

			static int	s_derivedMemoryUsed;	// # of bytes used by derived data structures (by all Derived structures)
			static int  s_numDerived;			// number of live derived structures

		};


	private:

		friend class CachedSilhouette;
		friend class SilhouetteCache;

		Mesh				m_mesh;					// mesh data
		mutable Derived*	m_derived;				// pointer to derived data (may be null)
		Client*				m_activePrev;			// previous in silhouette cache's active list
		Client*				m_activeNext;			// next in silhouette cache's active list
		bool				m_planar:1;				// is mesh completely planar?
		bool				m_active:1;				// is client currently in the active list?
		bool				m_optimizePlanarity:1;	// should plane equations be optimized? (initially true, but may be disabled later)
		bool				m_optimizeEdges:1;		// should plane planar edges be optimized? (initially true, but may be disabled later)

		static Client*		s_activeHead;		// head of active clients
		static Client*		s_activeTail;		// tail of active clients
 
							Client				(const Client&);	// not allowed
		Client&				operator=			(const Client&);	// not allowed
		void				deleteMesh			(void);
		void				setupDerived		(void) const;
		void				setActive			(void);
		void				setInactive			(void);
		bool				testPlanarity		(void) const;
		int					getDerivedMemoryUsed (void) const	// derived memory used by this Client
		{ 
			if (!m_derived)
				return 0;
			return sizeof(Derived) + sizeof(Vector4)*m_derived->m_numPlanes + sizeof(Edge)*m_derived->m_numSilhouetteEdges + sizeof(UINT16)*getTriangleCount();
		}

	protected:

							Client				(void);
		virtual				~Client				(void);

		// may return NULL if routine decides that mesh is too complex!
		const Derived*		getDerived			(void)
		{
			if (!m_derived) 
				SilhouetteCache::get()->setupDerived(this); 
			else
				setActive();

			return m_derived; 
		}
		
		const Mesh*			getMesh				(void) const	{ return &m_mesh; }
		int					getTriangleCount	(void) const	{ return m_mesh.getTriangleCount(); }
		const Vector3i*		getTriangleVertices	(void) const	{ return m_mesh.getTriangles();		}
		int					getVertexCount		(void) const	{ return m_mesh.getVertexCount();	}
		const Vector3*		getVertices			(void) const	{ return m_mesh.getVertices();		}
		bool				isPlanar			(void) const	{ return m_planar; }
		void				releaseDerived		(void);
		void				setMesh				(const Vector3* vertices, int nVertices, const Vector3i* triangles, int nTriangles);
	};

	struct EdgeSilhouetteInfo
	{
		const Vector3*		m_loc;					// vertex positions
		const Vector2i*		m_edges;				// edge vertex indices (corresponding with m_loc array)
		int					m_numVertices;			// number of vertices in m_loc[]
		int					m_numEdges;				// number of edges in m_edges[]
	};

							SilhouetteCache			(void);
							~SilhouetteCache		(void);

private:
							SilhouetteCache			(const SilhouetteCache&);	// not allowed
	SilhouetteCache&		operator=				(const SilhouetteCache&);	// not allowed

	struct CachedVertex
	{
		DPVS_FORCE_INLINE CachedVertex(void) {}

		UINT16	m_timeStamp;						// time stamp
		UINT16	m_index;							// output index
	};

	DPVS_CT_ASSERT(sizeof(CachedVertex)==4);
	
	void					freeRecyclers			(void);
	void					keepCacheSizeInBounds	(void);
	Node*					createNode				(Client*, const Vector4&);
	Node*					getBestNode				(Client*, const Vector4&) const;
	void					getExactSilhouette		(EdgeSilhouetteInfo& info, Client* c, const Vector4& camInObjectSpace);
	bool					isCacheCongested		(void) const;
	bool					isCacheFull				(void) const;
	DPVS_FORCE_INLINE void	refineSilhouette		(EdgeSilhouetteInfo& info, Node& node, const Vector4& camInObjectSpace);
	void					setupVertexCache		(const Client*);
	void					touchNode				(Node*);
	void					deleteNode				(Node*);

	int						m_nodeMemoryUsed;		// # of bytes of node cache memory used

	int						m_maxCacheSizeInBytes;	// maximum cache size in bytes
	Node*					m_nodeHead;				// first node (most recently used)
	Node*					m_nodeTail;				// last node (least recently used)
	int						m_nodeCount;			// number of nodes in existence
	UINT32					m_nodeTimeStamp;		// node time stamp
	int						m_nodeVertexCacheSize;	// node vertex cache size
	UINT32					m_nodeVertexCacheTimeStamp;	// time stamp
	Array<CachedVertex>		m_nodeVertexCache;		// vertex cache recycler
	Recycler<unsigned char>	m_recycler;				// shared recycler used internally for all operations
	Hash<Vector2i,int>*		m_edgeHash;				// edge hash table recycler
	WeldHash<Vector3i>		m_triangleWeldHash;		// triangle weld hash
	WeldHash<Vector3>		m_vertexWeldHash;		// vertex weld hash
	WeldHash<Vector4>		m_plEqWeldHash;			// plane equation weld hash
	
	static SilhouetteCache*	s_silhouetteCache;		// silhouette cache
};

} // DPVS

//------------------------------------------------------------------------
#endif //__DPVSSILHOUETTECACHE_HPP
	
