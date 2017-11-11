
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
 * Description: 	3D Spatial database implementation
 *
 * $Archive: /dpvs/implementation/sources/dpvsDatabase.cpp $
 * $Revision: #6 $
 * $Modtime: 2.10.02 14:14 $
 * $Date: 2003/11/18 $
 *
 ******************************************************************************/

#include "dpvsDatabase.hpp"
#include "dpvsImpObject.hpp"
#include "dpvsIntersect.hpp"
#include "dpvsRandom.hpp"
#include "dpvsSort.hpp"
#include "dpvsClipPolygon.hpp"
#include "dpvsSet.hpp"
#include "dpvsSystem.hpp"
#include "dpvsDebug.hpp"
#include "dpvsVisibilityQuery.hpp"
#include "dpvsTempArray.hpp"
#include "dpvsPriorityQueue.hpp"
#include "dpvsEvaluation.hpp"
#include "dpvsMesh.hpp"
#if defined (DPVS_DEBUG)
#	include "dpvsMemory.hpp"
#endif

//--------------------------------------------------------------------
// Don't inline the functions in debug build (as this would make
// debugging somewhat more difficult)
//--------------------------------------------------------------------

#if defined (DPVS_DEBUG)
#	define DB_INLINE
#else
#	define DB_INLINE DPVS_FORCE_INLINE
#endif

namespace DPVS
{

//--------------------------------------------------------------------
// Some constants that control creation & traversal of the database
//--------------------------------------------------------------------

static const float NODE_BASE_COST			= ImpObject::NODE_BASE_COST;		// assumed cost of processing a node
static const float MAX_SPLIT_FACTOR			= 1.2f;			// maximum object instance replication factor when splitting a node
static const int   MAX_RECURSION_DEPTH		= 96;			// maximum recursion depth (this is used only to survive potential bugs in the code)
static const int   MAX_INSTANCES_PER_OBJECT = 16;			// absolute object splitting limit
static const int   MIN_NODE_INSTANCE_COUNT	= 12;			// minimum number of instances in node to allow a split

class Node;

/******************************************************************************
 *
 * Struct:			DPVS::ObInstance
 *
 * Description:		Object instance class
 *
 * Notes:			Each time an object is instanced in a node, a matching
 *					ObInstance is created. The ObInstance is linked into two
 *					doubly-linked lists (node-specific and object-specific)
 *					and contains pointer back to ImpObject and Node.
 *
 ******************************************************************************/

struct ObInstance
{
public:
					ObInstance	(void) {}
	ImpObject*		m_ob;									// pointer back to Ob structure
	ObInstance*		m_vNext;								// next instance in same node
	ObInstance*		m_vPrev;								// prev instance in same node
	Node*			m_node;									// pointer back to node
	ObInstance*		m_obNext;								// next instance of same object (in different node)
	ObInstance*		m_obPrev;								// previous instance of same object (in different node)
private:
					ObInstance		(const ObInstance&);	// not allowed
	ObInstance&		operator=		(const ObInstance&);	// not allowed
};


/******************************************************************************
 *
 * Class:			DPVS::Node
 *
 * Description:		Node class (non-hashed standard tree implementation)
 *
 * Notes:			The size of the node has been very carefully determined.
 *					It is currently 96-8 == 88 bytes so that all nodes
 *					are cache line aligned and occupy 3 cache lines.
 *
 ******************************************************************************/

class Node
{
public:

	enum
	{
		AXIS_X			= 0,										// x-splitting axis
		AXIS_Y			= 1,										// y-splitting axis
		AXIS_Z			= 2,										// z-splitting axis
		LEFT			= 0,										// left child
		RIGHT			= 1,										// right child
		LEFT_MASK		= (1<<LEFT),								// left child mask
		RIGHT_MASK		= (1<<RIGHT),								// right child mask
		ALL_MASK		= LEFT_MASK | RIGHT_MASK,					// all children mask
		MAX_CHILDREN	= 2											// number of children
	};

	//--------------------------------------------------------------------
	// first cache line
	//--------------------------------------------------------------------

	unsigned char		m_lastOcclusionTestVisible:1;				// did last occlusion test find the node itself visible? (SHOULD HAVE A 32-BIT MASK)
	unsigned char		m_lastVisible:1;							// was last query visible? (SHOULD HAVE A 32-BIT MASK)
	unsigned char		m_costDirty:1;								// is the node's cost dirty?
	unsigned char		m_accumulatedCostDirty:1;					// is the node's accumulated cost dirty? (i.e. children that have new costs)
	unsigned char		m_dirty:1;									// does the node need updating?
	unsigned char		m_childMask:2;								// child mask (1 bit for each child)
	unsigned char		m_parentOffset:1;							// relative offset to parent [0,MAX_CHILDREN[
	unsigned char		m_anyObjectsProcessed:1;					// true if any objects were processed during last query
	unsigned char		m_splitAxis:2;								// splitting axis (AXIS_X,AXIS_Y,AXIS_Z)
	unsigned char		m_tabooCounter;								// taboo counter
	unsigned char		m_dummy;									// dummy counter

	Node*				m_children[MAX_CHILDREN];					// child pointers
	ObInstance*			m_firstInstance;							// first object instance in node
	ImpObject**			m_cachedObs;								// cached ob list (valid if non-null)
	int					m_numInstances;								// number of instances in node
	float				m_splitValue;								// splitting plane value
	AABB*				m_tightBounds;								// tight bounds (used by leaf nodes only)

	//--------------------------------------------------------------------
	// second cache line
	//--------------------------------------------------------------------

	Node*				m_parent;									// parent node
	float				m_cost;										// cost of the node's objects and cost of all child nodes (used by occlusion buffer's benefit calculations)
	float				m_accumulatedCost;							// accumulated cost
	Vector3				m_visiblePoint;								// visible point of the node
	Node*				m_updatePrev;								// dirty linked list
	Node*				m_updateNext;

	//--------------------------------------------------------------------
	// third cache line - 8 bytes
	//--------------------------------------------------------------------

	AABB				m_bounds;									// XYZ bounds (24 bytes) -- is this truly needed?

	//--------------------------------------------------------------------
	// Public API
	//--------------------------------------------------------------------

	bool				isLeaf							(void) const			{ return !m_childMask;					}
	bool				isRoot							(void) const			{ return !m_parent;						}
	bool				isDirty							(void) const			{ return m_dirty;						}

	void				addCost							(float f);
	void				calculateTightBounds			(AABB& dst) const;
	const AABB&			getAABB							(void) const			{ return m_bounds;							}
	float				getAccumulatedCost				(void);
	Node*				getChild						(int index) const		{ DPVS_ASSERT (index>=0 && index<MAX_CHILDREN); return m_children[index]; }
	unsigned int		getChildMask					(void) const			{ return m_childMask;	}
	int					getDepth						(void) const;
	ObInstance*			getFirstInstance				(void) const			{ return m_firstInstance;				}
	int					getInstanceCount				(void) const			{ return m_numInstances;				}
	bool				getLastVisible					(void) const			{ return m_lastVisible;					}
	Node*				getParent						(void) const			{ return m_parent;	}
	int					getParentOffset					(void) const			{ return m_parentOffset;	}
	int					getSplitAxis					(void) const			{ return m_splitAxis;					}
	float				getSplitValue					(void) const			{ return m_splitValue;	}
	const AABB&			getTestBounds					(void) const			{ return m_tightBounds ? *m_tightBounds: m_bounds;	}
	const Vector3&		getVisiblePoint					(void) const			{ return m_visiblePoint; }
	void				reset							(Node* parent, int parentOffset);
	void				setBounds						(const AABB& aabb)		{ m_bounds = aabb;  }
	void				setFirstInstance				(ObInstance* i)			{ m_firstInstance = i; }
	void				setLastVisible					(bool b)				{ if (b != m_lastVisible) m_lastVisible = b;	}
	void				setLastOcclusionTestVisible		(bool b)				{ if (b != m_lastOcclusionTestVisible) m_lastOcclusionTestVisible = b;	}
	void				setParentAccumulatedCostDirty	(void);
	void				setVisiblePoint					(const Vector3& v)		{ m_visiblePoint = v; }

#if defined (DPVS_DEBUG)
	bool				containsObject					(const ImpObject* ob) const;
#endif
};

/******************************************************************************
 *
 * Struct:			DPVS::TraversalElement
 *
 * Description:		Structure used to hold an object / voxel during spatial
 *					database traversal
 *
 ******************************************************************************/

class TraversalElement
{
private:

	static UINT32 getSquaredBoxNearDistance	(const AABB& a);
	static UINT32 getSquaredBoxFarDistance	(const AABB& a);

	//static bool			isACloser					(const AABB& a, const AABB& b);

	DPVS_FORCE_INLINE const AABB& getTestBounds		(void) const						{ return (m_type == OBJECT) ? m_object->getCellSpaceAABB() : m_node->getTestBounds();		 }

	union
	{
		ImpObject*		m_object;			// pointer to object
		Node*			m_node;				// pointer to node
	};

	UINT32				m_clipMask:31;		// clip mask (31 bits only)
	UINT32				m_type:1;			// Type encoded in one bit
	UINT32				m_sortValue;		// sort value (a positive float stored with integer bitpattern)

public:

	enum Type
	{
		OBJECT = 0,							// element is an object
		NODE   = 1							// element is a node
	};

	DPVS_FORCE_INLINE 				TraversalElement	(void)								{}
	DPVS_FORCE_INLINE 				TraversalElement	(ImpObject* o, UINT32 clipMask)		{ m_object = o; m_type = OBJECT; m_clipMask = clipMask; m_sortValue = getSquaredBoxNearDistance(getTestBounds());	}
	DPVS_FORCE_INLINE 				TraversalElement	(Node* n, UINT32 clipMask)			{ m_node   = n; m_type = NODE; m_clipMask = clipMask;  m_sortValue = getSquaredBoxNearDistance(getTestBounds());	}
	DPVS_FORCE_INLINE UINT32		getClipMask			(void) const						{ return m_clipMask;				}
	DPVS_FORCE_INLINE Node*			getNode				(void) const						{ DPVS_ASSERT(m_type == NODE); return m_node; }
	DPVS_FORCE_INLINE ImpObject*	getObject			(void) const						{ DPVS_ASSERT(m_type == OBJECT); return m_object; }
	DPVS_FORCE_INLINE Type			getType				(void) const						{ return (Type)(m_type);			}
    DPVS_FORCE_INLINE bool			operator<=			(const TraversalElement& s) const
	{
		return m_sortValue <= s.m_sortValue;
//		return isACloser(a,b);
	}
};

UINT32 TraversalElement::getSquaredBoxNearDistance	(const AABB& a)
{
	const Vector3& camLoc = VQData::get().getCameraLocation();

	// DEBUG DEBUG GameCube -- compiler fucks up inlining if using non-branching version
#if 0
	float x0 = Math::max(0.0f, a.getMin().x - camLoc.x);
	float x1 = Math::max(0.0f, camLoc.x - a.getMax().x);
	float x  = Math::max(x0, x1);

	float y0 = Math::max(0.0f, a.getMin().y - camLoc.y);
	float y1 = Math::max(0.0f, camLoc.y - a.getMax().y);
	float y  = Math::max(y0, y1);

	float z0 = Math::max(0.0f, a.getMin().z - camLoc.z);
	float z1 = Math::max(0.0f, camLoc.z - a.getMax().z);
	float z  = Math::max(z0, z1);

	DPVS_DEBUG_CODE
	(
		float xRef = camLoc.x < a.getMin().x ? a.getMin().x - camLoc.x :
				     camLoc.x > a.getMax().x ? camLoc.x - a.getMax().x :
				     0.0f;
		float yRef = camLoc.y < a.getMin().y ? a.getMin().y - camLoc.y :
				     camLoc.y > a.getMax().y ? camLoc.y - a.getMax().y :
				     0.0f;
		float zRef = camLoc.z < a.getMin().z ? a.getMin().z - camLoc.z :
				     camLoc.z > a.getMax().z ? camLoc.z - a.getMax().z :
				     0.0f;

		DPVS_ASSERT(x == xRef && y == yRef && z == zRef);
	)
#else
	float x = camLoc.x < a.getMin().x ? a.getMin().x - camLoc.x :
			  camLoc.x > a.getMax().x ? camLoc.x - a.getMax().x :
			  0.0f;
	float y = camLoc.y < a.getMin().y ? a.getMin().y - camLoc.y :
			  camLoc.y > a.getMax().y ? camLoc.y - a.getMax().y :
			  0.0f;
	float z = camLoc.z < a.getMin().z ? a.getMin().z - camLoc.z :
			  camLoc.z > a.getMax().z ? camLoc.z - a.getMax().z :
			  0.0f;
#endif

	float dist = x*x + y*y + z*z;

	return Math::bitPattern(dist);	// don't take the square root
}

UINT32 TraversalElement::getSquaredBoxFarDistance	(const AABB& a)
{
	const Vector3& camLoc = VQData::get().getCameraLocation();

	float x = camLoc.x < a.getMin().x ? a.getMax().x - camLoc.x :
			  camLoc.x > a.getMax().x ? camLoc.x - a.getMin().x :
			  0.0f;
	float y = camLoc.y < a.getMin().y ? a.getMax().y - camLoc.y :
			  camLoc.y > a.getMax().y ? camLoc.y - a.getMin().y :
			  0.0f;
	float z = camLoc.z < a.getMin().z ? a.getMax().z - camLoc.z :
			  camLoc.z > a.getMax().z ? camLoc.z - a.getMin().z :
			  0.0f;

	float dist = x*x + y*y + z*z;

	return Math::bitPattern(dist);	// don't take the square root
}

/*
bool TraversalElement::isACloser(const AABB& a, const AABB& b)
{
	const Vector3& camLoc = VQData::get().getCameraLocation();

	//--------------------------------------------------------------------
	// First we try to find a separating plane (try all six different
	// planes). If we can find one, the comparison is straighforward:
	// we just test on which side the camera pointer is
	//--------------------------------------------------------------------

	if (a.getMax().x <= b.getMin().x) return camLoc.x < a.getMax().x;
	if (b.getMax().x <= a.getMin().x) return camLoc.x > b.getMax().x;
	if (a.getMax().y <= b.getMin().y) return camLoc.y < a.getMax().y;
	if (b.getMax().y <= a.getMin().y) return camLoc.y > b.getMax().y;
	if (a.getMax().z <= b.getMin().z) return camLoc.z < a.getMax().z;
	if (b.getMax().z <= a.getMin().z) return camLoc.z > b.getMax().z;

	//--------------------------------------------------------------------
	// The AABBs are overlapping in space. Find the farthest distances
	// to each box and return true if dist(a) < dist(b);
	//--------------------------------------------------------------------

	return getSquaredBoxNearDistance(a) <= getSquaredBoxNearDistance(b);
}
*/

/******************************************************************************
 *
 * Class:			DPVS::Database
 *
 * Description:		Database class
 *
 * Notes:			This class maintains a spatial database of objects
 *
 ******************************************************************************/

class Database
{
private:

	enum NodeStatus
	{
		NODE_VISIBLE,		// node is visible
		NODE_HIDDEN,		// node is hidden
		NODE_KILLED			// node was killed (DO NOT USE THE NODE POINTER ANYMORE!!)
	};

	static void						freeTemporaryAllocations			(void);
	static int						getClippedBoxFace		(Vector3* output, const AABB& box, int faceIndex, unsigned int clipMask,bool&);
	static bool						isNodeFrontClipping		(const AABB& a);

	//--------------------------------------------------------------------
	// Database local data
	//--------------------------------------------------------------------

