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
 * Description:		Surface code
 *
 * $Archive: /dpvs/implementation/sources/dpvsSurface.cpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 14.06.01 16:41 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#include "dpvsSurface.hpp"
#include "dpvsBitMath.hpp"

#include <cstring>		// for memcpy()

using namespace DPVS;

/*****************************************************************************
 *
 * Function:		DPVS::Surface::Surface()
 *
 * Description:		Constructor
 *
 * Parameters:		w = initial width of the surface (in pixels)
 *					h = initial height of the surface (in pixels)
 *
 *****************************************************************************/

Surface::Surface(int w,int h) :
 	m_width(0),
	m_height(0),
	m_data(null)
{
	DPVS_ASSERT (w >= 0 && h >= 0);
	if (w*h)
	{
		resize(w,h);
		fill(0);
	}
}

/*****************************************************************************
 *
 * Function:		DPVS::Surface::~Surface()
 *
 * Description:		Destructor
 *
 *****************************************************************************/

Surface::~Surface(void)
{
	DELETE_ARRAY(m_data);
}

/*****************************************************************************
 *
 * Function:		DPVS::Surface::fill()
 *
 * Description:		Fill surface with specified intensity value
 *
 * Parameters:		c = intensity value
 *
 *****************************************************************************/

void Surface::fill(unsigned char c)
{
	if (m_data)
		fillByte(m_data,c,m_width*m_height);
}

/*****************************************************************************
 *
 * Function:		DPVS::Surface::copy()
 *
 * Description:		Copy surface
 *
 * Parameters:		s = reference to source surface
 *
 *****************************************************************************/

void Surface::copy (const Surface& s)
{
	if (this != &s)
	{
		this->resize(s.getWidth(),s.getHeight());
		if (m_data)
			memcpy(m_data,s.getData(),m_width*m_height);
	}
}

/*****************************************************************************
 *
 * Function:		DPVS::Surface::resize()
 *
 * Description:		Resizes surface
 *
 * Parameters:		w = width of new surface
 *					h = height of new surface
 *
 *****************************************************************************/

void Surface::resize(int w,int h)
{
	DPVS_ASSERT(w>=0 && h>=0);

	if(w==m_width && h==m_height)
		return;

	DELETE_ARRAY(m_data);
	m_data	 = null;

	m_width	 = w;
	m_height = h;

	if(w*h)
	{
		m_data = NEW_ARRAY<unsigned char>(m_width*m_height);
		fill(0);
	}
}

//-----------------------------------------------------------------------


