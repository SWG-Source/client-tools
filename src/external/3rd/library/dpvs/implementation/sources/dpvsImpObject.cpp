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
 * Description:		ImpObject code
 *
 * $Archive: /dpvs/implementation/sources/dpvsImpObject.cpp $
 * $Author: wili $ 
 * $Revision: #5 $
 * $Modtime: 2.10.02 13:36 $
 * $Date: 2003/08/24 $
 * 
 ******************************************************************************/

#include "dpvsImpObject.hpp"
#include "dpvsImpCommander.hpp"
#include "dpvsImpCamera.hpp"
#include "dpvsImpCell.hpp"
#include "dpvsIntersect.hpp"
#include "dpvsDatabase.hpp"
#include "dpvsSilhouette.hpp"
#include "dpvsRandom.hpp"
#include "dpvsSet.hpp"
#include "dpvsBounds.hpp"
#include "dpvsSystem.hpp"
#include "dpvsRange.hpp"
#include "dpvsVQData.hpp"
#include "dpvsDebug.hpp"
#include "dpvsMesh.hpp"		
#include "dpvsScratchpad.hpp"
#include "dpvsMemory.hpp"			// for consistency checks

//#include <cstdio>

using namespace DPVS;

//------------------------------------------------------------------------
// Static variables
//------------------------------------------------------------------------

bool		ImpObject::s_forceSelectAll			= false;		// is this a flash frame?
ImpObject*  ImpObject::s_head					= null;			// head of linked list
int			ImpObject::s_dynamicToStaticWork	= 0;			// number of dynamic->static "work units" used this frame (1 work unit == 1 vertex)

namespace DPVS
{
/*****************************************************************************
 *
 * Class:			DPVS::MatrixCache
 *
 * Description:		Local class for providing a cache for matrices
 *
 * Notes:			The cache is used for storing object*camera matrices
 *					that may otherwise be calculated multipled times per
 *					frame.
 *
 *****************************************************************************/

class MatrixCache
{
public:
	enum 
	{
		CACHE_SIZE = 64								// this matched with the size of the write queue
	};

	struct Entry
	{
		Matrix4x3			m_matrix;
		const ImpObject*	m_owner;
		UINT32				m_timeStamp;
		UINT32				m_padding[2];			// for alignment
	};

	static DPVS_FORCE_INLINE Entry&		getEntry	(int index)			
	{ 
		DPVS_ASSERT(index>=0 && index < CACHE_SIZE); 
		return (reinterpret_cast<Entry*>(Scratchpad::getPtr(Scratchpad::MATRIX_CACHE)))[index];
	}
	static DPVS_FORCE_INLINE void		reset		(void)
	{
		for (int i = 0; i < CACHE_SIZE; i++)
			getEntry(i).m_owner = null;
	}

private:
	MatrixCache		(void);							// not allowed
	~MatrixCache	(void);							// not allowed
};

DPVS_CT_ASSERT(sizeof(MatrixCache::Entry)==64);

//------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------

static const float	g_minPredictionLength				= 0.6f;		// minimum prediction length in seconds
static const float	g_maxAccessedPredictionLength		= 10.0f;	// maximum prediction length in seconds for visible/accessed objects
static const float	g_maxNonAccessedPredictionLength	= 3*60.0f;	// maximum prediction length in seconds for non-visible/non-accessed objects
static const float	g_downPredictionScale				= 0.75f;	// downwards prediction scaling factor
static const float	g_upPredictionScale					= 1.4f;		// upwards prediction scaling factor
static const float	g_timeUntilStatic					= 1.0f;		// time in seconds when object becomes static
static const int	g_maxDynamicToStaticWork			= 10000;	// max # of dynamic->static changes will allow per frame

//------------------------------------------------------------------------
// Some constants that control the occluder selection algorithms
//------------------------------------------------------------------------

static const float	RESPONSIVITY				= 0.5f;			// responsivity value (0,1)
static const int	HISTORY_LENGTH				= 16;			// length of history data

static const float	g_reciprocalTable[HISTORY_LENGTH+1] =
{
	0.0f,
	1.0f / 1,
	1.0f / 2,
	1.0f / 3,
	1.0f / 4,
	1.0f / 5,
	1.0f / 6,
	1.0f / 7,
	1.0f / 8,
	1.0f / 9,
	1.0f / 10,
	1.0f / 11,
	1.0f / 12,
	1.0f / 13,
	1.0f / 14,
	1.0f / 15,
	1.0f / 16
};


} // DPVS

/*****************************************************************************
 *
 * Function:		DPVS::ImpObject::initBenefit()
 *
 * Description:		Called for each object whose visibility is being
 *					evaluated, but only once per frame.
 *
 * Parameters:		timeStamp =
 *
 * Notes:			Evaluates current benefit value using two-term decay (value).
 *					Calculate incremental average of HISTORY_LENGTH previous benefits (average).
 *					The latter is required to prevent good occluders sliding
 *					outside the view frustum become classified as bad.
 *
 *****************************************************************************/

void ImpObject::initBenefit (UINT32 timeStamp)
{
	if(timeStamp!=m_benefitTimeStamp)	//clear only once per resolveVisibility()
	{
		if ((int)m_averageCount < HISTORY_LENGTH)
			m_averageCount++;

		DPVS_ASSERT ((int)(m_averageCount) <= HISTORY_LENGTH);

		float n		= (float)((int)(m_averageCount)-1);
		float oon	= g_reciprocalTable[m_averageCount];		// 1.0 / m_averageCount
		m_average = (m_average*n + m_currentBenefit) * oon;
		if (m_average < 1e-6f)									// avoid underflows
			m_average = 0.0f;

		m_lastFrameBenefit  = m_currentBenefit;
		m_benefitTimeStamp	= timeStamp;
		m_currentBenefit    = 0.f;
	}
}

/*****************************************************************************
 *
 * Function:		DPVS::ImpObject::estimateBenefit()
 *
 * Description:		Estimate benefit of the occluder
 *
 * Parameters:		pixelArea = 
 *
 * Notes:			Currently uses current knowledge and average over given
 *					timeperiod. 
 *
 *****************************************************************************/

DPVS_FORCE_INLINE float ImpObject::estimateBenefit (void) const
{
	return RESPONSIVITY*m_lastFrameBenefit + (1.0f-RESPONSIVITY)*m_average;  // DEBUG DEBUG DEBUG
}

/*****************************************************************************
 *
 * Function:		DPVS::ImpObject::goodOccluder()
 *
 * Description:		Estimate wheter an object is a good occluder or not.
 *
 * Parameters:		pixelArea = 
 *
 *****************************************************************************/

bool ImpObject::goodOccluder (bool enableForceSelect)
{
	//--------------------------------------------------------------------
	// If an object has been hidden for a long time, we have no idea what-
	// soever about its potential occlusion value.. so give it a 
	// "new player" boost. By setting the boost to the average, we give
	// the object an opportunity to become an occluder again during the
	// following few frames (for example when an object just slides into
	// the view frustum, its occlusion power isn't realized during the
	// first frame).
	//--------------------------------------------------------------------

	if( (int)(m_benefitTimeStamp - m_goodOccluderTimeStamp) > 1) 
	{
		m_averageCount	= 1;
		m_average		= 0;
		m_forceSelect	= enableForceSelect;
	}

	m_goodOccluderTimeStamp = m_benefitTimeStamp;
	
	//--------------------------------------------------------------------
	// The idea here is that the probability that we'll use an object
	// as an occluder is the ratio between the estimated benefit and the 
	// estimated cost. If estimated benefit is larger than cost, we'll
	// always determine that the object is a good occluder. Additionally
	// every Nth frame we choose all visible objects as occluders (in order
	// to find out fusion relationships between distinct objects).
	//--------------------------------------------------------------------

	if (m_forceSelect || s_forceSelectAll || estimateBenefit() >= estimateWriteCost() * 0.5f)
	{
		setLastOccluderUsedTimeStamp(m_goodOccluderTimeStamp);
		m_forceSelect = false;
		return true;
	}

	return false;		// we decide not to use the occluder
}