	unsigned int					m_timeStamp;			// traversal timestamp
	int								m_numObs;				// number of object Obs
	Node*							m_nodeRoot;				// root node (world)
	Node*							m_unboundedNode;		// node where we place all unbounded objects..
	int								m_numNodes;				// number of nodes
	Node*							m_updateHead;			// dirty list head
	Node*							m_updateTail;			// dirty list tail
	bool							m_optimizing;			// state variable indicating whether we're performing a background optimization right now

	UINT32							m_traverseFrustumMask;	// initial frustum mask used during traversal
	bool							m_traversing;			// state variable indicating whether we're performing a traversal right now

	//--------------------------------------------------------------------
	// Memory recyclers and other shared data
	//--------------------------------------------------------------------

	static Set<ImpObject*>			s_collectSet;					// recycled set for collecting objects
	static PQueue<TraversalElement>	s_queue;						// priority queue used during traversal
	static const int				s_boxFaceList[6][4];			// AABB face vertex lists
	static int						s_nodeCount;					// total node count of all databases (statistics info)
	static int						s_instanceCount;				// total instance count of all databases (statistics info)
public:
				Database						(void);
				~Database						(void);
	void		addObject						(ImpObject*);
	void		checkConsistency					(void) const;
	void		removeObject					(ImpObject*);
	void		updateObject					(ImpObject*);
	int			getObjectDeltaTimeStamp			(ImpObject*) const;
	bool		traverse						(void);
	void		optimizeMemoryUsage				(void);
	void		optimizeHiddenAreas				(double);
	void		traverseUnboundedNode			(UINT32 initialClipMask);

	static int	getInstanceCount				(void) { return s_instanceCount; }
	static int	getNodeCount					(void) { return s_nodeCount; }
private:
				Database						(const Database&); // not allowed
	Database&	operator=						(const Database&); // not allowed

	bool		areObjectBoundsOK				(const ImpObject*);
	Node*		getRootNode						(void) const							{ return m_nodeRoot;			}
	Node*		createNode						(Node*, int);
	void		deleteNode						(Node*);
	void		deleteNodesRecursive			(Node*);
	void		deleteNodeObInstances			(Node*);
	bool		updateDirtyNode					(Node*);
	void		splitNode						(Node*);
	void		pushObjectsDown					(Node*);
	void		calculateNodeBounds				(AABB& b, const Node* v);
	void		updateRootNode					(void);
	void		expandRootNode					(const AABB& aabb);
	void		collapseRootNode				(void);
	ObInstance*	createInstance					(Node*, ImpObject*);
	void		deleteInstance					(ObInstance*);
	bool		moveInstance					(Node* dest, ObInstance* instance);
	void		splitInstance					(ObInstance* instance);
	void		updateTimeStamp					(void);
	bool		isNodeVisible					(Node* v, unsigned int clipMask);
	int			collectNodeObs					(ImpObject** array, const Node* v) const;
	NodeStatus	traverseNode					(Node*,unsigned int&);
	void		traverseNodes					(UINT32 clipMask);
	bool		fitPlane						(float& bestSplitVal, int& bestAxis, float maxSplitFactor, const AABB& box, const AABB* obs, int N);
	void		collapseNodeChildren			(Node*);
	void		setNodeDirty					(Node*);
	void		unlinkUpdateNode				(Node*);
	void		linkNodeToUpdateTail			(Node*);
	int			getObjectInstanceCount			(const ImpObject* o) const;
	void		collectObjects					(Set<ImpObject*>& set, const Node* v) const;
	void		collectObjectsRecursive			(Set<ImpObject*>& set, const Node* v) const;
	bool		isObjectInViewFrustum			(ImpObject* o, UINT32 frustumMask) const;
	bool		clipObjectFloatingPlane			(ImpObject* o) const;
	void		traverseObject					(ImpObject* ob, UINT32 objectClipMask);
	bool		isSplitValid					(const Node*) const;

	void		addElementToQueue				(const TraversalElement&);
	void		popElementFromQueue				(TraversalElement&);
//	static void	sortInstances					(ObVisInfo*, int);

#if defined (DPVS_DEBUG)
	bool		isNodeValid						(const Node*) const;	// debugging func
	void		checkNodeConsistencyRecursive	(const Node*) const;
#endif
};

//------------------------------------------------------------------------
// List of vertices in each box face. The faces are in order:
// minx,maxx,miny,maxy,minz,maxz
//------------------------------------------------------------------------

const int Database::s_boxFaceList[6][4] =
{
	{2,0,4,6},
	{3,7,5,1},
	{1,5,4,0},
	{6,7,3,2},
	{2,3,1,0},
	{7,6,4,5}
};

//------------------------------------------------------------------------
// Shared recyclers for temporary memory allocations
//------------------------------------------------------------------------

Set<ImpObject*>				Database::s_collectSet;					// recycled set for collecting objects
PQueue<TraversalElement>	Database::s_queue;						// priority queue used during traversal
int							Database::s_instanceCount	= 0;		// total instance count
int							Database::s_nodeCount		= 0;		// total node count

} // DPVS


using namespace DPVS;

/*****************************************************************************
 *
 * Function:		Node::containsObject()
 *
 * Description:		Returns boolean value indicating whether node contains specified object
 *
 * Parameters:		ob = pointer to object
 *
 * Returns:			boolean value
 *
 *****************************************************************************/
#if defined (DPVS_DEBUG)

DB_INLINE bool Node::containsObject (const ImpObject* ob) const
{
	//--------------------------------------------------------------------
	// It is crucial to travel the list from the object's point of view..
	// Startup condition of large databases would otherwise slow down
	// enormously (objects have far less ObInstances than nodes do)..
	//--------------------------------------------------------------------

	DPVS_ASSERT(ob);
	for (const ObInstance* inst = ob->getFirstInstance(); inst; inst = inst->m_obNext)
		if (inst->m_node == this)
			return true;
	return false;
}
#endif
/*****************************************************************************
 *
 * Function:		Node::setParentAccumulatedCostDirty()
 *
 * Description:		Sets the accumulated cost of the parent as "dirty"
 *
 *****************************************************************************/

void Node::setParentAccumulatedCostDirty (void)
{
	if (m_parent && !m_parent->m_accumulatedCostDirty)
	{
		m_parent->m_accumulatedCostDirty = true;
		m_parent->setParentAccumulatedCostDirty();
	}
}

/*****************************************************************************
 *
 * Function:		Node::getDepth()
 *
 * Description:		Returns depth of the node in the hierarchy
 *
 * Returns:			depth of the node
 *
 * Notes:			The value is not cached into the node -- thus the function
 *					call takes some time as it needs to traverse the parent
 *					nodes (don't call it too often!)
 *
 *****************************************************************************/

DB_INLINE int Node::getDepth (void) const
{
	int depth = 0;
	for (const Node* p = getParent(); p; p = p->getParent())
		depth++;
	return depth;
}

/*****************************************************************************
 *
 * Function:		Node::addCost()
 *
 * Description:		Adds cost of the node by sum 'f'
 *
 * Parameters:		f = cost delta
 *
 *****************************************************************************/

void Node::addCost (float f)
{
	if (f != 0.0f)
	{
		m_cost				+= f;
		m_accumulatedCost	+= f;
		setParentAccumulatedCostDirty();
	}
}

// does work
/*****************************************************************************
 *
 * Function:		Node::reset()
 *
 * Description:
 *
 * Parameters:		parent			= pointer to parent
 *					parentOffset	= relative offset to parent (0 = left, 1 = right)
 *
 *****************************************************************************/

DB_INLINE void Node::reset (Node* parent, int parentOffset)
{
	DPVS_ASSERT(this);

	fillByte (this,0,sizeof(Node));

	m_tightBounds				= null;
	m_cachedObs					= null;
	m_lastOcclusionTestVisible	= false;
	m_lastVisible				= false;
	m_dirty						= true;
	m_costDirty					= true;	// could be false
	m_accumulatedCostDirty		= true;
	m_anyObjectsProcessed		= true;
	m_splitAxis					= Node::AXIS_X;
	m_splitValue				= 0.0f;
	m_cost						= 0.0f;
	m_accumulatedCost			= 0.0f;
	m_parent					= parent;
	m_parentOffset				= (unsigned char)(parentOffset);
	m_tabooCounter				= 1;		
	m_updatePrev				= null;
	m_updateNext				= null;

	if (parent)
	{
		DPVS_ASSERT (parent->m_children[parentOffset]==null);	// child already exists
		parent->m_children[parentOffset] = this;
		parent->m_childMask |= (1<<parentOffset);			// assign child mask

		Vector3 mn(parent->m_bounds.getMin());
		Vector3 mx(parent->m_bounds.getMax());

		if (parentOffset == Node::LEFT)
			mx[parent->m_splitAxis] = parent->m_splitValue;
		else
		{
			DPVS_ASSERT (parentOffset == Node::RIGHT);
			mn[parent->m_splitAxis] = parent->m_splitValue;
		}

		m_bounds.setMin(mn);
		m_bounds.setMax(mx);
	} else
		m_tabooCounter = 0;

	m_visiblePoint = m_bounds.getCenter();
}

/*****************************************************************************
 *
 * Function:		Node::calculateTightBounds(AABB&)
 *
 * Description:		Calculates tight bounds for a node
 *
 * Parameters:		dst = reference to AABB where the tight bounds are stored
 *
 *****************************************************************************/

DB_INLINE void Node::calculateTightBounds (AABB& dst) const
{
	if (m_firstInstance)
	{
		dst = m_firstInstance->m_ob->getCellSpaceAABB();
		DPVS_ASSERT(dst.getVolume()>=0);
		for (const ObInstance* instance = m_firstInstance; instance; instance = instance->m_vNext)
		{
			dst.grow (instance->m_ob->getCellSpaceAABB());
			DPVS_ASSERT(dst.getVolume()>=0);
		}
		dst.clamp (m_bounds);								// clamp to real bounds
		DPVS_ASSERT(m_bounds.containsFully(dst));
		DPVS_ASSERT(dst.getVolume()>=0);
	} else
		dst = m_bounds;
}

/*****************************************************************************
 *
 * Function:		DPVS::Node::getAccumulatedCost()
 *
 * Description:		Returns accumulated cost of the node
 *
 * Returns:			accumulated cost of the node
 *
 * Notes:			The accumulated cost is the total cost of the node
 *					and all of its children.
 *
 *****************************************************************************/

float Node::getAccumulatedCost (void)
{
	if (m_accumulatedCostDirty || m_costDirty)
	{
		if (m_costDirty)
		{
			m_costDirty = false;
			m_cost		= NODE_BASE_COST;		// init base cost

			for (const ObInstance* inst = m_firstInstance; inst; inst = inst->m_vNext)
			{
				DPVS_ASSERT (inst->m_ob);
				m_cost += inst->m_ob->getRenderCost();
			}
		}

		m_accumulatedCost		= m_cost;									// init to current cost
		m_accumulatedCostDirty	= false;
		for (int i=0; i < MAX_CHILDREN; i++)
		if (m_children[i])
			m_accumulatedCost += m_children[i]->getAccumulatedCost();
	}

	return m_accumulatedCost;
}

/*****************************************************************************
 *
 * Function:		DPVS::Database::checkConsistency()
 *
 * Description:		Performs internal consistency checks (debug build only)
 *
 *****************************************************************************/

void Database::checkConsistency (void) const
{
#if defined (DPVS_DEBUG)
	DPVS_ASSERT (this);
	DPVS_ASSERT (Memory::isValidPointer(this));
	checkNodeConsistencyRecursive(m_nodeRoot);			// recursive checks for Nodes
	checkNodeConsistencyRecursive(m_unboundedNode);		// recursive checks for Nodes
#endif
}

/*****************************************************************************
 *
 * Function:		DPVS::Database::getObjectInstanceCount()
 *
 * Description:		Returns number of instances of the object
 *
 * Parameters:		o = pointer to object (non-null)
 *
 * Returns:			number of instances
 *
 * Notes:			This function needs to traverse through all of the
 *					instances of the object --> so don't call it too often.
 *
 *****************************************************************************/

DB_INLINE int Database::getObjectInstanceCount (const ImpObject* o) const
{
	DPVS_ASSERT(o);
	int cnt = 0;
	for (const ObInstance* inst = o->getFirstInstance(); inst; inst = inst->m_obNext)
		cnt++;
	return cnt;
}

/*****************************************************************************
 *
 * Function:		Database::addElementToQueue()
 *
 * Description:		Adds a new element to the priority queue
 *
 * Parameters:		e = reference to TraversalElement
 *
 *****************************************************************************/

void Database::addElementToQueue (const TraversalElement& e)
{
	s_queue.insert(e);
}

/*****************************************************************************
 *
 * Function:		Database::popElementFromQueue()
 *
 * Description:		Pops elements from queue
 *
 * Parameters:		e = reference to TraversalElement where result is stored
 *
 *****************************************************************************/

void Database::popElementFromQueue (TraversalElement& e)
{
	s_queue.popMin(e);
}

/*****************************************************************************
 *
 * Function:		DPVS::Database::freeTemporaryAllocations()
 *
 * Description:		Frees memory allocated by the shared recyclers
 *
 * Notes:			This function is called periodically to perform
 *					"garbage collection" of temporary arrays. Also, any time
 *					a database is destructed, the recyclers are freed.
 *
 *****************************************************************************/

void Database::freeTemporaryAllocations (void)
{
	//s_instRecycler.free();
	s_collectSet.removeAll(true);			// minimize memory usage
	s_queue.removeAll();					// free all memory used by priority queue
}

/*****************************************************************************
 *
 * Function:		Database::unlinkUpdateNode()
 *
 * Description:		Unlinks a node from the dirty list
 *
 * Parameters:		node = pointer to node (non-null)
 *
 *****************************************************************************/

DB_INLINE void Database::unlinkUpdateNode (Node* node)
{
	DPVS_ASSERT (node);

	if (node->m_updatePrev)
		node->m_updatePrev->m_updateNext = node->m_updateNext;
	if (node->m_updateNext)
		node->m_updateNext->m_updatePrev = node->m_updatePrev;
	if (node == m_updateHead)
		m_updateHead = node->m_updateNext;
	if (node == m_updateTail)
		m_updateTail = node->m_updatePrev;

	node->m_updatePrev = null;
	node->m_updateNext = null;
}

/*****************************************************************************
 *
 * Function:		Database::linkNodeToUpdateTail()
 *
 * Description:		Unlinks a node from the dirty list
 *
 * Parameters:		node = pointer to node (non-null)
 *
 *****************************************************************************/

void Database::linkNodeToUpdateTail (Node* n)
{
	DPVS_ASSERT(n);

	unlinkUpdateNode(n);

	DPVS_ASSERT(!n->m_updatePrev);
	DPVS_ASSERT(!n->m_updateNext);
	DPVS_ASSERT(m_updateHead != n);
	DPVS_ASSERT(m_updateTail != n);

	n->m_updatePrev = m_updateTail;
	n->m_updateNext = null;
	if (n->m_updatePrev)
		n->m_updatePrev->m_updateNext = n;
	else
	{
		DPVS_ASSERT(m_updateHead==null);
		m_updateHead = n;
	}
	m_updateTail = n;
}

/*****************************************************************************
 *
 * Function:		Database::setNodeDirty()
 *
 * Description:		Marks node as dirty (i.e. requires an update)
 *
 * Parameters:		n = pointer to node (non-null)
 *
 * Notes:			Deletes the object instance pointer cache
 *
 *****************************************************************************/

