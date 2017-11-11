// ======================================================================
//
// ClientProceduralTerrainAppearance_ShaderCache.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientTerrain/FirstClientTerrain.h"
#include "clientTerrain/ClientProceduralTerrainAppearance_ShaderCache.h"

#include "clientGraphics/Material.h"
#include "clientGraphics/ShaderEffect.h"
#include "clientGraphics/ShaderEffectList.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/Texture.h"
#include "clientGraphics/TextureList.h"
#include "clientTerrain/ClientProceduralTerrainAppearance_ShaderData.h"
#include "clientTerrain/ConfigClientTerrain.h"
#include "sharedMath/VectorArgb.h"
#include "sharedUtility/FileName.h"

#include <algorithm>
#include <cstdio>

//-------------------------------------------------------------------

namespace ClientProceduralTerrainAppearanceShaderCacheNamespace
{
	Tag const TAG_SPEC = TAG(S,P,E,C);  // plain specular
	Tag const TAG_SPCM = TAG(S,P,C,M);  // specular map
	Tag const TAG_EMSM = TAG(E,M,S,M);  // emissive map

	Tag const TAG_MAIN = TAG(M,A,I,N);  // main material
	
	//----------------------------------------------------------------------
	
	/**
	* @retval whether specular was found or not
	*/
	
	bool setupMaterial(Material & material, StaticShader const & shader)
	{
		bool specularAvailable = ClientProceduralTerrainAppearance::getSpecularTerrainEnabled() && ClientProceduralTerrainAppearance::getSpecularTerrainCapable();
		
		bool specular = false;
		
		Material mat;
		bool const matFound = shader.getMaterial(TAG_MAIN, mat);

		if (matFound)
			material = mat;

		if (specularAvailable &&
			(shader.hasOptionTag(TAG_SPEC) || 
			shader.hasOptionTag(TAG_SPCM)))
		{
			static VectorArgb const specularColorDefault(0.01f, 0.3f, 0.3f, 0.3f);
			static float const specularPowerDefault = 16.0f;

			specular = true;			
			if (!matFound)
			{
				material.setSpecularColor(specularColorDefault);
				material.setSpecularPower(specularPowerDefault);
			}
			else
			{
				//-- default fallback bloom alpha is 1.0.  If this setting exists, use our default alpha.
				if (material.getSpecularColor().a == 1.0f)
				{
					VectorArgb spec = material.getSpecularColor();
					spec.a = specularColorDefault.a;
					material.setSpecularColor(spec);
				}

				//-- default fallback power is 0.0.  If this setting exists, use our default power.
				if (material.getSpecularPower() == 0.0f)
					material.setSpecularPower(specularPowerDefault);
			}
			
		}
		
		if (shader.hasOptionTag(TAG_EMSM))
		{
			if (!matFound || material.getEmissiveColor() == VectorArgb::solidBlack)
				material.setEmissiveColor(VectorArgb::solidWhite);				
		}
		
		return specular;
	}

	//----------------------------------------------------------------------
}

using namespace ClientProceduralTerrainAppearanceShaderCacheNamespace;

//----------------------------------------------------------------------


