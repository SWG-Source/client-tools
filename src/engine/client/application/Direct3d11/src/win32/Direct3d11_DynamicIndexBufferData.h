// ======================================================================
//
// Direct3d11_DynamicIndexBufferData.h
// copyright (c) 2026 Galaxies Reborn
//
// The dynamic index ring. Same shape and same reasoning as the dynamic vertex
// ring: one process-lifetime buffer, per-lock windows onto it, and an object that
// owns nothing because the engine builds these as stack locals and lets them die
// before the draw.
//
// Two differences from the vertex side, both from the interface rather than from
// choice. There is no capacity query at all -- DynamicIndexBufferGraphicsData has
// only lock and unlock -- so callers assume a size, and at least one assumes 32768
// indices are available. And the offset the draw path consumes is counted in
// INDICES, not bytes.
//
// Indices are 16 bit everywhere in the engine, so the ring is R16_UINT and its
// capacity is stated in indices throughout.
//
// ======================================================================

#ifndef INCLUDED_Direct3d11_DynamicIndexBufferData_H
#define INCLUDED_Direct3d11_DynamicIndexBufferData_H

// ======================================================================

#include "clientGraphics/DynamicIndexBuffer.h"

#include <d3d11_1.h>

// ======================================================================

class Direct3d11_DynamicIndexBufferData : public DynamicIndexBufferGraphicsData
{
public:

	static void   install();
	static void   remove();
	static void   beginFrame();

	static ID3D11Buffer *getRing();

public:

	Direct3d11_DynamicIndexBufferData();
	virtual ~Direct3d11_DynamicIndexBufferData();

	virtual Index  *lock(int numberOfIndices);
	virtual void    unlock();

	// Read by the draw path through class Direct3d11. In INDICES.
	int             getOffset() const;
	int             getNumberOfIndices() const;

private:

	Direct3d11_DynamicIndexBufferData(Direct3d11_DynamicIndexBufferData const &);
	Direct3d11_DynamicIndexBufferData &operator =(Direct3d11_DynamicIndexBufferData const &);

private:

	int   m_offset;
	int   m_numberOfIndices;
	bool  m_locked;
};

// ======================================================================

#endif