void Database::setNodeDirty (Node* n)
{
	DPVS_ASSERT(n);

	if (n->m_cachedObs)
	{
		DELETE_ARRAY(n->m_cachedObs);
		n->m_cachedObs = 0;
	}

	if (n->m_tightBounds)
	{
		DELETE(n->m_tightBounds);
		n->m_tightBounds = null;
	}
	n->m_dirty = true;
}

/*****************************************************************************
 *
 * Function:		DPVS::Database::optimizeMemoryUsage()
 *
 * Description:		Optimizes memory usage of the database
 *
 *****************************************************************************/

DB_INLINE void Database::optimizeMemoryUsage (void)
{
	freeTemporaryAllocations();
	// DEBUG DEBUG here perform other memory optimizations as well?
}

/*****************************************************************************
 *
 * Function:		DPVS::Database::optimizeHiddenAreas()
 *
 * Description:		Optimizes dirty areas of the database for 'tim' cycles
 *
 * Parameters:		tim = number of cycles we can spend for the optimization job
 *
 *****************************************************************************/

void Database::optimizeHiddenAreas (double tim)
{
	m_optimizing = true;

	double t = System::getCycleCount();

	if (t > 0.0)												// not supported on systems that don't have high-rez timers..
	{
		int cnt = m_numNodes>>6;								// update at most 1/64 of the database per frame.

		while (m_updateHead && cnt>=0)
		{
/*	if (m_updateHead->getInstanceCount() <= 50)			// don't update if less than 50 objects
				linkNodeToUpdateTail(m_updateHead);
			else
*/
			updateDirtyNode(m_updateHead);

			cnt--;
			if ((System::getCycleCount()-t) >= tim)				// we've exceeded our time limit
				break;
		}
	}

	m_optimizing = false;
}

/*****************************************************************************
 *
 * Function:		Database::isNodeValid(Node*)
 *
 * Description:		Debugging function that makes sure a node contains
 *					(more or less) valid data
 *
 * Parameters:		v = pointer to node
 *
 * Returns:			true if node is ok, false otherwise (DPVS_ASSERT that!)
 *
 *****************************************************************************/

#if defined (DPVS_DEBUG)
bool Database::isNodeValid (const Node* v) const
{
	if (!v)
		return false;
	if (m_numNodes < 0)
		return false;
	if (v->getInstanceCount() < 0)
		return false;
	DPVS_ASSERT(Memory::isValidPointer(v) && "Database::isNodeValid() -- memory allocation failure?");

	// Check all instances in the node
	int cnt = 0;
	for (const ObInstance* o = v->m_firstInstance; o; o = o->m_vNext, cnt++)
	{
		DPVS_ASSERT(Memory::isValidPointer(o));
		DPVS_ASSERT(o->m_ob);
		DPVS_ASSERT(Memory::isValidPointer(o->m_ob->getUserObject()));
		DPVS_ASSERT(o->m_node == v);
	}

	DPVS_ASSERT(v->m_numInstances == cnt);

	int i;
	for (i = 0; i < Node::MAX_CHILDREN; i++)
	{
		if ((v->getChildMask() & (1<<i)) && !(v->getChild(i)))
			return false;
		if (!(v->getChildMask() & (1<<i)) && (v->getChild(i)))
			return false;
	}
	return true;
}

/*****************************************************************************
 *
 * Function:		Database::checkNodeConsistencyRecursive()
 *
 * Description:		Performs recursive consistency check of nodes
 *
 *****************************************************************************/

void Database::checkNodeConsistencyRecursive (const Node* v) const
{
	if (v)
	{
		DPVS_ASSERT (isNodeValid(v) && "Database::checkNodeConsistencyRecusive() -- Node broken!");

		checkNodeConsistencyRecursive(v->getChild(0));
		checkNodeConsistencyRecursive(v->getChild(1));
	}
}

#endif

/*****************************************************************************
 *
 * Function:		Database::createNode(Node*,int)
 *
 * Description:		Creates a node
 *
 * Parameters:		parent			= pointer to parent (null if creating world root)
 *					parentOffset	= which child [0,MAX_CHILDREN[ is the new node?
 *
 * Returns:			pointer to node
 *
 *****************************************************************************/

Node* Database::createNode (Node* parent, int parentOffset)
{
	//--------------------------------------------------------------------
	// Create a new node a link it to the tail of the update list
	//--------------------------------------------------------------------

	DPVS_ASSERT (parentOffset >= 0 && parentOffset < Node::MAX_CHILDREN);	// valid parentOffsets

	Node* node = NEW<Node>();
	DPVS_ASSERT(node);

	node->reset(parent,parentOffset);

	linkNodeToUpdateTail(node);							// add to tail of update list

	if (!parent)
		m_nodeRoot = node;								// becomes root node

	//--------------------------------------------------------------------
	// Update statistics
	//--------------------------------------------------------------------

	m_numNodes++;
	s_nodeCount++;
	DPVS_PROFILE(Statistics::incStatistic(Library::STAT_DATABASENODESINSERTED,1));
	DPVS_ASSERT (isNodeValid(node));							// .. just checking

	return node;
}

/*****************************************************************************
 *
 * Function:		Database::Database()
 *
 * Description:		Constructor
 *
 * Notes:			Creates a single node (the root node)
 *
 *****************************************************************************/

DB_INLINE Database::Database (void)
{
	m_numObs		= 0;
	m_timeStamp		= (g_random.getI()&255)+1;			// initialize to random timestamp
	m_numNodes		= 0;
	m_nodeRoot		= null;
//	m_abort			= false;
	m_updateHead	= null;
	m_updateTail	= null;
	m_optimizing	= false;
	m_traversing	= false;
	m_traverseFrustumMask = 0;

	createNode (null, 0);								// create root node

	// create the extra node for unbounded objects

	m_unboundedNode = NEW<Node>();
	m_unboundedNode->reset(null, 0);
	m_numNodes++;
	s_nodeCount++;

	checkConsistency();
}

/*****************************************************************************
 *
 * Function:		Database::deleteInstance(ObInstance*)
 *
 * Description:		Deletes specified object instance
 *
 * Parameters:		instance = pointer to ObInstance to be deleted (non-null)
 *
 *****************************************************************************/

void Database::deleteInstance (ObInstance* instance)
{
	DPVS_ASSERT (instance);

	Node*			v = instance->m_node;
	ImpObject*		o = instance->m_ob;

	DPVS_ASSERT (o);

	v->addCost (-o->getRenderCost());

	if (instance->m_vPrev)
		instance->m_vPrev->m_vNext = instance->m_vNext;
	else
	{
		DPVS_ASSERT (instance == v->getFirstInstance());
		v->setFirstInstance(instance->m_vNext);
	}
	if (instance->m_vNext)
		instance->m_vNext->m_vPrev = instance->m_vPrev;

	if (instance->m_obPrev)
		instance->m_obPrev->m_obNext = instance->m_obNext;
	else
	{
		DPVS_ASSERT (instance == o->getFirstInstance());
		o->setFirstInstance(instance->m_obNext);
	}
	if (instance->m_obNext)
		instance->m_obNext->m_obPrev = instance->m_obPrev;

	v->m_numInstances--;									// mark node as dirty
	setNodeDirty(v);

	DPVS_ASSERT (v->getInstanceCount() >= 0);
	//m_obInstanceAlloc.free(instance);
	DELETE(instance);
	s_instanceCount--;
	DPVS_ASSERT	(s_instanceCount>=0);
	DPVS_PROFILE	(Statistics::incStatistic(Library::STAT_DATABASEINSTANCESREMOVED,1));
}

/*****************************************************************************
 *
 * Function:		Database::deleteNodeObInstances(Node*)
 *
 * Description:		Deletes all ImpObject instances in a node
 *
 * Parameters:		node = pointer to node
 *
 *****************************************************************************/

DB_INLINE void Database::deleteNodeObInstances (Node* node)
{
	DPVS_ASSERT (node);

	while (node->getFirstInstance())					// kill all instances
		deleteInstance(node->getFirstInstance());

	DPVS_ASSERT (node->getInstanceCount()==0);
}

/*****************************************************************************
 *
 * Function:		Database::deleteNode (Node*)
 *
 * Description:		Deletes specified node
 *
 * Notes:			All node's children should've been deleted prior to
 *					this call
 *
 * Parameters:		node = pointer to node
 *
 * Notes:			All instances in the node are automatically destroyed.
 *
 *****************************************************************************/

void Database::deleteNode (Node* node)
{
	DPVS_ASSERT (node);
	DPVS_ASSERT (isNodeValid(node));						// some extra debugging

	node->setParentAccumulatedCostDirty();					// parents' cost becomes dirty

	m_numNodes--;
	s_nodeCount--;
	DPVS_ASSERT (m_numNodes >= 0 && s_nodeCount >= 0);

	deleteNodeObInstances (node);

	Node* parent = node->getParent();

	if (parent)
	{
		int offset = node->getParentOffset();

		DPVS_ASSERT (node == parent->getChild(offset));
		DPVS_ASSERT (parent->getChildMask() & (1<<offset));
		parent->m_children[offset] = null;
		parent->m_childMask &= ~(1<<offset);

		setNodeDirty(parent);								// mark parent as dirty

	} else
		m_nodeRoot = null;

	DELETE_ARRAY(node->m_cachedObs);						// !!!

	unlinkUpdateNode(node);									// unlink from dirty list

	DELETE (node->m_tightBounds);
	DELETE (node);

	DPVS_PROFILE(Statistics::incStatistic(Library::STAT_DATABASENODESREMOVED,1));
}

// does not work

/*****************************************************************************
 *
 * Function:		Database::getObjectDeltaTimeStamp()
 *
 * Description:		Returns number of queries the object has not been
 *					processed in any way
 *
 * Parameters:		o = pointer to ImpObject
 *
 *****************************************************************************/

DPVS_FORCE_INLINE int Database::getObjectDeltaTimeStamp (ImpObject* ob) const
{
	DPVS_ASSERT (ob);
	if (!ob)										// object not in database
		return 0;
	return m_timeStamp - ob->getTimeStamp();
}

/*****************************************************************************
 *
 * Function:		Database::removeObject(ImpObject*)
 *
 * Description:		Removes object from the database
 *
 * Parameters:		o = pointer to ImpObject
 *
 * Notes:			All instances of the object are automatically destroyed.
 *
 *****************************************************************************/

DB_INLINE void Database::removeObject (ImpObject* ob)
{
	DPVS_ASSERT (ob);
	if (!ob || !ob->getFirstInstance())				// object not in database
		return;

	while (ob->getFirstInstance())					// kill all instances
		deleteInstance(ob->getFirstInstance());

	ob->setTestState(ImpObject::OC_DONE);			// use these to mark the object as deleted (it will be skipped
	ob->setTimeStamp(m_timeStamp);					// during object processing no matter what)

	m_numObs--;
	DPVS_ASSERT	(m_numObs >= 0);
	DPVS_PROFILE	(Statistics::incStatistic(Library::STAT_DATABASEOBSREMOVED,1));
}

/*****************************************************************************
 *
 * Function:		Database::~Database()
 *
 * Description:		Destructor
 *
 * Notes:			The destructor performs full cleanup - i.e. all objects
 *					are destroyed and all nodes are destroyed.
 *
 *****************************************************************************/

DB_INLINE Database::~Database (void)
{
	DPVS_ASSERT (m_numObs == 0);
	checkConsistency();											// execute consistency checks

	deleteNodesRecursive	(m_nodeRoot);
	deleteNode				(m_unboundedNode);					// delete the unbounded node..

	DPVS_ASSERT (!m_traversing);
	DPVS_ASSERT (m_updateHead == null);
	DPVS_ASSERT (m_updateTail == null);

	freeTemporaryAllocations();									// make sure all recyclers are freed
}

/*****************************************************************************
 *
 * Function:		Database::createInstance (Node*,ImpObject*)
 *
 * Description:		Creates instance of an object into specified node
 *
 * Parameters:		v = pointer to node (non-null)
 *					o = pointer to ImpObject (non-null)
 *
 * Returns:			pointer to the object instance
 *
 * Notes:			Automatically sets the node dirty flag.
 *
 *					It is an error to create more than one instance of an ImpObject
 *					to the same node. This error is not detected automatically
 *					in the release build.
 *
 *****************************************************************************/

ObInstance* Database::createInstance (Node* v, ImpObject* o)
{
	DPVS_ASSERT (v && o);

	//--------------------------------------------------------------------
	// In debug build check that a) object overlaps the node b)
	// object isn't already placed into the node
	//--------------------------------------------------------------------

#if defined (DPVS_DEBUG)
	if (v != getRootNode() && v != m_unboundedNode)		// don't check for root/unbounded node overlap (as object may not overlap the root node!)
		DPVS_ASSERT(intersect(o->getCellSpaceAABB(),v->getAABB()));
	
	if (o->isUnbounded())
		DPVS_ASSERT(v == m_unboundedNode);
	else
		DPVS_ASSERT(v != m_unboundedNode);

	DPVS_ASSERT(!v->containsObject(o));
#endif

	ObInstance* instance = NEW<ObInstance>();
	instance->m_node	= v;
	instance->m_ob		= o;
	instance->m_vPrev	= null;
	instance->m_vNext	= v->getFirstInstance();
	if (instance->m_vNext)
		instance->m_vNext->m_vPrev = instance;
	instance->m_obPrev	= null;
	instance->m_obNext	= o->getFirstInstance();
	if (instance->m_obNext)
		instance->m_obNext->m_obPrev = instance;
	o->setFirstInstance(instance);
	v->setFirstInstance(instance);
	v->m_numInstances++;
	setNodeDirty(v);

	v->addCost (o->getRenderCost());

	s_instanceCount++;

	DPVS_PROFILE(Statistics::incStatistic(Library::STAT_DATABASEINSTANCESINSERTED,1));

	return instance;
}

/*****************************************************************************
 *
 * Function:		Database::moveInstance(Node*,ObInstance*)
 *
 * Description:		Assigns existing instance to a different node
 *
 * Parameters:		dest     = pointer to node (non-null)
 *					instance = pointer to ObInstance (non-null)
 *
 * Returns:			true if instance was moved into the node, false otherwise
 *
 * Notes:			This is faster than deleting/re-creating an instance
 *					if instance needs to be assigned only to a single node.
 *
 *					The caller of this function must be sure that the
 *					destination node doesn't already contain an instance
 *					of the ImpObject (see createInstance() for further details)
 *
 *****************************************************************************/

bool Database::moveInstance (Node* dest, ObInstance* instance)
{
	DPVS_ASSERT (dest && instance);
	DPVS_ASSERT (dest != m_unboundedNode);

	Node* v = instance->m_node;							// old node

	DPVS_ASSERT (v != m_unboundedNode);

	if (dest == v)										// assigning to same node again - nothing happens
		return true;

	DPVS_ASSERT (v);
	ImpObject* o = instance->m_ob;

	//--------------------------------------------------------------------
	// In debug build check that object isn't already in the node.
	//--------------------------------------------------------------------

	DPVS_ASSERT(!dest->containsObject(o));

	//--------------------------------------------------------------------
	// Update costs etc.
	//--------------------------------------------------------------------

	if (v->getParent() != dest->getParent())			// if same parent, the cost doesn't change...
	{
		v->addCost		(-o->getRenderCost());
		dest->addCost	(+o->getRenderCost());

	}

	v->m_numInstances--;
	setNodeDirty(v);

	if (instance->m_vPrev)									// remove from source node
		instance->m_vPrev->m_vNext = instance->m_vNext;
	else
	{
		DPVS_ASSERT (instance == v->getFirstInstance());
		v->setFirstInstance(instance->m_vNext);
	}
	if (instance->m_vNext)
		instance->m_vNext->m_vPrev = instance->m_vPrev;

	instance->m_vPrev	= null;
	instance->m_vNext	= dest->getFirstInstance();
	if (instance->m_vNext)
		instance->m_vNext->m_vPrev = instance;
	instance->m_node				= dest;

	dest->setFirstInstance (instance);
	setNodeDirty(dest);
	dest->m_numInstances++;

	DPVS_PROFILE(Statistics::incStatistic(Library::STAT_DATABASEINSTANCESMOVED,1));

	return true;
}

