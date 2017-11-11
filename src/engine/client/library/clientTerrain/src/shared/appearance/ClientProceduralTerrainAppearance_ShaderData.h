// ======================================================================
//
// ClientProceduralTerrainAppearance_ShaderData.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ClientProceduralTerrainAppearance_ShaderData_H
#define INCLUDED_ClientProceduralTerrainAppearance_ShaderData_H

#include "clientTerrain/ClientProceduralTerrainAppearance.h"

// ======================================================================

struct ClientProceduralTerrainAppearance::ShaderData
{
public:

	int            numberOfTextures;

	const Texture* textures [4];
	const Texture* normals [4];
	const Texture* alphas [4];
	RotationType m_rotationTypes[4];

	const Shader*  outputShader;

	StaticShader const * inputShaders[4];

	ShaderData();

public:

	bool operator== (const ShaderData& rhs) const;
};

//----------------------------------------------------------------------

inline ClientProceduralTerrainAppearance::ShaderData::ShaderData() :
numberOfTextures(0),
outputShader(0)
{
	for (int i = 0; i < 4; ++i)
	{
		textures[i] = 0;
		normals[i] = 0;
		alphas[i] = 0;
		m_rotationTypes[i] = RT_invalid;
		inputShaders[i] = 0;
	}
}
// ======================================================================

#endif

