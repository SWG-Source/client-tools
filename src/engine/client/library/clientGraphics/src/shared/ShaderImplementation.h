// ======================================================================
//
// ShaderImplementation.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ShaderImplementation_H
#define INCLUDED_ShaderImplementation_H

// ======================================================================

class Iff;
class RecursiveMutex;
class ShaderImplementationPass;
class ShaderImplementationPassFixedFunctionPipeline;
class ShaderImplementationPassPixelShader;
class ShaderImplementationPassPixelShaderProgram;
class ShaderImplementationPassPixelShaderTextureSampler;
class ShaderImplementationPassStage;
class ShaderImplementationPassVertexShader;
class StaticShaderTemplate;

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/PersistentCrcString.h"
#include "clientGraphics/VertexBufferFormat.h"
#include <vector>

// ======================================================================

class ShaderImplementationGraphicsData
{
public:
	virtual DLLEXPORT ~ShaderImplementationGraphicsData();
};

// ======================================================================

class ShaderImplementation
{
	friend class Direct3d8_ShaderImplementationData;
	friend class Direct3d8_StaticShaderData;
	friend class Direct3d9_ShaderImplementationData;
	friend class Direct3d9_StaticShaderData;

public:

	static void install();
	static void remove();

public:

	typedef ShaderImplementationPass Pass;
	typedef stdmap<Tag, uint8>::fwd  TextureCoordinateSetMap;

public:

	ShaderImplementation(const char *name, Iff &iff);
	~ShaderImplementation();

	bool          isSupported() const;

	const char   *getName() const;
	void          fetch() const;
	void          release() const;

	int           getNumberOfPasses() const;
	int           getPhase() const;
	bool          isOpaqueSolid() const;
	bool          usesVertexShader() const;
	bool          castsShadows() const;
	bool          isCollidable() const;
	void          verifyCompatibility(VertexBufferFormat const & vertexBufferFormat, StaticShaderTemplate const & staticShaderTemplate) const;

	bool          usesMaterial(Tag materialTag) const;
	bool          usesTexture(Tag textureTag) const;
	bool          usesTextureCoordinateSet(Tag textureCoordinateSetTag) const;
	bool          usesTextureFactor(Tag textureFactorTag) const;
	bool          usesTextureScroll(Tag textureFactorTag) const;
	bool          usesAlphaReference(Tag alphaReferenceTag) const;
	bool          usesStencilReference(Tag alphaReferenceTag) const;

	bool          hasOptionTag(Tag tag) const;
	std::vector<Tag> const & getOptionTags() const;

	bool isHeatPass(int pass) const;

#if PRODUCTION == 0
	static void   reloadPixelShader(const char *name);
	static void   reloadVertexShader(const char *name);
#endif
	
private:

	/// Disabled.
	ShaderImplementation();

	/// Disabled.
	ShaderImplementation(const ShaderImplementation &);

	/// Disabled.
	ShaderImplementation &operator =(const ShaderImplementation &);

	void checkOldVersionForSupport();

private:

	typedef stdvector<Pass *>::fwd Passes;

private:

	void load(Iff &iff);
	void load_0000(Iff &iff);
	void load_0001(Iff &iff);
	void load_0002(Iff &iff);
	void load_0003(Iff &iff);
	void load_0004(Iff &iff);
	void load_0005(Iff &iff);
	void load_0006(Iff &iff);
	void load_0007(Iff &iff);
	void load_0008(Iff &iff);
	void load_0009(Iff &iff);

private:

	mutable int                                 m_users;
	mutable ShaderImplementationGraphicsData   *m_graphicsData;
	char                                       *m_name;
	Passes                                     *m_pass;
	int                                         m_phase;
	bool                                        m_castsShadows;
	bool                                        m_isCollidable;
	std::vector<Tag>                            m_optionTags;
};

// ======================================================================

//lint -esym(1925, ShaderImplementationPass::m_*) // 1925: Note -- Symbol is a public data member
class ShaderImplementationPass
{
public:

	typedef ShaderImplementationPassStage                 Stage;
	typedef ShaderImplementationPassPixelShader           PixelShader;
	typedef ShaderImplementationPassFixedFunctionPipeline FixedFunctionPipeline;
	typedef ShaderImplementationPassVertexShader          VertexShader;

public:

	explicit ShaderImplementationPass(Iff &iff);
	~ShaderImplementationPass();

public:

	typedef stdvector<Stage *>::fwd Stages;

	bool        hasAlphaBlending() const;
	bool        hasAlphaTesting() const;
	bool        usesVertexShader() const;
	bool        usesMaterial(Tag materialTag) const;
	bool        usesTexture(Tag textureTag) const;
	bool        usesTextureCoordinateSet(Tag textureCoordinateSetTag) const;
	bool        usesTextureFactor(Tag textureFactorTag) const;
	bool        usesTextureScroll(Tag textureFactorTag) const;
	bool        usesAlphaReference(Tag alphaReferenceTag) const;
	bool        usesStencilReference(Tag alphaReferenceTag) const;
	void        verifyCompatibility(VertexBufferFormat const & vertexBufferFormat, StaticShaderTemplate const & staticShaderTemplate) const;
	bool        isHeat() const;

private:

	/// Disabled.
	ShaderImplementationPass();

	/// Disabled.
	ShaderImplementationPass(const ShaderImplementationPass &);

	/// Disabled.
	ShaderImplementationPass &operator =(const ShaderImplementationPass &);


private:

	void load(Iff &iff);
	void load_0000(Iff &iff);
	void load_0001(Iff &iff);
	void load_0002(Iff &iff);
	void load_0003(Iff &iff);
	void load_0004(Iff &iff);
	void load_0005(Iff &iff);
	void load_0006(Iff &iff);
	void load_0007(Iff &iff);
	void load_0008(Iff &iff);
	void load_0009(Iff &iff);
	void load_0010(Iff &iff);

public:

	enum ShadeMode
	{
		SM_Flat,
		SM_Gouraud
	};

	enum Compare
	{
		C_Never,
		C_Less,
		C_Equal,
		C_LessOrEqual,
		C_Greater,
		C_GreaterOrEqual,
		C_NotEqual,
		C_Always
	};

	enum Blend
	{
		B_Zero,
		B_One,
		B_SourceColor,
		B_InverseSourceColor,
		B_SourceAlpha,
		B_InverseSourceAlpha,
		B_DestinationAlpha,
		B_InverseDestinationAlpha,
		B_DestinationColor,
		B_InverseDestinationColor,
		B_SourceAlphaSaturate
	};

	enum BlendOperation
	{
		BO_Add,
		BO_Subtract,
		BO_ReverseSubtract,
		BO_Min,
		BO_Max
	};

	enum StencilOperation
	{
		SO_Keep,
		SO_Zero,
		SO_Replace,
		SO_IncrementSaturate,
		SO_DecrementSaturate,
		SO_Invert,
		SO_IncrementWrap,
		SO_DecrementWrap
	};

	enum FogMode
	{
		FM_Normal,
		FM_Black,
		FM_White
	};

public:

	FixedFunctionPipeline *m_fixedFunctionPipeline;
	const VertexShader    *m_vertexShader;
	Stages                *m_stage;
	PixelShader           *m_pixelShader;

	ShadeMode              m_shadeMode;
	FogMode                m_fogMode;

	bool                   m_ditherEnable;

	bool                   m_zEnable;
	bool                   m_zWrite;
	Compare                m_zCompare;

	bool                   m_alphaBlendEnable;
	BlendOperation         m_alphaBlendOperation;
	Blend                  m_alphaBlendSource;
	Blend                  m_alphaBlendDestination;

	bool                   m_alphaTestEnable;
	Tag                    m_alphaTestReferenceValueTag;
	Compare                m_alphaTestFunction;
	uint8                  m_writeEnable;

	Tag                    m_textureFactorTag;
	Tag                    m_textureFactorTag2;
	Tag                    m_textureScrollTag;

	bool                   m_stencilEnable;
	bool                   m_stencilTwoSidedMode;
	Tag                    m_stencilReferenceValueTag;
	Compare                m_stencilCompareFunction;
	StencilOperation       m_stencilPassOperation;
	StencilOperation       m_stencilZFailOperation;
	StencilOperation       m_stencilFailOperation;
	Compare                m_stencilCounterClockwiseCompareFunction;
	StencilOperation       m_stencilCounterClockwisePassOperation;
	StencilOperation       m_stencilCounterClockwiseZFailOperation;
	StencilOperation       m_stencilCounterClockwiseFailOperation;
	uint32                 m_stencilWriteMask;
	uint32                 m_stencilMask;

