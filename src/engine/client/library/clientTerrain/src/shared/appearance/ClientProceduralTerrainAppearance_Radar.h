// ======================================================================
//
// ClientProceduralTerrainAppearance_Radar.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ClientProceduralTerrainAppearance_Radar_H
#define INCLUDED_ClientProceduralTerrainAppearance_Radar_H

#include "clientTerrain/ClientProceduralTerrainAppearance.h"

class Shader;
class Texture;

// ======================================================================

class ClientProceduralTerrainAppearance::Radar
{
public:

	void          createShader (const Vector & center, real range, int maxSize, RadarShaderInfo & rinfo, bool clip, const VectorArgb & clearColor, bool drawTerrain = true) const;
	void          renderChunksInto (uint8 * pixelData, int pitch, int textureSize, const Vector & origin, const Vector & extent) const;
	              Radar (const ClientProceduralTerrainAppearance & cmtat, const TerrainQuadTree & tree, int tilesPerChunk, int indexOffset);
	             ~Radar ();

private:
	                      Radar ();
	                      Radar (const Radar & rhs);
	Radar &               operator=    (const Radar & rhs);

private:

	Shader *      createShader (int textureSize, bool clip, Texture *& texture) const;

	mutable Texture *                          m_texture;
	mutable Texture *                          m_noClipTexture;
	mutable Shader *                           m_clipShader;
	mutable Shader *                           m_noClipShader;
	int                                        m_tilesPerChunk;
	int                                        m_indexOffset;
	const TerrainQuadTree &                    m_tree;
	const ClientProceduralTerrainAppearance & m_cmtat;
};

// ======================================================================

#endif
