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
 * Description:     Object/ROI intersection detection code
 *
 * Notes:           The (Imp)SweepAndPrune class handles the object vs. ROI
 *                  intersection detection. The class is divided into
 *                  an interface class and a separate implementation class
 *                  so that we can happily go and change everything in the .cpp
 *                  file without having to recompile a lot of code.
 *
 *
 * $Archive: /dpvs/implementation/sources/dpvsSweepAndPrune.cpp $
 * $Author: wili $ 
 * $Revision: #2 $
 * $Modtime: 25.10.01 16:29 $
 * $Date: 2003/08/24 $
 * 
 ******************************************************************************/

#include "dpvsSweepAndPrune.hpp"        // SweepAndPrune interface
#include "dpvsIntersect.hpp"            // includes dpvsAABB.hpp, dpvsSphere.hpp, contains intersection routines
#include "dpvsSort.hpp"                 // for quickSort()
#include "dpvsImpObject.hpp"            // Object class
#include "dpvsImpCommander.hpp"         // Commander class (callbacks to the user)
#include "dpvsTempArray.hpp"            // Temporary memory allocation class

namespace DPVS
{

/*****************************************************************************
 *
 * Class:           DPVS::ImpSweepAndPrune
 *
 * Description:     Implementation class for performing sweep and prune overlap 
 *                  testing between objects and ROIs
 *
 * Notes:           The sweep and prune algorithm performs a three-dimensional
 *                  sweep through the object set. The objects and ROIs are sorted on 
 *                  one of the major axii (selected based on current view direction,
 *                  so that we get rough front->back ordering at the same time -
 *                  there are six different orientations). They are also bucket-sorted
 *                  based on the other two axii. The actual sweep then walks
 *                  along the major axis, testing only against ROIs that overlap
 *                  on that axis _and_ the buckets the object occupies.
 *
 *                  Note that usually the number of objects and ROIs is relatively
 *                  small, since this process is a back-end for the visibility query -
 *                  we only process objects and ROIs that were found visible
 *                  (inside view frustum and not occluded).
 *
 *                  If the class was used for other purposes, a good choice for the 
 *                  major axis could be that of the maximal variance in object/light 
 *                  spatial distribution. 
 *
 *                  Variations of this code could be in theory used for
 *                  sweep and prune operations in coarse collision detection etc.
 *                  However, our scenario is somewhat more complicated, as we have some 
 *                  additional goals:
 *
 *                  a) Sweep direction must be approximately front->back relative
 *                     to the camera (improves z-buffering performance in 
 *                     rendering hardware)
 *
 *                  b) We want to minimize the number of light state changes, as
 *                     these are expensive to perform on the rendering
 *                     hardware. The code tracks the current set of
 *                     "active" lights (i.e. those signaled "on" to the user)
 *                     and only signals state _changes_ (i.e. when a light
 *                     is activated/deactivated).
 *
 *                  If the latter goal is removed, the innermost part of the 
 *                  sweep and prune code (ImpSweepAndPrune::processInternal())
 *                  is simplified considerably.
 *
 *                  Most of the member functions are force-inlined so that
 *                  the code is actually compiled into the wrapper class
 *                  (located at the end of this file).
 *
 * Todo:            Potential further optimizations include compression
 *                  of ObEntry structures (m_left and m_right could be quantized
 *                  into 0.16 fixed point, thus making the structure size
 *                  16 bytes and changing the comparison code to use integer
 *                  instructions).
 *
 *****************************************************************************/

class ImpSweepAndPrune
{
public:
                        ImpSweepAndPrune    (void); 
                        ~ImpSweepAndPrune   (void); 
    void                process             (Commander* cmd, const ImpObject* const * obs, const ImpObject* const* ROIs, int nObs, int nROI, const Vector3&);
    static void         minimizeMemoryUsage (void);

    struct ObEntry;

    struct ROIData                          // Extra data we need for each ROI
    {
        Sphere              m_sphere;       // bounding sphere
        const ObEntry*      m_mailBox;      // last object processed?
        bool                m_active;       // activity status
    };

    struct ObEntry                          // Object or ROI internal representation
    {
        const ImpObject*    m_ob;           // pointer back to original object / ROI
        ROIData*            m_ROIData;      // pointer to ROI data (only for ROIs - objects have null)
        float               m_left;         // min interval value on primary axis
        float               m_right;        // max interval value on primary axis
        unsigned char       m_x0;           // x left bucket    (inclusive)
        unsigned char       m_y0;           // y top bucket     (inclusive)
        unsigned char       m_x1;           // x right bucket   (exclusive)
        unsigned char       m_y1;           // y right bucket   (exclusive)

        bool operator< (const ObEntry& s) const { return m_left < s.m_left; }
        bool operator> (const ObEntry& s) const { return m_left > s.m_left; }
    };

    struct PObEntry                         // Wrapper class used by the quicksort routine (sorting ObEntries using pointers)
    {
        ObEntry* m_ptr;                     // ptr to data
        bool operator< (const PObEntry& s) const { return m_ptr->m_left < s.m_ptr->m_left; }
        bool operator> (const PObEntry& s) const { return m_ptr->m_left > s.m_ptr->m_left; }
    };

