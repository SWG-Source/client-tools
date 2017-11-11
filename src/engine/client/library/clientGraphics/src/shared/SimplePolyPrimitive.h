// ======================================================================
//
// SimplePolyPrimitive.h
// Portions copyright 1999, bootprint entertainment
// Portions copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.

// ======================================================================

#ifndef INCLUDED_SimplePolyPrimitive_H
#define INCLUDED_SimplePolyPrimitive_H

#include "clientGraphics/ShaderPrimitive.h"

#include "sharedMath/VectorArgb.h"
#include "sharedMath/Vector.h"

#include <vector>

class Appearance;
class Shader;
class HardwareVertexBuffer;

typedef stdvector<Vector>::fwd VertexList;


// This is a very simple primitive that draws a single, solid-color
// polygon with alpha blending. Since it uses a static vertex buffer
// internally, the color can only be set on creation.

class SimplePolyPrimitive : public ShaderPrimitive
{
public:

	SimplePolyPrimitive( Appearance const & owner, VertexList const & verts, VectorArgb const & newColor );
	virtual ~SimplePolyPrimitive();

	// ----------
	
	virtual float               alter(float time);
	virtual const Vector        getPosition_w() const;
	virtual const StaticShader &prepareToView() const;
	virtual float               getDepthSquaredSortKey() const;
	virtual int                 getVertexBufferSortKey() const;
	virtual void                prepareToDraw() const;
	virtual void                draw() const;
	virtual float               getRadius() const;

	// ----------

protected:

	void computeRadius();

	virtual HardwareVertexBuffer *	createVertexBuffer	( void ) const;
	virtual void					setVertexBuffer		( HardwareVertexBuffer * buffer );

	virtual Shader const *			createShader		( void ) const;
	virtual void					setShader			( Shader const * shader );

	Appearance const &		m_owner;
	Shader const *			m_shader;
	HardwareVertexBuffer *  m_vertexBuffer;
	VertexList              m_verts;
	VectorArgb              m_color;

private:

	SimplePolyPrimitive(const SimplePolyPrimitive &);
	SimplePolyPrimitive &operator =(const SimplePolyPrimitive &);
};

// ======================================================================

#endif