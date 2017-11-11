#ifndef __DPVSMESH_HPP
#define __DPVSMESH_HPP
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
 * Desc:	Mesh interface
 *
 * $Archive: /dpvs/implementation/include/dpvsMesh.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 7.05.01 12:06 $
 * $Date: 2003/01/22 $
 * 
 *****************************************************************************/

#if !defined (__DPVSVECTOR_HPP)
#	include "dpvsVector.hpp"				
#endif

namespace DPVS
{

/******************************************************************************
 *
 * Class:			DPVS::Mesh
 *
 * Description:		Class for storing a mesh. The mesh is constructed with
 *					a call to Mesh::set(), after that it can be accessed
 *					only by constant accessors.
 *
 ******************************************************************************/

class Mesh
{
private:
							Mesh				(const Mesh&);			// not allowed
	Mesh&					operator=			(const Mesh&);			// not allowed
	void					uncompress			(void) const;			// uncompression routine
	int						getBytesPerTriangle (void) const;			// compression ratio

	int						m_numVertices;								// number of vertices
	Vector3*				m_vertices;									// vertex positions
	int						m_numTriangles;								// number of triangles
	mutable UINT32*			m_triangles;								// triangle indices (either compressed or uncompressed)
	mutable bool			m_compressed;								// is stuff in compressed format?

	static int				s_memoryUsed;								// total memory used by meshes

public:
							Mesh			(void);
							~Mesh			(void);									

	void					compress		(void) const;
	int						getVertexCount	(void) const							{ return m_numVertices;		}
	int						getTriangleCount(void) const							{ return m_numTriangles;	}
	const Vector3*			getVertices		(void) const							{ return m_vertices; }
	const Vector3i*			getTriangles	(void) const							{ if (m_compressed) uncompress(); DPVS_ASSERT(!m_compressed); return reinterpret_cast<const Vector3i*>(m_triangles); }
	void					set				(int numVertices, int numTriangles, const Vector3* v, const Vector3i* t);

	static int				getMemoryUsed	(void)									{ return s_memoryUsed; }
};	

} // DPVS

//------------------------------------------------------------------------
#endif // __DPVSMESH_HPP
