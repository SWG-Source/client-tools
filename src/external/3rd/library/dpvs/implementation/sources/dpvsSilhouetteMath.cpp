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
 * $Archive: /dpvs/implementation/sources/dpvsSilhouetteMath.cpp $
 * $Author: wili $
 * $Revision: #1 $
 * $Modtime: 9.11.01 18:43 $
 * $Date: 2003/01/22 $
 *
 ******************************************************************************/

#include "dpvsSilhouetteMath.hpp"
#include "dpvsImpCamera.hpp"
#include "dpvsSilhouette.hpp"
#include "dpvsRange.hpp"
#include "dpvsOBB.hpp"
#include "dpvsMath.hpp"
#include "dpvsClipPolygon.hpp"
#include "dpvsRectangle.hpp"
#include "dpvsStatistics.hpp"
#include "dpvsVQData.hpp"

using namespace DPVS;

/*****************************************************************************
 *
 * Function:		SilhouetteMath::getCameraInUnitBoxSpace()
 *
 * Description:		Returns camera homogenous coordinate in unit-box space
 *
 * Parameters:		obb				= reference to OBB
 *					objectToCamera	= object->camera matrix
 *
 * Returns:			homogenous camera coordinate
 *
 *****************************************************************************/

Vector4 SilhouetteMath::getCameraInUnitBoxSpace (const OBB& obb, const Matrix4x3& objectToCamera)
{
	if (VQData::get().isOrthoProjection())
	{
		Matrix4x3 m(NO_CONSTRUCTOR);
		Math::productFromLeft	(m,obb.getMatrix(), objectToCamera);
		Math::invertMatrix		(m);

		return Vector4(-m.getDof(),0.0f);
	}

	return Vector4(Math::getInverseTranslation(obb.getMatrix(), objectToCamera),1.0f);
}


/*****************************************************************************
 *
 * Function:		SilhouetteMath::computeOBBDepthRange()
 *
 * Description:		Calculates far and near depth values from an OBB
 *
 * Parameters:		range			= output range in [0,1] range (NOT CLAMPED!)
 *					objectToCamera	= object->camera matrix
 *					cam				= pointer to camera
 *					ob				= reference to OBB
 *
 *****************************************************************************/

void SilhouetteMath::computeOBBDepthRange (Range<float>& range, const Matrix4x3& objectToCamera, const ImpCamera* cam, const OBB& obb)
{
	const Matrix4x3& a = obb.getMatrix();
	const Matrix4x3& b = objectToCamera;
	const Matrix4x4& c = cam->getCameraToScreen();

	//------------------------------------------------------------------------
	// Perform (a*b)*c
	//------------------------------------------------------------------------

	float m20,m21,m22,m23;
	float m30,m31,m32,m33;

	if (Math::isZero(c[3][3]))		// special case for perspective projection
	{
		DPVS_ASSERT(Math::isPerspectiveProjectionMatrix(c));

		m30 = b[2][0]*a[0][0] + b[2][1]*a[1][0] + b[2][2]*a[2][0];
		m31 = b[2][0]*a[0][1] + b[2][1]*a[1][1] + b[2][2]*a[2][1];
		m32 = b[2][0]*a[0][2] + b[2][1]*a[1][2] + b[2][2]*a[2][2];
		m33 = b[2][0]*a[0][3] + b[2][1]*a[1][3] + b[2][2]*a[2][3] + b[2][3];
		m20 = c[2][2]*m30;
		m21 = c[2][2]*m31;
		m22 = c[2][2]*m32;
		m23 = c[2][2]*m33 + c[2][3];
	} else
	{
		Matrix4x4 d(NO_CONSTRUCTOR);
		Math::productFromLeft(d,a,b);
		d.productFromLeft(c);

		m20 = d[2][0];
		m21 = d[2][1];
		m22 = d[2][2];
		m23 = d[2][3];
		m30 = d[3][0];
		m31 = d[3][1];
		m32 = d[3][2];
		m33 = d[3][3];
	}

	//------------------------------------------------------------------------
	// Ok, now we have the interesting parts of the matrix computed. Then loop
	// through the eight possible 'far' values and find the farthest one.
	//------------------------------------------------------------------------

#define TEST_CORNER(A,B,C) w = Math::fabs(m33 A m30 B m31 C m32); z = m23 A m20 B m21 C m22; z = (w!=0.0f) ? z/w : -1.0f; maxZ = Math::max(z, maxZ); minZ = Math::min(z, minZ);

	float maxZ = -FLT_MAX;
	float minZ = +FLT_MAX;
	float w,z;

	TEST_CORNER(-,-,-);
	TEST_CORNER(-,-,+);
	TEST_CORNER(-,+,-);
	TEST_CORNER(-,+,+);
	TEST_CORNER(+,-,-);
	TEST_CORNER(+,-,+);
	TEST_CORNER(+,+,-);
	TEST_CORNER(+,+,+);

#undef TEST_CORNER

	range.set (minZ*0.5f + 0.5f, maxZ*0.5f + 0.5f);
}

