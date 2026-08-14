// ======================================================================
//
// Direct3d11_DynamicIndexBufferData.cpp
// copyright (c) 2026 Galaxies Reborn
//
// ======================================================================

#include "FirstDirect3d11.h"
#include "Direct3d11_DynamicIndexBufferData.h"

#include "Direct3d11.h"
#include "Direct3d11_Device.h"
#include "Direct3d11_Metrics.h"

// ======================================================================

namespace Direct3d11_DynamicIndexBufferDataNamespace
{
	ID3D11Buffer *ms_ring;
	Index *ms_scratch;

	int ms_capacity; // indices
	int ms_used;	 // indices
	bool ms_newFrame;

	// DX9's default is 64 KB, which is 32768 sixteen-bit indices, and at least one
	// engine path assumes exactly that many are available. Matched rather than
	// raised, for the same reason the vertex ring is: batch sizes derived from ring
	// capacity feed draw counts, and draw counts are what the geometry gate
	// compares.
	int const cms_defaultIndices = 32768;
} // namespace Direct3d11_DynamicIndexBufferDataNamespace
using namespace Direct3d11_DynamicIndexBufferDataNamespace;

// ======================================================================

void Direct3d11_DynamicIndexBufferData::install()
{
	DEBUG_FATAL(ms_ring, ("Direct3d11_DynamicIndexBufferData::install called twice"));

	ms_capacity = cms_defaultIndices;
	ms_used = 0;
	ms_newFrame = true;

	ID3D11Device1 *const device = Direct3d11_Device::getDevice();
	NOT_NULL(device);

	D3D11_BUFFER_DESC description;
	Zero(description);
	description.ByteWidth = static_cast<UINT>(ms_capacity * sizeof(Index));
	description.Usage = D3D11_USAGE_DYNAMIC;
	description.BindFlags = D3D11_BIND_INDEX_BUFFER;
	description.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HRESULT const hresult = device->CreateBuffer(&description, NULL, &ms_ring);
	FATAL(FAILED(hresult) || !ms_ring, ("Direct3d11: the %d-index dynamic index ring could not be created (%s).", ms_capacity, Direct3d11_Device::describeHresult(hresult)));

	ms_scratch = new Index[ms_capacity];
	memset(ms_scratch, 0, ms_capacity * sizeof(Index));
}

// ----------------------------------------------------------------------

void Direct3d11_DynamicIndexBufferData::remove()
{
	delete[] ms_scratch;
	ms_scratch = NULL;

	if (ms_ring)
	{
		Direct3d11::forgetIndexBuffer(ms_ring);
		ms_ring->Release();
		ms_ring = NULL;
	}

	ms_capacity = 0;
	ms_used = 0;
}

// ----------------------------------------------------------------------

void Direct3d11_DynamicIndexBufferData::beginFrame()
{
	ms_newFrame = true;
}

// ----------------------------------------------------------------------

ID3D11Buffer *Direct3d11_DynamicIndexBufferData::getRing()
{
	return ms_ring;
}

// ======================================================================

Direct3d11_DynamicIndexBufferData::Direct3d11_DynamicIndexBufferData()
	: DynamicIndexBufferGraphicsData(),
	  m_offset(0),
	  m_numberOfIndices(0),
	  m_locked(false)
{
}

// ----------------------------------------------------------------------

Direct3d11_DynamicIndexBufferData::~Direct3d11_DynamicIndexBufferData()
{
	DEBUG_WARNING(m_locked, ("Direct3d11: a dynamic index buffer was destroyed while still locked."));
}

// ======================================================================

Index *Direct3d11_DynamicIndexBufferData::lock(int numberOfIndices)
{
	NOT_NULL(ms_scratch);
	DEBUG_FATAL(numberOfIndices <= 0, ("Direct3d11: a dynamic index lock asked for %d indices.", numberOfIndices));

	// The interface offers callers no way to ask how much room is left, so an
	// over-large request is a caller bug rather than a condition to absorb.
	FATAL(numberOfIndices > ms_capacity, ("Direct3d11: a dynamic index lock asked for %d indices, more than the whole %d-index ring.", numberOfIndices, ms_capacity));

	bool const discard = (ms_newFrame || (ms_used + numberOfIndices > ms_capacity));

	if (discard)
	{
		ms_used = 0;
		ms_newFrame = false;

		ID3D11DeviceContext1 *const context = Direct3d11_Device::getContext();
		if (context && ms_ring)
		{
			Direct3d11_Metrics::ScopedTimer timer(Direct3d11_Metrics::ringMapTicks);

			D3D11_MAPPED_SUBRESOURCE mapped;
			Zero(mapped);
			HRESULT const hresult = context->Map(ms_ring, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
			FATAL(FAILED(hresult), ("Direct3d11: the dynamic index ring could not be renamed (%s).", Direct3d11_Device::describeHresult(hresult)));
			context->Unmap(ms_ring, 0);
			++Direct3d11_Metrics::ringDiscards;
		}
	}

	m_offset = ms_used;
	m_numberOfIndices = numberOfIndices;
	m_locked = true;

	return ms_scratch + m_offset;
}

// ----------------------------------------------------------------------

void Direct3d11_DynamicIndexBufferData::unlock()
{
	DEBUG_WARNING(!m_locked, ("Direct3d11: a dynamic index buffer was unlocked without being locked."));

	ID3D11DeviceContext1 *const context = Direct3d11_Device::getContext();
	if (context && ms_ring && m_numberOfIndices > 0)
	{
		Direct3d11_Metrics::ScopedTimer timer(Direct3d11_Metrics::ringMapTicks);

		D3D11_MAPPED_SUBRESOURCE mapped;
		Zero(mapped);
		HRESULT const hresult = context->Map(ms_ring, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &mapped);
		FATAL(FAILED(hresult), ("Direct3d11: the dynamic index ring could not be mapped for append (%s).", Direct3d11_Device::describeHresult(hresult)));
		NOT_NULL(mapped.pData);

		memcpy(static_cast<Index *>(mapped.pData) + m_offset, ms_scratch + m_offset, m_numberOfIndices * sizeof(Index));
		context->Unmap(ms_ring, 0);

		++Direct3d11_Metrics::ringNoOverwrites;
		Direct3d11_Metrics::ringBytes += static_cast<int>(m_numberOfIndices * sizeof(Index));
	}

	// The index interface has a single unlock with no count, so the whole locked
	// range is always used.
	ms_used = m_offset + m_numberOfIndices;
	m_locked = false;
}

// ======================================================================

int Direct3d11_DynamicIndexBufferData::getOffset() const
{
	return m_offset;
}

// ----------------------------------------------------------------------

int Direct3d11_DynamicIndexBufferData::getNumberOfIndices() const
{
	return m_numberOfIndices;
}

// ======================================================================