/*****************************************************************************
 *
 * Function:		DPVS::ImpObject::calculateRenderCost()
 *
 * Description:		Estimate rendering cost. This information
 *					is internally used for cost-benefit analysis.
 *
 * Parameters:		vnum = 
 *					tnum = 
 *					complexity = 
 *
 *****************************************************************************/

float ImpObject::calculateRenderCost(int /*vnum*/,int tnum,float complexity)
{
	// vnum is not used any more...
	complexity = Math::max(complexity,1.0f);
	return RENDER_BASE_COST + RENDER_TRIANGLE_COST*tnum*complexity;
}

/*****************************************************************************
 *
 * Function:		DPVS::ImpObject::setRenderCost ()
 *
 * Description:		Inform library of actual rendering cost. This information
 *					is internally used for cost-benefit analysis and should
 *					therefore be set accordingly.
 *
 * Parameters:		vnum = 
 *					tnum = 
 *					complexity = 
 *
 *****************************************************************************/

void ImpObject::setRenderCost (int vnum,int tnum,float complexity)
{
	m_renderCost = ImpObject::calculateRenderCost(vnum,tnum,complexity);
	m_writeCost  = m_renderCost*2;
}

/*****************************************************************************
 *
 * Function:		DPVS::ImpObject::recalculateWriteCost()
 *
 * Description:		
 *
 *****************************************************************************/

void ImpObject::recalculateWriteCost (float newCost)
{
	m_writeCost = m_writeCost * 0.75f + newCost * 0.25f;		// lerp
}

/*****************************************************************************
 *
 * Function:		DPVS::ImpObject::setRenderCost ()
 *
 * Description:		Inform library of actual rendering cost. This information
 *					is internally used for cost-benefit analysis and should
 *					therefore be set accordingly.
 *
 * Parameters:		cost =
 *
 *****************************************************************************/

void ImpObject::setRenderCost(float cost)
{
	m_renderCost = Math::max(cost,(float)RENDER_BASE_COST);
}

/*****************************************************************************
 *
 * Function:		ImpObject::validateTBV()
 *
 * Description:		Makes sure the TBV is valid 
 *
 *****************************************************************************/

DPVS_FORCE_INLINE void ImpObject::validateTBV (void)
{
	m_TBV.validateBounds();		// handle case where the bounds have < 3 dimensions
}

/*****************************************************************************
 *
 * Function:		DPVS::ImpObject::unlinkFromDirtyList()
 *
 * Description:		
 *
 * Parameters:
 *
 *****************************************************************************/

DPVS_FORCE_INLINE void ImpObject::unlinkFromDirtyList (void)
{
	DPVS_ASSERT(m_cell);

	if (m_inDirtyList)
	{
		ImpCell* impCell = getCell()->getImplementation();
		DPVS_ASSERT(impCell);

		if (m_prevDirty)
			m_prevDirty->m_nextDirty = m_nextDirty;
		else
			impCell->m_dirtyHead = m_nextDirty;
		if (m_nextDirty)
			m_nextDirty->m_prevDirty = m_prevDirty;
		else
			impCell->m_dirtyTail = m_prevDirty;

		m_prevDirty		= null;
		m_nextDirty		= null;
		m_inDirtyList	= false;
	}
}


/*****************************************************************************
 *
 * Function:		DPVS::ImpObject::linkToDirtyListHead()
 *
 * Description:		
 *
 * Parameters:
 *
 *****************************************************************************/

// note that unlinkFromDirtyList() must be called first
DPVS_FORCE_INLINE void ImpObject::linkToDirtyListHead (void)
{
	DPVS_ASSERT(m_cell);	// MUST BE!
	DPVS_ASSERT(!m_inDirtyList && m_nextDirty == null && m_prevDirty == null);

	ImpCell* impCell = getCell()->getImplementation();
	DPVS_ASSERT(impCell);

	m_inDirtyList	= true;

	m_nextDirty		= impCell->m_dirtyHead;
	m_prevDirty		= null;
	if (m_nextDirty)
		m_nextDirty->m_prevDirty = this;
	else
		impCell->m_dirtyTail = this;
	impCell->m_dirtyHead = this;

	m_dirtyTimeStamp = DataPasser::getQueryTime();					// check again during next query
	m_static		 = m_firstBounds;								// expect object to be non-static (unless bounds haven't yet been calced)
	m_dirtyTestVertex= true;										// test vertex has become dirty
}

/*****************************************************************************
 *
 * Function:		DPVS::ImpObject::linkToDirtyListTail()
 *
 * Description:		
 *
 * Parameters:
 *
 *****************************************************************************/

// note that unlinkFromDirtyList() must be called first
DPVS_FORCE_INLINE void ImpObject::linkToDirtyListTail (void)
{
	DPVS_ASSERT(m_cell);	// MUST BE!
	DPVS_ASSERT(!m_inDirtyList && m_nextDirty == null && m_prevDirty == null);

	ImpCell* impCell = getCell()->getImplementation();
	DPVS_ASSERT(impCell);

	m_inDirtyList	= true;
	m_prevDirty		= impCell->m_dirtyTail;
	m_nextDirty		= null;
	if (m_prevDirty)
		m_prevDirty->m_nextDirty = this;
	else
		impCell->m_dirtyHead = this;

	impCell->m_dirtyTail = this;

	m_dirtyTimeStamp = DataPasser::getQueryTime() + g_timeUntilStatic;	// check again in N seconds
	m_static		 = true;													// expect object to be static
}

/*****************************************************************************
 *
 * Function:		DPVS::ImpObject::ImpObject()
 *
 * Description:		ImpObject constructor
 *
 *****************************************************************************/

ImpObject::ImpObject(Model* testModel) :
	m_lastStatus(0),
	m_clipMask(0),
	m_hasVisibilityParent(false),
	m_forceSelect(true),
	m_dirtyTestVertex(true),
	m_inDirtyList(false),
	m_firstBounds(true),
	m_static(true),
	m_enabled(true),
	m_selfEnabled(true),
	m_exactDatabaseFit(true),			//?
	m_contributionCulling(true),
	m_informVisible(true),
	m_floatingPortal(false),
	m_informPortalEnter(false),
	m_informPortalExit(false),
	m_informPortalPreExit(false),
	m_abortable(false),
	m_objectToCellUniform(true),
	m_occlusionWritable(false),
	m_userOccluder(true),
	m_unbounded(false),
	m_hasDynamic(false),
	m_reportImmediately(false),
	m_portal(false),
	m_type(TYPE_OBJECT),
	m_averageCount(0),
	m_testModel(null),
	m_renderCost(2*RENDER_BASE_COST),		// ad hoc value
	m_writeCost(2*RENDER_BASE_COST),		// ad hoc value
	m_writeModel(null),
	m_TBV(),
	m_visibilityParent(null),
	m_firstVisibilityChild(null),
	m_hiddenTimeStamp(0),
	m_lastVisibleTime(DataPasser::getQueryTime()),
	m_lastFrameBenefit(0.f),
	m_currentBenefit(0.f),
	m_average(0.f),
	m_benefitTimeStamp(0),
	m_goodOccluderTimeStamp((UINT32)(0)),			
	m_lastOccluderUsedTimeStamp((UINT32)(0)),			
	m_testVertex(),
	m_cell(null),
	m_cellPrev(null),
	m_cellNext(null),
	m_prev(null),
	m_next(null),
	m_firstInstance(null),
	m_prevDirty(null),
	m_nextDirty(null),
	m_dirtyTimeStamp(0.f),
	m_dynamic(null)
{
	//--------------------------------------------------------------------
	// Link to world-linked list
	//--------------------------------------------------------------------

	setTimeStamp(0);
	DPVS_CHECK_ALIGN(&(getObjectToCell()));						// MUST BE ALIGNED!!!
	const_cast<Matrix4x3&>(getObjectToCell()).ident();			// set object->cell matrix to identity

	InstanceCount::incInstanceCount(InstanceCount::OBJECT);

	if (s_head)
		s_head->m_prev = this;
	m_next	= s_head;
	s_head	= this;

//	linkToDirtyListHead		();									// link to head of dirty list
	setTestModel			(testModel);			
	setRenderCost			(m_testModel->getGeometryCost());	// Estimate initial render cost 
	checkConsistency		();
}

