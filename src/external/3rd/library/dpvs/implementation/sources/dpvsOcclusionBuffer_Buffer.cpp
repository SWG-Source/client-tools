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
 * Description: 	Buffer manipulation/visualization code
 *
 * $Archive: /dpvs/implementation/sources/dpvsOcclusionBuffer_Buffer.cpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 4/22/02 6:09p $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#include "dpvsOcclusionBuffer.hpp"
#include "dpvsSurface.hpp"
#include "dpvsImpCamera.hpp"
#include "dpvsHZBuffer.hpp"
#include "dpvsSpaceManager.hpp"

using namespace DPVS;


/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::getClosingDepth()
 *
 * Description:		Returns raster-space depth at which the occlusion buffer
 *					is completely full
 *
 * Notes:			If occlusion buffer itself isn't fully covered,
 *					the function returns 1.0 (far-clipping plane)
 *
 *****************************************************************************/

float OcclusionBuffer::getClosingDepth (void) const
{
	if (!isFull())
		return 1.0f;

	IHZBuffer::DepthValue depth = m_HZBuffer->getFarDepth();
	const float f = *(float*)(&depth);

	DPVS_ASSERT(f<=1.0f);

	return f;								// convert back into floating point
}

/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::sampleMinimum()
 *
 * Description:		Area samples the minimum value inside the given rectangle
 *
 * Parameters:		buf		= pointer to buffer
 *					rect	= rectangle area (inclusive)
 *					w		= buffer pitch
 *
 * Returns:			Minimum value inside the rectangle
 *
 *****************************************************************************/

DPVS_FORCE_INLINE unsigned char OcclusionBuffer::sampleMinimum	(const unsigned char* buf, const IntRectangle &rect, int w)
{
	DPVS_ASSERT(buf);

	unsigned char	value = 255;

	for(int y=rect.y0;y<=rect.y1;y++)
	for(int x=rect.x0;x<=rect.x1;x++)
	{
		int	offset = y*w + x;
		if(buf[offset]<value)
		{
			value = buf[offset];
			if(value==0)
				return value;
		}
	}

	return value;
}

/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::sampleMaximum()
 *
 * Description:		Area samples the maximum inside the given rectangle
 *
 * Parameters:
 *
 * Returns:			maximum value inside the rectagle
 *
 * Notes:			rect in inclusive
 *
 *****************************************************************************/

DPVS_FORCE_INLINE float OcclusionBuffer::sampleMaximum	(const float* buf, const IntRectangle &rect, int w) 
{
	float value = 0.f;

	for(int y=rect.y0;y<=rect.y1;y++)
	for(int x=rect.x0;x<=rect.x1;x++)
	{
		int	offset = y*w + x;
		if(buf[offset]>value)
			value = buf[offset];
	}

	return value;
}

/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::setStaticCoverageBuffer()
 *
 * Description:		
 *
 * Parameters:
 *
 *****************************************************************************/

