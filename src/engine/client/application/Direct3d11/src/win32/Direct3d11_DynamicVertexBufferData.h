// ======================================================================
//
// Direct3d11_DynamicVertexBufferData.h
// copyright (c) 2026 Galaxies Reborn
//
// A window onto one process-lifetime ring, shared by every dynamic vertex buffer
// in the engine.
//
// These objects own nothing. Eleven engine call sites construct a
// DynamicVertexBuffer as a STACK LOCAL, lock it, fill it, unlock it, bind it, and
// let it die before the draw is ever issued -- ShadowVolume, ClientTerrainSorter,
// NebulaShellShaderPrimitive, ShaderAppearance, ReticleManager, DustAppearance,
// GameCamera, OverheadMap and others. So the ring is static, the destructor
// releases nothing, and every parameter the draw needs is snapshotted into
// device-level state by setVertexBuffer rather than read back from this object
// later.
//
// lock() returns a pointer into a CPU scratch mirror, not into a mapped resource,
// and unlock() copies the slice into the ring. That is not a simplification -- it
// is forced:
//
//   The UI holds a lock open across unrelated engine work. CuiLayerRenderer locks
//   the shared ring, accumulates geometry across many calls including a shader
//   fetch, and unlocks much later. A Map held open that long is not viable.
//
//   Two dynamic vertex buffers can be locked simultaneously in a release build.
//   CuiLayer_EngineCanvas::RenderLineStrip builds its own while the renderer's
//   batch is still open; the engine DEBUG_FATALs it and ships past it.
//
//   Dynamic buffers are provably write-only -- their iterator has no getters and
//   the header says so -- so nothing needs to read back what a lock wrote.
//
// It also makes unlock(int) trivially correct: copy exactly the vertices claimed,
// which is what the UI depends on, since it locks all remaining space and unlocks
// with the fraction it actually filled.
//
// ======================================================================

#ifndef INCLUDED_Direct3d11_DynamicVertexBufferData_H
#define INCLUDED_Direct3d11_DynamicVertexBufferData_H

// ======================================================================

#include "clientGraphics/DynamicVertexBuffer.h"

#include <d3d11_1.h>

// ======================================================================

class Direct3d11_DynamicVertexBufferData : public DynamicVertexBufferGraphicsData
{
public:
	static void install();
	static void remove();

	// Marks the ring for a discard on the next lock, which is what makes a frame's
	// first lock start from the beginning.
	static void beginFrame();

	static ID3D11Buffer *getRing();

public:
	explicit Direct3d11_DynamicVertexBufferData(VertexBuffer const &vertexBuffer);
	virtual ~Direct3d11_DynamicVertexBufferData();

	virtual void *lock(int numberOfVertices, bool forceDiscard);
	virtual void unlock();
	virtual void unlock(int numberOfVertices);
	virtual VertexBufferDescriptor const &getDescriptor() const;
	virtual int getNumberOfLockableDynamicVertices(bool withDiscard);
	virtual int getSortKey();

	// Read by the draw path through class Direct3d11.
	int getOffset() const;
	int getNumberOfVertices() const;
	int getVertexSize() const;

private:
	Direct3d11_DynamicVertexBufferData();
	Direct3d11_DynamicVertexBufferData(Direct3d11_DynamicVertexBufferData const &);
	Direct3d11_DynamicVertexBufferData &operator=(Direct3d11_DynamicVertexBufferData const &);

private:
	VertexBufferDescriptor const &m_descriptor;

	// Offset in VERTICES, which is what the draw's base-vertex argument wants.
	int m_offset;
	int m_numberOfVertices;

	// Byte offset the unlock copy targets, and whether that copy renames.
	int m_lockByteOffset;
	bool m_locked;
};

// ======================================================================

#endif
