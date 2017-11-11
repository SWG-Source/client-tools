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
 * Description: 	Occlusion buffer 'BlockBuffer' code
 *
 * $Archive: /dpvs/implementation/sources/dpvsBlockBuffer.cpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 14.06.01 12:40 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#include "dpvsOcclusionBuffer.hpp"
#include "dpvsBlockBuffer.hpp"

using namespace DPVS;

/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::BlockBuffer::BlockBuffer()
 *
 * Description:		Constructor
 *
 *****************************************************************************/

OcclusionBuffer::BlockBuffer::BlockBuffer (int x, int y,unsigned char* full,unsigned char* partial) :
	m_full(null),
	m_partial(null),
	m_width(0),
	m_height(0)
{
	DPVS_ASSERT (isDWordAligned(full) && isDWordAligned(partial));
	DPVS_ASSERT (x >= 0 && y >= 0);

	m_full		= reinterpret_cast<UINT32*>(full);
	m_partial	= reinterpret_cast<UINT32*>(partial);
	m_width		= x>>5;
	m_height	= y;
}

/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::BlockBuffer::isSilhouetteVisible() const
 *
 * Description:		
 *
 * Returns:			
 *
 * Notes:			CURRENTLY UNUSED - DO NOT DELETE
 *
 *****************************************************************************/
/*
OcclusionBuffer::BlockBuffer::Result OcclusionBuffer::BlockBuffer::isSilhouetteVisible(const int* leftEdges, const int* rightEdges, int y0, int n) const
{
	//offset inside the buffer
	UINT32* src = m_full + y0*m_width;
	UINT32	visibleEntries = 0;
	int	x = 0;
	int	i;
	Result result = FULL;

	for(i=0;i<n;i++,src+=m_width)
	{
		int	l	 = leftEdges[i];										// inclusive left  coordinate
		int	r	 = (rightEdges[i]);										// exclusive right coordinate

		DPVS_ASSERT(r>=l);

		if(l==r)
			continue;

		int		ldw  = l/32;											// left  edge dword
		int		rdw  = (r+31)/32 - 1;									// right edge dword (inclusive)
		UINT32	lmsk = FULL_MASK << (l&31);								// left  border dword mask
		UINT32	rmsk = (r&31) ? (~(FULL_MASK << (r&31))) : FULL_MASK;	// right border dword mask

		DPVS_SELECT_ENDIAN(DPVS_NULL_STATEMENT,changeEndian(lmsk));
		DPVS_SELECT_ENDIAN(DPVS_NULL_STATEMENT,changeEndian(rmsk));

		if(ldw==rdw)			//single dword
		{
			x = ldw;
			lmsk &= rmsk;
//			src[x] |= lmsk;				//DEBUG
			visibleEntries = (src[x]&lmsk)^lmsk;
			if(visibleEntries)											//test full coverage
			{
				result = EMPTY;
				break;
			}
		}
		else
		{
			x = ldw;
//			src[x] |= lmsk;				//DEBUG
			visibleEntries = (src[x]&lmsk)^lmsk;
			if(visibleEntries)											//test left dword coverage
			{														
				result = EMPTY;										
				break;												
			}														
																	
			x = rdw;												
//			src[x] |= rmsk;				//DEBUG						
			visibleEntries = (src[x]&rmsk)^rmsk;					
			if(visibleEntries)											//test right dword coverage
			{														
				result = EMPTY;										
				break;												
			}														
																	
			for(x=ldw+1;x<rdw;x++)									
			{														
//				src[x] |= FULL_MASK;	//DEBUG						
				if(src[x] != FULL_MASK)									//test mid dword coverage
				{													
					result = EMPTY;
					break;
				}
			}
		}
	}
	return result;
}
*/
/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::BlockBuffer::setCovered()
 *
 * Description:		
 *
 * Returns:			
 *
 * Notes:			Handles endian issue by calling changeEndian() for
 *					left and right masks.
 *
 *					CURRENTLY UNUSED - DO NOT DELETE
 *
 *****************************************************************************/

/*
void	OcclusionBuffer::BlockBuffer::setCovered	(int x0,int y0,int x1,int y1)
{
	int				ys = y0;
	int				ye = y1;
	int				xs = x0/32;						//dword accuracy start x
	int				xe = (x1+31)/32;				//dword accuracy end x (exclusive)
	UINT32			ml =   0xffffffff << (x0&31);	//left  border dword mask
	UINT32			mr = ~(0xffffffff << (x1&31));	//right border dword mask
	if((x1&31)==0)	mr = 0xffffffff;				//convenient fix

	if(xs == xe-1)
		ml &= mr;						//combined mask

	//If big endian, flip DWORD mask bytes when testing against BYTE buffer
	DPVS_SELECT_ENDIAN(DPVS_NULL_STATEMENT,changeEndian(ml));
	DPVS_SELECT_ENDIAN(DPVS_NULL_STATEMENT,changeEndian(mr));

	for(int x=xs;x<xe;x++)
	{
		UINT32*	src		= m_full + ys*m_width;
		UINT32		cmask	= x==xs?ml: (x==xe-1?mr:FULL_MASK);

		for(int y=ys;y<ye;y++)
		{
			src[x] |= cmask;
			src += m_width;
		}
	}
}
*/

