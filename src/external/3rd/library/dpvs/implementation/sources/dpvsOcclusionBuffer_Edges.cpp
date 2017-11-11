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
 * Description: 	Occlusion buffer edge handling code
 *
 * $Archive: /dpvs/implementation/sources/dpvsOcclusionBuffer_Edges.cpp $
 * $Author: wili $
 * $Revision: #1 $
 * $Modtime: 8.01.02 15:34 $
 * $Date: 2003/01/22 $
 *
 ******************************************************************************/

#include "dpvsOcclusionBuffer.hpp"
#include "dpvsFiller.hpp"
#include "dpvsStatistics.hpp"
#include "dpvsImpObject.hpp"		// just to get the cost enumerations
#include <cstdlib>

using namespace DPVS;

#define IF_USELESS_TO_RASTERIZE if(bucket.isFull() && m_mode==WRITE) if(bucket.getReflectionTarget()==-1 || m_bucket[bucket.getReflectionTarget()].getBucketX() >= m_dirtyBucketRectangle.x1)

/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::setEdges()
 *
 * Description:		Processes all edges of an object.
 *					Calls setEdge(), which clips and stores edge specific
 *					data.
 *
 * Parameters:		EdgeSilhouette describtin
 *
 * Returns:			false if failure
 *
 * Notes:			edges are stored as subpixel corrected setupped lines.
 *					out of bounds edges only need conservative setup
 *
 *****************************************************************************/

bool OcclusionBuffer::setEdges	(EdgeSilhouette& es, bool updateDirtyRectangle)
{
	//--------------------------------------------------------------------
	// Test edge count for validity (must have L&R, but not too many)
	//--------------------------------------------------------------------

	if( es.getEdgeCount(0)==0 || es.getEdgeCount(1)==0 ||
	   (es.getEdgeCount(0)+es.getEdgeCount(1))*2 > MAX_EDGECOUNT)	// An egde can result in 2 edges in the buffer (normal+filler edge)
	{
		m_dirtyRectangleSet = false;
		return false;
	}

	//--------------------------------------------------------------------
	// Validate fixed point vertex locations
	//--------------------------------------------------------------------

	es.validateFixedPoint(Vector2(m_subSampling.x*16.0f, m_subSampling.y*16.0f));

	const Vector2i*	fl	= es.getFixedPointVertices();

	//--------------------------------------------------------------------
	// Scan dirty rectangle from vertices.
	// TODO: when we're sure there are only valid vertices, scan from vertex array
	//--------------------------------------------------------------------

	if(updateDirtyRectangle)
	{
		IntRectangle dirtyRect(0x7FFFFFFF,0x80000000,0x7FFFFFFF,0x80000000);
		dirtyRect.setSpace(SPACE_OCCLUSION);

		for (int s=0;s<2;s++)
		{
			const Vector2i* edges	= es.getEdges(s);
			const int	N			= es.getEdgeCount(s);

			for (int i=0;i<N;i++)
			{
				const Vector2i& a = fl[edges[i].i];		// first vertex coordinates (fixed point)
				const Vector2i& b = fl[edges[i].j];		// second vertex coordinates (fixed point)
				dirtyRect.grow(a.i, a.j);
				dirtyRect.grow(b.i, b.j);
			}
		}

		dirtyRect.x0 = (dirtyRect.x0>>4);
		dirtyRect.y0 = (dirtyRect.y0>>4);
		dirtyRect.x1 = (dirtyRect.x1>>4) + 1;
		dirtyRect.y1 = (dirtyRect.y1>>4) + 1;

		DPVS_ASSERT(dirtyRect.x0 < dirtyRect.x1);
		DPVS_ASSERT(dirtyRect.y0 < dirtyRect.y1);

		if(!setDirtyRectangle(dirtyRect))
			return false;
	}

	//--------------------------------------------------------------------
	// clear stats
	//--------------------------------------------------------------------

	m_verticalBucketCrossings = 0;
	m_edgeCount	= 0;
	m_empty		= false;

	//--------------------------------------------------------------------
	// Set edges to occlusion buffer (wrote as a loop to conserve
	// code space)
	//--------------------------------------------------------------------

	for (int s=0;s<2;s++)
	{
		int	N = es.getEdgeCount(s);

		DPVS_ASSERT(N);
		DPVS_PROFILE(Statistics::incStatistic(Library::STAT_OCCLUSIONBUFFEREDGESRASTERIZED,N));

		if(N==0)
			return false;						// nada - this is illegal, but better not try to draw it

		const Vector2i* edges	= es.getEdges(s);
		const bool	leftEdge	= (s==0);

		for (int i=0;i<N;i++)
		{
			const Vector2i& a = fl[edges[i].i];	// first vertex coordinates (fixed point)
			const Vector2i& b = fl[edges[i].j];	// second vertex coordinates (fixed point)
			setEdge(a.i,a.j,b.i,b.j,!leftEdge);	// clips and adds 0-2 edges to edge buffer
		}
	}

	//--------------------------------------------------------------------
	// Indicate failure if reflection targets might theretically create
	// more edge instances than supported.
	//--------------------------------------------------------------------

	if((m_verticalBucketCrossings + m_edgeCount) > MAX_EDGECOUNT)
		return false;

	//--------------------------------------------------------------------
	// Clear all buckets that can potentially be affected
	//--------------------------------------------------------------------

	clearPotentiallyAffectedBuckets();

	//--------------------------------------------------------------------
	// Assign edges to buckets
	//--------------------------------------------------------------------

	assignEdgesToBuckets();		// cannot fail

	//--------------------------------------------------------------------
	// Success
	//--------------------------------------------------------------------

	return true;
}


