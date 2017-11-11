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
 * Description:		Utility code
 *
 * $Archive: /dpvs/implementation/sources/dpvsUtility.cpp $
 * $Author: wili $ 
 * $Revision: #4 $
 * $Modtime: 26.10.01 13:02 $
 * $Date: 2003/08/24 $
 * 
 ******************************************************************************/

#include "dpvsPrivateDefs.hpp"
#include "dpvsUtility.hpp"
#include "dpvsHash.hpp"
#include "dpvsAABB.hpp"
#include "dpvsMath.hpp"
#include "dpvsRandom.hpp"
#include "dpvsSort.hpp"
#include "dpvsTempArray.hpp"
#include "dpvsPriorityQueue.hpp"
#include "dpvsModel.hpp"

//#include <cstdio>

using namespace DPVS;

/*
	Parameters used for the splitting process
*/

namespace DPVS
{
	static const int	MIN_ALWAYS_COMPLEX_TRIS = 384;	/* any object having more than this # of triangles will be split */
	static const int	DEFAULT_TARGET_TRIS		= 192;	/* default target tris / object									 */
//	static const int	MIN_COMPLEX_TRIS		= 8;	/* absolute minimum triangle count for splitting				 */
}

/*****************************************************************************
 *
 * Class:			DPVS::ImpObjectSplitter
 *
 * Description:		Implementation class for splitting objects
 *
 * Notes:			The splitter never clips triangles. A separate tesselator
 *					should be run externally to partition very large triangles
 *					(large relative to scene size) to enchance the results
 *					of the object splitter.
 *
 *****************************************************************************/

class ImpObjectSplitter
{
public:
						ImpObjectSplitter	(const Vector3* vertices, const Vector3i* triangles, const UINT32* triMaterials, int numVertices,int numTriangles, const Vector3& sceneScale, float minObjectSize, int targetObjects);
						~ImpObjectSplitter	(void);
	void				split				(void);
	int					assignTriangleIDs	(UINT32* objectIDs);
private:
						ImpObjectSplitter	(const ImpObjectSplitter&);
	ImpObjectSplitter&	operator=			(const ImpObjectSplitter&);

	struct Triangle;
	struct Object;

	struct Vertex
	{
		Vector3			m_location;					// vertex position
	};

	struct Triangle
	{
		Vertex*			m_vertices[3];				// pointers to three vertices
		Object*			m_object;					// which object does the triangle belong to?
		Triangle*		m_next;						// next triangle in same object
		Triangle*		m_prev;						// previous triangle in same object
		int				m_material;					// material index (remapped)

		Triangle() : m_object(null),m_next(null),m_prev(null),m_material(0) { m_vertices[0]=m_vertices[1]=m_vertices[2]=null; }

		float getSurfaceArea (void) const
		{
			const Vector3& a = m_vertices[0]->m_location;
			const Vector3& b = m_vertices[1]->m_location;
			const Vector3& c = m_vertices[2]->m_location;

 			float	x1 = b.x - a.x;
			float	y1 = b.y - a.y;
			float	z1 = b.z - a.z;
			float	x2 = c.x - a.x;
			float	y2 = c.y - a.y;
			float	z2 = c.z - a.z;
			float	nx = (y1*z2)-(z1*y2);
			float	ny = (z1*x2)-(x1*z2);
			float	nz = (x1*y2)-(y1*x2);
			float	d = nx*nx+ny*ny+nz*nz;

			return Math::sqrt(d)*0.25f;
		}

		Vector3 getCenter (void) const
		{
			return Vector3((m_vertices[0]->m_location+m_vertices[1]->m_location+m_vertices[2]->m_location)*0.333333f);
		}
	};

	struct Object
	{
		Triangle*		m_head;						// first triangle in the object
		Object*			m_next;						// next object
		Object*			m_prev;						// previous object
		int				m_objectID;					// id number (for external referencing)
		int				m_numTriangles;				// number of triangles in the object
		mutable bool	m_aabbDirty;				// is AABB dirty?
		mutable AABB	m_AABB;						// cached AABB

				Object				(void) : m_head(null),m_next(null),m_prev(null),m_objectID(0),m_numTriangles(0),m_aabbDirty(true) {}
		int		getTriangleCount	(void) const		{ return m_numTriangles; }
		void	setAABB				(const AABB& s)		{ m_AABB = s; m_aabbDirty = false;}
	};


	struct PQueueItem								// objects in the priority queue
	{
		Object*			m_object;					// pointer to object
		float			m_priority;					// current priority of object (smaller is more important!)

		DPVS_FORCE_INLINE bool operator<= (const PQueueItem& s) const
		{
			return m_priority <= s.m_priority;
		}
	};

	struct SortInfo
	{
		Triangle*	m_triangle;
		float		m_sortValue;					// sort value

		bool		operator> (const SortInfo& s) const	{ return m_sortValue > s.m_sortValue; }
		bool		operator< (const SortInfo& s) const	{ return m_sortValue < s.m_sortValue; }
	};

	AABB				m_sceneBounds;				// bounds of the entire scene
	Vector3				m_ooSceneScale;				// 1.0 / scene bounds dims
	Vector3				m_sceneScale;				// scene bounds dims
	Array<Vertex>		m_vertices;					// vertex array
	Array<Triangle>		m_triangles;				// triangle array
	PQueue<PQueueItem>	m_queue;					// priority queue
	Object*				m_firstObject;				// pointer to first object
	float				m_minObjectSize;			// minimum object size
	float				m_avgTriangleBoxArea;		// average triangle*box area
	int					m_numObjects;				// number of objects
	int					m_numTriangles;				// number of triangles
	int					m_numVertices;				// number of vertices
	int					m_numMaterials;				// number of different materials
	int					m_targetObjects;			// target object count

