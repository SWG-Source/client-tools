// ======================================================================
//
// StaticShader.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_StaticShader_H
#define INCLUDED_StaticShader_H

// ======================================================================

class  MemoryBlockManager;
class  ShaderImplementation;

#include "clientGraphics/Shader.h"
#include "clientGraphics/StaticShaderTemplate.h"
#include <vector>

// ======================================================================

class StaticShaderGraphicsData
{
public:
	virtual DLLEXPORT ~StaticShaderGraphicsData();
	virtual void  update(const StaticShader &shader) = 0;
	virtual int   getTextureSortKey() const = 0;
};

class StaticShader : public Shader
{
	friend class Direct3d8;
	friend class Direct3d8_StaticShaderData;
	friend class Direct3d9;
	friend class StaticShaderTemplate;

public:

	static void install();
	static void remove();

	static void *operator new(size_t size);
	static void  operator delete(void *pointer);

public:

	virtual const StaticShader &prepareToView() const;
	virtual bool                usesVertexShader() const;

	virtual Shader             *convertToModifiableShader() const;
	virtual bool                obeysCustomizationData() const;

	bool                        addedDot3Placeholder() const;

	StaticShader               *fetchModifiable() const;
	const StaticShaderTemplate &getStaticShaderTemplate() const;
	int                         getNumberOfPasses() const;
	int                         getPhase() const;
	void                        setPhaseTag(Tag phase);
	int                         getShaderTemplateSortKey() const;
	int                         getShaderImplementationSortKey() const;
	int                         getTextureSortKey() const;
	DLLEXPORT bool              containsPrecalculatedVertexLighting() const;

	bool                        obeysLightScale() const;
	void                        setObeysLightScale(bool obeysLightScale);

	bool                        isValid() const;

	void setMaterial(Tag tag, const Material &material);
	void setTexture(Tag tag, const Texture &texture);
	void setTextureCoordinateSet(Tag tag, uint8 textureCoordinateSet);
	void setTextureFactor(Tag tag, uint32 textureFactor);
	void setTextureScroll(Tag tag, const StaticShaderTemplate::TextureScroll &textureScroll);
	void setAlphaTestReferenceValue(Tag tag, uint8 alphaReferenceValue);
	void setStencilReferenceValue(Tag tag, uint32 stencilReferenceValue);

	DLLEXPORT bool getMaterial(Tag tag, Material &material) const;
	void           getTextureTags(stdvector<Tag>::fwd & textureTags) const;
	DLLEXPORT bool getTextureData(Tag tag, StaticShaderTemplate::TextureData &textureData) const;
	bool           getTexture(Tag tag, const Texture *&texture) const;
	DLLEXPORT bool getTextureCoordinateSet(Tag tag, uint8 &textureCoordinateSet) const;
	DLLEXPORT bool getTextureFactor(Tag tag, uint32 &textureFactor) const;
	DLLEXPORT bool getTextureScroll(Tag tag, StaticShaderTemplate::TextureScroll &textureScroll) const;
	DLLEXPORT bool getAlphaTestReferenceValue(Tag tag, uint8 &alphaReferenceValue) const;
	DLLEXPORT bool getStencilReferenceValue(Tag tag, uint32 &stencilReferenceValue) const;

	bool           hasMaterial(Tag tag) const;
	bool           hasTexture(Tag tag) const;
	bool           hasTextureFactor(Tag tag) const;
	bool           hasTextureScroll(Tag tag) const;

	bool isHeatPass(int pass) const;

	virtual const StaticShader *getStaticShader() const;
	virtual StaticShader *getStaticShader();

	bool           hasOptionTag(Tag tag) const;
private:

	enum Modifiable
	{
		modifiable
	};

	explicit StaticShader(const StaticShaderTemplate &staticShaderTemplate);
	StaticShader(const StaticShaderTemplate &staticShaderTemplate, Modifiable);

	StaticShader(const StaticShader &copyMe);
	virtual ~StaticShader();

	/// Disabled.
	StaticShader();

	/// Disabled.
	StaticShader &operator =(const StaticShader &);

	void createGlData() const;

private:

	static MemoryBlockManager              *ms_memoryBlockManager;

private:

	mutable StaticShaderGraphicsData                    *m_graphicsData;
	bool                                                 m_shared;
	bool                                                 m_addedDot3Placeholder;
	bool                                                 m_obeysLightScale;
	mutable bool                                         m_usesVertexShader;
	mutable int                                          m_phase;
	mutable int                                          m_numberOfPasses;
	mutable int                                          m_textureSortKey;
	const int                                            m_shaderImplementationSortKey;
	StaticShaderTemplate::MaterialMap                   *m_materialMap;
	StaticShaderTemplate::TextureDataMap                *m_textureDataMap;
	StaticShaderTemplate::TextureCoordinateSetMap       *m_textureCoordinateSetMap;
	StaticShaderTemplate::TextureFactorMap              *m_textureFactorMap;
	StaticShaderTemplate::TextureScrollMap              *m_textureScrollMap;
	StaticShaderTemplate::AlphaTestReferenceValueMap    *m_alphaTestReferenceValueMap;
	StaticShaderTemplate::StencilReferenceValueMap      *m_stencilReferenceValueMap;
	std::vector<uint8> m_isHeatPasses;
	std::vector<Tag> m_optionTags;
};

// ======================================================================
/**
 * Get the template for the static shader.
 * @return This StaticShader's StaticShaderTemplate.
 */

inline const StaticShaderTemplate &StaticShader::getStaticShaderTemplate() const
{
	return static_cast<const StaticShaderTemplate &>(getShaderTemplate());
}

// ----------------------------------------------------------------------
/**
 * Get a modifiable copy of this shader.
 * The reference count of the returned object will be 1.
 * @return A modifiable copy of this shader.
 */

inline StaticShader *StaticShader::fetchModifiable() const
{
	StaticShader *newShader = new StaticShader(*this);
	newShader->fetch();
	return newShader;
}

// ----------------------------------------------------------------------
/**
 * Get the phase in which to render this shader.
 */

inline int StaticShader::getPhase() const
{
	return m_phase;
}

// ----------------------------------------------------------------------

inline int StaticShader::getNumberOfPasses() const
{
	return m_numberOfPasses;
}

// ----------------------------------------------------------------------

inline bool StaticShader::isValid() const
{
	return m_graphicsData != NULL;
}

// ----------------------------------------------------------------------

inline bool StaticShader::obeysLightScale() const
{
	return m_obeysLightScale;
}

// ----------------------------------------------------------------------

inline bool StaticShader::addedDot3Placeholder() const
{
	return m_addedDot3Placeholder;
}

//----------------------------------------------------------------------

inline bool StaticShader::isHeatPass(int pass) const
{

	if (static_cast<int>(m_isHeatPasses.size()) <= pass || pass < 0)
		return false;

	return m_isHeatPasses[pass] != 0;
}

// ======================================================================

#endif
