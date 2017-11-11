// ======================================================================
//
// SimplePolyPrimitive.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/SimplePolyPrimitive.h"

#include "clientGraphics/Graphics.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/StaticVertexBuffer.h"
#include "sharedObject/Appearance.h"
#include "sharedMath/Transform.h"

// ----------------------------------------------------------------------

SimplePolyPrimitive::SimplePolyPrimitive( Appearance const & owner, VertexList const & verts, VectorArgb const & color )
: ShaderPrimitive(),
  m_owner(owner),
  m_shader(NULL),
  m_vertexBuffer(NULL),
  m_color(color)
{
	m_verts = verts;
}

// ----------

SimplePolyPrimitive::~SimplePolyPrimitive()
{
	delete m_vertexBuffer;
	m_vertexBuffer = NULL;

	if(m_shader) m_shader->release();
	m_shader = NULL;
}

//----------------------------------------------------------------------

void SimplePolyPrimitive::computeRadius()
{
	Vector maxVector;
	
	for (VertexList::const_iterator it = m_verts.begin(); it != m_verts.end(); ++it)
	{
		Vector const & pos = *it;

		maxVector.x = std::max(maxVector.x, abs(pos.x));
		maxVector.x = std::max(maxVector.y, abs(pos.y));
		maxVector.x = std::max(maxVector.z, abs(pos.z));		
	}

	m_radius = maxVector.magnitude();

}

// ----------------------------------------------------------------------

HardwareVertexBuffer * SimplePolyPrimitive::createVertexBuffer( void ) const
{
	VertexBufferFormat format;
	format.setPosition();
	format.setColor0();

	int bufferSize = (m_verts.size() - 1) * 2;

	StaticVertexBuffer * buffer = new StaticVertexBuffer(format,bufferSize);

	buffer->lock();

	VertexBufferReadWriteIterator v = buffer->begin();

	uint i = 0;

	for(i = 0; i < m_verts.size(); i++)
	{
		Vector const & pos = m_verts[i];
		
		v.setPosition(pos);
		v.setColor0(m_color);
		++v;
	}

	for(i = m_verts.size() - 2; i > 0; i--)
	{
		v.setPosition( m_verts[i] );
		v.setColor0(m_color);
		++v;
	}

	buffer->unlock();

	return buffer;

}

Shader const * SimplePolyPrimitive::createShader ( void ) const
{
	Shader const * shader = &ShaderTemplateList::get3dVertexColorACStaticShader();
	shader->fetch();

	return shader;
}

// ----------------------------------------------------------------------

void SimplePolyPrimitive::setVertexBuffer( HardwareVertexBuffer * buffer )
{
	if(m_vertexBuffer != buffer)
	{
		delete m_vertexBuffer;

		m_vertexBuffer = buffer;
	}
}

// ----------------------------------------------------------------------

void SimplePolyPrimitive::setShader ( Shader const * shader )
{
	NOT_NULL(shader);

	if(m_shader != shader)
	{
		if(m_shader) m_shader->release();

		m_shader = shader;
	}
}

// ----------------------------------------------------------------------

float SimplePolyPrimitive::alter(float time)
{
	if(m_vertexBuffer == NULL) 
	{
		setVertexBuffer( createVertexBuffer() );
		computeRadius();
	}
	if(m_shader == NULL)
	{
		setShader( createShader() );
	}

	NOT_NULL(m_shader);

	return m_shader->alter(time);
}

// ----------

const StaticShader &SimplePolyPrimitive::prepareToView() const
{
	NOT_NULL(m_shader);

	return m_shader->prepareToView();
}

// ----------

const Vector SimplePolyPrimitive::getPosition_w() const
{
	return m_owner.getTransform_w ().getPosition_p ();
}
	
//-------------------------------------------------------------------

float SimplePolyPrimitive::getDepthSquaredSortKey() const
{
	return m_owner.getTransform_w().getPosition_p().magnitudeBetweenSquared(ShaderPrimitiveSorter::getCurrentCameraPosition());
}

// ----------

int SimplePolyPrimitive::getVertexBufferSortKey() const
{
	return m_vertexBuffer->getSortKey();
}

// ----------

void SimplePolyPrimitive::prepareToDraw() const
{
	if(m_vertexBuffer)
	{
		Graphics::setObjectToWorldTransformAndScale (m_owner.getTransform_w(), Vector::xyz111);
		Graphics::setVertexBuffer(*m_vertexBuffer);
	}
}

// ----------

void SimplePolyPrimitive::draw() const
{
	Graphics::drawTriangleFan();
}

//----------------------------------------------------------------------

float SimplePolyPrimitive::getRadius() const
{
	return m_radius * m_owner.getScale().x;
}

// ----------------------------------------------------------------------