/*****************************************************************************
 *
 * Function:		DPVS::ImpObject::unlinkVisibilityChildren()
 *
 * Description:		Unlinks all visibility children
 *
 *****************************************************************************/

void ImpObject::unlinkVisibilityChildren (void)
{
	while (m_firstVisibilityChild)
	{
		VisibilityRelation* r = m_firstVisibilityChild;
		DPVS_ASSERT(r->m_parent == this);
		DPVS_ASSERT(r->m_child);
		r->m_child->setVisibilityParent(null);		// 
	}
}

/*****************************************************************************
 *
 * Function:		DPVS::ImpObject::setVisibilityParent()
 *
 * Description:		
 *
 *****************************************************************************/

void ImpObject::setVisibilityParent	(ImpObject* parent)
{
	DPVS_ASSERT (parent != this);

	//--------------------------------------------------------------------
	// Parent does not change
	//--------------------------------------------------------------------

	if (m_visibilityParent && m_visibilityParent->m_parent == parent)
	{
		DPVS_ASSERT(m_hasVisibilityParent);	// this must be in synch
		return;
	}

	//--------------------------------------------------------------------
	// First remove existing parent relationship
	//--------------------------------------------------------------------
	
	if (m_visibilityParent)
	{
		VisibilityRelation* r = m_visibilityParent;
		DPVS_ASSERT(r->m_parent);
		DPVS_ASSERT(r->m_child == this);

		if (r->m_prev)
			r->m_prev->m_next = r->m_next;
		else
		{
			DPVS_ASSERT( r == (r->m_parent->m_firstVisibilityChild));
			r->m_parent->m_firstVisibilityChild = r->m_next;
		}

		if (r->m_next)
			r->m_next->m_prev = r->m_prev;

		DELETE(r);
		m_visibilityParent		= null;
		m_hasVisibilityParent	= false;
	}

	//--------------------------------------------------------------------
	// If parent == null, just return now (no record is needed)
	//--------------------------------------------------------------------

	if (parent)
	{
		//--------------------------------------------------------------------
		// Create a new visibility parent record
		//--------------------------------------------------------------------

		VisibilityRelation* r = NEW<VisibilityRelation>();
		m_visibilityParent = r;
		m_hasVisibilityParent = true;
		r->m_parent = parent;
		r->m_child  = this;

		r->m_prev   = null;
		r->m_next   = parent->m_firstVisibilityChild;
		if (r->m_next)
			r->m_next->m_prev = r;
		parent->m_firstVisibilityChild = r;
	}

	checkEnableStatusRecursive();		// evaluate enable/disable state
}


/*****************************************************************************
 *
 * Function:		DPVS::ImpObject::~ImpObject()
 *
 * Description:		Implementation object destructor
 *
 *****************************************************************************/

ImpObject::~ImpObject(void)
{
	checkConsistency ();

	//--------------------------------------------------------------------
	// Remove visibility relationships
	//--------------------------------------------------------------------

	if (m_firstVisibilityChild)										// DEBUG DEBUG TODO: change to hasVisibilityChildren()
		unlinkVisibilityChildren();									// unlink from children
	
	if (hasVisibilityParent())
	{
		setVisibilityParent(null);									// unlink from parent
		DPVS_ASSERT(!hasVisibilityParent());						// WASS?
	}

	//--------------------------------------------------------------------
	// Remove deleted object from cell and global linked list
	//--------------------------------------------------------------------

	if (m_cell)
	{
		unlinkFromDirtyList();										
		m_cell->getImplementation()->removeObject(getUserObject());
	}

	m_cell = null;
	
	DPVS_ASSERT(!m_firstInstance);
	DPVS_ASSERT(!m_cellPrev);
	DPVS_ASSERT(!m_cellNext);
	DPVS_ASSERT(!m_prevDirty);
	DPVS_ASSERT(!m_nextDirty);

	if (m_next)
		m_next->m_prev = m_prev;

	if (m_prev)
		m_prev->m_next = m_next;
	else
	{
		DPVS_ASSERT(s_head == this);
		s_head = m_next;
	}

	InstanceCount::decInstanceCount(InstanceCount::OBJECT);
	if (!s_head)
		DPVS_ASSERT(InstanceCount::getInstanceCount(InstanceCount::OBJECT)==0);

	//--------------------------------------------------------------------
	// Release dynamic part of the object
	//--------------------------------------------------------------------

	if (m_hasDynamic)
	{
		DPVS_ASSERT(m_dynamic);						// mismatch!
		DELETE(m_dynamic); 
		m_dynamic		= null;
		m_hasDynamic	= false;
	}
	
	//--------------------------------------------------------------------
	// Decrease ref counts of test/write models
	//--------------------------------------------------------------------

	m_testModel->getUserModel()->release();

	if (m_writeModel)
		m_writeModel->getUserModel()->release();
	
	//--------------------------------------------------------------------
	// Tiny bit of internal error checking
	//--------------------------------------------------------------------

#if defined (DPVS_DEBUG)
	int cnt = InstanceCount::getInstanceCount(InstanceCount::OBJECT);
	if (cnt<=0)
		DPVS_ASSERT(s_head == null);
	if (!s_head)
		DPVS_ASSERT(cnt==0);
#endif
}

/*****************************************************************************
 *
 * Function:		DPVS::ImpObject::checkConsistency()
 *
 * Description:		Performs consistency checks (in debug build) for 
 *					a single object
 *
 *****************************************************************************/