	void				combineAllSimpleObjects (void);
	Object*				createObject			(void);
	void				deleteObject			(Object*);
	bool				fitPlane				(float& bestSplitVal, int& bestAxis, const AABB& box, const AABB* obs, int N);
	void				getObjectAABB			(AABB& box, const Object* o);
	float				getObjectTotalTriangleArea	(const Object* o);
	float				getObjectTotalTriangleVolume	(const Object* o);
	Triangle&			getTriangle				(int index) { DPVS_ASSERT(index>=0 && index < m_numTriangles);return m_triangles[index]; }
	void				getTriangleAABB			(AABB& box, const Triangle* t);
	float				getObjectCost			(Object* o) { AABB box; getObjectAABB(box,o); return box.getSurfaceArea() * o->m_numTriangles; }
	Vertex&				getVertex				(int index) { DPVS_ASSERT(index>=0 && index < m_numVertices); return m_vertices[index]; }
	bool				isObjectComplex			(const Object* o, const AABB& box);
	void				moveAllTrianglesToObject(Object* d, Object* s);
	void				removeAllObjects		(void);
	void				removeEmptyObjects		(void);
	void				scanTriangleNeighbors	(Object* o, Triangle* t);
	void				setTriangleToObject		(Object*, Triangle*);
	void				splitObject				(Object*);
	void				setupBuckets			(void);
	void				insertToPriorityQueue	(Object*);

	struct Bucket
	{
		Triangle**			m_triangles;			// array of triangle pointers
		int					m_numTriangles;			// number of triangles in the bucket
	};

	DynamicArray<Triangle*>	m_stack;				// internal stack used by search operations
	Hash<UINT32,Bucket*>	m_bucketHash;			// integer XYZ coords -> bucket
	Array<Bucket>			m_buckets;
	Array<Triangle*>		m_bucketTriangles;
	int						m_numBuckets;			// number of used buckets
	Vector3i				m_bucketDims;			// bucket integer scale values
	Vector3					m_weldDist;				// weld distances

	inline Vector3i		getVertexBucketCoords (const Vector3& v) const
	{
		Vector3 w = v - m_sceneBounds.getMin();
		w.x *= m_ooSceneScale.x * (m_bucketDims.i-1);
		w.y *= m_ooSceneScale.y * (m_bucketDims.j-1);
		w.z *= m_ooSceneScale.z * (m_bucketDims.k-1);
		
		int x = Math::intChop(w.x);	// values are positive so we can use chop here..
		int y = Math::intChop(w.y);
		int z = Math::intChop(w.z);

		return Vector3i(x,y,z);
	}

	UINT32 getBucketID (const Vector3i& v)
	{
		return v.i + v.j*m_bucketDims.i + v.k*(m_bucketDims.i*m_bucketDims.j);

	}
};

/******************************************************************************
 *
 * Function:		DPVS::ImpObjectSplitter::getTriangleAABB()
 *
 * Description:		Returns axis-aligned bounding box of a triangle
 *
 * Parameters:		box = reference to AABB where results are stored
 *					t	= pointer to triangle (non-null)
 *
 ******************************************************************************/

DPVS_FORCE_INLINE void ImpObjectSplitter::getTriangleAABB	(AABB& box, const Triangle* t)
{
	DPVS_ASSERT(t);
	box.set	(t->m_vertices[0]->m_location,t->m_vertices[0]->m_location);
	box.grow(t->m_vertices[1]->m_location);
	box.grow(t->m_vertices[2]->m_location);
}

/******************************************************************************
 *
 * Function:		DPVS::ImpObjectSplitter::getObjectAABB()
 *
 * Description:		Returns AABB of an object
 *
 * Parameters:		box = reference to AABB where results are stored
 *					o	= pointer to object
 *
 * Notes:			The function scans the AABB from the triangles, so
 *					it's pretty slow...
 *
 *					The return values are garbage (asserts in debug build)
 *					if object does not have any triangles assigned to it.
 *
 ******************************************************************************/

void ImpObjectSplitter::getObjectAABB (AABB& box, const Object* o)
{
	//--------------------------------------------------------------------
	// DEBUG DEBUG TODO: we could cache this information as it is
	// going to be queried multiple times per object.
	//--------------------------------------------------------------------
	
	if (!o->m_head)	// DOH!	
	{
		DPVS_ASSERT(false);		// ?
		return;
	}

	if (o->m_aabbDirty)
	{
		getTriangleAABB(o->m_AABB,o->m_head);

		for (const Triangle* t = o->m_head->m_next; t; t = t->m_next)
		for (int j = 0; j < 3; j++)
			o->m_AABB.grow(t->m_vertices[j]->m_location);
		o->m_aabbDirty = false;
	}
	box = o->m_AABB;
}
	
/******************************************************************************
 *
 * Function:		DPVS::ImpObjectSplitter::removeEmptyObjects()
 *
 * Description:		Kills all objects that don't have any triangles in them
 *
 ******************************************************************************/

void ImpObjectSplitter::removeEmptyObjects (void)
{
	Object* o = m_firstObject;

	while (o)
	{
		Object* next = o->m_next;			
		if (o->m_head==null)				// no triangles in object
			deleteObject(o);
		o = next;
	}
}


/******************************************************************************
 *
 * Function:		DPVS::ImpObjectSplitter::assignTriangleIDs()
 *
 * Description:		Generates IDs for the objects, then assigns them
 *					to triangles..
 *
 * Parameters:		objectIDs = output object ID array
 *
 * Returns:			number of output groups
 *
 ******************************************************************************/

