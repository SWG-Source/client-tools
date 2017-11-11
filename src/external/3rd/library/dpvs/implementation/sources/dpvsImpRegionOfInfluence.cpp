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
 * Description:		Region Of Influence implementation
 *
 * $Archive: /dpvs/implementation/sources/dpvsImpRegionOfInfluence.cpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 14.06.01 13:09 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#include "dpvsImpRegionOfInfluence.hpp"
#include "dpvsInstanceCount.hpp"

using namespace DPVS;

/*****************************************************************************
 *
 * Function:		DPVS::ImpRegionOfInfluence::ImpRegionOfInfluence()
 *
 * Description:		
 *
 * Parameters:
 *
 *****************************************************************************/

ImpRegionOfInfluence::ImpRegionOfInfluence	(Model* testModel) : 
	ImpObject(testModel)
{
	setType(TYPE_REGION_OF_INFLUENCE);

	setContributionCulling(false);	// disable by default
	setRenderCost(10000.f);			// large number so that we always attempt to cull the ROI
	InstanceCount::incInstanceCount(InstanceCount::REGIONOFINFLUENCE);
}

/*****************************************************************************
 *
 * Function:		DPVS::ImpRegionOfInfluence::~ImpRegionOfInfluence()
 *
 * Description:		
 *
 * Parameters:
 *
 *****************************************************************************/

ImpRegionOfInfluence::~ImpRegionOfInfluence	(void)
{
	InstanceCount::decInstanceCount(InstanceCount::REGIONOFINFLUENCE);
}

//------------------------------------------------------------------------
