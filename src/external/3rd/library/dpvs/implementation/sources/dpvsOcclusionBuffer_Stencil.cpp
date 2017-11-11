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
 * Description: 	Occlusion buffer stencil code (some embedded in Test and Redner aswell)
 *
 * $Archive: /dpvs/implementation/sources/dpvsOcclusionBuffer_Stencil.cpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 10.06.01 15:45 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#include "dpvsOcclusionBuffer.hpp"
#include "dpvsFiller.hpp"
#include "dpvsHZBuffer.hpp"
using namespace DPVS;

/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::initStencil()
 *
 * Description:		Fills stencil buffer contents inside the current scissor.
 *
 * Notes:			Since all operations clip their input to scissor, stencil
 *					buffer doesn't have to be initialized "perfectly".
 *					Bucket precision is good ok.
 *
 *****************************************************************************/

void OcclusionBuffer::initStencil(ImpCamera* c)
{
	m_useStencil = c!=null;

	if(!m_useStencil)
		return;

	setScissor(c);

	QWORD fillValue(FULL_MASK,FULL_MASK);

	IntRectangle br( m_intScissor );
	br.downSample( BUCKET_W,BUCKET_H );

	for(int y=br.y0;y<br.y1;y++)
	{
		for(int x=br.x0;x<br.x1;x++)
		{
			QWORD* stencil = m_stencilBuffer + (y*m_bucketRectangle.x1 + x) * BUCKET_W;

			for(int i=0;i<BUCKET_W;i++)
				stencil[i] = fillValue;
		}
	}
}


/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::clearAccordingToStencil()
 *
 * Description:		Clear all buffers according to stencil buffer
 *
 * Notes:			Clear coverage, DEB, full & partial blocks buffers if the
 *					corresponding 8x8 block in stencil buffer is set and
 *					interpolated Z value is closer than the value in DEB.
 *
 *****************************************************************************/

void OcclusionBuffer::clearAccordingToStencil()
{
#if defined(DPVS_DEBUG)
	testBucketStability();
#endif

	IntRectangle br( m_intScissor );
	br.downSample( BUCKET_W,BUCKET_H );

	//-------------------------------------------------------
	// For all buckets inside scissor area
	//-------------------------------------------------------

	for(int by=br.y0;by<br.y1;by++)
	for(int bx=br.x0;bx<br.x1;bx++)
	{
		DPVS_ASSERT( by >= 0 && bx >= 0);

		//-------------------------------------------------------
		// Computer per-bucket data
		//-------------------------------------------------------

		int		bucketNumber	= by*m_bucketRectangle.x1 + bx;
		Bucket&	bucket			= m_bucket[bucketNumber];
		QWORD*	stencil			= m_stencilBuffer + bucketNumber * BUCKET_W;
		QWORD*	cbuf			= &m_CBuffer[by*m_coverageBufferBucketRowQWPitch + bx*m_coverageBufferBucketQWPitch];
		int		boffset			= ((unsigned int)(by)*BUCKET_H/8*m_loresRectangle.x1) + ((unsigned int)(bx)*BUCKET_W/8);		//bit #

		//-------------------------------------------------------
		// For all block inside current block
		//-------------------------------------------------------

		bool	reflectionTargetsFixed = false;

		for(int j=0;j<BUCKET_H/8;j++)
		{
			for(int i=0;i<BUCKET_W/8;i++)
			{
				//-------------------------------------------------------
				// Fetch 8x8 block from stencil buffer (same format as cache)
				//-------------------------------------------------------

				const unsigned char* sbuf = stencil[i*BLOCK_W].getBytePtr(j);
				QWORD sdata(OcclusionBuffer::Cache::byteReorder(sbuf+4*24), OcclusionBuffer::Cache::byteReorder(sbuf+0));

				//-------------------------------------------------------
				// If the block is empty, no clearing must be performed
				//-------------------------------------------------------

				if(sdata.empty())
					continue;

				//-------------------------------------------------------
				// Interpolate Z.
				// If the (Z > DEB and FullBlocks buffer is set), don't clear
				//-------------------------------------------------------

				const int			offset		= boffset + i;
				const int			index		= offset>>3;
				const unsigned char	bMask		= (unsigned char)(1<<(offset&7));
				const bool			fullBlock	= (m_blockBufferF[index] & bMask) != 0;
				const int			zx			= bx*(BUCKET_W/8) + i;
				const int			zy			= by*(BUCKET_H/8) + j;

				if(fullBlock && (Math::bitPattern(m_ZGradient.interpolate(zx,zy)) > m_ZBuffer[zy*m_loresRectangle.x1 + zx]))
					continue;

				//-------------------------------------------------------
				// Clear coverage, DEB, full & partial blocks buffers
				//-------------------------------------------------------

				cbuf[i].setZero();//Both(0);										// clear coverage buffer
				m_ZBuffer[zy*m_loresRectangle.x1 + zx] = 0;				// clear DEB
				m_blockBufferF[index] &= ~bMask;						// 
				m_blockBufferC[index] &= ~bMask;						// 
				m_blockBufferP[index] &= ~bMask;						// 

				if(fullBlock)
				{
					//-------------------------------------------------------
					// if bucket was full, there might be releftion target PAST it
					//-------------------------------------------------------

					if(bucket.isFull())
					{
						m_totalBucketsFilled--;							// statistics
						DPVS_ASSERT(m_totalBucketsFilled>=0);

						if(!reflectionTargetsFixed)
						{
							const int bucketNumber = bucket.findNumber(m_bucketRectangle.x1);

							for(int b=by*m_bucketRectangle.x1;b<bucketNumber;b++)	// process buckets on the left
							{
								Bucket& b2 = m_bucket[b];
								if(!b2.hasReflectionTarget() ||						// Doesn't have one (bucket row is full)
									b2.getReflectionTarget() > bucketNumber)		// has one to the right
									b2.setReflectionTarget(bucketNumber);			// this bucket is now a valid receiver
							}
							reflectionTargetsFixed = true;
						}
					}

					bucket.addFullBlockCount(-1);									// block is no longer full
					bucket.clearReflectionTarget();									// cannot have anymore (not full)
					DPVS_ASSERT(bucket.getFullBlockCount()>=0 && bucket.getFullBlockCount()<(BUCKET_H*BUCKET_W/64));	//BW 32
				}
			}

			boffset	+= m_loresRectangle.x1;								// next y
			cbuf	+= (8*(unsigned int)(m_memoryRectangle.x1)/8) / sizeof(QWORD);		// next block in y
		}
	}

#if defined(DPVS_DEBUG)
	testBucketStability();
#endif

	//-----------------------------------------------------
	// Z-Buffer contents have changed (potentially)
	//-----------------------------------------------------

	// DEBUG DEBUG: give more exact modification area? (is it worth it?)
	m_HZBuffer->setRegionModified	(m_blockScissor.x0,m_blockScissor.y0, m_blockScissor.x1,m_blockScissor.y1);
}

