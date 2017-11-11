#ifndef INCLUDED_Headless_StaticIndexBufferData_H
#define INCLUDED_Headless_StaticIndexBufferData_H

#include "clientGraphics/StaticIndexBuffer.h"

class StaticIndexBuffer;

class Headless_StaticIndexBufferData : public StaticIndexBufferGraphicsData
{
public:
	Headless_StaticIndexBufferData( const StaticIndexBuffer &buf );

	virtual Index *lock(bool readOnly);
	virtual void   unlock();

private:
	uint    m_indexCount;
	uint    m_lockCount;
	Index * m_buffer;
};
#endif // INCLUDED_Headless_StaticIndexBufferData_H