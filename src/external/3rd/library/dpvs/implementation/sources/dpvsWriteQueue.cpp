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
 * Description: 	Occlusion buffer write queue code
 *
 * $Archive: /dpvs/implementation/sources/dpvsWriteQueue.cpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 4/22/02 6:43p $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#include "dpvsWriteQueue.hpp"
#include "dpvsMath.hpp"
#include "dpvsOcclusionBuffer.hpp"
#include "dpvsImpObject.hpp"
#include "dpvsImpCamera.hpp"
#include "dpvsSort.hpp"
#include "dpvsSpaceManager.hpp"
#include "dpvsVisibilityQuery.hpp"
#include "dpvsDebug.hpp"
#include "dpvsRandom.hpp"
#include "dpvsScratchpad.hpp"


//------------------------------------------------------------------------
// Some private defines affecting the compilation
//------------------------------------------------------------------------

// DEBUG DEBUG TIMO: WANT TO MAKE THESE GLOBAL (TO DPVSPRIVATEDEFS.HPP) ???

// This is a test for getting rid of some of the worst parasites
#define PARASITE_TEST


//#define EMERGENCY_POLICY_REJECT
#define DPVS_EXACT_FLUSH
#define DPVS_POSTPONE_Z

// FOR STATISTICS:
#define DPVS_INTERPOLATE_Z
#define DPVS_OBB_SILHOUETTE

//#define TIMO_PAINTERS_TEST
#ifdef TIMO_PAINTERS_TEST		// FOR TIMO's TESTS
Range<float> g_depthRange;
bool		 g_depthRangeValid;
#endif

using namespace DPVS;

// DEBUG DEBUG

class FlashLim
{
public:

	enum
	{
		SLOTS = WriteQueue::FLASH_RECTANGLE_Z
	};

	FlashLim()
	{
		m_lim[0]	 = 0.f;
		m_lim[SLOTS] = 1.f;

		for(int i=1;i<SLOTS;i++)
			m_lim[i] = 0.98f + 0.02f*(i-1.f)/(SLOTS-1);

		reset();
	}

	void	adapt()
	{
		for(int i=0;i<SLOTS;i++)
			m_sum[i] = m_cnt[i] ? m_sum[i]/m_cnt[i] : 0.f;

		for(int i=1;i<SLOTS;i++)
		{
			int wl = m_cnt[i-1];
			int wr = m_cnt[i];

			if(wr==wl)
				continue;

			if(i==1)		// special case for the first slot. (lerping the limits doesn't work)
			{
				if(wl>wr)	m_lim[i] = lerp(m_sum[i-1], m_lim[i],	0.90f);
				else		m_lim[i] = lerp(m_lim[i],	m_lim[i+1], 0.10f);
			}
			else
			{
				if(wl>wr)	m_lim[i] = lerp(m_lim[i-1], m_lim[i],	0.90f);
				else		m_lim[i] = lerp(m_lim[i],	m_lim[i+1], 0.10f);
			}
		}
	
//		dump();
		reset();
	}

	void	add(float zmin,float zmax)
	{
		int i=1;
		for(;i<SLOTS+1;i++)	// start pos
		{
			if(zmin < m_lim[i])
				break;
		}

		for(;i<SLOTS+1;i++)	// end pos
		{
			m_cnt[i-1] += 1;
			m_sum[i-1] += zmin;		// DEBUG DEBUG

			if(zmax < m_lim[i])
				break;
		}
	}

	bool	intersect(int bucket, float zmin,float zmax) const
	{
		DPVS_ASSERT(bucket>=0 && bucket<SLOTS);
		if(zmin > m_lim[bucket+1])	return false;
		if(zmax < m_lim[bucket])	return false;
		return true;
	}


	void	dump()
	{
/*		for(int i=0;i<SLOTS;i++)
			Debug::print("%d %f: %d\n",i,m_lim[i],m_cnt[i]);
		Debug::print("\n");
*/
	}


private:

	void	reset()
	{
		for(int i=0;i<4;i++)
		{
			m_cnt[i] = 0;
			m_sum[i] = 0.f;
		}
	}

	float	lerp(float a, float b, float t)
	{
		return a*(1-t) + b*t;
	}

	float	m_lim[SLOTS+1];
	int		m_cnt[SLOTS];
	float	m_sum[SLOTS];
};


FlashLim	g_flashLim;



// QWORD with full bits
#define FULL_QWORD (QWORD(0xFFFFFFFF,0xFFFFFFFF))	

//------------------------------------------------------------------------
// Debug build consistency test function
//------------------------------------------------------------------------

#if defined (DPVS_DEBUG)

inline void WriteQueue::testConsistency (void) const
{ 
	QWORD bucketMask(0,0); 
	const Guard<Bucket> buckets(m_buckets);
	for(int i=0;i<m_bucketRectangle.area();i++) 
		bucketMask |= buckets[i].getMask(); 
	DPVS_ASSERT(m_manager.getMask()==bucketMask); 
}

#else
DPVS_FORCE_INLINE void WriteQueue::testConsistency (void) const
{

}

#endif // DPVS_DEBUG

/*****************************************************************************
 *
 * Function:		DPVS::WriteQueue::Entry::Entry()
 *
 * Description:		
 *
 *****************************************************************************/

DPVS_FORCE_INLINE WriteQueue::Entry::Entry() :
	mask			(),
	object			(null),
	camera			(null),
	rectangle		(),
	depth			(),
#if defined (DPVS_OCCLUSIONBUFFER_INDEXBUFFER)	
	indexBufferIndex(0),
#endif
	bucketRectangle	()
{
	// nada
}

/*****************************************************************************
 *
 * Function:		DPVS::WriteQueue::Bucket::Bucket()
 *
 * Description:		
 *
 *****************************************************************************/

DPVS_FORCE_INLINE WriteQueue::Bucket::Bucket  (void) : 
	m_entryMask		(),
	m_lastIndex		(-1),
	m_occluderCount	(0)
{
	// nada	
}

/*****************************************************************************
 *
 * Function:		drawSilhouette()
 *
 * Description:		Draws wireframe silhouette into the debug output
 *
 * Parameters:		es		= reference to silhouette
 *					color	= color to use (RGBA)
 *
 * Notes:			This is purely a debug function - NOT the silhouette
 *					rasterizer...
 *
 *****************************************************************************/

static void drawSilhouette (Library::LineType type, EdgeSilhouette& es, const Vector4& colorLeft, const Vector4& colorRight)
{
	if(Debug::getLineDrawFlags() & type)
	{
		const Vector2*	loc = es.getVertices();

		{
			const Vector2i*	il	= es.getEdges(0);
			const int		cl	= es.getEdgeCount(0);

			for(int ix=0;ix<cl;ix++)	
			{
				Vector2 A,B;
				A.x = loc[il[ix].i].x;
				A.y = loc[il[ix].i].y;
				B.x = loc[il[ix].j].x;
				B.y = loc[il[ix].j].y;
				Debug::drawLine(type, A, B, colorLeft);
			}
		}

		{
			const Vector2i*	ir	= es.getEdges(1);
			const int		cr	= es.getEdgeCount(1);

			for(int ix=0;ix<cr;ix++)	
			{
				Vector2 A,B;
				A.x = loc[ir[ix].i].x;
				A.y = loc[ir[ix].i].y;
				B.x = loc[ir[ix].j].x;
				B.y = loc[ir[ix].j].y;
				Debug::drawLine(type, A, B, colorRight);
			}
		}
	}
}

/*****************************************************************************
 *
 * Function:		DPVS::WriteQueue::getClosingDepth()
 *
 * Description:		Returns depth at which the occlusion buffer is fully
 *					closed. If the occlusion buffer is not closed, the
 *					function returns 1.0, i.e. far clip plane distance.
 *					All objects having test values beyond the closing
 *					depth can be discarded (as both occludees and occluders).
 *
 *****************************************************************************/
/*
inline float WriteQueue::getClosingDepth (void) const
{
	if (!m_occlusionBuffer->isFull())				// not full yet, return 1.0
		return 1.0f;
	return m_occlusionBuffer->getClosingDepth();	// query the closing depth
}
*/

/*****************************************************************************
 *
 * Function:		DPVS::WriteQueue::Bucket::getEntries()
 *
 * Description:		
 *
 *****************************************************************************/

