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
 * Description: 	Occlusion buffer point/rectangle/silhouette query code
 *
 * $Archive: /dpvs/implementation/sources/dpvsOcclusionBuffer_Test.cpp $
 * $Author: wili $ 
 * $Revision: #2 $
 * $Modtime: 4/22/02 6:25p $
 * $Date: 2003/03/19 $
 * 
 ******************************************************************************/

#include "dpvsOcclusionBuffer.hpp"
#include "dpvsBlockBuffer.hpp"
#include "dpvsFiller.hpp"
#include "dpvsHZBuffer.hpp"
#include "dpvsSpaceManager.hpp"
#include "dpvsStatistics.hpp"

using namespace DPVS;

/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::coverageBufferOccludes()
 *
 * Description:		determines wheter accurate coverage buffer occludes
 *					given rectangle.
 *
 * Returns:			true is occluded
 *
 * Notes:			Test is performed with QWORD block masks and is therefore
 *					endian independent.
 *					- Called by isRectangleOccluded()
 *
 *****************************************************************************/

DPVS_FORCE_INLINE bool OcclusionBuffer::coverageBufferOccludes(int blockx,int blocky,IntRectangle& r)
{
	DPVS_PROFILE(Statistics::incStatistic(Library::STAT_OCCLUSIONACCURATEBLOCKQUERIES,1));

	//create mask
	r.x0 &= 7;
	r.y0 &= 7;
	r.x1  = (r.x1-1)&7;
	r.y1  = (r.y1-1)&7;

	QWORD	mask  = m_blockMasks0[8*r.y0 + r.x0];	//top left
			mask &= m_blockMasks1[8*r.y1 + r.x1];	//bottom right

	QWORD	block = m_CBuffer[blocky*m_blockMemoryRectangle.x1 + blockx];
			block&= mask;

	//DEBUG DEBUG write mask to occlusion buffer, don't destroy this
//	QWORD &tmp = m_CBuffer[blocky*m_blockMemoryRectangle.x1 + blockx];
//	tmp = mask;

	DPVS_PROFILE(Statistics::incStatistic(Library::STAT_OCCLUSIONACCURATEBLOCKUSEFUL,int(block == mask)));
	return block == mask;
}


bool OcclusionBuffer::isPointOccluded(int x,int y,float z) const
{
	DPVS_PROFILE(Statistics::incStatistic(Library::STAT_OCCLUSIONPOINTQUERIES,1));
//	DPVS_ASSERT(x>=m_intScissor.x0 && x<m_intScissor.x1);
//	DPVS_ASSERT(y>=m_intScissor.y0 && y<m_intScissor.y1);
	DPVS_ASSERT(x>=0 && x<m_clipRectangle.x1);
	DPVS_ASSERT(y>=0 && y<m_clipRectangle.y1);
	DPVS_ASSERT(z>=0.f);

	if(isEmpty())
		return false;

	const int bx = x>>3;
	const int by = y>>3;

	//---------------------------------------------------------------
	// test zvalue
	//---------------------------------------------------------------

	if(Math::bitPattern(z) <= m_ZBuffer[by*m_loresRectangle.x1+bx])
		return false;

	//---------------------------------------------------------------
	// if coverage buffer is full, skip the test (depth has been tested)
	//---------------------------------------------------------------

	if(isFull())
		return true;

	//---------------------------------------------------------------
	// NOTE: contribution culling is currently enabled only for objects
	//---------------------------------------------------------------

	BlockBuffer::Result result = m_blockBuffer->isPointVisible(bx,by);

	if(result==BlockBuffer::FULL)
		return true;

	if(result==BlockBuffer::EMPTY)
		return false;

	//---------------------------------------------------------------
	// full precision test
	//---------------------------------------------------------------

	const unsigned char	line8 = m_CBuffer[by*m_blockMemoryRectangle.x1 + bx].getByte(x&7);
	const bool res = ((line8>>(y&7))&1) == 1;

	DPVS_PROFILE(Statistics::incStatistic(Library::STAT_OCCLUSIONACCURATEPOINTQUERIES,1));
	DPVS_PROFILE(Statistics::incStatistic(Library::STAT_OCCLUSIONACCURATEPOINTUSEFUL,res));
	return res;
}

