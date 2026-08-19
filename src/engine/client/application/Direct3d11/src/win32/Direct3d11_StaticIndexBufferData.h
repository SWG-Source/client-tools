// ======================================================================
//
// Direct3d11_StaticIndexBufferData.h
// copyright (c) 2026 Galaxies Reborn
//
// Indices written once, or occasionally, and read back on demand.
//
// Same shape and same reasoning as the static vertex buffer: the engine's
// lockReadOnly hands the caller a const Index pointer over the buffer's current
// contents, so a system-memory copy has to exist for the buffer's lifetime.
//
// Indices are 16 bit throughout the engine -- Graphics.def declares
// `typedef unsigned short Index` -- so the format is R16_UINT everywhere and a
// vertex count above 65535 is not addressable by any index buffer. That is not a
// backend choice to revisit; it is a property of the asset format and of every
// index the engine holds.
//
// ======================================================================

#ifndef INCLUDED_Direct3d11_StaticIndexBufferData_H
#define INCLUDED_Direct3d11_StaticIndexBufferData_H

// ======================================================================

#include "clientGraphics/StaticIndexBuffer.h"

#include <d3d11_1.h>

// ======================================================================

class Direct3d11_StaticIndexBufferData : public StaticIndexBufferGraphicsData
{
public:
	explicit Direct3d11_StaticIndexBufferData(StaticIndexBuffer const &indexBuffer);
	virtual ~Direct3d11_StaticIndexBufferData();

	virtual Index *lock(bool readOnly);
	virtual void unlock();

	ID3D11Buffer *getBuffer() const;
	int getNumberOfIndices() const;

private:
	Direct3d11_StaticIndexBufferData();
	Direct3d11_StaticIndexBufferData(Direct3d11_StaticIndexBufferData const &);
	Direct3d11_StaticIndexBufferData &operator=(Direct3d11_StaticIndexBufferData const &);

	void createBuffer();

private:
	ID3D11Buffer *m_buffer;

	Index *m_shadow;
	int m_numberOfIndices;

	bool m_lockedForWriting;
	bool m_everUploaded;
};

// ======================================================================

#endif