DPVS_FORCE_INLINE int WriteQueue::Bucket::getEntries(int *list,const QWORD& clearMask) const
{
	DPVS_ASSERT(list);
	const QWORD mask = m_entryMask & (~clearMask);
	return maskToList(list,mask);
}

/*****************************************************************************
 *
 * Function:		DPVS::WriteQueue::Bucket::appendObject()
 *
 * Description:		
 *
 *****************************************************************************/

DPVS_FORCE_INLINE void WriteQueue::Bucket::appendObject(OccluderManager& bm, ImpObject* o)
{
	const int index = bm.addEntry(o);		// set "o", clears benefit
	bm.link(index,m_lastIndex);				// build reversed linked list
	m_lastIndex = index;
	m_occluderCount++;
}

/*****************************************************************************
 *
 * Function:		DPVS::WriteQueue::Bucket::giveBenefits()
 *
 * Description:		
 *
 *****************************************************************************/

DPVS_FORCE_INLINE int	WriteQueue::Bucket::giveBenefits(const Array<Occluder>& list)
{
	if(m_occluderCount==0)
		return 0;

	int	  counter = 0;
	int   index   = m_lastIndex;
	float benefit = 0.0f;

	do
	{
		DPVS_ASSERT(m_occluderCount>0);
		const Occluder& occ = list[index];
		benefit += occ.benefit/m_occluderCount--;		// benefitForOccluder
		occ.object->addBenefit(benefit);
		index = occ.next;
		counter++;
	} while(index!=-1);

	DPVS_ASSERT(index==-1);

	m_lastIndex = -1;									// NOTE: unlink from the list - Added due to PARASITE check
	return counter;
}

/*****************************************************************************
 *
 * Function:		DPVS::WriteQueue::Manager::allocate()
 *
 * Description:		
 *
 *****************************************************************************/

DPVS_FORCE_INLINE QWORD WriteQueue::Manager::allocate(void)
{
	DPVS_ASSERT(!m_entryMask.full());	//FULL
	const QWORD t = ~m_entryMask;			//zero == free
	QWORD f = t;
	f.dec();								// (t-1)
	f &= t;									// (t&(t-1))
	f ^= t;									// t ^ (t&(t-1))
	m_entryMask |= f;						// mark as reserved
	return f;
}




/*****************************************************************************
 *
 * Function:		DPVS::WriteQueue::WriteQueue()
 *
 * Description:		Constructor
 *
 * Parameters:		buf = pointer to occlusion buffer
 *
 *****************************************************************************/

WriteQueue::WriteQueue	(OcclusionBuffer *occlusionBuffer) :
	m_occlusionBuffer(occlusionBuffer),
	m_buckets(),
	m_bucketAlloc(null),
	m_flashRectangle(),
	m_flashBudget(0.f),
	m_flashZBucket(0),
	m_ooScreenHeight(0.f),
	m_frame(0),
	m_bucketRectangle(),
	m_subSampling(),
	m_overFlow(false),
	m_occluderManager(),
	m_contributed(),
	m_fullyCovered(),
	m_testSilhouette(),
	m_writeSilhouette(),
#if defined (DPVS_OCCLUSIONBUFFER_INDEXBUFFER)	
	m_indexBufferIndex(0),
#endif
	m_manager()
{
	for (int i = 0; i < FLASH_RECTANGLE_FRAMES; i++)
		m_flashOrder[i] = (unsigned char)(i);

	m_flashRectangle.setSpace(SPACE_RASTER);

	DPVS_ASSERT(occlusionBuffer);

	m_occlusionBuffer->getResolution	(m_bucketRectangle,m_subSampling);
	
	DPVS_ASSERT(m_bucketRectangle.getSpace() == SPACE_OCCLUSION_BUCKET);
	DPVS_ASSERT(m_subSampling.x > 0.0 && m_subSampling.y > 0.0);

	int area = m_bucketRectangle.area();

	//--------------------------------------------------------------------
	// If the buckets use less than 2560 bytes, we place them into the
	// scratchpad.
	//--------------------------------------------------------------------

	if ((area*sizeof(Bucket)) <= 2560)
	{
		m_buckets		= Guard<Bucket>(reinterpret_cast<Bucket*>(Scratchpad::getAddress(Scratchpad::WRITEQUEUE_BUCKETS)), 0, area);
		m_bucketAlloc	= null;
	} else
	{
		m_bucketAlloc	= NEW_ARRAY<Bucket>(area);
		m_buckets		= Guard<Bucket>(m_bucketAlloc, 0, area);
	}

	m_contributed.reset (m_bucketRectangle.area());
	m_fullyCovered.reset(m_bucketRectangle.area());

	DPVS_ASSERT(m_manager.isEmpty());

	clear(null);
}

/*****************************************************************************
 *
 * Function:		DPVS::WriteQueue::~WriteQueue()
 *
 * Description:		Destructor
 *
 *****************************************************************************/

WriteQueue::~WriteQueue()
{
	DELETE_ARRAY(m_bucketAlloc);	// if buckets were allocated from the heap, release them...
	DPVS_ASSERT(!m_overFlow);		// ?? HUH ?? 
}

/*****************************************************************************
 *
 * Function:		DPVS::WriteQueue::clear()
 *
 * Description:		
 *
 *****************************************************************************/

void WriteQueue::clear		(ImpCamera *c)
{
	// DEBUG DEBUG REMOVE
//	Debug::print("-------------------------------------------\n");

	g_flashLim.adapt();
	m_flashBudget = (float)ImpObject::FLASH_BUDGET;

	//--------------------------------------------------------------------
	// Clear occlusion buffer
	//--------------------------------------------------------------------

	m_occlusionBuffer->setScissor(null);
	m_occlusionBuffer->setScissor(c);		//must call twice to fool the internal mechanism
	m_occlusionBuffer->clear();

	m_occlusionBuffer->setScissor(null);

	//--------------------------------------------------------------------
	// Clear buckets
	//--------------------------------------------------------------------

	Guard<Bucket> buckets(m_buckets);
	for(int i=m_bucketRectangle.area()-1;i>=0;i--)
	{
		buckets[i].clear(FULL_QWORD);
		buckets[i].clearList();
	}

	//--------------------------------------------------------------------
	// Clear memory allocator and statistics
	//--------------------------------------------------------------------

	m_manager.free(FULL_QWORD);		//free all
	m_occluderManager.clear();

	testConsistency();

#if defined (DPVS_OCCLUSIONBUFFER_INDEXBUFFER)	
	m_indexBufferIndex = 0;
#endif

	//--------------------------------------------------------------------
	// Generate the flash rectangle. Randomize the selection order every
	// Nth frame...
	//--------------------------------------------------------------------


	if (c)
	{
		m_frame++;
		if (m_frame==FLASH_RECTANGLE_FRAMES)
		{
			m_frame = 0;
			for (int i = 0; i < FLASH_RECTANGLE_FRAMES; i++)
				m_flashOrder[i] = (unsigned char)(i);
			for (int i = 0; i < FLASH_RECTANGLE_FRAMES; i++)
				swap(m_flashOrder[i],m_flashOrder[g_random.getI()%FLASH_RECTANGLE_FRAMES]);
		}

		DPVS_ASSERT(m_frame>=0 && m_frame<FLASH_RECTANGLE_FRAMES);

		int frame = m_flashOrder[m_frame];

		const FloatRectangle&	screen  = c->getRasterViewport();
		float					xOffset = ((frame / FLASH_RECTANGLE_Z) % FLASH_RECTANGLE_X     )/(float)(FLASH_RECTANGLE_X);
		float					yOffset = (frame / (FLASH_RECTANGLE_Z*FLASH_RECTANGLE_X) % FLASH_RECTANGLE_Y )/(float)(FLASH_RECTANGLE_Y);
		m_flashZBucket					= (frame % FLASH_RECTANGLE_Z);

		m_ooScreenHeight	= 1.f/screen.height();

		m_flashRectangle.x0 = screen.x0 + xOffset * screen.width();
		m_flashRectangle.x1 = screen.x0 + (xOffset+1.0f/FLASH_RECTANGLE_X) * screen.width();
		m_flashRectangle.y0 = screen.y0 + yOffset * screen.height();
		m_flashRectangle.y1 = screen.y0 + (yOffset+1.0f/FLASH_RECTANGLE_Y) * screen.height();

/*
		float col = m_flashZBucket / 4.0f + 0.25f;

		Debug::drawLine(Library::LINE_MISC,
			Vector2(m_flashRectangle.x0,m_flashRectangle.y0), 
			Vector2(m_flashRectangle.x1,m_flashRectangle.y0), 
			Vector4(col,col,col,1.0f));
		Debug::drawLine(Library::LINE_MISC,
			Vector2(m_flashRectangle.x0,m_flashRectangle.y1), 
			Vector2(m_flashRectangle.x1,m_flashRectangle.y1), 
			Vector4(col,col,col,1.0f));
		Debug::drawLine(Library::LINE_MISC,
			Vector2(m_flashRectangle.x0,m_flashRectangle.y0), 
			Vector2(m_flashRectangle.x0,m_flashRectangle.y1), 
			Vector4(col,col,col,1.0f));
		Debug::drawLine(Library::LINE_MISC,
			Vector2(m_flashRectangle.x1,m_flashRectangle.y0), 
			Vector2(m_flashRectangle.x1,m_flashRectangle.y1), 
			Vector4(col,col,col,1.0f));
*/
	}
}