/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::isRectangleOccluded()
 *
 * Description:		
 *
 *****************************************************************************/

bool OcclusionBuffer::isRectangleOccluded(const IntRectangle &r, float zval)
{
	DPVS_ASSERT(r.getSpace() == SPACE_OCCLUSION);
	DPVS_ASSERT(zval>=0.f);

	DPVS_PROFILE(Statistics::incStatistic(Library::STAT_OCCLUSIONRECTANGLEQUERIES,1));

	//---------------------------------------------------------------
	// If occlusion buffer is empty, the rectangle cannot be occluded
	//---------------------------------------------------------------

	if(isEmpty())			
		return false;

	//---------------------------------------------------------------
	// Is rectangle behind closing depth (and thus occluded)
	//---------------------------------------------------------------

	if (isFull() && zval > getClosingDepth())	
		return true;

	//---------------------------------------------------------------
	// Get block rectangle
	//---------------------------------------------------------------

	IntRectangle t(r.x0>>3,(r.x1+7)>>3, r.y0>>3,(r.y1+7)>>3);

	//---------------------------------------------------------------
	// Test if there're empty blocks -> cannot be occluded
	//---------------------------------------------------------------

	BlockBuffer::Result result = BlockBuffer::EMPTY;
	
	if(!isFull())
	{
		if(m_contributionCullingEnabled)
			m_blockBuffer->setFullBlocksBuffer(m_blockBufferC);		// [CONTRIBUTION] - activate

		result = m_blockBuffer->isRectangleVisible(t.x0,t.y0,t.x1,t.y1);
		m_blockBuffer->setFullBlocksBuffer(m_blockBufferF);			// [CONTRIBUTION] - restore

		if(result == BlockBuffer::EMPTY)
			return false;
	}

	//---------------------------------------------------------------
	// Test DEPTH
	//---------------------------------------------------------------

	if(m_HZBuffer->isVisible(t.x0,t.y0,t.x1,t.y1,Math::bitPattern(zval)))
		return false;

	//---------------------------------------------------------------
	// If coverage buffer is full, skip the test (depth has been tested)
	//---------------------------------------------------------------

	if(isFull())
		return true;

	if(result == BlockBuffer::PARTIAL)
	{
		//---------------------------------------------------------------
		// Get blocks where more accurate visibility determination should be performed
		//---------------------------------------------------------------

		if(m_contributionCullingEnabled)
			m_blockBuffer->setFullBlocksBuffer(m_blockBufferC);		// [CONTRIBUTION] - activate

		const int problemCount = m_blockBuffer->getProblemBlocks(&m_blockCoordinates[0],t.x0,t.y0,t.x1,t.y1);
		m_blockBuffer->setFullBlocksBuffer(m_blockBufferF);			// [CONTRIBUTION] - restore

		DPVS_ASSERT(problemCount);

		//---------------------------------------------------------------
		// Resolve at higher precision if potentially helpful
		// Full resolution integer rectangle
		//---------------------------------------------------------------

		for(int p=0;p<problemCount;p++)
		{
			//---------------------------------------------------------------
			// Block's rectangle in full resolution integer coordinates
			//---------------------------------------------------------------

			int x = m_blockCoordinates[p].getX();
			int y = m_blockCoordinates[p].getY();
			IntRectangle brect(8*x,8*(x+1), 8*y,8*(y+1));
			brect.setSpace(SPACE_OCCLUSION);

			DPVS_DEBUG_CODE(bool value =) brect.intersect(r);
			DPVS_ASSERT(value);

			if(!coverageBufferOccludes(x,y,brect))
				return false;
		}
	}

	return true;
}


/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::setVisiblePoint()
 *
 * Description:		Sets visible point (stored in RASTER space)
 *
 *****************************************************************************/

