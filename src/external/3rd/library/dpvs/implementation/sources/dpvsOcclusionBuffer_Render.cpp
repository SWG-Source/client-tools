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
 * Description: 	Occlusion buffer rendering code (+ some cache code)
 *
 * $Archive: /dpvs/implementation/sources/dpvsOcclusionBuffer_Render.cpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 4/22/02 6:47p $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#include "dpvsFiller.hpp"
#include "dpvsHZBuffer.hpp"
#include "dpvsStatistics.hpp"
#include "dpvsTempArray.hpp"
#include "dpvsImpObject.hpp"	// just to get the cost enumerations

//#include "dpvsSystem.hpp"

//#define DEBUG_FILL							// use only when debugging/viewing cbuffer
//#define FORCE_CONTRIBUTION_CULL				// forces contribution culling
//#define FORCE_CONTRIBUTION_CULL_BITS 50			// bit coun to check against

using namespace DPVS;


/*****************************************************************************
 *
 * Function:		DPVS::Cache::Cache()
 *
 * Description:		Constructor
 *
 * Notes:			
 *
 *****************************************************************************/

OcclusionBuffer::Cache::Cache (void) : m_full(false)
{
	//nada
}

/*****************************************************************************
 *
 * Function:		DPVS::Cache::~Cache()
 *
 * Description:		dtor
 *
 * Notes:			May be allocated from scratchpad
 *
 *****************************************************************************/

OcclusionBuffer::Cache::~Cache (void)			
{ 
	//nada
}

/*****************************************************************************
 *
 * Function:		DPVS::Cache::assertEmpty()
 *
 * Description:		Asserts that the cache is totally empty
 *
 * Notes:			DEBUG function
 *
 *****************************************************************************/

void OcclusionBuffer::Cache::assertEmpty(void) const
{
	for(int i=0;i<WIDTH*PLANECOUNT;i++)
		DPVS_ASSERT(getData()[i].empty());
}

/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::frameStart()
 *
 * Description:		Called once, when frame starts
 *
 * Notes:			
 *
 *****************************************************************************/

void OcclusionBuffer::frameStart	(void)
{
	m_cache->allocate();
}

/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::frameEnd()
 *
 * Description:		Called once, when frame ends
 *
 * Notes:			
 *
 *****************************************************************************/

void OcclusionBuffer::frameEnd		(void)
{
	m_cache->free();
}


/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::clearBucketArea()
 *
 * Description:		
 *
 * Notes:			Fixed width
 *
 *****************************************************************************/

static inline void clearBlockRow (QWORD *buf)
{
#ifdef DPVS_TILE_16
	buf[0].setZero();//Both(0);
	buf[1].setZero();//Both(0);
#endif

#ifdef DPVS_TILE_32
	buf[0].setZero();//Both(0);
	buf[1].setZero();//Both(0);
	buf[2].setZero();//Both(0);
	buf[3].setZero();//Both(0);
#endif

#ifdef DPVS_TILE_64
	buf[0].setZero();//Both(0);
	buf[1].setZero();//Both(0);
	buf[2].setZero();//Both(0);
	buf[3].setZero();//Both(0);
	buf[4].setZero();//Both(0);
	buf[5].setZero();//Both(0);
	buf[6].setZero();//Both(0);
	buf[7].setZero();//Both(0);
#endif

#ifdef DPVS_TILE_128
	buf[0].setZero();//Both(0);
	buf[1].setZero();//Both(0);
	buf[2].setZero();//Both(0);
	buf[3].setZero();//Both(0);
	buf[4].setZero();//Both(0);
	buf[5].setZero();//Both(0);
	buf[6].setZero();//Both(0);
	buf[7].setZero();//Both(0);
	buf[8].setZero();//Both(0);
	buf[9].setZero();//Both(0);
	buf[10].setZero();//Both(0);
	buf[11].setZero();//Both(0);
	buf[12].setZero();//Both(0);
	buf[13].setZero();//Both(0);
	buf[14].setZero();//Both(0);
	buf[15].setZero();//Both(0);
#endif
}