/*****************************************************************************
 *
 * Function:		DPVS::WriteQueue::allocateEntry()
 *
 * Description:		
 *
 *****************************************************************************/

DPVS_FORCE_INLINE WriteQueue::Entry& WriteQueue::allocateEntry(void)
{
	const QWORD	entryMask	= m_manager.allocate();
	const int	entryNum	= getHighestSetBit(entryMask);
	Entry& e				= m_entries[entryNum];
	e.mask					= entryMask;
	return e;
}

/*****************************************************************************
 *
 * Function:		DPVS::WriteQueue::getBucketRectangle()
 *
 * Description:		
 *
 *****************************************************************************/

DPVS_FORCE_INLINE void WriteQueue::getBucketRectangle(IntRectangle& br, const IntRectangle &r)
{
	DPVS_ASSERT(r.area()>0);
	DPVS_ASSERT(r.x0 < r.x1);
	DPVS_ASSERT(r.y0 < r.y1);

	SpaceManager::occlusionToBucket(br,r);

	DPVS_ASSERT(br.x0 >= 0);
	DPVS_ASSERT(br.y0 >= 0);
	DPVS_ASSERT(br.x1 <= m_bucketRectangle.x1);
	DPVS_ASSERT(br.y1 <= m_bucketRectangle.y1);
	DPVS_ASSERT(br.x0 < br.x1);
	DPVS_ASSERT(br.y0 < br.y1);
	DPVS_ASSERT(br.area() > 0);
}

/*****************************************************************************
 *
 * Function:		DPVS::WriteQueue::setBenefitToRectangle()
 *
 * Description:		
 *
 *****************************************************************************/

void WriteQueue::setBenefitToRectangle(const IntRectangle &rect,float benefitForRectangle)
{
	DPVS_ASSERT(rect.getSpace() == SPACE_OCCLUSION_BUCKET);
	DPVS_ASSERT(rect.area() > 0);

	//Debug::print ("adding %.5f benefit\n",benefitForRectangle);

	float				benefitForBucket	= benefitForRectangle / rect.area();
	int					by					= rect.y0 * m_bucketRectangle.x1;
	Guard<Occluder>		occluderList(m_occluderManager.m_occluders);
	Guard<Bucket>		buckets(m_buckets);

	for(int y=rect.y0; y<rect.y1; y++,by+=m_bucketRectangle.x1)
	for(int x=rect.x0; x<rect.x1; x++)
	{
		const int lastIndex = buckets[by+x].getLastIndex();
		if(lastIndex==-1)
			continue;
		occluderList[lastIndex].benefit += benefitForBucket;
	}
}


/*****************************************************************************
 *
 * Function:		DPVS::WriteQueue::evaluateBenefits()
 *
 * Description:		Gives benefits to succesful occluders
 *
 *****************************************************************************/

void WriteQueue::evaluateBenefits(void)
{
	const Array<Occluder>&	occluderList	= m_occluderManager.m_occluders;
	Guard<Bucket>			buckets(m_buckets);
	const int				area			= m_bucketRectangle.area(); 
	for(int i=0;i<area;i++)
		buckets[i].giveBenefits(occluderList);
}

/*****************************************************************************
 *
 * Function:		DPVS::WriteQueue::clearMaskFromBuckets()
 *
 * Description:		
 *
 *****************************************************************************/

void WriteQueue::clearMaskFromBuckets(const DPVS::IntRectangle &rect, const QWORD mask)
{
	int		pitch	= m_bucketRectangle.x1;
	int		width	= rect.x1 - rect.x0;
	int		height	= rect.y1 - rect.y0;

	Bucket* b = &m_buckets[rect.y0 * pitch + rect.x0];
	
	for(int y = 0; y < height; y++, b += pitch)
	for(int x = 0; x < width;  x++)
		b[x].clear(mask);
}

/*****************************************************************************
 *
 * Function:		DPVS::WriteQueue::outOfCapacity(float flushDepth)
 *
 * Description:		Function for resolving the case when the write queue
 *					becomes 100% full.
 *
 * Parameters:
 *
 * Returns:
 *
 *****************************************************************************/

DPVS_FORCE_INLINE void WriteQueue::outOfCapacity(float flushDepth)
{
	DPVS_ASSERT( m_manager.isFull());

	//-----------------------------------------------------
	// Flush objects that are closer to 'flushDepth'. Note
	// that we currently have the separate "overflow"
	// policy -- it will not flush all of the objects,
	// if too many are closer than 'flushDepth'.
	//-----------------------------------------------------

	m_overFlow = true;											// turn on emergency policy

#ifdef EMERGENCY_POLICY_REJECT
	Guard<Bucket> buckets(m_buckets);
	for(int i=0;i<m_bucketRectangle.area();i++)
		buckets[i].clear(FULL_QWORD);
	m_manager.free(FULL_QWORD);

#else

	IntRectangle intRectangle(0,0,0,0);
	intRectangle.setSpace(SPACE_OCCLUSION);

	testConsistency();
	flush(m_bucketRectangle,intRectangle,flushDepth,false);	//flush everything closer
	testConsistency();

#endif

	DPVS_PROFILE(Statistics::incStatistic(Library::STAT_WRITEQUEUEOVERFLOW,1));

	//-----------------------------------------------------
	// If it didn't help, which is very unlikely to happen, do an emergency
	// flush. This event will be costly and should be prevented...
	// Hmmm.. we'll see later if this actually ever happens.
	//-----------------------------------------------------

	if(m_manager.isFull())
	{
#ifdef EMERGENCY_POLICY_REJECT

		Guard<Bucket> buckets(m_buckets);
		for(int i=0;i<m_bucketRectangle.area();i++)
			buckets[i].clear(FULL_QWORD);
		m_manager.free(FULL_QWORD);

#else

		DPVS_ASSERT(m_bucketRectangle.area()>0);
		testConsistency();
		flush(m_bucketRectangle,intRectangle,FLT_MAX,false);
		testConsistency();

#endif

		DPVS_PROFILE(Statistics::incStatistic(Library::STAT_WRITEQUEUEOVERFLOWEVERYTHING,1));
	}

	DPVS_ASSERT(!m_manager.isFull());

	m_overFlow = false;										// turn off emergency policy
}

/*****************************************************************************
 *
 * Function:		DPVS::WriteQueue::forceFlush(FloatRectangle& rect)
 *
 * Description:
 *
 * Parameters:
 *
 *****************************************************************************/

void WriteQueue::forceFlush		(const FloatRectangle& rectangle)
{
	DPVS_ASSERT(rectangle.getSpace() == SPACE_RASTER);

	const float sx = (float)(m_subSampling.x)/BUCKET_WIDTH;
	const float sy = (float)(m_subSampling.y)/BUCKET_HEIGHT;
	IntRectangle bucketRectangle;
	bucketRectangle.x0 = Math::intFloor(rectangle.x0 * sx);
	bucketRectangle.y0 = Math::intFloor(rectangle.y0 * sy);
	bucketRectangle.x1 = Math::intCeil (rectangle.x1 * sx);
	bucketRectangle.y1 = Math::intCeil (rectangle.y1 * sy);
	bucketRectangle.setSpace(SPACE_OCCLUSION_BUCKET);

	IntRectangle intRectangle(0,0,0,0);
	intRectangle.setSpace(SPACE_OCCLUSION);

	testConsistency();
	flush(bucketRectangle,intRectangle,FLT_MAX,false);
	testConsistency();
}