int ImpObjectSplitter::assignTriangleIDs (UINT32* objectIDs)
{
	DPVS_ASSERT(objectIDs);

	int id  = 0;
	int cnt = 0;
	for (Object* o = m_firstObject; o; o = o->m_next)
	{
		if (o->m_head)
			o->m_objectID = id++;						// assign IDs for all objects with triangles
		cnt += o->m_numTriangles;
	}

	DPVS_ASSERT(cnt == m_numTriangles);
	DPVS_ASSERT(id>0);								// HUH?

	for (int i = 0; i < m_numTriangles; i++)
	{
		DPVS_ASSERT(getTriangle(i).m_object);
		objectIDs[i] = getTriangle(i).m_object->m_objectID; 
	}

	return id;
}

/******************************************************************************
 *
 * Function:		DPVS::ImpObjectSplitter::removeAllObjects()
 *
 * Description:		
 *
 ******************************************************************************/

void ImpObjectSplitter::removeAllObjects (void)
{
	for (int i = 0; i < m_numTriangles; i++)
	{
		getTriangle(i).m_object = null;
		getTriangle(i).m_prev   = null;
		getTriangle(i).m_next   = null;
	}

	for (Object* o = m_firstObject; o; o = o->m_next)
	{
		o->m_head			= null;
		o->m_numTriangles	= 0;
	}

	while (m_firstObject)
		deleteObject (m_firstObject);

	DPVS_ASSERT (m_numObjects==0);
}

static float measure (const AABB& box)
{
	float x = box.getMax().x - box.getMin().x;
	float y = box.getMax().y - box.getMin().y;
	float z = box.getMax().z - box.getMin().z;

	float l = Math::sqrt(x*x+y*y) + 
			  Math::sqrt(x*x+z*z) +
			  Math::sqrt(y*y+z*z);

	return l*l;
}

/******************************************************************************
 *
 * Function:		DPVS::ImpObjectSplitter::insertToPriorityQueue()
 *
 * Description:		
 *
 * Parameters:		o = pointer to destination object (may be null)
 *
 ******************************************************************************/

void ImpObjectSplitter::insertToPriorityQueue (Object* o)
{
	if (o && o->m_numTriangles)
	{
		PQueueItem item;
		AABB box;
		item.m_object = o;

		float tArea		= 0.0f;

		for (Triangle* t = o->m_head; t; t = t->m_next)
			tArea   += t->getSurfaceArea();

		getObjectAABB(box,o);

		float cost  = 1.0f / measure(box);
		cost /= (o->m_numTriangles);

		if (o->m_numTriangles >= 2048)
			cost = -0.0001f;	// force split in all cases...
	
		item.m_priority = cost; // the smaller the priority is, the more important it is.. 
		m_queue.insert(item);
	}
}

/******************************************************************************
 *
 * Function:		DPVS::ImpObjectSplitter::setTriangleToObject()
 *
 * Description:		
 *
 * Parameters:		o = pointer to destination object (may be null)
 *					t = pointer to triangle
 *
 * Notes:			If triangle belongs to some object, it is removed from the
 *					old object.
 *
 ******************************************************************************/

// o can be null
void ImpObjectSplitter::setTriangleToObject	(Object* o, Triangle* t)
{
	DPVS_ASSERT(t);		

	// TODO: early exit if t->m_obejct==o ???

	if (t->m_object)
	{
		if (t->m_prev) 
			t->m_prev->m_next = t->m_next; 
		else
		{
			DPVS_ASSERT(t == t->m_object->m_head);
			t->m_object->m_head = t->m_next;
		}

		if (t->m_next)
			t->m_next->m_prev = t->m_prev;

		t->m_object->m_numTriangles--;				// number of triangles ... 
		t->m_object->m_aabbDirty = true;

		DPVS_ASSERT (t->m_object->m_numTriangles >= 0);
		DPVS_ASSERT ((t->m_object->m_head) || (t->m_object->m_numTriangles==0));

		t->m_object = null;							
		t->m_prev   = null;
		t->m_next   = null;
	} else
	{
		DPVS_ASSERT(!t->m_prev);
		DPVS_ASSERT(!t->m_next);
	}

	if (o)											// target object
	{
		t->m_object = o;
		t->m_prev   = null;
		t->m_next   = o->m_head;
		if (t->m_next)
			t->m_next->m_prev = t;
		o->m_head = t;
		o->m_numTriangles++;						// increase triangle count
		o->m_aabbDirty = true;
	}
}

void ImpObjectSplitter::moveAllTrianglesToObject (Object* d, Object* s)
{
	Triangle* t = s->m_head;
	while (t)
	{
		Triangle* next = t->m_next;
		setTriangleToObject(d,t);
		t = next;
	}
}

/******************************************************************************
 *
 * Function:		DPVS::ImpObjectSplitter::createObject()
 *
 * Description:		
 *
 * Returns:			pointer to newly created Object
 *
 ******************************************************************************/

ImpObjectSplitter::Object* ImpObjectSplitter::createObject (void)
{
	Object* o			= NEW<Object>();			// DEBUG DEBUG USE POOL ALLOCATOR
	o->m_head			= null;
	o->m_objectID		= 0;						// not needed yet.. we'll assign them later
	o->m_numTriangles	= 0;
	o->m_prev			= null;
	o->m_next			= m_firstObject;
	if (o->m_next)
		o->m_next->m_prev = o;
	m_firstObject		= o;

	m_numObjects++;
	return o;
}

/******************************************************************************
 *
 * Function:		DPVS::ImpObjectSplitter::deleteObject()
 *
 * Description:		
 *
 * Parameters:		o = pointer to object
 *
 ******************************************************************************/

void ImpObjectSplitter::deleteObject (Object* o)
{
	DPVS_ASSERT (o);
	DPVS_ASSERT (!o->m_numTriangles);
	DPVS_ASSERT (!o->m_head);

	if (o->m_prev)
		o->m_prev->m_next = o->m_next;
	else
	{
		DPVS_ASSERT(o == m_firstObject);
		m_firstObject = o->m_next;
	}

	if (o->m_next)
		o->m_next->m_prev = o->m_prev;

	DELETE(o);

	m_numObjects--;
	DPVS_ASSERT(m_numObjects >= 0);
}

