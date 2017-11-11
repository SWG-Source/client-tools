//===================================================================
//
// IndexedTriangleListShaderPrimitive.h
// copyright 2001, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_IndexedTriangleListShaderPrimitive_H
#define INCLUDED_IndexedTriangleListShaderPrimitive_H

//===================================================================

class Appearance;
class IndexedTriangleList;

#include "clientGraphics/ShaderPrimitive.h"
#include "sharedMath/VectorArgb.h"

//===================================================================

class IndexedTriangleListShaderPrimitive : public ShaderPrimitive
{
public:

	IndexedTriangleListShaderPrimitive (const Appearance& appearance, const IndexedTriangleList* indexedTriangleList, bool bOwnsList, const VectorArgb& color);
	virtual ~IndexedTriangleListShaderPrimitive ();

	virtual const StaticShader& prepareToView () const;
	virtual const Vector        getPosition_w () const;
	virtual float               getDepthSquaredSortKey () const;
	virtual int                 getVertexBufferSortKey () const;
	virtual void                prepareToDraw () const;
	virtual void                draw () const;

	IndexedTriangleList const * getMesh( void ) const;

private:

	IndexedTriangleListShaderPrimitive ();
	IndexedTriangleListShaderPrimitive (const IndexedTriangleListShaderPrimitive&);
	IndexedTriangleListShaderPrimitive& operator= (const IndexedTriangleListShaderPrimitive&);

private:

	const Appearance&          m_appearance;
	const IndexedTriangleList* m_indexedTriangleList;
	const VectorArgb           m_color;
	const bool                 m_bOwnsList;
};

//===================================================================

#endif