/*****************************************************************************
 *
 * Function:		DPVS::WriteQueue::appendOccluder(ImpObject*,ImpCamera*)
 *
 * Description:		Conditionally insert object into the occluder write queue
 *
 * Parameters:		o	= pointer to object
 *					c	= pointer to camera
 *
 *****************************************************************************/

void WriteQueue::appendOccluder	(void)
{
	DPVS_ASSERT(VQData::get().testProperties(VQData::FLOW_ACTIVE));
	DPVS_ASSERT(VQData::get().getObject());
	DPVS_ASSERT(VQData::get().getCamera());

	ImpObject* o = VQData::get().getObject();
	if(m_occlusionBuffer->isFull() || !o->isOcclusionWritable())
		return;

	FloatRectangle	rectangle;						// XY-rectangle of the object (will be computed later if necessary)
	bool			rectangleComputed = false;		// has the rectangle been computed yet?

	//--------------------------------------------------------------------
	// Test whether object is a good occluder or not. If object is found
	// to be a bad occluder, return immediately.
	//--------------------------------------------------------------------

#if defined (DPVS_OCCLUDER_SELECTION)

	//-----------------------------------------------------
	// Get depth range for the object.
	//-----------------------------------------------------

	SilhouetteMath::computeOBBDepthRange_FLOW();
	Range<float> zRange = VQData::get().getDepthRange();

	if (zRange.getMax() < 0.0f || zRange.getMin() > 1.0f)		// no go
		return;

	zRange.clampToUnit();

	// update flash z-estimator..
	g_flashLim.add(zRange.getMin(),zRange.getMax());

	if(!o->goodOccluder(true))
	{
		//-----------------------------------------------------------------
		// NOTE: An object getting into here is almost certainly a crappy occluder.
		// Still, we must try objects once in a while just to see if things have changed
		// since the previous information was collected.
		//
		// In order to maximize the occluder fusion, we use "localized" flashing.
		// Any object intersecting a flash volume is a valid occluder candidate.
		//-----------------------------------------------------------------

		// check if object intersects the flash Z range or if we don't have any flash
		// budget left..

		if (m_flashBudget <= 0.0f || !g_flashLim.intersect(m_flashZBucket,zRange.getMin(), zRange.getMax()))
			return;

		//-----------------------------------------------------------------
		// Russian roulette selection against the remaining flash budget
		//
		// Motivation:
		// To provide a bounded processing time for crappy objects.
		// - When m_flashBudget reaches zero, no occluder will be accepted.
		// - When m_flashBudget is above zero, russian roulette may allow
		//   objects with upto 5xm_flashBudget to be tried. This is exponentially
		//   unlikely.
		//-----------------------------------------------------------------

		{

			const float hardLimit = 1.f / (5*5);
			float c = o->estimateWriteCost();
			float ooc = c!=0.f ? 1.f/c : 0.f;
			float p = m_flashBudget * ooc;						// m_flashBudget / o->estimateWriteCost()
			if(p>1.f) p=1.f;
			p*=p;												// double the budget -> 25% change

			if(p<g_random.get() || p<hardLimit)
				return;
		}

		//-----------------------------------------------------------------
		// Compute the object write rectangle and intersect it against the
		// flash rectangle. If the intersection is empty, return (object
		// is not in current flash area).
		//-----------------------------------------------------------------

		if (!rectangleComputed)
		{
			if(!o->getWriteRectangle(rectangle))			// get the write rectangle..
				return;
			rectangleComputed = true;
			DPVS_ASSERT(rectangle.area() > 0.f);
			DPVS_ASSERT(VQData::get().testProperties(VQData::RECTANGLE_CLIPPED));

		}

		FloatRectangle tmp = rectangle;
		if (!tmp.intersect(m_flashRectangle))
			return;

		//-----------------------------------------------------------------
		// If the object hasn't been an occluder during the last 16 frames,
		// it must be tried. Otherwise some very good occluders may fail
		// to get selected because area probability test confuses
		// between them and large crappy objects...
		//-----------------------------------------------------------------

		if(o->getBenefitTimeStamp() - o->getLastOccluderUsedTimeStamp() > 16)	// We don't know!
		{
		}
		else																	// We know!
		{
			//-----------------------------------------------------------------
			// Russian roulette selection based on rectangle area (p ~ 1/n)
			//
			// Motivation:
			// Objects intersecting N flash rectangles should have probability
			// of ~1/N of getting selected during this frame.
			//-----------------------------------------------------------------

			float a   = rectangle.area();
			float ooa = (a!=0.f) ? (1.f/(4*a)) : (0.f);
			float p = m_flashRectangle.area() * ooa;	// m_flashRectangle.area() / (4*rectangle.area())
			if(p>1.f) p=1.f;


			if(p < g_random.get())						// kill according to the estimated size
			{
				return;
			}
		}

		//-----------------------------------------------------------------
		// Adjust flash budget
		//-----------------------------------------------------------------

		o->setLastOccluderUsedTimeStamp(o->getBenefitTimeStamp());			// It will be used!

		m_flashBudget -= o->estimateWriteCost();
		if(m_flashBudget<0.f)
			m_flashBudget = 0.f;
	}

#endif // DPVS_OCCLUDER_SELECTION

	// if write rectangle was not computed yet, do it now...
	if (!rectangleComputed)
	{
		if(!o->getWriteRectangle(rectangle))			
			return;

		DPVS_ASSERT(rectangle.area() > 0.f);
		DPVS_ASSERT(VQData::get().testProperties(VQData::RECTANGLE_CLIPPED));
	}

#ifdef TIMO_PAINTERS_TEST
	{
		int	list[WRITE_QUEUE_SIZE];
		const int cnt = maskToList(list,m_manager.getMask());
		DPVS_PROFILE(Statistics::incStatistic(Library::STAT_HOAX3,cnt));
		DPVS_PROFILE(Statistics::incStatistic(Library::STAT_HOAX4,1));
	}
#endif

	//-----------------------------------------------------
	// If manager is full, perform a flush
	//-----------------------------------------------------

	if(m_manager.isFull())
		outOfCapacity(VQData::get().getDepthRange().getMin());		// max??

	//-----------------------------------------------------
	// Assign object into buckets it overlaps
	//-----------------------------------------------------

	DPVS_PROFILE(Statistics::incStatistic(Library::STAT_WRITEQUEUEWRITESREQUESTED,1));

	Entry& e	= allocateEntry();
	e.object	= o;
	e.camera	= VQData::get().getCamera();
	e.depth		= VQData::get().getDepthRange();
#if defined (DPVS_OCCLUSIONBUFFER_INDEXBUFFER)	
	e.indexBufferIndex	= m_indexBufferIndex++;
#endif

#if defined (DPVS_DEBUG)
	e.debugCode = 0xdeadbabe;
#endif


	SpaceManager::rasterToOcclusion(e.rectangle,rectangle,m_subSampling);
	getBucketRectangle(e.bucketRectangle,e.rectangle);

	//-----------------------------------------------------
	// Mark buckets to entry and entry to bucket
	//-----------------------------------------------------

	Guard<Bucket> b(m_buckets);
	int by = e.bucketRectangle.y0 * m_bucketRectangle.x1;
	for(int y=e.bucketRectangle.y0; y<e.bucketRectangle.y1; y++,by+=m_bucketRectangle.x1)
	for(int x=e.bucketRectangle.x0; x<e.bucketRectangle.x1; x++)
		b[by+x].set(e.mask);

	testConsistency();

	//-----------------------------------------------------
	// Optionally flush immediately (DEBUG)
	//-----------------------------------------------------

#if !defined (DPVS_POSTPONE_OCCLUDERS)
	IntRectangle intRectangle(0,0,0,0);
	intRectangle.setSpace(SPACE_OCCLUSION);
	flush(m_bucketRectangle,intRectangle,FLT_MAX,false);
	testConsistency();
#endif // DPVS_POSTPONE_OCCLUDERS	
}

/*****************************************************************************
 * 
 * Function:		DPVS::WriteQueue::isPointOccluded_IM(const Vector3& v)
 *
 * Description:		Tests if the point specified is occluded (IMMEDIATE MODE)
 *
 * Parameters:		v = reference to point (raster space XY, Z = [0,1])
 *
 * Returns:			true if point is occluded, false otherwise
 *
 *****************************************************************************/

