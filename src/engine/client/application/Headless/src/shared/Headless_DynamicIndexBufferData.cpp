#include "FirstHeadless.h"
#include "Headless_DynamicIndexBufferData.h"

#include "clientGraphics/DynamicIndexBuffer.h"


Headless_DynamicIndexBufferData::Headless_DynamicIndexBufferData() :
m_buffer( 0 ),
m_indexCount( 0 ),
m_lockedIndices( 0 )
{
}

Index *Headless_DynamicIndexBufferData::lock( int count )
{
	Index *tmp = new Index[ count + m_lockedIndices ];

	if ( m_lockedIndices == 0 )
	{
		delete [] m_buffer;
	}

	m_buffer = tmp;

	Index *offset = m_buffer + m_lockedIndices;

	m_lockedIndices += count;

	return offset;
}

void Headless_DynamicIndexBufferData::unlock()
{
	m_lockedIndices = 0;
	delete [] m_buffer;
	m_buffer = 0;
}
