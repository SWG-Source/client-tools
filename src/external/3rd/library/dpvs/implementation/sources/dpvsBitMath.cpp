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
 * Description:		Bitmath implementation
 *
 * $Archive: /dpvs/implementation/sources/dpvsBitMath.cpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 26.08.02 18:20 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#include "dpvsBitMath.hpp"

namespace DPVS
{

BitVector& BitVector::operator=	(const BitVector& s)	
{ 
	if (&s != this) 
	{ 
		reset(s.m_dwords*32); 
		if (m_dwords)
			::memcpy (m_array, s.m_array,m_dwords*sizeof(UINT32)); 
	} 
	return *this; 
}


#if !defined (DPVS_X86_ASSEMBLY) && (!defined (DPVS_CPU_GAMECUBE) || !defined (DPVS_BUILD_CW)) // we don't need the LUT for x86/GameCube machines as they can use the bsr operation
namespace BitMath
{
	extern "C" const signed char s_highestLUT[256] =			// lookup table for 'highest set bit' operation
	{
		-1,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,
		4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
		5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
		5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
		6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
		6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
		6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
		6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7
	};
} // DPVS::BitMath
#endif // !DPVS_X86_ASSEMBLY && (!defined (DPVS_CPU_GAMECUBE) || !defined (DPVS_BUILD_CW))
} // DPVS

//------------------------------------------------------------------------