void OcclusionBuffer::clearBucketArea(const Bucket &bucket, QWORD *cbuf)
{
	DPVS_PROFILE(Statistics::incStatistic(Library::STAT_OCCLUSIONBUFFERBUCKETSCLEARED,1));
	
	QWORD *buf = &cbuf[bucket.getCoverageBufferOffset()];
	UINT32 pitch = (UINT32)m_coverageBufferBlockRowQWPitch;
	for(int i=0;i<(BUCKET_H/BLOCK_H);i++, buf+=pitch)
		clearBlockRow(buf);
}

/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::render(BitVector& contributedToBlock)
 *
 * Description:		
 *
 * Parameters:		visualize = boolean value indicating whether debug visualization should be performed
 *
 * Notes:			
 *
 *****************************************************************************/

bool OcclusionBuffer::write(EdgeSilhouette& es, BitVector& contributedToBucket, BitVector& fullyCoveredTheBucket)
{
	m_lastWriteCost = ImpObject::WRITE_COST_PER_OBJECT;			// init to object cost..

#if defined(DPVS_DEBUG)
	m_cache->assertEmpty();
#endif

	DPVS_ASSERT(m_dirtyRectangleSet);			// must have dirty rectangle

	//-----------------------------------------------------
	// clear all object related variables
	//-----------------------------------------------------

	m_mode					= WRITE;
	m_visiblePointValid		= false;
	m_previousBlocksFilled	= 0;		// statistics
	m_previousBucketsFilled	= 0;
	m_ZChangeCount			= 0;		// potential z changes are collected to buffer
	m_dirtyScanlines.setZero();//Both(0);

	//-----------------------------------------------------
	// clip and scan convert edges
	//-----------------------------------------------------

	if(!setEdges(es,true))	// validate dirty rectangle
		return false;

	//-----------------------------------------------------
	// init variables
	//-----------------------------------------------------

	int			bmin				= 0;				// for cache -> mainmemory propagation
	int			bmax				= BUCKET_W/8;		// intialized due to compiler warning
	bool		result				= false;

	DPVS_ASSERT(m_dirtyBucketRectangle.y0 >= 0 && m_dirtyBucketRectangle.x0 >= 0);

//	static int debugCounter = 0;
//	debugCounter++;

#if defined (DPVS_OCCLUSIONBUFFER_INDEXBUFFER)	
	++m_indexBufferIndex;
#endif

	//-----------------------------------------------------
	// Make sure that the fill structure is QWORD-aligned
	// (not all compilers keep the stack aligned!)
	//-----------------------------------------------------

	unsigned char	fillDummy[3*sizeof(QWORD)+8];
	QWORD*			fill = (QWORD*)((reinterpret_cast<UPTR>(fillDummy)+(size_t)sizeof(QWORD)-1)&~(size_t)(sizeof(QWORD)-1));

	//-----------------------------------------------------
	// process all dirty buckets
	//-----------------------------------------------------

	for(int by=m_dirtyBucketRectangle.y0; by<m_dirtyBucketRectangle.y1 ;by++)
	{
		const int b = by*m_bucketRectangle.x1;

		ParallelMath::fillZero(fill);
		QWORD fvalue(0,0);

		for(int bx=m_dirtyBucketRectangle.x0; bx<m_dirtyBucketRectangle.x1 ;bx++)
		{
			const int bucketNumber = b+bx;

			DPVS_ASSERT(bucketNumber>=0 && bucketNumber<m_bucketRectangle.area());

			//-------------------------------------------------------
			// skip bucket if empty and there's not an ongoing fill
			//-------------------------------------------------------

			m_cache->init();

			int limit		= m_dirtyRectangle.x1 - bx*BUCKET_W;
			if(limit>BUCKET_W)	limit = BUCKET_W;

			OcclusionBuffer::Bucket	&bucket = m_bucket[bucketNumber];
			contributedToBucket.  clear(bucketNumber);			// clear contribution flag
			fullyCoveredTheBucket.clear(bucketNumber);			// clear full coverage flag (PARASITE)

			int	xmin=0;			// lower bound for the filler
			int	xmax=0;			// upper bound for the filler

			if(rasterizeEdges(bucket, xmin,xmax))
			{
				m_cache->setRange(xmin,xmax);
				m_cache->fillerNBIT(m_dirtyScanlines, fill,fvalue,bmin,bmax,limit);
			}
			else
			{
				//-------------------------------------------------------
				// full bucket (i.e. cannot have active border either -> totally passive)
				//-------------------------------------------------------

				if(bucket.isFull())
				{
					//-------------------------------------------------------
					//if full bucket doesn't have reflection target, everything to the right is full aswell.
					//otherwise reflectiontarget MIGHT have been modified
					//-------------------------------------------------------

					const int rt = bucket.getReflectionTarget();

					if(rt == -1)
						goto nextBucketRow;

					if(m_bucket[rt].getBucketX() >= m_dirtyBucketRectangle.x1)
						goto nextBucketRow;

					continue;
				}

				//-------------------------------------------------------
				// bucket is empty and blank -> skip
				//-------------------------------------------------------

				if(fvalue.empty())
					continue;

				//-------------------------------------------------------
				//detect full bucket and skip cache filler
				//-------------------------------------------------------

				bmin = 0;
				bmax = BUCKET_W/8;
				m_dirtyScanlines = fvalue;

				if(limit < BUCKET_W)
				{
					bmax = (limit+7)>>3;
					m_cache->fillPlane0(0,limit,fvalue);
				}
				else
				{
					if(!m_useStencil && fvalue.full())	// this cannot be used if m_useStencil is active
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
				QWORD *stencil	= m_stencilBuffer + (b+bx) * BUCKET_W;		// stencil buffer position
				QWORD *cache	= (*m_cache)[0];
				QWORD  stmask;

				const int mn = 8*bmin;
				const int mx = 8*bmax;

				for(int i=mn;i<mx;i++)
				{
					cache[3*i] &= stencil[i];								// remove masked contents of cache
					stmask	   |= stencil[i];
				}

				m_dirtyScanlines &= stmask;

				if(m_dirtyScanlines.empty())								// skip bucket if empty after stencil op
				{
					m_cache->cleanup(bmin,bmax);
					continue;
				}
			}

			//-------------------------------------------------------
			//update coverage & zebuffers (postponed)
			//-------------------------------------------------------

			const int tmpBlockCount = m_previousBlocksFilled;
			const int tmpZCount		= m_ZChangeCount;

			//--------------------------------------------------------------------
			// perform byte reordering and cache to coveragebuffer fusion
			//--------------------------------------------------------------------

			if(m_cache->full())	// MARKED full (data is empty)
			{
				cacheToCoverageBufferFAST(bx,by,bmax);
			}
			else
			{
				if(bucket.hasPendingClear())
				{
					clearBucketArea(bucket,m_CBuffer);
					cacheToCoverageBufferREPLACE(bx,by,bmin,bmax);
				}
				else
				{
					cacheToCoverageBufferFUSION(bx,by,bmin,bmax);
				}

				m_cache->cleanup(bmin,bmax);
			}

			bucket.setPendingClear(false);

			//-------------------------------------------------------
			//test if bucket became full and validate reflection target information
			//-------------------------------------------------------

			if(m_ZChangeCount>tmpZCount)
			{
				contributedToBucket.set(b+bx);						// did contribute to bucket
				bucket.addFullBlockCount(m_previousBlocksFilled - tmpBlockCount);

				if(bucket.isFull())
				{
					m_justFilledBuckets[m_previousBucketsFilled++].set(bx,by);

					//---------------------------------------------------
					// test if the current object ALONE fully covered the bucket (PARASITE)
					//---------------------------------------------------

					if(m_cache->fullCoverage())
						fullyCoveredTheBucket.set(b+bx);				// fully covered the bucket

					//---------------------------------------------------
					// set reflection target
					//---------------------------------------------------

					int i;
					bucket.clearReflectionTarget();

					for(i=bx+1;i<m_bucketRectangle.x1;i++)
					{
						if(!m_bucket[b+i].isFull())
						{
							bucket.setReflectionTarget(b+i);
							break;
						}
					}

					//---------------------------------------------------
					// fix affected reflection targets
					//---------------------------------------------------

					for(i=0;i<bx;i++)
					{
						if(m_bucket[b+i].getReflectionTarget() == bucketNumber)
							m_bucket[b+i].setReflectionTarget(bucket.getReflectionTarget());
					}
				}
			}
		}
nextBucketRow:;
	}

	//--------------------------------------------------------------------
	// Runtime statistics 
	//--------------------------------------------------------------------

	m_totalBlocksFilled	 += m_previousBlocksFilled;
	m_totalBucketsFilled += m_previousBucketsFilled;

	//--------------------------------------------------------------------
	// If coverage buffer was modified, update the depth buffer
	// as well.
	//--------------------------------------------------------------------

	if(m_ZChangeCount>0)
	{
		m_lastWriteCost += m_ZChangeCount * ImpObject::WRITE_COST_PER_ZCHANGE;	// estimated cost for a z change...
		updateDepthBuffer();						// this will update the HZBuffer as well
		result = true;
	}
	else
	{
		DPVS_ASSERT(result==false);	// DEBUG DEBUG
	}

	//--------------------------------------------------------------------
	// set maximimum depth values for buckets that got full
	//--------------------------------------------------------------------

	for(int i=0;i<m_previousBucketsFilled;i++)
	{
		const int bx = (int)m_justFilledBuckets[i].getX();
		const int by = (int)m_justFilledBuckets[i].getY();

		UINT32  maxDepth = 0;
		UINT32* zbuf = m_ZBuffer + ((unsigned int)(by) * BUCKET_H/BLOCK_H) * m_loresRectangle.x1 + ((unsigned int)(bx) * BUCKET_W/BLOCK_W);

		for(int y=0;y<BUCKET_H/BLOCK_H;y++)
		for(int x=0;x<BUCKET_W/BLOCK_W;x++)
		{
			const int offset = y*m_loresRectangle.x1 + x;
			if(zbuf[offset] > maxDepth)
				maxDepth = zbuf[offset];
		}

		m_bucket[by*m_bucketRectangle.x1 + bx].setMaxDepth(maxDepth);
	}

#if defined(DPVS_DEBUG)
	m_cache->assertEmpty();
#endif

	m_dirtyRectangleSet	= false;
	return result;
}


/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::cacheToCoverageBufferFAST()
 *
 * Description:		Cache is full -> no need to test it
 *
 * Parameters:		bx		= bucket x coordinate
 *					by		= bucket y coordinate
 *					bmax	= maximum dirty block inside the bucket (x)
 *
 *****************************************************************************/

void OcclusionBuffer::cacheToCoverageBufferFAST(int bx,int by,int bmax)
{
	DPVS_ASSERT(bx>=0 && by >= 0);

	int	boffset	= ((unsigned int)(by)*BUCKET_H/8*m_loresRectangle.x1) + ((unsigned int)(bx)*BUCKET_W/8);		//bit #
	int	zx		= (unsigned int)(bx)*(BUCKET_W/8);
	int	zy		= (unsigned int)(by)*(BUCKET_H/8);

	DPVS_PROFILE(Statistics::incStatistic(Library::STAT_OCCLUSIONBUFFERBUCKETSPROCESSED,1));

#if defined(DPVS_DEBUG)
	IntRectangle	dirtyBlockRectangle( m_dirtyRectangle );
	dirtyBlockRectangle.downSample( 8,8 );
#endif

	for(int j=0;j<BUCKET_H/BLOCK_H;j++,zy++)
	{
		int offset = boffset;

		if(m_dirtyScanlines.getByte(j))
		for(int i=0;i<bmax;i++,offset++)
		{
			unsigned char&	blockFull	= m_blockBufferF[offset>>3];		//yyyyxxxx yyyyxxxx
			const unsigned char	bMask	= (unsigned char)(1<<(offset&7));

			if(blockFull & bMask)											// skip if block full
				continue;

			blockFull				  |= bMask;								// mark block as full
			m_blockBufferC[offset>>3] |= bMask;								// [CONTRIBUTION]

			DPVS_ASSERT(int(zx+i) >= dirtyBlockRectangle.x0 && int(zx+i) < dirtyBlockRectangle.x1);
			DPVS_ASSERT(zy >= dirtyBlockRectangle.y0 && zy < dirtyBlockRectangle.y1);
			m_blockCoordinates[m_ZChangeCount++].set(zx+i,zy,true);

			m_previousBlocksFilled++;							// needed?
		}
		boffset	+= m_loresRectangle.x1;							// next y
	}
}

// DEBUG DEBUG THIS NOT OK FOR CW

/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::cacheToCoverageBufferREPLACE()
 *
 * Description:		
 *
 * Parameters:		bx		= bucket x coordinate
 *					by		= bucket y coordinate
 *					bmin	= minimum dirty block inside the bucket (x)
 *					bmax	= maximum dirty block inside the bucket (x)
 *
 *****************************************************************************/

void OcclusionBuffer::cacheToCoverageBufferREPLACE(int bx,int by,int bmin,int bmax)
{
	DPVS_ASSERT(bx >= 0 && by >= 0);

	QWORD*	cbuf		= &m_CBuffer[by*m_coverageBufferBucketRowQWPitch + bx*m_coverageBufferBucketQWPitch];
	int		boffset		= ((unsigned int)(by)*BUCKET_H/8*m_loresRectangle.x1) + ((unsigned int)(bx)*BUCKET_W/8);		//bit #
	const int zx		= (unsigned int)(bx)*(BUCKET_W/8);
	const int zy		= (unsigned int)(by)*(BUCKET_H/8);

	DPVS_PROFILE(Statistics::incStatistic(Library::STAT_OCCLUSIONBUFFERBUCKETSPROCESSED,1));
	DPVS_ASSERT(Cache::getPlaneCount()*sizeof(QWORD)==24);

#if defined(DPVS_DEBUG)
	IntRectangle	dirtyBlockRectangle( m_dirtyRectangle );
	dirtyBlockRectangle.downSample( 8,8 );
#endif

	//perform COPY operation to coverage buffer.
	for(int j=0;j<BUCKET_H/BLOCK_H;j++)		//blocks in y
	{
		int offset = boffset + bmin;

		if(m_dirtyScanlines.getByte(j))
		for(int i=bmin; i<bmax; i++,offset++)
		{
			QWORD cdata(m_cache->getReordered(i,j));

			if(!cdata.empty())									//detect changes per 8x8 block
			{
				DPVS_ASSERT(int(zx+i) >= dirtyBlockRectangle.x0 && int(zx+i) < dirtyBlockRectangle.x1);
				DPVS_ASSERT(int(zy+j) >= dirtyBlockRectangle.y0 && int(zy+j) < dirtyBlockRectangle.y1);

				if(cdata.full())
				{
					m_blockBufferF[offset>>3] |= (unsigned char)(1<<(offset&7));	// mark block as full
					m_blockBufferC[offset>>3] |= (unsigned char)(1<<(offset&7));	// mark block as full [CONTRIBUTION]
					m_previousBlocksFilled++;
#ifdef DEBUG_FILL
					cbuf[i].setFull();//Both(FULL_MASK);
#endif
					m_blockCoordinates[m_ZChangeCount++].set(zx+i,zy+j,true);			// collect to a zlist
				}
				else
				{
					if(cdata.getBitCount() >= m_contributionThreshold)
						m_blockBufferC[offset>>3] |= (unsigned char)(1<<(offset&7));// mark block as full [CONTRIBUTION]

					m_blockBufferP[offset>>3] |= (unsigned char)(1<<(offset&7));	//mark block as partial
					cbuf[i] = cdata;												//write partial data to coverage buffer
					m_blockCoordinates[m_ZChangeCount++].set(zx+i,zy+j,false);			//collect to a zlist
				}
			}
		}

		boffset	+= m_loresRectangle.x1;									//next y
		cbuf	+= (8*(unsigned int)(m_memoryRectangle.x1)/8) / sizeof(QWORD);			//next block in y
	}
}

/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::cacheToCoverageBufferFUSION()
 *
 * Description:		
 *
 * Parameters:		bx		= bucket x coordinate
 *					by		= bucket y coordinate
 *					bmin	= minimum dirty block inside the bucket (x)
 *					bmax	= maximum dirty block inside the bucket (x)
 *
 *****************************************************************************/

void OcclusionBuffer::cacheToCoverageBufferFUSION(int bx,int by,int bmin,int bmax)
{
	DPVS_ASSERT(bx>=0 && by >= 0);
	QWORD*	cbuf		= &m_CBuffer[by*m_coverageBufferBucketRowQWPitch + bx*m_coverageBufferBucketQWPitch];
	int		boffset		= ((unsigned int)(by)*BUCKET_H/8*m_loresRectangle.x1)			 + ((unsigned int)(bx)*BUCKET_W/8);		//bit #
	const int zx		= bx*(BUCKET_W/8);
	const int zy		= by*(BUCKET_H/8);

	DPVS_PROFILE(Statistics::incStatistic(Library::STAT_OCCLUSIONBUFFERBUCKETSPROCESSED,1));

#if defined(DPVS_DEBUG)
	IntRectangle	dirtyBlockRectangle( m_dirtyRectangle );
	dirtyBlockRectangle.downSample( 8,8 );
#endif

	//perform OR operation to coverage buffer and check changes
	for(int j=0;j<BUCKET_H/BLOCK_H;j++)		//blocks in y
	{
		if(m_dirtyScanlines.getByte(j))
		{
			int offset = boffset + bmin;

			for(int i=bmin; i<bmax; i++,offset++)
			{
				unsigned char&	blockFull	= m_blockBufferF[offset>>3];
				const unsigned char	bMask	= (unsigned char)(1<<(offset&7));

				if(blockFull & bMask)
					continue;

				QWORD cdata(m_cache->getReordered(i,j));

				if(cdata.full())
				{
					blockFull				  |= bMask;								// mark block as full
					m_blockBufferC[offset>>3] |= (unsigned char)(1<<(offset&7));	// [CONTRIBUTION]

					DPVS_ASSERT(int(zx+i) >= dirtyBlockRectangle.x0 && int(zx+i) < dirtyBlockRectangle.x1);
					DPVS_ASSERT(int(zy+j) >= dirtyBlockRectangle.y0 && int(zy+j) < dirtyBlockRectangle.y1);
					m_blockCoordinates[m_ZChangeCount++].set(zx+i,zy+j,true);	//collect to a zlist
					m_previousBlocksFilled++;
#ifdef DEBUG_FILL
					cbuf[i].setFull();//Both(FULL_MASK);
#endif
					continue;
				}

				//---------------------------------------------------
				// combine blocks of coverage buffer and cache
				//---------------------------------------------------

				if(cbuf[i].changesIfMergedWith(cdata))
				{
					DPVS_ASSERT(int(zx+i) >= dirtyBlockRectangle.x0 && int(zx+i) < dirtyBlockRectangle.x1);
					DPVS_ASSERT(int(zy+j) >= dirtyBlockRectangle.y0 && int(zy+j) < dirtyBlockRectangle.y1);

					cbuf[i] |= cdata;

					if(cbuf[i].full())
					{
						blockFull |= bMask;											//mark block as full
						m_blockBufferC[offset>>3] |= (unsigned char)(1<<(offset&7));// [CONTRIBUTION]
						m_blockCoordinates[m_ZChangeCount++].set(zx+i,zy+j,true);	//collect to a zlist
						m_previousBlocksFilled++;
					}
					else
					{
						if(cbuf[i].getBitCount() >= m_contributionThreshold)
							m_blockBufferC[offset>>3] |= (unsigned char)(1<<(offset&7));// [CONTRIBUTION]

						m_blockBufferP[offset>>3]	|= bMask;						//mark block as partial
						m_blockCoordinates[m_ZChangeCount++].set(zx+i,zy+j,false);	//collect to a zlist
					}
				}
			}
		}

		boffset	+= m_loresRectangle.x1;								// next y
		cbuf	+= (8*(unsigned int)(m_memoryRectangle.x1)/8) / sizeof(QWORD);		// next block in y
	}
}


/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::clearZBuffer()
 *
 * Description:		Clears depth buffer if it is marked as dirty
 *
 * Notes:			Marks the HZ Buffer as fully modified
 *
 *****************************************************************************/

void OcclusionBuffer::clearZBuffer (void)
{
	DPVS_PROFILE(Statistics::incStatistic(Library::STAT_WRITEQUEUEDEPTHCLEARS,1));
	fillDWord(m_ZBuffer,0,m_loresRectangle.width()*m_blockClipRectangle.height());
	m_HZBuffer->setRegionModified(m_blockClipRectangle.x0,m_blockClipRectangle.y0, m_blockClipRectangle.x1,m_blockClipRectangle.y1);

#if defined (DPVS_OCCLUSIONBUFFER_INDEXBUFFER)	
	fillDWord(m_indexBuffer,0,m_loresRectangle.width()*m_blockClipRectangle.height());
#endif
}

/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::updateDepthBuffer()
 *
 * Description:		Updates depth buffer and the hierarchical depth buffer
 *
 *****************************************************************************/

// Special-case minmax routines for guaranteedly positive values. These routines
// don't perform any branches..
static DPVS_FORCE_INLINE INT32 positiveMax2 (INT32 a, INT32 b) 
{ 
	DPVS_ASSERT( a >= 0 && b >= 0);
	INT32 delta = (a-b);
	INT32 r     = a-(delta&(delta>>31));
	DPVS_ASSERT( r == ((a > b) ? a : b));
	return r;
}
static DPVS_FORCE_INLINE INT32 positiveMin2 (INT32 a, INT32 b) 
{ 
	DPVS_ASSERT (a >= 0 && b >= 0);
	INT32 delta = (a-b);
	INT32 r     = b+(delta&(delta>>31));
	DPVS_ASSERT( r == ((a < b) ? a : b));
	return r;
}

void OcclusionBuffer::updateDepthBuffer (void)
{
	if (!m_ZChangeCount)						// no changes?
		return;

	DPVS_PROFILE(Statistics::incStatistic(Library::STAT_WRITEQUEUEDEPTHWRITES,m_ZChangeCount));

	//--------------------------------------------------------------------
	// Smallest rectangle that encloses all of the updated pixels
	//--------------------------------------------------------------------

	IntRectangle dirtyBlockRectangle	(m_dirtyRectangle);
	dirtyBlockRectangle.downSample		(8,8);

	int				minx		= dirtyBlockRectangle.x1;								// init to invalid values..
	int				maxx		= dirtyBlockRectangle.x0;
	int				miny		= dirtyBlockRectangle.y1;
	int				maxy		= dirtyBlockRectangle.y0;
	const ByteLoc*	changes		= &m_blockCoordinates[0];
	int				changeCount = m_ZChangeCount;
	UINT32*			zbuffer		= m_ZBuffer;

	const int CHUNK = 64;															// size of the chunk we process at a time...

	for (int j = 0; j < changeCount; j+=CHUNK, changes+=CHUNK)
	{
		int cnt = changeCount-j;
		if (cnt > CHUNK)
			cnt = CHUNK;

		UINT32 tmp[CHUNK];															// temporary array

		m_ZGradient.interpolateList((float*)tmp,changes,cnt);						// interpolate <64 depth values

		for (int i = 0; i < cnt; i++)												
		{
			const int x = changes[i].getX();										// read the x,y values
			const int y = changes[i].getY();

			DPVS_ASSERT(x >= dirtyBlockRectangle.x0 && x < dirtyBlockRectangle.x1);
			DPVS_ASSERT(y >= dirtyBlockRectangle.y0 && y < dirtyBlockRectangle.y1);

			UINT32	zval		= tmp[i];											// read interpolated Z value
			UINT32& currentz	= zbuffer[y*m_loresRectangle.x1 + x];				// read zbuffer value

			if (zval > currentz)													// select farther of two values (conservative)
			{
				currentz = zval;													// update Z-buffer
				minx = positiveMin2(x,minx);										// update bounding rectangle..
				maxx = positiveMax2(x,maxx);
				miny = positiveMin2(y,miny);
				maxy = positiveMax2(y,maxy);

	#if defined (DPVS_OCCLUSIONBUFFER_INDEXBUFFER)	
				m_indexBuffer[y*m_loresRectangle.x1 + x] = m_indexBufferIndex;		// this is debug info
	#endif

			}
		}
	}

	//--------------------------------------------------------------------
	// Tell HZBuffer that a portion of the Z-buffer has been modified
	// (first check that any changes really happened).
	//--------------------------------------------------------------------

	if (maxx >= minx)
	{
		DPVS_ASSERT(minx >= dirtyBlockRectangle.x0 && minx < dirtyBlockRectangle.x1);
		DPVS_ASSERT(miny >= dirtyBlockRectangle.y0 && miny < dirtyBlockRectangle.y1);
		DPVS_ASSERT(maxx >= dirtyBlockRectangle.x0 && maxx < dirtyBlockRectangle.x1);
		DPVS_ASSERT(maxy >= dirtyBlockRectangle.y0 && maxy < dirtyBlockRectangle.y1);

		DPVS_ASSERT(maxy >= miny);
		m_HZBuffer->setRegionModified (minx,miny,maxx+1,maxy+1);	// exclusive region
	}
}

//------------------------------------------------------------------------
