#ifndef __DPVSSURFACE_HPP
#define __DPVSSURFACE_HPP
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
 * Desc:	Internal surface class for debug purposes
 *
 * $Archive: /dpvs/implementation/include/dpvsSurface.hpp $
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
 * Class:			DPVS::Surface
 *
 * Description:		Byte-per-pixel surface class for debug visualization
 *
 ******************************************************************************/

namespace DPVS
{
	class Surface
	{
	public:
								Surface		(int w=0,int h=0);
								~Surface	(void);

		void					copy		(const Surface &s);
		void					fill		(unsigned char c=0);
		const unsigned char*	getData		(void) const { return m_data; }
		int						getHeight	(void) const { return m_height; }
		unsigned char			getPixel	(int x,int y) const;
		int						getWidth	(void) const { return m_width; }
		void					resize		(int w,int h);
		void					setPixel	(int x,int y,unsigned char c);
	private:
								Surface		(const Surface&);
		Surface&				operator=	(const Surface&);
		int						m_width;
		int						m_height;
		unsigned char*			m_data;
	};

	DPVS_FORCE_INLINE void Surface::setPixel (int x,int y,unsigned char c)
	{
		DPVS_ASSERT(x>=0 && y>=0 && x<m_width && y<m_height);
		m_data[y*m_width+x] = c;
	}

	DPVS_FORCE_INLINE unsigned char Surface::getPixel	(int x,int y) const
	{
		DPVS_ASSERT(x>=0 && y>=0 && x<m_width && y<m_height);
		return m_data[y*m_width+x];
	}
} // DPVS

//------------------------------------------------------------------------
#endif // __DPVSSURFACE_HPP