/*****************************************************************************
 *
 * Function:		SilhouetteMath::computeOBBWritePlanes()
 *
 * Description:		Finds bounding planes of an OBB (WRITE PLANES!!!)
 *
 * Parameters:		s				= output silhouette (where the planes are stored)
 *					objectToCamera	= object->camera transformation matrix
 *					c				= pointer to camera
 *					obb				= reference to OBB
 *
 *****************************************************************************/

void SilhouetteMath::computeOBBWritePlanes (EdgeSilhouette& s, const Matrix4x3& objectToCamera, ImpCamera* c, const OBB& obb)
{
	DPVS_ASSERT (c);

	//--------------------------------------------------------------------
	// Find out which planes should be selected (i.e. select all back-
	// facing planes)
	//--------------------------------------------------------------------

	Vector4			camLoc		= getCameraInUnitBoxSpace(obb,objectToCamera);
	unsigned int	faceMask	= 0;

	if (camLoc.x >= -camLoc.w) faceMask |= 1;
	if (camLoc.y >= -camLoc.w) faceMask |= 2;
	if (camLoc.z >= -camLoc.w) faceMask |= 4;
	if (camLoc.x <= +camLoc.w) faceMask |= 8;
	if (camLoc.y <= +camLoc.w) faceMask |= 16;
	if (camLoc.z <= +camLoc.w) faceMask |= 32;

	//--------------------------------------------------------------------
	// Lookup table used for indicating which vertices form each of the
	// six faces..
	//--------------------------------------------------------------------

	static const int pv[6][3] =
	{
		{0,2,6},
		{1,0,4},
		{0,1,3},
		{3,1,5},
		{2,3,7},
		{5,4,6}
	};

	//--------------------------------------------------------------------
	// Construct transformation matrix (OBB -> screen)
	//--------------------------------------------------------------------

	Matrix4x4				m(NO_CONSTRUCTOR);						// no constructors are called
	float					fv[(8+7)*3];
	Vector3*				v = (Vector3*)(fv);

	Math::unitBoxToScreen(m, obb.getMatrix(), objectToCamera,c->getCameraToScreen());

	//--------------------------------------------------------------------
	// Transform eight vertices of the box to into raster-space. If the
	// box is front-clipping we need to run a separate routine that can
	// calculate gradients for front-clipping OBBs
	//--------------------------------------------------------------------

	if (!Math::transformUnitBoxXYZDivByW (v, m))
	{
		//--------------------------------------------------------------------
		// Let's handle the front-clipping case. The idea here is that
		// we select all the back faces, run them through a clipper, then
		// feed the clipper's output to the silhouette
		//--------------------------------------------------------------------
		Math::transformOBBVertices (v,obb.getMatrix(),objectToCamera);

		int						planeCount  = 0;
		Vector3*				inVertices  = v+8;
		Vector3*				outVertices = inVertices+3;

		for (int i = 0; i < 6; i++)
		if (faceMask & (1<<i))										// if back-facing plane
		{
			inVertices[0] = v[pv[i][0]];							// setup input triangle
			inVertices[1] = v[pv[i][1]];
			inVertices[2] = v[pv[i][2]];

			int N = clipPolygonPlane (outVertices, inVertices, c->getClipPlane(ImpCamera::FRONT), 3);
			DPVS_ASSERT( N >= 0 && N <= 4);	 // WASS?

			if (N >= 3)
			{
				// Transform first three output vertices into raster-space, then assign those
				// to the silhouette
				Math::objectToRaster (outVertices,outVertices,3,c->getCameraToScreen(),c->getRasterViewport());
				s.setPlaneVertices(planeCount++,outVertices[0],outVertices[1],outVertices[2]);
			}
		}
		s.setPlaneCount(planeCount);								// set output plane counbt
	} else // not front-clipping case..
	{
		Math::screenToRaster (v, 8, c->getRasterViewport());				// screen->raster transformation
		int planeCount = 0;
		for (int i = 0; i < 6; i++)
		if (faceMask & (1<<i))
			s.setPlaneVertices(planeCount++,v[pv[i][0]],v[pv[i][1]],v[pv[i][2]]);
		s.setPlaneCount(planeCount);
	}
}

