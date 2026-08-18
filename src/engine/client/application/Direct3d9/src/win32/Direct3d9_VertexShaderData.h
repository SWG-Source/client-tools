// ======================================================================
//
// Direct3d9_VertexShaderData.h
// Copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_Direct3d9_VertexShaderData_H
#define INCLUDED_Direct3d9_VertexShaderData_H

#ifdef VSPS

// ======================================================================

struct IDirect3DVertexShader9;

#include "clientGraphics/ShaderImplementation.h"

// ======================================================================

class Direct3d9_VertexShaderData : public ShaderImplementationPassVertexShaderGraphicsData
{
public:

	typedef stdvector<Tag>::fwd TextureCoordinateSetTags;

public:

	static void install();
	static void remove();

public:

	Direct3d9_VertexShaderData(ShaderImplementationPassVertexShader const & VertexShader);
	virtual ~Direct3d9_VertexShaderData();

	IDirect3DVertexShader9 * getVertexShader(uint32 textureCoordinateSetKey) const;

	TextureCoordinateSetTags const * getTextureCoordinateSetTags() const;

private:

	typedef stdmap<uint32, IDirect3DVertexShader9 *>::fwd Container;

private:

	IDirect3DVertexShader9 * createVertexShader(uint32 textureCoordinateSetKey) const;

private:

	ShaderImplementation::Pass::VertexShader const * m_vertexShader;

	bool                                             m_hlsl;
	char                                             m_hlslTarget[8];
	const char *                                     m_compileText;
	int                                              m_compileTextLength;
	TextureCoordinateSetTags *                       m_textureCoordinateSetTags;

	mutable Container *                              m_container;
	mutable IDirect3DVertexShader9 *                 m_nonPatchedVertexShader;

	mutable uint32                                   m_lastRequestedKey;
	mutable IDirect3DVertexShader9 *                 m_lastReturnedValue;
};

// ======================================================================

inline Direct3d9_VertexShaderData::TextureCoordinateSetTags const * Direct3d9_VertexShaderData::getTextureCoordinateSetTags() const
{
	return m_textureCoordinateSetTags;
}

// ======================================================================

#endif
#endif
