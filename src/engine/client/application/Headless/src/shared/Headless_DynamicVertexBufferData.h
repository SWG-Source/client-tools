#ifndef INCLUDED_Headless_DynamicVertexBufferData_H
#define INCLUDED_Headless_DynamicVertexBufferData_H

#include "clientGraphics/DynamicVertexBuffer.h"

class VertexBufferFormat;

class Headless_DynamicVertexBufferData : public DynamicVertexBufferGraphicsData
{
public:
	Headless_DynamicVertexBufferData( const VertexBuffer &vb );

	virtual void                          *lock(int numberOfVertices, bool forceDiscard);
	virtual void                           unlock();
	virtual void                           unlock(int numberOfVertices);
	virtual const VertexBufferDescriptor  &getDescriptor() const;
	virtual int                            getNumberOfLockableDynamicVertices(bool withDiscard);
	virtual int                            getSortKey();

private:
	VertexBufferFormat                     m_format;
	char *                                 m_buffer;
	int                                    m_lockCount;
};

#endif // INCLUDED_Headless_DynamicVertexBufferData_H