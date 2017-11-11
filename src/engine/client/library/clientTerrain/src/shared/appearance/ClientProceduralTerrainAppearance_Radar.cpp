// ======================================================================
//
// ClientProceduralTerrainAppearance_Radar.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientTerrain/FirstClientTerrain.h"
#include "clientTerrain/ClientProceduralTerrainAppearance_Radar.h"

#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/Texture.h"
#include "clientGraphics/TextureList.h"
#include "clientTerrain/ClientProceduralTerrainAppearance_ClientChunk.h"
#include "sharedMath/VectorArgb.h"
#include "sharedTerrain/TerrainQuadTree.h"

#include <algorithm>

// ======================================================================
//-----------------------------------------------------------------

ClientProceduralTerrainAppearance::Radar::Radar (const ClientProceduralTerrainAppearance & cmtat, const TerrainQuadTree & tree, int tilesPerChunk, int aindexOffset) :
m_texture (0),
m_noClipTexture (0),
m_clipShader (0),
m_noClipShader (0),
m_tilesPerChunk (tilesPerChunk),
m_indexOffset (aindexOffset),
m_tree (tree),
m_cmtat (cmtat)
{
}

//-----------------------------------------------------------------

ClientProceduralTerrainAppearance::Radar::~Radar ()
{
	if (m_texture)
	{
		m_texture->release();
		m_texture = 0;
	}
	if (m_noClipTexture)
	{
		m_noClipTexture->release();
		m_noClipTexture = 0;
	}

	if (m_clipShader)
	{
		m_clipShader->release ();
		m_clipShader = 0;
	}
	if (m_noClipShader)
	{
		m_noClipShader->release ();
		m_noClipShader = 0;
	}
}