/******************************************************************************
 *
 * Function:		DPVS::ImpObjectSplitter::setupBuckets()
 *
 * Description:		Sets up bucketing
 *
 ******************************************************************************/

void ImpObjectSplitter::setupBuckets (void)
{
	m_buckets.reset(m_numVertices);

	m_numBuckets	= 0;
	m_bucketDims.i	= 64*8;										// 1.0 / # of X dimension etc.
	m_bucketDims.j	= 64*8;
	m_bucketDims.k	= 64*8;

	m_weldDist = (m_sceneBounds.getMax() - m_sceneBounds.getMin());	// weld distance vector
	m_weldDist.x /= (m_bucketDims.i * 2.0f);
	m_weldDist.y /= (m_bucketDims.j * 2.0f);
	m_weldDist.z /= (m_bucketDims.k * 2.0f);

	for (int i = 0; i < m_numVertices; i++)
	{
		m_buckets[i].m_triangles    = null;
		m_buckets[i].m_numTriangles = 0;							// zero triangles
	}

	int numTriangles = 0;

	for (int i = 0; i < m_numTriangles; i++)
	{	
		int id[3];

		for (int j = 0; j < 3; j++)
			id[j] = getBucketID(getVertexBucketCoords(m_triangles[i].m_vertices[j]->m_location));

		if (id[0] == id[1]) id[1] = -1;			// remove duplicate entries
		if (id[0] == id[2]) id[2] = -1;
		if (id[1] == id[2]) id[2] = -1;

		for (int j = 0; j < 3; j++)
		if (id[j] >= 0)
		{
			int bucketID = id[j];
			Bucket* b = m_bucketHash.get(bucketID);						// get ptr to bucket
			if (!b)
			{
				b = &m_buckets[m_numBuckets++];							// allocate new bucket
				m_bucketHash.insert(bucketID,b);						// insert bucket to hash
			}

			b->m_numTriangles++;										// increase triangle count
			numTriangles++;
		}
	}

	DPVS_ASSERT(m_numBuckets <= m_numVertices);

	m_bucketTriangles.reset(numTriangles);

	Triangle** t = &m_bucketTriangles[0];
	for (int i = 0; i < m_numBuckets; i++)
	{
		m_buckets[i].m_triangles = t;				// assign ptr
		t += m_buckets[i].m_numTriangles;			// advance ptr
		m_buckets[i].m_numTriangles = 0;			// clear it
	}

	DPVS_ASSERT((t-(&m_bucketTriangles[0])) == numTriangles);

	for (int i = 0; i < m_numTriangles; i++)
	{
		int id[3];

		for (int j = 0; j < 3; j++)
			id[j] = getBucketID(getVertexBucketCoords(m_triangles[i].m_vertices[j]->m_location));

		if (id[0] == id[1]) id[1] = -1;			// remove duplicate entries
		if (id[0] == id[2]) id[2] = -1;
		if (id[1] == id[2]) id[2] = -1;

		for (int j = 0; j < 3; j++)
		if (id[j] >= 0)
		{
			UINT32 bucketID = id[j];
			Bucket* b = m_bucketHash.get(bucketID);					// get ptr to bucket
			DPVS_ASSERT(b);											// WASSSS??????
			b->m_triangles[b->m_numTriangles++] = &m_triangles[i];	// assign triangle to list
			numTriangles--;
		}
	}

	DPVS_ASSERT(numTriangles == 0);	
}

float ImpObjectSplitter::getObjectTotalTriangleArea (const Object* o)
{
	float totalArea = 0.0f;
	
	for (const Triangle* t = o->m_head; t; t = t->m_next)
	{
		AABB tBox;
		getTriangleAABB(tBox,t);
		totalArea += tBox.getSurfaceArea(); /*t->getSurfaceArea();*/
	}

	return totalArea;
}

float ImpObjectSplitter::getObjectTotalTriangleVolume (const Object* o)
{
	float totalVolume = 0.0f;
	
	for (const Triangle* t = o->m_head; t; t = t->m_next)
	{
		AABB tBox;
		getTriangleAABB(tBox,t);
		totalVolume += tBox.getVolume(); /*t->getSurfaceArea();*/
	}

	return totalVolume;
}

/******************************************************************************
 *
 * Function:		DPVS::ImpObjectSplitter::isObjectComplex()
 *
 * Description:		Determines whether an object is complex enough to be
 *					considered for splitting
 *
 * Parameters:		o	= pointer to object (non-NULL)
 *					box = AABB of the object
 *
 ******************************************************************************/

bool ImpObjectSplitter::isObjectComplex (const Object* o, const AABB& box)
{
	DPVS_ASSERT(o);

	if (o->m_numTriangles >= MIN_ALWAYS_COMPLEX_TRIS)					// object is complex (due to raw triangle count)
		return true;

	if (o->m_numTriangles <= 1 || box.getDiagonalLength() <= m_minObjectSize)
		return false;
	
	return true;
}
/******************************************************************************
 *
 * Function:		DPVS::ImpObjectSplitter::splitObject()
 *
 * Description:		Attempts to perform spatial splitting of an object
 *
 * Parameters:		o = pointer to object (may be NULL - in this case nothing is done).
 *
 ******************************************************************************/