/*****************************************************************************
 *
 * Function:		SilhouetteMath::computeFrontClippingOBBTestSilhouette()
 *
 * Description:		Generates testing silhouette from an OBB when the OBB
 *					is front-clipping
 *
 * Parameters:		s				= reference to silhouette
 *					depthRange		= output depth range of the silhouette
 *					objectToCamera	= object->camera transformation matrix
 *					c				= pointer to camera (DEBUG DEBUG MAKE IT CONST?)
 *					obb				= reference to OBB
 *
 * Returns:			true if silhouette can be used, false otherwise
 *
 * Notes:			This is an internal function, only called by
 *					combuteOBBTestSilhouette()
 *
 *****************************************************************************/

bool SilhouetteMath::computeFrontClippingOBBTestSilhouette (EdgeSilhouette &s, Range<float>& depthRange, const Matrix4x3& objectToCamera, ImpCamera* c, const OBB& obb)
{
	DPVS_PROFILE(Statistics::incStatistic(Library::STAT_MODELTESTSILHOUETTESCLIPPED,1));

	//--------------------------------------------------------------------
	// Find out which planes should be selected (i.e. select all front-
	// facing planes).
	//--------------------------------------------------------------------

	Vector4			camLoc		= getCameraInUnitBoxSpace(obb,objectToCamera);
	unsigned int	faceMask	= 0;

	if (camLoc.x <= -camLoc.w) faceMask |= 1;
	if (camLoc.x >= +camLoc.w) faceMask |= 8;
	if (camLoc.y <= -camLoc.w) faceMask |= 2;
	if (camLoc.y >= +camLoc.w) faceMask |= 16;
	if (camLoc.z <= -camLoc.w) faceMask |= 4;
	if (camLoc.z >= +camLoc.w) faceMask |= 32;

	if (!faceMask)												// camera is inside the OBB!
		return false;

	//--------------------------------------------------------------------
	// Lookup table used for indicating which vertices form each of the
	// six faces..
	//--------------------------------------------------------------------

	static const int pv[6][4] =
	{
		{0,2,6,4},
		{1,0,4,5},
		{1,3,2,0},
		{1,5,7,3},
		{2,3,7,6},
		{6,7,5,4}
	};

	// DEBUG DEBUG USING FLOAT->VECTOR3 CAST TRICK BECAUSE OF STUPID STUPID
	// GCC 2.95 BUG...

	float					fv[(4*8+64)*3];
	Vector3*				v = (Vector3*)(fv);
	Vector3*				outVertices = &v[8];
	int						nOutVertices	= 0;
	int						planeCount		= 0;

	Math::transformOBBVertices (v,obb.getMatrix(),objectToCamera);

	for (int i = 0; i < 6; i++)									// evaluate through all of the planes
	if (faceMask & (1<<i))
	{
		//----------------------------------------------------------------
		// Clip the face against left/right/bottom/top planes.
		//----------------------------------------------------------------
		Vector3*				src = &v[4*8];
		Vector3*				dst = &v[4*8+32];

		src[0] = v[pv[i][0]];									//  input vertices
		src[1] = v[pv[i][1]];
		src[2] = v[pv[i][2]];
		src[3] = v[pv[i][3]];

		int vertices = 4;

		for (int plane = (int)(ImpCamera::LEFT); plane <= (int)(ImpCamera::BOTTOM); plane++)
		{
			vertices = clipPolygonPlane(dst,src,c->getClipPlane(plane),vertices);
			if (!vertices)										// face not on screen
				break;
			swap(dst,src);
		}

		if (!vertices)
			continue;

		DPVS_ASSERT(vertices >= 3 && vertices <= 8);

		//----------------------------------------------------------------
		// Now let's see if the face is still front-clipping. If it is,
		// we cannot create a valid test silhouette (and return thus 'false').
		//----------------------------------------------------------------

		const Vector4& plEq = c->getClipPlane(ImpCamera::FRONT);					// query front plane
		for (int j = 0; j < vertices; j++)
		if ((plEq.x*src[j].x+plEq.y*src[j].y+plEq.z*src[j].z+plEq.w) < 0.0f)	// quad is front-clipping
			return false;

		//----------------------------------------------------------------
		// Transform vertices into raster-space. Then setup the Z-rasterization.
		//----------------------------------------------------------------

		Vector3* out = outVertices + nOutVertices;

		Math::objectToRaster	(out,src,vertices,c->getCameraToScreen(),c->getRasterViewport());
		s.setPlaneVertices		(planeCount++,out[0],out[1],out[2]);

		//----------------------------------------------------------------
		// Copy edges to the silhouette.
		//----------------------------------------------------------------

		for (int j = 0; j < vertices; j++)
		{
			int A = nOutVertices+j;
			int B = nOutVertices+j+1;
			if (B == (nOutVertices+vertices))
				B = nOutVertices;
			int right = (outVertices[A].y > outVertices[B].y) ? 1 : 0;

			s.addEdge (right,A,B);
		}

		nOutVertices += vertices;
	}

	if (!planeCount)	// DEBUG DEBUG should we somehow indicate that the silhouette is hidden!?
		return false;

	DPVS_ASSERT(nOutVertices <= 24);
	DPVS_ASSERT(planeCount   <= 3);

	//--------------------------------------------------------------------
	// Update the 'depthRange' by computing minimum and maximum values
	// from the vertices of the silhouette.
	//--------------------------------------------------------------------

	float mn = outVertices[0].z;
	float mx = outVertices[0].z;

	for (int i = 1; i < nOutVertices; i++)
	{
		mn = Math::min(outVertices[i].z,mn);
		mx = Math::max(outVertices[i].z,mx);
	}

	DPVS_ASSERT(mn>=0.0f);

	if (mn >= 1.0f)												// we should've rejected this OBB earlier
		return false;

	mx = Math::min(mx,1.0f);									// clamp max to 1.0
	depthRange.set(mn,mx);										// set new depth range

	//--------------------------------------------------------------------
	// Update the output silhouette structure
	//--------------------------------------------------------------------

	s.setVertices	(outVertices,nOutVertices);					// copy vertices
	s.setPlaneCount	(planeCount);								// set output plane counbt

	return true;
}

