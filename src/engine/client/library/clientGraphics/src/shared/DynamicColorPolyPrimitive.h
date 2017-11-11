// ======================================================================
//
// DynamicColorPolyPrimitive.h
// Portions copyright 1999, bootprint entertainment
// Portions copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.

// ======================================================================

#ifndef INCLUDED_DynamicColorPolyPrimitive_H
#define INCLUDED_DynamicColorPolyPrimitive_H

#include "clientGraphics/SimplePolyPrimitive.h"

// This is a very simple primitive that draws a single, solid-color, 
// double-sided polygon with alpha blending. Since it uses a dynamic 
// vertex buffer internally, its color can be changed whenever you like.

class DynamicColorPolyPrimitive : public SimplePolyPrimitive
{
public:

	DynamicColorPolyPrimitive( Appearance const & owner, VertexList const & verts, VectorArgb const & newColor);
	virtual ~DynamicColorPolyPrimitive();

	// ----------
	
	virtual void                prepareToDraw() const;

	// ----------

	virtual void                setColor	( VectorArgb color );
	virtual VectorArgb			getColor    ( void ) const;

protected:

	virtual HardwareVertexBuffer *	createVertexBuffer	( void ) const;

private:

	DynamicColorPolyPrimitive(const DynamicColorPolyPrimitive &);
	DynamicColorPolyPrimitive &operator =(const DynamicColorPolyPrimitive &);
};

// ======================================================================

#endif