DB_INLINE bool Database::areObjectBoundsOK (const ImpObject* o)
{
	const AABB& aabb = o->getCellSpaceAABB();
	if (!aabb.isOK())
		return false;
	if (!Math::isFinite(aabb.getMin()) ||
		!Math::isFinite(aabb.getMax()))
		return false;
	return true;
}

/*****************************************************************************
 *
 * Function:		Database::addObject()
 *
 * Description:		Inserts object into the database
 *
 * Parameters:		o = pointer to object
 *
 * Notes:			The object may not exist in the database already (this is asserted!)
 *
 *****************************************************************************/

DB_INLINE void Database::addObject (ImpObject* ob)
{
	DPVS_ASSERT (ob);
	DPVS_ASSERT (!ob->getFirstInstance());


	if (ob->isUnbounded())
	{
		DPVS_ASSERT			(m_unboundedNode);
		ob->setStatus		(ImpObject::VISIBLE, 0xFFFFFFFF);
		ob->setTimeStamp	(m_timeStamp);
		ob->setTestState	(ImpObject::VF_DONE);
		createInstance		(m_unboundedNode, ob);							// insert to unbounded node

	} else
	{
		// [wili 18/Nov/03] Added check to protect release builds
		if (!areObjectBoundsOK(ob))
			return;

		ob->setStatus		(ImpObject::HIDDEN, 0xFFFFFFFF);
		ob->setTimeStamp	(m_timeStamp);
		ob->setTestState	(ImpObject::VF_DONE);
		createInstance		(getRootNode(), ob);							// insert to root
	}

	m_numObs++;

	//--------------------------------------------------------------------
	// If object is inserted while a traversal is in progress (!!!), let's
	// place it straight into the process queue. Note that there is a theoretical
	// infinite loop that can be caused if the user repeatedly uses a REPORT_IMMEDIATELY
	// flag to remove and re-insert the _same_ object in the database!!
	//--------------------------------------------------------------------

	if (m_traversing)
	{
		// Fixed [wili 020925] Unbounded objects need to be processed immediately 
		// without adding them to the queue!
		if (ob->isUnbounded())
		{
			VQData::get().newObject(ob);
			ob->setStatus (ImpObject::VISIBLE, VQData::get().getCameraIDMask());
			VQData::get().getVisibilityQuery()->setVisible(m_traverseFrustumMask);
		} else // not unbounded -> just add it to the traversal queue
		{
			addElementToQueue(TraversalElement(ob, m_traverseFrustumMask));		// insert object into priority queue (for visibility tests!)
		}
	}

	DPVS_PROFILE(Statistics::incStatistic(Library::STAT_DATABASEOBSINSERTED,1));
}

/*****************************************************************************
 *
 * Function:		Database::updateObject(ImpObject*)
 *
 * Description:		Updates an object's status in the database
 *
 * Parameters:		ob = pointer to object
 *
 * Notes:			This function is used to internally update an object if it has
 *					moved or if its bounds have changed.
 *
 *****************************************************************************/

DB_INLINE void Database::updateObject (ImpObject* ob)
{
	DPVS_ASSERT (ob);


	DPVS_PROFILE(Statistics::incStatistic(Library::STAT_DATABASEOBSUPDATED,1));

	//--------------------------------------------------------------------
	// Never update unbounded objects...
	//--------------------------------------------------------------------

	if (ob->isUnbounded())	
	{
		DPVS_ASSERT(ob->getFirstInstance());
		DPVS_ASSERT(!ob->getFirstInstance()->m_obNext);		// must have only one instance
		DPVS_ASSERT(ob->getFirstInstance()->m_node == m_unboundedNode);
		return;
	} else
	{
		// database protects itself against broken objects (!)

		bool boundsOK = areObjectBoundsOK(ob);
		
		if (!ob->getFirstInstance())
		{
			if (boundsOK)
				addObject(ob);
			else
				return;
		} else
		{
			if (!boundsOK)
			{
				removeObject(ob);
				return;
			}
		}
	}

	//--------------------------------------------------------------------
	// Find node (by traversing BSP upwards) that could fully
	// enclose the object's AABB. If object doesn't fit anywhere else,
	// we assign it to the root node.
	//--------------------------------------------------------------------

	Node* source	= ob->getFirstInstance()->m_node;
	Node* target	= getRootNode();						// set target to root node (if loop below doesn't find anything better)

	DPVS_ASSERT (source && target);

	for (Node* v = source; v != target; v = v->getParent())	// loop upwards until we can fit the object somewhere
	if (v->getAABB().containsFully(ob->getCellSpaceAABB()))
	{
		target = v;
		break;
	}

	//--------------------------------------------------------------------
	// Check if the object did not move from the node..
	//--------------------------------------------------------------------

	if (target == source)
	{
		bool ok = true;
		for (ObInstance* instance = ob->getFirstInstance(); instance; instance = instance->m_obNext)
		if (!intersect(instance->m_node->getAABB(),ob->getCellSpaceAABB()))	// object isn't contained in some node any more...
		{
			ok = false;
			break;
		}

		if (ok)
		{
			// If target is a leaf node and uses its tight bounds and the object moves
			// outside the tight bounds, we must set the target cell as dirty.
			if (target->m_tightBounds && !target->isDirty() && !target->m_tightBounds->containsFully(ob->getCellSpaceAABB()))
				setNodeDirty(target);
			return;
		}

		// Mark target as dirty in any case (this takes care of the special case of root->root node
		// motion...

		setNodeDirty(target);
	}

	DPVS_PROFILE	(Statistics::incStatistic(Library::STAT_DATABASEOBSUPDATEPROCESSED,1));
	DPVS_ASSERT	(target);

	//--------------------------------------------------------------------
	// Kill all instances of the Ob except one, which is moved to
	// the target node. This is faster than deleting all instances
	// and creating a single new one.
	//--------------------------------------------------------------------

	ObInstance* instance = ob->getFirstInstance()->m_obNext;	// kill all instances except the first one
	while (instance)
	{
		ObInstance* next = instance->m_obNext;
		deleteInstance (instance);
		instance = next;
	}

	moveInstance (target, ob->getFirstInstance());				// move the first instance to the target
}

/*****************************************************************************
 *
 * Function:		Database::updateTimeStamp()
 *
 * Description:		Updates timestamp of the database
 *
 *****************************************************************************/

DB_INLINE void Database::updateTimeStamp (void)
{
	++m_timeStamp;	// Increase database timestamp counter

	//--------------------------------------------------------------------
	// Free all recycler data occasionally (just to make sure that if huge
	// allocs have been made, they'll get cleaned up).
	//--------------------------------------------------------------------

	if (!(g_random.getI()&511))							// approximately every 512th frame
		freeTemporaryAllocations();						// .. actual frame is selected by random
}

/*****************************************************************************
 *
 * Function:		Database::collapseNodeChildren(Node*)
 *
 * Description:		Destroys all nodes below specified node and
 *					moves their instances to the node
 *
 * Parameters:		v = node whose children will be collapsed (non-null)
 *
 *****************************************************************************/

void Database::collapseNodeChildren (Node* v)
{
	Set<ImpObject*>	set;

	//--------------------------------------------------------------------
	// Create an empty set and let all child nodes recursively insert
	// objects they contain into the set. The set class itself gets
	// rid of duplicates.
	//--------------------------------------------------------------------

	for (int i = 0; i < Node::MAX_CHILDREN; i++)
	{
		Node* child = v->getChild(i);
		if (child)
		{
			collectObjectsRecursive(set,child);
			deleteNodesRecursive			(child);
		}
	}

	//--------------------------------------------------------------------
	// If children had object instances, the set contains now list of
	// objects that must be inserted into the node
	//--------------------------------------------------------------------

	if (!set.isEmpty())
	{
		Set<ImpObject*>::Array obArray(set);				// get linear array
		for (int i = 0; i < obArray.getSize(); i++)			// create instances
			createInstance(v,obArray[i]);
	}
}

/*****************************************************************************
 *
 * Function:		Database::calculateNodeBounds(AABB&,const Node*)
 *
 * Description:		Calculates bounding box of all instances directly inside
 *					a node (not its children)
 *
 * Parameters:		b = reference to destination AABB
 *					v = pointer to node
 *
 * Notes:			The bounds will be undefined if node doesn't contain
 *					any instances, so check the instance count first
 *
 *****************************************************************************/

DPVS_FORCE_INLINE void Database::calculateNodeBounds (AABB& b, const Node* v)
{
    DPVS_ASSERT (v);
	DPVS_ASSERT (v != m_unboundedNode);

    if (!v->getFirstInstance())
        return;

    b = v->getFirstInstance()->m_ob->getCellSpaceAABB();                            // init bounding box to first object
    for (const ObInstance* i = v->getFirstInstance(); i; i = i->m_vNext)
    {
        const AABB& box = i->m_ob->getCellSpaceAABB();
    #if defined (DPVS_DEBUG)
        DPVS_ASSERT(!i->m_ob->isUnbounded());
        DPVS_ASSERT(box.getMin().x < box.getMax().x);
        DPVS_ASSERT(box.getMin().y < box.getMax().y);
        DPVS_ASSERT(box.getMin().z < box.getMax().z);
    #endif
		if (box.isOK())
			b.grow(box);
    }
}

/*****************************************************************************
 *
 * Function:		Database::splitInstance(ObInstance*)
 *
 * Description:		Attempts to split object instance to current node's child
 *					nodes and to destroy the original ObInstance
 *
 * Parameters:		instance = pointer to ObInstance
 *
 *****************************************************************************/

void Database::splitInstance (ObInstance* instance)
{
	DPVS_ASSERT (instance);

	ImpObject*	ob	 = instance->m_ob;

	Node*			v			= instance->m_node;

	DPVS_ASSERT (!v->isLeaf());	// this shouldn't hapeen

	const AABB&		obBounds	= ob->getCellSpaceAABB();
	unsigned int	childMask	= 0;
	float			splitValue	= v->getSplitValue();
	int				splitAxis	= v->getSplitAxis();

	//--------------------------------------------------------------------
	// Determine which sub-nodes the instance will be inserted into
	//--------------------------------------------------------------------

	if (obBounds.getMin()[splitAxis] <= splitValue)
		childMask |= Node::LEFT_MASK;
	if (obBounds.getMax()[splitAxis] >  splitValue)
		childMask |= Node::RIGHT_MASK;

	DPVS_ASSERT(childMask);

	//--------------------------------------------------------------------
	// If instance is split to both sub-nodes, we may want to decide
	// that the split isn't necessarily such a great idea. This decision
	// is made based on object's "width" (i.e. dimension in the split
	// axis direction).
	//--------------------------------------------------------------------

	if (childMask == Node::ALL_MASK)
	{
//		if (obBounds.getAxisLength(splitAxis) >= 1.0f*v->getAABB().getAxisLength(splitAxis))
//			return;

		//--------------------------------------------------------------------
		// If object is assumed to stay static, we perform a more exact
		// mesh vs. AABB test to determine if the object actually overlaps
		// both of the child nodes.
		//--------------------------------------------------------------------

		if (ob->isStatic())
		{
		//	DPVS_ASSERT(ob->intersectCellSpaceAABB(v->getAABB()));

			AABB leftBox  = v->getAABB();
			AABB rightBox = v->getAABB();
			leftBox.setMax (splitAxis,splitValue);
			rightBox.setMin(splitAxis,splitValue);

			if (!ob->intersectCellSpaceAABB(leftBox))
				childMask &= ~Node::LEFT_MASK;
			if (!ob->intersectCellSpaceAABB(rightBox))
				childMask &= ~Node::RIGHT_MASK;

			//--------------------------------------------------------------------
			// There is a special case where a triangle (object) is found to
			// be a part of the parent node, but not a part of either child
			// nodes. This can happen due to extremely nasty floating-point
			// accuracy errors in the triangle vs. box intersection code.
			//--------------------------------------------------------------------

			if (!childMask)		// just a precaution if shit hits the fan somehow
			{
				// this says that ob doesn't intersect the AABB?? WTF??
				if (!ob->intersectCellSpaceAABB(v->getAABB()))
				{
					if (getObjectInstanceCount(ob)>1)
					{
//						Debug::print ("shit shit shit\n");
						deleteInstance(instance);
					}
				}
				return;
			}
		}
	}

	//--------------------------------------------------------------------
	// Avoid crazy splitting of objects (shouldn't happen, but is possible
	// in some cases).
	//--------------------------------------------------------------------

	if (childMask == Node::ALL_MASK && getObjectInstanceCount(ob)>= MAX_INSTANCES_PER_OBJECT)
		return;

	//--------------------------------------------------------------------
	// Now move the existing instance to first child nodes. Also,
	// create a new instance in the case the object was split.
	//--------------------------------------------------------------------

	for (int i = 0; i < Node::MAX_CHILDREN; i++)
	if (childMask & (1<<i))
	{
		Node* c = v->getChild(i);
		if (!c)
			c = createNode(v,i);
		DPVS_ASSERT (c);
		if (instance)										// if original instance still exists (first child)
		{													// .. let's just move it to the child (this saves some time)
			if (moveInstance (c, instance))					// if we managed to move it to the node..
				instance = null;							// .. set instance to null
		} else
			createInstance (c, ob);							// we need to create new instance
	}

	DPVS_ASSERT (!instance);								// make sure the instance got moved
}

/*****************************************************************************
 *
 * Function:		DPVS::Database::fitPlane()
 *
 * Description:		Find optimal splitting plane for objects in a node
 *
 * Parameters:		bestSplitValue  = reference to float where the best split value is stored
 *					bestAxis		= reference to int where the best axis is stored [0,2]
 *					maxSplitFactor	= maximum object increase due to split (i.e. objects falling in both nodes) (1.2 == 20% increase)
 *					box				= parent axis-aligned bounding box (sets absolute min/max values for the planes)
 *					obs				= array of axis-aligned bounding boxes (objects)
 *					N				= number of objects in 'obs' array
 *
 * Returns:			true if a valid split plane was found (bestSplitVal and bestAxis contain valid info),
 *					false otherwise
 *
 * Notes:			The routine tests all different combinations for the
 *					split plane. The algorithm used internally for testing
 *					the "goodness" of a split is to evaluate the surface
 *					areas of the two boxes and to multiply the areas with
 *					the number of objects touching each box. The surface area
 *					corresponds with the probability of hitting an object
 *					with a random ray. The goal is to minimize the total
 *					cost of prob(A)*cost(A) + prob(B)*cost(B).
 *
 *					If the partitioning cannot improve over the existing
 *					solution (i.e. prob(parent)*cost(parent)) by at least
 *					5%, we discard the solution. Also, if all solutions
 *					lead to more than 'maxSplitFactor' increase in objects,
 *					we discard the solutions.
 *
 *****************************************************************************/

/******************************************************************************
 *
 * Struct:			DPVS::PFSortInfo
 *
 * Description:		Internal structure used to hold sort values in the
 *					plane-fitting process
 *
 ******************************************************************************/

struct PFSortInfo
{
	float		m_sortValue;					// sort value
	int			m_side;							// which side (0 = left, 1 = right)

