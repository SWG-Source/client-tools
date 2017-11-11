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
 * Description: 	Silhouette code
 *
 * $Archive: /dpvs/implementation/sources/dpvsSilhouette.cpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 14.06.01 14:50 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#include "dpvsSilhouette.hpp"
#include "dpvsRectangle.hpp"
#include "dpvsMath.hpp"

#include <cstring>		// for memcpy()
using namespace DPVS;

/*****************************************************************************
 *
 * Function:		DPVS::EdgeSilhouette::EdgeSilhouette()
 *
 * Description:		Constructor
 *
 *****************************************************************************/

EdgeSilhouette::EdgeSilhouette (void) : m_vertex(0), m_fixedVertex(0), m_vertexCount(0),m_planeCount(0)
{ 
	clear(); 
}

/*****************************************************************************
 *
 * Function:		DPVS::EdgeSilhouette::~EdgeSilhouette()
 *
 * Description:		Destructor
 *
 *****************************************************************************/

EdgeSilhouette::~EdgeSilhouette	(void)
{
	// nada (stored in .cpp just to reduce amount of code generated)
}

/*****************************************************************************
 *
 * Function:		DPVS::EdgeSilhouette::getRectangle()
 *
 * Description:		Scans axis-aligned bounding box of the vertices
 *
 * Parameters:		f = reference to destination float rectangle
 *
 *****************************************************************************/

void EdgeSilhouette::getRectangle (FloatRectangle& f) const
{
	DPVS_ASSERT(m_vertexCount > 0);

	Vector2 mn(NO_CONSTRUCTOR),mx(NO_CONSTRUCTOR);
	Math::minMax (mn,mx,&m_vertex[0],m_vertexCount);

	f.set (mn.x,mx.x,mn.y,mx.y);

	f.x1++;									//make exclusive end
	f.y1++;									//make exclusive end
	f.setSpace(SPACE_RASTER);				// NOTE: assumed to be in raster-space
}

/*****************************************************************************
 *
 * Function:		DPVS::EdgeSilhouette::setVertices()
 *
 * Description:		Assigns vertex array to the silhouette 
 *
 * Parameters:		loc = pointer to vertices
 *					n	= number of vertices
 *
 *****************************************************************************/

void EdgeSilhouette::setVertices		(const Vector2* loc,int n)
{
	if(n > (int)(m_vertex.getSize()))		// allocate space
		resizeVertexArray(n);

	for(int i=0;i<n;i++)
	{
		m_vertex[i].x = loc[i].x;
		m_vertex[i].y = loc[i].y;
	}

	m_vertexCount = n;
}

/*****************************************************************************
 *
 * Function:		DPVS::EdgeSilhouette::setVertices()
 *
 * Description:		Assigns vertex array to the silhouette 
 *
 * Parameters:		loc = pointer to vertices
 *					n	= number of vertices
 *
 *****************************************************************************/

void EdgeSilhouette::setVertices		(const Vector3* loc,int n)
{
	if(n > (int)(m_vertex.getSize()))
		resizeVertexArray(n);

	for(int i=0;i<n;i++)
	{
		m_vertex[i].x = loc[i].x;
		m_vertex[i].y = loc[i].y;
	}

	m_vertexCount = n;
}

/*****************************************************************************
 *
 * Function:		DPVS::EdgeSilhouette::setVertices()
 *
 * Description:		Assigns vertex array to the silhouette 
 *
 * Parameters:		loc = pointer to vertices
 *					n	= number of vertices
 *
 *****************************************************************************/

void EdgeSilhouette::setVertices		(const Vector4* loc,int n)
{
	if(n > (int)(m_vertex.getSize()))
		resizeVertexArray(n);

	for(int i=0;i<n;i++)
	{
		m_vertex[i].x = loc[i].x;
		m_vertex[i].y = loc[i].y;
	}

	m_vertexCount = n;
}

/*****************************************************************************
 *
 * Function:		DPVS::EdgeSilhouette::resizeEdgeBuffer()
 *
 * Description:		Resizes either the left or the right edge buffer
 *
 * Parameters:		t		= side index (0 = left, 1 = right)
 *					newSize = number of edges in the new buffer
 *	
 *****************************************************************************/

void EdgeSilhouette::resizeEdgeBuffer (int t, int newSize)
{
	DPVS_ASSERT(t>=0 && t<=1);
	DPVS_ASSERT(newSize >= 0);
	m_edge[t].resize(newSize);	
}

/*****************************************************************************
 *
 * Function:		DPVS::EdgeSilhouette::resizeVertexArray()
 *
 * Description:		Resizes the vertex array
 *
 * Parameters:		newSize = number of vertices in the new buffer
 *	
 *****************************************************************************/

void EdgeSilhouette::resizeVertexArray (int n)
{
	DPVS_ASSERT(n>=0);
	m_vertex.reset(n);
	m_fixedVertex.reset(n);
}

//------------------------------------------------------------------------
