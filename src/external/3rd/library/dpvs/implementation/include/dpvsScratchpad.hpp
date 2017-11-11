#ifndef __DPVSSCRATCHPAD_HPP
#define __DPVSSCRATCHPAD_HPP
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
 * $Archive: /dpvs/implementation/include/dpvsScratchpad.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 12.02.02 15:02 $
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
 * Class:			DPVS::ScratchPad
 *
 * Description:		16Kb ScratchPad memory class designed for PS2.
 *
 *
 ******************************************************************************/

class Scratchpad
{
private:
	static UINT8 DPVS_ALIGN32(s_allocated[0x4000]);				// copy of the data (or real buffer on non-PS2 machines)
	static bool  s_valid;										// is Scratchpad use ok?

					Scratchpad	(void);
					Scratchpad	(const Scratchpad&);
					~Scratchpad (void);
public:

	enum Address
	{
		BASE				  = 0,
		MATRIX_CACHE          = 0,								// 4096 bytes
		OCCLUSIONBUFFER_CACHE = MATRIX_CACHE + 64*64,			// 768 bytes		
		VQDATA				  = OCCLUSIONBUFFER_CACHE + 768,	// 384 bytes
		BLOCKBUFFERS          = VQDATA + 384,					// 1024*3+96 bytes
		WRITEQUEUE_BUCKETS	  = BLOCKBUFFERS + 1024*3+96,		// 2560 bytes
		MAX_ADDRESS			  = WRITEQUEUE_BUCKETS+2560			// maximum supported address (MUST be <= 0x4000)
	};

	DPVS_CT_ASSERT(MAX_ADDRESS <= 0x4000);						// must be smaller than 0x4000

	static void		push		(void);
	static void		pop			(void);

	// this function can be called even outside Scratchpad::push() / pop() pair. Note that
	// the pointer _should not_ be used 
	static DPVS_FORCE_INLINE void*	getAddress (Address offset)
	{
		DPVS_ASSERT(offset >= 0 && offset < MAX_ADDRESS);
#if defined (DPVS_PS2) && defined (DPVS_PS2_USE_SCRATCHPAD)
//PS2-BEGIN
		return (void*)(0x70000000 + (int)offset);
//PS2-END
#else

		//DPVS_ASSERT(!((unsigned long)(s_allocated)&31));	// WHAT?
		return &s_allocated[offset];
#endif
	}

	// this function can be called only inside a Scratchpad::push()/pop() pair

	static DPVS_FORCE_INLINE void*	getPtr (Address offset)
	{
		DPVS_ASSERT(s_valid);
		return getAddress(offset);
	}


};

} // DPVS


//------------------------------------------------------------------------
#endif //__DPVSSCRATCHPAD_HPP
