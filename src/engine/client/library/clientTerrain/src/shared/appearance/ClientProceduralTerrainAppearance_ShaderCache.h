// ======================================================================
//
// ClientProceduralTerrainAppearance_ShaderCache.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ClientProceduralTerrainAppearance_ShaderCache_H
#define INCLUDED_ClientProceduralTerrainAppearance_ShaderCache_H

#include "clientTerrain/ClientProceduralTerrainAppearance.h"
#include "sharedSynchronization/Mutex.h"

class StaticShader;
class ShaderEffect;
class Material;

// ======================================================================
//
// ClientProceduralTerrainAppearance::ShaderCache
//

class ClientProceduralTerrainAppearance::ShaderCache
{
private:

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct BlendedShaderCacheNode
	{
	public:

		int             numberOfTextures;
		const Texture*  textures[4];
		const Texture*  alphas[4];
		const Texture*  normals[4];

		Shader*         shader;
		int             referenceCount;
		float           timeout;

		BlendedShaderCacheNode() :
		numberOfTextures(0),
		shader(0),
		referenceCount(0),
		timeout(0.0f)
		{
			Zero(textures);
			Zero(alphas);
			Zero(normals);
		}
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct ShaderCacheNode
	{
		StaticShader const * shader;
		const Texture*  diffuseTexture;
		const Texture*  normalTexture;

		ShaderCacheNode() :
		shader(0),
		diffuseTexture(0),
		normalTexture(0)
		{
		}
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

private:

	const ShaderGroup&     shaderGroup;
	ShaderCacheNode**      cache;
	int                    maxNumberOfChildren;
	int                    maxNumberOfFamilies;

	StaticShader*          blendingShader [4];
	StaticShader*          blendingShaderSpecular [4];
	const Shader*          blendingOneQuarterShader;
	const Shader*          blendingOneHalfShader;
	const Shader*          blendingThreeQuarterShader;
	
	Texture const * const  m_defaultTexture;
	Texture const * const  m_defaultNormal;
	const ShaderEffect*    dot3Effect;

	mutable ArrayList<BlendedShaderCacheNode> nodeList;

	Texture const * m_whiteTexture;
	Texture const * m_blackTexture;

private:

	void preloadShaders () const;
		
	void setupShaderData(StaticShader & shader, BlendedShaderCacheNode & node, ShaderData const & shaderData, Material const * materials, bool useSpecularBlendingShader) const;


private:

	ShaderCache ();
	ShaderCache (const ShaderCache& rhs);
	ShaderCache& operator= (const ShaderCache& rhs);

public:

	explicit ShaderCache (const ShaderGroup& shaderGroup);
	~ShaderCache ();

	void               getTextures (const ShaderGroup::Info& sgi, StaticShader const *& baseInputShader, const Texture *&shader, const Texture *&normalTexture) const;
	const Shader*      createBlendedShader (const ShaderData& shaderData) const;
	Shader const *     findCachedShader(ShaderData const & shaderData) const;
	void               destroyShader (const Shader* shader) const;

	//-- 
	const ShaderGroup& getShaderGroup () const;

	const Shader*      getBlendingOneQuarterShader () const;
	const Shader*      getBlendingOneHalfShader () const;
	const Shader*      getBlendingThreeQuarterShader () const;

	void               alter (float elapsedTime);

	void flushCache();
};

// ======================================================================

#endif
