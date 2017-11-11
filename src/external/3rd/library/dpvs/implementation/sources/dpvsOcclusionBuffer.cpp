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
 * Description: 	Occlusion buffer code, generic
 *
 * $Archive: /dpvs/implementation/sources/dpvsOcclusionBuffer.cpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 11/20/01 3:02p $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#include "dpvsOcclusionBuffer.hpp"
#include "dpvsBlockBuffer.hpp"
#include "dpvsFiller.hpp"
#include "dpvsHZBuffer.hpp"
#include "dpvsImpCamera.hpp"			// viewport
#include "dpvsSpaceManager.hpp"

using namespace DPVS;

/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::OcclusionBuffer()
 *
 * Description:		
 *
 * Parameters:		screenWidth	 =
 *					screenHeight = 
 *					sx			 =	
 *					sy			 =
 *
 *****************************************************************************/

OcclusionBuffer::OcclusionBuffer	(int screenWidth, int screenHeight, float sx,float sy) :
	m_edgeManager(),
	m_edges(),
	m_edges2(),
	m_clipRectangle(),
	m_memoryRectangle(),
	m_bucketRectangle(),
	m_loresRectangle(),
	m_blockClipRectangle(),
	m_blockMemoryRectangle(),
	m_silhouetteRectangle(),
	m_bufferAlloc(null),
	m_ZBuffer(null),
	m_CBuffer(null),
	m_stencilBuffer(null),
	m_blockBufferF(null),
	m_blockBufferC(null),
	m_blockBufferP(null),
	m_blockBuffer(null),
	m_HZBuffer(null),
	m_cache(null),
	m_bucket(),
	m_justFilledBuckets(),
	m_blockCoordinates(),
	m_staticZBuffer(null),
	m_staticCoverageBuffer(null),
	m_staticFullBlocks(null),
	m_staticPartialBlocks(null),
	m_subSampling(),
	m_ooSubSampling(),
	m_verticalBucketCrossings(0),
	m_coverageBufferBlockQWPitch(0),
	m_coverageBufferBlockRowQWPitch(0),
	m_coverageBufferBucketQWPitch(0),
	m_coverageBufferBucketRowQWPitch(0),
	m_scissorCamera(null),
	m_scissorSilhouette(null),
	m_floatScissor(),
	m_intScissor(),
	m_int16Scissor(),
	m_blockScissor(),
	m_ZGradient(),
	m_dirtyRectangle(),
	m_dummyContributed(),
	m_dirtyBucketRectangle(),
	m_dirtyScanlines(),
	m_edgeCount(0),
	m_totalBlocksFilled(0),
	m_totalBucketsFilled(0),
	m_previousBlocksFilled(0),
	m_previousBucketsFilled(0),
	m_ZChangeCount(0),
	m_contributionThreshold(64),
	m_mode(WRITE),
	m_visiblePoint(),
	m_visiblePointValid(false),
	m_useStencil(false),
	m_empty(true),
	m_dirtyRectangleSet(false),
	m_contributionCullingEnabled(false),
	m_lastWriteCost(0)
#if defined (DPVS_OCCLUSIONBUFFER_INDEXBUFFER)	
	,m_indexBuffer(null),
	m_indexBufferIndex(0)