/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::appendEdge()
 *
 * Description:		Appends one edge to m_edges and its bucket coordinates to m_edges2.
 *					Increases m_edgeCount by 1.
 *
 * Parameters:
 *
 *****************************************************************************/

void OcclusionBuffer::appendEdge	(int fx0,int fx1,int fy0,int fy1,bool leftEdge)
{
	//---------------------------------------------------------------
	// Allocate memory
	//---------------------------------------------------------------

	Edge& e			= m_edges.getElement(m_edgeCount);
	BucketEdge& be	= m_edges2.getElement(m_edgeCount);
	m_edgeCount++;

	//---------------------------------------------------------------
	// Set edge data
	//---------------------------------------------------------------

	e.m_fx0			= fx0;
	e.m_fx1			= fx1;
	e.m_fy0			= fy0;
	e.m_fy1			= fy1;
	e.m_leftEdge	= leftEdge;
	e.m_initialized	= false;

	int	bx0,bx1;

	if(fx0 < fx1)
	{
		bx0 = fx0 >> (4+BUCKET_XBITS);			// inclusive
		bx1 = fx1 >> (4+BUCKET_XBITS);			// inclusive
	}
	else
	{
		bx0 = fx1 >> (4+BUCKET_XBITS);			// inclusive
		bx1 = fx0 >> (4+BUCKET_XBITS);			// inclusive
	}

	const int by0 = fy0 >> (4+BUCKET_YBITS);			// inclusive
	const int by1 = ((fy1 >> 4) - 1) >> (BUCKET_YBITS);	// inclusive

	m_verticalBucketCrossings += (by1+1 - by0) * (bx1+1 - bx0);

	//---------------------------------------------------------------
	// copy bucket rectangle to integers and classify to buckets
	//---------------------------------------------------------------

	DPVS_ASSERT(bx0>=0 && bx1>=0 && by0>=0 && by1>=0);
	be.bx0 = (unsigned char)bx0;
	be.bx1 = (unsigned char)bx1;
	be.by0 = (unsigned char)by0;
	be.by1 = (unsigned char)by1;
}

/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::setEdge()
 *
 * Description:		Clips edge, appends it, and optionally appends one connecting
 *					edge for the filler. As a result, 0-2 edges can be appended.
 *
 * Parameters:		fx0		= x0 in 28.4 fixed point format
 *					fy0		= y0 in 28.4 fixed point format
 *					fx1		= x1 in 28.4 fixed point format
 *					fy1		= y1 in 28.4 fixed point format
 *					leftEdge= is it a left egde?
 *
 * Notes:
 *
 *****************************************************************************/

