// ======================================================================
//
// Direct3d11_DynamicVertexBufferData.cpp
// copyright (c) 2026 Galaxies Reborn
//
// ======================================================================

#include "FirstDirect3d11.h"
#include "Direct3d11_DynamicVertexBufferData.h"

#include "Direct3d11_Device.h"
#include "Direct3d11_Metrics.h"
#include "Direct3d11_VertexBufferDescriptorMap.h"

#include "clientGraphics/VertexBufferDescriptor.h"
#include "clientGraphics/VertexBufferFormat.h"

// ======================================================================

namespace Direct3d11_DynamicVertexBufferDataNamespace
{
	constexpr int cs_kilobyte = 1024;
	constexpr int cs_lowVideoMemoryMegabytes = 16;
	constexpr int cs_mediumVideoMemoryMegabytes = 32;
	constexpr int cs_highVideoMemoryMegabytes = 64;
	constexpr int cs_lowRingBytes = 256 * cs_kilobyte;
	constexpr int cs_mediumRingBytes = 512 * cs_kilobyte;
	constexpr int cs_highRingBytes = 1024 * cs_kilobyte;
	constexpr int cs_maximumRingBytes = 2048 * cs_kilobyte;

	ID3D11Buffer *ms_ring;

	// A mirror of the ring in system memory. Locks write here; unlock copies the
	// slice across. Same size as the ring so a lock's byte offset is the same in
	// both, which keeps the copy a single memcpy with no second address space to
	// reason about.
	uint8 *ms_scratch;

	int ms_size; // bytes
	int ms_used; // bytes
	bool ms_newFrame;

	// Every dynamic vertex buffer shares one sort key, because they all share one
	// ring and the sorter's only use of the key is to group draws that can be
	// batched. Zero, kept disjoint from the static buffers' keys which start at 1.
	int const cms_sharedSortKey = 0;

	void roundUpUsed(int vertexSize);
} // namespace Direct3d11_DynamicVertexBufferDataNamespace
using namespace Direct3d11_DynamicVertexBufferDataNamespace;

// ======================================================================
/**
 * Advance the shared cursor to the next whole vertex.
 *
 * The same helper has to be used by lock() and by
 * getNumberOfLockableDynamicVertices(), or the two disagree. That matters: the UI
 * asks how many vertices fit without discarding and then locks exactly that many,
 * so a one-vertex over-report makes the lock discard the ring and silently destroy
 * geometry already bound for a pending draw.
 */

void Direct3d11_DynamicVertexBufferDataNamespace::roundUpUsed(int vertexSize)
{
	if (vertexSize <= 0)
		return;

	int const remainder = ms_used % vertexSize;
	if (remainder)
		ms_used += vertexSize - remainder;
}

// ======================================================================

void Direct3d11_DynamicVertexBufferData::install()
{
	DEBUG_FATAL(ms_ring, ("Direct3d11_DynamicVertexBufferData::install called twice"));

	// DX9's tiers, reproduced. Note DX9 has the config-driven size commented out,
	// so these tiers are its only policy, and the ceiling is 2 MB.
	//
	// The ceiling is not arbitrary and must not be raised here. ShadowVolume
	// captures getNumberOfLockableDynamicVertices(true) once at install -- the
	// whole ring divided by the vertex size -- rounds it down to a multiple of
	// four, and uses it forever as a per-batch vertex budget whose indices it
	// writes as 16-bit values. Growing the ring enlarges that budget and pushes
	// those indices past 65535, and changes shadow batching, and therefore changes
	// the draw counts the geometry-integrity gate compares. Resizing is a
	// measured performance change, not a port detail.
	int const videoMemory = Direct3d11_Device::getVideoMemoryInMegabytes();
	if (videoMemory <= cs_lowVideoMemoryMegabytes)
		ms_size = cs_lowRingBytes;
	else if (videoMemory <= cs_mediumVideoMemoryMegabytes)
		ms_size = cs_mediumRingBytes;
	else if (videoMemory <= cs_highVideoMemoryMegabytes)
		ms_size = cs_highRingBytes;
	else
		ms_size = cs_maximumRingBytes;

	ms_used = 0;
	ms_newFrame = true;

	ID3D11Device1 *const device = Direct3d11_Device::getDevice();
	NOT_NULL(device);

	D3D11_BUFFER_DESC description;
	Zero(description);
	description.ByteWidth = static_cast<UINT>(ms_size);
	description.Usage = D3D11_USAGE_DYNAMIC;
	description.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	description.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HRESULT const hresult = device->CreateBuffer(&description, NULL, &ms_ring);
	FATAL(FAILED(hresult) || !ms_ring, ("Direct3d11: the %d KB dynamic vertex ring could not be created (%s).", ms_size / cs_kilobyte, Direct3d11_Device::describeHresult(hresult)));
	++Direct3d11_Metrics::constantBufferCreations;

	ms_scratch = new uint8[ms_size];
	memset(ms_scratch, 0, ms_size);
}