#endif
{

	DPVS_ASSERT(screenWidth>0);
	DPVS_ASSERT(screenHeight>0);

	//-----------------------------------------------------
	// Minimum size is 32x64 (tile)
	//-----------------------------------------------------

	if(screenWidth*sx  < DPVS_TILE_WIDTH)	sx = ((float)DPVS_TILE_BITS)/screenWidth;
	if(screenHeight*sy < 64)				sy = (64.f)/screenHeight;

	//-----------------------------------------------------
	// Set subsampling
	//-----------------------------------------------------

	m_subSampling.x = sx;
	m_subSampling.y = sy;
	m_ooSubSampling.x = 1.f / m_subSampling.x;
	m_ooSubSampling.y = 1.f / m_subSampling.y;
	m_ZGradient.setSubSampling(sx*0.125f,sy*0.125f);			// divide by 8 since we want 8x8 block rez!


	IntRectangle virtualRectangle(0,screenWidth, 0,screenHeight);

	//-----------------------------------------------------
	// Construct optimally sized clipping rectangle
	//-----------------------------------------------------

	m_clipRectangle.x0 = Math::intFloor(virtualRectangle.x0 * sx);	// inclusive
	m_clipRectangle.y0 = Math::intFloor(virtualRectangle.y0 * sy);	// inclusive
	m_clipRectangle.x1 = Math::intCeil (virtualRectangle.x1 * sx);	// exclusive	WAS +1
	m_clipRectangle.y1 = Math::intCeil (virtualRectangle.y1 * sy);	// exclusive	WAS +1
	m_clipRectangle.setSpace(SPACE_OCCLUSION);

	setScissor(null);

	m_bucketRectangle	= m_clipRectangle;
	m_bucketRectangle.downSample( BUCKET_W, BUCKET_H );
	m_bucketRectangle.setSpace(SPACE_OCCLUSION_BUCKET);

	m_memoryRectangle	= m_bucketRectangle;
	m_memoryRectangle.upSample( BUCKET_W, BUCKET_H );
	m_memoryRectangle.setSpace(SPACE_OCCLUSION);

	DPVS_ASSERT(m_memoryRectangle.x1>0 && m_memoryRectangle.y1>0);

	m_blockClipRectangle	= m_clipRectangle;
	m_blockClipRectangle.downSample( 8,8 );
	m_blockClipRectangle.setSpace(SPACE_OCCLUSION_BLOCK);

	m_blockMemoryRectangle	= m_memoryRectangle;
	m_blockMemoryRectangle.downSample( 8,8 );
	m_blockMemoryRectangle.setSpace(SPACE_OCCLUSION_BLOCK);

	//NOTE: HCBuffer tests coverage with DWORDs and therefore width has to be MULTIPLE OF 32!
	m_loresRectangle.set(	0, getNextPowerOfTwo(UINT32(m_blockMemoryRectangle.x1)),
							0, getNextPowerOfTwo(UINT32(m_blockMemoryRectangle.y1)) );
	m_loresRectangle.pad(32,1);
	m_loresRectangle.setSpace(SPACE_OCCLUSION_BLOCK);

	// we allocate cbuffer,zbuffer and blockbuffer with a single call. All buffers
	// are then cache-line aligned separately.
	
	const int allocSize = 
					bitsToQWords(2*m_memoryRectangle.area())*sizeof(QWORD) +		// coverage, validation & stencil buffer
							        m_loresRectangle.area()*sizeof(UINT32) +		// Z-buffer
					 bitsToBytes(3*m_loresRectangle.area()) +	// full & almostFull & partial blocks
								7*DPVS::CACHE_LINE_SIZE;						// 7 extra lines for alignment (one for each alloc)

	m_bufferAlloc = NEW_ARRAY<unsigned char>(allocSize);

	fillByte (m_bufferAlloc, 0, allocSize * sizeof(unsigned char));

	m_stencilBuffer		= reinterpret_cast<QWORD*>(alignCacheLine	(m_bufferAlloc));
	m_CBuffer			= reinterpret_cast<QWORD*>(alignCacheLine	(m_stencilBuffer+ bitsToQWords(m_memoryRectangle.area())));
	m_ZBuffer			= reinterpret_cast<UINT32*>(alignCacheLine	(m_CBuffer		+ bitsToQWords(m_memoryRectangle.area())));

	//--------------------------------------------------------------------
	// If the resolution is small enough, we assign the block buffers 
	// into the scratchpad...
	//--------------------------------------------------------------------

	if ((bitsToBytes(m_loresRectangle.area ())) <= 1024)
	{
		m_blockBufferF	= reinterpret_cast<unsigned char*>(Scratchpad::getAddress(Scratchpad::BLOCKBUFFERS));
	}
	else
		m_blockBufferF	= reinterpret_cast<unsigned char*>(alignCacheLine	(m_ZBuffer		+ m_loresRectangle.area ()));

	m_blockBufferC		= reinterpret_cast<unsigned char*>(alignCacheLine	(m_blockBufferF	+ bitsToBytes(m_loresRectangle.area ())));
	m_blockBufferP		= reinterpret_cast<unsigned char*>(alignCacheLine	(m_blockBufferC	+ bitsToBytes(m_loresRectangle.area ())));

#if defined (DPVS_OCCLUSIONBUFFER_INDEXBUFFER)	
	m_indexBuffer		= NEW_ARRAY<UINT32>(m_loresRectangle.area());
#endif
	

	m_cache				= NEW<Cache>();

	m_bucket.reset				(m_bucketRectangle.area());
	m_justFilledBuckets.reset	(m_bucketRectangle.area());

	//create hierarchical buffers

	m_blockBuffer		= new (MALLOC(sizeof(BlockBuffer))) BlockBuffer(m_loresRectangle.width(), m_loresRectangle.height() ,m_blockBufferF, m_blockBufferP);
	m_HZBuffer			= new (MALLOC(sizeof(IHZBuffer)))  IHZBuffer(m_ZBuffer,getHighestSetBit(UINT32(m_loresRectangle.width())),getHighestSetBit(UINT32(m_loresRectangle.height())),m_loresRectangle.width()*sizeof(UINT32));

	m_blockCoordinates.reset	(m_blockClipRectangle.area());		// Was m_loresRectangle

	//optional static buffers
	m_staticCoverageBuffer	= null;
	m_staticZBuffer			= null;

	m_dummyContributed.reset(m_bucketRectangle.area());
	m_scissorSilhouette = NEW<EdgeSilhouette>();

	//-----------------------------------------------------
	// Additional initialization, allocate everything before calling
	//-----------------------------------------------------

	createBlockMasks();
	initializeBuffers();
}