//-----------------------------------------------------------------
/**
* Render the radar view into a pixel buffer
*/
void ClientProceduralTerrainAppearance::Radar::renderChunksInto (uint8 * pixelData, int pitch, int textureSize, const Vector & origin, const Vector & aextent) const
{

	UNREF (pitch);

	const real denom = aextent.x - origin.x;

	if (denom == CONST_REAL (0))
		return;

	const real texelsPerMeter = static_cast<real> (textureSize) / denom;

	const int num_usable_poles = m_tilesPerChunk * 2;

	const int row_words = pitch / 4;

	TerrainQuadTree::ConstIterator node_iter (m_tree.getTopNode ());

	const TerrainQuadTree::Node * snode = 0;

#define MARK_CHUNK_CORNERS 0 //lint !e1923 //macro could be const var
#if MARK_CHUNK_CORNERS
	std::vector <uint32 *> blackPtrs;
#endif
	const Vector& directionToLight = ClientProceduralTerrainAppearance::getDirectionToLight ();

	//-- iterate through the quadtree
	while ((snode = node_iter.getCurNode ()) != 0)
	{

		const BoxExtent & box = snode->getBoxExtent ();
		const Vector & minExtent = box.getMin ();
		const Vector & maxExtent = box.getMax ();
		UNREF (maxExtent);

		if (maxExtent.x < origin.x ||
			maxExtent.z < origin.z ||
			minExtent.x >= aextent.x ||
			minExtent.z >= aextent.z)
		{
			IGNORE_RETURN (node_iter.advance ());
			continue;
		}

		if (!snode->isSelectedForRender ())
		{
			IGNORE_RETURN (node_iter.descend ());
			continue;
		}

		const ClientChunk * chunk = safe_cast<const ClientChunk *> (snode->getChunk ());

		if (chunk == 0)
		{
			IGNORE_RETURN (node_iter.advance ());
			continue;
		}

		NOT_NULL (chunk);

		Vector worldPos;

		const real poleDistance    = chunk->getChunkWidthInMeters () / num_usable_poles;

		const uint32 * endPtr = reinterpret_cast <uint32 *>(pixelData + (textureSize * pitch));
		UNREF (endPtr);

		const real poleDistance_times_texelsPerMeter = poleDistance * texelsPerMeter;

		worldPos.z = minExtent.z;

		//-- loop through all the usable height poles, mapping each one to one or more texels

		for (int z = 0; z <= num_usable_poles && worldPos.z < aextent.z; ++z, worldPos.z += poleDistance)
		{
			if (worldPos.z + poleDistance < origin.z)
				continue;

			const int heightmap_z = z + m_indexOffset;

			const real world_diff_z_times_texelsPerMeter = (worldPos.z - origin.z) * texelsPerMeter;

			const int start_texture_z = std::max (0,               static_cast<int> (world_diff_z_times_texelsPerMeter));
			const int next_texture_z  = std::min (textureSize - 1, static_cast<int> (world_diff_z_times_texelsPerMeter + poleDistance_times_texelsPerMeter));

			if (next_texture_z < 0 || next_texture_z < start_texture_z)
			{
				WARNING (true, ("An error occured in radar shader generation z. next_texture_z=%d, start_texture_z=%d, world_diff_z_times_texelsPerMeter=%f", 
					next_texture_z, start_texture_z, world_diff_z_times_texelsPerMeter));
				return;
			}

			worldPos.x = minExtent.x;

			//-----------------------------------------------------------------

			for (int x = 0; x <= num_usable_poles && worldPos.x < aextent.x; ++x, worldPos.x += poleDistance)
			{
				if (worldPos.x + poleDistance < origin.x)
					continue;

				const real world_diff_x_times_texelsPerMeter = (worldPos.x - origin.x) * texelsPerMeter;

				const int start_texture_x = std::max (0,               static_cast<int> (world_diff_x_times_texelsPerMeter));
				const int next_texture_x  = std::min (textureSize - 1, static_cast<int> (world_diff_x_times_texelsPerMeter + poleDistance_times_texelsPerMeter));

				if (next_texture_x < 0 || next_texture_x < start_texture_x)
				{
					WARNING (true, ("An error occured in radar shader generation x. next_texture_x=%d, start_texture_x=%d, world_diff_x_times_texelsPerMeter=%f", 
						next_texture_x, start_texture_x, world_diff_x_times_texelsPerMeter));
					return;
				}
		
				// TODO: is this clamp necessary?
				const int heightmap_x = x + m_indexOffset;
								
				//-- transform the lighting into the range [0.5,1.0], to prevent completely black areas
				const real lighting = 0.5f + (directionToLight.dot (chunk->getNormalAt (x, z)) * 0.5f);

				uint32 * ptr = reinterpret_cast<uint32 *>(pixelData + (start_texture_z * pitch) + (start_texture_x * 4));
				

				PackedRgb prgb (chunk->getColorAt (heightmap_x, heightmap_z));
				/*
				const StaticShader * const shader = NON_NULL (static_cast<const StaticShader *>(chunk->getShaderAt (heightmap_x, heightmap_z)));
				UNREF (shader);
				
				Texture * texture = 0;
				if (shader->getTexture (TAG (M,A,I,N), texture))
				{
					Texture::LockData lockData (TF_ARGB_8888, texture->getMipmapLevelCount () - 1, 0, 0, 1, 1, false);
					texture->lockReadOnly (lockData);
					
					const void * const pixelData = lockData.getPixelData ();
					
					const uint32 word = static_cast<const uint32 *>(pixelData) [0];
					
					prgb.r = static_cast<uint8> (static_cast<float>(prgb.r) * static_cast<float>(word & 0x00ff0000) / 255.0f);
					texture->unlock (lockData);
					
				}
				*/

				prgb.r = static_cast<uint8> (static_cast<float>(prgb.r) * lighting);
				prgb.g = static_cast<uint8> (static_cast<float>(prgb.g) * lighting);
				prgb.b = static_cast<uint8> (static_cast<float>(prgb.b) * lighting);

#if MARK_CHUNK_CORNERS
				if (x == 0 || z == 0)
					blackPtrs.push_back (ptr);
#endif

				//-- fill in all channels of the appropriate texels

 				const uint32 wordValue = 0xff000000 | (prgb.r << 16) | (prgb.g << 8) | prgb.b;

				for (int i = start_texture_x; i <= next_texture_x; ++i, ++ptr)
				{
					uint32 * bleed_ptr = ptr;

					for (int j = start_texture_z; j <= next_texture_z; ++j, bleed_ptr += row_words)
					{
						*bleed_ptr = wordValue;
					}
				}

				DEBUG_FATAL (ptr > endPtr, ("doh!"));
			}
		}
		
		IGNORE_RETURN (node_iter.advance ());
	}
#if MARK_CHUNK_CORNERS

	for (std::vector<uint8 *>::iterator iter = blackPtrs.begin (); iter != blackPtrs.end (); ++iter)
	{
		*(*iter) = 0xff000000;
	}
#endif
}

