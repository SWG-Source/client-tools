#include "FirstHeadless.h"
#include "Headless_DynamicVertexBufferData.h"
#include "VertexBufferDescriptorCache.h"

#include "clientGraphics/VertexBuffer.h"
#include "clientGraphics/VertexBufferFormat.h"

namespace
{
	const int cs_maxNumLockableDynamicVerts = 1000000;
}

Headless_DynamicVertexBufferData::Headless_DynamicVertexBufferData( const VertexBuffer &vb ) :
m_format( vb.getFormat() ),
m_buffer( 0 ),
m_lockCount( 0 )
{
}

void *Headless_DynamicVertexBufferData::lock( int numberOfVertices, bool )
{
	//UNREF(numberOfVertices);
	const VertexBufferDescriptor &desc = getDescriptor();

	char *tmp = new char[ desc.vertexSize * ( numberOfVertices + m_lockCount ) ];

	if ( m_lockCount != 0 )
	{
		delete [] m_buffer;
	}

	m_buffer = tmp;

	// compute offset into vertex buffer where this new lock occurs
	char *offset = m_buffer + m_lockCount;

	// increment the number of locked vertices
	m_lockCount += numberOfVertices;

	return offset;
}

void Headless_DynamicVertexBufferData::unlock()
{
	m_lockCount = 0;
	delete [] m_buffer;
	m_buffer = 0;
}

void Headless_DynamicVertexBufferData::unlock( int count )
{
	m_lockCount -= count;

	if ( m_lockCount == 0 )
	{
		delete [] m_buffer;
		m_buffer = 0;
	}
}

const VertexBufferDescriptor &Headless_DynamicVertexBufferData::getDescriptor() const
{
	return VertexBufferDescriptorCache::getDescriptor( m_format );
}

int Headless_DynamicVertexBufferData::getNumberOfLockableDynamicVertices( bool )
{
	return cs_maxNumLockableDynamicVerts - m_lockCount;

}

int Headless_DynamicVertexBufferData::getSortKey()
{
	return 0;
}
