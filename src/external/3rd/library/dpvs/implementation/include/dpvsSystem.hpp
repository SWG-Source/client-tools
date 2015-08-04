#ifndef __DPVSSYSTEM_HPP
#define __DPVSSYSTEM_HPP
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
 * Desc:	Low-level system functionality
 *
 * $Archive: /dpvs/implementation/include/dpvsSystem.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 12.02.02 14:28 $
 * $Date: 2003/01/22 $
 * 
 *****************************************************************************/

#if !defined (__DPVSX86_HPP)
#	include "dpvsX86.hpp"
#endif

namespace DPVS
{
/******************************************************************************
 *
 * Class:			DPVS::System
 *
 * Description:		Low-level platform independent functionality
 *
 * Notes:			This class contains various low-level functions that
 *					don't fall into any other category. Different implementations
 *					are required for different platforms.
 *
 ******************************************************************************/

class System
{
public:
	static DPVS_FORCE_INLINE void		prefetch		(const void* mem);		
	static DPVS_FORCE_INLINE double		getCycleCount	(void);
};

//------------------------------------------------------------------------
// Implementation
//------------------------------------------------------------------------

// Prefetch memory at location 'mem'. This prefetch *MUST* be made a 
// memory location that is valid (i.e. read access exists!!)

#if defined (DPVS_CPU_X86)
	DPVS_FORCE_INLINE void		System::prefetch		(const void* mem)				{ X86::prefetch(mem);			}
	DPVS_FORCE_INLINE double	System::getCycleCount	(void)							{ return X86::getCycleCount();	}
#else
	DPVS_FORCE_INLINE void		System::prefetch		(const void*)			{/* prefetching not supported on this system*/}
	DPVS_FORCE_INLINE double	System::getCycleCount	(void)					{ return 0.0; /* cycle count not supported on this system*/}
#endif

} // DPVS


//------------------------------------------------------------------------
#endif // __DPVSX86_HPP