/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::~OcclusionBuffer()
 *
 * Description:		Destructor
 *
 *****************************************************************************/

OcclusionBuffer::~OcclusionBuffer	(void)
{
	DELETE(m_cache);
	DELETE(m_blockBuffer);
	DELETE(m_HZBuffer);

	DELETE_ARRAY(m_staticCoverageBuffer);
	DELETE_ARRAY(m_staticZBuffer);
	DELETE_ARRAY(m_staticFullBlocks);
	DELETE_ARRAY(m_staticPartialBlocks);
	DELETE_ARRAY(m_bufferAlloc);			// this releases cbuffer,zbuffer,fullbocks,partialblocks

#if defined (DPVS_OCCLUSIONBUFFER_INDEXBUFFER)	
	DELETE_ARRAY (m_indexBuffer);
#endif
	DELETE (m_scissorSilhouette);
}


/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::Bucket::Bucket()
 *
 * Description:		
 *
 *****************************************************************************/

OcclusionBuffer::Bucket::Bucket() :
		m_firstEdge				(-1),
		m_reflectionTarget		(-1),
		m_fullBlockCount		(0),
		m_bx					(0),
		m_by					(0),
		m_pendingClear			(false),
		m_padded				(false),
		m_paddedTwice			(false),
		m_maxDepth				(0),
		m_coverageBufferOffset	(0)
{
	clear();
	clearState();
}

/*****************************************************************************
 *
 * Function:		DPVS::EdgeManager::resize(int newSize)
 *
 * Description:		Resizes egdemanager capacity (internal)
 *
 * Parameters:		newSize = size of allocation
 *
 * Notes:			In .cpp file in order to avoid code bloat
 *
 *****************************************************************************/

void OcclusionBuffer::EdgeManager::resize(int newSize)
{
	DPVS_ASSERT(newSize >= 0);
	m_entries.resize(newSize);
}


/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::setDirtyRectangle()
 *
 * Description:		
 *
 * Parameters:
 *
 *****************************************************************************/