ClientProceduralTerrainAppearance::ShaderCache::ShaderCache (const ShaderGroup& newShaderGroup) :
	shaderGroup (newShaderGroup),
	cache (0),
	maxNumberOfChildren (0),
	maxNumberOfFamilies (0),
	m_defaultTexture (TextureList::fetchDefaultTexture ()),
	m_defaultNormal (TextureList::fetchDefaultNormalTexture ()),
	dot3Effect (0),
	nodeList (),
	m_whiteTexture(TextureList::fetchDefaultWhiteTexture()),
	m_blackTexture(TextureList::fetchDefaultBlackTexture())
{
	maxNumberOfFamilies = shaderGroup.getNumberOfFamilies ();
	maxNumberOfChildren = 0;

	int i;
	for (i = 0; i < maxNumberOfFamilies; i++)
		maxNumberOfChildren = std::max (shaderGroup.getNumberOfChildren (i), maxNumberOfChildren);

	cache = new ShaderCacheNode* [maxNumberOfFamilies];  //lint !e737  // loss of sign
	NOT_NULL (cache);

	for (i = 0; i < maxNumberOfFamilies; i++)
	{
		cache [i] = new ShaderCacheNode [maxNumberOfChildren];  //lint !e737  // loss of sign

		int j;
		for (j = 0; j < maxNumberOfChildren; j++)
		{
			cache [i][j].diffuseTexture = 0;
			cache [i][j].normalTexture  = 0;
			cache[i][j].shader = 0;
		}
	}

	{
		for (i = 0; i < 4; i++)
		{
			blendingShader[i] = 0;			
			blendingShaderSpecular[i] = 0;
		}
	}

	// this effect is used by all the terrain shaders whether or not dot3 terrain is being rendered.
	// preload it here to avoid lots of redundant file loads
	dot3Effect = ShaderEffectList::fetch("effect/dot3_terrain.eft");

	blendingOneQuarterShader   = ShaderTemplateList::fetchShader ("shader/blnd_onequarter.sht");
	blendingOneHalfShader      = ShaderTemplateList::fetchShader ("shader/blnd_onehalf.sht");
	blendingThreeQuarterShader = ShaderTemplateList::fetchShader ("shader/blnd_threequarter.sht");
	
	{
		bool const specular = ClientProceduralTerrainAppearance::getSpecularTerrainCapable();
		bool const dot3 = ClientProceduralTerrainAppearance::getDot3Terrain ();
		
		//-- load blend tiles
		for (int i = 0; i < 4; i++)
		{
			char shaderName [64];
			sprintf (shaderName , "shader/terrain_%sblend%i.sht", dot3 ? "dot3_" : "", i);
			blendingShader [i] = safe_cast<StaticShader*>(ShaderTemplateList::fetchModifiableShader (shaderName));
			blendingShader [i]->setObeysLightScale (ConfigClientTerrain::getEnableLightScaling ());
			
			if (specular)
			{
				sprintf (shaderName , "shader/terrain_%sblend%i_spec.sht", dot3 ? "dot3_" : "", i);
				blendingShaderSpecular[i] = safe_cast<StaticShader*>(ShaderTemplateList::fetchModifiableShader (shaderName));
				blendingShaderSpecular[i]->setObeysLightScale (ConfigClientTerrain::getEnableLightScaling ());
			}
			else
				blendingShaderSpecular[i] = NULL;
		}
	}


	//-- preload shaders
	// @todo avoid loading textures for entire planet up front
	preloadShaders ();
}

//-------------------------------------------------------------------

ClientProceduralTerrainAppearance::ShaderCache::~ShaderCache ()
{
	if (m_whiteTexture)
		m_whiteTexture->release();
	if (m_blackTexture)
		m_blackTexture->release();

	m_whiteTexture = 0;
	m_blackTexture = 0;

	//-- delete shaders
	int i;
	for (i = 0; i < maxNumberOfFamilies; i++)
	{
		int j;
		for (j = 0; j < maxNumberOfChildren; j++)
		{
			ShaderCacheNode &c = cache [i][j];

			if (c.shader)
				c.shader->release();
			if (c.diffuseTexture)
				c.diffuseTexture->release();
			if (c.normalTexture)
				c.normalTexture->release();
		}

		delete [] cache [i];
	}

	//-- delete family cache
	delete [] cache;
	cache = 0;

	//-- delete blended shaders
	for (i = 0; i < nodeList.getNumberOfElements (); i++)
		nodeList [i].shader->release();
	nodeList.clear ();

	blendingOneQuarterShader->release();
	blendingOneQuarterShader = 0;

	blendingOneHalfShader->release();
	blendingOneHalfShader = 0;

	blendingThreeQuarterShader->release();
	blendingThreeQuarterShader = 0;

	for (i = 0; i < 4; i++)
	{
		blendingShader [i]->release();
		blendingShader [i] = 0;

		if (blendingShaderSpecular[i])
		{
			blendingShaderSpecular[i]->release();
			blendingShaderSpecular[i] = 0;
		}

	}

	if (m_defaultTexture)
		m_defaultTexture->release();

	if (m_defaultNormal)
		m_defaultNormal->release();

	if (dot3Effect)
	{
		dot3Effect->release();
		dot3Effect= 0;
	}
}

//-------------------------------------------------------------------

const ShaderGroup& ClientProceduralTerrainAppearance::ShaderCache::getShaderGroup () const
{
	return shaderGroup;
}

//-------------------------------------------------------------------

const Shader* ClientProceduralTerrainAppearance::ShaderCache::getBlendingOneQuarterShader () const
{
	return blendingOneQuarterShader;
}

//-------------------------------------------------------------------

const Shader* ClientProceduralTerrainAppearance::ShaderCache::getBlendingOneHalfShader () const
{
	return blendingOneHalfShader;
}

