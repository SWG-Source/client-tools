#include "FirstHeadless.h"
#include "Headless_StaticIndexBufferData.h"

Headless_StaticIndexBufferData::Headless_StaticIndexBufferData( const StaticIndexBuffer &buf ) :
m_indexCount( buf.getNumberOfIndices() ),
m_lockCount( 0 ),
m_buffer( 0 )
{
}

Index *Headless_StaticIndexBufferData::lock( bool )
{
	if ( !m_lockCount++ )
	{
		m_buffer = new Index[ m_indexCount * sizeof( int ) ];
		memset( m_buffer, 0, m_indexCount * sizeof( int ) );
	}

	return m_buffer;
}


void Headless_StaticIndexBufferData::unlock()
{
	if ( !--m_lockCount )
	{
		delete [] m_buffer;
		m_buffer = 0;
	}
}