#ifndef __DPVSIMPREGIONOFINFLUENCE_HPP
#define __DPVSIMPREGIONOFINFLUENCE_HPP
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
 * Description: 	Physical portal implementation class
 *
 * $Archive: /dpvs/implementation/include/dpvsImpRegionOfInfluence.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 7.05.01 12:06 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#if !defined (__DPVSIMPOBJECT_HPP)
#	include "dpvsImpObject.hpp"
#endif

namespace DPVS
{
/******************************************************************************
 *
 * Class:			DPVS::ImpRegionOfInfluence
 *
 * Description:		Implementation of RegionOfInfluence
 *
 *****************************************************************************/

class ImpRegionOfInfluence : public ImpObject
{
public:
							ImpRegionOfInfluence	(Model* testModel);
	virtual					~ImpRegionOfInfluence	(void);

private:
							ImpRegionOfInfluence	(const ImpRegionOfInfluence&);
	ImpRegionOfInfluence&	operator=				(const ImpRegionOfInfluence&);
};
} // DPVS

//------------------------------------------------------------------------
#endif // __DPVSIMPREGIONOFINFLUENCE_HPP