    struct IntervalInfo                     // Structure used for storing the bucketing information
    {
        Vector2         m_topLeft;          // top left corner in cell-space
        Vector2         m_scale;            // 1.0 / (bottomRight - topLeft)
        int             m_xBuckets;         // [0,255] number of x-direction buckets
        int             m_yBuckets;         // [0,255] number of y-direction buckets
        int             m_primaryAxis;      // [0,2] primary axis
        int             m_secondaryAxis;    // [0,2] secondary axis
        int             m_tertiaryAxis;     // [0,2] last axis 
        bool            m_primaryNegative;  // is primary axis flipped?
    };

    struct ROIInstance                      // Instance of a ROI (inside a single bucket)
    {
        ObEntry*        m_ROI;              // pointer back to ROI data
        ROIInstance*    m_next;             // next ROI instance inside the same bucket (singly-linked list)
    };


    struct Bucket                           // Bucket holding ROI Instances
    {
        ROIInstance*    m_tail;             // tail (used during list construction only)
        ROIInstance*    m_head;             // current head of the linked list
    };

private:
                        ImpSweepAndPrune    (const ImpSweepAndPrune&);  // not allowed
    ImpSweepAndPrune&   operator=           (const ImpSweepAndPrune&);  // not allowed


    void                processNoROIs       (Commander* cmd, const ImpObject* const* obs, int nObs) const;
    void                processInternal     (Commander* cmd, ObEntry* objects, ObEntry* regionsOfInfluence, int nObs,  int nROI, const IntervalInfo&);
    void                setROIActive        (ObEntry& ROI, bool active);
    static void         sort                (PObEntry* arr, int N);
    static void         assignIntervals     (ObEntry* d, const ImpObject* const* src, int N, const IntervalInfo&);
    static void         scanExtents         (Vector2& topLeft, Vector2& bottomRight, int axis0, int axis1, const ImpObject* const* obs, int N);

    Commander*          m_commander;        // ptr to current commander
    ImpCommander*       m_impCommander;     // ptr to commander implementation
    int                 m_activeROIs;       // # of ROIs currently active (used for debugging only)
};

} // DPVS

using namespace DPVS;

/*****************************************************************************
 *
 * Function:        DPVS::ImpSweepAndPrune::minimizeMemoryUsage()
 *
 * Description:     Minimizes recycler memory usage 
 *
 *****************************************************************************/

DPVS_FORCE_INLINE void ImpSweepAndPrune::minimizeMemoryUsage(void) 
{ 
    // Nada currently as recyclers not used here
}


/*****************************************************************************
 *
 * Function:        DPVS::ImpSweepAndPrune::ImpSweepAndPrune()
 *
 * Description:     Constructor
 *
 *****************************************************************************/

DPVS_FORCE_INLINE  ImpSweepAndPrune::ImpSweepAndPrune (void) :
    m_commander(null),
    m_impCommander(null),
    m_activeROIs(0)
{
    // Nada
}

/*****************************************************************************
 *
 * Function:        DPVS::ImpSweepAndPrune::~ImpSweepAndPrune()
 *
 * Description:     Destructor
 *
 * Notes:           The shared recyclers are released when the last ImpSweepAndPrune
 *                  object is destroyed.
 *
 *****************************************************************************/

DPVS_FORCE_INLINE ImpSweepAndPrune::~ImpSweepAndPrune (void) 
{
    DPVS_ASSERT (!m_commander);
    DPVS_ASSERT (!m_impCommander);
    DPVS_ASSERT (!m_activeROIs);
}

/*****************************************************************************
 *
 * Function:        DPVS::ImpSweepAndPrune::assignIntervals()
 *
 * Description:     Sets up the ObEntry structures for source objects and calculates
 *                  the sweep intervals (i.e. bucketing of each object)
 *
 * Parameters:      d       = destination ObEntry array
 *                  s       = source ImpObject* array
 *                  N       = number of objects
 *                  ival    = other source parameters
 *
 * Notes:           The same function is used to setup the ROI structures and the 
 *                  object structures
 *
 *****************************************************************************/