/*****************************************************************************
 *
 * Function:		SilhouetteMath::computeOBBTestSilhouette()
 *
 * Description:		Generates testing silhouette from an OBB
 *
 * Parameters:		s				= reference to silhouette
 *					depthRange		= output depth range of the silhouette
 *					objectToCamera	= object->camera transformation matrix
 *					c				= pointer to camera (DEBUG DEBUG MAKE IT CONST?)
 *					obb				= reference to OBB
 *
 * Returns:			true if silhouette can be used, false otherwise
 *
 *****************************************************************************/

bool SilhouetteMath::computeOBBTestSilhouette (EdgeSilhouette &s, Range<float>& depthRange, const Matrix4x3& objectToCamera, ImpCamera* c, const OBB& obb)
{
	DPVS_PROFILE(Statistics::incStatistic(Library::STAT_MODELTESTSILHOUETTESQUERIED,1));

	//--------------------------------------------------------------------
	// Find depth range. If OBB is front-clipping, then call separate
	// (slower) routine that handles front-clipping OBBs.
	//--------------------------------------------------------------------

	computeOBBDepthRange (depthRange, objectToCamera,c,obb);	// always sets

	if (depthRange.getMin() <= 0.0f)							// front-clipping (cannot use silhouette)
	{
		bool accept = computeFrontClippingOBBTestSilhouette(s,depthRange,objectToCamera,c,obb);
		if (!accept)
		{
			DPVS_PROFILE(Statistics::incStatistic(Library::STAT_MODELTESTSILHOUETTESREJECTED,1));
		}
		return accept;
	}

	if (depthRange.getMin() > 1.0f)								//
		depthRange.setMin(1.0f);								// DEBUG DEBUG SHOULD WE JUST DISCARD THIS?
	if (depthRange.getMax() > 1.0f)								
		depthRange.setMax(1.0f);

	//--------------------------------------------------------------------
	// Construct transformation matrix (OBB -> screen)
	//--------------------------------------------------------------------

	RawVector<Vector3,8>		v;
	Matrix4x4					m(NO_CONSTRUCTOR);

	Math::unitBoxToScreen(m, obb.getMatrix(), objectToCamera,c->getCameraToScreen());

	//--------------------------------------------------------------------
	// Transform eight vertices of the box
	//--------------------------------------------------------------------

	if (!Math::transformUnitBoxXYZDivByW (v, m))					// THIS SHOULD NOT HAPPEN!
		DPVS_ASSERT(false);

	Math::screenToRaster			(v, 8, c->getRasterViewport());	// screen->raster xform

	s.setVertices(v,8);												// copy 8 vertices into output silhouette

	//--------------------------------------------------------------------
	// Find out which edges are visible
	//--------------------------------------------------------------------

	enum							// twelve edges
	{
		E_0_1	= (1<<0),
		E_1_3	= (1<<1),
		E_3_2	= (1<<2),
		E_2_0	= (1<<3),
		E_4_5	= (1<<4),
		E_5_7	= (1<<5),
		E_7_6	= (1<<6),
		E_6_4	= (1<<7),
		E_0_4	= (1<<8),
		E_1_5	= (1<<9),
		E_2_6	= (1<<10),
		E_3_7	= (1<<11)
	};

	static const int			g_edges[12][2] = {{0,1},{1,3},{2,3},{0,2},{4,5},{5,7},{6,7},{4,6},{0,4},{1,5},{2,6},{3,7}};
	static const unsigned int	g_faceXorMask[6] =
	{
		(E_0_1|E_1_3|E_3_2|E_2_0),
		(E_4_5|E_5_7|E_7_6|E_6_4),
		(E_3_2|E_3_7|E_7_6|E_2_6),
		(E_0_1|E_1_5|E_4_5|E_0_4),
		(E_1_3|E_1_5|E_5_7|E_3_7),
		(E_2_0|E_0_4|E_6_4|E_2_6)
	};

	static const unsigned int	g_faceFlipMask[6] =
	{
		E_3_2|E_2_0,
		E_4_5|E_5_7,
		E_7_6|E_2_6,
		E_0_1|E_1_5,
		E_3_7|E_1_3,
		E_6_4|E_0_4
	};

	//--------------------------------------------------------------------
	// Setup front-facing planes (these are used by the occlusion culling
	// routine to determine depth values for pixels).
	//--------------------------------------------------------------------

	Vector4			camLoc		= getCameraInUnitBoxSpace(obb, objectToCamera);

	int				planeCount	= 0;
	unsigned int	edgeMask	= 0;		// which of the 12 edges are valid
	unsigned int	flipMask	= 0;

	if (camLoc.z <= -camLoc.w) { s.setPlaneVertices(planeCount++,v[0],v[1],v[3]); edgeMask ^= g_faceXorMask[0]; flipMask |= g_faceFlipMask[0]; }
	if (camLoc.z >= +camLoc.w) { s.setPlaneVertices(planeCount++,v[5],v[4],v[6]); edgeMask ^= g_faceXorMask[1]; flipMask |= g_faceFlipMask[1]; }
	if (camLoc.y >= +camLoc.w) { s.setPlaneVertices(planeCount++,v[2],v[3],v[7]); edgeMask ^= g_faceXorMask[2]; flipMask |= g_faceFlipMask[2]; }
	if (camLoc.y <= -camLoc.w) { s.setPlaneVertices(planeCount++,v[1],v[0],v[4]); edgeMask ^= g_faceXorMask[3]; flipMask |= g_faceFlipMask[3]; }
	if (camLoc.x >= +camLoc.w) { s.setPlaneVertices(planeCount++,v[3],v[1],v[5]); edgeMask ^= g_faceXorMask[4]; flipMask |= g_faceFlipMask[4]; }
	if (camLoc.x <= -camLoc.w) { s.setPlaneVertices(planeCount++,v[0],v[2],v[6]); edgeMask ^= g_faceXorMask[5]; flipMask |= g_faceFlipMask[5]; }

	s.setPlaneCount(planeCount);

	//--------------------------------------------------------------------
	// Evaluate through active _silhouette_ edges and add them to the
	// output silhouette.
	//--------------------------------------------------------------------

	int i;
	for (i = 0; i < 12; i++)
	if (edgeMask & (1<<i))
	{
		int A = g_edges[i][0];
		int B = g_edges[i][1];

		if (flipMask & (1<<i))					// we want edge to run this way
			swap(A,B);

		int right = (v[A].y < v[B].y) ? 1 : 0;

		s.addEdge (right,A,B);
	}

	return true;								// indicate success
}

