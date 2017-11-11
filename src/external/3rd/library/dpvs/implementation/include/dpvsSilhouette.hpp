#ifndef __DPVSSILHOUETTE_HPP
#define __DPVSSILHOUETTE_HPP
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
 * Description: 	Internal Misc stuff. Will be removed to somewhere later on
 *
 * $Archive: /dpvs/implementation/include/dpvsSilhouette.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 7.05.01 12:06 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#if !defined (__DPVSARRAY_HPP)
#	include "dpvsArray.hpp"
#endif

#if !defined (__DPVSMATH_HPP)
#	include "dpvsMath.hpp"
#endif

namespace DPVS
{

class FloatRectangle;

/******************************************************************************
 *
 * Class:			DPVS::EdgeSilhouette
 *
 * Description:		Class for storing a 2D silhouette
 *
 *****************************************************************************/

class EdgeSilhouette
{
public:

	enum
	{
		LO = 0,
		HI = 1
	};
						EdgeSilhouette			(void);
						~EdgeSilhouette			(void);
	void				clear					(void)								{ m_vertexCount=0; m_edgeCount[0]=0; m_edgeCount[1]=0; m_planeCount = 0; }
	void				addEdge					(int t, int v0, int v1)				{ DPVS_ASSERT(t>=0 && t<=1); int index = m_edgeCount[t]; if (index >= m_edge[t].getSize()) resizeEdgeBuffer(t,index*2+8); m_edge[t][index].i = v0; m_edge[t][index].j = v1; m_edgeCount[t]++; }
	int					getEdgeCount			(int t) const						{ DPVS_ASSERT(t>=0 && t<=1); return m_edgeCount[t];	}
	const Vector2i*		getEdges				(int t) const						{ DPVS_ASSERT(t>=0 && t<=1); return &m_edge[t][0];		}
	const Vector2i*		getFixedPointVertices	(void) const						{ return &m_fixedVertex[0]; }
	void				getRectangle			(FloatRectangle& f) const;
	int					getVertexCount			(void) const						{ return m_vertexCount; }

	const Vector2*		getVertices				(void) const						{ return &m_vertex[0]; }
	void				setVertices				(const Vector2* loc,int n);
	void				setVertices				(const Vector3* loc,int n);
	void				setVertices				(const Vector4* loc,int n);
	void				validateFixedPoint		(const Vector2& scale)				{ Math::rasterToFixed(&m_fixedVertex[0],&m_vertex[0],scale,m_vertexCount); }

	// Z-gradient	
	int					getPlaneCount			(void) const						{ return m_planeCount; }
	const Vector3*		getPlaneVertices		(void) const						{ return m_planes; }
	void				setPlaneVertices		(int index, const Vector3& a, const Vector3& b, const Vector3& c) { DPVS_ASSERT(index>=0 && index < 6); Vector3* p = m_planes + index*3; p[0] = a; p[1] = b; p[2] = c; }
	void				setPlaneCount			(int N)								{ DPVS_ASSERT(N>=0 && N<=6); m_planeCount = N; }

private:						
						EdgeSilhouette			(const EdgeSilhouette&); // not allowed
	EdgeSilhouette&		operator=				(const EdgeSilhouette&); // not allowed
	void				resizeEdgeBuffer		(int t, int newSize);
	void				resizeVertexArray		(int newSize);

	Array<Vector2>		m_vertex;				// vertex locations
	Array<Vector2i>		m_fixedVertex;			// fixed point vertex locations
	Array<Vector2i>		m_edge[2];				// edge arrays
	int					m_vertexCount;			// number of vertices
	int					m_edgeCount[2];			// number of left/right edges
	
	Vector3				m_planes[3*6];			// vertices forming bounding planes (three vertices always form a plane)
	int					m_planeCount;			// number of bounding planes
};
} // DPVS

//------------------------------------------------------------------------
#endif //__DPVSSILHOUETTE_HPP