	Tag                    m_materialTag;
	bool m_heat;
};

// ======================================================================

class ShaderImplementationPassFixedFunctionPipeline
{
	friend class ShaderImplementationPass;

public:

	ShaderImplementationPassFixedFunctionPipeline(Iff &iff, bool old);
	~ShaderImplementationPassFixedFunctionPipeline();

	void verifyCompatibility(VertexBufferFormat const & vertexBufferFormat, StaticShaderTemplate const & staticShaderTemplate) const;

private:

	/// Disabled.
	ShaderImplementationPassFixedFunctionPipeline();

	/// Disabled.
	ShaderImplementationPassFixedFunctionPipeline(const ShaderImplementationPassFixedFunctionPipeline &);

	/// Disabled.
	ShaderImplementationPassFixedFunctionPipeline &operator =(const ShaderImplementationPassFixedFunctionPipeline &);

private:

	void load(Iff &iff);
	void load_old(Iff &iff);
	void load_0000(Iff &iff);
	void load_0001(Iff &iff);

public:

	enum MaterialSource
	{
		MS_Material,
		MS_VertexColor0,
		MS_VertexColor1
	};

public:

	bool              m_lighting;
	bool              m_lightingSpecularEnable;
	bool              m_lightingColorVertex;
	Tag               m_deprecated_lightingMaterialTag;
	MaterialSource    m_lightingAmbientColorSource;
	MaterialSource    m_lightingDiffuseColorSource;
	MaterialSource    m_lightingSpecularColorSource;
	MaterialSource    m_lightingEmissiveColorSource;
};

// ======================================================================

class ShaderImplementationPassVertexShaderGraphicsData
{
public:
	virtual DLLEXPORT ~ShaderImplementationPassVertexShaderGraphicsData();
};

// ======================================================================

class ShaderImplementationPassVertexShader
{
	friend class ShaderImplementationPass;

public:

	typedef ShaderImplementationPassVertexShader VertexShader;
	
	static void                assignAsynchronousLoaderFunctions();
	static const VertexShader *fetch(const char *filename);

	static void reportDangling();

public:

	void fetch() const;
	void release() const;

	bool usesTextureCoordinateSet(Tag textureCoordinateSetTag) const;

	void verifyCompatibility(VertexBufferFormat const & vertexBufferFormat, StaticShaderTemplate const & staticShaderTemplate) const;

	void reloadShaderProgram();

	char const * const getFilename() const { return m_fileName.getString(); }

private:

	ShaderImplementationPassVertexShader(CrcString const &fileName);
	~ShaderImplementationPassVertexShader();

	/// Disabled.
	ShaderImplementationPassVertexShader();

	/// Disabled.
	ShaderImplementationPassVertexShader(const VertexShader &);

	/// Disabled.
	ShaderImplementationPassVertexShader &operator =(const VertexShader &);

	static const void         *asynchronousLoaderFetchNoCreate(const char *fileName);
	static void                asynchronousLoaderRelease(const void *vertexShader);
	static const VertexShader *fetch(const char *fileName, bool create);

private:

	void load(CrcString const &fileName);

private:

	mutable int    m_referenceCount;

public:

	PersistentCrcString                                m_fileName;
	char                                              *m_text;
	int                                                m_textLength;
	ShaderImplementationPassVertexShaderGraphicsData  *m_graphicsData;
};

// ======================================================================

//lint -esym(1925, ShaderImplementationPassStage::*) // 1925: Note -- Symbol is a public data member
class ShaderImplementationPassStage
{
public:

	explicit ShaderImplementationPassStage(Iff &iff);
	~ShaderImplementationPassStage();

	bool usesTexture(Tag textureTag) const;
	bool usesTextureCoordinateSet(Tag textureCoordinateSetTag) const;
	void verifyCompatibility(VertexBufferFormat const & vertexBufferFormat, StaticShaderTemplate const & staticShaderTemplate) const;

private:

	/// Disabled.
	ShaderImplementationPassStage();

