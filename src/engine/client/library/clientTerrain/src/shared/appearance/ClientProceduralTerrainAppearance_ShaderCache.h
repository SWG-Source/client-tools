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
#include "sharedSynchronization/RecursiveMutex.h"

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

	// nodeList is mutated from BOTH the ClientTerrain worker thread (findCachedShader /
	// createBlendedShader -> nodeList.add, reached from ClientChunk::createTileShader) and the
	// main thread (alter / destroyShader / flushCache, plus the synchronous LevelOfDetail
	// createChunk fallback). ArrayList::add re-allocates its buffer (new[]/copy/delete[]), so a
	// concurrent iterate-vs-add or add-vs-add stale-frees the buffer -> allocator freelist
	// poisoned -> a later small allocation (typically a loader path string) lands on TOP of a
	// live object (observed 2026-07-03: a StaticShaderTemplate MaterialMap overwritten with
	// "...ader file..." text -> AV in std::map::find on the terrain thread). The Mutex.h include
	// above predates this fix but no lock was ever added -- this member completes it. Leaf lock,
	// same shape as the TreeFile::SearchCache fix (9c03f53c5). RECURSIVE because
	// createBlendedShader legitimately re-enters via findCachedShader; Guard-idiom RAII matches
	// ShaderTemplateList. Ordering: taken only around nodeList access, so it always precedes
	// ShaderTemplateList::ms_criticalSection (reached via fetchModifiable) -- no inversion.
	mutable RecursiveMutex m_nodeListLock;

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