void OcclusionBuffer::setStaticCoverageBuffer	(const unsigned char* buf,int w,int h,int pitch)
{
	DPVS_ASSERT( w >= 0 && h >= 0);

	//--------------------------------------------------------------------
	// If 'buf' is null, we delete existing coverage buffer and return.
	//--------------------------------------------------------------------

	if(!buf)
	{
		DELETE_ARRAY(m_staticCoverageBuffer);
		DELETE_ARRAY(m_staticFullBlocks);
		DELETE_ARRAY(m_staticPartialBlocks);
		m_staticCoverageBuffer	= null;
		m_staticFullBlocks		= null;
		m_staticPartialBlocks	= null;
		return;
	}

	//--------------------------------------------------------------------
	// Allocate new stastic coverage buffer
	//--------------------------------------------------------------------

	if(!m_staticCoverageBuffer)
	{
		m_staticCoverageBuffer	= NEW_ARRAY<QWORD>(bitsToQWords(m_memoryRectangle.area()));			// resolution * 1bpp
		m_staticFullBlocks		= NEW_ARRAY<unsigned char>(bitsToBytes(m_loresRectangle.area()));	// 1bpp
		m_staticPartialBlocks	= NEW_ARRAY<unsigned char>(bitsToBytes(m_loresRectangle.area()));	// 1bpp
	}

	//--------------------------------------------------------------------
	// Clear and set padding
	//--------------------------------------------------------------------

	clearCoverageBuffer	(m_staticCoverageBuffer);
	padCoverageBuffer	(m_staticCoverageBuffer);

	fillByte (m_staticFullBlocks,    0, bitsToBytes(m_loresRectangle.area()));
	fillByte (m_staticPartialBlocks, 0, bitsToBytes(m_loresRectangle.area()));

	//--------------------------------------------------------------------
	// Scale minimum sampled copy from input buffer
	//--------------------------------------------------------------------

	IntRectangle	rect;
	int	y = 0;

	const int dx = 65536*w / m_clipRectangle.width();	//x stepper in input buf
	const int dy = 65536*h / m_clipRectangle.height();	//y stepper in input buf

	int cHeight = m_clipRectangle.height();
	int cWidth  = m_clipRectangle.width();

	for(int j=0;j<cHeight;j++,y+=dy)
	{
		rect.y0 = (y)		>> 16;
		rect.y1 = (y+dy-1)	>> 16;
		int	x	= 0;

		for(int i=0;i<cWidth;i++,x+=dx)
		{
			rect.x0 = (x)		>> 16;
			rect.x1 = (x+dx-1)	>> 16;

			if(sampleMinimum(buf,rect,pitch))
			{
				unsigned char* ptr = m_staticCoverageBuffer[(j>>3)*m_blockMemoryRectangle.x1 + (i>>3)].getBytePtr(i&7);
				
				DPVS_ASSERT((UPTR)(ptr)>=(UPTR)m_staticCoverageBuffer);
				DPVS_ASSERT((UPTR)(ptr)<(UPTR)(m_staticCoverageBuffer + bitsToQWords(m_memoryRectangle.area())));

				*ptr |= 1<<(j&7);
			}
		}
	}

	//--------------------------------------------------------------------
	// set full&partial block buffers accordingly
	//--------------------------------------------------------------------

	for(y=0;y<m_blockClipRectangle.y1;y++)
	for(int x=0;x<m_blockClipRectangle.x1;x++)
	{
		const QWORD &block = m_staticCoverageBuffer[y*m_blockMemoryRectangle.x1 + x];

		DPVS_ASSERT((UPTR)(&block)>=(UPTR)m_staticCoverageBuffer);
		DPVS_ASSERT((UPTR)(&block)<(UPTR)(m_staticCoverageBuffer + bitsToQWords(m_memoryRectangle.area())));
		
		const int o = y*m_loresRectangle.x1+x;

		DPVS_ASSERT(o>=0 && (o>>3)<(bitsToBytes(m_loresRectangle.area())));

		if(block.full())
			m_staticFullBlocks[o>>3] |= 1<<(o&7);
		else if(!block.empty())
			m_staticPartialBlocks[o>>3] |= 1<<(o&7);
	}
}

/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::setStaticZBuffer()
 *
 * Description:		
 *
 * Parameters:
 *
 *****************************************************************************/

