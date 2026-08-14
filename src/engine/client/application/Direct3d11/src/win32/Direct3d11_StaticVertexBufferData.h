// ======================================================================
//
// Direct3d11_StaticVertexBufferData.h
// copyright (c) 2026 Galaxies Reborn
//
// A vertex buffer whose contents are written once, or occasionally, and read back
// by the engine at arbitrary times.
//
// That last part decides the design. StaticVertexBuffer::lockReadOnly exists and
// is used at runtime to read geometry that was loaded long ago, so the CPU has to
// be able to see the buffer's current contents on demand. D3D9 got that for free
// from D3DPOOL_MANAGED, which keeps a system-memory copy the runtime can hand back
// on any Lock. D3D11 has no managed pool, and a USAGE_DEFAULT buffer cannot be
// mapped at all.
//
// So this class keeps its own shadow for the buffer's lifetime and returns it from
// every lock, pushing it to the GPU on unlock only when the lock was writable.
// That is one system-memory copy per static buffer, which is exactly what
// D3DPOOL_MANAGED was already costing -- the memory is not new, it has simply
// become visible.
//
// A note on what this deliberately does NOT do. The prior DX11 attempt also kept a
// shadow, but re-uploaded the WHOLE buffer with UpdateSubresource on every unlock
// regardless of whether the lock was writable, and kept the shadow uninitialised
// until first use. Immutable buffers, which most of these are, then paid a full
// upload for a read.
//
// ======================================================================

#ifndef INCLUDED_Direct3d11_StaticVertexBufferData_H
#define INCLUDED_Direct3d11_StaticVertexBufferData_H

// ======================================================================

#include "clientGraphics/StaticVertexBuffer.h"

#include <d3d11_1.h>

// ======================================================================

class Direct3d11_StaticVertexBufferData : public StaticVertexBufferGraphicsData
{
public:
	explicit Direct3d11_StaticVertexBufferData(StaticVertexBuffer const &vertexBuffer);
	virtual ~Direct3d11_StaticVertexBufferData();

	virtual VertexBufferDescriptor const &getDescriptor() const;
	virtual int getSortKey();

	virtual void *lock(bool readOnly);
	virtual void unlock();

	// Reached by the draw path through class Direct3d11, which is the friend.
	ID3D11Buffer *getBuffer() const;
	int getVertexSize() const;

private:
	Direct3d11_StaticVertexBufferData();
	Direct3d11_StaticVertexBufferData(Direct3d11_StaticVertexBufferData const &);
	Direct3d11_StaticVertexBufferData &operator=(Direct3d11_StaticVertexBufferData const &);

	void createBuffer();

private:
	StaticVertexBuffer const &m_vertexBuffer;
	VertexBufferDescriptor const &m_descriptor;

	ID3D11Buffer *m_buffer;

	// Lives as long as this object, because a read-only lock can come at any time.
	uint8 *m_shadow;
	int m_shadowBytes;

	bool m_lockedForWriting;
	bool m_everUploaded;

	int m_sortKey;
};

// ======================================================================

#endif