void ImpSweepAndPrune::assignIntervals (ObEntry* dst, const ImpObject* const* s, int N, const IntervalInfo& ival)
{
    DPVS_ASSERT (ival.m_primaryAxis   != ival.m_secondaryAxis);
    DPVS_ASSERT (ival.m_primaryAxis   != ival.m_tertiaryAxis);
    DPVS_ASSERT (ival.m_secondaryAxis != ival.m_tertiaryAxis);
    DPVS_ASSERT (N>=0);
    
    for (int i = 0; i < N; i++)
    {
        const ImpObject*    obj = s[i];                         // get pointer
        ObEntry&            d   = dst[i];                       // target

        d.m_ob      = obj;                                      
        d.m_ROIData = null;

        //----------------------------------------------------------------
        // Setup primary axis (note ordering for negative primary axis)
        //----------------------------------------------------------------

        const AABB& aabb = obj->getCellSpaceAABB();

        if (ival.m_primaryNegative)
        {
            d.m_left    = -aabb.getMax()[ival.m_primaryAxis];
            d.m_right   = -aabb.getMin()[ival.m_primaryAxis];
        } else
        {
            d.m_left    = aabb.getMin()[ival.m_primaryAxis];
            d.m_right   = aabb.getMax()[ival.m_primaryAxis];
        }
                    
        //----------------------------------------------------------------
        // Calculate bucket area (x0,y0) - (x1,y1) for the second and
        // tertiary axis
        //----------------------------------------------------------------

        Vector2 topLeft     (aabb.getMin()[ival.m_secondaryAxis],aabb.getMin()[ival.m_tertiaryAxis]);
        Vector2 bottomRight (aabb.getMax()[ival.m_secondaryAxis],aabb.getMax()[ival.m_tertiaryAxis]);

        topLeft.x       -= ival.m_topLeft.x;
        topLeft.x       *= ival.m_scale.x;          
        topLeft.y       -= ival.m_topLeft.y;
        topLeft.y       *= ival.m_scale.y;
        bottomRight.x   -= ival.m_topLeft.x;
        bottomRight.x   *= ival.m_scale.x;
        bottomRight.y   -= ival.m_topLeft.y;
        bottomRight.y   *= ival.m_scale.y;

        int x0,x1,y0,y1;

        x0 = Math::intChop(topLeft.x);              // we know that topLeft/bottomRight are going to be positive,
        y0 = Math::intChop(topLeft.y);              // ...so we can use chop instead of flooring..
        x1 = Math::intChop(bottomRight.x)+1;        // need to do chop()+1 (cannot do ceil()) as x1,y1 exclusive..
        y1 = Math::intChop(bottomRight.y)+1;        

        // we've ensured earlier that ival.m_xBuckets and ival.m_yBuckets
        // is < 255, i.e. results fit into an unsigned char.

        if (x0 < 0) x0 = 0; if (x0 >= ival.m_xBuckets)  x0 = ival.m_xBuckets-1;
        if (y0 < 0) y0 = 0; if (y0 >= ival.m_yBuckets)  y0 = ival.m_yBuckets-1;
        if (x1 < 0) x1 = 0; if (x1 >  ival.m_xBuckets)  x1 = ival.m_xBuckets;       // Note: exclusive
        if (y1 < 0) y1 = 0; if (y1 >  ival.m_yBuckets)  y1 = ival.m_yBuckets;

        d.m_x0 = (unsigned char)x0;
        d.m_y0 = (unsigned char)y0;
        d.m_x1 = (unsigned char)x1;
        d.m_y1 = (unsigned char)y1;
    }
}

/*****************************************************************************
 *
 * Function:        DPVS::ImpSweepAndPrune::scanExtents()
 *
 * Description:     Enlarges rectangle [topLeft-bottomRight] by the objects' 
 *                  bounding areas
 *                  
 * Parameters:      topLeft         = output top-left vector
 *                  bottomRight     = output bottom-right vector
 *                  axis0           = index of first axis
 *                  axis1           = index of second axis
 *                  obs             = source object array
 *                  N               = number of objects
 *
 * Notes:           This function is used to find the area used for
 *                  second/tertiary axis bucketing.
 *
 *****************************************************************************/

void ImpSweepAndPrune::scanExtents (Vector2& topLeft, Vector2& bottomRight, int axis0, int axis1, const ImpObject* const* obs, int N)
{
    //----------------------------------------------------------------
    // Note that the routine doesn't seek the actual min-max extents, 
    // we only need some rough idea about the XY size for bucketing.
    //----------------------------------------------------------------

    for (int i = 0; i < N; i++)     
    {   
        const AABB& aabb    = obs[i]->getCellSpaceAABB();                           // AABB of the object
        float       x       = (aabb.getMin()[axis0] + aabb.getMax()[axis0])*0.5f;   // center-point x
        float       y       = (aabb.getMin()[axis1] + aabb.getMax()[axis1])*0.5f;   // center-point y
        
        topLeft.x           = Math::min(topLeft.x, x);
        topLeft.y           = Math::min(topLeft.y, y);
        bottomRight.x       = Math::max(bottomRight.x,x);
        bottomRight.y       = Math::max(bottomRight.y,y);
    }
}

/*****************************************************************************
 *
 * Function:        DPVS::ImpSweepAndPrune::setROIActive()
 *
 * Description:     Enables or disables a ROI
 *
 * Parameters:      ROI     = reference to ObEntry structure
 *                  active  = new state of the ROI (true == active)
 *
 * Notes:           This function sends the actual active/inactive commands
 *                  to the Commander
 *
 *****************************************************************************/

DPVS_FORCE_INLINE void ImpSweepAndPrune::setROIActive   (ObEntry& ROI, bool active)
{
    DPVS_ASSERT(ROI.m_ROIData);

    //----------------------------------------------------------------
    // If the status (active/inactive) of the ROI changes, send a 
    // message to the commander.
    //----------------------------------------------------------------

    if (active != ROI.m_ROIData->m_active)          
    {
        m_activeROIs += (active) ? 1 : -1;                      // update counter
        DPVS_ASSERT(m_activeROIs >= 0);                         // sanity check
        
        ROI.m_ROIData->m_active = active;                       

        DPVS_ASSERT     (m_commander && m_impCommander);
        DPVS_PROFILE    (Statistics::incStatistic(Library::STAT_ROISTATECHANGES,1));
        
        m_impCommander->setInstanceObject   (ROI.m_ob);
		#if defined (DPVS_DEBUG)
			UINT32 oldFPUMode = getFPUMode();
		#endif

        m_commander->command                (active ? Commander::REGION_OF_INFLUENCE_ACTIVE : Commander::REGION_OF_INFLUENCE_INACTIVE);
		DPVS_ASSERT(getFPUMode() == oldFPUMode && "FPU mode changed in user callback!");
    }
}

