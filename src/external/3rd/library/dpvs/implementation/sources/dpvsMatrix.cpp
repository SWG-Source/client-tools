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
 * Description: 	Matrix4 class
 *
 * $Archive: /dpvs/implementation/sources/dpvsMatrix.cpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 20.06.01 12:01 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#include "dpvsMatrix.hpp"

using namespace DPVS;

// avoid underflow in any of the components...
void Matrix4x3::flushToZero (void)			
{
	float* f = &m[0][0];
	for (int i = 0; i < 12; i++)
		Math::flushToZero(f[i]);
}

// avoid underflow in any of the components...
void Matrix4x4::flushToZero (void)			
{
	float* f = &m[0][0];
	for (int i = 0; i < 16; i++)
		Math::flushToZero(f[i]);
}


//------------------------------------------------------------------------
