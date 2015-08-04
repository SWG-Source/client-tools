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
 * Description: 	Mesh implementation
 *
 * $Archive: /dpvs/implementation/sources/dpvsMesh.cpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 20.06.01 16:31 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#include "dpvsMesh.hpp"

#include <cstring>	// for ::memcpy()

using namespace DPVS;

int Mesh::s_memoryUsed = 0;					// total memory used by meshes

DPVS_FORCE_INLINE int Mesh::getBytesPerTriangle (void) const
{
	if (m_numVertices > 65536)
		return 12;

	int bytesPerTriangle = 6;	// 16bpp per index
	if (m_numVertices <= 1024)
		bytesPerTriangle = 4;
	if (m_numVertices <= 256)
		bytesPerTriangle = 3;
	if (m_numVertices <= 32)
		bytesPerTriangle = 2;

	return bytesPerTriangle;
}

/*****************************************************************************
 *
 * Function:		DPVS::Mesh::Mesh()
 *
 * Description:		Constructor 
 *
 *****************************************************************************/

Mesh::Mesh (void) :
	m_numVertices(0),
	m_vertices(null),
	m_numTriangles(0),
	m_triangles(null),
	m_compressed(false)
{ 
	s_memoryUsed += sizeof(Mesh);
}

/*****************************************************************************
 *
 * Function:		DPVS::Mesh::set()
 *
 * Description:		Assigns vertices/triangles to mesh
 *
 * Parameters:		numVertices  = number of vertices
 *					numTriangles = number of triangles
 *					vertices     = array of vertices
 *					triangles	 = array of triangle vertex indices
 *
 *****************************************************************************/

void Mesh::set (int numVertices, int numTriangles,const Vector3* v, const Vector3i* t)
{
	//--------------------------------------------------------------------
	// Update statistics
	//--------------------------------------------------------------------

	s_memoryUsed -= m_numTriangles * (m_compressed ? getBytesPerTriangle() : sizeof(Vector3i));
	s_memoryUsed -= m_numVertices * sizeof(Vector3);

	//--------------------------------------------------------------------
	// Free existing mesh
	//--------------------------------------------------------------------

	DELETE_ARRAY(m_vertices);
	DELETE_ARRAY(m_triangles);

	m_vertices		= null;
	m_triangles		= null;
	m_numVertices	= 0;
	m_numTriangles	= 0;
	m_compressed    = false;

	//--------------------------------------------------------------------
	// In debug build assert that triangle indices are inside proper range and
	// that vertex locations are OK
	//--------------------------------------------------------------------

	DPVS_ASSERT (numVertices>=0 && numTriangles>=0);

	if (numTriangles>0)
		DPVS_ASSERT(v && t);

#if defined (DPVS_DEBUG)
	for (int i = 0; i < numTriangles; i++)
	for (int j = 0; j < 3; j++)
		DPVS_ASSERT(t[i][j] >= 0 && t[i][j] < numVertices);
#endif

	//--------------------------------------------------------------------
	// Copy data
	//--------------------------------------------------------------------

	if (numTriangles > 0)
	{
		m_numVertices	= numVertices; 
		m_numTriangles	= numTriangles; 
		m_vertices		= NEW_ARRAY<Vector3>(numVertices);
		m_triangles		= NEW_ARRAY<UINT32>(numTriangles*3);	// 3*sizeof(UINT32)

		::memcpy (m_vertices,  v, m_numVertices  * sizeof(Vector3));
		::memcpy (m_triangles, t, m_numTriangles * sizeof(Vector3i));

		s_memoryUsed += m_numTriangles * sizeof(Vector3i) + m_numVertices * sizeof(Vector3);
	}
}

/*****************************************************************************
 *
 * Function:		DPVS::Mesh::compress()
 *
 * Description:		Changes mesh status to compressed (data is not directly
 *					accessible but consumes less memory)
 *
 *****************************************************************************/

