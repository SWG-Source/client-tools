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
 * Description: 	Commander code
 *
 * $Archive: /dpvs/implementation/sources/dpvsImpCommander.cpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 14.06.01 13:00 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#include "dpvsImpCommander.hpp"
#include "dpvsOcclusionBuffer.hpp"
#include "dpvsSurface.hpp"

// DPVS::Commander stuff is in dpvsWrapper.cpp

using namespace DPVS;

ImpCommander* ImpCommander::s_first = null;		// linked list

/*****************************************************************************
 *
 * Function:		DPVS::ImpCommander::releaseAll()
 *
 * Description:		Destroys all ImpCommanders
 *
 *****************************************************************************/

void ImpCommander::releaseAll (void)
{
	while (s_first)
	{
		// the dtor automagically handles unlinking.. so, yes, this function is safe...
		DELETE(s_first);	
	}
}

/*****************************************************************************
 *
 * Function:		DPVS::ImpCommander::getBuffer()
 *
 * Description:		Copies current debug buffer into surface (may modify
 *					size of the surface!)
 *
 *****************************************************************************/

Library::BufferType ImpCommander::getBuffer	(Surface &surface)
{
	DPVS_ASSERT(m_occlusionBuffer);
	DPVS_ASSERT(&surface);

	Surface* s = null;

	switch(m_bufferType)
	{
		case Library::BUFFER_COVERAGE:		s = m_occlusionBuffer->visualizeCBuffer(); break;
		case Library::BUFFER_DEPTH:			s = m_occlusionBuffer->visualizeZBuffer(); break;
		case Library::BUFFER_FULLBLOCKS:	s = m_occlusionBuffer->visualizeBBuffer(); break;
		case Library::BUFFER_STENCIL:		s = m_occlusionBuffer->visualizeSBuffer(); break;
		default: 
			DPVS_ASSERT(!"ImpCamera::grabBuffer() - Unknown buffer type"); 
			break;
	}

	if (s)
	{
		surface.resize	(s->getWidth(), s->getHeight());
		surface.copy	(*s);
		DELETE(s);
	}

	return m_bufferType;
}

//------------------------------------------------------------------------