/*****************************************************************************
 *
 * Function:        DPVS::ImpSweepAndPrune::processNoROIs()
 *
 * Description:     Lists visible objects to the commander in the case
 *                  that there are no active ROIs
 *
 * Parameters:      cmd     = pointer to commander
 *                  obs     = pointer to head of object array
 *                  nObs    = number of obs in the object array
 *
 * Notes:           This function is just an optimized case. Having this
 *                  function also made the implementation of the real case
 *                  somewhat easier.
 *
 *****************************************************************************/

void ImpSweepAndPrune::processNoROIs (Commander* cmd, const ImpObject* const* obs, int nObs) const
{
    DPVS_ASSERT(cmd && obs);

	if (nObs > 0)
	{
		ImpCommander* impCmd = cmd->getImplementation();
		DPVS_ASSERT(impCmd);

		for (int i = 0; i < nObs; i++)                          // send commands
		{
			impCmd->setInstanceObject   (obs[i]);
		#if defined (DPVS_DEBUG)
			UINT32 oldFPUMode = getFPUMode();
		#endif
			cmd->command                (Commander::INSTANCE_VISIBLE);
			DPVS_ASSERT(getFPUMode() == oldFPUMode && "FPU mode changed in user callback!");
		}

		impCmd->setInstanceObject(null);                        // set instance object to null to trap "leak" errors later
	}
}

/*****************************************************************************
 *
 * Function:        DPVS::ImpSweepAndPrune::process ()
 *
 * Description:     Performs sweep & prune overlap testing between a set of objects
 *                  and ROIs
 *
 * Parameters:      cmd             = pointer to commander
 *                  obs             = pointer to head of object array
 *                  ROIs            = pointer to head of ROI array
 *                  nObs            = number of obs in the object array
 *                  nROI            = number of ROIs in the ROI array (can be zero)
 *                  sweepDirection  = camera view direction (i.e. eye-space Z) in cell-space coordinate system
 *
 * Notes:           This function mainly just sets up all the data structures.
 *                  It then calls ImpSweepAndPrune::processInternal() to do 
 *                  the real job.
 *
 *****************************************************************************/