void OcclusionBuffer::setStaticZBuffer	(const float* buf,int w,int h, int pitch,float farValue)
{
	if(!buf)
	{
		DELETE_ARRAY(m_staticZBuffer);
		m_staticZBuffer = null;
		return;
	}

	if(!m_staticZBuffer)
		m_staticZBuffer = NEW_ARRAY<UINT32>(m_loresRectangle.area());

	fillByte (m_staticZBuffer,0,m_loresRectangle.area()*sizeof(UINT32));

	//-----------------------------------------------------
	//scaled maximum sampled copy from input buffer
	//-----------------------------------------------------

	IntRectangle rect;
	int	y = 0;
	const int dx = 65536*w / m_blockClipRectangle.width();		//x stepper in input buf
	const int dy = 65536*h / m_blockClipRectangle.height();		//y stepper in input buf

	for(int j=0;j<m_blockClipRectangle.height();j++,y+=dy)
	{
		rect.y0 = (y)		>> 16;
		rect.y1 = (y+dy-1)	>> 16;
		int	x	= 0;

		for(int i=0;i<m_blockClipRectangle.width();i++,x+=dx)
		{
			rect.x0 = (x)		>> 16;
			rect.x1 = (x+dx-1)	>> 16;

			float value = sampleMaximum(buf,rect,pitch/sizeof(float));
			DPVS_ASSERT(value>=0.f);
			if(value == farValue)
				value = 0.f;								// z-buffer -> DEB conversion
			m_staticZBuffer[j*m_loresRectangle.x1 + i] = Math::bitPattern(value);
		}
	}
}



/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::initializeBuffers()
 *
 * Description:		
 *
 * Notes:			clears full buffer and pads vertically
 *
 *****************************************************************************/

void OcclusionBuffer::initializeBuffers		()
{
	//-----------------------------------------------------
	// clear buffers fully
	//-----------------------------------------------------

	fillDWord((UINT32*)m_CBuffer, 0, bitsToDWords(m_memoryRectangle.area()));				//1bpp
	fillByte (m_blockBufferF,0,bitsToBytes(m_loresRectangle.area()));	//1bpp
	fillByte (m_blockBufferC,0,bitsToBytes(m_loresRectangle.area()));	//1bpp 	[CONTRIBUTION]
	fillByte (m_blockBufferP,0,bitsToBytes(m_loresRectangle.area()));	//1bpp
	fillDWord(m_ZBuffer,0,m_loresRectangle.area());											//32bpp

	//-----------------------------------------------------
	//calculate address generation helpers for coverage buffer
	//-----------------------------------------------------

	m_coverageBufferBlockQWPitch	= 1;
	m_coverageBufferBlockRowQWPitch = m_blockMemoryRectangle.x1;
	m_coverageBufferBucketQWPitch	= BUCKET_W/BLOCK_W*m_coverageBufferBlockQWPitch;
	m_coverageBufferBucketRowQWPitch= BUCKET_H/BLOCK_H*m_coverageBufferBlockRowQWPitch;

	//-----------------------------------------------------
	//validate bucket data
	//-----------------------------------------------------

	const bool hpad = m_memoryRectangle.width()  != m_clipRectangle.width();
	const bool vpad = m_memoryRectangle.height() != m_clipRectangle.height();

	for(int y=0;y<m_bucketRectangle.height();y++)
	for(int x=0;x<m_bucketRectangle.width();x++)
	{
		DPVS_ASSERT(x<256);		// unsigned char capacity
		DPVS_ASSERT(y<256);		// unsigned char capacity

		const bool right = (x==m_bucketRectangle.width()-1);
		const bool bottom= (y==m_bucketRectangle.height()-1);

		const int n = y*m_bucketRectangle.width()+x;
		Bucket &bucket = m_bucket[n];

		bucket.setPendingClear	(false);
		bucket.setPadded		((right && hpad) || (bottom && vpad), (right && hpad) && (bottom && vpad));
		bucket.setNumber		(x,y);
		bucket.setCoverageBufferOffset((y*m_coverageBufferBucketRowQWPitch) + (x*m_coverageBufferBucketQWPitch));
		bucket.clear();
		bucket.clearState();
	}

	m_totalBlocksFilled	 = 0;		//statistics
	m_totalBucketsFilled = 0;		//statistics
}

/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::clearCoverageBuffer()
 *
 * Description:		
 *
 * Parameters:
 *
 *****************************************************************************/

