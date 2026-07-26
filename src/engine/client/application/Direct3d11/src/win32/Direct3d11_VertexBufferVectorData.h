// ======================================================================
//
// Direct3d11_VertexBufferVectorData.h
// copyright (c) 2026 Galaxies Reborn
//
// The backend's per-VertexBufferVector object, which holds nothing, deliberately.
//
// ----------------------------------------------------------------------
// Why it is empty
//
// DX9 uses this to pre-fetch an IDirect3DVertexDeclaration9 for the combination of vertex
// formats in the vector, so the declaration is ready before the first draw
// (Direct3d9_VertexBufferVectorData.cpp).
//
// D3D11's equivalent of a vertex declaration is an input layout, and an input layout cannot be
// created from the vertex formats alone -- it needs the vertex shader's input signature as
// well, which is not known here and cannot be. Which shader will be applied to these buffers
// is decided by the material at draw time.
//
// So the resolution happens where both halves of the key are in hand:
// Direct3d11_InputLayoutCache, called from prepareToDraw, keyed on the stream formats and the
// shader bytecode together. Caching anything here would be caching half a key.
//
// The object still exists because the engine allocates one per vector and deletes it in the
// destructor, and because a backend returning null for a create call is indistinguishable from
// one that failed.
//
// ======================================================================

#ifndef INCLUDED_Direct3d11_VertexBufferVectorData_H
#define INCLUDED_Direct3d11_VertexBufferVectorData_H

// ======================================================================

#include "clientGraphics/VertexBufferVector.h"

// ======================================================================

class Direct3d11_VertexBufferVectorData : public VertexBufferVectorGraphicsData
{
public:

	Direct3d11_VertexBufferVectorData();
	virtual ~Direct3d11_VertexBufferVectorData();

private:

	Direct3d11_VertexBufferVectorData(Direct3d11_VertexBufferVectorData const &);
	Direct3d11_VertexBufferVectorData &operator =(Direct3d11_VertexBufferVectorData const &);
};

// ======================================================================

#endif