bool WriteQueue::isPointOccluded_IM(const Vector3& v)
{
	DPVS_ASSERT(v.z>=0.f);
	int x = Math::intFloor(v.x*m_subSampling.x);
	int y = Math::intFloor(v.y*m_subSampling.y);
	return m_occlusionBuffer->isPointOccluded(x,y,v.z);		//TODO: scissor??
}

/*****************************************************************************
 *
 * Function:		DPVS::WriteQueue::isRectangleOccluded_IM()
 *
 * Description:		Tests if the rectangle specified is occluded (IMMEDIATE MODE)
 *
 * Notes:			- Cannot flush (immediate mode query)
 *					- input is in raster-space, converted to occlusion-space
 *					- input is inside the view frustum
 *					- Does not set scissor to occlusion buffer
 *
 *****************************************************************************/

bool WriteQueue::isRectangleOccluded_IM	(const FloatRectangle& r, float depth)
{
	IntRectangle ir;
	SpaceManager::rasterToOcclusion(ir,r,m_subSampling);		// RASTER -> OCCLUSION
	return m_occlusionBuffer->isRectangleOccluded(ir,depth);
}

/*****************************************************************************
 *
 * Function:		DPVS::WriteQueue::isPointOccluded(ImpObject *o,const Vector3& v)
 *
 * Description:		Tests is point specified is occluded
 *
 * Parameters:		o = pointer to object where point belongs (can be null)
 *					v = reference to point (RASTER SPACE XY, Z = [0,1])
 *
 * Returns:			true if point is occluded, false otherwise
 *
 *****************************************************************************/

bool WriteQueue::isPointOccluded(const Vector3& v)
{
	DPVS_PROFILE(Statistics::incStatistic(Library::STAT_WRITEQUEUEPOINTQUERIES,1));
	DPVS_ASSERT(VQData::get().testProperties(VQData::FLOW_ACTIVE));
	DPVS_ASSERT(v.z>=0.f);										// must be - not tested anymore!

	int x = Math::intChop(v.x*m_subSampling.x);
	int y = Math::intChop(v.y*m_subSampling.y);

	//-----------------------------------------------------
	// Test if point is already hidden
	//-----------------------------------------------------

	if(m_occlusionBuffer->isPointOccluded(x,y,v.z))				// OCCLUSION SPACE
		return true;

	//-----------------------------------------------------
	// Attempt to flush the write queue
	//-----------------------------------------------------

	if(m_manager.isEmpty() || !VQData::get().testProperties(VQData::ALLOW_FLUSH))
		return false;

	//-----------------------------------------------------
	// Enable exact flushes (rectangle intersection)
	//-----------------------------------------------------

	IntRectangle rectangle(x,x+1,y,y+1);
	rectangle.setSpace(SPACE_OCCLUSION);

	int bx = (unsigned int)(x) / BUCKET_WIDTH;
	int by = (unsigned int)(y) / BUCKET_HEIGHT;
	IntRectangle br(bx,bx+1,by,by+1);
	br.setSpace(SPACE_OCCLUSION_BUCKET);

	testConsistency();

#ifdef TIMO_PAINTERS_TEST
	g_depthRange.set(v.z,v.z);		// DEBUG DEBUG
	g_depthRangeValid = false;
#endif

	if(flush(br,rectangle,v.z,true))
	if(m_occlusionBuffer->isPointOccluded(x,y,v.z))			// OCCLUSION SPACE
		return true;
	testConsistency();
	return false;
}

/*****************************************************************************
 *
 * Function:		DPVS::WriteQueue::isObjectOccluded()
 *
 * Description:		
 *
 *****************************************************************************/

bool WriteQueue::isObjectOccluded	(void)
{
	DPVS_PROFILE(Statistics::incStatistic(Library::STAT_WRITEQUEUEOBJECTQUERIES,1));
	DPVS_ASSERT(VQData::get().testProperties(VQData::FLOW_ACTIVE));
	DPVS_ASSERT(VQData::get().getObject());
	DPVS_ASSERT(VQData::get().getCamera());

	ImpObject*	o = VQData::get().getObject();
	ImpCamera*	c = VQData::get().getCamera();
	bool		allowBenefit = true;

	VQData::get().setProperties	(VQData::VPT_VALID,false);

	//--------------------------------------------------------------------
	// Start by querying a test rectangle.
	//--------------------------------------------------------------------

	FloatRectangle floatRectangle;

	if(!o->getTestRectangle(floatRectangle))
		return true;											

	//--------------------------------------------------------------------
	// Is the test rectangle completely behind the view frustum?
	//--------------------------------------------------------------------

	DPVS_ASSERT(VQData::get().testProperties(VQData::DEPTH_RANGE_VALID));

	if(VQData::get().getDepthRange().getMin() >= 1.0f)
	{
		DPVS_PROFILE(Statistics::incStatistic(Library::STAT_WRITEQUEUEOCCLUDEESDEPTHREJECTED,1));
		return true;
	}

	//--------------------------------------------------------------------
	// Perform contribution culling
	//--------------------------------------------------------------------

	if(o->isContributionCullingEnabled())	// applies to both SCREENSIZE and CONTRIBUTION
	{
		if(VQData::get().testProperties(VQData::PERFORM_SCREENSIZE_CULLING))
		if(floatRectangle.width() < VQData::get().getObjectMinimumCoverage(0) &&
		   floatRectangle.height()< VQData::get().getObjectMinimumCoverage(1))
		   return true;

		m_occlusionBuffer->enableContributionCulling(VQData::get().testProperties(VQData::PERFORM_CONTRIBUTION_CULLING));
	}

	//--------------------------------------------------------------------
	// Clip rectangle to scissor area.
	// NOTE: this must be done AFTER the contribution culling test.
	//--------------------------------------------------------------------

	DPVS_ASSERT(!VQData::get().testProperties(VQData::RECTANGLE_CLIPPED));	// performance check..
	if(!floatRectangle.intersect(VQData::get().getRasterViewport()))			// outside the screen ?
	{
		VQData::get().setProperties(VQData::RECTANGLE_CLIPPED,true);
		m_occlusionBuffer->enableContributionCulling(false);
		return true;													// .. object is outside the VF and thus hidden
	}

	IntRectangle intRectangle;
	SpaceManager::rasterToOcclusion(intRectangle,floatRectangle,m_subSampling);

	//--------------------------------------------------------------------
	// Policy: Rectangle - flush - Rectangle - Silhouette
	//--------------------------------------------------------------------

	IntRectangle bucketRectangle;
	getBucketRectangle(bucketRectangle,intRectangle);

	bool occluded = false;
	float minDepth = VQData::get().getDepthRange().getMin();

	if(m_occlusionBuffer->isRectangleOccluded (intRectangle,minDepth))
	{
		occluded = true;														// rectangle was occluded
	}
	else
	{
		testConsistency();

#ifdef TIMO_PAINTERS_TEST
		g_depthRange = VQData::get().getDepthRange();
		g_depthRangeValid = true;
#endif

		if(VQData::get().testProperties(VQData::ALLOW_FLUSH))
		if(!m_manager.isEmpty() && flush(bucketRectangle,intRectangle,minDepth,true))		// if flushing was allowed & changed the situation in any way
		{
			testConsistency();
			if(m_occlusionBuffer->isRectangleOccluded (intRectangle,minDepth))		// rectangle occluded
				occluded = true;
		}

#ifdef DPVS_OBB_SILHOUETTE
		//-----------------------------------------------------------
		// If the object is still visible - perform a silhouette query
		// and update the Visible Point in the process. The silhouette
		// is queried from the object.
		//-----------------------------------------------------------

		if (!occluded)
		{
			m_testSilhouette.clear();

			if (o->getTestSilhouette(m_testSilhouette))
			{
				// re-query the rectangle (if there's a mismatch between silhouette and test rectangles!)
				m_testSilhouette.getRectangle(floatRectangle);

				if(!floatRectangle.intersect(VQData::get().getRasterViewport()))			// outside the screen?
				{
					VQData::get().setProperties(VQData::RECTANGLE_CLIPPED,true);
					occluded		= true;
					allowBenefit	= false;	// don't give benefit in this case...
				}
				else
				{
					SpaceManager::rasterToOcclusion(intRectangle,floatRectangle,m_subSampling);
					getBucketRectangle(bucketRectangle,intRectangle);			

					// this if() rule here is to ensure that we don't cull away accidentally small objects
					// that don't scan convert exactly as in hardware. The +2 is due to +1.0f in rectangle
					// computation and ceil() operation that sum up to 2 in the worst case.

					if (intRectangle.x1 > (intRectangle.x0+2) && 
						intRectangle.y1 > (intRectangle.y0+2))
					{
						DPVS_ASSERT(VQData::get().testProperties(VQData::DEPTH_RANGE_VALID));

	#ifndef DPVS_INTERPOLATE_Z
						m_occlusionBuffer->setZConstant		(ZGradient::TEST, VQData::get().getDepthRange().getMin());		// TESTING: flat Z
	#else
						m_occlusionBuffer->setZGradients	(ZGradient::TEST, 
															 m_testSilhouette.getPlaneVertices(), 
															 m_testSilhouette.getPlaneCount(),
															 VQData::get().getDepthRange().getMin(),	
															 VQData::get().getDepthRange().getMax());
	#endif
						m_occlusionBuffer->setScissor		(c);
						m_occlusionBuffer->setDirtyRectangle(intRectangle);
						occluded = m_occlusionBuffer->test	(m_testSilhouette);

						if(m_occlusionBuffer->getPreviousVisiblePoint(VQData::get().accessVPT()))
							VQData::get().setProperties	(VQData::VPT_VALID,true);
					}
				}

				// visualize test silhouettes?
				if(Debug::getLineDrawFlags() & Library::LINE_TEST_SILHOUETTES)
				{
					static const Vector4 g_testSilhouetteColorHidden (1.0f,0.3f,0.3f,0.8f);
					static const Vector4 g_testSilhouetteColorVisible(0.4f,0.4f,1.0f,0.8f);

					drawSilhouette (Library::LINE_TEST_SILHOUETTES,m_testSilhouette, 
						occluded ? g_testSilhouetteColorHidden : g_testSilhouetteColorVisible, 
						occluded ? g_testSilhouetteColorHidden : g_testSilhouetteColorVisible);
				}
			}
		}
#endif //DPVS_OBB_SILHOUETTE
	}

	//-----------------------------------------------------------
	// Assign occlusion result
	//-----------------------------------------------------------

	if(occluded && allowBenefit)
	{
		setBenefitToRectangle(bucketRectangle,
			o->getRenderCost() + 
			floatRectangle.area() * ImpObject::RENDER_PIXEL_COST
			);		// Benefits to occluders
	}

	m_occlusionBuffer->enableContributionCulling(false);
	return occluded;
}