/*****************************************************************************

  *
 * Function:		DPVS::OcclusionBuffer::BlockBuffer::getProblemBlocks()
 *
 * Description:		Generates list of partially covered blocks. Listed
 *					blocks are exposed to more accurate tests.
 *
 * Returns:			Number of partially covered blocks,
 *
 * Notes:			Maximum # of blocks returned is equal to
 *					OcclusionBuffer::m_blockClipRectangle.area().
 *
 *****************************************************************************/

//Builds a list of blocks that were only partially full
int	OcclusionBuffer::BlockBuffer::getProblemBlocks(ByteLoc* list,int x0,int y0,int x1,int y1) const
{
	static int tmplist[32];	// DEBUG DEBUG SHOULD THIS BE STATIC ????
	int				ys = y0;
	int				ye = y1;
	int				xs = x0>>5;						//dword accuracy start x
	int				xe = (x1+31)>>5;				//dword accuracy end x (exclusive)
	UINT32			ml =   0xffffffff << (x0&31);	//left  border dword mask
	UINT32			mr = ~(0xffffffff << (x1&31));	//right border dword mask
	if((x1&31)==0)	mr = 0xffffffff;				//convenient fix

	if(xs == xe-1)
		ml &= mr;						//combined mask

	//If big endian, flip DWORD mask bytes when testing against BYTE buffer
	DPVS_SELECT_ENDIAN(DPVS_NULL_STATEMENT,changeEndian(ml));
	DPVS_SELECT_ENDIAN(DPVS_NULL_STATEMENT,changeEndian(mr));

	int cnt = 0;

	for(int x=xs;x<xe;x++)
	{
		UINT32 mask = x==xs?ml: (x==xe-1?mr:0xffffffff);
		int		o	 = ys*m_width + x;

		for(int y=ys;y<ye;y++)
		{
			UINT32 problems = ((m_full[o]&mask)^mask) & m_partial[o]; //bits requiring further investigation are set
			if(problems)
			{
				DPVS_SELECT_ENDIAN(DPVS_NULL_STATEMENT,changeEndian(problems));
				int c = maskToList(tmplist,problems);	//get problem bit numbers
				for(int i=0;i<c;i++)
					list[cnt++].set(x*32+tmplist[i],y);	//store block coordinates (x,y)
			}
			o += m_width;
		}
	}
	return cnt;
}


/*****************************************************************************
 *
 * Function:		generatePosition()
 *
 * Description:		Returns x coordinate that is closest to center
 *
 * Notes:			Incoming mask (x) is always little endian
 *
 *****************************************************************************/
/*
static int generatePosition(int x,UINT32 failedEntries)
{
	static bool valid = false;
	static signed char lowestLUT[256];
	static signed char highestLUT[256];

	if(!valid)
	{
		valid = true;
		for(int i=0;i<256;i++)
		{
			int mn = 8;
			int mx = -1;
			for(int b=0;b<8;b++)
			{
				if(i&(1<<b))
				{
					if(b<mn) mn=b;
					if(b>mx) mx=b;
				}
			}
			lowestLUT[i]  = (signed char)(mn!=8  ? mn   : -1);
			highestLUT[i] = (signed char)(mx!=-1 ? 7-mx : -1);		//inverted values
		}
	}

	int l = lowestLUT [(failedEntries>>16)&255];
	int r = highestLUT[(failedEntries>>8)&255];

	if(l!=-1 && r!=-1)
		return (l<r) ? (32*x+15-l) : (32*x+16+r);
	if(l!=-1)
		return 32*x+15-l;
	if(r!=-1)
		return 32*x+16+r;

	l = lowestLUT [(failedEntries>>24)];
	r = highestLUT[(failedEntries)&255];

	if(l!=-1 && r!=-1)
		return (l<r) ? (32*x+15-(l+8)) : (32*x+16+r+8);
	if(l!=-1)
		return 32*x+15-(l+8);
	if(r!=-1)
		return 32*x+16+r+8;

	DPVS_ASSERT(false);
	return 0;
}
*/
//------------------------------------------------------------------------