void OcclusionBuffer::clearCoverageBuffer	(QWORD *cbuf)
{
	int	i;

	const int area = m_bucketRectangle.area(); 
	for(i=0;i < area;i++)
	{
		Bucket &bucket = m_bucket[i];
		bucket.clearState();

		if(bucket.isPadded())	
		{
			clearBucketArea(bucket,cbuf);
			DPVS_ASSERT(bucket.hasPendingClear() == false);
		}
		else
			bucket.setPendingClear(true);
	}
}

/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::clear()
 *
 * Description:		
 *
 * Parameters:
 *
 *****************************************************************************/

void OcclusionBuffer::clear	()
{
	//DEBUG DEBUG
//	std::cout << "-----------------------------------------------\n";

//	setScissor(null);		//now in Write Queue
#if defined (DPVS_OCCLUSIONBUFFER_INDEXBUFFER)	
	m_indexBufferIndex = 0;
#endif
	//-----------------------------------------------------
	// coverage buffer
	//-----------------------------------------------------

	if(m_staticCoverageBuffer)
	{
		int i;		
		DPVS_ASSERT (m_staticFullBlocks && m_staticPartialBlocks);
		DPVS_ASSERT (m_CBuffer && m_blockBufferF && m_blockBufferC && m_blockBufferP);

		//-----------------------------------------------------
		// even in initial state the coverage buffer is not empty
		//-----------------------------------------------------

		m_empty = false;

		//-----------------------------------------------------
		// copy static coveragebuffer,fullblocks,partialblocks and validate bucket states
		//-----------------------------------------------------

		memcpy(m_CBuffer,		m_staticCoverageBuffer,	bitsToQWords(m_memoryRectangle.area()) * sizeof(QWORD));
		memcpy(m_blockBufferF,	m_staticFullBlocks,		bitsToBytes(m_loresRectangle.area()));
		memcpy(m_blockBufferC,	m_staticFullBlocks,		bitsToBytes(m_loresRectangle.area()));	// [CONTRIBUTION]
		memcpy(m_blockBufferP,	m_staticPartialBlocks,	bitsToBytes(m_loresRectangle.area()));

		//-----------------------------------------------------
		// clear bucket data
		//-----------------------------------------------------

		const int bucketCount = m_bucketRectangle.area();
		for(i=0;i<bucketCount;i++)
		{
			m_bucket[i].clear();
			m_bucket[i].clearState();
			m_bucket[i].setPendingClear(false);
		}

		//-----------------------------------------------------
		// set fullBlocks data to valid state
		//-----------------------------------------------------

		for(int y=0;y<m_blockClipRectangle.y1;y++)
		for(int x=0;x<m_blockClipRectangle.x1;x++)
		{
			const int o = y*m_loresRectangle.x1+x;
			if((m_staticFullBlocks[o>>3]>>(o&7)) & 1)
			{
				int bucketx	= (unsigned int)(x) / (BUCKET_W/BLOCK_W);
				int buckety	= (unsigned int)(y) / (BUCKET_H/BLOCK_H);
				int offset  = buckety*m_bucketRectangle.x1+bucketx;
				m_bucket[offset].addFullBlockCount(1);
			}
		}

		//-----------------------------------------------------
		// validate reflection targets
		//-----------------------------------------------------

		for(int by=0;by<m_bucketRectangle.y1;by++)
		for(int bx=0;bx<m_bucketRectangle.x1;bx++)
		{
			const int	b = by*m_bucketRectangle.x1;
			const int offset = b + bx;

			if(!m_bucket[offset].isFull())					// doesn't need reflection target
				continue;

			m_bucket[offset].clearReflectionTarget();		// doesn't have

			for(i=bx+1;i<m_bucketRectangle.x1;i++)			// search to the right
			{
				if(m_bucket[b+i].isFull())
					continue;

				m_bucket[offset].setReflectionTarget(b+i);
				break;
			}

		}

	}
	else
	{
		//-----------------------------------------------------
		// in initial state the coverage buffer is empty
		//-----------------------------------------------------

		m_empty = true;

		//-----------------------------------------------------
		// clear & pad buffers
		//-----------------------------------------------------

		clearCoverageBuffer	(m_CBuffer);
		padCoverageBuffer	(m_CBuffer);

		fillByte (m_blockBufferF, 0, bitsToBytes(m_loresRectangle.width()*m_blockClipRectangle.height()));	//1bpp
		fillByte (m_blockBufferC, 0, bitsToBytes(m_loresRectangle.width()*m_blockClipRectangle.height()));	//1bpp [CONTRIBUTION]
		fillByte (m_blockBufferP, 0, bitsToBytes(m_loresRectangle.width()*m_blockClipRectangle.height()));	//1bpp
	}

	//-----------------------------------------------------
	// zbuffer
	//-----------------------------------------------------

	if(m_staticZBuffer)	
	{
		//copy static data, mark HZBuffer as modified, disable m_zeBufferDirty(for safety)
		memcpy(m_ZBuffer, m_staticZBuffer, m_loresRectangle.area()*sizeof(UINT32));
		m_HZBuffer->setRegionModified(m_blockClipRectangle.x0,m_blockClipRectangle.y0, m_blockClipRectangle.x1,m_blockClipRectangle.y1);
	}
	else
	{
		clearZBuffer();
	}

	//-----------------------------------------------------
	// initial scissoring
	//-----------------------------------------------------

	if(m_scissorCamera && m_scissorCamera->isScissorActive())
	{
		//-----------------------------------------------------------
		// create silhouette of the scissor rectangle
		//-----------------------------------------------------------

		const float sx = m_ooSubSampling.x;
		const float sy = m_ooSubSampling.y;

		Vector2	locations[] = { Vector2(sx*m_clipRectangle.x0,sy*m_clipRectangle.y0),Vector2(sx*m_clipRectangle.x0,sy*m_clipRectangle.y1),
								Vector2(sx*m_intScissor.x0,   sy*m_intScissor.y0),   Vector2(sx*m_intScissor.x1,   sy*m_intScissor.y0),
								Vector2(sx*m_intScissor.x0,   sy*m_intScissor.y1),   Vector2(sx*m_intScissor.x1,   sy*m_intScissor.y1)};

		m_scissorSilhouette->clear();
		m_scissorSilhouette->setVertices(locations,6);
		m_scissorSilhouette->addEdge(1, 0,1);		//right side is outside the screen (not needed)
		m_scissorSilhouette->addEdge(0, 2,4);
		m_scissorSilhouette->addEdge(1, 3,5);

		//-----------------------------------------------------------
		// Render "occluder"
		//-----------------------------------------------------------

		FloatRectangle fr;
		fr.x0 = m_clipRectangle.x0 * m_ooSubSampling.x;
		fr.x1 = m_clipRectangle.x1 * m_ooSubSampling.x;
		fr.y0 = m_clipRectangle.y0 * m_ooSubSampling.y;
		fr.y1 = m_clipRectangle.y1 * m_ooSubSampling.y;
		fr.setSpace(SPACE_RASTER);

		this->setScissor		(fr);						// SPACE_RASTER
		this->setDirtyRectangle	(m_clipRectangle);			// SPACE_OCCLUSION
		this->setZConstant		(ZGradient::WRITE, 0.f);
		this->write				(*m_scissorSilhouette,m_dummyContributed,m_dummyContributed);
		this->setScissor		(null);
	}

	m_totalBlocksFilled	 = 0;
	m_totalBucketsFilled = 0;
}