#if defined (DPVS_DEBUG)
void ImpObject::checkConsistency (void) const
{

	//--------------------------------------------------------------------
	// Object itself and global linked list
	//--------------------------------------------------------------------

	DPVS_ASSERT (this && "ImpObject::checkConsistency() - NULL pointer");
	DPVS_ASSERT (getUserObject() && "ImpObject::checkConsistency() - NULL pointer");
	DPVS_ASSERT (Memory::isValidPointer(getUserObject()) && "ImpObject::checkConsistency() -- invalid Object!");
	DPVS_ASSERT (this != m_prev);
	DPVS_ASSERT (this != m_next);

	if (this == s_head)
		DPVS_ASSERT(!m_prev && "ImpObject::checkConsistency() -- world linked list broken");
	else
		DPVS_ASSERT(m_prev && "ImpObject::checkConsistency() -- world linked list broken");

	//--------------------------------------------------------------------
	// Dynamic structs
	//--------------------------------------------------------------------

	if (m_dynamic)
		DPVS_ASSERT (Memory::isValidPointer(m_dynamic) && "ImpObject::checkConsistency() -- broken m_dynamic!");

	//--------------------------------------------------------------------
	// Cell
	//--------------------------------------------------------------------

	if (m_cell)
		DPVS_ASSERT (Memory::isValidPointer(m_cell));

	DPVS_ASSERT (m_cellPrev  != this);
	DPVS_ASSERT (m_cellNext  != this);

	if (m_cellPrev)
		DPVS_ASSERT (Memory::isValidPointer(m_cellPrev->getUserObject()));
	if (m_cellNext)
		DPVS_ASSERT (Memory::isValidPointer(m_cellNext->getUserObject()));

	//--------------------------------------------------------------------
	// ObInstance
	//--------------------------------------------------------------------

	if (m_firstInstance)
		DPVS_ASSERT (Memory::isValidPointer(m_firstInstance));

	//--------------------------------------------------------------------
	// Visibility relations
	//--------------------------------------------------------------------

	if (m_visibilityParent)
		DPVS_ASSERT (Memory::isValidPointer(m_visibilityParent) && "ImpObject::checkConsistency() - broken visibility parent");

	if (m_firstVisibilityChild)
		DPVS_ASSERT (Memory::isValidPointer(m_firstVisibilityChild) && "ImpObject::checkConsistency() - broken visibility parent");

	//--------------------------------------------------------------------
	// Test and write models
	//--------------------------------------------------------------------

	Model* testModel = getUserTestModel();

	DPVS_ASSERT (Memory::isValidPointer(testModel) && "ImpObject::checkConsistency() -- broken testModel!");

	Model* writeModel = getUserWriteModel();

	if (writeModel)
		DPVS_ASSERT (Memory::isValidPointer(writeModel) && "ImpObject::checkConsistency() -- broken writeModel!");

	//--------------------------------------------------------------------
	// Dirty list
	//--------------------------------------------------------------------

	DPVS_ASSERT (m_prevDirty != this);
	DPVS_ASSERT (m_nextDirty != this);

	if (m_prevDirty)
		DPVS_ASSERT (Memory::isValidPointer(m_prevDirty->getUserObject()));

	if (m_nextDirty)
		DPVS_ASSERT (Memory::isValidPointer(m_nextDirty->getUserObject()));
}
#endif

/*****************************************************************************
 *
 * Function:		DPVS::ImpObject::checkConsistencyAll()
 *
 * Description:		Checks consistency of _all_ ImpObjects in the world
 *
 *****************************************************************************/

void ImpObject::checkConsistencyAll (void)
{
	// TODO: loop detection?
#if defined (DPVS_DEBUG)
	int cnt = 0;
	for (const ImpObject* o = s_head; o; o = o->m_next, cnt++)
		o->checkConsistency();
	DPVS_ASSERT(InstanceCount::getInstanceCount(InstanceCount::OBJECT) == cnt);
#endif
}

/*****************************************************************************
 *
 * Function:		DPVS::ImpObject::suggestGarbageCollect()
 *
 * Description:		Static function for collecting objects that haven't
 *					been accessed for the specified time period
 *
 * Parameters: 
 *
 *****************************************************************************/

void ImpObject::suggestGarbageCollect (Commander* cmd, float dt)
{
	DPVS_ASSERT (cmd);

	if (dt < 0.f)														// oops!
		return;

	//--------------------------------------------------------------------
	// Setup local data
	//--------------------------------------------------------------------

	DynamicArray<ImpObject*>	objects;									// dynamic array of object pointers
	int							numObjects	= 0;							// number of objects selected
	float						currentTime = DataPasser::getQueryTime();	// get current time

	//--------------------------------------------------------------------
	// Iterate through all objects in the world and collect then ones
	// that haven't been accessed for the specified period
	//--------------------------------------------------------------------

	for (ImpObject* obj = s_head; obj; obj = obj->m_next)
	{
		float tm = currentTime - obj->getLastVisibleTime();
		if ( tm >= dt)														
			objects.getElement(numObjects++) = obj;						// add object to the list
	}

	//--------------------------------------------------------------------
	// Now that we have the array, we can start reporting to the user
	//--------------------------------------------------------------------

	ImpCommander*					imp	= cmd->getImplementation();
	const DynamicArray<ImpObject*>&	obj	= objects;

	for (int i = 0; i < numObjects; i++)
	{
		imp->setInstanceObject (obj[i]);								// set as current instance object

		#if defined (DPVS_DEBUG)
			UINT32 oldFPUMode = getFPUMode();
		#endif
		
		cmd->command		   (Commander::REMOVAL_SUGGESTED);			// send command
		DPVS_ASSERT(getFPUMode() == oldFPUMode && "FPU mode changed in user callback!");
	}

	imp->setInstanceObject (null);										// set instance object to null to trap potential errors later
}

/*****************************************************************************
 *
 * Function:		DPVS::ImpObject::setTestModel()
 *
 * Description:		Assigns a test model to an object
 *
 * Parameters:		m = pointer to model (non-null)
 *
 *****************************************************************************/

void ImpObject::setTestModel (Model* m)
{ 
	DPVS_ASSERT(m);

	//--------------------------------------------------------------------
	// If object is a portal, perform some extra checks
	//--------------------------------------------------------------------

	if (isPortal())
		DPVS_API_ASSERT(m->test(Model::BACKFACE_CULLABLE) && "Object::setTestModel(): Portal models must be backface cullable");

	//--------------------------------------------------------------------
	// If model actually changes, update reference counts
	//--------------------------------------------------------------------

	ImpModel* imp = m->getImplementation();
	DPVS_ASSERT(imp);
	
	if(m_testModel != imp) 
	{ 
		imp->getUserModel()->addReference();					// inc reference count of new model
		if (m_testModel)
			m_testModel->getUserModel()->release();				// dec reference of old count
		m_testModel = imp;

		setBoundsDirty();										// !!!
		m_static = true;										// assume that this is a one-time change...
	}	

	checkConsistency();
}

/*****************************************************************************
 *
 * Function:		DPVS::ImpObject::setUnbounded()
 *
 * Description:		Makes an object either bounded or unbounded
 *
 * Parameters:		b = if true, set object unbounded
 *
 * Notes:			Portals are _never_ _ever_ unbounded.
 *
 *****************************************************************************/

void ImpObject::setUnbounded (bool b)
{
	checkConsistency();
	if (isUnbounded()==b || isPortal())							// status does not change..
		return;
	
	m_unbounded = b;
	
	if (m_cell)													// remove and add object into the cell (the Database will then handle everything)..
		m_cell->getImplementation()->enableObject(getUserObject(), m_enabled);
	
	setBoundsDirty();
	if (m_enabled)
		m_static = true;

	checkConsistency();
}

/*****************************************************************************
 *
 * Function:		DPVS::ImpObject::updateOcclusionWritable()
 *
 * Description:		Updates the m_occlusionWritable field
 *
 *
 *****************************************************************************/

void ImpObject::updateOcclusionWritable	(void)
{
	if(m_writeModel && m_writeModel->isOcclusionWritable() && m_userOccluder)
		m_occlusionWritable = true;
	else
		m_occlusionWritable = false;
}

/*****************************************************************************
 *
 * Function:		DPVS::ImpObject::setWriteModel()
 *
 * Description:		Assigns a write model to an object
 *
 * Parameters:		m = pointer to write model (may be null)
 *
 *****************************************************************************/

