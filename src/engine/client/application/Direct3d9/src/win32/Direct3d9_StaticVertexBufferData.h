// ======================================================================
//
// Direct3d9_StaticVertexBufferData.h
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_Direct3d9_StaticVertexBufferData_H
#define INCLUDED_Direct3d9_StaticVertexBufferData_H

// ======================================================================

struct IDirect3DVertexBuffer9;
struct IDirect3DVertexDeclaration9;
class  MemoryBlockManager;

#include "clientGraphics/StaticVertexBuffer.h"
#include "Direct3d9_VertexBufferDescriptorMap.h"

// ======================================================================

class Direct3d9_StaticVertexBufferData: public StaticVertexBufferGraphicsData
{
public:

	static void *operator new(size_t size);
	static void  operator delete(void *memory);

public:

	static void install();
	static void remove();

public:

	explicit Direct3d9_StaticVertexBufferData(const StaticVertexBuffer &vertexBuffer);
	virtual ~Direct3d9_StaticVertexBufferData();

	virtual void                            *lock(bool readOnly);
	virtual void                             unlock();
	virtual const VertexBufferDescriptor    &getDescriptor() const;
	virtual int                              getSortKey();

	IDirect3DVertexBuffer9      *getVertexBuffer() const;
	int                          getVertexSize() const;
	IDirect3DVertexDeclaration9 *getVertexDeclaration() const;
	int                          getOffset() const;

#ifdef _DEBUG
	bool                    firstTimeUsedThisFrame() const;
	int                     getMemorySize() const;
#endif

private:

	/// disabled.
	Direct3d9_StaticVertexBufferData(void);
	Direct3d9_StaticVertexBufferData(const Direct3d9_StaticVertexBufferData &);
	Direct3d9_StaticVertexBufferData &operator =(const Direct3d9_StaticVertexBufferData &);

private:

	static MemoryBlockManager     *ms_memoryBlockManager;

private:

	const StaticVertexBuffer      &m_vertexBuffer;
	const VertexBufferDescriptor  &m_descriptor;
	IDirect3DVertexBuffer9        *m_d3dVertexBuffer;
	IDirect3DVertexDeclaration9   *m_vertexDeclaration;

#ifdef _DEBUG
	mutable int                    m_lastUsedFrameNumber;
	int                            m_memorySize;
#endif
};

// ======================================================================

inline IDirect3DVertexBuffer9 *Direct3d9_StaticVertexBufferData::getVertexBuffer() const
{
	return m_d3dVertexBuffer;
}

// ----------------------------------------------------------------------

inline int Direct3d9_StaticVertexBufferData::getVertexSize() const
{
	return m_descriptor.vertexSize;
}

// ----------------------------------------------------------------------

inline IDirect3DVertexDeclaration9 *Direct3d9_StaticVertexBufferData::getVertexDeclaration() const
{
	return m_vertexDeclaration;
}

// ======================================================================

#endif