void ImpSweepAndPrune::process  (Commander* cmd, const ImpObject* const* obs, const ImpObject* const * ROIs, int nObs, int nROI, const Vector3& sweepDirection)
{
    if (!nObs)                          // nothing to do?
        return;

    DPVS_ASSERT(nObs>=0 && nROI >= 0);

    //----------------------------------------------------------------
    // Perform some statistics updates and data validation in the
    // debug build.
    //----------------------------------------------------------------

    DPVS_ASSERT (cmd);
    DPVS_PROFILE    (Statistics::incStatistic(Library::STAT_CAMERAVISIBILITYCALLBACKS,nObs));
    DPVS_PROFILE    (Statistics::incStatistic(Library::STAT_ROIACTIVE,nROI));

#if defined (DPVS_DEBUG)
    if (nObs) DPVS_ASSERT(obs);
    if (nROI) DPVS_ASSERT(ROIs);

    for (int i = 0; i < nObs; i++)
        DPVS_ASSERT(obs[i]);            // cannot pass null object pointer
    for (int i = 0; i < nROI; i++)
        DPVS_ASSERT(ROIs[i]);           // cannot pass null ROI pointer
#endif

    //----------------------------------------------------------------
    // Handle "unbounded" objects and ROIs. We just take copies of the
	// input lists and assign bounded objects/ROIs to the beginning
	// of each array and the unbounded ones to the end of each array.
    //----------------------------------------------------------------

	TempArray<const ImpObject*>	tmpObs(nObs);	// temporary object array
	TempArray<const ImpObject*> tmpROIs(nROI);
	int							nBoundedObs		= 0;
	int							nUnboundedObs	= 0;
	int							nBoundedROIs	= 0;
	int							nUnboundedROIs	= 0;

	for (int i = 0; i < nObs; i++)						// sort objects into bounded and unbounded ones
	{
		const ImpObject* o = obs[i];

		if (o->isUnbounded())							// organized unbounded obs to the end of the array
		{
			nUnboundedObs++;
			tmpObs[nObs-nUnboundedObs] = o;
		} else
		{
			tmpObs[nBoundedObs] = o;					// bounded ones to the beginning..
			nBoundedObs++;
		}
	}

	for (int i = 0; i < nROI; i++)						// sort objects into bounded and unbounded ones
	{
		const ImpObject* r = ROIs[i];

		if (r->isUnbounded())
		{
			nUnboundedROIs++;
			tmpROIs[nROI-nUnboundedROIs] = r;
		} else
		{
			tmpROIs[nBoundedROIs] = r;
			nBoundedROIs++;
		}
	}

    //----------------------------------------------------------------
    // Enable all unbounded ROIs
    //----------------------------------------------------------------


	if (nUnboundedROIs)
	{
		const ImpObject* const *	unboundedROIs   = &tmpROIs[nBoundedROIs];
		ImpCommander*				imp				= cmd->getImplementation();
		for (int i = 0; i < nUnboundedROIs; i++)
		{
			imp->setInstanceObject	(unboundedROIs[i]);
		#if defined (DPVS_DEBUG)
			UINT32 oldFPUMode = getFPUMode();
		#endif

			cmd->command			(Commander::REGION_OF_INFLUENCE_ACTIVE);
			DPVS_ASSERT(getFPUMode() == oldFPUMode && "FPU mode changed in user callback!");
		}
        DPVS_PROFILE  (Statistics::incStatistic(Library::STAT_ROISTATECHANGES,nUnboundedROIs));
	}

    //----------------------------------------------------------------
    // Handle all unbounded objects (there shouldn't be any, but someone
	// might just be crazy enough to use them)... We enable ALL active
	// ROIs in the cell, then process the unbounded objects... phew...
    //----------------------------------------------------------------

	if (nUnboundedObs)
	{
		ImpCommander*				imp				= cmd->getImplementation();
		const ImpObject* const *	unboundedObs    = &tmpObs[nBoundedObs];
		
		for (int i = 0; i < nBoundedROIs; i++)				// enable ALL bounded ROIs!!!
		{
			imp->setInstanceObject	(tmpROIs[i]);
		#if defined (DPVS_DEBUG)
			UINT32 oldFPUMode = getFPUMode();
		#endif
			cmd->command			(Commander::REGION_OF_INFLUENCE_ACTIVE);
			DPVS_ASSERT(getFPUMode() == oldFPUMode && "FPU mode changed in user callback!");
		}

		for (int i = 0; i < nUnboundedObs; i++)
		{
		#if defined (DPVS_DEBUG)
			UINT32 oldFPUMode = getFPUMode();
		#endif
			imp->setInstanceObject   (unboundedObs[i]);
			cmd->command             (Commander::INSTANCE_VISIBLE);
			DPVS_ASSERT(getFPUMode() == oldFPUMode && "FPU mode changed in user callback!");
		}
		for (int i = 0; i < nBoundedROIs; i++)						// disable ALL bounded ROIs!!!
		{
			imp->setInstanceObject	(tmpROIs[i]);
		#if defined (DPVS_DEBUG)
			UINT32 oldFPUMode = getFPUMode();
		#endif
			cmd->command			(Commander::REGION_OF_INFLUENCE_INACTIVE);
			DPVS_ASSERT(getFPUMode() == oldFPUMode && "FPU mode changed in user callback!");
		}

        DPVS_PROFILE  (Statistics::incStatistic(Library::STAT_ROISTATECHANGES,nBoundedROIs*2));
	
	}

    //----------------------------------------------------------------
    // If no bounded ROIs are visible, use this mucho quicker loop 
	// (this avoids sorting objects etc.)
    //----------------------------------------------------------------

    if (!nBoundedROIs || !nBoundedObs)
        processNoROIs(cmd,&tmpObs[0],nBoundedObs);		// just report objects as visible...
	else
	{
		//----------------------------------------------------------------
		// Decide primary, secondary and tertiary axii. Select the
		// largest component in the sweep direction to be the primary
		// axis. The order of the two other axii does not matter.
		//----------------------------------------------------------------

		int     primary         = 0;
		int     secondary       = 1;
		int     tertiary        = 2;
		bool    primaryNegative = false;

		if (Math::fabs(sweepDirection[1]) > Math::fabs(sweepDirection[primary])) 
			primary = 1;
		if (Math::fabs(sweepDirection[2]) > Math::fabs(sweepDirection[primary])) 
			primary = 2;

		if (sweepDirection[primary]<0.0f)
			primaryNegative = true;

		if (primary == 1)
		{
			secondary = 0;
			tertiary  = 2;
		} else 
		if (primary == 2)
		{
			secondary = 0;
			tertiary  = 1;
		}

		//----------------------------------------------------------------
		// Scan XY (or actually secondary/tertiary axis) extents of the 
		// objects and ROIs to topLeft & bottomRight
		//----------------------------------------------------------------

		Vector2 topLeft     (+FLT_MAX,+FLT_MAX);
		Vector2 bottomRight (-FLT_MAX,-FLT_MAX);

		scanExtents (topLeft,bottomRight,secondary,tertiary, &tmpObs[0],nBoundedObs);
		scanExtents (topLeft,bottomRight,secondary,tertiary, &tmpROIs[0],nBoundedROIs);

		//----------------------------------------------------------------
		// Decide how many buckets we're going to use (# of buckets
		// should be approximately (nBoundedROIs+nBoundedObs)^0.666) and setup the
		// IntervalInfo structure. If there's three or less ROIs, don't
		// use XY bucketing at all (force bucketsPerDir to 1).
		//----------------------------------------------------------------

		IntervalInfo        ival;
		static const float  oo3 = 1.0f / 3.0f;
		int bucketsPerDir = Math::intChop(Math::pow((float)(nBoundedROIs+nBoundedObs),oo3));   // cubic root + 1
    
		if (bucketsPerDir < 1 || (nBoundedROIs <= 3))
			bucketsPerDir = 1;

		if (bucketsPerDir > 0xFF)                                               // internal maximum (although this case never happens)
			bucketsPerDir = 0xFF;

		ival.m_primaryAxis      = primary;
		ival.m_primaryNegative  = primaryNegative;
		ival.m_secondaryAxis    = secondary;
		ival.m_tertiaryAxis     = tertiary;

		ival.m_xBuckets         = bucketsPerDir;
		ival.m_yBuckets         = bucketsPerDir;
		ival.m_topLeft          = topLeft;

		float scalex            = bottomRight.x - topLeft.x;
		float scaley            = bottomRight.y - topLeft.y;

		ival.m_scale.x          = scalex ? ival.m_xBuckets / scalex : 0.0f;
		ival.m_scale.y          = scaley ? ival.m_yBuckets / scaley : 0.0f;


		//----------------------------------------------------------------
		// Assign bucket interval values for the objects and ROIs. The
		// arrays are allocated from the "Temporary Memory Manager".
		//----------------------------------------------------------------

		{
			TempArray<ObEntry,false>    objects(nBoundedObs);
			TempArray<ObEntry,false>    regionsOfInfluence(nBoundedROIs);
			TempArray<ROIData,false>    data(nBoundedROIs);

			assignIntervals (&objects[0],				&tmpObs[0], nBoundedObs, ival);              
			assignIntervals (&regionsOfInfluence[0],	&tmpROIs[0],nBoundedROIs,ival);  

			//----------------------------------------------------------------
			// Allocate and setup ROI-specific data for ROIs (compute cell-space 
			// spheres etc.)
			//----------------------------------------------------------------

			for (int i = 0; i < nBoundedROIs; i++)
			{
				ROIData* d = &data[i];
				regionsOfInfluence[i].m_ROIData = d;
				d->m_active  = false;
				d->m_mailBox = null;
				d->m_sphere  = regionsOfInfluence[i].m_ob->getCellSpaceSphere();
			}

			//----------------------------------------------------------------
			// Sort objects/ROIs based on their intervals and handle
			// overlap testing
			//----------------------------------------------------------------

			processInternal (cmd, &objects[0], &regionsOfInfluence[0], nBoundedObs, nBoundedROIs, ival);   // internal process function
		}
	}

    //----------------------------------------------------------------
    // Disable all unbounded ROIs
    //----------------------------------------------------------------

	if (nUnboundedROIs)
	{
		const ImpObject* const *	unboundedROIs   = &tmpROIs[nBoundedROIs];
		ImpCommander*				imp				= cmd->getImplementation();
		for (int i = 0; i < nUnboundedROIs; i++)
		{
			imp->setInstanceObject	(unboundedROIs[i]);
		#if defined (DPVS_DEBUG)
			UINT32 oldFPUMode = getFPUMode();
		#endif
			cmd->command            (Commander::REGION_OF_INFLUENCE_INACTIVE);
			DPVS_ASSERT(getFPUMode() == oldFPUMode && "FPU mode changed in user callback!");
		}
        DPVS_PROFILE  (Statistics::incStatistic(Library::STAT_ROISTATECHANGES,nUnboundedROIs));
	}

    cmd->getImplementation()->setInstanceObject(null);                        // set instance object to null to trap "leak" errors later
}

