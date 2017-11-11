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
 * Description: 	Class reference-count checking code
 *
 * $Archive: /dpvs/implementation/sources/dpvsInstanceCount.cpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 7.05.01 12:06 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#include "dpvsInstanceCount.hpp"

using namespace DPVS;

int InstanceCount::s_references[CLASS_MAX];		// # of existing instances

/*****************************************************************************
 *
 * Function:		DPVS::InstanceCount::init()
 *
 * Description:		
 *
 *****************************************************************************/

void InstanceCount::init (void)
{
	for(int i=0;i<CLASS_MAX;i++)
		s_references[i] = 0;
}

/*****************************************************************************
 *
 * Function:		DPVS::InstanceCount::exit()
 *
 * Description:		
 *
 *****************************************************************************/

void InstanceCount::exit (void)
{
	for(int i=0;i<CLASS_MAX;i++)
	{
		if (!s_references[i])
			continue;

		switch(i)
		{
			case 0:		DPVS_API_ASSERT(!"Library::exit() - Some Cells were not released prior to Library::exit()");			break;
			case 1:		DPVS_API_ASSERT(!"Library::exit() - Some Cameras were not released prior to Library::exit()");			break;
			case 2:		DPVS_API_ASSERT(!"Library::exit() - Some Viewers were not released prior to Library::exit()");			break;
			case 3:		DPVS_API_ASSERT(!"Library::exit() - Some VirtualPortals were not released prior to Library::exit()");	break;
			case 4:		DPVS_API_ASSERT(!"Library::exit() - Some PhysicalPortals were not released prior to Library::exit()");	break;
			case 5:		DPVS_API_ASSERT(!"Library::exit() - Some RegionOfInfluences were not released prior to Library::exit()");break;
			case 6:		DPVS_API_ASSERT(!"Library::exit() - Some Objects were not released prior to Library::exit()");			break;
			case 7:		DPVS_API_ASSERT(!"Library::exit() - Some MeshModels were not released prior to Library::exit()");		break;
			case 8:		DPVS_API_ASSERT(!"Library::exit() - Some Models were not released prior to Library::exit()");			break;
			default:	DPVS_ASSERT(!"INTERNAL: Unknown object type in Library::exit()");						break;
		}

		s_references[i] = 0;					// Make sure
	}
}

//------------------------------------------------------------------------