/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::padCoverageBuffer()
 *
 * Description:		
 *
 * Parameters:
 *
 * Notes:			Handles endian issue by reversing byte order of written QWORDs
 *
 *****************************************************************************/

void OcclusionBuffer::padCoverageBuffer	(QWORD *cbuf)
{
	//-----------------------------------------------------
	// set horizontal padding entries to coverage buffer
	//-----------------------------------------------------

	if(m_memoryRectangle.x1 - m_clipRectangle.x1)
	{
		//-----------------------------------------------------
		// write partial BLOCK to the right
		//-----------------------------------------------------

		int right  = m_clipRectangle.x1;		//WAS -1

		UINT32	xblock	= right>>3;
		UINT32	xshift	= right&7;
		UINT32	mask0	= xshift<4 ? 0xffffffff << 8*(xshift-4) : 0;
		UINT32	mask1	= xshift<4 ? 0xffffffff : 0xffffffff << 8*(xshift&3);

		DPVS_SELECT_ENDIAN(DPVS_NULL_STATEMENT,changeEndian(mask0));							// flip bytes
		DPVS_SELECT_ENDIAN(DPVS_NULL_STATEMENT,changeEndian(mask1));							// flip bytes
		QWORD mask(mask1,mask0);

		if(!mask.full())
		{
			for(int i=0;i<m_blockMemoryRectangle.y1;i++)
			{
				cbuf[m_blockMemoryRectangle.x1*i + xblock] = mask;
			}
			xblock++;
		}

		for(int j=xblock;j<m_blockMemoryRectangle.x1;j++)
		for(int i=0;i<m_blockMemoryRectangle.y1;i++)
		{
			const int offset = ((unsigned int)(i)/(BUCKET_H/BLOCK_H))*m_bucketRectangle.x1 + ((unsigned int)(j)/(BUCKET_W/BLOCK_W));
			m_bucket[offset].addFullBlockCount(1);
		}
	}

	//-----------------------------------------------------
	// set vertical padding entries to coverage buffer
	//-----------------------------------------------------

	bool cornerBucketHandled = false;
	int	 cornerBucketIndex = -1;
	int	 cornerBucketColumns = -1;

	if(m_memoryRectangle.y1 - m_clipRectangle.y1)
	{
		//-----------------------------------------------------
		// write partial BLOCK on the bottom
		//-----------------------------------------------------

		int bottom = m_clipRectangle.y1;		//WAS -1

		UINT32	yblock	= bottom>>3;
		UINT32	yshift	= bottom&7;
		UINT32	bm		= (0xff << yshift) & 0xff;
		UINT32	dm		= (bm<<24) | (bm<<16) | (bm<<8) | bm;
		QWORD	mask(dm,dm);

		//NOTE: All 8 bytes are the same -> no little/big endian issues here!

		if(!mask.full())
		{
			for(int j=0;j<m_blockMemoryRectangle.x1;j++)
			{
				cbuf[(m_memoryRectangle.x1>>3)*yblock + j] |= mask;
			}
			yblock++;
		}

		//-----------------------------------------------------
		// write full Blocks on the bottom
		//-----------------------------------------------------

		for(int i=yblock;i<m_blockMemoryRectangle.y1;i++)
		for(int j=0;j<m_blockMemoryRectangle.x1;j++)
		{
			int bnum = ((unsigned int)(i)/(BUCKET_H/BLOCK_H))*m_bucketRectangle.x1 + ((unsigned int)(j)/(BUCKET_W/BLOCK_W));

			//if padded both horizontally and vertically, double work gets done for the corner tile!
			if(!cornerBucketHandled && m_bucket[bnum].isPaddedTwice())
			{
				cornerBucketHandled = true;
				cornerBucketIndex	= bnum;
				cornerBucketColumns = m_bucket[bnum].getFullBlockCount() / (BUCKET_H/BLOCK_H);
				m_bucket[bnum].setFullBlockCount(0);
			}

			m_bucket[bnum].addFullBlockCount(1);
		}
	}

	if(cornerBucketHandled)
	{
		Bucket& bucket = m_bucket[cornerBucketIndex];
		const int cornerBucketRows = (unsigned int)(bucket.getFullBlockCount()) / (BUCKET_W/BLOCK_W);
		const int difference = cornerBucketColumns * (BUCKET_H/BLOCK_H - cornerBucketRows);
		bucket.addFullBlockCount(difference);	// restore original count
	}
}