/*****************************************************************************
 *
 * Function:        DPVS::ImpSweepAndPrune::sort ()
 *
 * Description:     Sorts an array of objects
 *
 * Parameters:      arr = pointer to array
 *                  N   = number of objects
 *
 * Notes:           The function currently used quickSort() as it has been
 *                  found to be faster (in all practical cases) than
 *                  some theoretically O(N) algorithms, such as radix sorting.
 *
 *                  Made non-inline so that the template sort code gets
 *                  compiled here only once.
 *
 *****************************************************************************/

void ImpSweepAndPrune::sort (PObEntry* arr, int N)
{
    DPVS_ASSERT (arr && N >= 0);
    quickSort (arr, N); 
}

/*****************************************************************************
 *
 * Function:        DPVS::ImpSweepAndPrune::processInternal ()
 *
 * Description:     Performs sweep & prune overlap testing between a set of objects
 *                  and ROIs
 *
 * Parameters:      cmd     = pointer to commander
 *                  obs     = pointer to head of object array
 *                  ROIs    = pointer to head of ROI array
 *                  nObs    = number of obs in the object array
 *                  nROI    = number of ROIs in the ROI array (can be zero)
 *                  ival    = reference to interval info structure
 *
 * Notes:           This function does the real sweeping and pruning. It
 *                  gets the objects and ROIs as its input and it reports
 *                  (via the Commander) the overlaps.
 *
 *****************************************************************************/