/*****************************************************************************
 *
 * Function:		SilhouetteMath::getClippedRectangle()
 *
 * Description:		Calculates raster-space rectangle from an OBB (when it is clipped into the view frustum)
 *
 * Parameters:		r				= reference to output rectangle structure
 *					range			= min-max depth values
 *					objectToCamera	= object->camera transformation matrix
 *					cam				= pointer to camera
 *					obb				= reference to OBB
 *
 * Returns:			true if rectangle is valid, false otherwise (false == OBB completely outside screen)
 *
 * Notes:			The routine is somewhat unoptimized (we could do faster
 *					processing by clipmask computation).
 *
 *****************************************************************************/

bool SilhouetteMath::getClippedRectangle (FloatRectangle& r, Range<float>& range, const Matrix4x3 &objectToCamera, ImpCamera* c, const OBB& obb)
{
	//--------------------------------------------------------------------
	// Lookup table used for indicating which vertices form each of the
	// six faces..
	//--------------------------------------------------------------------

	static const int pv[6][4] =
	{
		{0,2,6,4},
		{1,0,4,5},
		{1,3,2,0},
		{1,5,7,3},
		{2,3,7,6},
		{6,7,5,4}
	};

	//----------------------------------------------------------------
	// Handle case where camera is inside the OBB
	//----------------------------------------------------------------

	//----------------------------------------------------------------
	// First transform vertices of the OBB into camera-space
	//----------------------------------------------------------------

	RawVector<Vector3,8>	v;
	Math::transformOBBVertices (v,obb.getMatrix(),objectToCamera);


	{

		Vector4			camLoc		= getCameraInUnitBoxSpace(obb,objectToCamera);
		unsigned int	faceMask	= 0;

		if (camLoc.x < -camLoc.w) faceMask |= 1;
		if (camLoc.x > +camLoc.w) faceMask |= 8;
		if (camLoc.y < -camLoc.w) faceMask |= 2;
		if (camLoc.y > +camLoc.w) faceMask |= 16;
		if (camLoc.z < -camLoc.w) faceMask |= 4;
		if (camLoc.z > +camLoc.w) faceMask |= 32;

		// if camera is inside the OBB -> generate full-screen test rectangle
		if (!faceMask)						
		{
			r = c->getRasterViewport();
			r.setSpace(SPACE_RASTER);
			float farZ = -1.0f;
			for (int i = 0; i < 8; i++)
			{
				Vector4 vec = c->getCameraToScreen().transform(v[i]);
				if (vec.z > -vec.w)
				{
					float z = vec.z / vec.w;
					if (z > farZ)
						farZ = z;
				}
			}
			if (farZ > 1.0f) farZ = 1.0f;
			range.set(0.0f, farZ * 0.5f + 0.5f);
			return true;
		}
	}


	//----------------------------------------------------------------
	// Initialize min/max values
	//----------------------------------------------------------------

	float mxx = c->getRasterViewport().x0;
	float mxy = c->getRasterViewport().y0;
	float mxz = 0.0f;
	float mnx = c->getRasterViewport().x1;
	float mny = c->getRasterViewport().y1;
	float mnz = 1.0f;

	//----------------------------------------------------------------
	// Loop through each face
	//----------------------------------------------------------------

	bool valid = false;

	// DEBUG DEBUG OPTIMIZE: we could perform here some optimizations
	// by calculating first clipmasks and then later skipping quads
	// where all vertices have already been checked...

	for (int i = 0; i < 6; i++)									// evaluate through all of the faces
	{
		//----------------------------------------------------------------
		// Clip the face against left/right/bottom/top planes.
		//----------------------------------------------------------------

		RawVector<Vector3,2*10> fv3;							// 20 = max # of vertices we can have
		Vector3*				src = &fv3[0];
		Vector3*				dst = &fv3[10];

		src[0] = v[pv[i][0]];									//  input vertices
		src[1] = v[pv[i][1]];
		src[2] = v[pv[i][2]];
		src[3] = v[pv[i][3]];

		int vertices = 4;

		for (int plane = (int)(ImpCamera::BACK); plane <= (int)(ImpCamera::BOTTOM); plane++)
		{
			vertices = clipPolygonPlane(dst,src,c->getClipPlane(plane),vertices);
			if (!vertices)										// face not on screen
				break;
			swap(dst,src);
		}

		if (!vertices)
			continue;

		valid = true;

		//----------------------------------------------------------------
		// Now let's see if the face is still front-clipping. If it is,
		// we cannot create a valid test silhouette and must thus generate
		// a rectangle that has the size of the viewport...
		//----------------------------------------------------------------

		const Vector4& plEq = c->getClipPlane(ImpCamera::FRONT);					// query front plane

		// DEBUG DEBUG TODO: IS THIS VALID FOR ORTHO-PROJECTION???
		for (int j = 0; j < vertices; j++)
		if ((plEq.x*src[j].x+plEq.y*src[j].y+plEq.z*src[j].z+plEq.w) < 0.0f)	// quad is front-clipping
		{
			r = c->getRasterViewport();
			r.setSpace(SPACE_RASTER);

			computeOBBDepthRange (range,objectToCamera,c,obb);
			range.clampToUnit();												// clamp min/max to [0..1]
			if (range.getMax() < 0.0f)
				range.setMax(0.0f);
			if (range.getMin() > range.getMax())
				range.setMin(range.getMax());
			return true;
		}

		DPVS_ASSERT(vertices>=3 && vertices <= 10);

		Math::objectToRaster (src,src,vertices,c->getCameraToScreen(),c->getRasterViewport());

		for (int j = 0; j < vertices; j++)
		{
			float x = src[j].x, y = src[j].y, z = src[j].z;
			mnx = Math::min(x,mnx);
			mny = Math::min(y,mny);
			mnz = Math::min(z,mnz);
			mxx = Math::max(x,mxx);
			mxy = Math::max(y,mxy);
			mxz = Math::max(z,mxz);
		}
	}

	if (!valid)
		return false;				// camera is not inside the OBB and OBB is outside the screen

	

	mnz = Math::max(mnz, 0.0f);		// dispel floating point inaccuracies
	mxz = Math::min(mxz, 1.0f);		// dispel floating point inaccuracies
	if (mnz > mxz)					// not visible..
		return false;

	//----------------------------------------------------------------
	// Copy min/max values
	//----------------------------------------------------------------

	r.x0 = mnx;
	r.y0 = mny;
	r.x1 = mxx+1.0f;	// exclusivity
	r.y1 = mxy+1.0f;	// exclusivity

	r.setSpace(SPACE_RASTER);

	range.set(mnz, mxz);
	return true;
}