	bool		operator> (const PFSortInfo& s) const	{ return m_sortValue > s.m_sortValue; }
	bool		operator< (const PFSortInfo& s) const	{ return m_sortValue < s.m_sortValue; }
	PFSortInfo&	operator= (const PFSortInfo& s)			{ m_sortValue = s.m_sortValue; m_side = s.m_side; return *this; }
};

bool Database::fitPlane (float& bestSplitVal, int& bestAxis, float maxSplitFactor, const AABB& box, const AABB* obs, int N)
{
	if (N <= 0)															// nothing to do?
		return false;

	//--------------------------------------------------------------------
	// We evaluate for each axis the possible split values (start and
	// end points of boxes)
	//--------------------------------------------------------------------

	bestAxis				= 0;										// initialize best axis to X
	bestSplitVal			= box.getMin()[0];							// initalize best split location

	int							maxObjects	= Math::intFloor(N*maxSplitFactor);		// max objects allowed
	float						bestValue	= N * box.getSurfaceArea()*0.95f;		// 5% win required (!)
	TempArray<PFSortInfo,false>	vals(N*2);
	bool						anyFound	= false;								// any split found?

	for (int axis = 0; axis < 3; axis++)											// evaluate each axis
	{
		//----------------------------------------------------------------
		// Determine absolute left/right limits for the split plane. If we
		// have a degenerate dimension here, skip it (otherwise we may
		// run into float underflows etc.)
		//----------------------------------------------------------------

		if (box.getAxisLength(axis) <= 1e-10f)			// avoid undeflow and whatnot...
			continue;

		float	delta		= box.getAxisLength(axis);
		float	absMin		= box.getMin()[axis] + delta*0.01f;
		float	absMax		= box.getMax()[axis] - delta*0.01f;

		//----------------------------------------------------------------
		// Create sort entries (end and start points of objects in the
		// current axis).
		//----------------------------------------------------------------

		for (int i = 0; i < N; i++)
		{
			vals[i*2+0].m_sortValue = obs[i].getMin()[axis];
			vals[i*2+1].m_sortValue = obs[i].getMax()[axis];
			vals[i*2+0].m_side		= 0;	// start point
			vals[i*2+1].m_side		= 1;	// end point
		}

		// sort the start/end points in ascending order based on m_sortValues
		quickSort (&vals[0], N*2);

		//----------------------------------------------------------------
		// Evaluate through all possible split locations. Here we perform
		// a one-dimensional "sweep&prune" over the set of objects. The
		// "points of interest" are the start/end points of the objects.
		// We calculate the cost of each possible partitioning and choose
		// the one with the smallest cost.
		//----------------------------------------------------------------

		int		leftCount	= 0;	// # of objects in left box
		int		rightCount	= N;	// # of objects in right box
		AABB	leftBox(box);		// initialize left box
		AABB	rightBox(box);		// initialize right box

		for (int i = 0; i < N*2; i++)
		{
			float splitVal = (i==(N*2-1)) ?
				vals[i].m_sortValue :									// last entry
				(vals[i].m_sortValue+vals[i+1].m_sortValue)*0.5f;		// fit between objects...

			if (splitVal > absMin && splitVal < absMax)	// if valid plane
			{
				leftBox.setMax	(axis,splitVal);
				rightBox.setMin	(axis,splitVal);

				if ((leftCount+rightCount) <= maxObjects)	// if valid partitioning
				{
					float value = leftBox.getSurfaceArea() * leftCount +
								  rightBox.getSurfaceArea() * rightCount;
					if (value <= bestValue)		// if best cost so far..
					{
						bestAxis		= axis;
						bestValue		= value;
						bestSplitVal	= splitVal;
						anyFound		= true;
					}
				}
			}

			if (vals[i].m_side == 0)	// this was a start point
				leftCount++;
			else
				rightCount--;			// this was an end point
		}
	}

	return anyFound;					// did we find any valid solutions?
}

/*****************************************************************************
 *
 * Function:		Database::collectObjects()
 *
 * Description:		Collects all object instances in a voxel into a set
 *
 * Parameters:		v = node (may be null)
 *
 *****************************************************************************/

void Database::collectObjects (Set<ImpObject*>& set, const Node* v) const
{
	if (v)
	{
		for (const ObInstance* instance = v->getFirstInstance(); instance; instance = instance->m_vNext)
			set.insert(instance->m_ob);
	}
}

/*****************************************************************************
 *
 * Function:		Database::isSplitValid()
 *
 * Description:		Determines if a node's computed optimal split
 *					position is valid or note
 *
 * Parameters:		n = node (may be null)
 *
 * Returns:			True if valid, false otherwise
 *
 *****************************************************************************/

bool Database::isSplitValid	(const Node* n) const
{
	DPVS_ASSERT (n);

	int			axis	= n->m_splitAxis;
	float		value	= n->m_splitValue;
	const AABB& aabb	= n->getAABB();

	if (!Math::isFinite(value))
		return false;

	if ((value >= aabb.getMin()[axis] && value <  aabb.getMax()[axis]) ||
		(value >  aabb.getMin()[axis] && value <= aabb.getMax()[axis]))
		return true;

	return false;
}

/**************************************************************************
 *
 * Function:		Database::splitNode(Node*)
 *
 * Description:		Attempts to split node
 *
 * Parameters:		v = pointer to node (non-null)
 *
 * Notes:			If node is not a leaf, nothing is done.
 *
 *****************************************************************************/

void Database::splitNode (Node* v)
{
	//--------------------------------------------------------------------
	// If DPVS_HIERARCHICAL_DATABASE is not defined, never split a node
	//--------------------------------------------------------------------

#if defined (DPVS_HIERARCHICAL_DATABASE)

	DPVS_ASSERT (v && v != m_unboundedNode);

	if (!v->isLeaf() || (v->getInstanceCount()<MIN_NODE_INSTANCE_COUNT))		// not a leaf node or not enough objects (why subdivide then???)
		return;

	//--------------------------------------------------------------------
	// We have a pre-defined maximum recursion depth for the database..
	// Make sure that we don't exceed it. The max recursion depth is
	// used solely to allow the traversal functions to handle stack
	// management more easily...
	//--------------------------------------------------------------------

	int depth = v->getDepth();

	if (depth >= MAX_RECURSION_DEPTH-1)
		return;

	//--------------------------------------------------------------------
	// Don't subdivide nodes that contain infinite ranges (these are
	// all user errors but we must deal with them!).
	//--------------------------------------------------------------------

	if (!Math::isFinite(v->getAABB().getMin()) ||
		!Math::isFinite(v->getAABB().getMax()))
		return;

	//--------------------------------------------------------------------
	// First determine if there's "slack space" in the voxel. In such
	// a case we would generate a dummy child containing all the slack.
	//--------------------------------------------------------------------

	bool fitFound		= false;

	// DEBUG DEBUG the slack detection code below works nicely,
	// but it has not been enabled yet because it hasn't been tested
	// enough.
/*
	if (depth <= 7)								// early on only
	{
		DPVS_ASSERT(v->getFirstInstance());	// must be
		AABB tightBounds;

		calculateNodeBounds(tightBounds,v);

		float bestSlack  = 0.0f;
		int	  bestAxis   = 0;
		float splitValue = 0.0f;

		const AABB& aabb = v->getAABB();

		for (int axis = 0; axis < 3; axis++)
		{
			// test minimum direction slack

			float axisWidth = aabb.getAxisLength(axis);
			if (!axisWidth)
				continue;	// avoid div by zero

			float smin = (tightBounds.getMin()[axis]-aabb.getMin()[axis]) / axisWidth;
			float smax = (aabb.getMax()[axis]-tightBounds.getMax()[axis]) / axisWidth;

			float sl = smin;
			float sv = tightBounds.getMin()[axis];

			if (smax > smin)
			{
				sl = smax;
				sv = tightBounds.getMax()[axis];
			}

			if (sl > bestSlack)
			{
				bestSlack	= sl;
				bestAxis	= axis;
				splitValue	= sv;
			}
		}

		if (bestSlack > 0.15f)
		{
			// go for it man!

//			Debug::print ("found %.3f worth of slack!\n",bestSlack);

			fitFound = true;
			v->m_splitAxis  = (unsigned char)(bestAxis);
			v->m_splitValue = splitValue;
		}
	}
*/

	//--------------------------------------------------------------------
	// We have two separate algorithms here. The exact one is used if the
	// object count is below some threshold (as it needs temporary memory
	// and performs sorting etc). The less exact one is used when the
	// object count is larger than the threshold.
	//--------------------------------------------------------------------

	if (!fitFound)
	{
		static const int	ANALYSIS_LIMIT	= 2000;						// max objects to perform more expensive analysis
		int					numInstances	= v->getInstanceCount();

		if (numInstances <= ANALYSIS_LIMIT)
		{
			// DEBUG DEBUG TODO: take only into account objects that might be pushed downwards

			//--------------------------------------------------------------------
			// Find optimal splitting plane for the objects. Allocate memory
			// for the boxes using a recycler (the memory is garbage collected
			// later).
			//--------------------------------------------------------------------

			TempArray<AABB,false>	boxes(numInstances);
			int						numBoxes	= 0;

			for (ObInstance* inst = v->getFirstInstance(); inst; inst = inst->m_vNext)
			{
				const AABB& aabb = inst->m_ob->getCellSpaceAABB();		

				if (aabb.isOK())
					boxes[numBoxes++] = aabb;							// copy bounding box
			}

				//--------------------------------------------------------------------
			// Optimal splitting plane
			//--------------------------------------------------------------------

			float splitValue;
			int	  splitAxis;

			fitFound  = fitPlane (splitValue, splitAxis, MAX_SPLIT_FACTOR, v->getAABB(), &boxes[0], numBoxes);

			DPVS_ASSERT(splitAxis>=0 && splitAxis<=2);

			v->m_splitAxis  = (unsigned char)(splitAxis);
			v->m_splitValue = splitValue;
		} else
		{
			// DEBUG DEBUG TODO: IMPROVE THIS!

			int			splitAxis  = 0;
			const AABB& aabb       = v->getAABB();

			if (aabb.getAxisLength(1) > aabb.getAxisLength(0))			splitAxis = 1;
			if (aabb.getAxisLength(2) > aabb.getAxisLength(splitAxis))  splitAxis = 2;

			v->m_splitAxis  = (unsigned char)splitAxis;
			v->m_splitValue = (aabb.getMin()[splitAxis]*0.5f+aabb.getMax()[splitAxis]*0.5f);
			fitFound = true;	// ??

			// detect invalid split DEBUG DEBUG TODO: we might want to have some other conditions as
			// well...
			if (v->m_splitValue == aabb.getMin()[splitAxis])
				fitFound = false;
		}
	}

	//--------------------------------------------------------------------
	// If a fit was found, create child nodes (if either one becomes empty
	// as no objects are inserted, it will be self-destructed later during a
	// node traversal).
	//--------------------------------------------------------------------

	if (fitFound && isSplitValid(v))
	{
		DPVS_PROFILE(Statistics::incStatistic(Library::STAT_DATABASENODESPLITS,1));

		DPVS_ASSERT(v->m_splitValue >= v->getAABB().getMin()[v->m_splitAxis] &&
					 v->m_splitValue <= v->getAABB().getMax()[v->m_splitAxis]);

		createNode (v,Node::LEFT);		// create two child nodes
		createNode (v,Node::RIGHT);
	}
	// DEBUG DEBUG TODO: _else set taboo counter??

#endif // DPVS_HIERARCHICAL_DATABASE
}

/*****************************************************************************
 *
 * Function:		Database::pushObjectsDown(Node*)
 *
 * Description:		Attempts to push object instances down the tree
 *
 * Parameters:		v = pointer to node
 *
 * Notes:			If node is a leaf node, nothing is done (the child
 *					nodes must be created first)
 *
 *****************************************************************************/

void Database::pushObjectsDown (Node* v)
{
	DPVS_ASSERT (v);

	if (v->isLeaf())								// don't handle leaf nodes..
		return;

	ObInstance* instance = v->getFirstInstance();
	while (instance)
	{
		ObInstance* next = instance->m_vNext;
		splitInstance(instance);					// .. push it downwards..
		instance = next;
	}
}

/*****************************************************************************
 *
 * Function:		Database::updateDirtyNode(Node*)
 *
 * Description:		Performs updates for a dirty node
 *
 * Parameters:		v = pointer to node (non-null)
 *
 * Returns:			boolean value indicating if node is still valid after
 *					the update operation (true = node valid, false = node
 *					destroyed)
 *
 * Notes:			The updates may involve collapsing (deletion) of the
 *					node or splitting (insertion of child nodes).
 *
 *****************************************************************************/

bool Database::updateDirtyNode (Node *v)
{
	DPVS_ASSERT	(v);
	DPVS_ASSERT (v != m_unboundedNode);

	//----------------------------------------------------------------
	// Leaf node without instances --> we can kill it without the
	// least bit of regret
	//----------------------------------------------------------------

	if (v->isLeaf() && !v->isRoot())
	{
		if (!v->getFirstInstance())
		{
			setNodeDirty(v->getParent());									// mark parent node as dirty
			deleteNode(v);													// kill the node
			return false;													// return false to indicate that node got destroyed (and pointer is not thus valid)
		}
	}

	//----------------------------------------------------------------
	// If node has been marked as taboo, we'll just decrease its
	// taboo counter and return (node stays dirty during next frame
	// as well). Taboos apply only for leaf nodes (!)
	//----------------------------------------------------------------

	if (v->isLeaf() && v->m_tabooCounter > 0)
	{
		v->m_tabooCounter--;
		return true;
	}

	v->m_tabooCounter = 0;

	//----------------------------------------------------------------
	// Make sure the object list gets destroyed (this is done by
	// the setNodeDirty() function).. sometimes this function may
	// get called even if a node isn't dirty (background optimization
	// process, for example).
	//----------------------------------------------------------------

	if (!v->isDirty())
		setNodeDirty(v);

	//----------------------------------------------------------------
	// Update statistics
	//----------------------------------------------------------------

	DPVS_PROFILE	(Statistics::incStatistic(Library::STAT_DATABASENODEDIRTYUPDATES,1));

	//--------------------------------------------------------------------
	// See if this branch of the hierarchy has become useless (there
	// are too few objects below the node). We are careful here to ensure that
	// we never cause repeated create-split cycles.
	//--------------------------------------------------------------------

	if (!v->isLeaf() && v->getInstanceCount() < MIN_NODE_INSTANCE_COUNT)
	{
		bool collapse	= true;													// allow collapsing..

		for (int i = 0; i < 2; i++)												// count number of instances below..
		if (v->getChild(i) && (!v->getChild(i)->isLeaf() || v->getChild(i)->getInstanceCount() >= MIN_NODE_INSTANCE_COUNT))	// cannot collapse
			collapse = false;

		if (collapse)
		{
			s_collectSet.removeAll(false);										// remove everything (but don't release the memory!)

			collectObjects(s_collectSet,v);

			for (int i = 0; i < 2; i++)											// count number of instances below..
				collectObjects(s_collectSet,v->getChild(i));

			if (s_collectSet.getSize() < (MIN_NODE_INSTANCE_COUNT))				// avoid create-split cycle
			{
				collapseNodeChildren(v);										// collapse the children

				DPVS_ASSERT(v->isLeaf());										// make sure node became leaf
				DPVS_ASSERT(v->getInstanceCount() < MIN_NODE_INSTANCE_COUNT);	// huh?
			}
		}
	}

	//----------------------------------------------------------------
	// If node contains too many instances, attempt to split it
	//----------------------------------------------------------------

	if (v->getFirstInstance())
	{
		splitNode		(v);											// attempt to split node (if it's not a leaf)
		pushObjectsDown (v);												// push objects down to children
	}

	//--------------------------------------------------------------------
	// Calculate tight bounding box for the node
	//--------------------------------------------------------------------

	if (v->isLeaf())
	{
		if (!v->m_tightBounds)
			v->m_tightBounds = NEW<AABB>();
		v->calculateTightBounds (*v->m_tightBounds);						// calculate tight bounds....
		DPVS_ASSERT(v->m_bounds.containsFully(*(v->m_tightBounds)));
	} else
	{
		if (v->m_tightBounds)
		{
			DELETE(v->m_tightBounds);
			v->m_tightBounds = null;
		}
	}

	//--------------------------------------------------------------------
	// Initialize a visible point
	//--------------------------------------------------------------------

	v->m_visiblePoint = v->getTestBounds().getCenter();

	//--------------------------------------------------------------------
	// Move node back to update list tail
	//--------------------------------------------------------------------

	linkNodeToUpdateTail(v);

	//--------------------------------------------------------------------
	// Setup taboo counter for the node. This is done to prevent
	// a large number of splitting during one frame.
	//--------------------------------------------------------------------

	int taboo = 0;
	if (taboo > 255)
		taboo = 255;
	v->m_tabooCounter	= (unsigned char)(taboo);
	v->m_dirty			= 0;											// clear dirty flag (must be at the bottom of the func!)
	return true;														// return true to indicate that node is still valid
}

