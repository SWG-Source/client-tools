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
 * Description: 	DataPasser code
 *
 * $Archive: /dpvs/implementation/sources/dpvsDataPasser.cpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 8.10.02 12:35 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#include "dpvsDataPasser.hpp"
#include "dpvsImpCommander.hpp"

using namespace DPVS;

//------------------------------------------------------------------------
// Static variables
//------------------------------------------------------------------------

Commander*	DataPasser::s_commander				= 0;
int			DataPasser::s_frameCounter			= 0;
float		DataPasser::s_queryTime				= 0.0f;
float		DataPasser::s_objectImportance		= 1.f;
ImpCamera*	DataPasser::s_viewerCamera			= 0;

/*****************************************************************************
 *
 * Function:		DataPasser::getImpCommander()
 *
 * Description:		Returns implementation of the current commander or null
 *
 * Returns:			implementation pointer or null
 *
 *****************************************************************************/

ImpCommander* DataPasser::getImpCommander		(void)
{
	DPVS_ASSERT(s_commander);
	if(s_commander)
		return s_commander->getImplementation();
	else
		return null;
}

//------------------------------------------------------------------------
