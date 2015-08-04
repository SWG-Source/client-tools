// ======================================================================
//
// Direct3d9_DynamicIndexBufferData.cpp
//
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstDirect3d9.h"
#include "Direct3d9_DynamicIndexBufferData.h"

#include "ConfigDirect3d9.h"
#include "Direct3d9.h"
#include "Direct3d9_Metrics.h"
#include "sharedFoundation/MemoryBlockManager.h"

// ======================================================================

bool                               Direct3d9_DynamicIndexBufferData::ms_newFrame;
MemoryBlockManager                *Direct3d9_DynamicIndexBufferData::ms_memoryBlockManager;
int                                Direct3d9_DynamicIndexBufferData::ms_numberOfIndices;
int                                Direct3d9_DynamicIndexBufferData::ms_usedNumberOfIndices;
IDirect3DIndexBuffer9             *Direct3d9_DynamicIndexBufferData::ms_d3dIndexBuffer;
int                                Direct3d9_DynamicIndexBufferData::ms_locksSinceBeginFrame;
int                                Direct3d9_DynamicIndexBufferData::ms_discardsSinceBeginFrame;
int                                Direct3d9_DynamicIndexBufferData::ms_locksSinceResourceCreation;
int                                Direct3d9_DynamicIndexBufferData::ms_discardsSinceResourceCreation;
int                                Direct3d9_DynamicIndexBufferData::ms_locksEver;
int                                Direct3d9_DynamicIndexBufferData::ms_discardsEver;

// ======================================================================

void Direct3d9_DynamicIndexBufferData::install()
{
	ms_numberOfIndices = (ConfigDirect3d9::getDynamicIndexBufferSize() * 1024) / sizeof(Index);
	restoreDevice();
	ms_memoryBlockManager = new MemoryBlockManager("Direct3d9_DynamicIndexBufferData", true, sizeof(Direct3d9_DynamicIndexBufferData), 0, 0, 0);
}

// ----------------------------------------------------------------------

void Direct3d9_DynamicIndexBufferData::remove()
{
	lostDevice();

	if (ms_memoryBlockManager)
	{
		delete ms_memoryBlockManager;
		ms_memoryBlockManager = NULL;
	}
}

// ----------------------------------------------------------------------

void Direct3d9_DynamicIndexBufferData::beginFrame()
{
	ms_newFrame = ConfigDirect3d9::getDiscardDynamicBuffersAtBeginningOfFrame();
	ms_locksSinceBeginFrame = 0;
	ms_discardsSinceBeginFrame = 0;
}

// ----------------------------------------------------------------------

void *Direct3d9_DynamicIndexBufferData::operator new(size_t size)
{
	UNREF(size);
	NOT_NULL(ms_memoryBlockManager);
	DEBUG_FATAL(size != sizeof (Direct3d9_DynamicIndexBufferData), ("bad size"));
	DEBUG_FATAL(size != static_cast<size_t> (ms_memoryBlockManager->getElementSize()), ("installed with bad size"));

	return ms_memoryBlockManager->allocate();
}

// ----------------------------------------------------------------------

void Direct3d9_DynamicIndexBufferData::operator delete(void *memory)
{
	NOT_NULL(ms_memoryBlockManager);
	ms_memoryBlockManager->free(memory);
}

// ----------------------------------------------------------------------

void Direct3d9_DynamicIndexBufferData::setSize(int numberOfIndices)
{
	if (ms_numberOfIndices != numberOfIndices)
	{
		lostDevice();
		ms_numberOfIndices = numberOfIndices;
		restoreDevice();
	}
}

// ----------------------------------------------------------------------

void Direct3d9_DynamicIndexBufferData::lostDevice()
{
	if (ms_d3dIndexBuffer)
	{
		ms_d3dIndexBuffer->Release();
		ms_d3dIndexBuffer = NULL;
		ms_locksSinceResourceCreation = 0;
		ms_discardsSinceResourceCreation = 0;
	}
}

// ----------------------------------------------------------------------

void Direct3d9_DynamicIndexBufferData::restoreDevice()
{
	ms_newFrame = ConfigDirect3d9::getDiscardDynamicBuffersAtBeginningOfFrame();
	ms_usedNumberOfIndices = 0;
	IDirect3DDevice9 *device = Direct3d9::getDevice();
	const HRESULT hresult = device->CreateIndexBuffer(ms_numberOfIndices * sizeof(Index), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &ms_d3dIndexBuffer, NULL);
	FATAL_DX_HR("could not restore dynamic IB %s", hresult);
	ms_locksSinceResourceCreation = 0;
	ms_discardsSinceResourceCreation = 0;
}

// ======================================================================

Direct3d9_DynamicIndexBufferData::Direct3d9_DynamicIndexBufferData()
:
	m_offset(0),
	m_numberOfIndices(0)
{
}

// ----------------------------------------------------------------------

Direct3d9_DynamicIndexBufferData::~Direct3d9_DynamicIndexBufferData()
{
}

// ----------------------------------------------------------------------

Index *Direct3d9_DynamicIndexBufferData::lock(int numberOfIndices)
{
	m_numberOfIndices = numberOfIndices;

	const int length = m_numberOfIndices * sizeof(Index);

#ifdef _DEBUG
	Direct3d9_Metrics::indexBufferMemoryDynamic += length;
#endif
	++ms_locksSinceBeginFrame;
	++ms_locksSinceResourceCreation;
	++ms_locksEver;

	// check for space in the dynamic index buffer
	DWORD lockFlag = D3DLOCK_NOOVERWRITE;
	int discard = 0;
	if (ms_newFrame || ms_usedNumberOfIndices + numberOfIndices > ms_numberOfIndices)
	{
		ms_newFrame = false;
		++ms_discardsSinceBeginFrame;
		++ms_discardsSinceResourceCreation;
		++ms_discardsEver;
		discard = 1;

		// make sure this IB will fit even when the dynamic ib is empty
		DEBUG_FATAL(numberOfIndices > ms_numberOfIndices, ("Too many indices %d/%d", numberOfIndices, ms_numberOfIndices));

#ifdef _DEBUG
		Direct3d9_Metrics::indexBufferDiscards += 1;
#endif

		// lock, discard contents
		lockFlag = D3DLOCK_DISCARD;
		ms_usedNumberOfIndices = 0;
	}

	// use up indices from this dynamic vb
	m_offset                = ms_usedNumberOfIndices;
	ms_usedNumberOfIndices += numberOfIndices;

	void *data = NULL;
	HRESULT const hresult = ms_d3dIndexBuffer->Lock(m_offset * sizeof(Index), length, &data, lockFlag);
	FATAL(FAILED(hresult), ("Could not lock dynamic %s %d=err %d=discard %d=offset %d=length %d/%d/%d=locks %d/%d/%d=discards", "ib", HRESULT_CODE(hresult), discard, m_offset * sizeof(Index), length, ms_locksSinceBeginFrame, ms_locksSinceResourceCreation, ms_locksEver, ms_discardsSinceBeginFrame, ms_discardsSinceResourceCreation, ms_discardsEver));
	NOT_NULL(data);

	if (IsBadWritePtr(data, 1) != 0)
		data = Direct3d9::getTemporaryBuffer(length);

	return reinterpret_cast<Index *>(data);
}

// ----------------------------------------------------------------------

void Direct3d9_DynamicIndexBufferData::unlock()
{
	const HRESULT hresult = ms_d3dIndexBuffer->Unlock();
	FATAL_DX_HR("Could not unlock IB %s", hresult);
}

// ======================================================================