/*****************************************************************************
 *
 * Function:		SilhouetteMath::getRectangle()
 *
 * Description:		Calculates raster-space rectangle from an OBB
 *
 * Parameters:		r				= reference to output rectangle structure
 *					range			= min-max depth values
 *					objectToCamera	= object->camera transformation matrix
 *					cam				= pointer to camera
 *					obb				= reference to OBB
 *					frontClipping	= reference to bool where front clipping status is stored
 *
 * Returns:			true if rectangle is valid, false otherwise
 *
 *****************************************************************************/

bool SilhouetteMath::getRectangle (FloatRectangle& r, Range<float>& range, const Matrix4x3 &objectToCamera, ImpCamera *cam, const OBB& obb)
{
	//--------------------------------------------------------------------
	// Compute unit box->screen matrix
	//--------------------------------------------------------------------

	Matrix4x4				m(NO_CONSTRUCTOR);
	RawVector<Vector3,8>	dst;
	const FloatRectangle&	v	= cam->getRasterViewport();

	Math::unitBoxToScreen (m, obb.getMatrix(), objectToCamera,cam->getCameraToScreen());

	//--------------------------------------------------------------------
	// Transform and project box corners. If transformation fails
	// (i.e. OBB is front-clipping), find out near and far Z values
	// differently (using a custom clipping routine)
	//--------------------------------------------------------------------

	bool frontClip = !Math::transformUnitBoxXYZDivByW(dst,m);

	if (!frontClip)
	{
		// DEBUG DEBUG OPTIMIZE: or values together as unsigned ints, check largest then
		// (negative values have highest bit set)
		for (int i = 0; i < 8; i++)		// check if any of the vertices are clipping
		if (dst[i].z <= -1.0f)
		{
			frontClip = true;
			break;
		}
	}

	if (frontClip)
		return getClippedRectangle (r,range,objectToCamera,cam,obb);

	//--------------------------------------------------------------------
	// Find min-max of the extents and store them to 'range'
	//--------------------------------------------------------------------

	Vector3 topLeft;
	Vector3 bottomRight;

	Math::minMax (topLeft,bottomRight,dst,8);		// find minmax

	if (topLeft.z >= 1.0f)							// behind back-clipping plane
		return false;
	if (topLeft.z <= 0.0f)							// this is an error, but let's live with it...
		topLeft.z = 0.0f;

	bottomRight.z = Math::min(bottomRight.z,1.0f);	// clamp to back-clipping plane
	range.set(topLeft.z * 0.5f + 0.5f, bottomRight.z * 0.5f + 0.5f);

	//--------------------------------------------------------------------
	// Transform rectangle into raster-space.
	//--------------------------------------------------------------------

	float xscale	=  0.5f * v.width();
	float xofs		=  0.5f * v.width()  + v.x0;
	float yscale	= -0.5f * v.height();
	float yofs		=  0.5f * v.height() + v.y0;

	r.x0 = topLeft.x     * xscale + xofs;
	r.x1 = bottomRight.x * xscale + xofs + 1.0f;	// make exclusive
	r.y0 = bottomRight.y * yscale + yofs;			// here flip because yscale is negative
	r.y1 = topLeft.y	 * yscale + yofs + 1.0f;	// make exclusive

	r.setSpace(SPACE_RASTER);

	return true;									// indicate valid rectangle
}