/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::createBlockMasks()
 *
 * Description:		Creates masks for quick overlap testing of coverage buffer
 *
 * Notes:			Handles endian issue with QWORD getBytePtr()
 *
 *****************************************************************************/

void OcclusionBuffer::createBlockMasks()
{
	fillByte (m_blockMasks0,0,8*8*sizeof(QWORD));
	fillByte (m_blockMasks1,0,8*8*sizeof(QWORD));

	for(int y=0;y<8;y++)
	for(int x=0;x<8;x++)
	{
		int	x2,y2;
		QWORD&	mask0 = m_blockMasks0[y*8+x];
		QWORD&	mask1 = m_blockMasks1[y*8+x];

		//-----------------------------------------------------
		//fill towards the bottom right corner [(x,y) inclusive]
		//-----------------------------------------------------

		for(y2=y;y2<8;y2++)
		for(x2=x;x2<8;x2++)
			*(mask0.getBytePtr(x2&7)) |= 1<<(y2&7);

		//-----------------------------------------------------
		//fill towards the top left corner [(x,y) exclusive]
		//NOTE: stores values for [1,8] to slots [0,7]!!
		//-----------------------------------------------------

		for(y2=0;y2<=y;y2++)
		for(x2=0;x2<=x;x2++)
			*(mask1.getBytePtr(x2&7)) |= 1<<(y2&7);
	}
}


