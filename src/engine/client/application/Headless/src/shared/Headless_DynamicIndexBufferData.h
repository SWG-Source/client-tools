#ifndef INCLUDED_Headless_DynamicIndexBufferData_H
#define INCLUDED_Headless_DynamicIndexBufferData_H

#include "clientGraphics/DynamicIndexBuffer.h"

class Headless_DynamicIndexBufferData : public DynamicIndexBufferGraphicsData
{
public:
	Headless_DynamicIndexBufferData();

	virtual Index *lock(int numberOfIndices);
	virtual void   unlock();

private:
	Index     *m_buffer;
	uint       m_indexCount;
	uint       m_lockedIndices;
};
#endif // INCLUDED_Headless_DynamicIndexBufferData_H