//------------------------------------------------------------------------
// Stubs for FLOW functions 
//------------------------------------------------------------------------

void SilhouetteMath::computeOBBWritePlanes_FLOW		(EdgeSilhouette& s)
{
	DPVS_ASSERT(VQData::get().testProperties(VQData::FLOW_ACTIVE));
	ImpObject* o = VQData::get().getObject();
	ImpCamera* c = VQData::get().getCamera();
	computeOBBWritePlanes(s,o->getObjectToCameraMatrix(c),c,o->getModelOBB());
}

void SilhouetteMath::computeOBBDepthRange_FLOW		()
{
	DPVS_ASSERT(VQData::get().testProperties(VQData::FLOW_ACTIVE));
	ImpObject* o = VQData::get().getObject();
	ImpCamera* c = VQData::get().getCamera();
	VQData::get().setProperties(VQData::DEPTH_RANGE_VALID,true);		// ALWAYS
	computeOBBDepthRange(VQData::get().accessDepthRange(), o->getObjectToCameraMatrix(c), c, o->getModelOBB());
}

bool SilhouetteMath::computeOBBTestSilhouette_FLOW	(EdgeSilhouette &s)
{
	DPVS_ASSERT(VQData::get().testProperties(VQData::FLOW_ACTIVE));
	ImpObject* o = VQData::get().getObject();
	ImpCamera* c = VQData::get().getCamera();
	VQData::get().setProperties(VQData::DEPTH_RANGE_VALID,true);		// ALWAYS
	return computeOBBTestSilhouette(s, VQData::get().accessDepthRange(), o->getObjectToCameraMatrix(c), c, o->getModelOBB());
}

bool SilhouetteMath::getRectangle_FLOW				(FloatRectangle& r)
{
	DPVS_ASSERT(VQData::get().testProperties(VQData::FLOW_ACTIVE));
	ImpObject* o = VQData::get().getObject();
	ImpCamera* c = VQData::get().getCamera();
	VQData::get().setProperties(VQData::DEPTH_RANGE_VALID,true);		// if valid
	return getRectangle (r, VQData::get().accessDepthRange(), o->getObjectToCameraMatrix(c), c, o->getModelOBB());
}

bool SilhouetteMath::getClippedRectangle_FLOW		(FloatRectangle& r)
{
	DPVS_ASSERT(VQData::get().testProperties(VQData::FLOW_ACTIVE));
	ImpObject* o = VQData::get().getObject();
	ImpCamera* c = VQData::get().getCamera();
	VQData::get().setProperties(VQData::DEPTH_RANGE_VALID,true);		// if valid
	return getClippedRectangle(r, VQData::get().accessDepthRange(), o->getObjectToCameraMatrix(c), c, o->getModelOBB());
}

//------------------------------------------------------------------------