//input coordinates are 28.4 fixed point
void OcclusionBuffer::setEdge	(int fx0,int fy0,int fx1,int fy1,bool leftEdge)
{
	DPVS_ASSERT(m_scissorCamera);			// must have scissor

	//---------------------------------------------------------------
	// Edge classification
	//---------------------------------------------------------------

	enum
	{
		NONE	= 0,
		LEFT	= (1<<0),
		RIGHT	= (1<<1),
		TOP		= (1<<2),
		BOTTOM	= (1<<3)
	};

	const int SHIFT = 4;		// # of subpixels

	//-----------------------------------------------------------
	// Calculate frustum masks for vertices and clip/reject
	//-----------------------------------------------------------

	int		lefty0	= 0;
	int		lefty1	= 0;
	UINT32	mask0	= 0;
	UINT32	mask1	= 0;

	mask0 |= (fx0<m_int16Scissor.x0) ? LEFT : ((fx0>=m_int16Scissor.x1) ? RIGHT  : NONE);
	mask1 |= (fx1<m_int16Scissor.x0) ? LEFT : ((fx1>=m_int16Scissor.x1) ? RIGHT  : NONE);
	mask0 |= (fy0<m_int16Scissor.y0) ? TOP  : ((fy0>=m_int16Scissor.y1) ? BOTTOM : NONE);
	mask1 |= (fy1<m_int16Scissor.y0) ? TOP  : ((fy1>=m_int16Scissor.y1) ? BOTTOM : NONE);

	UINT32 maskBoth = mask0 & mask1;
	UINT32 maskDiff = mask0 ^ mask1;

	if(maskBoth & (RIGHT|TOP|BOTTOM))	//out from right,top,bottom -> reject
		return;

	if(maskBoth & LEFT)					//out from left
	{
		lefty0	= fy0;					//the edge became "clamp edge"
		lefty1	= fy1;
		fy0		= fy1;					//invalidate normal edge
		maskDiff= 0;
	}

	if(maskDiff)						//clip input
	{
		DPVS_PROFILE(Statistics::incStatistic(Library::STAT_OCCLUSIONBUFFEREDGESCLIPPING,1));

		if(maskDiff & (TOP|BOTTOM))		//clip horizontal
		{
			DPVS_ASSERT(fy0!=fy1);
			const float dx = (float)fx1-fx0;
			const float dy = (float)fy1-fy0;
			const float dxdy = dx/dy;			//dy != 0.f;

			if(maskDiff & TOP)
			{
				if(mask0 & TOP)		fx0 += Math::intFloor( dxdy*(m_int16Scissor.y0 - fy0)), fy0=m_int16Scissor.y0;
				if(mask1 & TOP)		fx1 += Math::intFloor( dxdy*(m_int16Scissor.y0 - fy1)), fy1=m_int16Scissor.y0;
			}
			if(maskDiff & BOTTOM)
			{
				if(mask0 & BOTTOM)	fx0 += Math::intFloor( dxdy*(m_int16Scissor.y1 - fy0)), fy0=m_int16Scissor.y1;
				if(mask1 & BOTTOM)	fx1 += Math::intFloor( dxdy*(m_int16Scissor.y1 - fy1)), fy1=m_int16Scissor.y1;
			}
		}

		//-----------------------------------------------------------
		// re-classify horizontally (y is of no interest anymore)
		//-----------------------------------------------------------

		mask0 = (fx0<m_int16Scissor.x0) ? LEFT : ((fx0>=m_int16Scissor.x1) ? RIGHT  : NONE);
		mask1 = (fx1<m_int16Scissor.x0) ? LEFT : ((fx1>=m_int16Scissor.x1) ? RIGHT  : NONE);
		maskDiff = mask0 ^ mask1;
		maskBoth = mask0 & mask1;

		if(maskBoth & RIGHT)			//out from right -> reject
		{
			return;
		}

		if(maskBoth & LEFT)
		{
			lefty0	= fy0;				//the edge became "clamp edge"
			lefty1	= fy1;
			fy0		= fy1;				//invalidate normal edge
			maskDiff= 0;
		}

		if(maskDiff & (LEFT|RIGHT))
		{
			DPVS_ASSERT(fx0!=fx1);
			const float dx = (float)fx1-fx0;
			const float dy = (float)fy1-fy0;
			const float dydx = dy/dx;				//dx != 0.f;
			const float clipperSafeRegion = 0.5f;	//half a subpixel safe region - for avoiding incorrect rounding

			if(maskDiff & LEFT)
			{
				int	ly0=0,ly1=0;

				if(mask0 & LEFT)
				{
					ly0 = fy0;
					fy0+= Math::intFloor(dydx*(m_int16Scissor.x0 - fx0) + clipperSafeRegion);
					fy0 = fy0<m_int16Scissor.y0 ? m_int16Scissor.y0 : (fy0>m_int16Scissor.y1 ? m_int16Scissor.y1 : fy0);
					fx0 = m_int16Scissor.x0;
					ly1 = fy0;
				}

				if(mask1 & LEFT)
				{
					ly0 = fy1;
					fy1+= Math::intFloor(dydx*(m_int16Scissor.x0 - fx1) + clipperSafeRegion);
					fy1 = fy1<m_int16Scissor.y0 ? m_int16Scissor.y0 : (fy1>m_int16Scissor.y1 ? m_int16Scissor.y1 : fy1);
					fx1 = m_int16Scissor.x0;
					ly1 = fy1;
				}

				lefty0 = ly0;
				lefty1 = ly1;
			}
			if(maskDiff & RIGHT)
			{
				if(mask0 & RIGHT)
				{
					fy0+= Math::intFloor(dydx*(m_int16Scissor.x1 - fx0) + clipperSafeRegion);
					fy0 = fy0<m_int16Scissor.y0 ? m_int16Scissor.y0 : (fy0>m_int16Scissor.y1 ? m_int16Scissor.y1 : fy0);
					fx0 = m_int16Scissor.x1;
				}
				if(mask1 & RIGHT)
				{
					fy1+= Math::intFloor(dydx*(m_int16Scissor.x1 - fx1) + clipperSafeRegion);
					fy1 = fy1<m_int16Scissor.y0 ? m_int16Scissor.y0 : (fy1>m_int16Scissor.y1 ? m_int16Scissor.y1 : fy1);
					fx1 = m_int16Scissor.x1;
				}
			}
		}
	}

	//-----------------------------------------------------------
	// Arrange top -> down
	//-----------------------------------------------------------

	if(lefty0 > lefty1)
	{
		swap (lefty0,lefty1);		// with subpixels
	}

	if (fy0 > fy1)
	{
		swap (fx0,fx1);				// with subpixels
		swap (fy0,fy1);
	}

	//-----------------------------------------------------------
	// DEBUG assert
	//-----------------------------------------------------------

#ifdef DPVS_DEBUG
	{
		int x0 = fx0>>SHIFT;
		int x1 = fx1>>SHIFT;
		int y0 = fy0>>SHIFT;
		int y1 = fy1>>SHIFT;

		if(y0<y1)		// non-flat
		{
			const int yFrac	= (~fy0 & 0x0F);
			const int xStep	= Math::divFixed32_16(fx1-fx0, fy1-fy0);
			const int xEdge	= ((xStep * yFrac) >> 4) + (fx0 << (16-4));

			y1-= 1;									// inclusive y1
			x1 = (xEdge + (y1-y0) * xStep) >> 16;	// inclusive x1

			DPVS_ASSERT(x0>=m_dirtyRectangle.x0 && "This probably means that bounds (AA rectangle) is too small");	// inc
			DPVS_ASSERT(y0>=m_dirtyRectangle.y0 && "This probably means that bounds (AA rectangle) is too small");	// inc
			DPVS_ASSERT(x1<=m_dirtyRectangle.x1 && "This probably means that bounds (AA rectangle) is too small");	// inc
			DPVS_ASSERT(y1<=m_dirtyRectangle.y1 && "This probably means that bounds (AA rectangle) is too small");	// inc
		}
	}
#endif //DPVS_DEBUG

	//-----------------------------------------------------------
	// Append edges. clip "clamp edge", reject flat lines
	//-----------------------------------------------------------

	int ly0 = lefty0 >> SHIFT;
	int ly1 = lefty1 >> SHIFT;

	if(ly0<ly1)
	{
		if(ly0 < m_dirtyRectangle.y0)		// clip top to dirty area
			ly0 = m_dirtyRectangle.y0;

		if(ly1 > m_dirtyRectangle.y1)		// clip bottom to dirty area
			ly1 = m_dirtyRectangle.y1;

		DPVS_ASSERT(m_intScissor.x0>=m_dirtyRectangle.x0 && m_intScissor.x0<m_dirtyRectangle.x1);

		if(ly0<ly1)
			appendEdge(m_int16Scissor.x0,m_int16Scissor.x0, ly0<<SHIFT,ly1<<SHIFT, leftEdge);
	}

	const int y0  = fy0 >> SHIFT;
	const int y1  = fy1 >> SHIFT;

	if(y0<y1)
		appendEdge(fx0,fx1, fy0,fy1, leftEdge);
}


