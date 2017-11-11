#ifndef __DPVSEVALUATION_HPP
#define __DPVSEVALUATION_HPP
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
 * Description: 	Evaluation copy code
 *
 * $Archive: /dpvs/implementation/include/dpvsEvaluation.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 17.09.01 12:47 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#if !defined (__DPVSPRIVATEDEFS_HPP)
#	include "dpvsPrivateDefs.hpp"
#endif

// enable DPVS_EVALUATION to build the FlexLM-protected evaluation build

//#define DPVS_EVALUATION

#if defined (DPVS_EVALUATION) && !defined (DPVS_DEBUG) && defined (DPVS_OS_WIN32)
#	include "../flexlm/dpvsflexlm.hpp"
#   pragma comment (lib, "dpvsflexlm.lib")    
#else
#	undef DPVS_EVALUATION
#endif

//------------------------------------------------------------------------
#endif // __DPVSEVALUATION_HPP