void ImpObjectSplitter::splitObject (Object* o)
{
	if (!o)								
		return;

	//--------------------------------------------------------------------
	// Check if there's any need for spatial splitting. Here we have
	// a hard-coded absolute limit.
	//--------------------------------------------------------------------

	if (o->m_numTriangles <= 2)		// object is too simple to be split under any circumstances
		return;

	Object* left	= null;
	Object* right	= null;
	int		N		= 0;
	float	oldCost = getObjectCost(o);

	{	// the scope here is important: we want to free the array once we're done!!

		//--------------------------------------------------------------------
		// Compute bounding box for the object
		//--------------------------------------------------------------------

		AABB box;
		getObjectAABB(box,o);				// get bounding box for the entire object
		Vector3	c = box.getCenter();

		//--------------------------------------------------------------------
		// Find out if the object is "complicated and large" enough to be 
		// split.
		//--------------------------------------------------------------------

		if (!isObjectComplex(o,box))
			return;

		//--------------------------------------------------------------------
		// Build triangle pointer array and compute surface areas. Then
		// sort the array.
		//--------------------------------------------------------------------

		left  = createObject();			// create left sub-object
		right = createObject();			// create right sub-object

		TempArray<SortInfo,false>	sortInfo(o->m_numTriangles);
	
		for (const Triangle* t = o->m_head; t; t = t->m_next)
		{
			sortInfo[N].m_triangle	= const_cast<Triangle*>(t);
			N++;
		}

		DPVS_ASSERT(N == o->m_numTriangles);

		AABB leftBox;							/* init AABBs */
		AABB rightBox;

		{
			//--------------------------------------------------------------------
			// Find starting triangle
			//--------------------------------------------------------------------

			Triangle*	l		= null;

			{
				float		maxDist = 0.0f;

				for (int i = 0; i < N; i++)
				{
					Triangle* t = sortInfo[i].m_triangle;

					for (int j = 0; j < 3; j++)
					{
						Vector3 tmp = (t->m_vertices[j]->m_location - c);
						float dist  = dot(tmp,tmp);

						if (dist >= maxDist)
						{
							maxDist = dist;
							l       = t;
						}
					}
				}

				DPVS_ASSERT(l);
			}

			for (int i = 0; i < N; i++)
			{
				Vector3 tmp = sortInfo[i].m_triangle->getCenter() - box.getCenter();//l->getCenter();//box.getCenter();//l->getCenter();					
				sortInfo[i].m_sortValue = -dot(tmp,tmp);
			}

			quickSort (&sortInfo[0], N);

			//--------------------------------------------------------------------
			// Now find triangle with farthest center from the "left" one..
			//--------------------------------------------------------------------

			Triangle*	r		= null;

			{
				float		maxDist = 0.0f;
				Vector3		c		= l->getCenter();

				for (int i = 0; i < N; i++)
				{
					Triangle* t = sortInfo[i].m_triangle;

					if (t == l)									/* under no circumstances do this.. */
						continue;

					Vector3 tmp = (t->getCenter() - c);
					float dist  = dot(tmp,tmp);

					if (dist >= maxDist)
					{
						maxDist = dist;
						r       = t;
					}
				}

				DPVS_ASSERT(r);
			}

			setTriangleToObject(left, l);		// init left & right objects...
			setTriangleToObject(right,r);

			getTriangleAABB(leftBox,l);			// init left and right boxes
			getTriangleAABB(rightBox,r);
		}
		
		//--------------------------------------------------------------------
		// Now keep adding triangles to left and right boxes (largest
		// triangles first)
		//--------------------------------------------------------------------

		for (int i = 0; i < N; i++)
		{
			Triangle* t = sortInfo[i].m_triangle;
			if (t->m_object != o)						// was already assigned to left/right (i.e. was 'l' or 'r')
				continue;

			AABB triBox;
			getTriangleAABB(triBox,t);					// AABB of the triangle

			AABB newLeftBox  = leftBox;
			AABB newRightBox = rightBox;

			newLeftBox.grow(triBox);						// new modified boxes
			newRightBox.grow(triBox);

			// now select which would be a better idea...

			float costLeft	=  measure(newLeftBox) * (left->m_numTriangles+1) + measure(rightBox)    * (right->m_numTriangles);
			float costRight =  measure(leftBox)    * (left->m_numTriangles)   + measure(newRightBox) * (right->m_numTriangles+1);

			if (costLeft < costRight)
			{
				leftBox = newLeftBox;
				setTriangleToObject(left,t);
			} else
			{
				rightBox = newRightBox;
				setTriangleToObject(right,t);
			}
		}

		left->setAABB(leftBox);
		right->setAABB(rightBox);

	}

	float newCost = getObjectCost(left) + getObjectCost(right);

	if (left->m_numTriangles==0 || right->m_numTriangles==0 || (N <= 100 && newCost >= oldCost))		// failure! didn't get enough wins!
	{
		moveAllTrianglesToObject(o,left);		
		moveAllTrianglesToObject(o,right);
		deleteObject(left);
		deleteObject(right);
		return;
	}

	insertToPriorityQueue(left);
	insertToPriorityQueue(right);
}


// DEBUG DEBUG CURRENTLY WE USE THIS TO PARTITION OUR OBJECTS
void ImpObjectSplitter::combineAllSimpleObjects (void)
{
	int					cnt = 0;
	TempArray<Object*>	objects(m_numObjects);

	for (Object* o = m_firstObject; o; o = o->m_next)		// collect all objects into an array
		objects[cnt++] = o;						

	Object* n  = createObject();			// create new object

	for (int i = 0; i < cnt; i++)
	{
		Object* o = objects[i];
		// here determine whether object is "simple"

		if (o->m_numTriangles >= 128)						
			continue;

		AABB	box;
		getObjectAABB(box,o);								// get bounding box for the entire object

//		if (isObjectComplex(o,box))							// object is complex
//			continue;


		Triangle* t = o->m_head;							// move all triangles into new object
		while (t)
		{
			Triangle* next	= t->m_next;
			setTriangleToObject(n,t);
			t = next;
		}
	}

//	printf ("combo object has %d tris\n",n->m_numTriangles);
	removeEmptyObjects();
}

