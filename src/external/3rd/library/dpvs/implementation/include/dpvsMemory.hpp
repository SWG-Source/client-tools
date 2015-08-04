#ifndef __DPVSMEMORY_HPP
#define __DPVSMEMORY_HPP
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
 * Desc:	Memory management
 *
 * $Archive: /dpvs/implementation/include/dpvsMemory.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 2.10.02 13:48 $
 * $Date: 2003/01/22 $
 * 
 *****************************************************************************/

#if !defined (__DPVSPRIVATEDEFS_HPP)
#	include "dpvsPrivateDefs.hpp"
#endif

namespace DPVS
{

/******************************************************************************
 *
 * Class:			DPVS::Memory
 *
 * Description:		Class containing memory-related functions
 *
 * Notes:			This class redirects memory allocation calls to the
 *					Heap class and provides debugging + leak detection
 *					information in the debug build.
 *
 ******************************************************************************/

class Memory
{
public:
	static void*	allocate			(int bytes);
	static void		checkConsistency	(void);
	static void		dump				(void);
	static bool		isValidPointer		(const void*);
	static int		getAllocationCount	(void) { return s_allocates - s_releases;	}	
	static int		getMemoryReserved	(void);
	static int		getMemoryUsed		(void);
	static void	    release				(void*)  ;
	static void		shutdown			(void);
private:

	static void*	allocateInternal	(int bytes);
	static void	    releaseInternal		(void*);
	static void		checkInternal		(const void*);
	
	static int		s_allocates;	// number of allocate() calls made
	static int		s_releases;		// number of release() calls made
//	static bool		s_isShutDown;
};

} // DPVS

//------------------------------------------------------------------------
#endif // __DPVSMEMORY_HPP