/*****************************************************************************
 *
 * Function:		Database::expandRootNode(const AABB&)
 *
 * Description:		Recursively creates new root nodes until the root
 *					fully contains the AABB
 *
 * Notes:			All instances contained (directly) in the old root(s)
 *					are moved to the new root
 *
 * Parameters:		aabb = axis-aligned bounding box (root must become larger than this)
 *
 *****************************************************************************/

void Database::expandRootNode (const AABB& aabb)
{
	Node* root = getRootNode();

	if (root->getAABB().containsFully(aabb))								// ok, contains fully the AABB. We're done.
		return;

	DPVS_PROFILE(Statistics::incStatistic(Library::STAT_DATABASENODENEWROOTS,1));

	if (root->isLeaf())													// root is still a leaf node..
	{																	// .. we can just expand it to fit all objects in it..
		root->setBounds(aabb);
		return;															// .. and we're done..
	}

	int			index		= 0;										// parent index for old root
	int			axis		= 0;										// splitting axis
	float		splitVal	= 0.0f;										// split value
	AABB		rootBounds	= root->getAABB();							// take copy
	Vector3		mn(rootBounds.getMin());								// minimum range
	Vector3		mx(rootBounds.getMax());								// maximum range
	Vector3		dim(rootBounds.getDimensions());						// dimensions

	if (aabb.getMin().x < mn.x) { index = Node::RIGHT; axis = Node::AXIS_X; splitVal = mn.x; mn.x -= dim.x;} else
	if (aabb.getMin().y < mn.y) { index = Node::RIGHT; axis = Node::AXIS_Y; splitVal = mn.y; mn.y -= dim.y;} else
	if (aabb.getMin().z < mn.z) { index = Node::RIGHT; axis = Node::AXIS_Z; splitVal = mn.z; mn.z -= dim.z;} else
	if (aabb.getMax().x > mx.x) { index = Node::LEFT;  axis = Node::AXIS_X; splitVal = mx.x; mx.x += dim.x;} else
	if (aabb.getMax().y > mx.y) { index = Node::LEFT;  axis = Node::AXIS_Y; splitVal = mx.y; mx.y += dim.y;} else
	if (aabb.getMax().z > mx.z) { index = Node::LEFT;  axis = Node::AXIS_Z; splitVal = mx.z; mx.z += dim.z;} else
	{
		DPVS_ASSERT (false);		// huh? the root contains the AABB...
		return;
	}

	rootBounds.setMin (mn);												// set new minimum / maximum
	rootBounds.setMax (mx);

	//--------------------------------------------------------------------
	// Create new root and move instances from old root to the new one.
	//--------------------------------------------------------------------

	Node* newRoot = createNode(null,0);								// this becomes new root
	newRoot->m_children[index]	= root;
	newRoot->m_childMask	   |= (1<<index);
	newRoot->setBounds(rootBounds);
	newRoot->m_splitAxis		= (unsigned char)(axis);
	newRoot->m_splitValue		= splitVal;
	root->m_parent              = newRoot;
	root->m_parentOffset		= (unsigned char)index;

	while (root->getFirstInstance())									// move instances upwards
	{
		if (!moveInstance (newRoot, root->getFirstInstance()))
			DPVS_ASSERT(false);											// SOMETHING WENT WRONG!
	}

	expandRootNode (aabb);												// recursively expand
}

/*****************************************************************************
 *
 * Function:		Database::collapseRootNode()
 *
 * Description:		Collapses a root node in the case that a root has
 *					only a single child node and root itself doesn't
 *					contain any objects (i.e. all objects in the world would
 *					fit into bounds of the single child). If root has zero or
 *					more than one children or if root contains children,
 *					the function returns silently.
 *
 *****************************************************************************/

void Database::collapseRootNode (void)
{
	Node* oldRoot = getRootNode();

	//--------------------------------------------------------------------
	// Calculate child count of the root node.
	//--------------------------------------------------------------------

	if (oldRoot->getFirstInstance())										// old root has children
		return;

	int childCount = 0;
	if (oldRoot->getChild(0)) childCount++;
	if (oldRoot->getChild(1)) childCount++;

	if (childCount != 1)													// wrong number of children..
		return;

	Node* newRoot = null;

	for (int c = 0; c < Node::MAX_CHILDREN; c++)							// locate the child
	if (oldRoot->getChild(c))
	{
		newRoot = oldRoot->getChild(c);
		break;
	}

	DPVS_ASSERT (newRoot);

	deleteNode (oldRoot);

	newRoot->m_parent = null;
	m_nodeRoot = newRoot;
}

/*****************************************************************************
 *
 * Function:		Database::updateRootNode()
 *
 * Description:		Performs update for root node
 *
 * Notes:			If objects move outside the root node's bounds, we must
 *					create (recursively) new roots until all objects fit
 *					inside the BSP.
 *
 *					This function is called prior to traversal if root
 *					node has been marked as dirty. The dirty flag is not
 *					cleared as the traversal routine itself will continue
 *					to process the root.
 *
 *****************************************************************************/

DB_INLINE void Database::updateRootNode (void)
{
	Node* v = getRootNode();
	DPVS_ASSERT(v != m_unboundedNode);

	if (!v->getFirstInstance())											// try collapsing root node
	{
		collapseRootNode();
		return;
	}

	DPVS_ASSERT (v->getFirstInstance());
	AABB aabb;
	calculateNodeBounds	(aabb, v);										// calculate bound for objects in root node

	if (v->getAABB().getVolume() != 0.0f)								// if volume has been defined...
		aabb.grow (v->getAABB());										// grow by current node boundaries
	expandRootNode			(aabb);										// recursively expand root until all objects fit into it
}

/*****************************************************************************
 *
 * Function:		Database::isNodeFrontClipping()
 *
 * Description:		Returns boolean value indicating if AABB (cell-space) is front-clipping
 *
 * Parameters:		a			= reference to bounding box
 *
 * Returns:			true if node is front-clipping, false otherwise
 *
 *****************************************************************************/

DB_INLINE bool Database::isNodeFrontClipping (const AABB& a)
{
	Vector3			m(a.getCenter());	// center of AABB
	Vector3			d(a.getMax() - m);	// half-diagonal

	const Vector4&	p	= VQData::get().getViewFrustumPlanes()[ImpCamera::FRONT];
	float			NP	= d.x*Math::fabs(p.x)+d.y*Math::fabs(p.y)+d.z*Math::fabs(p.z);
	float			MP	= m.x*p.x+m.y*p.y+m.z*p.z+p.w;

	return ((MP-NP) <= 0.0f);
}

/*****************************************************************************
 *
 * Function:		Database::isNodeVisible ()
 *
 * Description:		Performs occlusion test using the node's silhouette
 *
 * Parameters:		v			= pointer to node (null is allowed)
 *					clipMask	= current node clip mask
 *
 * Returns:			true if node is even partially visible, false otherwise
 *
 * Notes:			If the node is a leaf node, the test is done using
 *					the node's tight bounding box, otherwise the full-size
 *					bounding box is used.
 *
 *****************************************************************************/

bool Database::isNodeVisible (Node* v, unsigned int clipMask)
{
	DPVS_ASSERT(v != m_unboundedNode);
	VisibilityQuery* vq			= VQData::get().getVisibilityQuery();	// PTR to visibility query

	//--------------------------------------------------------------------
	// If occlusion culling is disabled, determine that object is always
	// visible.
	//--------------------------------------------------------------------

	if (!VQData::get().performOcclusionCulling())
		return true;

	//--------------------------------------------------------------------
	// If the node crosses the zero plane, turn off the zero plane and
	// turn the front plane on instead (we must perform the clipping
	// against the rasterizable front plane).
	//--------------------------------------------------------------------

	const AABB&	box	= v->getTestBounds();

	if ((clipMask & (1<<ImpCamera::ZERO)) || isNodeFrontClipping(box))
		clipMask |=  (1<<ImpCamera::FRONT);

	clipMask &= ~(1<<ImpCamera::ZERO);
	clipMask &= ~(1<<ImpCamera::FLOATING);		// DEBUG DEBUG DEBUG WHY NOT???? FIGURE OUT WHY?

	//--------------------------------------------------------------------
	// Construct face visibility mask.
	//--------------------------------------------------------------------

	const Vector3&	loc			= VQData::get().getCameraLocation();
	unsigned int	faceMask	= 0;

	if (loc.x < box.getMin().x) faceMask |= (1<<0);
	if (loc.x > box.getMax().x) faceMask |= (1<<1);
	if (loc.y < box.getMin().y) faceMask |= (1<<2);
	if (loc.y > box.getMax().y) faceMask |= (1<<3);
	if (loc.z < box.getMin().z) faceMask |= (1<<4);
	if (loc.z > box.getMax().z) faceMask |= (1<<5);

	//--------------------------------------------------------------------
	// If camera is inside the box, the node must be visible!
	//--------------------------------------------------------------------

	if (!faceMask)					// camera is inside the box
		return true;

	//--------------------------------------------------------------------
	// Perform visible point tracking (if last occlusion test showed
	// that node was visible).
	//--------------------------------------------------------------------

	bool doVPT = v->m_lastOcclusionTestVisible;

	if (!(g_random.getI()&31))		// 1:32 chance to turn VPT off
		doVPT = false;

	if (doVPT && VQData::get().isPointVisible(v->getVisiblePoint()))
	{
		DPVS_PROFILE(Statistics::incStatistic(Library::STAT_DATABASENODEVPTSUCCEEDED,1));
		return true;
	}

	//--------------------------------------------------------------------
	// Evaluate through each front-facing face of the bounding box.
	// Clip the face against the view frustum and perform occlusion
	// test. Early-exit if any of the faces are visible.
	//--------------------------------------------------------------------

	DPVS_PROFILE(Statistics::incStatistic(Library::STAT_DATABASENODESOCCLUSIONTESTED,1));

	float	cost = v->getAccumulatedCost();			// cost of the node
	int		face = 0;

	while (faceMask)
	{
		if (faceMask&1)								// .. if face is active
		{
			RawVector<Vector3,32>	silhouette;
			bool					frontClipping	= false;
			int						N				= getClippedBoxFace (&silhouette[0], box,face,clipMask,frontClipping);

			//--------------------------------------------------------------------
			// If face is front-clipping after it has been XY-clipped and far-
			// plane clipped, the node is visible (we're inside the node)
			//--------------------------------------------------------------------

			if (frontClipping)
			{
				DPVS_PROFILE	(Statistics::incStatistic(Library::STAT_DATABASENODESFRONTCLIPPING,1));
				if (doVPT)
					DPVS_PROFILE	(Statistics::incStatistic(Library::STAT_DATABASENODEVPTFAILED,1));
				return true;
			}

			DPVS_ASSERT( N >= 0 && N <= 16);		// we ran out of space! WASSS???

			//--------------------------------------------------------------------
			// If output silhouette contains any vertices, perform an occlusion
			// test...
			//--------------------------------------------------------------------

			if (N)
			{
				// DEBUG DEBUG do we want the doVPT thing here?
				if(vq->isSilhouetteVisible (silhouette, N, cost))
				{
					if (doVPT)
						DPVS_PROFILE (Statistics::incStatistic(Library::STAT_DATABASENODEVPTFAILED,1));

					//----------------------------------------------------
					// Update visible point (VPT) of the node
					//----------------------------------------------------

					if (VQData::get().testProperties(VQData::VPT_VALID))
					{
						DPVS_PROFILE (Statistics::incStatistic(Library::STAT_DATABASENODEVPTUPDATED,1));
						v->setVisiblePoint(VQData::get().getScreenToCell().transformDivByW(VQData::get().getVPT()));
					}

					return true;	// face visible - return true
				}
			}
		}

		face++;				// advance face index
		faceMask>>=1;		// remove one bit from face mask
	}

	//--------------------------------------------------------------------
	// None of the front-facing box faces were visible - node must be
	// hidden
	//--------------------------------------------------------------------

	return false;
}

/*****************************************************************************
 *
 * Function:		Database::collectNodeObs()
 *
 * Description:		Collects objects in a node into a linear array
 *
 * Parameters:		array = destination array (must hold at least node->m_instanceCount entries)
 *					v	  = pointer to node
 *
 * Returns:			number of instances collected
 *
 *****************************************************************************/

int Database::collectNodeObs (ImpObject** array, const Node* v) const
{
	int cnt = 0;
	for (ObInstance* instance = v->getFirstInstance(); instance; instance = instance->m_vNext)
	{
		DPVS_ASSERT(getRootNode()->getAABB().containsFully(instance->m_ob->getCellSpaceAABB()));
		array[cnt++] = instance->m_ob;

	}
	DPVS_ASSERT(cnt == v->getInstanceCount());
	return cnt;
}

/*****************************************************************************
 *
 * Function:		Database::clipFloatingPlane()
 *
 * Description:		Does object clip the floating plane?
 *
 * Returns:			true if object is on both sides of the plane
 *
 * Notes:			Note that AABB test should've been made already to see
 *					if the AABB clipped the floating plane
 *
 *****************************************************************************/

bool Database::clipObjectFloatingPlane (ImpObject* o) const
{
	// DEBUG DEBUG TODO: HERE WE SHOULD USE THE OBB INSTEAD!!!!!!!
	DPVS_ASSERT(o);

	VertexArray hv;				// query convex hull vertices from object
	o->getTestMesh(hv);
	if (!hv.m_vertexPositions)		// DOH!
		return true;

	// Note the negation in the plane equation (we want to early exit if any of the vertices
	// are on the _other side_.

	const Matrix4x3& objectToCell = o->getObjectToCell();
	return intersectHullPlane (hv.m_vertexPositions, hv.m_vertexCount, -VQData::get().getViewFrustumPlanes()[ImpCamera::FLOATING], objectToCell);
}

/*****************************************************************************
 *
 * Function:		Database::isObjectInViewFrustum()
 *
 * Description:		Performs a more accurate object vs. view frustum test
 *
 * Parameters:		frustumMask = clip plane activity mask
 *
 * Returns:			true if object is inside the view frustum false otherwise
 *
 *****************************************************************************/

