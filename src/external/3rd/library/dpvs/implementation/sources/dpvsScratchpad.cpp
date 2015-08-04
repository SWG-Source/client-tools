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
 * Desc:	Scratch pad memory class. Targetted for PS2, but used
 *			also by all other platforms.
 *
 * $Archive: /dpvs/implementation/sources/dpvsScratchpad.cpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 12.02.02 15:03 $
 * $Date: 2003/01/22 $
 * 
 *****************************************************************************/

#include "dpvsScratchpad.hpp"
#include <cstring>										// for memcpy

using namespace DPVS;

//------------------------------------------------------------------------
// Static variables
//------------------------------------------------------------------------

unsigned char	Scratchpad::s_allocated[0x4000];	
bool			Scratchpad::s_valid = false;

/*****************************************************************************
 *
 * Function:		DPVS::Scratchpad::push()
 *
 * Description:		Stores contents of the scratchpad into memory (on PS2).
 *					On other platforms does nothing
 *
 *****************************************************************************/

void Scratchpad::push	(void)
{
	DPVS_ASSERT(!s_valid);
	s_valid = true;
#if defined (DPVS_PS2) && defined (DPVS_PS2_USE_SCRATCHPAD)
//PS2-BEGIN
	::memcpy(s_allocated,(const void*)0x70000000,MAX_ADDRESS);	// preserve spad
//PS2-END
#endif

#if defined (DPVS_DEBUG)
	::memset(getPtr(BASE),0xCD,MAX_ADDRESS);					// trash spad
#endif
}

/*****************************************************************************
 *
 * Function:		DPVS::Scratchpad::pop()
 *
 * Description:		Restores stored scratchpad contents on PS2. No-op for
 *					other platforms.
 *
 *****************************************************************************/

void Scratchpad::pop	(void)
{
	if (s_valid)
	{
#if defined (DPVS_PS2) && defined (DPVS_PS2_USE_SCRATCHPAD)
//PS2-BEGIN
		memcpy((void*)0x70000000,s_allocated,MAX_ADDRESS);		// restore spad
//PS2-END
#else
	#if defined (DPVS_DEBUG)
		::memset(getPtr(BASE),0xBA,MAX_ADDRESS);				// trash spad
	#endif
#endif
		s_valid = false;
	}
}

//------------------------------------------------------------------------