/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::Edge::Edge()
 *
 * Description:		Constructs an edge
 *
 *****************************************************************************/

OcclusionBuffer::Edge::Edge			(void) :
	m_fx0(0),
	m_fx1(0),
	m_fy0(0),
	m_fy1(0),
	m_xEdge(0),
	m_xStep(0),
	m_initialized(false),
	m_leftEdge(false),
	m_singleBucket(false)
{
	// nada
}

/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::Edge::setup()
 *
 * Description:		Performs setup for an edge
 *
 * Notes:			Returns immedietely if setup already performed
 *
 *****************************************************************************/

static DPVS_FORCE_INLINE int divFixed32_16_FAST (int a, int b)
{
	DPVS_ASSERT(b>0);				// divider is positive
	DPVS_ASSERT(b<= 16384*16);		// 2^14 (max reso) * 2^4 (4 subbits)
	DPVS_ASSERT(a>=-16384*16);		// 2^14 (max reso) * 2^4 (4 subbits) [negative side]
	DPVS_ASSERT(a<= 16384*16);		// 2^14 (max reso) * 2^4 (4 subbits)

	// Operation:		return (INT32)(((INT64)(a)*65536)/b);
	//
	// Numeric range:
	//					a = 2 * 2^14 * 2^4 = 2^19 = 2^18 + SIGN
	//					b = 2^14 * 2^4 = 2^18
	//					result = 16.16
	//					dividee = +-2^18 * 2^13 = +- 2^31
	//					divider =   2^18

	if(Math::abs(a)<2048*16)
		return (a<<16) / b;						// +-2^11 * 2^4 * 2^16 = +-2^31
	else
		return Math::divFixed32_16(a,b);
}