/*****************************************************************************
 *
 * Function:		DPVS::WriteQueue::isSilhouetteOccluded (const Vector3*,int,float,float, ImpCamera *c)
 *
 * Description:		Performs silhouette occlusion test
 *
 * Parameters:		vloc = vertex coordinates in 2D (+Z)
 *					cnt	 = number of vertices
 *					cost = cost of rendering contents of the silhouette (used for cost-benefit analysis)
 *					c	 = pointer to camera
 *
 * Returns:			true if silhouette is occluded, false otherwise
 *
 * Notes:			Silhouette must be convex and vertices in clock-wise order
 *					Silhouette must be front-clipped !
 *
 *****************************************************************************/

// DEBUG DEBUG DEBUG ASSUMES PLANAR SILHOUETTE?

bool WriteQueue::isSilhouetteOccluded(const Vector3* vloc, int cnt,float cost)
{
	DPVS_ASSERT(VQData::get().testProperties(VQData::FLOW_ACTIVE));
	ImpCamera *c = VQData::get().getCamera();
	DPVS_ASSERT(c);
	DPVS_ASSERT(vloc && c && cnt >= 3);

	DPVS_PROFILE	(Statistics::incStatistic(Library::STAT_WRITEQUEUESILHOUETTEQUERIES,1));

	VQData::get().setProperties	(VQData::VPT_VALID,false);

	//--------------------------------------------------------------------
	// Calculate axis-aligned bounding rectangle from the vertex locations
	// and scan minimum Z
	//--------------------------------------------------------------------

	FloatRectangle	floatRectangle(vloc[0].x,vloc[0].x,vloc[0].y,vloc[0].y);
	float minDepth = vloc[0].z;
	float maxDepth = vloc[0].z;

	for(int i=1;i<cnt;i++)
	{
		floatRectangle.grow(vloc[i].x,vloc[i].y);					//input bounds

		minDepth = Math::min(minDepth, vloc[i].z);					// find minZ
		maxDepth = Math::max(maxDepth, vloc[i].z);					// find maxZ
	}

	if (maxDepth < 0.0f || minDepth >= 1.0f)
	{
		DPVS_PROFILE(Statistics::incStatistic(Library::STAT_WRITEQUEUEOCCLUDEESDEPTHREJECTED,1));
		return true;
	}

	minDepth = Math::max(minDepth,0.0f);
	maxDepth = Math::min(maxDepth,1.0f);

	floatRectangle.x1++;	//make exclusive end
	floatRectangle.y1++;	//make exclusive end
	floatRectangle.setSpace(SPACE_RASTER);

	//--------------------------------------------------------------------
	// Intersect rectangle with view frustum, return 'hidden' if outside
	//--------------------------------------------------------------------

	if(!floatRectangle.intersect(VQData::get().getRasterViewport()))
		return true;

	VQData::get().setProperties(VQData::RECTANGLE_CLIPPED,true);

	//--------------------------------------------------------------------
	// Clip rectangle to scissor area
	//--------------------------------------------------------------------

	IntRectangle intRectangle;
	SpaceManager::rasterToOcclusion(intRectangle,floatRectangle,m_subSampling);

	//--------------------------------------------------------------------
	// Policy: Rectangle - flush - Rectangle - Silhouette
	//--------------------------------------------------------------------

	IntRectangle bucketRectangle;
	getBucketRectangle(bucketRectangle,intRectangle);

	bool occluded = false;

	if(m_occlusionBuffer->isRectangleOccluded (intRectangle,minDepth))			// rectangle occluded?
		occluded = true;
	else
	{
		testConsistency();

#ifdef TIMO_PAINTERS_TEST
		g_depthRange = VQData::get().getDepthRange();
		g_depthRangeValid = true;
#endif

		if(VQData::get().testProperties(VQData::ALLOW_FLUSH))
		if(!m_manager.isEmpty() && flush(bucketRectangle,intRectangle,minDepth,true))	// flushing was potentially uselful
		{
			testConsistency();
			if(m_occlusionBuffer->isRectangleOccluded (intRectangle,minDepth))	// rectangle occluded?
				occluded = true;
		}
#ifdef DPVS_OBB_SILHOUETTE
		if(!occluded)
//		if(e.rectangle.area()>=4.f)									// area threshold
//		if(e.rectangle.width()>=2.f && e.rectangle.height()>=2.f)		// w & h threshold
		{
			m_testSilhouette.clear();
			m_testSilhouette.setVertices(vloc,cnt);

			for(int i=0;i<cnt;i++)
			{
				int A = i;
				int B = i+1;			// B = (i+1)%cnt
				if (B == cnt)
					B = 0;
				m_testSilhouette.addEdge(vloc[A].y < vloc[B].y ? 1 : 0,A,B);
			}

#ifndef DPVS_INTERPOLATE_Z
			m_occlusionBuffer->setZConstant			(ZGradient::TEST, minDepth);		// TESTING: flat Z
#else
			const int	loc0 = 0;						// select three vertices to compute test gradient
			const int loc2 = 2*cnt/3;
			const int loc1 = loc2>>1;
			m_occlusionBuffer->setZGradient			(ZGradient::TEST, vloc[loc0],vloc[loc1],vloc[loc2],minDepth,maxDepth);
#endif
			m_occlusionBuffer->setScissor			(c);
			m_occlusionBuffer->setDirtyRectangle	(intRectangle);
			occluded = m_occlusionBuffer->test		(m_testSilhouette);

			if(m_occlusionBuffer->getPreviousVisiblePoint(VQData::get().accessVPT()))
				VQData::get().setProperties	(VQData::VPT_VALID,true);
		}
#endif //DPVS_OBB_SILHOUETTE
	}

	//-----------------------------------------------------------
	// Assign occlusion result
	//-----------------------------------------------------------

	if(occluded)
	{
		setBenefitToRectangle(bucketRectangle,cost);						//benefits to occluders
	}

	return occluded;
}


