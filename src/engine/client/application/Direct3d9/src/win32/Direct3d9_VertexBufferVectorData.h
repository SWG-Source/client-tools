// ======================================================================
//
// Direct3d9_VertexBufferVectorData.h
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_Direct3d9_VertexBufferVectorData_H
#define INCLUDED_Direct3d9_VertexBufferVectorData_H

// ======================================================================

#include "Direct3d9.h"
#include "clientGraphics/VertexBufferVector.h"

// ======================================================================

class Direct3d9_VertexBufferVectorData : public VertexBufferVectorGraphicsData
{
public:

	Direct3d9_VertexBufferVectorData(VertexBufferVector const & vertexBufferVector);
	virtual ~Direct3d9_VertexBufferVectorData();

	IDirect3DVertexDeclaration9 * getVertexDeclaration();

private:

	IDirect3DVertexDeclaration9 * m_vertexDeclaration;
};

// ======================================================================

inline IDirect3DVertexDeclaration9 * Direct3d9_VertexBufferVectorData::getVertexDeclaration()
{
	return m_vertexDeclaration;
}

// ======================================================================

#endif
