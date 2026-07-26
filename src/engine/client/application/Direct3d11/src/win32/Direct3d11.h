// ======================================================================
//
// Direct3d11.h
// copyright (c) 2026 Galaxies Reborn
//
// The Direct3D 11 raster backend. Loaded by clientGraphics as gl11_{r,o,d}.dll
// when [ClientGraphics] rasterMajor is 11, and reached only through the Gl_api
// function table this file's GetApi returns.
//
// ======================================================================

#ifndef INCLUDED_Direct3d11_H
#define INCLUDED_Direct3d11_H

class StaticShader;
class StaticShaderGraphicsData;

// ======================================================================

struct Gl_install;
class  HardwareVertexBuffer;
class  HardwareIndexBuffer;
class  VertexBufferVector;

// ======================================================================

class Direct3d11
{
public:

	static bool install(Gl_install *gl_install);

	// These four are members rather than namespace functions for one reason: they
	// read m_graphicsData out of the engine's buffer objects, and the engine
	// befriends `class Direct3d11`, not free functions in a namespace. DX9 has the
	// same three for the same reason and says so in a comment.
	static void setVertexBuffer(HardwareVertexBuffer const &vertexBuffer);
	static void setVertexBufferVector(VertexBufferVector const &vertexBufferVector);
	static void setIndexBuffer(HardwareIndexBuffer const &indexBuffer);

	// Recorded by the static shader data when it binds a vertex shader, and
	// consumed at draw time to resolve an input layout. DX9 could pick its vertex
	// declaration when the buffer was bound, because a declaration depends only on
	// the vertex format; a D3D11 input layout is validated against specific shader
	// bytecode, so it cannot be resolved until both are known.
	static void setCurrentVertexShaderBytecode(void const *bytecode, unsigned int size);
	static void setCurrentTextureCoordinateSetMapping(int const *mapping, int count);
	static float getCurrentTimeValue();

	// StaticShader keeps its graphics data private and names class Direct3d11 a friend.
	// A namespace function has no friend access, so the reach-through lives here.
	static StaticShaderGraphicsData *getStaticShaderGraphicsData(StaticShader const &shader);

	// The single pre-draw chokepoint every draw entry point goes through. Returns
	// false when the draw must be skipped, having counted and explained it.
	static bool prepareToDraw();

	// Slice state, written only by the three bind functions above.
	static int  getSliceNumberOfVertices();
	static int  getSliceFirstVertex();
	static int  getSliceNumberOfIndices();
	static int  getSliceFirstIndex();

private:

	Direct3d11();
	Direct3d11(Direct3d11 const &);
	Direct3d11 &operator =(Direct3d11 const &);
};

// ======================================================================

#endif
