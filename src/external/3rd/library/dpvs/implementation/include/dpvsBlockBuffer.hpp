#ifndef __DPVSBLOCKBUFFER_HPP
#define __DPVSBLOCKBUFFER_HPP

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
 *
 * Desc:	Filler code for "dpvsOcclusionBuffer_render.cpp"
 *			Included only in a single place. DO NOT MOVE the functions
 *			to .cpp file.
 *
 * $Archive: /dpvs/implementation/include/dpvsBlockBuffer.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 14.06.01 12:41 $
 * $Date: 2003/01/22 $
 * 
 *****************************************************************************/

#if !defined (__DPVSOCCLUSIONBUFFER_HPP)
#	include "dpvsOcclusionBuffer.hpp"
#endif

namespace DPVS
{
/******************************************************************************
 *
 * Class:			DPVS::OcclusionBuffer::BlockBuffer
 *
 * Description:		
 *
 ******************************************************************************/

class OcclusionBuffer::BlockBuffer
{
private:
					BlockBuffer	(const BlockBuffer&);		// not allowed
	BlockBuffer&	operator=	(const BlockBuffer&);		// not allowed	

	UINT32*		m_full;
	UINT32*		m_partial;
	int			m_width;
	int			m_height;
public:
	enum Result
	{
		FULL	= 0,
		PARTIAL	= 1,
		EMPTY	= 2
	};

			BlockBuffer			(int x,int y,unsigned char *full,unsigned char* partial);

	Result	isPointVisible		(int x0,int y0) const;
	Result	isRectangleVisible	(int x0,int y0,int x1,int y1) const;
	void	setFullBlocksBuffer	(unsigned char *full)				
	{
		DPVS_ASSERT(isDWordAligned(full));
		m_full = reinterpret_cast<UINT32*>(full); // for contribution culling
	} 
	int		getProblemBlocks	(ByteLoc* list,int x0,int y0,int x1,int y1) const;

	//CURRENTLY UNUSED FUNCTIONS
//	Result	isSilhouetteVisible	(const int* leftEdges, const int* rightEdges, int y0, int n) const;
//	void	setCovered			(int x0,int y0,int x1,int y1);
};

/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::BlockBuffer::isPointVisible(int x0,int y0) const
 *
 * Description:		
 *
 * Returns:			
 *
 * Notes:			Handles endian issue by calling changeEndian() for the mask.
 *					- Single caller
 *
 *****************************************************************************/

DPVS_FORCE_INLINE OcclusionBuffer::BlockBuffer::Result OcclusionBuffer::BlockBuffer::isPointVisible(int x0,int y0) const
{
	DPVS_ASSERT(x0>=0);

	int			xs = x0>>5;						// dword accuracy start x
	int			o  = (y0*m_width + xs);			// offset inside DWORD buffer
	UINT32		ml = (UINT32)(1<<(x0&31));		// bitmask

	//If big endian, flip DWORD mask bytes when testing against BYTE buffer
	DPVS_SELECT_ENDIAN(DPVS_NULL_STATEMENT,changeEndian(ml));

	if(m_full[o] & ml)
		return FULL;

	if(m_partial[o] & ml)
		return PARTIAL;

	return EMPTY;
}

/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::BlockBuffer::isRectangleVisible(int x0,int y0,int x1,int y1,int &xpos,int &ypos) const
 *
 * Description:		
 *
 * Returns:			
 *
 * Notes:			Handles endian issue by calling changeEndian() for
 *					left and right masks.
 *					- Single caller
 *
 *****************************************************************************/

DPVS_FORCE_INLINE OcclusionBuffer::BlockBuffer::Result OcclusionBuffer::BlockBuffer::isRectangleVisible(int x0,int y0,int x1,int y1) const
{
	DPVS_ASSERT(x0 >= 0 && y0 >= 0);

	int				ys = y0;
	int				ye = y1;
	int				xs = x0>>5;							//dword accuracy start x
	int				xe = (x1+31)>>5;					//dword accuracy end x (exclusive)
	UINT32			ml =   0xffffffff << (x0&31);		//left  border dword mask
	UINT32			mr = ~(0xffffffff << (x1&31));		//right border dword mask
	if((x1&31)==0)	mr = 0xffffffff;					//convenient fix

	if(xs == xe-1)
		ml &= mr;						//combined mask

	//If big endian, flip DWORD mask bytes when testing against BYTE buffer
	DPVS_SELECT_ENDIAN(DPVS_NULL_STATEMENT,changeEndian(ml));
	DPVS_SELECT_ENDIAN(DPVS_NULL_STATEMENT,changeEndian(mr));

	Result result = FULL;

	for(int	x=xs;x<xe;x++)
	{
		UINT32 mask = x==xs?ml: (x==xe-1?mr:FULL_MASK);
		int		o	 = ys*m_width + x;

		for(int y=ys;y<ye;y++)
		{
			if((m_full[o]&mask) != mask)				//test full coverage
				result = PARTIAL;

			if(((m_full[o]|m_partial[o])&mask) != mask)	//test potential coverage
				return EMPTY;							//Empty block encountered -> VISIBLE

			o += m_width;
		}
	}
	return result;										//POSSIBLY occluded
}


} //DPVS

//------------------------------------------------------------------------
#endif //__DPVSBLOCKBUFFER_HPP

