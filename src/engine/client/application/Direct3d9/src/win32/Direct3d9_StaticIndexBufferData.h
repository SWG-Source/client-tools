// ======================================================================
//
// Direct3d9_StaticIndexBufferData.h
//
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_Direct3d9_StaticIndexBufferData_H
#define INCLUDED_Direct3d9_StaticIndexBufferData_H

// ======================================================================

struct IDirect3DIndexBuffer9;
class  MemoryBlockManager;

#include "clientGraphics/StaticIndexBuffer.h"

// ======================================================================

class Direct3d9_StaticIndexBufferData : public StaticIndexBufferGraphicsData
{
public:

	static void *operator new(size_t size);
	static void  operator delete(void *memory);

public:

	static void install();
	static void remove();

public:

	explicit Direct3d9_StaticIndexBufferData(const StaticIndexBuffer &indexBuffer);
	virtual ~Direct3d9_StaticIndexBufferData();

	virtual Index *lock(bool readOnly);
	virtual void   unlock();

	IDirect3DIndexBuffer9 *getIndexBuffer() const;

#ifdef _DEBUG
	bool                    firstTimeUsedThisFrame() const;
	int                     getMemorySize() const;
#endif

private:

	/// Disabled.
	Direct3d9_StaticIndexBufferData();

	/// Disabled.
	Direct3d9_StaticIndexBufferData(const Direct3d9_StaticIndexBufferData &);

	/// Disabled.
	Direct3d9_StaticIndexBufferData &operator =(const Direct3d9_StaticIndexBufferData &);

private:

	static MemoryBlockManager  *ms_memoryBlockManager;

private:

	const StaticIndexBuffer  &m_indexBuffer;
	IDirect3DIndexBuffer9    *m_d3dIndexBuffer;

#ifdef _DEBUG
	mutable int                              m_lastUsedFrameNumber;
	int                                      m_memorySize;
#endif
};

// ======================================================================

inline IDirect3DIndexBuffer9 *Direct3d9_StaticIndexBufferData::getIndexBuffer() const
{
	return m_d3dIndexBuffer;
}

// ======================================================================

#endif