void OcclusionBuffer::Edge::setup	(void)
{
	if(m_initialized)
		return;

	m_initialized	= true;

	const int yFrac	= (~m_fy0 & 0x0F);
	m_xStep			= divFixed32_16_FAST(m_fx1-m_fx0, m_fy1-m_fy0);
	m_xEdge			= ((m_xStep * yFrac) >> 4) + (m_fx0 << (16-4));		// 4 subbits

#ifdef DPVS_DEBUG		// check against 64 div - PARANOID!!
	DPVS_ASSERT(Math::divFixed32_16(m_fx1-m_fx0, m_fy1-m_fy0) == m_xStep);
#endif

#ifdef DPVS_DEBUG		// check interpolated edge against its bounds - PARANOID!!
	{
		int xl,xr;

		if (m_fx1 > m_fx0)
		{
			xl = m_fx0>>4;
			xr = m_fx1>>4;
		} else
		{
			xl = m_fx1>>4;
			xr = m_fx0>>4;
		}

		int y0 = (m_fy0 >> 4);							// inclusive
		int y1 = (m_fy1 >> 4) - 1;					// inclusive
		int x0 = (m_xEdge >> 16);						// "interpolated" x0
		int x1 = (m_xEdge + m_xStep*(y1-y0)) >> 16;		// "interpolated" x1
		DPVS_ASSERT(x0>=xl && x0<=xr && "Edge violates its own bounds! (START)");
		DPVS_ASSERT(x1>=xl && x1<=xr && "Edge violates its own bounds! (END)");
	}
#endif
}

/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::assignEdgesToBuckets()
 *
 * Description:		Assigns all edges to buckets
 *
 * Parameters:		-
 *
 * Notes:			Reflection targets are handles here
 *
 *****************************************************************************/