/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::visualizeCBuffer()
 *
 * Description:		
 *
 * Returns:			pointer to surface (caller is responsible for deleting it)
 *
 * Notes:			DEBUG visualization function - draws image of coverage buffer
 *					Handles endian issue with QWORD getByte()
 *
 *****************************************************************************/

Surface* OcclusionBuffer::visualizeCBuffer (void)
{
	IntRectangle &rect = m_clipRectangle;
	Surface* surface = NEW<Surface>();
	surface->resize( rect.width(),rect.height() );

	int	x,y;
	int xoffset = 0;
	int yoffset = 0;
//	int xoffset = m_memoryRectangle.x1 - m_clipRectangle.x1;	//DEBUG DEBUG (show padding)
//	int yoffset = m_memoryRectangle.y1 - m_clipRectangle.y1;	//DEBUG DEBUG (show padding)

	for(y=yoffset;y<rect.height()+yoffset;y++)
	for(x=xoffset;x<rect.width() +xoffset;x++)
	{
		const unsigned char line8 = m_CBuffer[(y>>3)*m_blockMemoryRectangle.x1 + (x>>3)].getByte(x&7);
		surface->setPixel(x-xoffset,y-yoffset,(line8>>((y)&7))&1 ? (unsigned char)(0xff) : (unsigned char)(0x00));
	}

/*	// DEBUG DEBUG visualize triage blockMasks. Don't delete this code
	for(y=0;y<8;y++)
	for(x=0;x<8;x++)
	{
		QWORD &block = m_CBuffer[y*m_blockMemoryRectangle.x1 + x];
		block = m_blockMasks0[y*8+x];
	}

	for(y=0;y<8;y++)
	for(x=0;x<8;x++)
	{
		QWORD &block = m_CBuffer[y*m_blockMemoryRectangle.x1 + x+8];
		block = m_blockMasks1[y*8+x];
	}
*/
	return surface;
}


/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::visualizeSBuffer()
 *
 * Description:		
 *
 * Returns:			pointer to surface (caller is responsible for deleting it)
 *
 * Notes:			DEBUG visualization function - draws image of coverage buffer
 *					Handles endian issue with QWORD getByte()
 *
 *****************************************************************************/

Surface* OcclusionBuffer::visualizeSBuffer	(void)
{
	IntRectangle &rect = m_clipRectangle;
	Surface *surface = NEW<Surface>();
	surface->resize( rect.width(),rect.height() );

	int	x,y;
	for(y=0;y<rect.height();y++)
	for(x=0;x<rect.width();x++)
	{
		const int	buckety = (unsigned int)(y)/BUCKET_H;
		const int	bucketx = (unsigned int)(x)/BUCKET_W;
		const int boffset = (buckety*m_bucketRectangle.x1 + bucketx) * BUCKET_W;	// each bucket is 32 consecutive QWORDS
		const int	x2 = x & (BUCKET_W-1);
		const int	y2 = y & (BUCKET_H-1);
		const unsigned char byte	= m_stencilBuffer[boffset + x2].getByte(y2>>3);
		surface->setPixel(x,y,(byte>>(y2&7))&1 ? (unsigned char)(0xff) : (unsigned char)(0x00));
	}

	return surface;
}

