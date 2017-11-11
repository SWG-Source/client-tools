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
 * Description:		Polygon Clipper Code
 *
 * $Archive: /dpvs/implementation/sources/dpvsClipPolygon.cpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 7.05.01 12:06 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#include "dpvsClipPolygon.hpp"
#include "dpvsVector.hpp"

using namespace DPVS;

/******************************************************************************
 *
 * Function:		DPVS::clipPolygonPlane()
 *
 * Description:		Clips polygon against a plane. Does not support homogenous coordinates.
 *
 * Returns:			number of vertices in output polygon
 *
 ******************************************************************************/

int DPVS::clipPolygonPlane (Vector3* clipped, const Vector3* polygon, const Vector4& plEq, int N)
{
	DPVS_ASSERT(N<=32 && clipped && polygon && N>=0);

	float	clipVal[32];
	int		i;
	bool	allClipped = true;

	for (i=0; i<N; i++)
	{
		clipVal[i] = -(polygon[i].x*plEq.x + polygon[i].y*plEq.y + polygon[i].z*plEq.z + plEq.w);	
		if (clipVal[i] < 0.0f)	// should this be <= ?
			allClipped = false;
	}

	if (allClipped)
		return 0;

	bool	nclip = (clipVal[0] > 0.f);
	int		cn = 0;

	for(i=0;i<N;i++)
	{
		int		cs, cd;
		int		j = i+1;

		if(j == N)
			j = 0;

		bool tclip = nclip;

		nclip = (clipVal[j] > 0.f);	

		if(tclip)
		{
			if(nclip)
				continue;

			cs = i;
			cd = j;
		}
		else
		{
		 	clipped[cn++] = polygon[i];				// copy vertex and inc output counter

			if(!nclip)
				continue;

			cs = j;
			cd = i;
		}

//		double dv = clipVal[cs]/(clipVal[cs]-clipVal[cd]);
		float dv = clipVal[cs]/(clipVal[cs]-clipVal[cd]);

		clipped[cn].x = (polygon[cs].x + (polygon[cd].x - polygon[cs].x) * dv);
		clipped[cn].y = (polygon[cs].y + (polygon[cd].y - polygon[cs].y) * dv);
		clipped[cn].z = (polygon[cs].z + (polygon[cd].z - polygon[cs].z) * dv);
		cn++;
		DPVS_ASSERT(cn<=32);
	}

	return cn;			// number of vertices in output polygon
}

/******************************************************************************
 *
 * Function:		DPVS::clipHomogenousFrontPlane()
 *
 * Description:		Clips triangle with vertices expressed as homogenous coordinates
 *					(i.e. after projection matrix has been applied).
 *
 * Returns:			number of vertices in output polygon
 *
 * Notes:			may produce up to 4 output vertices
 *
 ******************************************************************************/

int DPVS::clipHomogenousFrontPlane (Vector4* dst, const Vector4* src)
{
	float clipVal[3];

	clipVal[0] = -(src[0].z + src[0].w);
	clipVal[1] = -(src[1].z + src[1].w);
	clipVal[2] = -(src[2].z + src[2].w);

	bool	nclip	= clipVal[0] > 0.f;		// previous clipval
	int		cn		= 0;

	for( int i = 0; i < 3; i++)
	{
		int		j		= i+1;
		if (j == 3)
			j = 0;

		bool	tclip	= nclip;
		int		cs		= i;
		int		cd		= j;

		nclip = clipVal[j] > 0.f;	// DEBUG DEBUG >= 0.0f

		if(tclip)
		{
			if (nclip)
				continue;
		}
		else
		{
			dst[cn++] = src[i];

			if (!nclip)
				continue;

			swap(cs,cd);
		}

		float dv = clipVal[cs]/(clipVal[cs]-clipVal[cd]);

		dst[cn].x = src[cs].x + (src[cd].x - src[cs].x) * dv;
		dst[cn].y = src[cs].y + (src[cd].y - src[cs].y) * dv;
		dst[cn].w = src[cs].w + (src[cd].w - src[cs].w) * dv;
		dst[cn].z = -dst[cn].w;	// set absolutely that z = -w
		cn++;
	}

#if defined (DPVS_DEBUG)
	DPVS_ASSERT(cn>= 0 && cn<=4);	// HUHH?
	for (int i = 0; i < cn; i++)
		DPVS_ASSERT(dst[i].z >= -dst[i].w);
#endif
	return cn;
}

//------------------------------------------------------------------------