void OcclusionBuffer::assignEdgesToBuckets	(void)
{
	const DynamicArray<Edge>&		edgeBuffer	= m_edges;
	const DynamicArray<BucketEdge>&	edgeBuffer2	= m_edges2;
	Bucket *						buckets		= &m_bucket[0];
	const int						edgeCount	= m_edgeCount;
	const int						bucketW		= m_bucketRectangle.x1;
	const int						dirtyW		= m_dirtyBucketRectangle.x1;
	const bool						noSkip		= m_mode != WRITE;
	const int						W			= (BUCKET_W << 16) - 1;
	int								debugCnt	= 0;

	m_edgeManager.clear();
	m_edgeManager.ensureSpaceFor(m_verticalBucketCrossings + m_edgeCount);	// NOTE: conservative upper bound!

	for(int i=0;i<edgeCount;i++)
	{
		const BucketEdge& be = edgeBuffer2[i];

		const int bx0 = be.bx0;
		const int bx1 = be.bx1;
		const int by0 = be.by0;
		const int by1 = be.by1;

		for(int by=by0;by<=by1;by++)
		for(int bx=bx0;bx<=bx1;bx++)
		{
			Bucket& bucket = buckets[by * bucketW + bx];

//			DPVS_ASSERT(bucket.getBucketX() < dirtyW);

#ifdef OCC_USE_REFLECTION_TARGETS
			//---------------------------------------------------
			// RT - Normal case
			//---------------------------------------------------

			if(noSkip || !bucket.isFull())
			{
				debugCnt++;
				int index = m_edgeManager.addEntry(i);				// entry instance for edge "i"
				m_edgeManager.link(index,bucket.getFirstEdge());	// build reversed linked list
				bucket.setFirstEdge(index);
			}

			//---------------------------------------------------
			// RT - Clip to bucket and reflect if intersects
			//---------------------------------------------------
			else
			{
				const int rti = bucket.getReflectionTarget();

				if(rti==-1 || buckets[rti].getBucketX() >= dirtyW)
					continue;

				Bucket& rt = buckets[rti];

				//---------------------------------------------------
				// Clip edge to bucket boundaries and reflect the
				// clipped part. This way the border structure is
				// not required and the solution fully runs with edges.
				//---------------------------------------------------

				const int ymin = by << BUCKET_YBITS;	// inclusive top
				const int ymax = ymin + BUCKET_H - 1;	// inclusive bottom
				const int xmin = bx << BUCKET_XBITS;	// inclusive left

				Edge& e = edgeBuffer[i];

				int y0 =  e.m_fy0 >> 4;					// inclusive y0
				int	y1 = (e.m_fy1 >> 4) - 1;				// inclusive y1

				DPVS_ASSERT(y1>=y0);

				if (!e.m_initialized)
					e.setup();

				int xEdge			= e.m_xEdge;
				const int xStep		= e.m_xStep;
				const bool leftEdge	= e.m_leftEdge;

				DPVS_ASSERT(y0<=ymax);
				DPVS_ASSERT(y1>=ymin);

				if(y0<ymin)
				{
					xEdge += (ymin-y0) * xStep;
					y0     = ymin;						// clip top
				}

				if(y1>ymax)
					y1     = ymax;						// clip bottom

				y0 -= ymin;								// y inside the bucket
				y1 -= ymin;								// y inside the bucket

				//-------------------------------------------------
				// update bounds
				//-------------------------------------------------

				xEdge -= xmin << 16;
				const int x0 = xEdge>>16;						// x0 inside the bucket
				const int x1 = (xEdge + xStep*(y1-y0))>>16;	// x1 inside the bucket

				int	xl,xr;
				if(x0<=x1)	xl=x0, xr=x1;
				else		xl=x1, xr=x0;

//				int	n = -1;		// DEBUG DEBUG
//				int	y0_old = y0;
//				int	y1_old = y1;

				if(xl>=BUCKET_W || xr<0)
				{
					y0 = 1;		// invalid setup
					y1 = 0;
				}
				else
				{
					//-------------------------------------------------
					//					Clipper
					//-------------------------------------------------
					// CASE 0: x0 is out from the left,  xStep > 0
					// CASE 1: x1 is out from the left,  xStep < 0
					// CASE 2: x0 is out from the right, xStep < 0
					// CASE 3: x1 is out from the right, xStep > 0
					//-------------------------------------------------

					int	ybase = y0;

					// LEFT
					if(x0<0)
					{
						DPVS_ASSERT(xStep>0);
						const int n = Math::intCeil(-xEdge / (float)xStep);		// >= (ceil)
						y0 = ybase+n;
#ifdef DPVS_DEBUG
						// DEBUG DEBUG assert optimality
						const int tmpx0 = (xEdge + n*xStep) >> 16;
						const int tmpx1 = (xEdge + (n-1)*xStep) >> 16;
						DPVS_ASSERT(n>=0);
						DPVS_ASSERT(tmpx0>=0 && tmpx1<0);
#endif
					}
					else if(x1<0)
					{
						DPVS_ASSERT(xStep<0);
						const int n = Math::intFloor(-xEdge / (float)xStep);		// <= (floor)
						y1 = ybase+n;
#ifdef DPVS_DEBUG
						// DEBUG DEBUG assert optimality
						const int tmpx0 = (xEdge + n*xStep) >> 16;
						const int tmpx1 = (xEdge + (n+1)*xStep) >> 16;
						DPVS_ASSERT(n>=0);
						DPVS_ASSERT(tmpx0>=0 && tmpx1<0);
#endif
					}

					// RIGHT
					if(x0>=BUCKET_W)
					{
						DPVS_ASSERT(xStep<0);
						const int n = Math::intCeil( (W-xEdge) / (float)xStep);	// >= (ceil)
						y0 = ybase+n;
#ifdef DPVS_DEBUG
						// DEBUG DEBUG assert optimality
						const int tmpx0 = (xEdge + n*xStep) >> 16;
						const int tmpx1 = (xEdge + (n-1)*xStep) >> 16;
						DPVS_ASSERT(n>=0);
						DPVS_ASSERT(tmpx0<BUCKET_W && tmpx1>=BUCKET_W);
#endif
					}
					else if(x1>=BUCKET_W)
					{
						DPVS_ASSERT(xStep>0);
						const int n = Math::intFloor( (W-xEdge) / (float)xStep);	// <= (floor)
						y1 = ybase+n;
#ifdef DPVS_DEBUG
						// DEBUG DEBUG assert optimality
						const int tmpx0 = (xEdge + n*xStep) >> 16;
						const int tmpx1 = (xEdge + (n+1)*xStep) >> 16;
						DPVS_ASSERT(n>=0);
						DPVS_ASSERT(tmpx0<BUCKET_W && tmpx1>=BUCKET_W);
#endif
					}
				}

				DPVS_ASSERT(y0>=0);
				DPVS_ASSERT(y1<BUCKET_H);

				//-------------------------------------------------
				// create a new edge for the reflected part
				//-------------------------------------------------

				if(y0<=y1)
				{
					//---------------------------------------------------------------
					// Set edge data (NOTE: cannot use appendEdge() here)
					//---------------------------------------------------------------

					Edge& e2 = m_edges.getElement(m_edgeCount);

					e2.m_fx0 = rt.getBucketX() << (BUCKET_XBITS+4);		// occlusion space, inclusive
					e2.m_fx1 = e2.m_fx0;
					e2.m_fy0 = ((by<<(BUCKET_YBITS)) + y0) << 4;			// occlusion space, inclusive
					e2.m_fy1 = ((by<<(BUCKET_YBITS)) + y1 + 1) << 4;		// occlusion space, exclusive
					e2.m_leftEdge	= leftEdge;
					e2.m_initialized = false;

					debugCnt++;
					const int index = m_edgeManager.addEntry(m_edgeCount++);	// entry instance for edge "m_edgeCount"
					m_edgeManager.link(index,rt.getFirstEdge());		// build reversed linked list
					rt.setFirstEdge(index);
				}
			}
#else	//OCC_USE_REFLECTION_TARGETS


			//---------------------------------------------------
			// No reflection targets - DEBUG code
			//---------------------------------------------------

			IF_USELESS_TO_RASTERIZE
				continue;

			debugCnt++;
			const int index = m_edgeManager.addEntry(i);				// entry instance for edge "m_edgeCount"
			m_edgeManager.link(index,bucket.getFirstEdge());		// build reversed linked list
			bucket.setFirstEdge(index);
#endif //OCC_USE_REFLECTION_TARGETS

		}
	}

	DPVS_ASSERT(debugCnt <= m_verticalBucketCrossings + m_edgeCount && "Insufficient allocation in m_edgeManager");
}