/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::visualizeZBuffer()
 *
 * Description:		
 *
 * Returns:			pointer to surface (caller is responsible for deleting it)
 *
 * Notes:			DEBUG visualization function - draws image of depth estimation buffer
 *
 *****************************************************************************/

Surface* OcclusionBuffer::visualizeZBuffer (void)
{
	const int	w = m_blockClipRectangle.x1;
	const int	h = m_blockClipRectangle.y1;
	
	Surface* surface = NEW<Surface>();
	surface->resize ( w+2,h+2 );

#if defined (DPVS_OCCLUSIONBUFFER_INDEXBUFFER)	
	DPVS_ASSERT(m_indexBuffer);

	for(int y=0;y<h;y++)
	for(int x=0;x<w;x++)
	{
		UINT32 index = (m_indexBuffer[y*m_loresRectangle.x1 + x]);
		unsigned char c;

		if(index)	
			c = (unsigned char)(FastMath::intFloor(255.f - 255.0f*index/(m_indexBufferIndex+1)));
		else	
			c = 0;

		surface->setPixel(x,y,c);
	}

#else
	// scale floating point values...
	const float*	zb		= reinterpret_cast<const float*>(m_ZBuffer);
	const float		mn		= 0.98f;
	const float		mx		= 1.0f;
	const float		scale	= (mx-mn) ? 255.0f/(mx-mn) : 1.f;

	for(int y=0;y<h;y++)
	for(int x=0;x<w;x++)
	{
		float			t = (zb[y*m_loresRectangle.x1+x]);
		float			f = 255.0f-(t-mn)*scale;
		
		f = Math::min(f,255.0f);
		f = Math::max(f,0.0f);

		unsigned char c = t ? (unsigned char)(Math::intChop(f)) : (unsigned char)(0);

		surface->setPixel(x+1,y+1,c);
	}

#endif
	return surface;

}

/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::grabZBuffer()
 *
 * Description:		
 *
 * Returns:			
 *
 * Notes:			DEBUG visualization function
 *					Callee must destroy the returned surface
 *
 *****************************************************************************/

float*	OcclusionBuffer::grabZBuffer	(int&w,int&h)
{
	w = m_blockClipRectangle.x1;
	h = m_blockClipRectangle.y1;

	float* surf = NEW_ARRAY<float>(h*w);

	for(int y=0;y<h;y++)
	for(int x=0;x<w;x++)
		surf[y*w+x] = *(float*)&m_ZBuffer[y*m_loresRectangle.x1+x];

	return surf;
}

/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::visualizeBBuffer()
 *
 * Description:		Visualizes the full blocks buffer
 *
 * Returns:			pointer to surface (caller is responsible for deleting it)
 *
 * Notes:			DEBUG visualization function - draws image of full blocks buffer
 *
 *****************************************************************************/

Surface* OcclusionBuffer::visualizeBBuffer	(void)
{
	const int	w = m_blockClipRectangle.x1;
	const int	h = m_blockClipRectangle.y1;
	Surface* surface = NEW<Surface>();
	surface->resize ( w+2,h+2 );

	for(int y=0;y<h;y++)
	for(int x=0;x<w;x++)
	{
		bool pixelSet = (m_blockBufferC[y*(m_loresRectangle.x1>>3)+(x>>3)] >> (x&7)) & 1;
		if (pixelSet)
			surface->setPixel(x+1,y+1,(unsigned char)(0xff));
	}

	return surface;
}

//------------------------------------------------------------------------

