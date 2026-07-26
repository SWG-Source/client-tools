// ======================================================================
//
// Direct3d11_StaticVertexBufferData.cpp
// copyright (c) 2026 Galaxies Reborn
//
// ======================================================================

#include "FirstDirect3d11.h"
#include "Direct3d11_StaticVertexBufferData.h"

#include "Direct3d11_Device.h"
#include "Direct3d11_Metrics.h"
#include "Direct3d11_VertexBufferDescriptorMap.h"

#include "clientGraphics/VertexBufferDescriptor.h"
#include "clientGraphics/VertexBufferFormat.h"

// ======================================================================

namespace Direct3d11_StaticVertexBufferDataNamespace
{
	// Monotonic, never a pointer. DX9 computes its sort key as
	// reinterpret_cast<int>(the D3D buffer pointer), which does not even compile
	// on x64 without truncating, and a truncated pointer is not a stable ordering:
	// two live buffers can collide on the low 32 bits, which silently merges two
	// batches in the sorter.
	//
	// Static keys start at 1. Zero is reserved for the dynamic buffers, which all
	// share a single key because they all share one ring, so a dynamic buffer must
	// never compare equal to a static one.
	int ms_nextSortKey = 1;
}
using namespace Direct3d11_StaticVertexBufferDataNamespace;

// ======================================================================

Direct3d11_StaticVertexBufferData::Direct3d11_StaticVertexBufferData(StaticVertexBuffer const &vertexBuffer)
:
	StaticVertexBufferGraphicsData(),
	m_vertexBuffer(vertexBuffer),
	m_descriptor(Direct3d11_VertexBufferDescriptorMap::getDescriptor(vertexBuffer.getFormat())),
	m_buffer(NULL),
	m_shadow(NULL),
	m_shadowBytes(0),
	m_lockedForWriting(false),
	m_everUploaded(false),
	m_sortKey(ms_nextSortKey++)
{
	int const vertexCount = m_vertexBuffer.getNumberOfVertices();
	int const vertexSize  = static_cast<int>(m_descriptor.vertexSize);

	FATAL(vertexSize <= 0, ("Direct3d11: a static vertex buffer was created with a vertex size of %d.", vertexSize));

	// A zero-vertex buffer is legal to construct in the engine but cannot back a
	// D3D buffer, whose ByteWidth must be positive. One vertex of slack keeps
	// every later size calculation honest without special cases.
	m_shadowBytes = (vertexCount > 0) ? (vertexCount * vertexSize) : vertexSize;
	m_shadow = new uint8[m_shadowBytes];
	memset(m_shadow, 0, m_shadowBytes);

	createBuffer();
}

// ----------------------------------------------------------------------

Direct3d11_StaticVertexBufferData::~Direct3d11_StaticVertexBufferData()
{
	DEBUG_WARNING(m_lockedForWriting, ("Direct3d11: a static vertex buffer was destroyed while still locked."));

	if (m_buffer)
	{
		m_buffer->Release();
		m_buffer = NULL;
	}

	delete [] m_shadow;
	m_shadow = NULL;
}

// ----------------------------------------------------------------------
/**
 * Create the GPU buffer.
 *
 * USAGE_DEFAULT with UpdateSubresource rather than IMMUTABLE, even though most of
 * these are written exactly once. IMMUTABLE would be marginally better for the
 * write-once case and is impossible for the rest: the engine relocks static
 * buffers to modify geometry -- terrain and skeletal paths do -- and an immutable
 * resource cannot be updated at all, so it would have to be recreated, which
 * costs an allocation inside a frame. DEFAULT is the honest choice for a buffer
 * whose write count is not known at construction.
 */

void Direct3d11_StaticVertexBufferData::createBuffer()
{
	ID3D11Device1 * const device = Direct3d11_Device::getDevice();
	NOT_NULL(device);

	D3D11_BUFFER_DESC description;
	Zero(description);
	description.ByteWidth = static_cast<UINT>(m_shadowBytes);
	description.Usage     = D3D11_USAGE_DEFAULT;
	description.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	HRESULT const hresult = device->CreateBuffer(&description, NULL, &m_buffer);
	FATAL(FAILED(hresult) || !m_buffer, ("Direct3d11: a %d-byte static vertex buffer could not be created (%s).", m_shadowBytes, Direct3d11_Device::describeHresult(hresult)));
}

// ======================================================================

VertexBufferDescriptor const &Direct3d11_StaticVertexBufferData::getDescriptor() const
{
	// A reference into the process-lifetime descriptor map. The engine stores this
	// ADDRESS and every vertex iterator dereferences it, so it must not be a
	// member of this object.
	return m_descriptor;
}

// ----------------------------------------------------------------------

int Direct3d11_StaticVertexBufferData::getSortKey()
{
	return m_sortKey;
}

// ======================================================================
/**
 * Hand out the shadow.
 *
 * Always the same pointer, for the whole buffer, whether the lock is for reading
 * or writing -- which is what makes lockReadOnly work at all. readOnly only
 * decides whether unlock has anything to upload.
 */

void *Direct3d11_StaticVertexBufferData::lock(bool readOnly)
{
	NOT_NULL(m_shadow);

	DEBUG_WARNING(m_lockedForWriting, ("Direct3d11: a static vertex buffer was locked while already locked for writing."));

	// A read of a buffer nothing has written yet returns zeroes rather than
	// uninitialised memory. Cheap, and it makes a missing upload look like black
	// geometry rather than like noise that changes between runs.
	DEBUG_WARNING(readOnly && !m_everUploaded, ("Direct3d11: a static vertex buffer was read before anything was written to it."));

	m_lockedForWriting = !readOnly;
	return m_shadow;
}

// ----------------------------------------------------------------------

void Direct3d11_StaticVertexBufferData::unlock()
{
	if (!m_lockedForWriting)
		return;

	m_lockedForWriting = false;

	ID3D11DeviceContext1 * const context = Direct3d11_Device::getContext();
	if (!context || !m_buffer)
		return;

	// The whole buffer, because unlock carries no size and the engine has already
	// discarded the pointer it wrote through, so there is nothing to derive a
	// dirty range from. This is the one place where the engine's lock protocol
	// costs more under D3D11 than it did under D3D9's managed pool.
	context->UpdateSubresource(m_buffer, 0, NULL, m_shadow, 0, 0);

	m_everUploaded = true;
}

// ======================================================================

ID3D11Buffer *Direct3d11_StaticVertexBufferData::getBuffer() const
{
	return m_buffer;
}

// ----------------------------------------------------------------------

int Direct3d11_StaticVertexBufferData::getVertexSize() const
{
	return static_cast<int>(m_descriptor.vertexSize);
}

// ======================================================================