bool Database::isObjectInViewFrustum (ImpObject* o, UINT32 frustumMask) const
{
	DPVS_ASSERT(o);
	DPVS_ASSERT(frustumMask);			// why was this func called??????

	//--------------------------------------------------------------------
	// If visible point tracking is used, perform a test using the test
	// vertex. Note that visible point tracking is not used if the
	// object AABB intersects the "virtual plane".
	//--------------------------------------------------------------------

#if defined (DPVS_VISIBLE_POINT_TRACKING)
	if (!(frustumMask&(1 << ImpCamera::FLOATING)))
	{
		Vector3 t(NO_CONSTRUCTOR);
		if (Math::transformAndDivByW(t,o->getTestVertex(),VQData::get().getCellToScreen()) && intersectPointUnitBox(t))
			return true;
	}
#endif // DPVS_VISIBLE_POINT_TRACKING

	//--------------------------------------------------------------------
	// Perform OBB vs. frustum test
	//--------------------------------------------------------------------

	DPVS_PROFILE(Statistics::incStatistic(Library::STAT_DATABASEOBSVFEXACTTESTED));
	bool inside = intersectOBBFrustum(o->getTestModel()->getOBB(), o->getObjectToCell(), VQData::get().getViewFrustumPlanes(), frustumMask);

#if defined (DPVS_VISIBLE_POINT_TRACKING)
	if (inside)
		o->testVertexFailed();					// object was inside but test vertex wasn't (!)
#endif // DPVS_VISIBLE_POINT_TRACKING

	return inside;
}

/*****************************************************************************
 *
 * Function:		Database::getClippedBoxFace()
 *
 * Description:		Returns clipped polygon of an AABB's face
 *
 * Parameters:		output		= destination vertices (in -1,+1 range) -- must hold enough vertices for full clip output!
 *					box			= axis-aligned box
 *					faceIndex	= face index (0-5)
 *					clipMask	= current clip mask
 *					frontClipping = reference to bool where info is stored if the poly was front-clipping
 *
 * Returns:			number of vertices in output polygon (0 if polygon is completely outside the VF)
 *
 *****************************************************************************/

int Database::getClippedBoxFace (Vector3* output, const AABB& box, int faceIndex, unsigned int clipMask, bool& frontClipping)
{
	frontClipping = false;

	DPVS_ASSERT(output);
	DPVS_ASSERT(faceIndex >= 0 && faceIndex < 6);

	RawVector<Vector3,64> tmp;

	Vector3* d = &tmp[0];
	Vector3* s = &tmp[32];

	//--------------------------------------------------------------------
	// Fetch face vertices (must be in clockwise order!)
	//--------------------------------------------------------------------

	for (int i = 0; i < 4; i++)
	{
		int index = s_boxFaceList[faceIndex][i];
		s[i].x = (index&1) ? box.getMax().x : box.getMin().x;
		s[i].y = (index&2) ? box.getMax().y : box.getMin().y;
		s[i].z = (index&4) ? box.getMax().z : box.getMin().z;
	}
	//--------------------------------------------------------------------
	// Perform clipping. If entire polygon gets clipped away, return.
	// If polygon is still front-clipping after it has been XY-clipped
	// and far-clipped, we must determine the node is visible (set
	// the frontClipping flag!)
	//--------------------------------------------------------------------

	int	vertices = 4;							// number of vertices

	if (clipMask)
	{
		for (int plane = getHighestSetBit(clipMask); plane>=0; plane--)
		{
			if (clipMask & (1<<plane))
			{
				const Vector4& plEq = VQData::get().getViewFrustumPlanes()[plane];

				if (plane == ImpCamera::FRONT)										// front-clipping plane
				{
					DPVS_ASSERT(!(clipMask&(1<<ImpCamera::ZERO)));	// THIS MUST BE THE LAST ACTIVE PLANE!!!!!!!!!!
					for (int i = 0; i < vertices; i++)
					if ((plEq.x*s[i].x+plEq.y*s[i].y+plEq.z*s[i].z+plEq.w) < 0.0f) // still front clipping
					{
						frontClipping = true;						// cannot accept this
						return 0;
					}
					continue;										// not front-clipping so we got rid of the mask
				}

				vertices = clipPolygonPlane(d,s,plEq,vertices);
				DPVS_ASSERT(vertices>=0 && vertices <=32);
				if (!vertices)
					return 0;
				swap(d,s);											// swap pointers
			}
		}

		DPVS_ASSERT(vertices);
	}

	//--------------------------------------------------------------------
	// Transform the clipped vertices and perform division by W
	//--------------------------------------------------------------------

	Math::transformAndDivByW(output,s,vertices,VQData::get().getCellToScreen());

	//--------------------------------------------------------------------
	// Return number of vertices in output
	//--------------------------------------------------------------------

	return vertices;
}

/*****************************************************************************
 *
 * Function:		Database::traverseNode(Node*, unsigned int&, unsigned int&)
 *
 * Description:		Traverses a single node
 *
 * Parameters:		v				= pointer to node (null is not allowed)
 *					frustumMask		= reference to frustum mask (will be modified)
 *
 * Returns:			node visibility status
 *
 * Notes:			This function inserts objects (after VF culling) into
 *					the priority queue
 *
 *****************************************************************************/

Database::NodeStatus Database::traverseNode (Node* v, unsigned int& frustumMask)
{
	DPVS_ASSERT(v);
	DPVS_ASSERT(v != m_unboundedNode);

	//--------------------------------------------------------------------
	// Update some statistics
	//--------------------------------------------------------------------

	DPVS_PROFILE(Statistics::incStatistic(Library::STAT_DATABASENODESTRAVERSED,1));
	if (v->isLeaf())
		DPVS_PROFILE(Statistics::incStatistic(Library::STAT_DATABASELEAFNODESTRAVERSED,1));

	//--------------------------------------------------------------------
	// If non-zero frustum mask (parent node crosses at least one
	// clip plane), perform a view frustum test. If node is not inside
	// the VF, we can return at this point. Otherwise, update the
	// frustum mask so that child nodes can potentially do less work.
	//--------------------------------------------------------------------

	if (frustumMask)
	{
		DPVS_PROFILE	(Statistics::incStatistic(Library::STAT_DATABASENODESVFTESTED,1));

		const AABB& aabb = v->getTestBounds();
		if (!intersectAABBFrustum(aabb, VQData::get().getViewFrustumPlanes(), frustumMask, frustumMask))
		{
			DPVS_PROFILE(Statistics::incStatistic(Library::STAT_DATABASENODESVFCULLED,1));
			return NODE_HIDDEN;
		}
	}

	//--------------------------------------------------------------------
	// If the node is a leaf node and all of its objects have already been
	// processed (this can happen because of objects being in multiple
	// nodes -- it is actual quite common in some kinds of scenes), we
	// can determine the node to be "occlusion culled" without any further
	// testing. If the node does not contain any object we don't do this
	// optimization -- the node will get killed by the updateDirtyNode()
	// function later...
	//--------------------------------------------------------------------

	if (v->isLeaf() && v->getFirstInstance() && !(v->m_anyObjectsProcessed))
	{
		ObInstance* instance = null;
		for (instance = v->getFirstInstance(); instance; instance = instance->m_vNext)
		if (instance->m_ob->getTimeStamp() != m_timeStamp)
			break;							// early exit...

		if(!instance)	// went through whole loop and no objects were to process...so skip the node..
		{
			v->setLastOcclusionTestVisible (false);
			v->m_anyObjectsProcessed = false;
			return NODE_HIDDEN;
		}
	}

	//--------------------------------------------------------------------
	// Perform occlusion test for the node by testing the visibility
	// of its faces. If occlusion subsystem determines that node
	// is hidden, we can return from the function and don't need to
	// traverse objects inside the node or its children.
	//
	// In order to reduce tests we use Jiri Bittner's 'node skipping'
	// algorithm here. Non-leaf nodes that were visible last frame are
	// not tested - instead they're assumed always visible.
	//--------------------------------------------------------------------

	if (v->getFirstInstance() || !v->getLastVisible())
	{
		bool nodeVisible = isNodeVisible (v,frustumMask);

		v->setLastOcclusionTestVisible (nodeVisible);

		if (!nodeVisible)
		{
			v->m_anyObjectsProcessed = false;
			DPVS_PROFILE(Statistics::incStatistic(Library::STAT_DATABASENODESOCCLUSIONCULLED,1));
			return NODE_HIDDEN;
		}
	}
	else
		DPVS_PROFILE(Statistics::incStatistic(Library::STAT_DATABASENODESSKIPPED,1));

	//--------------------------------------------------------------------
	// If node is 'dirty', perform updates. This may cause collapsing
	// the sub-branches or splitting of the node. If the
	// updateDirtyNode() functions returns 'false', the node got
	// destroyed in the process and we must return immediately (node
	// data isn't valid anymore).
	//--------------------------------------------------------------------

	if (v->isDirty() && (!updateDirtyNode (v)))
		return NODE_KILLED;

	//--------------------------------------------------------------------
	// Update statistics
	//--------------------------------------------------------------------

	DPVS_PROFILE(Statistics::incStatistic(Library::STAT_DATABASENODESVISIBLE,1));

	//--------------------------------------------------------------------
	// Traverse all object instances in the node. If an object has
	// already been traversed (i.e. object overlaps multiple nodes),
	// then the object is not re-traversed.
	//--------------------------------------------------------------------

	int N = v->getInstanceCount();

	if (N)															// If node has instances..
	{
		DPVS_ASSERT(N>0);

		if ((Debug::getLineDrawFlags() & Library::LINE_VOXELS))					// draw voxels?
		{
			static const Vector4 color(0.2f,1.0f,0.2f,0.5f);
			VQData::get().drawBox3D (Library::LINE_VOXELS, v->getTestBounds(),color);
		}


		//--------------------------------------------------------------------
		// If we don't have the cached obs array, we must now make a decision
		// whether we are going to update it or not (we always have the
		// option of just creating a temporary copy).
		//--------------------------------------------------------------------

		ImpObject** obs = v->m_cachedObs;
		if (!obs)
		{
			v->m_cachedObs = NEW_ARRAY<ImpObject*>(N);
			obs = v->m_cachedObs;
			collectNodeObs (obs, v);											// collect node objects
		}

		DPVS_ASSERT(obs);

		//--------------------------------------------------------------------
		// Collect a list of objects that intersect (even partially) the view
		// frustum. Objects overlapping multiple nodes that have already been
		// tested during this traversal are not inserted into the list.
		//--------------------------------------------------------------------

		DPVS_PROFILE(Statistics::incStatistic(Library::STAT_DATABASEOBINSTANCESTRAVERSED,N));

		//----------------------------------------------------------------
		// Determine whether view frustum culling is required for the objects
		//----------------------------------------------------------------

		bool anyProcessed = false;

		if (!frustumMask)	// node does not intersect view frustum planes..
		{
			for (int i = 0; i < N; i++)
			{
				ImpObject* o = obs[i];
				if (o->getTimeStamp() != m_timeStamp)
				{
					DPVS_ASSERT(getRootNode()->getAABB().containsFully(o->getCellSpaceAABB()));
					o->setTimeStamp (m_timeStamp);										// update timestamp
					o->setTestState	(ImpObject::VF_DONE);
					anyProcessed = true;
					addElementToQueue(TraversalElement(o, 0));							// insert object
					DPVS_PROFILE(Statistics::incStatistic(Library::STAT_DATABASEOBSTRAVERSED,1));
				}
			}
		} else // node does intersect some view frustum planes..
		{
			UINT32 camIDMask = VQData::get().getCameraIDMask();

			for (int i = 0; i < N; i++)
			{
				ImpObject* o = obs[i];

				if (o->getTimeStamp() != m_timeStamp)
				{
					DPVS_ASSERT(getRootNode()->getAABB().containsFully(o->getCellSpaceAABB()));

					o->setTimeStamp (m_timeStamp);										// update timestamp
					o->setTestState	(ImpObject::OC_DONE);
					anyProcessed = true;

					//----------------------------------------------------------------
					// Update object's time stamp, test vertex and statistics
					//----------------------------------------------------------------

					DPVS_PROFILE(Statistics::incStatistic(Library::STAT_DATABASEOBSTRAVERSED,1));
					unsigned int mask = frustumMask;

					//--------------------------------------------------------
					// If node is clipping, perform a quick VF test using
					// the bounding box of the object.
					//--------------------------------------------------------

					if (mask)
					{
						DPVS_PROFILE(Statistics::incStatistic(Library::STAT_DATABASEOBSVFTESTED,1));

						//--------------------------------------------------------
						// Start by performing a bounding box vs. view frustum
						// test. This test also provides us with "active clip
						// planes" mask we can use to optimize the OBB vs.
						// frustum calculation later.
						//--------------------------------------------------------

						if (intersectAABBFrustum(o->getCellSpaceAABB(), VQData::get().getViewFrustumPlanes(),  mask, mask) == false)
						{
							DPVS_PROFILE(Statistics::incStatistic(Library::STAT_DATABASEOBSVFCULLED,1));
							o->setStatus (ImpObject::HIDDEN, camIDMask);
							continue;										// ...skip to next object
						}

						//--------------------------------------------------------
						// If object still cannot be trivially accepted/rejected,
						// perform a more exact test (using object's OBB)
						// against the view frustum.
						//--------------------------------------------------------

						if (mask)
						{
		#if defined (DPVS_EXACT_OBJECT_VF_CULLING)								// allow exact object-level VF culling?

							// DEBUG DEBUG TODO: maybe we should pass the mask onwards
							// if floating plane was set...

							if (!isObjectInViewFrustum(o,mask))
							{
								DPVS_PROFILE(Statistics::incStatistic(Library::STAT_DATABASEOBSVFEXACTCULLED,1));
								o->setStatus(ImpObject::HIDDEN, camIDMask);
								continue;
							}
		#endif
							//------------------------------------------------------------
							// If the mask still contains the FLOATING flag, we perform
							// a more detailed check to see if any of the convex hull
							// vertices really cross the FLOATING plane.
							//------------------------------------------------------------

							if (mask & (1<<ImpCamera::FLOATING))		// clipping a floating plane?
							{
								if (!clipObjectFloatingPlane(o))
									mask &= ~(1<<ImpCamera::FLOATING);	// remove the floating plane
							}
						}
					}

					addElementToQueue(TraversalElement(o, (UINT32)mask));	// add object to list
					o->setTestState	(ImpObject::VF_DONE);					// occlusion culling not done yet..

				}
			}
		}

		v->m_anyObjectsProcessed = anyProcessed;
	}

	return NODE_VISIBLE;	// indicate that the node is visible and alive
}

/*****************************************************************************
 *
 * Function:		Database::traverseUnboundedNode()
 *
 * Description:		Traverses through all instances in the unbounded node..
 *
 *****************************************************************************/

void Database::traverseUnboundedNode (UINT32 initialClipMask)
{
	DPVS_ASSERT(m_unboundedNode);
	DPVS_ASSERT(!m_unboundedNode->getChild(0));
	DPVS_ASSERT(!m_unboundedNode->getChild(1));

	UINT32				camIDMask	= VQData::get().getCameraIDMask();			// camera ID mask
	VisibilityQuery*	vq			= VQData::get().getVisibilityQuery();		// PTR to visibility query
	int					N			= 0;

	for (ObInstance* instance = m_unboundedNode->getFirstInstance(); instance; instance = instance->m_vNext)
	{
		ImpObject* ob = instance->m_ob;
		DPVS_ASSERT(ob->isUnbounded());
		VQData::get().newObject(ob);
		ob->setStatus	(ImpObject::VISIBLE, camIDMask);	// Record status of the object
		vq->setVisible	(initialClipMask);					// Inform that the object is visible (pass clipmask for further optimization)
		N++;
		// note that we won't debug visualize these objects as they are treated as having infinite size..
	}

	VQData::get().newObject(null);											// set it to null (for debugging purposes)
	DPVS_PROFILE(Statistics::incStatistic(Library::STAT_DATABASEOBSTRAVERSED,N));
	DPVS_PROFILE(Statistics::incStatistic(Library::STAT_DATABASEOBSVISIBLE,N));
}