/******************************************************************************
 *
 * Function:		DPVS::ImpObjectSplitter::scanTriangleNeighbors()
 *
 * Description:		Assigns all connected triangles to an object starting
 *					from triangle t
 *
 * Parameters:		o = pointer to object
 *					t = source triangle
 *
 * Notes:			The function uses the weld info computed earlier, so
 *					it can skip over small cracks etc.
 *
 ******************************************************************************/

void ImpObjectSplitter::scanTriangleNeighbors (Object* o, Triangle* t)
{
	DPVS_ASSERT(o && t);
	DPVS_ASSERT (!t->m_object);

	setTriangleToObject(o,t);										// assign triangle to specified object

	int stackTop = 1;
	m_stack.getElement(0) = t;

	while (stackTop > 0)
	{
		Triangle* t = m_stack[--stackTop];							// pop triangle from stack
		DPVS_ASSERT(t->m_object);

		Vector4 tPlEq = Math::getNormalizedPlaneEquation(t->m_vertices[0]->m_location,t->m_vertices[1]->m_location,t->m_vertices[2]->m_location);

		for (int j = 0; j < 3; j++)										// find neighbors for each vertex
		{
			const Vertex* v	= t->m_vertices[j];
		
			Vector3i mn = getVertexBucketCoords(v->m_location - m_weldDist);
			Vector3i mx = getVertexBucketCoords(v->m_location + m_weldDist);
			Vector3i b;

			for (b.i = mn.i; b.i <= mx.i; b.i++)
			for (b.j = mn.j; b.j <= mx.j; b.j++)
			for (b.k = mn.k; b.k <= mx.k; b.k++)
			{
				Bucket* bk = m_bucketHash.get(getBucketID(b));		// get pointer to bucket
				if (!bk)
					continue;
				/* NOTE: WE HAVE NEW CODE THAT REMOVES USED TRIANGLES FROM THE BUCKETS!! */

				int outTris = 0;

				for (int i = 0; i < bk->m_numTriangles; i++)
				{
					Triangle* n = bk->m_triangles[i];
					
					if (n->m_object)	
						continue;

					bk->m_triangles[outTris++] = n;	/* add to output list! */

					bool ok = false;

					for (int k = 0; k < 3; k++)
					{
						Vector3 d = n->m_vertices[k]->m_location - v->m_location;
						if (Math::fabs(d.x) <= m_weldDist.x &&
							Math::fabs(d.y) <= m_weldDist.y &&
							Math::fabs(d.z) <= m_weldDist.z)
						{
							ok = true;
							break;
						}
					}

					if (!ok)											// not close enough..
						continue;

					Vector4 nPlEq = Math::getNormalizedPlaneEquation(n->m_vertices[0]->m_location,n->m_vertices[1]->m_location,n->m_vertices[2]->m_location);

					if ( (tPlEq.x*nPlEq.x + tPlEq.y*nPlEq.y + tPlEq.z*nPlEq.z) <= -0.3f)
						continue;

					setTriangleToObject(o,n);
					m_stack.getElement(stackTop) = n;
					stackTop++;
				}

				bk->m_numTriangles = outTris;	/* update output counter!! */
			}
		}		
	}
}


/******************************************************************************
 *
 * Function:		DPVS::ImpObjectSplitter::split()
 *
 * Description:		Perform splitting of triangles into objects
 *
 ******************************************************************************/

void ImpObjectSplitter::split (void)
{
	removeAllObjects();

	float totalArea = 0.0f;

	//--------------------------------------------------------------------
	// Recognize connected triangles (that form "objects") having
	// the same material. Create a set of Objects.
	//--------------------------------------------------------------------

	for (int i = 0; i < m_numTriangles; i++)
	{
		Triangle* t = &getTriangle(i);				// get access to triangle

		AABB tBox;
		getTriangleAABB(tBox,t);
		totalArea += tBox.getSurfaceArea(); 
		
		if (!t->m_object)							// .. triangle is in no object yet...
		{
			Object* n = createObject();				// create new object
			scanTriangleNeighbors (n,t);			// start recursive scan
		}
	}

	m_avgTriangleBoxArea = totalArea / m_numTriangles;

//	printf ("initial objects = %d\n",m_numObjects);

	//--------------------------------------------------------------------
	// Now we have the scene partitioned into objects formed of
	// "connected components". Now let's combine all small objects into
	// one large object that is then fed to the splitter. After this
	// we have some decent (although very big) objects to work with.
	//--------------------------------------------------------------------

	combineAllSimpleObjects();						// combine all detail objects etc.
	removeEmptyObjects();							// remove empty objects..

	//--------------------------------------------------------------------
	// TODO: At this point we should check if we have already exceeded our
	// object budget (in such a case we need to combine some objects).
	//--------------------------------------------------------------------

	//--------------------------------------------------------------------
	// Insert all objects into a priority queue and start processing
	// (we always process the "worst" object first).
	//--------------------------------------------------------------------

	for (Object* o = m_firstObject; o; o = o->m_next)
		insertToPriorityQueue(o);

	for (;;)
	{
		if (m_queue.isEmpty())						// we're done...
			break;

		PQueueItem item;
		m_queue.popMin(item);						// pop object from queue

		if (item.m_priority >= 0.0f && m_numObjects >= m_targetObjects)
			break;

//		printf ("popped object with priority = %.8f (tris = %d)\n",item.m_priority, item.m_object->m_numTriangles);

		splitObject(item.m_object);					// call the splitter (this may introduce new objects)

		if (item.m_object->m_head == null)			// delete the empty object..
			deleteObject(item.m_object);
	}

//	printf ("final objects = %d\n",m_numObjects);
	
	removeEmptyObjects();							// remove empty objects..

#if defined (DPVS_DEBUG)
	int triCount = 0;

	for (Object* o = m_firstObject; o; o = o->m_next)
		triCount+=o->m_numTriangles;

	DPVS_ASSERT(triCount == m_numTriangles);
/*
	for (Object* o = m_firstObject; o; o = o->m_next)
	if (o->m_numTriangles >= 10000)		// DEBUG DEBUG DEBUG!!!
		DPVS_ASSERT(false);
*/
#endif 

/*	{
		float totalCost = 0.0f;
		float triCost = 0.0f;
		int   nObjects = 0;
		for (Object* o = m_firstObject; o; o = o->m_next)		// collect all objects into an array
		{
			triCost += getObjectTotalTriangleArea(o);
			totalCost += getObjectCost(o);
			nObjects++;
		}
		printf ("tri cost   = %.3f\n",triCost);
		printf ("total cost = %.3f (%.5f : 1)\n",totalCost,totalCost/triCost);

		float quality = (totalCost/triCost) * nObjects;

		printf ("quality = %.3f (smaller = better)\n",quality);
	}
*/
}

