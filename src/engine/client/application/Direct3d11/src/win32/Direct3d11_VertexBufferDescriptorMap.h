// ======================================================================
//
// Direct3d11_VertexBufferDescriptorMap.h
// copyright (c) 2026 Galaxies Reborn
//
// Vertex format to byte layout, cached for the life of the process.
//
// A VertexBufferDescriptor says where each component sits inside a vertex and how
// big the vertex is. There is no D3D in the computation at all -- it is arithmetic
// over the format flags -- so this is a straight port of
// Direct3d9_VertexBufferDescriptorMap with the same component order, the same
// sizes and the same sentinel of -1 for absent.
//
// The map has to outlive every buffer, and its entries must never move. The
// engine does not copy the descriptor it is handed: StaticVertexBuffer stores the
// ADDRESS of it and every vertex iterator dereferences that pointer on every
// component access. A descriptor stored per buffer object, or in a container that
// can reallocate, becomes a dangling read on the next vertex written.
//
// One hard limit worth stating: every offset and the vertex size itself are int8.
// A vertex therefore cannot exceed 127 bytes, and that is the engine's constraint
// rather than this backend's.
//
// ======================================================================

#ifndef INCLUDED_Direct3d11_VertexBufferDescriptorMap_H
#define INCLUDED_Direct3d11_VertexBufferDescriptorMap_H

// ======================================================================

class VertexBufferFormat;
struct VertexBufferDescriptor;

// ======================================================================

class Direct3d11_VertexBufferDescriptorMap
{
public:

	static void                           install();
	static void                           remove();

	// Both return a reference valid for the life of the process.
	static VertexBufferDescriptor const  &getDescriptor(VertexBufferFormat const &vertexFormat);
	static VertexBufferDescriptor const  &getDescriptor(uint32 formatFlags);

	static int                            getDescriptorCount();

private:

	Direct3d11_VertexBufferDescriptorMap();
	Direct3d11_VertexBufferDescriptorMap(Direct3d11_VertexBufferDescriptorMap const &);
	Direct3d11_VertexBufferDescriptorMap &operator =(Direct3d11_VertexBufferDescriptorMap const &);
};

// ======================================================================

#endif