/*****************************************************************************
 *
 * Function:		drawMesh()
 *
 * Description:		Internal function for drawing a mesh (debug lines)
 *
 * Parameters:		lineType = what debug lines are we using
 *					mesh	 = pointer to mesh (NULL is ok - nothing is drawn in that case)
 *					mtx		 = reference to object->cell transformation matrix
 *
 *****************************************************************************/

static void drawMesh (Library::LineType lineType, const Mesh* mesh, const Matrix4x3& mtx)
{
	if (!mesh)
		return;

	const Vector3i*		tris		= mesh->getTriangles();
	const Vector3*		vertices	= mesh->getVertices();
	int					nTris		= mesh->getTriangleCount();
	Vector4				color;														// what color is used for rendering the lines?

	if (lineType == Library::LINE_OBJECT_TEST_MODEL)
		color.make(0.4f,0.4f,1.0f,0.5f);
	else
		color.make(1.0f,0.4f,0.4f,0.5f);

	for (int i = 0; i < nTris; i++)
	{
		Vector3 a = mtx.transform(vertices[tris[i].i]);								// transform triangle vertices into cell-space
		Vector3 b = mtx.transform(vertices[tris[i].j]);
		Vector3 c = mtx.transform(vertices[tris[i].k]);

		VQData::get().drawLine3D (lineType, a, b, color);
		VQData::get().drawLine3D (lineType, b, c, color);
		VQData::get().drawLine3D (lineType, c, a, color);
	}
}

/*****************************************************************************
 *
 * Function:		Database::traverseObject()
 *
 * Description:		Processes object during visibility query traversal
 *
 * Parameters:		ob				= non-NULL pointer to object
 *					objectClipMask	= object clip mask
 *
 *****************************************************************************/

void Database::traverseObject (ImpObject* ob, UINT32 objectClipMask)
{
	//--------------------------------------------------------------------
	// Here we ensure that the object's visibility parent (if any) has 
	// been processed before the object.
	//--------------------------------------------------------------------

	if (ob->hasVisibilityParent())
	{
		ImpObject* parent = ob->getVisibilityParent();

		DPVS_ASSERT (parent);														// internal state broken?

		if (parent->getTimeStamp() != m_timeStamp)									// if parent has not been processed during this frame
		{
			DPVS_ASSERT(parent->getCell() == ob->getCell());						// object and parent in different databases!
			traverseObject(parent, m_traverseFrustumMask);							// make sure parent has been processed..
		}
	}

	//--------------------------------------------------------------------
	// Object already processed?
	//--------------------------------------------------------------------

	if (ob->getTestState() == ImpObject::OC_DONE && 
		ob->getTimeStamp() == m_timeStamp)											// object already processed (perhaps removed during the query?)
		return;

	DPVS_ASSERT(!ob->isUnbounded());

	//--------------------------------------------------------------------
	// Perform visibility query. Return if object is hidden.
	//--------------------------------------------------------------------

	ob->setTestState(ImpObject::OC_DONE);
	ob->setTimeStamp(m_timeStamp);												// just in case..

	DPVS_ASSERT (!ob->getVisibilityParent() || ob->getVisibilityParent()->getTimeStamp() == m_timeStamp); 

	VQData::get().newObject(ob);

	VisibilityQuery*	vq			= VQData::get().getVisibilityQuery();		// PTR to visibility query
	UINT32				camIDMask	= VQData::get().getCameraIDMask();			// camera ID mask

	if (VQData::get().performOcclusionCulling())
	{
		DPVS_PROFILE (Statistics::incStatistic(Library::STAT_DATABASEOBSOCCLUSIONTESTED,1));

		if (!vq->isObjectVisible(ob->assumeVisible(camIDMask)))					// perform occlusion test..
		{
			DPVS_PROFILE (Statistics::incStatistic(Library::STAT_DATABASEOBSOCCLUSIONCULLED,1));
			ob->setStatus (ImpObject::HIDDEN, camIDMask);
			return;
		}

		vq->applyOccluder	(objectClipMask);									// Inform that the object can be used as an occluder

	} else // no occlusion culling
	{
		if (!vq->isObjectVisible(true))											// perform back-face culling + visibility parent test
		{
			ob->setStatus (ImpObject::HIDDEN, camIDMask);
			return;
		}
	}

	//--------------------------------------------------------------------
	// Mark object as visible
	//--------------------------------------------------------------------

	ob->setStatus	(ImpObject::VISIBLE, camIDMask);							// Record status of the object
	vq->setVisible	(objectClipMask);											// Inform that the object is visible (pass clipmask for further optimization)

	DPVS_PROFILE(Statistics::incStatistic(Library::STAT_DATABASEOBSVISIBLE,1));

	//--------------------------------------------------------------------
	// If user has requested that TBVs or object meshes are visualized, 
    // display them now.
	//--------------------------------------------------------------------

	if (Debug::getLineDrawFlags() & (Library::LINE_OBJECT_BOUNDS|Library::LINE_REGIONS_OF_INFLUENCE|Library::LINE_OBJECT_TEST_MODEL|Library::LINE_OBJECT_WRITE_MODEL))
	{
		if (ob->assumeVisible(camIDMask))									// this effectively checks the status of the visibility query
		{
			if (Debug::getLineDrawFlags() & Library::LINE_OBJECT_TEST_MODEL)
				drawMesh(Library::LINE_OBJECT_TEST_MODEL, ob->getTestModel()->getTestMesh(), ob->getObjectToCell());
			
			if (Debug::getLineDrawFlags() & Library::LINE_OBJECT_WRITE_MODEL)
				drawMesh(Library::LINE_OBJECT_WRITE_MODEL, ob->getWriteModel() ? ob->getWriteModel()->getTestMesh() : null, ob->getObjectToCell());
			
			if (Debug::getLineDrawFlags() & Library::LINE_OBJECT_BOUNDS)
			{
				Vector4 color = (ob->isStatic() ? Vector4(0.4f,0.4f,1.0f,0.8f) : Vector4(1.0f,0.2f,0.2f,0.8f));
				VQData::get().drawBox3D (Library::LINE_OBJECT_BOUNDS, ob->getCellSpaceAABB(),color);
			}
			
			if ((Debug::getLineDrawFlags() & Library::LINE_REGIONS_OF_INFLUENCE) && ob->isRegionOfInfluence())
				VQData::get().drawBox3D (Library::LINE_REGIONS_OF_INFLUENCE, ob->getCellSpaceAABB(),Vector4(1.0f,1.0f,0.2f,0.9f));
		}
	}
}

/*****************************************************************************
 *
 * Function:		Database::traverseNodes()
 *
 * Description:		New database traversal function
 *
 *****************************************************************************/

void Database::traverseNodes (UINT32 initialClipMask)
{
	// insert root node so that we have something to do..

	addElementToQueue(TraversalElement(getRootNode(), initialClipMask));

	//----------------------------------------------------------------
	// Loop as long as we have data in the priority queue (when nodes
	// are processed, they insert objects into the queue)
	//----------------------------------------------------------------

	while (!s_queue.isEmpty())
	{
		TraversalElement e;

		popElementFromQueue(e);													// pop smallest (closest) element from the queue

		//----------------------------------------------------------------
		// Here we either process an object (occlusion testing) or a node
		//----------------------------------------------------------------

		if (e.getType() == TraversalElement::OBJECT)
		{
			traverseObject (e.getObject(), e.getClipMask());					// process the object
		} else  // process a node
		{
			DPVS_ASSERT(e.getType() == TraversalElement::NODE);					// process a node


			Node*			node		= e.getNode();							// get node pointer
			unsigned int	clipMask	= e.getClipMask();						// get clip mask
			NodeStatus		nodeStatus	= traverseNode (node, clipMask);		// perform vquery + occlusion culling

			DPVS_ASSERT (node != m_unboundedNode);

			if (nodeStatus == NODE_KILLED)	// if node was killed by the traversal, we must continue here
				continue;					// as the node pointer will not be valid anymore

			node->setLastVisible (false);	// set visibility status to false

			if (nodeStatus == NODE_VISIBLE)
			{
				if (node->getParent())								// update parent's visibility status
					node->getParent()->setLastVisible(true);

				// DEBUG DEBUG TODO: WE COULD PERFORM THE VF CULLING TEST HERE TO AVOID
				// UNNECESSARY INSERTIONS...

				if (node->getChild(0))
					addElementToQueue(TraversalElement(node->getChild(0), clipMask));
				if (node->getChild(1))
					addElementToQueue(TraversalElement(node->getChild(1), clipMask));
			}
		}
	}

	VQData::get().newObject(null);											// set it to null (for debugging purposes)
}

/*****************************************************************************
 *
 * Function:		Database::traverse()
 *
 * Description:		Begins traversal through the database
 *
 * Parameters:
 *
 * Returns:			boolean value indicating whether traversal (to other databases)
 *					should be performed. By default this is 'true', but in
 *					some cases (some visible object requests abortion) it
 *					may be false. The caller (camera) should check this value
 *					and respect it.
 *
 *****************************************************************************/

DB_INLINE bool Database::traverse ()
{
#if defined (DPVS_EVALUATION)	// evaluation copy code
	static unsigned int foo = DPVS::FLEXLM::getCode(DPVS_FLEXLM_CODE0);
#endif
	//--------------------------------------------------------------------
	// Perform some basic assertions...
	//--------------------------------------------------------------------

	DPVS_ASSERT(getRootNode());
	DPVS_ASSERT(isPowerOfTwo(VQData::get().getCameraIDMask()));	// make sure only one bit is set
	DPVS_ASSERT(!m_optimizing);									// state leak!
	DPVS_ASSERT(!m_traversing);									// state leak!

	//--------------------------------------------------------------------
	// Update timestamp of the database
	//--------------------------------------------------------------------

	DPVS_PROFILE	(Statistics::incStatistic(Library::STAT_DATABASETRAVERSALS,1));
	updateTimeStamp ();

	//--------------------------------------------------------------------
	// If view frustum culling is disabled, initialize frustum mask to
	// zero, otherwise initialize it to the initial frustum mask provided
	// by the traveler.
	//--------------------------------------------------------------------

	unsigned int frustumMask = VQData::get().getInitialFrustumMask();
	if (!VQData::get().performViewFrustumCulling())
		frustumMask = 0;

	frustumMask &= ~(1 << ImpCamera::FRONT);		// clear the zero plane (as it's not used for anything except portals)

	DPVS_ASSERT (!(frustumMask&(1<<31)));			// BIT NUMBER 31 MUST BE CLEAR!! (WE USE IT FOR OUR OWN DEVIOUS PURPOSES)

#if defined (DPVS_EVALUATION)						// evaluation copy check
	if ((foo/119 + 114407) != (DPVS_FLEXLM_ANSWER0/119 + 114407))
		frustumMask >>= 16;
#endif

	m_traverseFrustumMask = frustumMask;			// need to store it

	//--------------------------------------------------------------------
	// Perform traversal. We measure here the time spent by the
	// visibility query - a portion of this time is then spent to
	// perform "future optimizations".
	//--------------------------------------------------------------------

	double optimizationBudget = System::getCycleCount();

	m_traversing = true;
	traverseUnboundedNode	(frustumMask);

	//--------------------------------------------------------------------
	// Does the root node need an update?
	//--------------------------------------------------------------------

	if (getRootNode()->isDirty())
		updateRootNode();

	//--------------------------------------------------------------------
	// Traverse real nodes
	//--------------------------------------------------------------------

	traverseNodes (frustumMask);

	m_traversing = false;

	optimizationBudget = System::getCycleCount() - optimizationBudget;
	optimizationBudget*= 0.025;						// use up to N*100% of the traversal time

	//--------------------------------------------------------------------
	// Perform optimizations of hidden portions of the database
	//--------------------------------------------------------------------

	optimizeHiddenAreas(optimizationBudget);

	//--------------------------------------------------------------------
	// Return result of the query
	//--------------------------------------------------------------------
	return true;
}

//------------------------------------------------------------------------
// IDatabase wrapper code
//------------------------------------------------------------------------

using namespace DPVS;

		IDatabase::IDatabase			(void) : m_database(NEW<Database>()) {}
		IDatabase::~IDatabase			(void)							{ DELETE(m_database);						}
void	IDatabase::addObject			(ImpObject* o)					{ m_database->addObject(o);					}
void	IDatabase::checkConsistency		(void) const					{ m_database->checkConsistency();			}
void	IDatabase::removeObject			(ImpObject* o)					{ m_database->removeObject(o);				}
void	IDatabase::updateObjectBounds	(ImpObject* o)					{ m_database->updateObject(o);				}
int		IDatabase::getInstanceCount		(void)							{ return Database::getInstanceCount(); }
int		IDatabase::getNodeCount			(void)							{ return Database::getNodeCount(); }
int		IDatabase::getInstanceMemoryUsed(void)							{ return Database::getInstanceCount()*sizeof(ObInstance); }
int		IDatabase::getNodeMemoryUsed	(void)							{ return Database::getNodeCount()*sizeof(Node); }
int		IDatabase::getObjectDeltaTimeStamp (ImpObject* o) const			{ return m_database->getObjectDeltaTimeStamp(o); }
bool	IDatabase::traverse				()								{ return m_database->traverse();	}
void    IDatabase::optimize				(Optimization o)
{
	switch (o)
	{
		case OPTIMIZE_MEMORY_USAGE : m_database->optimizeMemoryUsage(); break;
		case OPTIMIZE_HIDDEN_AREAS : m_database->optimizeHiddenAreas(100000); break;
		default:
			DPVS_ASSERT(false); // WASS?
	}
}

//------------------------------------------------------------------------
// We needed to put the recursive functions here to the very end because
// CodeWarrior will barf completely if inlining is allowed and a function
// is recursive (sheeeesh!)
//------------------------------------------------------------------------

#if defined (DPVS_BUILD_CW) && !defined(DPVS_OS_MAC)
#	pragma auto_inline off
#	pragma always_inline off
#endif

/*****************************************************************************
 *
 * Function:		Database::deleteNodesRecursive(Node*)
 *
 * Description:		Deletes recursively node and its children starting from
 *					specified node
 *
 * Parameters:		node = pointer to node where to start deletion (getRootNode() if entire world is deleted)
 *
 *****************************************************************************/

void Database::deleteNodesRecursive (Node* node)
{
	if (!node)
		return;
	if (!node->isLeaf())
	{
		for (int i = 0; i < Node::MAX_CHILDREN; i++)
		if (node->getChild(i))
			deleteNodesRecursive(node->getChild(i));
	}

	deleteNode (node);
}

/*****************************************************************************
 *
 * Function:		Database::collectObjectsRecursive()
 *
 * Description:		Collects all object instances in a voxel and its children into a set
 *
 * Parameters:		v = node (may not be null)
 *
 *****************************************************************************/

void Database::collectObjectsRecursive(Set<ImpObject*>& set, const Node* v) const
{
	DPVS_ASSERT (v);
	collectObjects(set,v);

	for (int i = 0; i < Node::MAX_CHILDREN; i++)
	{
		Node* child = v->getChild(i);
		if (child)
			collectObjectsRecursive(set,child);
	}
}

//------------------------------------------------------------------------
