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
 * Desc:	Axis-aligned bounding box routines
 *
 * $Archive: /dpvs/implementation/sources/dpvsAABB.cpp $
 * $Author: wili $ 
 * $Revision: #2 $
 * $Modtime: 8/02/01 12:36p $
 * $Date: 2003/11/18 $
 * 
 *****************************************************************************/

#include "dpvsAABB.hpp"
#include "dpvsMath.hpp"

using namespace DPVS;

/*****************************************************************************
 *
 * Function:		DPVS::validateBounds()
 *
 * Description:		Routine for ensuring that an AABB has not degenerated
 *					into 1D or 2D
 *
 * Notes:			This function is needed so that we never get "flat", i.e.
 *					2D bounding volumes. If such a case is detected, we just
 *					expand the bounds slightly
 *
 *****************************************************************************/

namespace DPVS
{
	// increments floating point value by the smallest possible unit (1 ulp)
	static inline float fixFloat (float f)
	{
		UINT32 i = Math::bitPattern(f);

		// the code below effectively subtracts one if value is negative,
		// or adds one if value is positive

		i -= ((i & 0x80000000)>>30);					// subtract two if negative
		i++;											// inc by one
		
		return *(float*)(&i);
	}
}

void AABB::validateBounds (void)
{
	//--------------------------------------------------------------------
	// Some basic assertions
	//--------------------------------------------------------------------

	DPVS_ASSERT (getMax().x >= getMin().x);
	DPVS_ASSERT (getMax().y >= getMin().y);
	DPVS_ASSERT (getMax().z >= getMin().z);

	DPVS_ASSERT (	Math::isValidFloat(getMin().x) &&
					Math::isValidFloat(getMin().y) &&
					Math::isValidFloat(getMin().z) &&
					Math::isValidFloat(getMax().x) &&
					Math::isValidFloat(getMax().y) &&
					Math::isValidFloat(getMax().z));

	//--------------------------------------------------------------------
	// We perform the fixing by modifying the floating point value slightly
	// into the correct direction. The modification is the smallest
	// possible modification we can express using 32-bit floats.
	//--------------------------------------------------------------------

	if (m_min[0] == m_max[0] && (m_min[0] < FLT_MAX)) m_max[0] = fixFloat(m_max[0]);
	if (m_min[1] == m_max[1] && (m_min[1] < FLT_MAX)) m_max[1] = fixFloat(m_max[1]);
	if (m_min[2] == m_max[2] && (m_min[2] < FLT_MAX)) m_max[2] = fixFloat(m_max[2]);
}
//------------------------------------------------------------------------
