#ifndef __DPVSMT_HPP
#define __DPVSMT_HPP
/*****************************************************************************
 *
 * dPVS
 * -----------------------------------------
 *
 * (C) 1999-2002 Hybrid Graphics, Ltd.
 * All Rights Reserved.
 *
 * This file consists of unpublished, proprietary source code of
 * Hybrid Graphics, and is considered Confidential Information for
 * purposes of non-disclosure agreement. Disclosure outside the terms
 * outlined in signed agreement may result in irrepairable harm to
 * Hybrid Graphics and legal action against the party in breach.
 *
 *
 * Desc:	Some classes for multi-threaded version of dPVS
 *
 * $Archive: /dpvs/implementation/include/dpvsMT.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 8.10.02 14:19 $
 * $Date: 2003/01/22 $
 * 
 *****************************************************************************/

//#include <windows.h>

#if !defined (__DPVSDEBUG_HPP)
#	include "dpvsDebug.hpp"
#endif

namespace DPVS
{

/*
struct Mutex
{
public:
	Mutex (void)	{ InitializeCriticalSection(&cs); }
	~Mutex (void)	{ DeleteCriticalSection(&cs); }
	void enter (void) { EnterCriticalSection(&cs); }
	void leave (void) { LeaveCriticalSection(&cs); }
private:
	CRITICAL_SECTION cs;
};
*/
/******************************************************************************
 *
 * Class:			DPVS::Lock
 *
 * Description:		Class for acquiring/releasing the global Mutex (the
 *				    Mutex implementation itself is provided by the user)
 *
 ******************************************************************************/

class Lock			
{
public:
	DPVS_FORCE_INLINE Lock  (void) 
	{ 
#if defined (DPVS_MT)
		Debug::getServices()->enterMutex(); 
#endif
	}

	DPVS_FORCE_INLINE ~Lock (void) 
	{ 
#if defined (DPVS_MT)
		Debug::getServices()->leaveMutex(); 
#endif
	}

private:
		  Lock		(const Lock&);
	Lock& operator=	(const Lock&);
};

} // DPVS

//------------------------------------------------------------------------
#endif //__DPVSMT_HPP