/*****************************************************************************
 *
 * Function:		DPVS::WriteQueue::flush (const IntRectangle&,float,const ImpObject*)
 *
 * Description:		Attempts to flush the write queue
 *
 * Parameters:		rect		= flush rectangle
 *					z			= flush test depth value (near value)
 *
 * Returns:			boolean value indicating whether the flush changed contents of the occlusion buffer
 *
 *****************************************************************************/

namespace DPVS
{
	//-----------------------------------------------------------
	// class for sorting write queue entries
	//-----------------------------------------------------------

	struct WriteQueue::SortEntry
	{
		bool	operator>	(const SortEntry& s) const		{ return depth > s.depth; }
		bool	operator<	(const SortEntry& s) const		{ return depth < s.depth; }

		int		entryNum;
		float	depth;			// DEBUG DEBUG DEBUG USE DWORD COMPARISONS INSTEAD??
	};
}

void WriteQueue::sortFlushed	(SortEntry* d, int N)
{
	quickSort(d,N);
}

// Flush pending writes inside given rectangle having zvalue <= z
// NOTES: does NOT clip BUCKET RESOLUTION rectangle

bool WriteQueue::flush	(const DPVS::IntRectangle &bucketRectangle,const DPVS::IntRectangle &intRectangle,float flushDepth,bool exactOverlap)
{
	DPVS_ASSERT(VQData::get().testProperties(VQData::FLOW_ACTIVE));
	DPVS_ASSERT(bucketRectangle.getSpace() == SPACE_OCCLUSION_BUCKET);
	DPVS_ASSERT(intRectangle.getSpace() == SPACE_OCCLUSION);

	if(m_manager.isEmpty() || bucketRectangle.area()==0)
		return false;

	DPVS_PROFILE(Statistics::incStatistic(Library::STAT_WRITEQUEUEFLUSHES,1));

	//--------------------------------------------------------------------
	// Occlusion buffer is full, free everything and return (occluders
	// cannot improve the buffers in any way).
	//--------------------------------------------------------------------

	Guard<Bucket> buckets(m_buckets);

	if(m_occlusionBuffer->isFull())
	{
		for(int i=0;i<m_bucketRectangle.area();i++)
			buckets[i].clear(FULL_QWORD);

		m_manager.free(FULL_QWORD);
		testConsistency();
		return false; 
	}

	//--------------------------------------------------------------------
	// Local data
	//--------------------------------------------------------------------

	SortEntry	sort[WRITE_QUEUE_SIZE];		// entries used for sorting
	int			sortCount	= 0;			// number of objects in the sort[] array
	bool		useful		= false;		// was the flush useful?
	QWORD		draftedMask(0,0);			// write queue entries that have been drafted
	QWORD		testedMask(0,0);			// write queue entries that have been tested during this flush
	QWORD		postponedMask(0,0);			// write queue entries that have been postponed (used to be rejectedMask)

	testConsistency();

	//-----------------------------------------------------------
	// Traverse all buckets touching the occludee rectangle.
	//-----------------------------------------------------------

	for(int y=bucketRectangle.y0; y<bucketRectangle.y1; y++)
	for(int x=bucketRectangle.x0; x<bucketRectangle.x1; x++)
	{
		//-----------------------------------------------------------
		// Get list of *unprocessed* entries in the bucket
		//-----------------------------------------------------------

		const Bucket& bucket = buckets[y*m_bucketRectangle.x1 + x];
		if (bucket.empty())
			continue;

		int	list[WRITE_QUEUE_SIZE];
		const int cnt = bucket.getEntries(list,testedMask);		// get list of new objects (i.e. not the ones in testedMask)
		DPVS_ASSERT(cnt>=0 && cnt <= WRITE_QUEUE_SIZE);

		//-----------------------------------------------------------
		// Append potentially useful entries to sort list
		//-----------------------------------------------------------

		for(int i=0;i<cnt;i++)
		{
			DPVS_PROFILE(Statistics::incStatistic(Library::STAT_WRITEQUEUEBUCKETFLUSHWORK,1));

			const int index = list[i];
			DPVS_ASSERT(index>=0 && index<WRITE_QUEUE_SIZE);
			const Entry& e = m_entries[index];
			DPVS_ASSERT(e.object);
			DPVS_ASSERT(e.debugCode == 0xdeadbabe);
			DPVS_ASSERT(e.object->isOcclusionWritable());

			if(e.getSortDepth() > flushDepth)							// farther away than the flush depth...
				continue;

#if defined(DPVS_EXACT_FLUSH)
			if(exactOverlap)
			{
				IntRectangle overlap(intRectangle);
				if(!overlap.intersect(e.rectangle))
					continue;
			}
#endif

#ifdef TIMO_PAINTERS_TEST
			// test exact depth
			if(exactOverlap && g_depthRangeValid)
			{
				Range<float> range(g_depthRange);
				if(range.intersect(e.depth))
				{
					// need OBB sort
				}
			}
#endif

			draftedMask |= e.mask;										// mark object as drafted
			sort[sortCount].entryNum = index;							// and insert it into the sort list
			sort[sortCount].depth	 = e.getSortDepth();
			sortCount++;
			DPVS_ASSERT(sortCount>=0 && sortCount<=WRITE_QUEUE_SIZE);
		}

		//-----------------------------------------------------------
		// Update the 'testedMask'. If we've already tested all
		// entries in the queue, we can break from the loop.
		//-----------------------------------------------------------

		testedMask |= bucket.getMask();									// update the testedMask
		if (testedMask == m_manager.getMask())							// we have already tested/drafted all entries
			break;
	}

	//-----------------------------------------------------------
	// Couldn't flush anything (no exact overlaps, perhaps?)
	//-----------------------------------------------------------

	if (!sortCount)
		return false;

	//-----------------------------------------------------------
	// Sort entries in front->back order (this improves the depth
	// buffering).
	//-----------------------------------------------------------

	if (sortCount > 1)
		sortFlushed(sort,sortCount);

	//--------------------------------------------------------------------
	// Process the sorted entries. Note that if we're in an overflow 
	// situation, we have a limit on the maximum number of objects we 
	// flush from the write queue (as the flushing is caused by a mere
	// internal overflow).
	//--------------------------------------------------------------------

	int maxToRender = sortCount;						// max number of objects to render
	int rendered    = 0;								// number of objects rendered so far

	if (m_overFlow && maxToRender> 8)					// limit to eight if overflowing
		maxToRender = 8;

	for(int i=0;i<sortCount;i++)
	{
		int index = sort[i].entryNum;
		Entry &e = m_entries[index];					// fetch entry

		//--------------------------------------------------------------------
		// If we already have flushed enough (in case of overflow), let's just
		// postpone the remaining occluders 
		//--------------------------------------------------------------------

		if (rendered >= maxToRender)
		{
			DPVS_ASSERT(m_overFlow);		// should not happen otherwise
			DPVS_PROFILE(Statistics::incStatistic(Library::STAT_WRITEQUEUEWRITESPOSTPONED,1));
			postponedMask |= e.mask;
			continue;
		}

/*		// REMOVED DUE TO STATICAL ANALYSIS
		//--------------------------------------------------------------------
		// If the occlusion buffer became full, we can just discard the 
		// remaining occluders. Also, if the write rectangle of the occluder
		// has become hidden, we can discard it...
		//--------------------------------------------------------------------

		if (m_occlusionBuffer->isFull() || m_occlusionBuffer->isRectangleOccluded (e.rectangle,e.depth.getMin()))
		{
			DPVS_PROFILE(Statistics::incStatistic(Library::STAT_WRITEQUEUEWRITESDISCARDED,1));
			clearMaskFromBuckets(e.bucketRectangle,e.mask);
			continue;
		}
*/
		//--------------------------------------------------------------------
		// Query the write silhouette from the object
		//--------------------------------------------------------------------

		VQData::get().setProperties(VQData::FLOW_ACTIVE,false);	// NOTE: USES CACHED DATA!

		m_writeSilhouette.clear();
		// DEBUG DEBUG
		//double t = System::getCycleCount();		// DEBUG DEBUG TIMER

		const bool valid = e.object->getWriteSilhouette	(m_writeSilhouette,e.depth,e.camera);

		//--------------------------------------------------------------------
		// Silhouette is entirely backfacing or front clipping, so we must discard 
		// it.
		//--------------------------------------------------------------------

		if (!valid || m_writeSilhouette.getVertexCount()==0)
		{
			DPVS_PROFILE(Statistics::incStatistic(Library::STAT_WRITEQUEUEWRITESDISCARDED,1));
			clearMaskFromBuckets(e.bucketRectangle,e.mask);
			VQData::get().setProperties(VQData::FLOW_ACTIVE,true);
			continue;
		}
		
		//-----------------------------------------------------------
		// Render the occluder silhouette
		//-----------------------------------------------------------

		rendered++;														// increase # of occluders rendered

#ifndef DPVS_INTERPOLATE_Z
		m_occlusionBuffer->setZConstant		(ZGradient::WRITE, e.depth.getMax());		// TESTING: flat Z
#else
		m_occlusionBuffer->setZGradients	(ZGradient::WRITE, 
											 m_writeSilhouette.getPlaneVertices(), 
											 m_writeSilhouette.getPlaneCount(),
											 e.depth.getMin(),	
											 e.depth.getMax());
#endif
		m_occlusionBuffer->setScissor		(e.camera);
		m_occlusionBuffer->setDirtyRectangle(e.rectangle);

#if defined (DPVS_OCCLUSIONBUFFER_INDEXBUFFER)	
		m_occlusionBuffer->setIndexBufferIndex(e.indexBufferIndex);
#endif

/*
#ifdef DPVS_DEBUG
		// search extents
		FloatRectangle debugRect(FLT_MAX,-FLT_MAX,FLT_MAX,-FLT_MAX);
		debugRect.setSpace(SPACE_RASTER);

		for(int s=0;s<2;s++)
		{
			const Vector3*  v = m_writeSilhouette.getVertices();
			const Vector2i* e = m_writeSilhouette.getEdges(s);
			const int ec	  = m_writeSilhouette.getEdgeCount(s);

			for(int i=0;i<ec;i++)
			{
				debugRect.grow(v[e[i].i].x, v[e[i].i].y);
				debugRect.grow(v[e[i].j].x, v[e[i].j].y);
			}
		}

		FloatRectangle dummy = debugRect;

		if (debugRect.intersect(e.camera->getRasterViewport()))
		{
			SpaceManager::rasterToOcclusion(debugRect,m_subSampling);	// raster -> occlusion space

			// debugRect MUST be inside e.rectangle, which is used in occlusion buffer as dirty rectangle
			DPVS_ASSERT(e.rectangle.x0 - debugRect.x0 <= 0.001f);
			DPVS_ASSERT(e.rectangle.y0 - debugRect.y0 <= 0.001f);
			DPVS_ASSERT(debugRect.x1 - e.rectangle.x1 <= 0.001f);
			DPVS_ASSERT(debugRect.y1 - e.rectangle.y1 <= 0.001f);
		}
#endif*/

//		static int counter = 0;
//		counter++;
		DPVS_PROFILE(Statistics::incStatistic(Library::STAT_WRITEQUEUEWRITESPERFORMED,1));

		const bool contribute = m_occlusionBuffer->write(m_writeSilhouette,m_contributed,m_fullyCovered);
		
		//t = System::getCycleCount() - t;		// DEBUG DEBUG TIMER

		// cost = estimated cost of extracting the silhouette and writing the object
		{
			int		edgeCount   = (m_writeSilhouette.getEdgeCount(0) + m_writeSilhouette.getEdgeCount(1));
			int		extractCost = edgeCount * 550 + 4500;
			int		writeCost   = m_occlusionBuffer->getLastWriteCost();
			float	cost        = (float)(extractCost + writeCost);

			e.object->recalculateWriteCost(cost);		// assign new cost to object...
			
		}
		
		// float ratio2	  = float(t)/(cost);
		// Debug::print("Cycles: %d, %d (%d + %d), %f:1 (e = %d)\n",(int)(t),(int)cost,(int)extractCost,(int)writeCost, ratio2,edgeCount);


		VQData::get().setProperties(VQData::FLOW_ACTIVE,true);

		//-----------------------------------------------------------
		// Check if object actually contributed to the occlusion 
		// buffer (this is indicated by 'contribute'.
		//-----------------------------------------------------------
					
		if(contribute)
		{
			useful = true;

			//-----------------------------------------------------------
			// Mark current object as benefit receiver to all buckets it touches
			//-----------------------------------------------------------

			m_occluderManager.ensureSpaceFor(e.bucketRectangle.area());

			for(int by=e.bucketRectangle.y0; by<e.bucketRectangle.y1; by++)
			for(int bx=e.bucketRectangle.x0; bx<e.bucketRectangle.x1; bx++)
			{
				const int offset = by*m_bucketRectangle.x1 + bx;
				Bucket& bucket = buckets[offset];

				//-----------------------------------------------------------
				// If the current object fully covered the bucket, the previous
				// "occluders" (i.e. parasites) should receive no further benefits
				//-----------------------------------------------------------

#ifdef PARASITE_TEST
				if(m_fullyCovered.test(offset))
				{
					const Array<Occluder>&	occluderList = m_occluderManager.m_occluders;
					int numParasites = bucket.giveBenefits(occluderList);
					DPVS_PROFILE(Statistics::incStatistic(Library::STAT_HOAX0,numParasites));
				}
#endif

				//-----------------------------------------------------------
				// Append object to benefit receiver list of each bucket it
				// contributed to.
				//-----------------------------------------------------------

				if(m_contributed.test(offset))
					bucket.appendObject(m_occluderManager,e.object);

				//-----------------------------------------------------------
				// clear processed object from bucket
				//-----------------------------------------------------------

				bucket.clear(e.mask);
			}
		}
		else // did not contribute
		{
			DPVS_PROFILE(Statistics::incStatistic(Library::STAT_WRITEQUEUEHIDDENOCCLUDERS,1));
			clearMaskFromBuckets(e.bucketRectangle,e.mask);
		}

		//----------------------------------------------------------------
		// Debug output (only if LINE_SILHOUETTES has been set)
		//	- contributed silhouettes are drawn in yellow,
		//	- hidden (bad) silhouettes drawn in red
		//----------------------------------------------------------------

		if(Debug::getLineDrawFlags() & Library::LINE_SILHOUETTES)
		{
			static const Vector4 g_silhouetteColorContributingLeft	(0.3f,1.0f,0.3f,0.7f);
			static const Vector4 g_silhouetteColorContributingRight (1.0f,0.3f,0.3f,0.7f);
			static const Vector4 g_silhouetteColorNonContributing	(0.3f,0.3f,1.0f,0.7f);
			
			drawSilhouette (Library::LINE_SILHOUETTES,m_writeSilhouette, 
				contribute ? g_silhouetteColorContributingLeft : g_silhouetteColorNonContributing,
				contribute ? g_silhouetteColorContributingRight : g_silhouetteColorNonContributing);
		}
	}

	//--------------------------------------------------------------------
	// Free all processed entries
	//--------------------------------------------------------------------

	QWORD deleteMask = draftedMask & (~postponedMask);
	if (!deleteMask.empty())
		m_manager.free(deleteMask);

	//--------------------------------------------------------------------
	// Return boolean indicating whether the flush caused changes into
	// the occlusion buffer
	//--------------------------------------------------------------------

	testConsistency();			// DEBUG BUILD STUFF
	return useful;				// was the flush useful?
}

