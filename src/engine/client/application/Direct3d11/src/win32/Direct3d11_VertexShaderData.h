// ======================================================================
//
// Direct3d11_VertexShaderData.h
// copyright (c) 2026 Galaxies Reborn
//
// One vertex program: its bytecode, its shader object, and the texture-coordinate-set
// tags it declares.
//
// ----------------------------------------------------------------------
// One compiled variant per program, not one per texture-coordinate key
//
// This is the single deliberate structural departure from DX9 in the shader path, so it
// is worth being explicit about.
//
// A vertex program declares its texture coordinate sets by TAG -- MAIN, NRML, DOT3, DTLA
// -- and each material decides which of the vertex buffer's numbered sets a given tag
// reads, through StaticShader::getTextureCoordinateSet. Under D3D9 the vertex inputs were
// fixed registers v7..v14, so which register a tag lived in had to be settled at COMPILE
// time. DX9 therefore packs the per-tag set indices into a 24-bit key
// (Direct3d9_StaticShaderData.cpp:550-575), compiles a separate variant of the program for
// every key it sees, and caches them in a std::map per program -- with a per-draw lookup
// on top, guarded by a one-entry fast path.
//
// D3D11 binds vertex inputs by semantic NAME and INDEX, resolved by the input layout at
// bind time. The remapping the key encodes is exactly what an input layout does. So this
// class compiles the program once against a CANONICAL mapping -- tag i becomes TEXCOORD i
// in declaration order -- and the input layout routes the vertex buffer's set to the
// matching semantic. What the shader computes is unchanged; only which slot it reads from
// is, and the layout compensates.
//
// That removes, permanently: N compiled variants per program, the per-program shader map,
// and the per-draw map lookup. It also removes a failure mode. Because DX9 assigned the
// declared DIMENSION to the set index rather than to the tag, two tags mapping to one set
// with different dimensions collided -- which is what
// Direct3d9_VertexShaderData.cpp:594's "Competing dimensions" assertion guards. Under
// canonical indexing each tag owns its own index, so the collision cannot arise.
//
// Verified before being built: every reachable program in the corpus -- 192 HLSL vertex
// programs and 36 converted from assembly -- compiles under the canonical mapping, and the
// declared dimension follows the tag (DOT3 four components, everything else two)
// independently of any key.
//
// ----------------------------------------------------------------------
// Assembly is refused, by name
//
// A program whose first line says //asm, or which opens with a bare vs.1.1 directive, is
// not HLSL and D3DCompile has no assembler. Those programs are converted offline by
// scripts/asm2hlsl and shipped as HLSL in client-assets. If one still arrives as assembly
// the asset overlay is not deployed, and saying so by name is far more useful than a
// silently missing shader.
//
// ======================================================================

#ifndef INCLUDED_Direct3d11_VertexShaderData_H
#define INCLUDED_Direct3d11_VertexShaderData_H

// ======================================================================

#include "clientGraphics/ShaderImplementation.h"

#include <d3d11_1.h>

// ======================================================================

class Direct3d11_VertexShaderData : public ShaderImplementationPassVertexShaderGraphicsData
{
public:

	typedef stdvector<Tag>::fwd TextureCoordinateSetTags;

public:

	static void install();
	static void remove();

	static int  getLiveInstanceCount();

public:

	explicit Direct3d11_VertexShaderData(ShaderImplementationPassVertexShader const &vertexShader);
	virtual ~Direct3d11_VertexShaderData();

	// The compiled shader, or null when the program could not be built. A null is not
	// fatal here: the draw path drops the draw and counts it, which localises the failure
	// to the material that uses it rather than to startup.
	ID3D11VertexShader       *getVertexShader() const;

	// The tags this program declares, in declaration order. Tag i is TEXCOORD i, which is
	// what lets the input layout route the vertex buffer's sets. Null when the program
	// declares none.
	TextureCoordinateSetTags const *getTextureCoordinateSetTags() const;

	// The bytecode, which CreateInputLayout validates a layout against.
	void const               *getBytecode() const;
	unsigned int              getBytecodeSize() const;

private:

	Direct3d11_VertexShaderData();
	Direct3d11_VertexShaderData(Direct3d11_VertexShaderData const &);
	Direct3d11_VertexShaderData &operator =(Direct3d11_VertexShaderData const &);

	void parseHeader();
	void compile();

private:

	ShaderImplementationPassVertexShader const &m_vertexShader;

	// Where the compilable source starts: past the language marker and the leading
	// #define block, which the runtime replaces with its own macros.
	char const                                 *m_source;
	int                                         m_sourceLength;

	TextureCoordinateSetTags                   *m_textureCoordinateSetTags;

	ID3DBlob                                   *m_bytecode;
	ID3D11VertexShader                         *m_shader;
};

// ======================================================================

inline ID3D11VertexShader *Direct3d11_VertexShaderData::getVertexShader() const
{
	return m_shader;
}

// ----------------------------------------------------------------------

inline Direct3d11_VertexShaderData::TextureCoordinateSetTags const *Direct3d11_VertexShaderData::getTextureCoordinateSetTags() const
{
	return m_textureCoordinateSetTags;
}

// ======================================================================

#endif
