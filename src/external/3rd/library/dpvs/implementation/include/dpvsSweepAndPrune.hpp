#ifndef __DPVSSWEEPANDPRUNE_HPP
#define __DPVSSWEEPANDPRUNE_HPP

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
 * Description: 	Object vs. ROI sweep and prune code
 *
 * $Archive: /dpvs/implementation/include/dpvsSweepAndPrune.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 7.05.01 12:06 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#if !defined (__DPVSPRIVATEDEFS_HPP)
#	include "dpvsPrivateDefs.hpp"
#endif


namespace DPVS
{

class Commander;
class ImpCommander;
class ImpObject;

/*****************************************************************************
 *
 * Class:			DPVS::SweepAndPrune
 *
 * Description:		Class for performing sweep & prune overlap testing
 *					between objects and ROIs
 *
 * Notes:			The process() function of this class handles all
 *					signaling to the Commander.
 *
 *****************************************************************************/

class SweepAndPrune
{
public:
					SweepAndPrune		(void); 
					~SweepAndPrune		(void); 
	void			process				(Commander* cmd, const ImpObject* const * obs, const ImpObject* const* ROIs, int nObs, int nROI, const Vector3& primarySweepDirection);
	
	static void		minimizeMemoryUsage	(void);
private:
					SweepAndPrune	(const SweepAndPrune&);	// not allowed
	SweepAndPrune&	operator=		(const SweepAndPrune&);	// not allowed

	class ImpSweepAndPrune* m_imp;		// opaque pointer
};

} // DPVS

//------------------------------------------------------------------------
#endif // __DPVSSWEEPANDPRUNE_HPP