void OcclusionBuffer::setVisiblePoint(const QWORD& block,float z,int bucketx,int buckety,int blockx, int blocky)
{
	DPVS_ASSERT(!block.empty());
	DPVS_ASSERT(z>=0.f && z <= 1.f);

	//-----------------------------------------------------
	// byte == x coordinate
	// bit inside the byte == y coordinate
	//-----------------------------------------------------

	const int bit = getHighestSetBit(block);		// get highest set bit (0-63)
	const int x = bit>>3;							// x = 0-7
	const int y = bit&7;							// y = 0-7

	DPVS_ASSERT((y+(x*8))==bit);

	m_visiblePoint.x	= ((bucketx*BUCKET_W + blockx*8+x) + 0.5f) * m_ooSubSampling.x;
	m_visiblePoint.y	= ((buckety*BUCKET_H + blocky*8+y) + 0.5f) * m_ooSubSampling.y;
	m_visiblePoint.z	= z;
	m_visiblePointValid = true;
}

/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::cacheToCoverageBufferTEST()
 *
 * Description:		Returns true if occluded
 *
 * Parameters:		bx		= bucket x coordinate
 *					by		= bucket y coordinate
 *					bmin	= minimum dirty block inside the bucket (x)
 *					bmax	= maximum dirty block inside the bucket (x)
 *					pendingClear = is there a pending clear for the bucket?
 *
 *****************************************************************************/

#ifdef DPVS_DEBUG	//DEBUG DEBUG
bool OcclusionBuffer::cacheToCoverageBufferTEST(int bx,int by,int bmin,int bmax,bool pendingClear)
#else
DPVS_FORCE_INLINE bool OcclusionBuffer::cacheToCoverageBufferTEST(int bx,int by,int bmin,int bmax,bool pendingClear)
#endif
{
	DPVS_ASSERT(bx>=0 && by>=0);

	const int zx = (unsigned int)(bx)*(BUCKET_W/8);
	const int zy = (unsigned int)(by)*(BUCKET_H/8);

	//---------------------------------------------------------------
	// If bucket has a pending clear, it cannot be covered
	//---------------------------------------------------------------

	if(pendingClear)
	{
		for(int j=7;j>=0;j--)
		{
			if(m_dirtyScanlines.getByte(j))
			{
				for(int i=bmax-1; i>=bmin; i--)
				{
					QWORD cdata(m_cache->getReordered(i,j));

					if(cdata.empty())
						continue;

					const float z = m_ZGradient.interpolate(zx+i,zy+j);
					setVisiblePoint(cdata,z,bx,by,i,j);
					return false;	// not occluded
				}
			}
		}

		return true;		//occluded
	}

	//---------------------------------------------------------------
	// Perform coverage and depth tests for all dirty scanlines
	//---------------------------------------------------------------

	const QWORD*	cbuf	= &m_CBuffer[by*m_coverageBufferBucketRowQWPitch + bx*m_coverageBufferBucketQWPitch];
	UINT32			boffset	= ((unsigned int)(by)*BUCKET_H/8*m_loresRectangle.x1)			+ ((unsigned int)(bx)*BUCKET_W/8);		//bit #
	unsigned char* fullBlockBuffer = (m_contributionCullingEnabled) ? m_blockBufferC : m_blockBufferF; //[CONTRIBUTION]

	DPVS_PROFILE(Statistics::incStatistic(Library::STAT_OCCLUSIONBUFFERBUCKETSPROCESSED,1));

#if defined(DPVS_DEBUG)
	IntRectangle	dirtyBlockRectangle( m_dirtyRectangle );
	dirtyBlockRectangle.downSample( 8,8 );
#endif

	boffset	+= 7 * m_loresRectangle.x1;								// next y
	cbuf	+= 7 * (8*(unsigned int)(m_memoryRectangle.x1)/8) / sizeof(QWORD);		// next block in y

	UINT32 zTestValue = Math::bitPattern(m_ZGradient.getMin());		// base test value

	for(int j=7;j>=0;j--)
	{
		if(m_dirtyScanlines.getByte(j))
		{
			int	  offset		= boffset   + (bmax-1);
			const UINT32* zbuf	= m_ZBuffer + (zy+j)*m_loresRectangle.x1 + (zx+bmax-1);
	
			for(int i=bmax-1; i>=bmin; i--,zbuf--,offset--)
			{
				DPVS_ASSERT((zx+i) >= dirtyBlockRectangle.x0 && (zx+i) < dirtyBlockRectangle.x1);
				DPVS_ASSERT((zy+j) >= dirtyBlockRectangle.y0 && (zy+j) < dirtyBlockRectangle.y1);

				//---------------------------------------------------
				// See if we need to test coverage..
				//---------------------------------------------------

				if(!(fullBlockBuffer[offset>>3]&(1<<(offset&7))))	// pretty rare to have a non-full block
				{
					QWORD cdata(m_cache->getReordered(i,j));
					if (cdata.empty())								// no pixels in cache..
						continue;
					QWORD coverage = cbuf[i];
					if(coverage.changesIfMergedWith(cdata))			// at least one pixel is visible..
					{
						QWORD visible = cdata & (~coverage);
						float z = m_ZGradient.interpolate(zx+i,zy+j);
						setVisiblePoint(visible,z,bx,by,i,j);
						return false;								// not occluded
					}
				}

				//---------------------------------------------------
				// Test depth. We also have a postponed zero
				// coverage test. The reason why it's the very
				// last is due to probabilities....
				//---------------------------------------------------

				UINT32 zz = *zbuf;									// fetch depth value from z-buffer

				if(zTestValue > zz)									// even closest point is behind, so skip to next block
					continue;
				
				DPVS_PROFILE(Statistics::incStatistic(Library::STAT_OCCLUSIONBUFFEREXACTZTESTS,1));
				
				const float z = m_ZGradient.interpolate(zx+i,zy+j);		// calculate conservative z value for the 8x8 pixel area

				if(Math::bitPattern(z) <= zz)						// pixel fails the Z test and is thus visible
				{
					QWORD cdata(m_cache->getReordered(i,j));		// check if there was zero coverage....
					if (cdata.empty())								// 
						continue;									// 

					setVisiblePoint(cdata,z,bx,by,i,j);				// set new visible point
					return false;									// not occluded
				}
			}
		}

		boffset	-= m_loresRectangle.x1;								// next y
		cbuf	-= (8*(unsigned int)(m_memoryRectangle.x1)/8) / sizeof(QWORD);		// next block in y
	}

	return true;
}


