#ifndef __DPVSWRAPPER_HPP
#define __DPVSWRAPPER_HPP
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
 * Desc:	Some DPVS wrapper inline code
 *
 * $Archive: /dpvs/implementation/include/dpvsWrapper.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 8.10.02 14:33 $
 * $Date: 2003/01/22 $
 * 
 *****************************************************************************/

#if !defined (__DPVSPRIVATEDEFS_HPP)
#	include "dpvsPrivateDefs.hpp"
#endif

namespace DPVS
{ 
	extern  "C" bool g_libraryInitialized;
} // namespace DPVS

//--------------------------------------------------------------------
// assertion macros for checking that a matrix is a valid 4x3 matrix (i.e. bottom row is 0,0,0,1)
//--------------------------------------------------------------------

#define DPVS_ASSERT_4X3_MATRIX(X)			DPVS_API_ASSERT(X.is4x3Matrix() && "The matrix specified may not contain a projection term!")
#define DPVS_ASSERT_CELL_VALIDITY()			DPVS_API_ASSERT(this && "Cell pointer is null - origin of the problem is probably an object/camera that is not in any cell");
#define DPVS_ASSERT_INIT()					DPVS_API_ASSERT(g_libraryInitialized && "Function called outside Library::init() / Library::exit() pair")
#define DPVS_ASSERT_NO_VISIBILITY_QUERY()	DPVS_API_ASSERT(!g_visibilityQuery && "Function cannot be called during resolveVisibility()!")
#define DPVS_ASSERT_SELF_OK()				DPVS_API_ASSERT(Memory::isValidPointer(this) && "Application has called function with a broken pointer")

//------------------------------------------------------------------------
#endif //__DPVSWRAPPER_HPP