void ImpObject::setWriteModel (Model *m)				
{ 
	checkConsistency();
	ImpModel* imp = m ? m->getImplementation() : null;

	if (m_writeModel != imp)
	{
		//-----------------------------------------------------------
		// Handle reference counting
		//-----------------------------------------------------------

		if (imp)
			imp->getUserModel()->addReference();
		if (m_writeModel)
			m_writeModel->getUserModel()->release();

		m_writeModel = imp;

		//-----------------------------------------------------------
		// Set occlusion writable information (CACHE)
		//-----------------------------------------------------------

		updateOcclusionWritable();
	}
	checkConsistency();
}

/*****************************************************************************
 *
 * Function:		DPVS::ImpObject::setType()
 *
 * Description:		Assigns a write model to an object
 *
 * Parameters:		t = type of the object
 *
 *****************************************************************************/

void ImpObject::setType	(ObjectType t)
{
	m_type		= (UINT32)t;
	m_portal	= (t==TYPE_PHYSICAL_PORTAL || t==TYPE_VIRTUAL_PORTAL);
}

/*****************************************************************************
 *
 * Function:		DPVS::ImpObject::setCell()
 *
 * Description:		Assigns object to specified cell
 *
 * Parameters:		c = pointer to cell (may be null)
 *
 *****************************************************************************/

void ImpObject::setCell (Cell* c)			
{ 
	checkConsistency();
	//--------------------------------------------------------------------
	// Check if cell is the same as the old one..
	//--------------------------------------------------------------------

	if (c == m_cell)								
		return;

	Object* o = getUserObject();
	DPVS_ASSERT(o);
	Cell* oldCell = m_cell;

	if (oldCell)								// make sure object is not in the dirty list of the old cell
		unlinkFromDirtyList();

	m_cell = c;

	//--------------------------------------------------------------------
	// Add to new cell and remove from old one. 
	//--------------------------------------------------------------------

	if (oldCell)
		oldCell->getImplementation()->removeObject(o);	

	DPVS_ASSERT(!m_cellPrev);
	DPVS_ASSERT(!m_cellNext);

	if (m_cell)
	{
		m_cell->getImplementation()->addObject		(o);
		m_cell->getImplementation()->enableObject	(o, m_enabled);
	}

	//--------------------------------------------------------------------
	// Mark cell-space bounds as dirty and set object to 'static'
	//--------------------------------------------------------------------

	setBoundsDirty();							// this will place object to dirty list head
	m_static = true;
	checkConsistency();
}

/*****************************************************************************
 *
 * Function:		DPVS::ImpObject::enableInternal()
 *
 * Description:		Internal routine for enabling/disabling object
 *
 *****************************************************************************/

void ImpObject::enableInternal (bool v)
{
	if (v == (bool)m_enabled)						// state doesn't change
		return;

	checkConsistency();
	//--------------------------------------------------------------------
	// Set new enable status..
	//--------------------------------------------------------------------

	m_enabled = v;

	if (m_cell)
		m_cell->getImplementation()->enableObject(getUserObject(), m_enabled);

	//--------------------------------------------------------------------
	// Mark object bounds as dirty so that it can fit itself better
	// into the database...
	//--------------------------------------------------------------------
	
	setBoundsDirty();
	if (m_enabled)
		m_static = true;
	checkConsistency();
}

/*****************************************************************************
 *
 * Function:		DPVS::ImpObject::checkEnableStatusRecursive()
 *
 * Description:		Checks if object's m_enabled bit is correctly computed. If not,
 *					recursively evaluates children.
 *
 * Parameters:		v = boolean value indicating whether object should be enabled
 *
 *****************************************************************************/

void ImpObject::checkEnableStatusRecursive (void)
{
	// if object is self-disabled -> set as recursively disabled
	// if object's parent is recursively disabled -> set as recursively disabled
	
	bool		correct = m_selfEnabled;
	ImpObject*	parent  = getVisibilityParent();
	if (parent && parent->m_enabled == false)
		correct = false;

	if (isEnabled() != correct)			// if status is incorrect, update internal status
	{
		enableInternal (correct);		// update status

		for (const VisibilityRelation* r = m_firstVisibilityChild; r; r = r->m_next)	// eval children
		{
			DPVS_ASSERT(r->m_parent == this);
			DPVS_ASSERT(r->m_child);
			r->m_child->checkEnableStatusRecursive();
		}
	}
}

/*****************************************************************************
 *
 * Function:		DPVS::ImpObject::selfEnable()
 *
 * Description:		Sets object's self-enable status
 *
 * Parameters:		v = boolean value indicating whether object should be enabled
 *
 *****************************************************************************/

void ImpObject::selfEnable (bool v)
{
	m_selfEnabled = v;
	checkEnableStatusRecursive();
}

/*****************************************************************************
 *
 * Function:		DPVS::ImpObject::createNewTestVertex()
 *
 * Description:		Creates new test vertex for visible point tracking
 *
 *****************************************************************************/

void ImpObject::createNewTestVertex	(void)
{
	DPVS_PROFILE(Statistics::incStatistic(Library::STAT_DATABASEOBNEWVISIBLEPOINTS,1));

	m_dirtyTestVertex = false;

	VertexArray hv;				// query convex hull vertices from object
	getTestMesh(hv);

	const Matrix4x3& m = getObjectToCell();

	//--------------------------------------------------------------------
	// If the model type has vertices, we generate a random point by
	// picking three vertices by random and then interpolation (randomly)
	// between them. This routine is probably an overkill, but it is
	// guaranteed to eventually find a visible point by random.... Also,
	// it has never showed up in VTune or other profiling software, so
	// .. who cares???
	//--------------------------------------------------------------------

	if (hv.m_vertexPositions)		// if vertices were available...
	{
		DPVS_ASSERT(hv.m_vertexCount>0);

		int index[3];
		for (int i = 0; i < 3; i++)
			index[i] = (g_random.getI()&0x7fffffff) % hv.m_vertexCount;

		float r = g_random.get();

		Vector3 v = hv.m_vertexPositions[index[0]]*r + hv.m_vertexPositions[index[1]]*(1.0f-r);

		r = g_random.get();
		v = v*r + hv.m_vertexPositions[index[2]]*(1.0f-r);
		m_testVertex.make(m[0].x*v.x + m[0].y*v.y + m[0].z*v.z + m[0].w,
						  m[1].x*v.x + m[1].y*v.y + m[1].z*v.z + m[1].w,
						  m[2].x*v.x + m[2].y*v.y + m[2].z*v.z + m[2].w);		
	} else
	{
		// DEBUG DEBUG TODO: instead select random point from inside the bounding sphere!!!
		m_testVertex.make(m[0].w,m[1].w,m[2].w);
	}
}

/*****************************************************************************
 *
 * Function:		DPVS::ImpObject::getTestRectangle()
 *
 * Description:		Returns test rectangle of the object
 *
 * Parameters:		r			= reference to rectangle structure where the test rectangle is stored
 *					minDepth	= reference to float where the closest depth value is stored
 *					c			= pointer to camera
 *
 * Returns:			boolean indicating whether test rectangle is on the screen (and thus valid)
 *
 *****************************************************************************/

bool ImpObject::getTestRectangle (FloatRectangle &r)
{
	DPVS_ASSERT(VQData::get().testProperties(VQData::FLOW_ACTIVE));

	//--------------------------------------------------------------------
	// Portals are so expensive that we always want exact rectangles
	// (constructed from the vertex set of the convex hull).
	//--------------------------------------------------------------------
	
	bool value = 
		isPortal() ? 
		m_testModel->getExactRectangle	(r) :
		m_testModel->getOBBRectangle	(r);

	if(value)
	{
		DPVS_ASSERT(VQData::get().accessDepthRange().getMin() >= 0.f);
		VQData::get().setProperties(VQData::DEPTH_RANGE_VALID,true);
		VQData::get().setProperties(VQData::RECTANGLE_CLIPPED,false);
	}

	return value;
}