//----------------------------------------------------------------------

Shader * ClientProceduralTerrainAppearance::Radar::createShader (int textureSize, bool clip, Texture *& texture) const
{
	Shader* currentShader   = 0;
	Texture* currentTexture = 0;

	if (clip)
	{
		if (m_clipShader == 0)
			m_clipShader = NON_NULL (ShaderTemplateList::fetchModifiableShader("shader/uicanvas_radar.sht"));

		currentShader  = m_clipShader;
		currentTexture = m_texture;
	}
	else
	{
		if (m_noClipShader == 0)
			m_noClipShader = NON_NULL (ShaderTemplateList::fetchModifiableShader("shader/uicanvas_filtered.sht"));

		currentShader  = m_noClipShader;
		currentTexture = m_noClipTexture;
	}

	//--
	const TextureFormat runtimeTextureFormat = TF_ARGB_8888;

	if (currentTexture == 0 || currentTexture->getWidth () != textureSize)
	{ 
		if (currentTexture)
			currentTexture->release();

		currentTexture = TextureList::fetch (0, textureSize, textureSize, 1, &runtimeTextureFormat, 1);

		safe_cast<StaticShader*>(currentShader)->setTexture(TAG(M,A,I,N), *currentTexture);

		if(clip)
			m_texture = currentTexture;
		else
			m_noClipTexture = currentTexture;
	}

	texture = currentTexture;
	return currentShader;
}

//-----------------------------------------------------------------
/**
* Create a new radar shader
*/

void ClientProceduralTerrainAppearance::Radar::createShader (const Vector & center, real range, int textureSize, RadarShaderInfo & rinfo, bool clip, const VectorArgb & clearColor, bool drawTerrain) const
{
	UNREF (center);
	UNREF (range);

#define DO_TIMING 0 //lint !e1923 //macro could be const var
#if DO_TIMING
	PerformanceTimer timer;
	timer.start ();
#endif

	Texture * currentTexture = 0;
	Shader * const currentShader   = createShader (textureSize, clip, currentTexture);
	NOT_NULL (currentShader);
	NOT_NULL (currentTexture);

	TextureGraphicsData::LockData lockData (TF_ARGB_8888, 0, 0, 0, textureSize, textureSize, false);
	
	currentTexture->lock (lockData);

	uint8 * const pixelData = reinterpret_cast<uint8 *>(lockData.getPixelData());

	// clear
	{
		const uint32 u_clearColor = clearColor.convertToUint32 ();
		uint8 * pixelDataPtr = pixelData;

		for (int row = 0; row < lockData.getHeight (); ++row, pixelDataPtr += lockData.getPitch ())
		{
			uint32 * pixelRow = reinterpret_cast<uint32 *>(pixelDataPtr);

			for (int col = 0; col < lockData.getWidth (); ++col)
				*(pixelRow++) = u_clearColor;
		}
	}
	
	// origin in worldspace
	const Vector origin  (center.x - range, 0.0f, center.z - range);

	if (drawTerrain)
	{
		const Vector aextent (center.x + range, 0.0f, center.z + range);	
		//-- get the work done
		renderChunksInto (pixelData, lockData.getPitch (), textureSize, origin, aextent);
	}

	currentTexture->unlock (lockData);

	currentShader->fetch ();

	//-- return the results
	rinfo.m_shader = currentShader;
	rinfo.m_origin = origin;
	
#if DO_TIMING
	timer.stop ();
	const real endTime = timer.getElapsedTime ();
	REPORT_LOG (true, ("radar shader took %f total\n", endTime));
#endif

}

// ======================================================================
