// ======================================================================
//
// DynamicColorPolyPrimitive.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/DynamicColorPolyPrimitive.h"

#include "clientGraphics/VertexBufferFormat.h"
#include "clientGraphics/DynamicVertexBuffer.h"

// ----------------------------------------------------------------------
	
DynamicColorPolyPrimitive::DynamicColorPolyPrimitive( Appearance const & owner, VertexList const & verts, VectorArgb const & color )
: SimplePolyPrimitive( owner, verts, color )
{
}

// ----------

DynamicColorPolyPrimitive::~DynamicColorPolyPrimitive()
{
}

// ----------------------------------------------------------------------

HardwareVertexBuffer * DynamicColorPolyPrimitive::createVertexBuffer( void ) const
{
	VertexBufferFormat format;
	format.setPosition();
	format.setColor0();

	return new DynamicVertexBuffer(format);
}

// ----------------------------------------------------------------------

void DynamicColorPolyPrimitive::prepareToDraw() const
{
	DynamicVertexBuffer * buffer = dynamic_cast<DynamicVertexBuffer*>(m_vertexBuffer);
	NOT_NULL(buffer);

	// ----------

	int bufferSize = (m_verts.size() - 1) * 2;

	buffer->lock(bufferSize);

	VertexBufferWriteIterator v = buffer->begin();

	uint i = 0;

	VectorArgb color = getColor();

	for(i = 0; i < m_verts.size(); i++)
	{
		v.setPosition( m_verts[i] );
		v.setColor0(color);
		++v;
	}

	for(i = m_verts.size() - 2; i > 0; i--)
	{
		v.setPosition( m_verts[i] );
		v.setColor0(color);
		++v;
	}

	buffer->unlock();

	SimplePolyPrimitive::prepareToDraw();
}

// ----------------------------------------------------------------------

void DynamicColorPolyPrimitive::setColor ( VectorArgb color )
{
	m_color = color;
}

// ----------

VectorArgb DynamicColorPolyPrimitive::getColor ( void ) const
{
	return m_color;
}