/*****************************************************************************
 *
 * Function:		DPVS::ImpObject::getWriteRectangle()
 *
 * Description:		Returns write rectangle of the object
 *
 * Parameters:		r			= reference to rectangle structure where the test rectangle is stored
 *
 * Returns:			boolean value indicating whether the rectangle is on the screen
 *
 * Notes:			performs a different action based on type of the write model
 *
 *****************************************************************************/

bool ImpObject::getWriteRectangle	(FloatRectangle &r)
{
	DPVS_ASSERT(VQData::get().testProperties(VQData::FLOW_ACTIVE));
	DPVS_ASSERT(isOcclusionWritable());

	bool value = m_writeModel->getOBBRectangle(r);

	if(value)
	{
		value = r.intersect(VQData::get().getRasterViewport());				// doesn't intersect current viewport (WASS?)
		VQData::get().setProperties(VQData::DEPTH_RANGE_VALID,true);
		VQData::get().setProperties(VQData::RECTANGLE_CLIPPED,true);
	}

	return value;
}

/*****************************************************************************
 *
 * Function:		DPVS::ImpObject::getWriteSilhouette()
 *
 * Description:		Generates write silhouette of the object
 *
 * Parameters:		s			= reference to output silhouette structure
 *					depthRange	= reference to output depth range
 *					c			= pointer to camera
 *
 * Returns:			boolean value indicating whether the silhouette is on the screen
 *
 * Notes:			performs a different action based on type of the write model
 *
 *****************************************************************************/

bool ImpObject::getWriteSilhouette (EdgeSilhouette &s, Range<float>& range, ImpCamera *c)
{
	DPVS_ASSERT(!VQData::get().testProperties(VQData::FLOW_ACTIVE));
	DPVS_ASSERT(isOcclusionWritable());

	bool result = m_writeModel->getWriteSilhouette(s,range,getObjectToCameraMatrix(c),c);
	return result;
}

/*****************************************************************************
 *
 * Function:		DPVS::ImpObject::setObjectToCell(const Matrix4x3& mtx)
 *
 * Description:		Set new objectToCell matrix
 *
 * Parameters:		mtx = reference to object-to-cell matrix
 *
 *****************************************************************************/

void ImpObject::setObjectToCell(const Matrix4x3& mtx)
{
	DPVS_PROFILE(Statistics::incStatistic(Library::STAT_OBJECTMATRIXUPDATES,1));

	//--------------------------------------------------------------------
	// Check validity of the matrix in debug build
	//--------------------------------------------------------------------

	DPVS_API_ASSERT(Math::isInvertible(mtx) && "DPVS::Object::setObjectToCell() - matrix contains zero scale terms (is not invertible!)");

	// DEBUG DEBUG TODO: make a call from the wrapper side so that we could
	// start the prefetching there...

	Matrix4x3& objectToCell = const_cast<Matrix4x3&>(getObjectToCell());

	System::prefetch (&objectToCell[0]);
	System::prefetch (&objectToCell[2][0]);				// 32 bytes away

	//--------------------------------------------------------------------
	// Find out if the new matrix is a uniform rotation matrix (here
	// we're actually just spending time for the prefetch to come through)
	//--------------------------------------------------------------------

	bool uniform = Math::isUniformRotationMatrix(mtx);	

	//--------------------------------------------------------------------
	// Update motion predictor if enough time has passed. This is done
	// only if the object has a Dynamic structure...
	//--------------------------------------------------------------------

	if (m_hasDynamic)
	{
		DPVS_ASSERT(m_dynamic);									// MUST BE!

		float tim = DataPasser::getQueryTime(); 
		if ((tim-m_dynamic->m_lastLocationTime) >= 0.3f)		// update the velocity?
		{
			Vector3 location(mtx.getTranslation());
			m_dynamic->m_velocity			= (location - m_dynamic->m_lastLocation) / (tim-m_dynamic->m_lastLocationTime);
			m_dynamic->m_lastLocation		= location;
			m_dynamic->m_lastLocationTime	= tim;
		}
	}

	//--------------------------------------------------------------------
	// Quickly check if matrix is the same as the existing one. In that
	// case return. This can happen if the user just updates non-changed
	// matrices (stupid!).
	//--------------------------------------------------------------------
	
	if (memEqual (&mtx,&objectToCell,sizeof(Matrix4x3)))	// same matrix... return...
		return;

	objectToCell			= mtx;							// copy matrix
	m_objectToCellUniform	= uniform;						// set matrix uniformity status...

	setBoundsDirty();										// mark bounds as dirty
	checkConsistency();
}

/*****************************************************************************
 *
 * Function:		DPVS::ImpObject::getObjectToCameraMatrix()
 *
 * Description:		Returns object->camera transformation matrix
 *
 * Parameters:		c = pointer to camera (non-null)
 *
 * Notes:			Stores the object*camera matrix into a limited-size
 *					matrix cache (as it is likely that the matrix will be
 *					needed again shortly) and returns reference to that.
 *
 *****************************************************************************/

const Matrix4x3& ImpObject::getObjectToCameraMatrix(ImpCamera* c) const
{
	DPVS_ASSERT(c);

	UPTR hashValue = ((reinterpret_cast<UPTR>(this)));
	hashValue = (((hashValue>>8)*113) ^ (hashValue>>16)) & (MatrixCache::CACHE_SIZE-1);

	MatrixCache::Entry& entry = MatrixCache::getEntry(hashValue);

	if (entry.m_owner != this || entry.m_timeStamp != c->getTimeStamp())
	{
		DPVS_PROFILE(Statistics::incStatistic(Library::STAT_OBJECTCAMERAMATRIXUPDATES,1));

		entry.m_owner		= this;
		entry.m_timeStamp	= c->getTimeStamp();

		Math::productFromLeft(entry.m_matrix, getObjectToCell(), c->getCellToCamera());
	}

	return entry.m_matrix;
}

/*****************************************************************************
 *
 * Function:		DPVS::ImpObject::setBoundsDirty()
 *
 * Description:		Marks object's bounds as dirty
 *
 *****************************************************************************/

void ImpObject::setBoundsDirty(void)
{
	// If object is not in a cell, we don't need to do anything (it will be linked
	// into the dirty list when it's inserted into a cell)...
	if (m_cell)
	{
		unlinkFromDirtyList();
		linkToDirtyListHead();
	}
}

/*****************************************************************************
 *
 * Function:		DPVS::ImpObject::resetObjectTimeStamps()
 *
 * Description:		Resets object timestamps
 *
 * Notes:			This function is only called when the 32-bit timestamp
 *					counters wrap around (i.e. never in practice). We just
 *					have it here for "completeness sake".
 *
 *****************************************************************************/

void ImpObject::resetObjectTimeStamps	 (void)
{
	for (ImpObject* o = s_head; o; o = o->m_next)
	{
		o->resetBenefitTimeStamp ();
		o->m_hiddenTimeStamp	= 0;			// when this object was hidden  (portal time)
		o->m_dirtyTimeStamp		= 0;			// dirty check timestamp
	}
}

/*****************************************************************************
 *
 * Function:		DPVS::ImpObject::getCellSpaceSphere()
 *
 * Description:		Calculates bounding sphere in cell-space
 *
 * Notes:			This is not a particularly cheap function -- the AABBs
 *					of the objects are usually much faster to query...
 *
 *****************************************************************************/