/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::stencilOp()
 *
 * Description:		
 *
 * Parameters:		visualize = boolean value indicating whether debug visualization should be performed
 *
 * Notes:			Checks all buckets and clears them after they have been processed.
 *					Therefore no "dirty bucket"-list is maintained and each bucket
 *					must be cleared under all circumtances (i.e. before each continue-statement). 
 *
 *****************************************************************************/

bool OcclusionBuffer::stencilOp(EdgeSilhouette& es)
{
	DPVS_ASSERT(m_useStencil);

#if defined(DPVS_DEBUG)
	m_cache->assertEmpty();		// DEBUG DEBUG HC
#endif

	//-----------------------------------------------------
	// set dirty rectangle
	//-----------------------------------------------------

	setDirtyRectangle(m_intScissor);

	//-----------------------------------------------------
	// clear all object related variables
	//-----------------------------------------------------

	m_mode					= STENCIL;
	m_visiblePointValid		= false;

	//-----------------------------------------------------
	// clip and scan convert edges
	//-----------------------------------------------------

	if(!setEdges(es,false))	// do not validate dirty rectangle
		return false;

	//-----------------------------------------------------
	// construct bucket resolution dirty rectangle
	//-----------------------------------------------------

	IntRectangle dirtyBucketRectangle	( m_intScissor );
	dirtyBucketRectangle.downSample		( BUCKET_W,BUCKET_H );

	QWORD		fvalue;
	QWORD		overallStencil(0,0);

	DPVS_ASSERT(dirtyBucketRectangle.y0 >= 0 && dirtyBucketRectangle.x0 >= 0);

	//-----------------------------------------------------
	// process all dirty buckets
	//-----------------------------------------------------

	for(int by=dirtyBucketRectangle.y0; by<dirtyBucketRectangle.y1 ;by++)
	{
		int	b = by*m_bucketRectangle.x1;

		fvalue.setZero();//Both(0);

		for(int bx=dirtyBucketRectangle.x0; bx<dirtyBucketRectangle.x1 ;bx++)
		{
			int	bmin = -1;	// intialized due to compiler warning
			int	bmax = -1;	// intialized due to compiler warning

			DPVS_ASSERT((b+bx)>=0 && (b+bx)<m_bucketRectangle.area());

			//-------------------------------------------------------
			// skip bucket if empty and there's not an ongoing fill
			//-------------------------------------------------------

			QWORD *stencil	= m_stencilBuffer + (b+bx) * BUCKET_W;		// stencil buffer position
			int limit		= m_intScissor.x1 - bx*BUCKET_W;
			if(limit>BUCKET_W)	limit = BUCKET_W;

			m_cache->init();											// initialize cache

			OcclusionBuffer::Bucket	&bucket = m_bucket[b+bx];			// reference to current bucket

			int	xmin,xmax;
			if(rasterizeEdges(bucket,xmin,xmax))
			{
				m_cache->setRange(xmin,xmax);
				m_cache->fillerXOR(m_dirtyScanlines, fvalue,bmin,bmax,limit);
			}
			else
			{
				if(fvalue.empty())
				{
					for(int i=0;i<BUCKET_W;i++)
						stencil[i].setZero();//Both(0);

					continue;
				}

				//-------------------------------------------------------
				// fill cache (no filler logic is needed)
				//-------------------------------------------------------

				bmin = 0;
				bmax = (limit+7)>>3;
				m_dirtyScanlines = fvalue;
				m_cache->fillPlane0(0,limit,fvalue);
			}

			//--------------------------------------------------------------------
			// update stencil buffer
			//--------------------------------------------------------------------

			QWORD *cache = (*m_cache)[0];

			for(int i=0;i<BUCKET_W;i++)
			{
				stencil[i] &= cache[3*i];			// update stencil
				cache[3*i].setZero();//Both(0);				// clear cache
				overallStencil |= stencil[i];		// track if there is anything left in the stencil
			}

			if(bmax<4)								// clear last (exclusive line...)
				cache[bmax*8+1].setZero();//Both(0);
		}
	}

#if defined(DPVS_DEBUG)
	m_cache->assertEmpty();		// DEBUG DEBUG HC
#endif

	m_dirtyRectangleSet	= false;

	if(overallStencil.empty())
		return false;
	
	return true;
}

//------------------------------------------------------------------------