//-------------------------------------------------------------------

const Shader* ClientProceduralTerrainAppearance::ShaderCache::getBlendingThreeQuarterShader () const
{
	return blendingThreeQuarterShader;
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::ShaderCache::getTextures (const ShaderGroup::Info& sgi, StaticShader const *& shader, const Texture *&diffuseTexture, const Texture *&normalTexture) const
{
	shader = 0;
	diffuseTexture = 0;
	normalTexture = 0;

	if (sgi.getFamilyId () == 0)
	{
		diffuseTexture = m_defaultTexture;

		if (ClientProceduralTerrainAppearance::getDot3Terrain ())
			normalTexture = m_defaultNormal;

		return;
	}

	const int childIndex = ConfigClientTerrain::getShaderGroupUseFirstChildOnly () ? 0 : shaderGroup.createShader (sgi);

	const ShaderCacheNode &c = cache[sgi.getPriority ()][childIndex];

	shader = c.shader;
	diffuseTexture = c.diffuseTexture;
	normalTexture = c.normalTexture;
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::ShaderCache::preloadShaders () const
{
	int i;
	int j;
	for (i = 0; i < shaderGroup.getNumberOfFamilies (); ++i)
	{
		for (j = 0; j < shaderGroup.getNumberOfChildren (i); ++j)
		{
			ShaderCacheNode &c = cache[i][j];

			if (!c.diffuseTexture)
			{
				ShaderGroup::FamilyChildData fcd = shaderGroup.getChild (i, j);

				StaticShader const * staticShader = 
					 safe_cast<const StaticShader*>(ShaderTemplateList::fetchShader (FileName (FileName::P_shader, fcd.shaderTemplateName)));

				c.shader = staticShader;

				bool result = staticShader->getTexture(TAG(M,A,I,N), c.diffuseTexture);
				DEBUG_FATAL(!result, ("Could not get texture from static shader"));
				c.diffuseTexture->fetch();

				if (ClientProceduralTerrainAppearance::getDot3Terrain ())
				{
					result = staticShader->getTexture(TAG(N,R,M,L), c.normalTexture);
					if (!result)
						c.normalTexture = m_defaultNormal;
					c.normalTexture->fetch();
				}
			}
		}
	}
}

//-------------------------------------------------------------------

Shader const * ClientProceduralTerrainAppearance::ShaderCache::findCachedShader(ShaderData const & shaderData) const
{
	for (int i = 0; i < nodeList.getNumberOfElements (); i++)
	{
		BlendedShaderCacheNode& node = nodeList [i];
		
		if (node.numberOfTextures == shaderData.numberOfTextures)
		{
			bool found = true;
						
			//-- look for differences
			int j;
			for (j = 0; j < shaderData.numberOfTextures; ++j)
			{
				if (node.alphas[j]   != shaderData.alphas[j] ||
					node.normals[j]  != shaderData.normals[j] ||
					node.textures[j] != shaderData.textures[j])
				{
					found = false;
					break;
				}
			}
			
			
			if (found)
			{
				++node.referenceCount;
				node.timeout = 0.0f;
				return node.shader;
			}
		}
	}
	return NULL;
}

//-------------------------------------------------------------------

const Shader* ClientProceduralTerrainAppearance::ShaderCache::createBlendedShader (const ShaderData& shaderData) const
{
	//-- is shader already in list?

	{
		Shader const * shader = findCachedShader(shaderData);
		if (shader)
			return shader;	
	}
		
	bool useSpecularBlendingShader = false;
				
	Material materials[4];
	
	for (int i = 0; i < shaderData.numberOfTextures; i++)
	{
		if (shaderData.inputShaders[i])
		{
			useSpecularBlendingShader = setupMaterial(materials[i], *shaderData.inputShaders[i]) || useSpecularBlendingShader;
		}
	}
	
	//--
	//-- select the blending shader (specular or not)
	//--

	StaticShader * shader = 0;
	int const blendCount = shaderData.numberOfTextures - 1;
	if (useSpecularBlendingShader && blendingShaderSpecular[blendCount])
		shader = safe_cast<StaticShader*>(blendingShaderSpecular[blendCount]->fetchModifiable());
	else
	{
		shader = safe_cast<StaticShader*>(blendingShader[blendCount]->fetchModifiable());
		
		//-- ensure that we've disabled specular in the event that no specular blending shader exists
		useSpecularBlendingShader = false;
	}
	
	BlendedShaderCacheNode node;
	
	setupShaderData(*shader, node, shaderData, materials, useSpecularBlendingShader);
	
	//-- add to list
	nodeList.add (node);
	
	return shader;
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::ShaderCache::destroyShader (const Shader* shader) const
{
	int i;
	for (i = 0; i < nodeList.getNumberOfElements (); i++)
	{
		if (nodeList [i].shader == shader)
		{
			--nodeList [i].referenceCount;
			return;
		}
	}
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::ShaderCache::alter (const float elapsedTime)
{
	//-- clean blended shaders
	int n = nodeList.getNumberOfElements ();
	int i = 0;
	while (i < n)
	{
		BlendedShaderCacheNode &node = nodeList[i];

		if (node.referenceCount == 0)
		{
			node.timeout += elapsedTime;
			if (node.timeout > 5.0f)
			{
				nodeList [i].shader->release();
				nodeList [i].shader = 0;
				nodeList.removeIndexAndCompactList (i);
				--n;
			}
			else
				++i;
		}
		else
			++i;
	}
}

//----------------------------------------------------------------------

void ClientProceduralTerrainAppearance::ShaderCache::flushCache()
{
	nodeList.clear ();
}

//----------------------------------------------------------------------

void ClientProceduralTerrainAppearance::ShaderCache::setupShaderData(StaticShader & shader, BlendedShaderCacheNode & node, ShaderData const & shaderData, Material const * materials, bool useSpecularBlendingShader) const
{
	//----------------------------------------------------------------------

	static const Tag s_blendTags[] =
	{
		TAG (L,V,L,0),
		TAG (N,R,M,0),
		0, // no alpha for main texture
		TAG (L,V,L,1),
		TAG (N,R,M,1),
		TAG (A,L,P,1),
		TAG (L,V,L,2),
		TAG (N,R,M,2),
		TAG (A,L,P,2),
		TAG (L,V,L,3),
		TAG (N,R,M,3),
		TAG (A,L,P,3)
	};
	
	//----------------------------------------------------------------------
	
	static Tag const s_auxTags[4] =
	{
		TAG(A,U,X,0),
		TAG(A,U,X,1),
		TAG(A,U,X,2),
		TAG(A,U,X,3)
	};

	//----------------------------------------------------------------------

	static Tag const s_materialTags[4] =
	{
		TAG(M,A,T,0),
		TAG(M,A,T,1),
		TAG(M,A,T,2),
		TAG(M,A,T,3)
	};

	//----------------------------------------------------------------------

	node.numberOfTextures = shaderData.numberOfTextures;
	node.shader           = &shader;
	node.referenceCount   = 1;
	node.timeout          = 0.0f;

	shader.setPhaseTag(TAG(T,E,R,R));
	
	for (int i = 0; i < shaderData.numberOfTextures; i++)
	{
		node.textures[i] = shaderData.textures[i];
		node.normals[i]  = shaderData.normals[i];
		node.alphas[i]   = shaderData.alphas[i];

		StaticShader const * const inputShader = shaderData.inputShaders[i];

		if (!inputShader)
			continue;

		//-- diffuse texture
		shader.setTexture(s_blendTags[(i * 3) + 0], *shaderData.textures[i]);
		
		//-- normal texture
		if (ClientProceduralTerrainAppearance::getDot3Terrain ())
			shader.setTexture(s_blendTags[(i * 3) + 1], *shaderData.normals[i]);
		
		//-- alpha blend texture
		if (shaderData.alphas[i])
			shader.setTexture(s_blendTags[(i * 3) + 2], *shaderData.alphas[i]);
		
		//-- specmap/spec
		if (useSpecularBlendingShader)
		{
			Tag const auxTag = s_auxTags[i];
			
			if (inputShader->hasOptionTag(TAG_SPCM))
			{
				Texture const * auxTex = NULL;
				if (inputShader->getTexture(s_auxTags[0], auxTex) && auxTex)
					shader.setTexture(auxTag, *auxTex);
				else
					WARNING(true, ("ClientProceduralTerrainAppearance_ShaderCache tile shader [%s] specifies optional SPCM specmap, but no AUX%d texture is found!", inputShader->getName(), i));	
			}
			else if (inputShader->hasOptionTag(TAG_SPEC))
			{
				shader.setTexture(auxTag, *m_whiteTexture);
			}
			else
			{
				shader.setTexture(auxTag, *m_blackTexture);
			}
			
			shader.setMaterial(s_materialTags[i], materials[i]);
		}
	}
}

// ======================================================================