/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::test()
 *
 * Description:		Performs a visibility test using a silhouette
 *
 * Parameters:		Returns true if occluded
 *
 *****************************************************************************/

bool OcclusionBuffer::test(EdgeSilhouette& es)
{
	DPVS_PROFILE(Statistics::incStatistic(Library::STAT_OCCLUSIONSILHOUETTEQUERIES,1));
	DPVS_PROFILE(Statistics::incStatistic(Library::STAT_OCCLUSIONBUFFEREDGESTESTED,es.getEdgeCount(0)+es.getEdgeCount(1)));
	
	DPVS_ASSERT(m_dirtyRectangleSet);			// must have dirty rectangle

#if defined(DPVS_DEBUG)
	m_cache->assertEmpty();		// DEBUG DEBUG HC
#endif

	//-----------------------------------------------------
	// Clear all object-related variables
	//-----------------------------------------------------

	m_mode					= TEST;
	m_visiblePointValid		= false;
	m_previousBlocksFilled	= 0;		// statistics
	m_previousBucketsFilled	= 0;		// statistics
	m_ZChangeCount			= 0;		// potential z changes are collected to buffer
	m_dirtyScanlines.setZero();//Both(0);

	//-----------------------------------------------------
	// Clip and scan convert edges
	//-----------------------------------------------------

	if(!setEdges(es,true))	return true;	// Possible, single line due to coverage analysis!

	//-----------------------------------------------------
	// Construct bucket resolution dirty rectangle
	//-----------------------------------------------------

	int			bmin	= 0;								//for cache -> mainmemory propagation
	int			bmax	= BUCKET_W/8;						//intialized due to compiler warning
	bool		result	= true;
//	Bucket*		pb		= null;								//DEBUG DEBUG

//	bool		edgePixels	= false;
//	bool		collapsed	= true;

	DPVS_ASSERT(m_dirtyBucketRectangle.y0 >= 0 && m_dirtyBucketRectangle.x0 >= 0);
//	static int debug = 0;
//	debug++;

	//-----------------------------------------------------
	// Process all dirty buckets
	//-----------------------------------------------------

	bool anySamples = false;

	for(int by=m_dirtyBucketRectangle.y0; by<m_dirtyBucketRectangle.y1 ;by++)
	{
		const int b = by*m_bucketRectangle.x1;

		QWORD fvalue(0,0);

		for(int bx=m_dirtyBucketRectangle.x0; bx<m_dirtyBucketRectangle.x1 ;bx++)
		{
			DPVS_ASSERT((b+bx)>=0 && (b+bx)<m_bucketRectangle.area());

			m_cache->assertEmpty();		// DEBUG DEBUG HC
			m_cache->init();

			OcclusionBuffer::Bucket	&bucket = m_bucket[b+bx];
//			pb = &bucket;

			int limit = m_dirtyRectangle.x1 - bx*BUCKET_W;
			if(limit>BUCKET_W)	
				limit = BUCKET_W;

			int	xmin,xmax;
			if(rasterizeEdges(bucket,xmin,xmax))
			{
				m_cache->setRange(xmin,xmax);
				m_cache->fillerXOR(m_dirtyScanlines, fvalue,bmin,bmax,limit);

				//DEBUG DEBUG
//				edgePixels = true;				// yes it did
//				if(!m_dirtyScanlines.empty())
//					collapsed = false;			// something truly got rasterized
			}
			else
			{
				if(fvalue.empty())			// cache is empty
					continue;
	
				if(bucket.isFull() && !m_useStencil && bucket.getMaxDepth() < Math::bitPattern(m_ZGradient.getMin()))
					continue;				// hidden bucket

				//-------------------------------------------------------
				// Get dirty rectangle inside the cache
				//-------------------------------------------------------

				bmin = 0;
				bmax = (limit<BUCKET_W) ? ((limit+7)>>3) : (BUCKET_W/8);
				m_dirtyScanlines =  fvalue;

				//-------------------------------------------------------
				// Detect full cache and skip cache filler
				//-------------------------------------------------------

				if(limit < BUCKET_W)
				{
					m_cache->fillPlane0(0,limit,fvalue);
				}
				else
				{
					if(!m_useStencil && fvalue.full())				// this cannot be used if m_useStencil is active
						m_cache->setFull();
					else
						m_cache->fillPlane0(0,BUCKET_W,fvalue);
				}
			}

			//--------------------------------------------------------------------
			// Apply stencil mask (if available)
			//--------------------------------------------------------------------

			if(m_useStencil)
			{
				const QWORD*	stencil	= m_stencilBuffer + (b+bx) * BUCKET_W;		// stencil buffer position
				QWORD*			cache	= (*m_cache)[0];
				const int		mn		= 8*bmin;
				const int		mx		= 8*bmax;
				QWORD			stmask;

				for(int i=mn;i<mx;i++)
				{
					cache[3*i] &= stencil[i];									// remove masked contents of cache
					stmask	   |= stencil[i];
				}

				m_dirtyScanlines &= stmask;
			}

			//--------------------------------------------------------------------
			// Test coverage and depth
			//--------------------------------------------------------------------

			if(m_dirtyScanlines.empty())
			{
				m_cache->cleanup(bmin,bmax);
				continue;
			}

			anySamples = true;

			result = cacheToCoverageBufferTEST(bx,by,bmin,bmax,bucket.hasPendingClear());

			m_cache->cleanup(bmin,bmax);

			if(!result)
				goto out;
		}
	}

out:;

#if defined(DPVS_DEBUG)
	m_cache->assertEmpty();
#endif

	m_dirtyRectangleSet	= false;

	//--------------------------------------------------------------------
	// If the scan conversion didn't generate any samples, we must 
	// conservatively assume that the object is visible.
	//--------------------------------------------------------------------

	if (!anySamples)
		result = false;
//	if(edgePixels && collapsed)
//		result = false;

	return result;
}

//------------------------------------------------------------------------
