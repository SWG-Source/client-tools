// ======================================================================
//
// Direct3d9_VertexDeclarationMap.h
// Copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_Direct3d9_VertexDeclarationMap_H
#define INCLUDED_Direct3d9_VertexDeclarationMap_H

// ======================================================================

struct IDirect3DVertexDeclaration9;
class  VertexBufferFormat;

// ======================================================================

class Direct3d9_VertexDeclarationMap
{
public:

	static void                         install();
	static void                         remove();
	static IDirect3DVertexDeclaration9 *fetchVertexDeclaration(VertexBufferFormat const &vertexBufferFormat);
	static IDirect3DVertexDeclaration9 *fetchVertexDeclaration(VertexBufferFormat const * const * vertexBufferFormat, int count);
};

// ======================================================================

#endif