	/// Disabled.
	ShaderImplementationPassStage(const ShaderImplementationPassStage &);

	/// Disabled.
	ShaderImplementationPassStage &operator =(const ShaderImplementationPassStage &);

private:

	void load(Iff &iff);
	void load_0000(Iff &iff);
	void load_0001(Iff &iff);

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

	enum CoordinateGeneration
	{
		CG_passThru,
		CG_cameraSpacePosition,
		CG_cameraSpaceNormal,
		CG_cameraSpaceReflectionVector,
		CG_scroll1,
		CG_scroll2
	};

	enum TextureOperation
	{
		TO_disable,
		TO_selectArg1,
		TO_selectArg2,
		TO_modulate,
		TO_modulate2x,
		TO_modulate4x,
		TO_add,
		TO_addSigned,
		TO_addSigned2x,
		TO_subtract,
		TO_addSmooth,
		TO_blendDiffuseAlpha,
		TO_blendTextureAlpha,
		TO_blendFactorAlpha,
		TO_blendTextureAlphapm,
		TO_blendCurrentAlpha,
		TO_premodulate,
		TO_modulateAlpha_addColor,
		TO_modulateColor_addAlpha,
		TO_modulateInvalpha_addColor,
		TO_modulateInvcolor_addAlpha,
		TO_bumpEnvMap,
		TO_bumpEnvMapLuminance,
		TO_dotProduct3,
		TO_multiplyAdd,
		TO_lerp
	};

	enum TextureArgument
	{
		TA_current,
		TA_diffuse,
		TA_specular,
		TA_temp,
		TA_texture,
		TA_textureFactor
	};

public:

	TextureOperation     m_colorOperation;
	TextureArgument      m_colorArgument0;
	bool                 m_colorArgument0Complement;
	bool                 m_colorArgument0AlphaReplicate;
	TextureArgument      m_colorArgument1;
	bool                 m_colorArgument1Complement;
	bool                 m_colorArgument1AlphaReplicate;
	TextureArgument      m_colorArgument2;
	bool                 m_colorArgument2Complement;
	bool                 m_colorArgument2AlphaReplicate;

	TextureOperation     m_alphaOperation;
	TextureArgument      m_alphaArgument0;
	bool                 m_alphaArgument0Complement;
	TextureArgument      m_alphaArgument1;
	bool                 m_alphaArgument1Complement;
	TextureArgument      m_alphaArgument2;
	bool                 m_alphaArgument2Complement;

	TextureArgument      m_resultArgument;

	Tag                  m_textureTag;
	Tag                  m_textureCoordinateSetTag;
	TextureAddress       m_textureAddressU;
	TextureAddress       m_textureAddressV;
	TextureAddress       m_textureAddressW;
	TextureFilter        m_textureMipFilter;
	TextureFilter        m_textureMinificationFilter;
	TextureFilter        m_textureMagnificationFilter;
	CoordinateGeneration m_textureCoordinateGeneration;
};

// ======================================================================

class ShaderImplementationPassPixelShader
{
public:

	typedef ShaderImplementationPassPixelShaderProgram        Program;
	typedef ShaderImplementationPassPixelShaderTextureSampler TextureSampler;
	typedef stdvector<TextureSampler *>::fwd TextureSamplers;

public:

	explicit ShaderImplementationPassPixelShader(Iff &iff);
	~ShaderImplementationPassPixelShader();

	bool usesTexture(Tag textureTag) const;

	void verifyCompatibility(VertexBufferFormat const & vertexBufferFormat, StaticShaderTemplate const & staticShaderTemplate) const;

	void reloadShaderProgram();

private:

	/// Disabled.
	ShaderImplementationPassPixelShader();

	/// Disabled.
	ShaderImplementationPassPixelShader(const ShaderImplementationPassPixelShader &);

	/// Disabled.
	ShaderImplementationPassPixelShader &operator =(const ShaderImplementationPassPixelShader &);

private:

	void load(Iff &iff);
	void load_0001(Iff &iff);

public:

	const Program   *m_program;
	TextureSamplers *m_textureSamplers;
	int              m_maxTextureSampler;
};

// ======================================================================

class ShaderImplementationPassPixelShaderProgramGraphicsData
{
public:
	virtual DLLEXPORT ~ShaderImplementationPassPixelShaderProgramGraphicsData();
};

