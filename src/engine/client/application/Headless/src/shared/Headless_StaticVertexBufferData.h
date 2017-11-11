#ifndef INCLUDED_Headless_StaticVertexBufferData_H
#define INCLUDED_Headless_StaticVertexBufferData_H

/*
class StaticVertexBufferGraphicsData
{
public:
	virtual DLLEXPORT ~StaticVertexBufferGraphicsData();

	virtual const VertexBufferDescriptor  &getDescriptor() const = 0;
	virtual int                            getSortKey() = 0;

	virtual void                          *lock(bool readOnly) = 0;
	virtual void                           unlock() = 0;
};

*/

#include "clientGraphics/StaticVertexBuffer.h"

class Headless_StaticVertexBufferData : public StaticVertexBufferGraphicsData
{
public:
	Headless_StaticVertexBufferData( const StaticVertexBuffer &vb );

	virtual const VertexBufferDescriptor  &getDescriptor() const;
	virtual int                            getSortKey();
	virtual void                          *lock(bool readOnly);
	virtual void                           unlock();

private:
	VertexBufferFormat   m_format;
	char *               m_buffer;
	uint                 m_vertexCount;
	uint                 m_lockCount;

};
#endif // INCLUDED_Headless_StaticVertexBufferData_H