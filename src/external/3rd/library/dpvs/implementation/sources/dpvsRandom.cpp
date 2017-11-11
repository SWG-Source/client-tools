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
 * Description: 	Random number generator
 *
 * $Archive: /dpvs/implementation/sources/dpvsRandom.cpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 9.05.01 17:07 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#include "dpvsRandom.hpp"
#include "dpvsDebug.hpp"		// for getTime() functionality

using namespace DPVS;

Random DPVS::g_random;			// one global random number generator shared by all classes

UINT32 Random::getI(void) 
{
	UINT32 x = m_randbuffer[m_p1] = DPVS::rotateLeft(m_randbuffer[m_p1] + m_randbuffer[m_p2], RR);

	m_p1--;
	m_p1 += (m_p1>>31)&(KK);
	m_p2--;
	m_p2 += (m_p2>>31)&(KK);

	return x;
}

float Random::get(void) 
{
	UINT32 x = m_randbuffer[m_p1] = DPVS::rotateLeft(m_randbuffer[m_p1] + m_randbuffer[m_p2], RR);

	m_p1--;
	m_p1 += (m_p1>>31)&(KK);
	m_p2--;
	m_p2 += (m_p2>>31)&(KK);


	FloatInt p;
	p.i = (x & 0x7fffff) | 0x3F800000;			// get 32-bit random number and map into a float in range [1,2[
	return p.f-1.0f;							// subtract 1 using FPU -> maps value to [0,1[
}

void Random::reset (UINT32 seed) 
{
	int i;
	if (seed==0) 
		seed--;

	for (i=0; i<KK; i++) 
	{
		seed ^= seed << 13; 
		seed ^= seed >> 17; 
		seed ^= seed << 5;
		m_randbuffer[i] = seed;
	}

	m_p1 = 0;  
	m_p2 = JJ;
	for (i=0; i<9; i++) 
		getI();
}

Random::Random (void)	
{ 
	reset (0xFFFFFFFF);
}


//------------------------------------------------------------------------
