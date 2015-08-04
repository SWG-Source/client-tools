#include "FirstHeadless.h"
#include "Headless_StaticVertexBufferData.h"
#include "VertexBufferDescriptorCache.h"
#include "clientGraphics/VertexBuffer.h"
#include "clientGraphics/VertexBufferFormat.h"

Headless_StaticVertexBufferData::Headless_StaticVertexBufferData( const StaticVertexBuffer &vb ) :
m_format( vb.getFormat() ),
m_buffer( 0 ),
m_vertexCount( vb.getNumberOfVertices() ),
m_lockCount( 0 )
{
}

void *Headless_StaticVertexBufferData::lock( bool )
{

	if ( !m_lockCount++ )
	{
		m_buffer = new char[ getDescriptor().vertexSize * m_vertexCount ];
	}

	return m_buffer;

}

void Headless_StaticVertexBufferData::unlock()
{
	if ( !--m_lockCount )
	{
		delete [] m_buffer;
		m_buffer = 0;
	}
}


const VertexBufferDescriptor &Headless_StaticVertexBufferData::getDescriptor() const
{
	return VertexBufferDescriptorCache::getDescriptor( m_format );
}

int Headless_StaticVertexBufferData::getSortKey()
{
	return 0;
}