Sphere ImpObject::getCellSpaceSphere (void) const
{
	const Sphere& s = m_testModel->getSphere();

	Vector3 loc    = getObjectToCell().transform(s.getCenter());
	float   radius = s.getRadius() * getObjectToCell().getMaxScale();	// maximum scale
	return Sphere(loc,radius);
}

/*****************************************************************************
 *
 * Function:		DPVS::ImpObject::intersectCellSpaceAABB()
 *
 * Description:		Function for performing object vs. AABB intersection test
 *
 * Parameters:		aabb = reference to AABB in cell space coordinates
 *
 * Returns:			true if object's test model intersects the AABB, false otherwise
 *
 * Notes:			We don't use the model's geometry for the intersection
 *					test anymore.. only the OBB...
 *
 *****************************************************************************/

bool ImpObject::intersectCellSpaceAABB (const AABB& aabb)
{
	//--------------------------------------------------------------------
	// Make sure that the exact AABB has been updated
	//--------------------------------------------------------------------

	if (m_inDirtyList)						
		setupExactBounds();
	
	//--------------------------------------------------------------------
	// First perform two early-exit tests using the AABBs.. this way we
	// can avoid the exact mesh/AABB tests in most cases. Note that we
	// have equality case here as "rejected" because we "fix" the bounding
	// boxes of the objects (by validateBounds())...
	//--------------------------------------------------------------------

	if (m_TBV.getMin().x >=  aabb.getMax().x)  return false;
	if (m_TBV.getMin().y >=  aabb.getMax().y)  return false;
	if (m_TBV.getMin().z >=  aabb.getMax().z)  return false;
	if (m_TBV.getMax().x <=  aabb.getMin().x)  return false;
	if (m_TBV.getMax().y <=  aabb.getMin().y)  return false;
	if (m_TBV.getMax().z <=  aabb.getMin().z)  return false;

	//--------------------------------------------------------------------
	// Check if the object's bounding box is entirely inside the AABB
	//--------------------------------------------------------------------

	if (aabb.containsFully(m_TBV))
		return true;						// guaranteed intersection

	//--------------------------------------------------------------------
	// Perform test using OBBs
	//--------------------------------------------------------------------

	bool retValue = intersectOBBAABB (getModelOBB(), getObjectToCell(), aabb);
	return retValue;
}


/*****************************************************************************
 *
 * Function:		ImpObject::setupQuickBounds()
 *
 * Description:		Quickly calculates cell-space AABB from OBB
 *
 *****************************************************************************/

void ImpObject::setupQuickBounds (AABB& aabb)
{
	const Matrix4x3& a = m_testModel->getOBB().getMatrix();
	const Matrix4x3& b = getObjectToCell();

	float x = (b[0][0]*a[0][3] + b[0][1]*a[1][3] + b[0][2]*a[2][3] + b[0][3]);
	float y = (b[1][0]*a[0][3] + b[1][1]*a[1][3] + b[1][2]*a[2][3] + b[1][3]);
	float z = (b[2][0]*a[0][3] + b[2][1]*a[1][3] + b[2][2]*a[2][3] + b[2][3]);

	float xofs =    Math::fabs(b[0][0]*a[0][0] + b[0][1]*a[1][0] + b[0][2]*a[2][0]) +
					Math::fabs(b[0][0]*a[0][1] + b[0][1]*a[1][1] + b[0][2]*a[2][1]) +
					Math::fabs(b[0][0]*a[0][2] + b[0][1]*a[1][2] + b[0][2]*a[2][2]);

	float yofs = 
					Math::fabs(b[1][0]*a[0][0] + b[1][1]*a[1][0] + b[1][2]*a[2][0]) +
					Math::fabs(b[1][0]*a[0][1] + b[1][1]*a[1][1] + b[1][2]*a[2][1]) +
					Math::fabs(b[1][0]*a[0][2] + b[1][1]*a[1][2] + b[1][2]*a[2][2]);

	float zofs = 
					Math::fabs(b[2][0]*a[0][0] + b[2][1]*a[1][0] + b[2][2]*a[2][0]) +
					Math::fabs(b[2][0]*a[0][1] + b[2][1]*a[1][1] + b[2][2]*a[2][1]) +
					Math::fabs(b[2][0]*a[0][2] + b[2][1]*a[1][2] + b[2][2]*a[2][2]);

	aabb.setMin(Vector3(x-xofs,y-yofs,z-zofs));
	aabb.setMax(Vector3(x+xofs,y+yofs,z+zofs));
}

/*****************************************************************************
 *
 * Function:		ImpObject::setupExactBounds()
 *
 * Description:		Internal routine for calculating exact bounding sphere/AABB of the object
 *
 * Notes:			The member m_TBV is guaranteedly valid after this
 *
 *					This routine always performs work - so please check the dirty
 *					flag before calling the routine
 *
 *****************************************************************************/

void ImpObject::setupExactBounds (void)
{
	VertexArray hv;
	m_testModel->getTestMesh(hv);
	
	if (hv.m_vertexCount>0)				// do we have the test mesh?
	{
		calculateAABB (m_TBV, hv.m_vertexPositions, getObjectToCell(), hv.m_vertexCount);
		s_dynamicToStaticWork += hv.m_vertexCount;
	}
	else								// .. no convex hull, so calculate it from the OBB
		setupQuickBounds(m_TBV);

	validateTBV();
}

/*****************************************************************************
 *
 * Function:		ImpObject::updateBounds()
 *
 * Description:		Recalculates an object's temporal bounding volume
 *
 *****************************************************************************/

