// ======================================================================
//
// Direct3d9_StaticVertexBufferData.cpp
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstDirect3d9.h"
#include "Direct3d9_StaticVertexBufferData.h"

#include "Direct3d9.h"
#include "Direct3d9_Metrics.h"
#include "Direct3d9_VertexDeclarationMap.h"

#include "sharedFoundation/MemoryBlockManager.h"

// ======================================================================

MemoryBlockManager * Direct3d9_StaticVertexBufferData::ms_memoryBlockManager;

// ======================================================================

void Direct3d9_StaticVertexBufferData::install()
{
	ms_memoryBlockManager  = new MemoryBlockManager("Direct3d9_StaticVertexBufferData", true, sizeof(Direct3d9_StaticVertexBufferData), 0, 0, 0);
}

// ----------------------------------------------------------------------

void Direct3d9_StaticVertexBufferData::remove()
{
	delete ms_memoryBlockManager;
	ms_memoryBlockManager = NULL;
}

// ----------------------------------------------------------------------

void *Direct3d9_StaticVertexBufferData::operator new(size_t size)
{
	UNREF(size);
	NOT_NULL(ms_memoryBlockManager);
	DEBUG_FATAL(size != sizeof(Direct3d9_StaticVertexBufferData), ("wrong new called"));
	DEBUG_FATAL(size != static_cast<size_t> (ms_memoryBlockManager->getElementSize()), ("installed with bad size"));
	return ms_memoryBlockManager->allocate();
}

// ----------------------------------------------------------------------

void Direct3d9_StaticVertexBufferData::operator delete(void *memory)
{
	NOT_NULL(ms_memoryBlockManager);
	ms_memoryBlockManager->free(memory);
}

// ======================================================================

Direct3d9_StaticVertexBufferData::Direct3d9_StaticVertexBufferData(const StaticVertexBuffer &vertexBuffer)
:
	m_vertexBuffer(vertexBuffer),
	m_descriptor(Direct3d9_VertexBufferDescriptorMap::getDescriptor(vertexBuffer.getFormat())),
	m_d3dVertexBuffer(0),
	m_vertexDeclaration(Direct3d9_VertexDeclarationMap::fetchVertexDeclaration(vertexBuffer.getFormat()))
#ifdef _DEBUG
	,
	m_lastUsedFrameNumber(0),
	m_memorySize(m_descriptor.vertexSize * m_vertexBuffer.getNumberOfVertices())
#endif
{
	// create the VB
	IDirect3DDevice9 *device = Direct3d9::getDevice();
	HRESULT hresult = device->CreateVertexBuffer(m_descriptor.vertexSize * m_vertexBuffer.getNumberOfVertices(), 0, 0, D3DPOOL_MANAGED, &m_d3dVertexBuffer, NULL);
	FATAL_DX_HR("could not create VB %d", hresult);

#ifdef _DEBUG
	Direct3d9_Metrics::vertexBufferMemoryTotal += m_memorySize;
	Direct3d9_Metrics::vertexBufferMemoryCreated += m_memorySize;
#endif
}

// ----------------------------------------------------------------------

Direct3d9_StaticVertexBufferData::~Direct3d9_StaticVertexBufferData()
{
#ifdef _DEBUG
	Direct3d9_Metrics::vertexBufferMemoryTotal -= m_memorySize;
	Direct3d9_Metrics::vertexBufferMemoryDestroyed += m_memorySize;
#endif

	IGNORE_RETURN(m_vertexDeclaration->Release());
	IGNORE_RETURN(m_d3dVertexBuffer->Release());
}

// ----------------------------------------------------------------------

const VertexBufferDescriptor  &Direct3d9_StaticVertexBufferData::getDescriptor() const
{
	return m_descriptor;
}

// ----------------------------------------------------------------------

void *Direct3d9_StaticVertexBufferData::lock(bool readOnly)
{
	NOT_NULL(m_d3dVertexBuffer);

#ifdef _DEBUG
	if (!readOnly)
		Direct3d9_Metrics::vertexBufferMemoryModified += m_memorySize;
#endif

	void *data = NULL;
	const HRESULT hresult = m_d3dVertexBuffer->Lock(0, 0, &data, readOnly ? D3DLOCK_READONLY : 0);
	FATAL_DX_HR("Could not lock static vb %s", hresult);

	NOT_NULL(data);
	return data;
}

// ----------------------------------------------------------------------

void Direct3d9_StaticVertexBufferData::unlock()
{
	const HRESULT hresult = m_d3dVertexBuffer->Unlock();
	FATAL_DX_HR("Could not unlock vb %s", hresult);
}

// ----------------------------------------------------------------------

int Direct3d9_StaticVertexBufferData::getSortKey()
{
	return reinterpret_cast<int>(m_d3dVertexBuffer);
}

// ----------------------------------------------------------------------

#ifdef _DEBUG
bool Direct3d9_StaticVertexBufferData::firstTimeUsedThisFrame() const
{
	if (m_lastUsedFrameNumber == Direct3d9::getFrameNumber())
		return false;

	m_lastUsedFrameNumber = Direct3d9::getFrameNumber();
	return true;
}
#endif

// ----------------------------------------------------------------------

#ifdef _DEBUG
int Direct3d9_StaticVertexBufferData::getMemorySize() const
{
	return m_memorySize;
}
#endif

// ======================================================================
