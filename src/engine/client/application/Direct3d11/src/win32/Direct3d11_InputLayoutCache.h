// ======================================================================
//
// Direct3d11_InputLayoutCache.h
// copyright (c) 2026 Galaxies Reborn
//
// Vertex formats plus a vertex shader signature, resolved to an ID3D11InputLayout
// and kept.
//
// This is the one piece of the buffer path with no D3D9 counterpart in shape. D3D9
// matched a vertex declaration to a shader by SEMANTIC, at draw time, and tolerated
// a shader declaring an input the bound buffer did not supply -- the engine even
// has machinery for that case, so it demonstrably happens in shipped data. D3D11
// validates a layout against specific shader bytecode at creation, and
// CreateInputLayout fails outright when an input is unsatisfied.
//
// So the key is (the formats of every bound stream, the shader's input signature).
// The signature, not the bytecode: two shaders differing only in their bodies share
// a layout, and keying on a blob pointer instead would alias a freed blob's address
// onto a live entry.
//
// The cache owns a reference to every layout it hands out and never returns one it
// does not own, because the engine buffer objects that triggered the creation are
// routinely stack locals that are destroyed before the draw.
//
// ======================================================================

#ifndef INCLUDED_Direct3d11_InputLayoutCache_H
#define INCLUDED_Direct3d11_InputLayoutCache_H

// ======================================================================

#include <d3d11_1.h>

// ======================================================================

class Direct3d11_InputLayoutCache
{
public:

	enum
	{
		// The vector path binds at most this many streams. D3D11 allows far more;
		// the engine has never used more than two.
		MAX_STREAMS = 4,
		MAX_TEXTURE_COORDINATE_SETS = 8
	};

public:

	static void                install();
	static void                remove();

	// formatFlags holds one VertexBufferFormat::getFlags() per bound stream, in
	// slot order. Returns null only if the layout genuinely cannot be built, having
	// already said why.
	// textureCoordinateSetMapping says, for each texture coordinate set TAG the shader
	// declares in order, which of the vertex buffer's numbered sets it reads. That is the
	// information DX9 baked into the shader as a compile-time key; keeping it here instead
	// is what lets one compiled program serve every material -- see
	// Direct3d11_VertexShaderData.h. Pass a count of zero for a program that addresses sets
	// by number rather than by tag, and the sets bind in their natural order.
	static ID3D11InputLayout  *getInputLayout(uint32 const *formatFlags, int streamCount, void const *vertexShaderBytecode, unsigned int vertexShaderBytecodeSize, int const *textureCoordinateSetMapping, int mappingCount);

	static int                 getLayoutCount();

private:

	Direct3d11_InputLayoutCache();
	Direct3d11_InputLayoutCache(Direct3d11_InputLayoutCache const &);
	Direct3d11_InputLayoutCache &operator =(Direct3d11_InputLayoutCache const &);
};

// ======================================================================

#endif
