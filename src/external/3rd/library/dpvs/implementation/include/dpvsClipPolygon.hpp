#ifndef __DPVSCLIPPOLYGON_HPP
#define __DPVSCLIPPOLYGON_HPP
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
 * Description: 	Polygon Clipper
 *
 * $Archive: /dpvs/implementation/include/dpvsClipPolygon.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 7.05.01 12:06 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#if !defined (__DPVSPRIVATEDEFS_HPP)
#	include "dpvsPrivateDefs.hpp"
#endif

namespace DPVS
{
	// routine for clipping triangle against a plane
	int clipPolygonPlane			(Vector3* dst, const Vector3* src, const Vector4& plEq, int nVertices);
	int clipHomogenousFrontPlane	(Vector4* dst, const Vector4* src);

} // DPVS
//------------------------------------------------------------------------
#endif // __DPVSCLIPPOLYGON_HPP