// ======================================================================

class ShaderImplementationPassPixelShaderProgram
{
	friend class ShaderImplementationPassPixelShader;

public:

	typedef ShaderImplementationPassPixelShaderProgram Program;

	static void           assignAsynchronousLoaderFunctions();
	static const Program *fetch(const char *fileName);

public:

	DLLEXPORT char const * getFileName() const;

	void fetch() const;
	void release() const;
	void reload();

	DLLEXPORT int  getVersionMajor() const;
	DLLEXPORT int  getVersionMinor() const;

private:

	ShaderImplementationPassPixelShaderProgram(CrcString const & fileName);
	~ShaderImplementationPassPixelShaderProgram();

	/// Disabled.
	ShaderImplementationPassPixelShaderProgram();

	/// Disabled.
	ShaderImplementationPassPixelShaderProgram(const ShaderImplementationPassPixelShaderProgram &);

	/// Disabled.
	ShaderImplementationPassPixelShaderProgram &operator =(const ShaderImplementationPassPixelShaderProgram &);

private:

	static const void    *asynchronousLoaderFetchNoCreate(const char *fileName);
	static void           asynchronousLoaderRelease(const void *vertexShader);
	static const Program *fetch(const char *fileName, bool create);

private:

	void load(Iff &iff);
	void load_0000(Iff &iff);

private:

	mutable int    m_referenceCount;

public:

	PersistentCrcString                                      m_fileName;
	DWORD                                                   *m_exe;
	ShaderImplementationPassPixelShaderProgramGraphicsData  *m_graphicsData;
};

// ======================================================================

class ShaderImplementationPassPixelShaderTextureSampler
{
public:

	explicit ShaderImplementationPassPixelShaderTextureSampler(Iff &iff);
	~ShaderImplementationPassPixelShaderTextureSampler();

	bool usesTexture(Tag textureTag) const;
	void verifyCompatibility(VertexBufferFormat const & vertexBufferFormat, StaticShaderTemplate const & staticShaderTemplate) const;

private:

	/// Disabled.
	ShaderImplementationPassPixelShaderTextureSampler();

	/// Disabled.
	ShaderImplementationPassPixelShaderTextureSampler(const ShaderImplementationPassPixelShaderTextureSampler &);

	/// Disabled.
	ShaderImplementationPassPixelShaderTextureSampler &operator =(const ShaderImplementationPassPixelShaderTextureSampler &);

private:

	void load(Iff &iff);
	void load_0000(Iff &iff);
	void load_0001(Iff &iff);
	void load_0002(Iff &iff);

public:

	int                                                 m_textureIndex;
	Tag                                                 m_textureTag;
	ShaderImplementationPassStage::TextureAddress       m_textureAddressU;
	ShaderImplementationPassStage::TextureAddress       m_textureAddressV;
	ShaderImplementationPassStage::TextureAddress       m_textureAddressW;
	ShaderImplementationPassStage::TextureFilter        m_textureMipFilter;
	ShaderImplementationPassStage::TextureFilter        m_textureMinificationFilter;
	ShaderImplementationPassStage::TextureFilter        m_textureMagnificationFilter;
};

// ======================================================================

inline int ShaderImplementation::getPhase() const
{
	return m_phase;
}

// ----------------------------------------------------------------------

inline const char *ShaderImplementation::getName() const
{
	return m_name;
}

// ----------------------------------------------------------------------

inline bool ShaderImplementation::castsShadows() const
{
	return m_castsShadows;
}

// ----------------------------------------------------------------------

inline bool ShaderImplementation::isCollidable() const
{
	return m_isCollidable;
}

//----------------------------------------------------------------------

inline bool ShaderImplementationPass::isHeat() const
{
	return m_heat;
}

// ======================================================================

inline bool ShaderImplementationPass::usesVertexShader() const
{
	return (m_vertexShader != NULL);
}

// ----------------------------------------------------------------------

inline bool ShaderImplementationPass::hasAlphaBlending() const
{
	return m_alphaBlendEnable;
}

// ----------------------------------------------------------------------

inline bool ShaderImplementationPass::hasAlphaTesting() const
{
	return m_alphaTestEnable;
}

// ======================================================================

#endif