/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::rasterizeEdges()
 *
 * Description:		Resterizes edges crossing this bucket to the cache
 *
 * Parameters:		bucket	= the bucket of interest
 *					left	= integer to hold bounding box minimum for filler
 *					right	= integer to hold bounding box maximum for filler
 *
 * Notes:			All time is spent on ParallelMath::inc/dec comamnds
 *					Clipper doesn't pay off - slightly slower due to short edges
 *
 *****************************************************************************/

// optimized: using Guard instead of const DynamicArray&
// optimized: using local values left/right instead of aliasing references (copying proper values at the very end)

DPVS_FORCE_INLINE void OcclusionBuffer::rasterizeLeftEdge(int y0, int y1, int xEdge, int xStep)
{
	for(int y=y0;y<=y1;y++)
	{
		const int x = xEdge >> 16;		// x inside the bucket
		xEdge += xStep;
		if((unsigned int)(x)>=BUCKET_W)		// tests for x < 0 at the same time!
			continue;
//		DPVS_ASSERT(x>=dirtyX0 && x<dirtyX1);
		m_cache->inc(x,y);
	}
}

DPVS_FORCE_INLINE void OcclusionBuffer::rasterizeRightEdge(int y0, int y1, int xEdge, int xStep)
{
	for(int y=y0;y<=y1;y++)
	{
		const int x = xEdge >> 16;		// x inside the bucket
		xEdge += xStep;
		if((unsigned int)(x)>=BUCKET_W)		// tests for x < 0 at the same time!
			continue;
//		DPVS_ASSERT(x>=dirtyX0 && x<dirtyX1);
		m_cache->dec(x,y);
	}
}

// now there are approx. 3 times more calls than there would be if we modified the database
// silhouette test somewhat...

DPVS_FORCE_INLINE void OcclusionBuffer::rasterizeTestEdge(int y0, int y1, int xEdge, int xStep)
{
	DPVS_ASSERT(y0<=y1);	// WASS?
	Cache* c = m_cache;
	do
	{
		const int x = xEdge >> 16;			// x inside the bucket
		if((unsigned int)(x)<BUCKET_W)		// tests for x < 0 at the same time!
			c->eor(x,y0);
		xEdge+=xStep;
		y0++;
	} while (y0 <= y1);
}