/******************************************************************************
 *
 * Function:		DPVS::ImpObjectSplitter::ImpObjectSplitter()
 *
 * Description:		Constructor
 *
 * Notes:			All input data is copied and internal auxiliary data
 *					structures are set up
 *
 ******************************************************************************/

ImpObjectSplitter::ImpObjectSplitter (const Vector3* vertices, const Vector3i* triangles, const UINT32* triMaterials, int numVertices,int numTriangles, const Vector3& sceneScale, float minObjectSize, int targetObjects)
{
	DPVS_API_ASSERT(vertices);
	DPVS_API_ASSERT(triangles);
	DPVS_API_ASSERT(numVertices >= 3);
	DPVS_API_ASSERT(numTriangles >= 1);

	m_targetObjects			= targetObjects;
	m_minObjectSize			= minObjectSize;
	m_firstObject			= null;
	m_numObjects			= 0;
	m_numVertices			= 0;									// init to 0 (we get actual count later)
	m_numMaterials			= 0;									// init to 0 (we get actual count later)
	m_numTriangles			= numTriangles;							// triangle count is same as input

	if (m_targetObjects <= 0)
		m_targetObjects = (numTriangles / DEFAULT_TARGET_TRIS)+1;	
	
	m_triangles.reset(numTriangles);
	m_vertices.reset(numVertices);

	//------------------------------------------------------------------------
	// Generate triangles and map the vertices into "unique" vertices (based
	// on location). Copy vertex data when first created.
	//------------------------------------------------------------------------


	{
		Hash<Vector3,Vertex*>	vertexHash;								// location -> vertex hash
		Hash<int,int>			materialHash;							// input material id -> "tight material id" hash

		for (int i = 0; i < numTriangles; i++)
		{
			Triangle* t = &getTriangle(i);

			for (int j = 0; j < 3; j++)									// for each vertex of the triangle
			{
				int				index	= triangles[i][j];						// vertex index

				DPVS_API_ASSERT (index >= 0 && index < numVertices && "ImpObjectSplitter::ImpObjectSplitter() - vertex indices out of range");

				const Vector3&	loc		= vertices[index];
				Vertex*			v		= null;

				if (!vertexHash.get(loc,v))
				{
					DPVS_ASSERT(m_numVertices>=0 && m_numVertices < numVertices);	// HUH?
					
					v = &m_vertices[m_numVertices];									// allocate new vertex
					v->m_location				= loc;								// assign location
					vertexHash.insert(loc,v);										// insert to hash

					m_numVertices++;
				}

				t->m_vertices[j] = v;												// assign vertex
			}

			int mat = 0;

			if (triMaterials)														// if material array is defined...
			{
				if (!materialHash.get(triMaterials[i],mat))							// if material was not found...
				{
					mat = m_numMaterials;											// introduce new material...
					materialHash.insert(triMaterials[i],m_numMaterials++);	
				}
			}
			t->m_material	= mat;													// assign material
			t->m_object     = null;
			t->m_prev       = null;
			t->m_next		= null;
		}

		// hash tables fall out of scope and get destructed here
	}

	DPVS_ASSERT(m_numVertices>0);

	//--------------------------------------------------------------------
	// Calculate scene bounding box (to give us some idea of the overall
	// size)
	//--------------------------------------------------------------------

	m_sceneBounds.set(getVertex(0).m_location,getVertex(0).m_location);
	for (int i = 1; i < m_numVertices; i++)
		m_sceneBounds.grow(getVertex(i).m_location);

	// 1.0 / bounding box dimensions

	m_ooSceneScale	 = m_sceneBounds.getMax() - m_sceneBounds.getMin();
	m_ooSceneScale.x = m_ooSceneScale.x ? 1.0f / m_ooSceneScale.x : 0.0f;
	m_ooSceneScale.y = m_ooSceneScale.y ? 1.0f / m_ooSceneScale.y : 0.0f;
	m_ooSceneScale.z = m_ooSceneScale.z ? 1.0f / m_ooSceneScale.z : 0.0f;

	setupBuckets(); /* DEBUG DEBUG DEBUG */
	
	// setup scene scale 

	m_sceneScale = m_sceneBounds.getMax() - m_sceneBounds.getMin();
	if (sceneScale.x > m_sceneScale.x) m_sceneScale.x = sceneScale.x;
	if (sceneScale.y > m_sceneScale.y) m_sceneScale.y = sceneScale.y;
	if (sceneScale.z > m_sceneScale.z) m_sceneScale.z = sceneScale.z;
}

