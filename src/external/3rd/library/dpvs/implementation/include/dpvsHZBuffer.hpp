#ifndef __DPVSHZBUFFER_HPP
#define __DPVSHZBUFFER_HPP
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
 * Desc:	Hierarchical depth buffer
 *
 * $Archive: /dpvs/implementation/include/dpvsHZBuffer.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 7.05.01 12:06 $
 * $Date: 2003/01/22 $
 * 
 *****************************************************************************/

#if !defined (__DPVSPRIVATEDEFS_HPP)
#	include "dpvsPrivateDefs.hpp"
#endif

/******************************************************************************
 *
 * Class:			DPVS::IHZBuffer
 *
 * Description:		Hierarchical depth buffer interface
 *
 * Notes:			The hierarchical depth buffer operates on top of a
 *					standard depth buffer. Whenever regions in the
 *					"real" depth buffer are modified, the IHZBuffer should
 *					be notified by calling IHZBuffer::setRegionModified().
 *
 *					The IHZBuffer combines modified regions internally and
 *					updates the hierarchical depth buffer in a lazy fashion.
 *					For this reason it is much more efficient to perform
 *					a number of modifications first and then a number of
 *					queries rather than performing modify-query-modify-query.
 *
 *					The "real" depth buffer is passed in as a constructor
 *					parameter and may _not_ be destroyed until the IHZBuffer
 *					is destructed.
 *
 *					The hierarchical depth buffer performs internal
 *					consistency checking in debug build.
 *
 ******************************************************************************/

namespace DPVS
{
	class IHZBuffer
	{
	public:
		typedef UINT32 DepthValue;

					IHZBuffer				(const DepthValue* src, int log2xWidth, int log2yHeight, int p);
					~IHZBuffer				(void);
		DepthValue	getFarDepth				(void) const;
		bool		isVisible				(int x,  int y,  DepthValue z) const;
		bool		isVisible				(int x0, int y0, int x1, int y1, DepthValue z) const;
		bool		isSilhouetteVisible		(const int* leftEdges, const int* rightEdges, DepthValue z, int y0, int n) const;
		void		setRegionModified		(int x0, int y0, int x1, int y1);
	private:
					IHZBuffer				(const IHZBuffer&); // not allowed
		IHZBuffer&	operator=				(const IHZBuffer&);	// not allowed
		class HZBuffer*	m_ptr;				// opaque pointer
	};
} // DPVS

//------------------------------------------------------------------------
#endif // __DPVSHZBUFFER_HPP
