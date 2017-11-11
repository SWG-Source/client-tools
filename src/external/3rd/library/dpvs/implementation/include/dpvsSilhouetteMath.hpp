#ifndef __DPVSSILHOUETTEMATH_HPP
#define __DPVSSILHOUETTEMATH_HPP
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
 * Description: 	Silhouette math routines
 *
 * $Archive: /dpvs/implementation/include/dpvsSilhouetteMath.hpp $
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

class ImpCamera;
class OBB;
class EdgeSilhouette;
class FloatRectangle;

/******************************************************************************
 *
 * Class:			DPVS::SilhouetteMath
 *
 * Description:		Static functions for constructing silhouettes out
 *					of OBBs etc.
 *
 *****************************************************************************/

class SilhouetteMath
{
public:
	// FLOW stubs
	static void computeOBBDepthRange_FLOW				();
	static void computeOBBWritePlanes_FLOW				(EdgeSilhouette& s);
	static bool computeOBBTestSilhouette_FLOW			(EdgeSilhouette &s);
	static bool getRectangle_FLOW						(FloatRectangle& r);
	static bool getClippedRectangle_FLOW				(FloatRectangle& r);

	// Actual functions (do not use VQData)
	static void computeOBBDepthRange					(Range<float>& range, const Matrix4x3& objectToCamera, const ImpCamera* cam, const OBB& obb);
	static void computeOBBWritePlanes					(EdgeSilhouette& s, const Matrix4x3& objectToCamera, ImpCamera* c, const OBB& obb);
	static bool computeOBBTestSilhouette				(EdgeSilhouette &s, Range<float>& range, const Matrix4x3& objectToCamera, ImpCamera* c, const OBB& obb);
	static bool getRectangle							(FloatRectangle& r, Range<float>& range, const Matrix4x3& objectToCamera, ImpCamera* c, const OBB& obb);
	static bool getClippedRectangle						(FloatRectangle& r, Range<float>& range, const Matrix4x3& objectToCamera, ImpCamera* c, const OBB& obb);
private:
	SilhouetteMath();	// cannot make instances

	static bool		computeFrontClippingOBBTestSilhouette	(EdgeSilhouette &s, Range<float>& range, const Matrix4x3& objectToCamera, ImpCamera* c, const OBB& obb);
	static Vector4	getCameraInUnitBoxSpace					(const OBB& obb, const Matrix4x3& objectToCamera);
};

} // DPVS

//------------------------------------------------------------------------
#endif // __DPVSSILHOUETTEMATH_HPP