/******************************************************************************
 *
 * Function:		DPVS::ImpObjectSplitter::~ImpObjectSplitter()
 *
 * Description:		Destructor
 *
 * Notes:			Releases all temporary data structures
 *
 ******************************************************************************/

ImpObjectSplitter::~ImpObjectSplitter ()
{
	removeAllObjects();

	DPVS_ASSERT (!m_firstObject);
}

//------------------------------------------------------------------------
// Wrapper code
//------------------------------------------------------------------------

/******************************************************************************
 *
 * Function:		DPVS::ObjectSplitter::split()
 *
 * Description:		Splits a scene into "objects"
 *
 * Parameters:		objectIDs		= output object ID array
 *					vertices		= input vertex positions
 *					triangles		= input triangle vertex indices
 *					triMaterials	= input triangle materials (may be null)
 *					numVertices		= number of vertices
 *					numTriangles	= number of triangles
 *
 * Returns:			number of objects the input object was split into
 *
 * Notes:			The output array will contain object IDs for each triangle.
 *					The object IDs are in range [0,numObjects[, where numObjects
 *					is the return value of the function.
 *
 *					The 'triMaterials' array is optional (i.e. null may be
 *					submitted). If a triangle material array is provided, arbitrary
 *					UINT32 'material tags' can be used.
 *
 ******************************************************************************/

int	ObjectSplitter::splitEx (UINT32 objectIDs[], const Vector3* vertices, const Vector3i* triangles, const UINT32* triMaterials, int numVertices,int numTriangles, const Vector3& sceneScale, float minObjectSize, int targetObjects)
{
	DPVS_API_ASSERT(objectIDs && vertices && triangles && numVertices > 0 && numTriangles > 0 && targetObjects >= 0);

	//------------------------------------------------------------------------
	// Handle invalid input
	//------------------------------------------------------------------------

	if (!numVertices || !numTriangles)						
	{
		for (int i = 0; i < numTriangles; i++)
			objectIDs[i] = 0;
		return 1;
	}

	//------------------------------------------------------------------------
	// Validate input data
	//------------------------------------------------------------------------

#if defined (DPVS_DEBUG)

	for (int i = 0; i < numVertices; i++)
		DPVS_API_ASSERT(Math::isFinite(vertices[i]) && "DPVS::ObjectSplitter::split() - invalid vertex input data");

	for (int i = 0; i < numTriangles; i++)
	{
		DPVS_API_ASSERT(triangles[i].i >= 0 && triangles[i].i < numVertices);
		DPVS_API_ASSERT(triangles[i].j >= 0 && triangles[i].j < numVertices);
		DPVS_API_ASSERT(triangles[i].k >= 0 && triangles[i].k < numVertices);
	}

#endif // DPVS_DEBUG

	//------------------------------------------------------------------------
	// Perform the splitting
	//------------------------------------------------------------------------

	ImpObjectSplitter splitter(vertices,triangles,triMaterials,numVertices,numTriangles,sceneScale, minObjectSize,targetObjects);
	splitter.split();

	//------------------------------------------------------------------------
	// Copy the object triangle IDs to the output objectIDs[] array
	//------------------------------------------------------------------------

	int outObjects = splitter.assignTriangleIDs(objectIDs);
	return outObjects;
}

struct SplitSortInfo
{
	int			m_origIndex;			// index to original triangle
	UINT32		m_sortValue;			// sort value

	bool		operator> (const SplitSortInfo& s) const	{ return m_sortValue > s.m_sortValue; }
	bool		operator< (const SplitSortInfo& s) const	{ return m_sortValue < s.m_sortValue; }
};

// note that there's a ref count of 1 for each of the output models
int	ObjectSplitter::splitEx	(MeshModel** models, const Vector3* vertices, const Vector3i* triangles, const UINT32* triMaterials, int numVertices,int numTriangles, bool clockwise, const Vector3& sceneScale, float minObjectSize, int targetObjects)
{
	DPVS_ASSERT(models);

	TempArray<UINT32> ids(numTriangles);				// triangle IDs
	int nObs = splitEx(&ids[0],vertices,triangles, triMaterials, numVertices, numTriangles, sceneScale, minObjectSize, targetObjects);

	TempArray<SplitSortInfo>	sortInfo(numTriangles);
	TempArray<Vector3i>			tmpTris(numTriangles);

	for (int i = 0; i < numTriangles; i++)
	{
		sortInfo[i].m_origIndex = i;
		sortInfo[i].m_sortValue = ids[i];
	}

	quickSort (&sortInfo[0], numTriangles);

	int outObs			= 0;
	int oldIndex		= 0;
	int i				= 1;

	// create models and assign triangles into them. The MeshModel ctor
	// will get rid of the extra vertices...

	while (i <= numTriangles)
	{
		while (i < numTriangles && sortInfo[i-1].m_sortValue == sortInfo[i].m_sortValue)
			i++;

		// i now points to a new object.. time to create the old one...
		int nTris = i - oldIndex;
		for (int j = 0; j < nTris; j++)
			tmpTris[j] = triangles[sortInfo[oldIndex + j].m_origIndex];	// copy triangles

		models[outObs] = MeshModel::create(vertices,&tmpTris[0],numVertices,nTris,clockwise);
		outObs++;

		oldIndex = i;
		i++;
	}

	DPVS_ASSERT(outObs == nObs);		// WTF????????
	return nObs;
}

namespace DPVS
{
int	ObjectSplitter::split (UINT32 objectIDs[], const Vector3* vertices, const Vector3i* triangles, const UINT32* triMaterials, int numVertices,int numTriangles, float minObjectSize)
{
	return splitEx(objectIDs,vertices,triangles,triMaterials,numVertices,numTriangles,Vector3(0.0f,0.0f,0.0f),minObjectSize,0);
}
}

//------------------------------------------------------------------------