void ImpSweepAndPrune::processInternal (Commander* cmd, ObEntry* objects, ObEntry* regionsOfInfluence, int nObs,  int nROI, const IntervalInfo& ival)
{
    DPVS_ASSERT (cmd);
    m_commander     = cmd;
    m_impCommander  = cmd->getImplementation();

    DPVS_ASSERT (m_impCommander);

    m_activeROIs = 0;

    DPVS_ASSERT (nObs>=0 && nROI>=0);

    int overlapTests    = 0;                                    // internal statistics counters
    int overlaps        = 0;                                    // internal statistics counters

    //--------------------------------------------------------------------
    // Allocate pointer arrays to the data (so that we can sort just
    // the pointers, not the actual structures)
    //--------------------------------------------------------------------

    TempArray<PObEntry,false> obs(nObs);                        // allocate memory from the stack-like TempAllocator
    TempArray<PObEntry,false> ROIs(nROI);

    for (int i = 0; i < nObs; i++)
    {
        DPVS_ASSERT(objects[i].m_left <= objects[i].m_right);
        obs[i].m_ptr = objects + i;
    }
    
    for (int i = 0; i < nROI; i++)
    {
        DPVS_ASSERT(regionsOfInfluence[i].m_left <= regionsOfInfluence[i].m_right);
        ROIs[i].m_ptr = regionsOfInfluence+i;
    }

    //--------------------------------------------------------------------
    // Sort the objects and ROIs separately based on their "left" values
    // on the primary axis
    //--------------------------------------------------------------------

    sort (&obs[0], nObs);
    sort (&ROIs[0], nROI);

    //--------------------------------------------------------------------
    // Find out how many ROI instances we're going to have (knowing
    // this in advance simplifies the temporary memory allocation code).
    //--------------------------------------------------------------------

    int nROIInstances   = 0;
    int nBuckets        = ival.m_xBuckets * ival.m_yBuckets;

    for (int i = 0; i < nROI; i++)
    {
        const ObEntry* r = ROIs[i].m_ptr;
        nROIInstances += (r->m_x1 - r->m_x0) * (r->m_y1 - r->m_y0);
    }
    
    //--------------------------------------------------------------------
    // Perform temporary memory allocations
    //--------------------------------------------------------------------

    TempArray<Bucket,false>         bucket(nBuckets);
    TempArray<ROIInstance,false>    instAlloc(nROIInstances);
    TempArray<ObEntry*,false>       cleanups(nROI*2);
    ObEntry**                       cleanupList     = &cleanups[0];
    ObEntry**                       cleanupList2    = &cleanups[nROI];
    int                             cleanupCount    = 0;
    int                             cleanupCount2   = 0;
    
    //--------------------------------------------------------------------
    // Clear bucket structures
    //--------------------------------------------------------------------

    for (int i = 0; i < nBuckets; i++)
    {   
        bucket[i].m_head = null;
        bucket[i].m_tail = null;
    }

    //--------------------------------------------------------------------
    // Create ROI instances for each bucket a ROI overlaps
    //--------------------------------------------------------------------

    int allocIndex = 0;
    for (int i = 0; i < nROI; i++)
    {
        ObEntry* r = ROIs[i].m_ptr;

        for (int y = r->m_y0; y < r->m_y1; y++)                     // loop through all intersected buckets..
        for (int x = r->m_x0; x < r->m_x1; x++)
        {
            Bucket& b = bucket[x + y * ival.m_xBuckets];

            ROIInstance* instance = &instAlloc[allocIndex++];       // allocate instance
            DPVS_ASSERT(allocIndex <= nROIInstances);               // debug debug
            
            instance->m_ROI  = r;
            instance->m_next = null;
            if (b.m_tail) 
                b.m_tail->m_next = instance;
            b.m_tail = instance;
            if (!b.m_head)
                b.m_head = instance;
        }
    }
    DPVS_ASSERT(nROIInstances == allocIndex);           // DEBUG DEBUG

    int cnt = 0;

    //--------------------------------------------------------------------
    // Now iterate through all objects in front->back order (along the 
    // primary axis). If an object touches multiple buckets, we evaluate 
    // the object's overlaps in all of the buckets.
    //--------------------------------------------------------------------

    for (int i = 0; i < nObs; i++)
    {
        const ObEntry* object = obs[i].m_ptr;                                           // pointer to ObEntry
        const AABB&    aabb   = object->m_ob->getCellSpaceAABB();                       // object's bounding box
        Sphere         sphere(aabb.getCenter(), aabb.getDiagonalLength()*0.5f);         // object's (approximated) bounding sphere

        for (int y = (int)object->m_y0; y < (int)object->m_y1; y++)
        for (int x = (int)object->m_x0; x < (int)object->m_x1; x++)
        {
            Bucket& b = bucket[x + y * ival.m_xBuckets];

            //----------------------------------------------------------------
            // Iterate through all active ROIs in the bucket. The iteration can 
            // be stopped immediately when we go past the "right" edge of the 
            // object
            //----------------------------------------------------------------

            ROIInstance* prev = null;                                   // keep track of previous entry so we can manage the singly-linked list without rescanning

            for (ROIInstance* roi = b.m_head; roi; roi = roi->m_next)
            {
                ObEntry* r = roi->m_ROI;                                // pointer to actual ROI

                //----------------------------------------------------------------
                // See if we've gone past a ROI (remove it from the active list)
                //----------------------------------------------------------------
                
                if (r->m_left > object->m_right)                        // we went past 
                    break;
                else
                if (r->m_right < object->m_left)                        // we can kill the ROI
                {
                    setROIActive(*r,false);                             // turn ROI off
                    if (prev) 
                        prev->m_next = roi->m_next; 
                    else
                        b.m_head = roi->m_next;                         // new head
                } 
                else
                {
                    //----------------------------------------------------------------
                    // Perform bounding box intersection test. First check if the
                    // object has already been tested against the ROI by testing 
                    // the ROI's mail box value
                    //----------------------------------------------------------------

                    overlapTests++;                                         // inc counter here because reading the mailbox value is work as well..
    
                    if (r->m_ROIData->m_mailBox != object)
                    {
                        r->m_ROIData->m_mailBox = object;

                        //----------------------------------------------------------------
                        // Bounding box and bounding sphere tests  (we do both, as spheres
                        // tend to bound the ROIs better than AABBs).
                        //----------------------------------------------------------------

                        bool active = 
                            intersect(sphere,r->m_ROIData->m_sphere) &&
                            intersect(aabb, r->m_ob->getCellSpaceAABB());

                        //----------------------------------------------------------------
                        // If ROI is active, add it to the cleanup list
                        //----------------------------------------------------------------

                        if (active)
                        {
                            DPVS_ASSERT(cleanupCount2>=0 && cleanupCount2 < nROI);
                            cleanupList2[cleanupCount2++] = r;
                            overlaps++;                                 // overlap detected
                        }

                        //----------------------------------------------------------------
                        // If the status of the ROI changes, inform user about the change
                        //----------------------------------------------------------------

                        if (active != r->m_ROIData->m_active)                       
                            setROIActive(*r, active);                   
                    }
                }
                prev = roi;                                             // prev entry
            }
        }

        //----------------------------------------------------------------
        // Turn off ROIs that were activated by previous object but aren't
        // active for the current object
        //----------------------------------------------------------------

        for (int j = 0; j < cleanupCount; j++)
        {
            if (cleanupList[j]->m_ROIData->m_mailBox != object)
                setROIActive (*cleanupList[j],false);                   // turn ROI off
        }

        //----------------------------------------------------------------
        // Double-buffering of the cleanup lists (just a pointer swap)
        //----------------------------------------------------------------

        swap (cleanupList, cleanupList2);

        cleanupCount  = cleanupCount2;
        cleanupCount2 = 0;

        //----------------------------------------------------------------
        // Inform commander that the object is visible
        //----------------------------------------------------------------

        m_impCommander->setInstanceObject   (object->m_ob);
		#if defined (DPVS_DEBUG)
			UINT32 oldFPUMode = getFPUMode();
		#endif
        m_commander->command                (Commander::INSTANCE_VISIBLE);
		DPVS_ASSERT(getFPUMode() == oldFPUMode && "FPU mode changed in user callback!");

        cnt += m_activeROIs;                        
        DPVS_ASSERT( cnt == overlaps);              // just checking...
    }

    //----------------------------------------------------------------
    // Here we disable the active ROIs of the last object so that all
    // ROIs in the world are disabled when this function exits...
    //----------------------------------------------------------------

    for (int i = 0; i < cleanupCount; i++)
        setROIActive (*cleanupList[i],false);       // turn off

    DPVS_ASSERT(m_activeROIs == 0);                 // make sure everything went smoothly

    //----------------------------------------------------------------
    // Cleanup and statistics updates
    //----------------------------------------------------------------

    DPVS_PROFILE (Statistics::incStatistic(Library::STAT_ROIOBJECTOVERLAPTESTS,overlapTests));
    DPVS_PROFILE (Statistics::incStatistic(Library::STAT_ROIOBJECTOVERLAPS,overlaps));

    m_impCommander->setInstanceObject(null);        // trap some errors later (?)
    m_commander     = null;
    m_impCommander  = null;

/*  
    // display statistics directly to stdout
    printf ("overlaps        = %d\n",overlaps);
    printf ("check           = %d\n",cnt);
    printf ("overlaps/object = %.3f\n",(double)(overlaps) / nObs);
    printf ("tests/object    = %.3f\n",(double)(overlapTests) / nObs);
*/
}

//------------------------------------------------------------------------
// Wrapper code (just re-directs calls to the implementation class).
//------------------------------------------------------------------------

SweepAndPrune::SweepAndPrune (void) : m_imp(NEW<ImpSweepAndPrune>())
{
    // nada
}

SweepAndPrune::~SweepAndPrune (void)
{
    DELETE(m_imp);
    m_imp = null;
}

void SweepAndPrune::minimizeMemoryUsage (void)
{
    ImpSweepAndPrune::minimizeMemoryUsage();
}

void SweepAndPrune::process (Commander* cmd, const ImpObject* const * obs, const ImpObject* const* ROIs, int nObs, int nROI, const Vector3& primarySweepDirection)
{
    m_imp->process(cmd,obs,ROIs,nObs,nROI,primarySweepDirection);
}

//------------------------------------------------------------------------
