// ======================================================================
//
// StaticShaderTemplate.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_StaticShaderTemplate_H
#define INCLUDED_StaticShaderTemplate_H

// ======================================================================

class Iff;
class Material;
class MemoryBlockManager;
class ShaderEffect;
class Texture;
class VertexBufferFormat;

#include "clientGraphics/ShaderTemplate.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"

// ======================================================================

class StaticShaderTemplate : public ShaderTemplate
{
	friend class Direct3d8_StaticShaderData;
	friend class Direct3d9_StaticShaderData;
	friend class StaticShader;

public:

	enum TextureAddress
	{
		TA_wrap,
		TA_mirror,
		TA_clamp,
		TA_border,
		TA_mirrorOnce,

		TA_invalid
	};

	enum TextureFilter
	{
		TF_none,
		TF_point,
		TF_linear,
		TF_anisotropic,
		TF_flatCubic,
		TF_gaussianCubic,

		TF_invalid
	};

	struct TextureData
	{
		bool            placeholder;
		TextureAddress  addressU;
		TextureAddress  addressV;
		TextureAddress  addressW;
		TextureFilter   mipFilter;
		TextureFilter   minificationFilter;
		TextureFilter   magnificationFilter;
		int             maxAnisotropy;
		const Texture  *texture;
	};

	struct TextureScroll
	{
		float  u1;
		float  v1;
		float  u2;
		float  v2;
	};

	typedef stdmap<Tag, Material>::fwd       MaterialMap;
	typedef stdmap<Tag, TextureData>::fwd    TextureDataMap;
	typedef stdmap<Tag, uint8>::fwd          TextureCoordinateSetMap;
	typedef stdmap<Tag, uint32>::fwd         TextureFactorMap;
	typedef stdmap<Tag, TextureScroll>::fwd  TextureScrollMap;
	typedef stdmap<Tag, uint8>::fwd          AlphaTestReferenceValueMap;
	typedef stdmap<Tag, uint32>::fwd         StencilReferenceValueMap;

public:

	static void install(bool preloadStaticShaderTemplates);
	static void remove();

	static void *operator new(size_t size);
	static void  operator delete(void *pointer);

public:

	virtual const Shader *fetchShader() const;
	virtual Shader       *fetchModifiableShader() const;

	virtual bool          isOpaqueSolid() const;
	virtual bool          isCollidable() const;
	virtual bool          castsShadows() const;
	virtual bool          containsPrecalculatedVertexLighting() const;
	
	const ShaderEffect   &getShaderEffect() const;
	int                   getShaderImplementationSortKey() const;
	bool                  getTextureCoordinateSetTag(Tag tag, uint8 &index) const;
	uint8                 getTextureCoordinateSetUsageMask() const;
	bool                  hasTextureData(Tag tag) const;

private:

	static ShaderTemplate *create(const CrcString &name, Iff &iff);

private:

	/// Disabled.
	StaticShaderTemplate();

	/// Disabled.
	StaticShaderTemplate(const StaticShaderTemplate &);

	/// Disabled.
	StaticShaderTemplate &operator =(const StaticShaderTemplate &);

	StaticShaderTemplate(const CrcString &name, Iff &iff);
	virtual ~StaticShaderTemplate();

	void destroyStaticShader(const StaticShader &staticShader) const;

	void load(Iff &iff);
	void load_0000(Iff &iff);
	void load_0001(Iff &iff);

	void load_texture(Iff & iff );
	void load_texture_0000(Iff & iff);
	void load_texture_0001(Iff & iff);
	void load_texture_0002(Iff & iff);

private:

	static MemoryBlockManager *ms_memoryBlockManager;

private:

	const ShaderEffect                      *m_effect;
	MaterialMap                             *m_materialMap;
	TextureDataMap                          *m_textureDataMap;
	TextureCoordinateSetMap                 *m_textureCoordinateSetMap;
	TextureFactorMap                        *m_textureFactorMap;
	TextureScrollMap                        *m_textureScrollMap;
	AlphaTestReferenceValueMap              *m_alphaTestReferenceValueMap;
	StencilReferenceValueMap                *m_stencilReferenceValueMap;

	mutable const StaticShader              *m_staticShader;

	bool                                     m_addedDot3Placeholder;
};

// ======================================================================

#endif