bool OcclusionBuffer::setDirtyRectangle(const IntRectangle& rect)
{
	DPVS_ASSERT(rect.getSpace() == SPACE_OCCLUSION);

	m_dirtyRectangle = rect;
	bool valid = m_dirtyRectangle.intersect(m_intScissor);		// clip to scissor

	m_dirtyBucketRectangle = m_dirtyRectangle;
	m_dirtyBucketRectangle.downSample( BUCKET_W,BUCKET_H );
	m_dirtyRectangleSet = true;

	return valid;
}

/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::clearPotentiallyAffectedBuckets()
 *
 * Description:		Checks consistency of all bucket data.
 *
 * Notes:			Only functional in debug build
 *
 *****************************************************************************/

void OcclusionBuffer::clearPotentiallyAffectedBuckets	(void)
{
	for(int by=m_dirtyBucketRectangle.y0; by<m_dirtyBucketRectangle.y1 ;by++)
	for(int bx=m_dirtyBucketRectangle.x0; bx<m_dirtyBucketRectangle.x1 ;bx++)
	{
		Bucket& b = m_bucket[by*m_bucketRectangle.x1+bx];
		b.clear();
	}

	//---------------------------------------------------------------
	// after this function all buckets must by in cleared state
	//---------------------------------------------------------------

#ifdef DPVS_DEBUG
	testBucketStability();
#endif
}


/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::testBucketStability()
 *
 * Description:		Checks consistency of all bucket data.
 *
 * Notes:			Only functional in debug build
 *
 *****************************************************************************/

void OcclusionBuffer::testBucketStability (void) const
{
#ifdef DPVS_DEBUG
	const int area = m_bucketRectangle.area();
	for(int i=0;i<area;i++)
	{
		if(m_bucket[i].isFull() && m_bucket[i].getReflectionTarget()==-1)
		{
			int by = i / m_bucketRectangle.x1;		// y position of bucket
			int bx = i % m_bucketRectangle.x1;		// x position of bucket
			for(int j=bx+1;j<m_bucketRectangle.x1;j++)
				DPVS_ASSERT(m_bucket[by*m_bucketRectangle.x1+j].isFull());
		}
	}
#endif
}

/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::minimizeMemoryUsage()
 *
 * Description:		Minimizes memory footprint of the occlusion buffer
 *
 *****************************************************************************/

void OcclusionBuffer::minimizeMemoryUsage	(void)
{
	//nada
}

/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::setScissor()
 *
 * Description:		
 *
 *****************************************************************************/

void OcclusionBuffer::setScissor (const ImpCamera* c)
{
	if(c==null)
	{
		m_scissorCamera		= null;
		return;
	}

	if(c==m_scissorCamera)
		return;

	//-----------------------------------------------------
	// Assign scissor camera
	//-----------------------------------------------------

	m_scissorCamera = c;

	//-----------------------------------------------------
	// subsample scissor from viewport
	//-----------------------------------------------------

	setScissor(c->getRasterViewport());
}

/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::setScissor()
 *
 * Description:		
 *
 * Notes:			
 *
 *****************************************************************************/


void OcclusionBuffer::setScissor	(const FloatRectangle& rect)
{
	SpaceManager::rasterToOcclusion(m_intScissor,rect,m_subSampling);

	//-----------------------------------------------------
	// full screen scissor (no portals)?
	//-----------------------------------------------------

	DPVS_DEBUG_CODE(bool value =) m_intScissor.intersect(m_clipRectangle);
	DPVS_ASSERT(value);

	//-----------------------------------------------------
	// block resolution scissor
	//-----------------------------------------------------

	m_blockScissor = m_intScissor;
	m_blockScissor.downSample(8,8);
	m_blockScissor.setSpace(SPACE_OCCLUSION_BLOCK);

	//-----------------------------------------------------
	// floating point precision scissor
	//-----------------------------------------------------

	m_floatScissor.set ((float)m_intScissor.x0,(float)m_intScissor.x1,(float)m_intScissor.y0,(float)m_intScissor.y1);
	m_floatScissor.setSpace(SPACE_OCCLUSION);

	//-----------------------------------------------------
	// subpixel (16*integer) scissor
	//-----------------------------------------------------

	m_int16Scissor = m_intScissor;
	m_int16Scissor.x0 *= 16;	// inclusive
	m_int16Scissor.x1 *= 16;	// exclusive
	m_int16Scissor.y0 *= 16;	// inclusive
	m_int16Scissor.y1 *= 16;	// exclusive (never drawn ~ inclusive)
	m_int16Scissor.x1 -= 1;		// inclusive
}





















