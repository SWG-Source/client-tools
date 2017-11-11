#ifndef __DPVSRANDOM_HPP
#define __DPVSRANDOM_HPP
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
 * Description: 	Random number generation code
 *
 * $Archive: /dpvs/implementation/include/dpvsRandom.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 9.05.01 17:07 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#if !defined (__DPVSPRIVATEDEFS_HPP)
#	include "dpvsPrivateDefs.hpp"
#endif

namespace DPVS
{
/******************************************************************************
 *
 * Class:			DPVS::Random
 *
 * Description:		Random number generator that returns floating point
 *					random numbers in range [0,1[ or integer random numbers
 *					in range [0,2^32-1]
 *
 * Notes:			The code is based on Agner Fog's RANROT A algorithm
 *					and original C++ implementation. The if's from the original
 *					code have been replaced by bit arithmetic and the code
 *					is somewhat cleaned up. For further information about
 *					the algorithm, please see "http://www.agner.org/random/".
 *
 *					The integer version executes in approximately 13.5 Pentium II
 *					clocks and the floating point version in 17.5.
 *
 *****************************************************************************/

class Random
{            
public:
     						Random		(void);
	float					get			(void);                   
	UINT32					getI		(void);
	void					reset		(UINT32 seed);       
private:
	enum
	{
		KK = 11,
		JJ =  7,
		RR = 13
	};

	int		m_p1, m_p2;                    
	UINT32	m_randbuffer[KK];            
};

extern Random g_random;			// one global random-number generator

} // DPVS
//------------------------------------------------------------------------
#endif // __DPVSRANDOM_HPP