/*****************************************************************************
 *
 * Function:		DPVS::WriteQueue::freeEntriesUsing()
 *
 * Description:		Frees all entries using given camera
 *
 * Notes:			This is relatively rare operation and can only happen
 *					when using stencils in portals
 *
 *****************************************************************************/

int WriteQueue::freeEntriesUsing(ImpCamera* c)
{
	QWORD releaseMask(0,0);
	int freed = 0;

	for(int i=0;i<WRITE_QUEUE_SIZE;i++)
	if(m_entries[i].camera == c)
	{
		m_entries[i].camera = null;			// clear camera pointer (optional)
		releaseMask |= m_entries[i].mask;	// append to release list
		freed++;
	}

	{
		Guard<Bucket> buckets(m_buckets);
		const int bucketCount = m_bucketRectangle.area();
		for(int i=0;i<bucketCount;i++)
			buckets[i].clear(releaseMask);	// free from buckets

		m_manager.free(releaseMask);			// free from manager
	}

	return freed;
}

/*****************************************************************************
 *
 * Function:		DPVS::WriteQueue::OccluderManager::resize()
 *
 * Description:		
 *
 *****************************************************************************/

void WriteQueue::OccluderManager::resize(int newSize)
{
	DPVS_ASSERT(newSize >= 0);
	m_occluders.resize(newSize);
}

//------------------------------------------------------------------------