bool OcclusionBuffer::rasterizeEdges	(Bucket& bucket,int& leftRef,int& rightRef)
{
	int listIndex = bucket.getFirstEdge();
	DPVS_ASSERT(listIndex>=-1);

	if(listIndex==-1)
		return false;

	Guard<Edge>			edges(m_edges);								//	const DynamicArray<Edge>&	edges	= m_edges;
	const EdgeManager&	em			= m_edgeManager;
	int					left		= 0x7FFFFFFF;
	int					right		= 0x80000000;
	Mode				mode		= m_mode;
	const int			ymin		= bucket.getBucketY() << BUCKET_YBITS;
	const int			ymax		= ymin + BUCKET_H - 1;
	const int			xmin		= bucket.getBucketX() << BUCKET_XBITS;
/*
#ifdef DPVS_DEBUG
	const int			dirtyX0		= m_dirtyRectangle.x0 - xmin;	// inside the bucket
	const int			dirtyX1		= m_dirtyRectangle.x1 - xmin;	// inside the bucket
#endif
*/
	do
	{
		//----------------------------------------------------------------
		// Fetch next edge and make sure it has been set up
		//----------------------------------------------------------------

		const int index = em.m_entries[listIndex].m_edgeNum;

		Edge& e = edges[index];

		if (!e.m_initialized)							// edge not initialized yet?
			e.setup();

		m_lastWriteCost += ImpObject::WRITE_COST_PER_BUCKET_EDGE;

		//----------------------------------------------------------------
		// Clip the edge into the bucket
		//----------------------------------------------------------------

		int			xEdge	= e.m_xEdge;
		const int	xStep	= e.m_xStep;
		int			y0		= e.m_fy0 >> 4;				// inclusive
		int			y1		= (e.m_fy1 >> 4) - 1;		// inclusive

		DPVS_ASSERT(y0<=ymax);
		DPVS_ASSERT(y1>=ymin);

		if(y0 < ymin)									// clip top?
		{
			xEdge += (ymin-y0) * xStep;
			y0   = ymin;
		}

		if(y1 > ymax)									// clip bottom?
			y1 = ymax;

		y0 -= ymin;										// y inside the bucket
		y1 -= ymin;										// y inside the bucket

		DPVS_ASSERT(y0<=y1);

		//-------------------------------------------------
		// Update bounds
		//-------------------------------------------------

		xEdge -= xmin << 16;
		const int x0 = (xEdge>>16);						// x inside the bucket
		const int x1 = ((xEdge + xStep*(y1-y0))>>16);	// x inside the bucket

		int	xl,xr;

		if(x0<=x1)
		{
			xl=x0;
			xr=x1;
		} else
		{
			xl=x1;
			xr=x0;
		}

		if(xl>=BUCKET_W || xr<0)						// doesn't intersect the bucket -> skip
		{
			listIndex = em.m_entries[listIndex].m_next;	// traverse linked list
			continue;
		}

		if(xl < left)									// update bound values...
			left  = xl;
		if(xr > right)
			right = xr;

		//----------------------------------------------------------------
		// Rasterizer. The three cases inside each group are: write left
		// edge, write right edge, test (either edge)... The non-clipping
		// variants were removed because they were not useful anymore...
		//----------------------------------------------------------------

		if(mode==WRITE)
		{
			m_lastWriteCost += (y1-y0) * ImpObject::WRITE_COST_PER_SCANLINE;	

			if(e.m_leftEdge)
				rasterizeLeftEdge(y0,y1,xEdge,xStep);
			else
				rasterizeRightEdge(y0,y1,xEdge,xStep);
		} else
			rasterizeTestEdge(y0,y1,xEdge,xStep);

		//----------------------------------------------------------------
		// Advance in linked list...
		//----------------------------------------------------------------

		listIndex = em.m_entries[listIndex].m_next;	// advance in linked list

	}while(listIndex != -1);

	bucket.setFirstEdge(-1);						// cleanup (SAFETY)

	//---------------------------------------------------------------
	// No edge touched the cache -> empty.
	//---------------------------------------------------------------

	if(left > right)
		return false;

	//---------------------------------------------------------------
	// Clamp bounding box to bucket borders (there's no clipper above)
	// and copy the left/right values to leftRef and rightRef
	//---------------------------------------------------------------

	leftRef		= left < 0 ? 0 : left;
	rightRef	= right >= BUCKET_W ? BUCKET_W-1 : right;

	return true;									// cache was modified...
}

/*

TEST CODE

typedef unsigned	int UINT32;
typedef signed		int INT32;

//using namespace DPVS;

int divFixed32_16 (int a, int b)
{

	int correct = (INT32)((__int64(a)*65536)/__int64(b));

	int retval;
	__asm
	{
		mov		eax,[a]
		mov		ebx,[b]
		mov		edx,eax
		sar		edx,31
		shld	edx,eax,16
		shl		eax,16
		idiv	ebx
		mov		[retval],eax
	}

	if (correct != retval)
	{
		printf ("correct = %d, retval = %d\n",correct,retval);
		exit(1);
	}

	return correct;
}

template <class T> inline void swap (T& a, T& b)
{
	T t = a;
	a = b;
	b = t;
}


void main (void)
{
	const int ITER = 1000000;
	int cnt = 0;
	for (int i = 0; i < ITER; i++)
	{
		int fx0    = (rand()%(640*16));
		int fx1    = (rand()%(640*16));//(rand()%(10*16)-5*16) + fx0;

		int fy0    = (rand()%(480*16));
		int fy1    = (rand()%(480*16));

		if (fy1 < fy0)
		{
			swap(fy0,fy1);
			swap(fx0,fx1);
		}

		if ((fy0>>4)==(fy1>>4))
			continue;

		int xStep	= divFixed32_16(fx1-fx0,fy1-fy0);

		int xl,xr;
		if(fx1>fx0)	xl=fx0>>4, xr=fx1>>4;
		else		xl=fx1>>4, xr=fx0>>4;


		const int yFrac	= 15-(fy0&15);//(~fy0 & 0x0F);
		int xEdge		= ((xStep * yFrac) >> 4) + (fx0 << (16-4));		// 4 subbits

		int y0 =  fy0 >> 4;			// inclusive
		int y1 = (fy1 >> 4) - 1;	// inclusive
		int x0 = xEdge >> 16;						// "interpolated" x0
		int x1 = (xEdge + xStep*(y1-y0)) >> 16;		// "interpolated" x1


		if (x0 < xl || x0 > xr ||
			x1 < xl || x1 > xr)
		{
			cnt++;
			printf ("fx0 = %d\n",fx0);
			printf ("fx1 = %d\n",fx1);
//			printf ("delta = %d\n",fx1-fx0);
			printf ("--------------------------\n");
			if (cnt == 10)
				break;
		}
	}

	printf ("errors = %.3f%%\n",(cnt*100.0f)/ITER);
}

*/
//------------------------------------------------------------------------