// ----------------------------------------------------------------------

void Direct3d11_DynamicVertexBufferData::remove()
{
	delete[] ms_scratch;
	ms_scratch = NULL;

	if (ms_ring)
	{
		ms_ring->Release();
		ms_ring = NULL;
	}

	ms_size = 0;
	ms_used = 0;
}

// ----------------------------------------------------------------------

void Direct3d11_DynamicVertexBufferData::beginFrame()
{
	ms_newFrame = true;
}

// ----------------------------------------------------------------------

ID3D11Buffer *Direct3d11_DynamicVertexBufferData::getRing()
{
	return ms_ring;
}

// ======================================================================

Direct3d11_DynamicVertexBufferData::Direct3d11_DynamicVertexBufferData(VertexBuffer const &vertexBuffer)
	: DynamicVertexBufferGraphicsData(),
	  m_descriptor(Direct3d11_VertexBufferDescriptorMap::getDescriptor(vertexBuffer.getFormat())),
	  m_offset(0),
	  m_numberOfVertices(0),
	  m_lockByteOffset(0),
	  m_locked(false)
{
}

// ----------------------------------------------------------------------

Direct3d11_DynamicVertexBufferData::~Direct3d11_DynamicVertexBufferData()
{
	// Owns nothing on purpose. These objects are routinely stack locals that die
	// while the geometry they wrote is still bound and awaiting a draw, so
	// releasing anything here would free memory the GPU is about to read.
	DEBUG_WARNING(m_locked, ("Direct3d11: a dynamic vertex buffer was destroyed while still locked."));
}

// ======================================================================