/******************************************************************************

						Write silhouette functions

******************************************************************************/
/*
void OcclusionBuffer::constructWritePolygon	(const Vector2* vloc, int cnt, const IntRectangle &rect2, IntRectangle &DWORDrect)
{

//	//downsampled version
//	DPVS::Rectangle<float> rect(1000,0,1000,0);
//	for(i=0;i<cnt;i++)
//		rect.grow(vloc[i].x,vloc[i].y);			//input bounds

//	rect2.x0 = FastMath::intFloor(rect.x0*m_subSampling.x/8);
//	rect2.x1 = FastMath::intFloor(rect.x1*m_subSampling.x/8+1);
//	rect2.y0 = FastMath::intFloor(rect.y0*m_subSampling.y/8);
//	rect2.y1 = FastMath::intFloor(rect.y1*m_subSampling.y/8+1);	//sub sampled 8x8 block bounds

//	rect.downSample(8,8);
//	rect.x1++;
//	rect.y1++;
	//block clip rectangle
//	rect2 = rect;
//	rect2.x0 = FastMath::intFloor(rect.x0);
//	rect2.y0 = FastMath::intFloor(rect.y0);
//	rect2.x1 = FastMath::intCeil (rect.x1);
//	rect2.y1 = FastMath::intCeil (rect.y1);

//	if(!rect2.intersect(m_blockClipRectangle))
//		return false;

	DWORDrect = rect2;
	DWORDrect.downSample(32,1);

	//enlarge buffers if required
	if(cnt>m_silhouetteVertexCount)
	{
		m_silhouetteEdgeFunctions.resize(cnt);
		m_silhouetteActiveEdgeList.resize(cnt*m_blockClipRectangle.height());
		m_silhouetteActiveEdgeCount.resize(m_blockClipRectangle.height());
		m_silhouetteVertexCount = cnt;
	}

	memset(&m_silhouetteActiveEdgeCount[0],0,sizeof(int)*rect2.height());
	memset(&m_silhouetteBuffer[rect2.y0*(m_silhouetteRectangle.x1/32)],0,sizeof(UINT32)*m_silhouetteRectangle.width()/32*rect2.height());

	Vector3* ef = &m_silhouetteEdgeFunctions[0];

	// calculate edgefunctions and assign edges to buckets
	{
		int i;
		for(i = 0; i < cnt; i++)
		{
			int a = i;
			int b = (i + 1)%cnt;
			float dx = (vloc[b].x - vloc[a].x) * m_subSampling.x;
			float dy = (vloc[b].y - vloc[a].y) * m_subSampling.y;
			ef[i].x =  dy;
			ef[i].y = -dx;
			ef[i].z = -(vloc[a].x*m_subSampling.x*ef[i].x + vloc[a].y*m_subSampling.y*ef[i].y);

			int	y0,y1;
			if(dy>0)	y0 = FastMath::intFloor(vloc[a].y*m_subSampling.y/8),	y1=FastMath::intFloor(vloc[b].y*m_subSampling.y/8+1);
			else		y0 = FastMath::intFloor(vloc[b].y*m_subSampling.y/8),	y1=FastMath::intFloor(vloc[a].y*m_subSampling.y/8+1);

			if(y0<0)								y0 = 0;
			if(y1>=m_blockClipRectangle.height())	y1 = m_blockClipRectangle.height();
			y0 -= rect2.y0;
			y1 -= rect2.y0;

			for(int j=y0;j<y1;j++)
				m_silhouetteActiveEdgeList[j*cnt + m_silhouetteActiveEdgeCount[j]++] = i;
		}
	}


	int	*activeEdges = &m_silhouetteActiveEdgeList[0];
	for(int y=rect2.y0; y<rect2.y1; y++,activeEdges+=cnt)
	{
		int	ecount = m_silhouetteActiveEdgeCount[y-rect2.y0];
		if(!ecount)
			continue;

		//start
		int x;
		for(x=rect2.x0; x<rect2.x1; x++)
		{
			int i;
			//evaluate edge functions for corner points of the block
			for(i=0; i<ecount; i++)
			{
				int	e = activeEdges[i];
				float	x0 = (8*x+0) * ef[e].x;
				float	x1 = (8*x+7) * ef[e].x;
				float	y0 = (8*y+0) * ef[e].y;
				float	y1 = (8*y+7) * ef[e].y;

				if(x0+y0+ef[e].z>0.f)	break;
				if(x1+y0+ef[e].z>0.f)	break;
				if(x0+y1+ef[e].z>0.f)	break;
				if(x1+y1+ef[e].z>0.f)	break;
			}
			if(i==ecount)	//start found
				break;
		}
		if(x==rect2.x1)
			continue;		//not found

		int	left = x;

		//start
		for(x=rect2.x1-1; x>left; x--)
		{
			int i;
			//evaluate edge functions for corner points of the block
			for(i=0; i<ecount; i++)
			{
				int	e = activeEdges[i];
				float	x0 = (8*x+0) * ef[e].x;
				float	x1 = (8*x+7) * ef[e].x;
				float	y0 = (8*y+0) * ef[e].y;
				float	y1 = (8*y+7) * ef[e].y;

				if(x0+y0+ef[e].z>0.f)	break;
				if(x1+y0+ef[e].z>0.f)	break;
				if(x0+y1+ef[e].z>0.f)	break;
				if(x1+y1+ef[e].z>0.f)	break;
			}
			if(i==ecount)	//end found
				break;
		}

		int	right = x;

		//set pixels to bit buffer
		UINT32	*sbuf = m_silhouetteBuffer + y*m_silhouetteRectangle.width()/32;	//pos in bitbuffer
		for(x=left;x<=right;x++)
			sbuf[x/32] |= 1<<(x&31);


//		//visible, write block(s)
//		for(i=left;i<=right;i++)
//		{
//			for(int by=0;by<8;by++)
//			for(int bx=0;bx<8;bx++)
//			{
//				surf->setPixel(8*i+bx, 8*y+by, surf->getPixel(8*i+bx, 8*y+by) + 0x007f0000);
//			}
//		}
	}
}

template<class T>
T myMin(T a, T b)
{
	if(a < b) return a;
	else return b;
}

template<class T>
T myMax(T a, T b)
{
	if(a > b) return a;
	else return b;
}


//NOTE: input vertices must be in CW order
void OcclusionBuffer::constructTestPolygon(IntRectangle &rect, const Vector2* vloc, int cnt, int &ymin, int &ymax)
{
	int i;

	//clear
	for(i=rect.y0;i<rect.y1;i++)
	{
		m_testSilhouetteLeft [i] = 0x7fffffff;
		m_testSilhouetteRight[i] = 0x80000000;
	}

	//=====================================================
	// not so conservative test TSÄH
	//=====================================================

	ymin = 0x7fffffff;
	ymax = 0;

	for(i=0;i<cnt;i++)
	{
		int a = i;
		int b = (i+1) % cnt;
		float dy = vloc[b].y - vloc[a].y;

		Vector2 clipped[2];

		int	x0 = FastMath::intFloor(vloc[a].x/8);
		int	y0 = FastMath::intFloor(vloc[a].y/8);

		if(dy>0)	//RIGHT
		{
			setTestEdge(m_testSilhouetteRight,  vloc[a], vloc[b], clipped, false);
			x0 = FastMath::intFloor(clipped[0].x/8)+1;
			y0 = FastMath::intFloor(clipped[0].y/8);
			if(y0>=m_blockScissor.y0 && y0<m_blockScissor.y1)
				m_testSilhouetteRight[y0] = myMax(m_testSilhouetteRight[y0], x0);
		}
		else		//LEFT
		{
			setTestEdge(m_testSilhouetteLeft, vloc[b], vloc[a], clipped, true);
			x0 = FastMath::intFloor(clipped[1].x/8);
			y0 = FastMath::intFloor(clipped[1].y/8);
			if(y0>=m_blockScissor.y0 && y0<m_blockScissor.y1)
				m_testSilhouetteLeft[y0] = myMin(m_testSilhouetteLeft [y0], x0);
		}

		ymin = myMin(ymin, y0);
		ymax = myMax(ymax, y0+1);
	}

	ymin = myMax(ymin,m_blockScissor.y0);
	ymax = myMin(ymax,m_blockScissor.y1);

	//clamp
	for(i=ymin;i<ymax;i++)
	{
		if(m_testSilhouetteLeft[i]==0x7fffffff)
		{
			m_testSilhouetteLeft [i] = 0;
			m_testSilhouetteRight[i] = 0;
		}
		else
		{
			m_testSilhouetteLeft [i] = (UINT32)myMax(m_blockScissor.x0,(int)m_testSilhouetteLeft [i]);
			m_testSilhouetteRight[i] = (UINT32)myMin(m_blockScissor.x1,(int)m_testSilhouetteRight[i]);

			if(m_testSilhouetteRight[i]<m_testSilhouetteLeft [i])
			{
				m_testSilhouetteLeft [i] = 0;
				m_testSilhouetteRight[i] = 0;
			}
		}
	}

}

void OcclusionBuffer::setTestEdge(Array<int>& array, const Vector2& aa, const Vector2& bb, Vector2 clipped[2], bool left)
{
	int y;
	const int BLOCKSIZE = 8;

	Vector2 a, b;
	a = aa;
	b = bb;

	float	dx	= (b.x - a.x) / (b.y - a.y);

	//clip top
	if(a.y<m_floatScissor.y0)
	{
		float deltay = m_floatScissor.y0 - a.y;
		float xstep  = dx * deltay;
		a.x += xstep;
		a.y = m_floatScissor.y0;
	}

	//clip bottom
	if(b.y>m_floatScissor.y1)
		b.y = m_floatScissor.y1;

	if(a.y>=b.y)
		return;

	clipped[0] = a;
	clipped[1] = b;

	int		y0			= FastMath::intFloor(a.y);
	int		y1			= FastMath::intFloor(b.y);
	int		firstFullY	= (y0 + BLOCKSIZE-1)&(~(BLOCKSIZE-1));
	int		lastFullY	= y1&(~(BLOCKSIZE-1));
	float	subStep		= firstFullY - a.y;
	float	startx	    = dx * subStep + a.x;

	for(y = firstFullY; y < lastFullY; y += BLOCKSIZE)
	{
		int x0 = FastMath::intFloor(startx);						// changed to intFloor (was: (int))
		int x1 = FastMath::intFloor(startx + dx*(BLOCKSIZE-1));

		x0 /= 8;
		x1 /= 8;

		if(left)	array[y/BLOCKSIZE] = myMin(x0, x1);
		else		array[y/BLOCKSIZE] = myMax(x0, x1);

		startx += dx * BLOCKSIZE;
	}

	if((FastMath::intFloor(a.y) & ~(BLOCKSIZE-1)) == (FastMath::intFloor(b.y) & ~(BLOCKSIZE-1)))
		return;

	if(y1 >= firstFullY)
	{
		subStep = (firstFullY-1) - a.y;
		if(subStep < 0.f)		// WHOAH
			subStep = 0.f;

		startx  = (subStep * dx + a.x)/8;

		if(left)	array[y0 / BLOCKSIZE] = myMin(array[y0/BLOCKSIZE], FastMath::intFloor(startx));
		else		array[y0 / BLOCKSIZE] = myMax(array[y0/BLOCKSIZE], FastMath::intCeil(startx));
	}

	if(y1 >= lastFullY && y1 < m_intScissor.y1)
	{
		subStep = (lastFullY) - a.y;
		startx  = (subStep * dx + a.x)/8;

		if(left)	array[y1 / BLOCKSIZE] = myMin(array[y1/BLOCKSIZE], FastMath::intFloor(startx));
		else		array[y1 / BLOCKSIZE] = myMax(array[y1/BLOCKSIZE], FastMath::intCeil(startx));
	}
}

//------------------------------------------------------------------------

//Silhouette query
bool OcclusionBuffer::isSilhouetteOccluded(IntRectangle &BRr, const Vector2* vloc, int cnt,float z)
{
	DPVS_ASSERT(false);

	int	xpos,ypos;

	//-------------------------------------------------------------------------
	//DEBUG DEBUG
	//-------------------------------------------------------------------------
#ifdef ffffffffffffffffffffffffffffffffffffffffffffff
//	Vector2 vloc2[5];
	Vector2 vloc2[] =
	{
//		Vector2(150.f, 20.f),
//		Vector2(250.f, 60.f),
//		Vector2(240.f, 160.f),
//		Vector2(70.f, 200.f),
//		Vector2(80.f, 40.f)
		Vector2(100.f,  20.f),
		Vector2(150.f, 120.f),
		Vector2( 50.f, 120.f)
	};
	cnt = 3;

	DPVS::Rectangle<float> rect(vloc2[0].x,vloc2[0].x,vloc2[0].y,vloc2[0].y);
	for(int i=1;i<cnt;i++)
		rect.grow(vloc2[i].x,vloc2[i].y);							//input bounds

	Rectangle<int>	BRr;
	if(!getBlockRectangle_SCALE_REMOVED(BRr,rect))		//outside the scissor area
		return true;

	static float zrot = 0.f;
	zrot += 0.02f;
	Matrix3 rot;
	rot.ident();
	rot.rotateZ(zrot);


	for(i = 0; i < 5; i++)
	{
		Vector3 r(v[i].x, v[i].y, 0.f);
		r -= Vector3(160.f, 120.f, 0.f);
//		r.scale(srVector3(0.1f, 0.6f, 1.f));
		r.scale(Vector3(0.6f, 0.6f, 1.f));
		r = rot.transform(r);
//		r *= 0.6f;
		vloc2[i] = Vector2(r.x, r.y) + Vector2(160.f, 120.f) + Vector2(0.f, yofs);
	}
#endif

	DPVS_ASSERT(z>=0.f);

	if(isEmpty())
		return false;

	int ymin,ymax;

	//--------------------------------------------------------------------
	// Scan convert the silhouette
	//--------------------------------------------------------------------

	constructTestPolygon(BRr,vloc,cnt,ymin,ymax);

	if(ymin>=ymax)
		return true;

	//--------------------------------------------------------------------
	// Perform depth buffer test
	//--------------------------------------------------------------------

	if (m_HZBuffer->isSilhouetteVisible(&m_testSilhouetteLeft[ymin],&m_testSilhouetteRight[ymin],Math::bitPattern(z),ymin,ymax-ymin))
		return false;

	//--------------------------------------------------------------------
	// Perform coverage test
	//--------------------------------------------------------------------

	if( m_blockBuffer->isSilhouetteVisible(&m_testSilhouetteLeft[ymin],&m_testSilhouetteRight[ymin],ymin,ymax-ymin,xpos,ypos) )
		return false;

	//--------------------------------------------------------------------
	// Silhouette is visible
	//--------------------------------------------------------------------

	return true;
	
}
*/

//------------------------------------------------------------------------
