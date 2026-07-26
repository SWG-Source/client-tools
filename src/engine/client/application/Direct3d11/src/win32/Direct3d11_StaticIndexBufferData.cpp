// ======================================================================
//
// Direct3d11_StaticIndexBufferData.cpp
// copyright (c) 2026 Galaxies Reborn
//
// ======================================================================

#include "FirstDirect3d11.h"
#include "Direct3d11_StaticIndexBufferData.h"

#include "Direct3d11_Device.h"
#include "Direct3d11_Metrics.h"

// ======================================================================

Direct3d11_StaticIndexBufferData::Direct3d11_StaticIndexBufferData(StaticIndexBuffer const &indexBuffer)
:
	StaticIndexBufferGraphicsData(),
	m_buffer(NULL),
	m_shadow(NULL),
	m_numberOfIndices(indexBuffer.getNumberOfIndices()),
	m_lockedForWriting(false),
	m_everUploaded(false)
{
	// As with vertices, a zero-length buffer is constructible in the engine but
	// cannot back a D3D resource.
	int const indices = (m_numberOfIndices > 0) ? m_numberOfIndices : 1;

	m_shadow = new Index[indices];
	memset(m_shadow, 0, indices * sizeof(Index));

	createBuffer();
}

// ----------------------------------------------------------------------

Direct3d11_StaticIndexBufferData::~Direct3d11_StaticIndexBufferData()
{
	DEBUG_WARNING(m_lockedForWriting, ("Direct3d11: a static index buffer was destroyed while still locked."));

	if (m_buffer)
	{
		m_buffer->Release();
		m_buffer = NULL;
	}

	delete [] m_shadow;
	m_shadow = NULL;
}

// ----------------------------------------------------------------------

void Direct3d11_StaticIndexBufferData::createBuffer()
{
	ID3D11Device1 * const device = Direct3d11_Device::getDevice();
	NOT_NULL(device);

	int const indices = (m_numberOfIndices > 0) ? m_numberOfIndices : 1;

	D3D11_BUFFER_DESC description;
	Zero(description);
	description.ByteWidth = static_cast<UINT>(indices * sizeof(Index));
	description.Usage     = D3D11_USAGE_DEFAULT;
	description.BindFlags = D3D11_BIND_INDEX_BUFFER;

	HRESULT const hresult = device->CreateBuffer(&description, NULL, &m_buffer);
	FATAL(FAILED(hresult) || !m_buffer, ("Direct3d11: a %d-index static index buffer could not be created (%s).", indices, Direct3d11_Device::describeHresult(hresult)));
}

// ======================================================================

Index *Direct3d11_StaticIndexBufferData::lock(bool readOnly)
{
	NOT_NULL(m_shadow);

	DEBUG_WARNING(m_lockedForWriting, ("Direct3d11: a static index buffer was locked while already locked for writing."));
	DEBUG_WARNING(readOnly && !m_everUploaded, ("Direct3d11: a static index buffer was read before anything was written to it."));

	m_lockedForWriting = !readOnly;
	return m_shadow;
}

// ----------------------------------------------------------------------

void Direct3d11_StaticIndexBufferData::unlock()
{
	if (!m_lockedForWriting)
		return;

	m_lockedForWriting = false;

	ID3D11DeviceContext1 * const context = Direct3d11_Device::getContext();
	if (!context || !m_buffer)
		return;

	context->UpdateSubresource(m_buffer, 0, NULL, m_shadow, 0, 0);

	m_everUploaded = true;
}

// ======================================================================

ID3D11Buffer *Direct3d11_StaticIndexBufferData::getBuffer() const
{
	return m_buffer;
}

// ----------------------------------------------------------------------

int Direct3d11_StaticIndexBufferData::getNumberOfIndices() const
{
	return m_numberOfIndices;
}

// ======================================================================