void *Direct3d11_DynamicVertexBufferData::lock(int numberOfVertices, bool forceDiscard)
{
	NOT_NULL(ms_scratch);
	DEBUG_FATAL(numberOfVertices <= 0, ("Direct3d11: a dynamic vertex lock asked for %d vertices.", numberOfVertices));

	int const vertexSize = static_cast<int>(m_descriptor.vertexSize);
	FATAL(vertexSize <= 0, ("Direct3d11: a dynamic vertex buffer has a vertex size of %d.", vertexSize));

	roundUpUsed(vertexSize);

	int const bytes = numberOfVertices * vertexSize;
	FATAL(bytes > ms_size, ("Direct3d11: a dynamic vertex lock of %d vertices needs %d bytes, more than the whole %d byte ring.", numberOfVertices, bytes, ms_size));

	// Exactly DX9's condition, and nothing added to it. A discard that DX9 would
	// not have done throws away geometry that is bound and waiting to be drawn --
	// and that window is real: the sorter draws once per shader pass with other
	// code building its own dynamic buffers in between.
	bool const discard = (ms_newFrame || forceDiscard || (ms_used + bytes > ms_size));

	if (discard)
	{
		ms_used = 0;
		ms_newFrame = false;

		// Rename now, at lock time, where DX9 renames. Doing it at unlock instead
		// would reorder the rename with respect to another buffer's writes when two
		// dynamic buffers are locked at once.
		ID3D11DeviceContext1 *const context = Direct3d11_Device::getContext();
		if (context && ms_ring)
		{
			Direct3d11_Metrics::ScopedTimer timer(Direct3d11_Metrics::ringMapTicks);

			D3D11_MAPPED_SUBRESOURCE mapped;
			Zero(mapped);
			HRESULT const hresult = context->Map(ms_ring, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
			FATAL(FAILED(hresult), ("Direct3d11: the dynamic vertex ring could not be renamed (%s).", Direct3d11_Device::describeHresult(hresult)));
			context->Unmap(ms_ring, 0);
			++Direct3d11_Metrics::ringDiscards;
		}
	}

	m_lockByteOffset = ms_used;
	m_offset = ms_used / vertexSize;
	m_numberOfVertices = numberOfVertices;
	m_locked = true;

	// The cursor is NOT advanced here. DX9 advances it in unlock, by the count the
	// caller says it actually used, which is how the UI can lock all remaining
	// space and give most of it back.
	return ms_scratch + m_lockByteOffset;
}

// ----------------------------------------------------------------------

void Direct3d11_DynamicVertexBufferData::unlock()
{
	unlock(m_numberOfVertices);
}

// ----------------------------------------------------------------------

void Direct3d11_DynamicVertexBufferData::unlock(int numberOfVertices)
{
	DEBUG_WARNING(!m_locked, ("Direct3d11: a dynamic vertex buffer was unlocked without being locked."));

	int const vertexSize = static_cast<int>(m_descriptor.vertexSize);
	int const bytes = numberOfVertices * vertexSize;

	ID3D11DeviceContext1 *const context = Direct3d11_Device::getContext();
	if (context && ms_ring && bytes > 0)
	{
		Direct3d11_Metrics::ScopedTimer timer(Direct3d11_Metrics::ringMapTicks);

		// NO_OVERWRITE: this slice was not written this frame, so appending cannot
		// disturb anything the GPU is still reading.
		D3D11_MAPPED_SUBRESOURCE mapped;
		Zero(mapped);
		HRESULT const hresult = context->Map(ms_ring, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &mapped);

		// FATAL rather than the workaround DX9 carries here. DX9 tests the mapped
		// pointer with IsBadWritePtr and, if it looks bad, redirects the write to a
		// shared heap block no GPU resource sees -- while still unlocking and still
		// advancing the ring, so the draw proceeds against stale contents. That
		// converts a driver fault into silent corruption.
		FATAL(FAILED(hresult), ("Direct3d11: the dynamic vertex ring could not be mapped for append (%s).", Direct3d11_Device::describeHresult(hresult)));
		NOT_NULL(mapped.pData);

		// Map returns the start of the whole resource, where D3D9's Lock returned
		// the start of the locked range. Forgetting the offset makes every dynamic
		// draw after the first render the first mesh's vertices.
		memcpy(static_cast<uint8 *>(mapped.pData) + m_lockByteOffset, ms_scratch + m_lockByteOffset, bytes);
		context->Unmap(ms_ring, 0);

		++Direct3d11_Metrics::ringNoOverwrites;
		Direct3d11_Metrics::ringBytes += bytes;
	}

	// Advance by what was actually used, not by what was locked.
	ms_used = m_lockByteOffset + bytes;

	m_numberOfVertices = numberOfVertices;
	m_locked = false;
}

// ======================================================================

VertexBufferDescriptor const &Direct3d11_DynamicVertexBufferData::getDescriptor() const
{
	return m_descriptor;
}

// ----------------------------------------------------------------------

int Direct3d11_DynamicVertexBufferData::getNumberOfLockableDynamicVertices(bool withDiscard)
{
	int const vertexSize = static_cast<int>(m_descriptor.vertexSize);
	if (vertexSize <= 0)
		return 0;

	roundUpUsed(vertexSize);

	return (ms_size - (withDiscard ? 0 : ms_used)) / vertexSize;
}

// ----------------------------------------------------------------------

int Direct3d11_DynamicVertexBufferData::getSortKey()
{
	return cms_sharedSortKey;
}

// ======================================================================

int Direct3d11_DynamicVertexBufferData::getOffset() const
{
	return m_offset;
}

// ----------------------------------------------------------------------

int Direct3d11_DynamicVertexBufferData::getNumberOfVertices() const
{
	return m_numberOfVertices;
}

// ----------------------------------------------------------------------

int Direct3d11_DynamicVertexBufferData::getVertexSize() const
{
	return static_cast<int>(m_descriptor.vertexSize);
}

// ======================================================================