void Mesh::compress	(void) const									
{ 
	if (m_compressed)					// already in compressed format
		return;

	m_compressed = true;				// mark as compressed

	if (m_numVertices > 65536)			// we don't compress these meshes
		return;

	//--------------------------------------------------------------------
	// We have here currently a very naive compression scheme that merely
	// removes redundant bits away. We could improve this by a variety
	// of means, such as delta compression etc.
	//--------------------------------------------------------------------

	int bytesPerTriangle = getBytesPerTriangle();

	UINT32*			d = NEW_ARRAY<UINT32>((m_numTriangles*bytesPerTriangle+3)>>2);	// 3 bytes per triangle
	const Vector3i* s = reinterpret_cast<const Vector3i*>(m_triangles);

	if (bytesPerTriangle==2)	// 5:5:5
	{
		UINT16* dst = reinterpret_cast<UINT16*>(d);
		for (int i = 0; i < m_numTriangles; i++)
			dst[i] = (UINT16)(s[i].i | (s[i].j<<5) | (s[i].k<<10));
	} else
	if (bytesPerTriangle==3)	// 8:8:8
	{
		unsigned char* dst = reinterpret_cast<unsigned char*>(d);
		for (int i = 0; i < m_numTriangles; i++)
		{
			dst[i*3+0] = (unsigned char)s[i].i;
			dst[i*3+1] = (unsigned char)s[i].j;
			dst[i*3+2] = (unsigned char)s[i].k;
		}
	} else						
	if (bytesPerTriangle==4)	// 10:10:10
	{
		for (int i = 0; i < m_numTriangles; i++)
			d[i] = (UINT32)(s[i].i | (s[i].j<<10) | (s[i].k<<20));
	} else 
	if (bytesPerTriangle==6) // 16:16:16
	{

		UINT16* dst = reinterpret_cast<UINT16*>(d);
		for (int i = 0; i < m_numTriangles; i++)
		{
			dst[i*3+0] = (UINT16)s[i].i;
			dst[i*3+1] = (UINT16)s[i].j;
			dst[i*3+2] = (UINT16)s[i].k;
		}
	} else
		DPVS_ASSERT(false);

	DELETE_ARRAY(m_triangles);							// free old ones
	m_triangles   = d;									// assign new pointer
	
	s_memoryUsed -= m_numTriangles * ((int)(sizeof(Vector3i))-bytesPerTriangle);
}

/*****************************************************************************
 *
 * Function:		DPVS::Mesh::uncompress()
 *
 * Description:		Changes mesh status to uncompressed (data is now directly
 *					accessible)
 *
 *****************************************************************************/

void Mesh::uncompress (void) const
{ 
	if (!m_compressed)			// already in uncompressed format
		return;
	m_compressed = false;		// now in uncompressed format..
	if (m_numVertices > 65536)	// we did not compress these..
		return;

	UINT32* dst = NEW_ARRAY<UINT32>(m_numTriangles*3);

	int bytesPerTriangle = getBytesPerTriangle();

	if (bytesPerTriangle == 2)	// 5:5:5
	{
		Vector3i*     d = reinterpret_cast<Vector3i*>(dst);
		const UINT16* src = reinterpret_cast<const UINT16*>(m_triangles);

		for (int i = 0; i < m_numTriangles; i++)
		{
			d[i].i = (int)(src[i])&31;
			d[i].j = ((int)src[i]>>5)&31;
			d[i].k = ((int)src[i]>>10)&31;
		}
	} else 
	if (bytesPerTriangle == 3) // 8:8:8
	{
		Vector3i*				d	= reinterpret_cast<Vector3i*>(dst);
		const unsigned char*	src = reinterpret_cast<const unsigned char*>(m_triangles);
		for (int i = 0; i < m_numTriangles; i++)
		{
			d[i].i = (int)src[i*3+0];
			d[i].j = (int)src[i*3+1];
			d[i].k = (int)src[i*3+2];
		}
	} else
	if (bytesPerTriangle == 4)	// 10:10:10
	{
		Vector3i*		d	= reinterpret_cast<Vector3i*>(dst);
		const UINT32*	src = reinterpret_cast<const UINT32*>(m_triangles);

		for (int i = 0; i < m_numTriangles; i++)
		{
			d[i].i = (int)(src[i]&1023);
			d[i].j = (int)((src[i]>>10)&1023);
			d[i].k = (int)((src[i]>>20)&1023);
		}
	} else 
	if (bytesPerTriangle == 6)	// 16:16:16
	{
		Vector3i*		d	= reinterpret_cast<Vector3i*>(dst);
		const UINT16*	src = reinterpret_cast<const UINT16*>(m_triangles);
		for (int i = 0; i < m_numTriangles; i++)
		{
			d[i].i = (int)src[i*3+0];
			d[i].j = (int)src[i*3+1];
			d[i].k = (int)src[i*3+2];
		}
	} else
		DPVS_ASSERT(false);

	DELETE_ARRAY(m_triangles);
	m_triangles = dst;

	s_memoryUsed += m_numTriangles * ((int)sizeof(Vector3i)-bytesPerTriangle);
}

/*****************************************************************************
 *
 * Function:		DPVS::Mesh::~Mesh()
 *
 * Description:		Destructor
 *
 *****************************************************************************/

Mesh::~Mesh	(void)									
{ 
	set(0,0,null,null);
	s_memoryUsed -= sizeof(Mesh);
	DPVS_ASSERT(s_memoryUsed >= 0);
}

//------------------------------------------------------------------------
