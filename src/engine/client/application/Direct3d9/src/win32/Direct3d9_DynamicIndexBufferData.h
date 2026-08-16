// ======================================================================
//
// Direct3d9_DynamicIndexBufferData.h
//
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_Direct3d9_DynamicIndexBufferData_H
#define INCLUDED_Direct3d9_DynamicIndexBufferData_H

// ======================================================================

struct IDirect3DIndexBuffer9;
class  MemoryBlockManager;

#include "clientGraphics/DynamicIndexBuffer.h"

// ======================================================================

class Direct3d9_DynamicIndexBufferData : public DynamicIndexBufferGraphicsData
{
public:

	static void  install();
	static void  remove();
	static void  beginFrame();
	static void  lostDevice();
	static void  restoreDevice();
	static void  setSize(int numberOfIndices);
	static IDirect3DIndexBuffer9 *getIndexBuffer();

	static void *operator new(size_t size);
	static void  operator delete(void *memory);

public:

	Direct3d9_DynamicIndexBufferData();
	virtual ~Direct3d9_DynamicIndexBufferData();

	virtual Index *lock(int numberOfIndices);
	virtual void   unlock();

	DWORD                  getOffset() const;
	int                    getNumberOfIndices() const;

private:

	// Disabled.
	Direct3d9_DynamicIndexBufferData(const Direct3d9_DynamicIndexBufferData &);

	// Disabled.
	Direct3d9_DynamicIndexBufferData &operator =(const Direct3d9_DynamicIndexBufferData &);

private:

	static bool                    ms_newFrame;
	static MemoryBlockManager  *   ms_memoryBlockManager;
	static int                     ms_numberOfIndices;
	static int                     ms_usedNumberOfIndices;
	static IDirect3DIndexBuffer9 * ms_d3dIndexBuffer;

	static int                     ms_locksSinceBeginFrame;
	static int                     ms_discardsSinceBeginFrame;
	static int                     ms_locksSinceResourceCreation;
	static int                     ms_discardsSinceResourceCreation;
	static int                     ms_locksEver;
	static int                     ms_discardsEver;

private:

	DWORD  m_offset;
	int    m_numberOfIndices;
};

// ======================================================================

inline IDirect3DIndexBuffer9 *Direct3d9_DynamicIndexBufferData::getIndexBuffer()
{
	return ms_d3dIndexBuffer;
}

// ----------------------------------------------------------------------

inline DWORD Direct3d9_DynamicIndexBufferData::getOffset() const
{
	return m_offset;
}

// ----------------------------------------------------------------------

inline int Direct3d9_DynamicIndexBufferData::getNumberOfIndices() const
{
	return m_numberOfIndices;
}

// ======================================================================

#endif