void ImpObject::updateBounds (void)
{
	if (!getCell() || !m_enabled)		// if object doesn't belong to any cell or it is disabled..
		return;

	bool update			= true;			// boolean value indicating whether updates should be performed
	
	m_exactDatabaseFit	= true;			// use exact database fitting?

	//----------------------------------------------------------------
	// If object is assumed to stay static for a while, we calculate
	// an exact AABB and bounding sphere from its vertex set.
	//
	// Note that we have a limit on the # of dynamic->static changes
	// we can perform per frame, except for objects that appear for
	// the first time.
	//----------------------------------------------------------------

	bool accessedRecently	= (getCell()->getImplementation()->getDatabase()->getObjectDeltaTimeStamp(this) < 10);

	if (m_unbounded || m_firstBounds || (m_static && accessedRecently && s_dynamicToStaticWork < g_maxDynamicToStaticWork))
	{
		setupExactBounds();												// setup exact bounds (this will inc the vertices processed count)

		if (m_hasDynamic)												// object is now static.. so remove Dynamic structure..
		{
			DPVS_ASSERT(m_dynamic);
			DELETE(m_dynamic);
			m_dynamic		= null;
			m_hasDynamic	= false;
		}
	}
	else																// object is expected to move...
	{
		//------------------------------------------------------------
		// If object doesn't have a Dynamic structure yet, create one..
		//------------------------------------------------------------
		
		if (!m_hasDynamic)												
		{
			DPVS_ASSERT(!m_dynamic);
			//Debug::print ("Object becomes dynamic\n");

			m_dynamic = NEW<Dynamic>();
			m_dynamic->m_lastLocation		= getObjectToCell().getTranslation();
			m_dynamic->m_lastLocationTime	= DataPasser::getQueryTime();
			m_dynamic->m_velocity.clear();
			m_dynamic->m_predictionLength	= g_minPredictionLength;
			m_hasDynamic = true;
		}
		
		//------------------------------------------------------------
		// If the object has been accessed recently during a database
		// query, we must make its TBV tighter. Our rule is that if
		// the object has been accessed during last 10 queries, it
		// is considered "accessed". Accessing is not the same as
		// being visible -- it means that the object has been processed
		// somehow by a visibility query, i.e. work of some kind has
		// been performed.
		//------------------------------------------------------------
			
		DPVS_ASSERT(m_dynamic);
		update = (m_dynamic->m_predictionLength > g_minPredictionLength && accessedRecently);

		//------------------------------------------------------------
		// Estimate conservatively current AABB using a fast scheme
		//------------------------------------------------------------

		AABB aabb;
		setupQuickBounds (aabb);						// calculate quick bounds
		if (!update && !m_TBV.containsFully(aabb))		// bounds violates the existing TBV...
			update = true;								// .. force re-calculation

		//------------------------------------------------------------
		// If the bounds were violated, update them..
		//------------------------------------------------------------

		if(update)		
		{
			//------------------------------------------------------------
			// Update prediction length and clamp to valid range
			//------------------------------------------------------------

			DPVS_ASSERT(m_dynamic);
			m_dynamic->m_predictionLength *= (accessedRecently ? g_downPredictionScale : g_upPredictionScale);

			if (m_dynamic->m_predictionLength <= (g_minPredictionLength+0.03f))
				m_dynamic->m_predictionLength = g_minPredictionLength;
			
			if (accessedRecently)
			{
				if (m_dynamic->m_predictionLength > g_maxAccessedPredictionLength)
					m_dynamic->m_predictionLength = g_maxAccessedPredictionLength;
			} else
			{
				if (m_dynamic->m_predictionLength > g_maxNonAccessedPredictionLength)
					m_dynamic->m_predictionLength = g_maxNonAccessedPredictionLength;
			}

			//------------------------------------------------------------
			// Calculate velocity...
			//------------------------------------------------------------

			Vector3 v(m_dynamic->m_velocity * m_dynamic->m_predictionLength);

			//------------------------------------------------------------
			// Detect extreme velocities - in such a case guess that the
			// object has "teleported", i.e. it is not moving "nicely".
			//------------------------------------------------------------

			float limitSqr = aabb.getDiagonalLengthSqr() * 100.0f;			// (10.0 * diagonal length)^2

			if (v.lengthSqr() >= limitSqr)									// object is teleporting!
			{
				//------------------------------------------------------------
				// Object is teleporting.. so set minimal bounds just to get things under
				// some kind of control... also reset the dynamic counters so that if the
				// object starts to behave nicely, things will work a-ok...
				//------------------------------------------------------------

				m_dynamic->m_lastLocation		= getObjectToCell().getTranslation();
				m_dynamic->m_lastLocationTime	= DataPasser::getQueryTime();
				m_dynamic->m_velocity.clear();								// clear velocity...
				m_dynamic->m_predictionLength	= g_minPredictionLength;	// reset prediction length

				m_TBV.set(aabb.getMin(), aabb.getMax());					// take the exact bounds
			} else
			{
				//------------------------------------------------------------
				// Object is behaving "nicely". So construct the 
				// motion bounds from the estimated velocity and approximate
				// AABB (slightly over-estimated to allow rotation).
				//------------------------------------------------------------

				Vector3 c	= aabb.getCenter();								// calculate new bounding box
				Vector3 d	= aabb.getDimensions()*0.5f;					// .. with some over-estimation...
				Vector3 mn	= c - d*1.20f;
				Vector3 mx	= c + d*1.20f;

				if (v.x < 0.0f) mn.x += v.x; else mx.x+=v.x;				// apply motion vector
				if (v.y < 0.0f) mn.y += v.y; else mx.y+=v.y;
				if (v.z < 0.0f) mn.z += v.z; else mx.z+=v.z;

				m_TBV.set(mn,mx);											// assign new TBV
			}
			
			validateTBV();													// make sure the TBV is all right!
		} 
		
		//------------------------------------------------------------
		// Add the object to the tail of the dirty list so that we
		// re-check its status after a while to determine if object
		// has become static.
		//------------------------------------------------------------

		linkToDirtyListTail();								
		m_exactDatabaseFit = false;											// don't use exact database fits...
	}

	//--------------------------------------------------------------------
	// If an update is required, send the updated information to the
	// database..
	//--------------------------------------------------------------------

	if (update)
	{
		getCell()->getImplementation()->getDatabase()->updateObjectBounds(this);
		m_firstBounds = false;									
	}
}

/*****************************************************************************
 *
 * Function:		DPVS::ImpObject::updateAllDirtyBounds()
 *
 * Description:		Updates dirty bounds of objects (checks timestamps)
 *
 * Notes:			This is a static function called in the beginning
 *					of any Camera::resolveVisibility() query.
 *
 *****************************************************************************/

void ImpObject::updateAllDirtyBounds (ImpCellDirtyObjectList& list)
{
	MatrixCache::reset();								// we enter a new cell, so let's reset the matrix cache (just in case)

	s_dynamicToStaticWork = 0;

	if (list.m_dirtyHead)
	{
		float tim = DataPasser::getQueryTime();
//		Debug::print ("updating bounds for cell %p\n",&list);
		while (list.m_dirtyHead && (list.m_dirtyHead->m_dirtyTimeStamp <= tim))
		{
			ImpObject* ob		= list.m_dirtyHead;
			list.m_dirtyHead	= list.m_dirtyHead->m_nextDirty;
			if (list.m_dirtyHead)
				list.m_dirtyHead->m_prevDirty = null;
			else
				list.m_dirtyTail = null;

			ob->m_prevDirty		= null;						// remove ob from linked list
			ob->m_nextDirty		= null;
			ob->m_inDirtyList	= false;
			
			ob->updateBounds();
		}
	}
}

/*****************************************************************************
 *
 * Function:		DPVS::ImpObject::backFaceCull()
 *
 * Description:		Performs back-face culling for an object
 *
 *****************************************************************************/

bool ImpObject::backFaceCull (class ImpCamera* c) const				
{ 
	//--------------------------------------------------------------------
	// If the object is a portal, compute the "camera in object-space" 
	// location in a more robust way that is not affected by camera 
	// rotation.
	//--------------------------------------------------------------------
	
	if (isPortal() && !VQData::get().isOrthoProjection())
	{
		Matrix4x4 tmp;
		Matrix4x3 cellToObject;
		Vector4   nlat;
		Math::invertMatrix(cellToObject,getObjectToCell());
		Math::productFromLeft(tmp, c->getCameraToCell(), cellToObject);
		nlat = Vector4(tmp[0][3],tmp[1][3],tmp[2][3], 1.0f);
		//printf ("nlat = %.8f %.8f %.8f %.8f\n", nlat.x,nlat.y,nlat.z,nlat.w);
		return m_testModel->backFaceCull(nlat); 
	}

	//--------------------------------------------------------------------
	// The "camera in object space" is expressed as a homogenous coordinate. 
	// The W value is 1.0 if we're using a perspective projection, 0.0 if
	// using ortho projection.
	//--------------------------------------------------------------------
	
	const Matrix4x3&	mtx = getObjectToCameraMatrix(c);
	Vector4				tlat;

	if (VQData::get().isOrthoProjection())
	{
		Matrix4x3 cameraToObject(NO_CONSTRUCTOR);
		Math::invertMatrix(cameraToObject, mtx);
		tlat = Vector4(-cameraToObject.getDof(),0.0f);
	} else
		tlat = Vector4(Math::getInverseTranslation (mtx),1.0f);

	
	return m_testModel->backFaceCull(tlat); 
}

//------------------------------------------------------------------------
