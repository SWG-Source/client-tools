// ======================================================================
// Node.h
// ======================================================================

#ifndef NODE_H
#define NODE_H

// ======================================================================

class Iff;
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"
#include "clientGraphics\VertexBufferFormat.h"
#include <vector>

// ======================================================================
// forward declare classes here, since these all refer to each other

class CLeftView;

class CTemplateTreeNode;
class CMaterialTreeNode;
class CTextureTreeNode;
class CTextureCoordinateSetTreeNode;
class CTextureFactorTreeNode;
class CTextureScrollTreeNode;
class CAlphaReferenceValueTreeNode;
class CStencilReferenceValueTreeNode;
class CEffectNode;
class CImplementationNode;
class CPassNode;
class CStageNode;
class CMaterialNode;
class CTextureNode;
class CTextureCoordinateSetNode;
class CTextureFactorNode;
class CTextureScrollNode;
class CAlphaReferenceValueNode;
class CStencilReferenceValueNode;
class CPixelShaderNode;
class CFixedFunctionPipelineNode;
class CVertexShaderNode;
class CVertexShaderProgramNode;
class CPixelShaderProgramNode;
class CVertexShaderProgramTreeNode;
class CPixelShaderProgramTreeNode;
class CIncludeTreeNode;
class CIncludeNode;

// ======================================================================

typedef std::map<CString, int8> Map;

class CNode
{
public:

	enum Type
	{
		Template,
		MaterialTree,
		TextureTree,
		TextureCoordinateSetTree,
		TextureFactorTree,
		TextureScrollTree,
		AlphaReferenceValueTree,
		StencilReferenceValueTree,
		Effect,
		Material,
		Texture,
		TextureCoordinateSet,
		TextureFactor,
		TextureScroll,
		AlphaReferenceValue,
		StencilReferenceValue,
		Implementation,
		Pass,
		Stage,
		PixelShader,
		FixedFunctionPipeline,
		VertexShader,
		VertexShaderProgram,
		PixelShaderProgram,
		VertexShaderProgramTree,
		PixelShaderProgramTree,
		Include,
		IncludeTree,

		Count
	};

	enum Whom
	{
		Parent,
		Self,
		Either
	};

public:

	static void install();
	static void remove();

public:

	CNode(Type newType);
	virtual ~CNode();

	virtual bool IsLocked(Whom whom) const;
	void         SetItemImages() const;
	void         SetRendering(bool rendering) const;

	virtual CNode  *Find(HTREEITEM item);
	Type        GetType() const;
	void        GetName(CString &string) const;
	HTREEITEM   GetTreeItem() const;

protected:

	void         InsertItem(const char *name, CNode &node, HTREEITEM after=TVI_LAST);
	void         RemoveItem(CNode &node);
	CNode       *FindNamedChild(const char *name);

	CNode       *GetFirstChild();
	CNode       *GetNextChild(const CNode &node);
	CNode       *GetFirstChild(CNode::Type type);
	CNode       *GetNextChild(CNode::Type type, const CNode &node);

	const CNode  *GetFirstChild() const;
	const CNode  *GetNextChild(const CNode &node) const;
	const CNode  *GetFirstChild(CNode::Type type) const;
	const CNode  *GetNextChild(CNode::Type type, const CNode &node) const;

protected:

	static CLeftView *leftView;
	static CTreeCtrl *treeCtrl;

protected:

	typedef std::map<HTREEITEM, CNode *> Children;

protected:

	static Map ms_textureOp;
	static Map ms_textureArg;
	static Map ms_textureAddress;
	static Map ms_textureFilter;
	static Map ms_coordinateGeneration;
	static Map ms_shadeMode;
	static Map ms_compare;
	static Map ms_blend;
	static Map ms_blendOp;
	static Map ms_materialSource;
	static Map ms_stencilOp;
	static Map ms_textureIndex;
	static Map ms_fogMode;

protected:

	HTREEITEM      treeItem;
	const Type     type;
	Children       children;
	mutable bool   isRendering;
};

// ======================================================================

class CTemplateTreeNode : public CNode
{
public:

	static CTemplateTreeNode *GetInstance();

public:

	CTemplateTreeNode(CLeftView &leftView, CTreeCtrl &newTreeCtrl);
	virtual ~CTemplateTreeNode();

	virtual CNode  *Find(HTREEITEM item);

	bool                                  Load(Iff &iff);
	bool                                  Save(Iff &iff) const;
	bool                                  Validate();

	CMaterialTreeNode                    &GetMaterialTree();
	CTextureTreeNode                     &GetTextureTree();
	CTextureCoordinateSetTreeNode        &GetTextureCoordinateSetTree();
	CTextureFactorTreeNode               &GetTextureFactorTree();
	CTextureScrollTreeNode               &GetTextureScrollTree();
	CAlphaReferenceValueTreeNode         &GetAlphaReferenceValueTree();
	CStencilReferenceValueTreeNode       &GetStencilReferenceValueTree();
	CEffectNode                          &GetEffect();
	CIncludeTreeNode                     &GetIncludeTree();
	CVertexShaderProgramTreeNode         &GetVertexShaderProgramTree();
	CPixelShaderProgramTreeNode          &GetPixelShaderProgramTree();

	const CMaterialTreeNode              &GetMaterialTree() const;
	const CTextureTreeNode               &GetTextureTree() const;
	const CTextureCoordinateSetTreeNode  &GetTextureCoordinateSetTree() const;
	const CTextureFactorTreeNode         &GetTextureFactorTree() const;
	const CTextureScrollTreeNode         &GetTextureScrollTree() const;
	const CAlphaReferenceValueTreeNode   &GetAlphaReferenceValueTree() const;
	const CStencilReferenceValueTreeNode &GetStencilReferenceValueTree() const;
	const CEffectNode                    &GetEffect() const;

private:

	static CTemplateTreeNode *instance;

	bool                                  Load_0000(Iff &iff);
	bool                                  Load_0001(Iff &iff);

private:

	CMaterialTreeNode               *materialTreeNode;
	CTextureTreeNode                *textureTreeNode;
	CTextureCoordinateSetTreeNode   *textureCoordinateSetTreeNode;
	CTextureFactorTreeNode          *textureFactorTreeNode;
	CTextureScrollTreeNode          *textureScrollTreeNode;
	CAlphaReferenceValueTreeNode    *alphaReferenceValueTreeNode;
	CStencilReferenceValueTreeNode  *stencilReferenceValueTreeNode;
	CEffectNode                     *effectNode;

	CIncludeTreeNode                *includeTreeNode;
	CVertexShaderProgramTreeNode    *vertexShaderProgramTreeNode;
	CPixelShaderProgramTreeNode     *pixelShaderProgramTreeNode;
};

// ======================================================================

template <class T>
class CTaggedTreeNode : public CNode
{
public:

	typedef T ChildNode;

public:

	CTaggedTreeNode(Type newType);
	virtual ~CTaggedTreeNode();

	void Add(const char *name);
	void Remove(const char *name);

	void GetChildTags(std::set<CString> &tags);
};

// ======================================================================

class CMaterialTreeNode : public CTaggedTreeNode<CMaterialNode>
{
public:
	CMaterialTreeNode();
	virtual ~CMaterialTreeNode();

	bool Load(Iff &iff);
	bool Save(Iff &iff) const;
	bool Validate();

	CMaterialNode *GetMaterial(const CString &name);
};

// ======================================================================

class CTextureTreeNode : public CTaggedTreeNode<CTextureNode>
{
public:
	CTextureTreeNode();
	virtual ~CTextureTreeNode();

	bool Load(Iff &iff);
	bool Save(Iff &iff) const;
	bool Validate();

	CTextureNode *GetTexture(const CString &name);
};

// ======================================================================

class CTextureCoordinateSetTreeNode : public CTaggedTreeNode<CTextureCoordinateSetNode>
{
public:
	CTextureCoordinateSetTreeNode();
	virtual ~CTextureCoordinateSetTreeNode();

	bool Load(Iff &iff);
	bool Save(Iff &iff) const;
	bool Validate();

	CTextureCoordinateSetNode *GetTextureCoordinateSet(const CString &name);
};

// ======================================================================

class CTextureFactorTreeNode : public CTaggedTreeNode<CTextureFactorNode>
{
public:
	CTextureFactorTreeNode();
	virtual ~CTextureFactorTreeNode();

	bool Load(Iff &iff);
	bool Save(Iff &iff) const;
	bool Validate();

	CTextureFactorNode *GetTextureFactor(const CString &name);
};

// ======================================================================

class CTextureScrollTreeNode : public CTaggedTreeNode<CTextureScrollNode>
{
public:
	CTextureScrollTreeNode();
	virtual ~CTextureScrollTreeNode();

	bool Load(Iff &iff);
	bool Save(Iff &iff) const;
	bool Validate();

	CTextureScrollNode *GetTextureScroll(const CString &name);
};

// ======================================================================

class CAlphaReferenceValueTreeNode : public CTaggedTreeNode<CAlphaReferenceValueNode>
{
public:
	CAlphaReferenceValueTreeNode();
	virtual ~CAlphaReferenceValueTreeNode();

	bool Load(Iff &iff);
	bool Save(Iff &iff) const;
	bool Validate();

	CAlphaReferenceValueNode *GetAlphaReferenceValue(const CString &name);
};

// ======================================================================

class CStencilReferenceValueTreeNode : public CTaggedTreeNode<CStencilReferenceValueNode>
{
public:
	CStencilReferenceValueTreeNode();
	virtual ~CStencilReferenceValueTreeNode();

	bool Load(Iff &iff);
	bool Save(Iff &iff) const;
	bool Validate();

	CStencilReferenceValueNode *GetStencilReferenceValue(const CString &name);
};

// ======================================================================

class CEffectNode : public CNode
{
	friend class CImplementationNode;

public:
	CEffectNode();
	virtual ~CEffectNode();

	virtual bool               IsLocked(Whom whom) const;

	bool                       IsLocal() const;
	void                       Localize();

	void                       Reset();
	bool                       Load(Iff &iff);
	bool                       Save(Iff &iff, bool direct) const;
	bool                       Validate();
	void                       Lint();

	CImplementationNode       &AddLocalImplementation();
	void                       AddLocalImplementation(const CImplementationNode &implementation, HTREEITEM after);
	void                       RemoveImplementation(CImplementationNode &implementation);

	const CString             &GetFileName() const;
	void                       SetFileName(const CString &newValue);

	const CImplementationNode *GetFirstImplementation() const;
	const CImplementationNode *GetNextImplementation(const CImplementationNode &implementationNode) const;

public:

	bool    m_containsPrecalculatedVertexLighting;

private:

	bool              Load_0000(Iff &iff);
	bool              Load_0001(Iff &iff);

private:

	bool    m_isLocal;
	CString m_fileName;
};

// ======================================================================

class CImplementationNode : public CNode
{
	friend class CPassNode;

public:
	CImplementationNode(CEffectNode &effect);
	CImplementationNode(CEffectNode &effect, const CImplementationNode &copyDataFrom, HTREEITEM after);
	virtual ~CImplementationNode();

	virtual bool               IsLocked(Whom whom) const;

	bool              Load(Iff &iff);
	bool              Save(Iff &iff, bool direct) const;
	bool              Validate();
	void              Lint();

	void              Delete();

	void              AddLocalPass();
	void              AddLocalPass(const CPassNode &pass, HTREEITEM after);
	void              RemovePass(CPassNode &pass);

	const CPassNode  *GetFirstPass() const;
	const CPassNode  *GetNextPass(const CPassNode &passNode) const;

	const CString    &GetFileName() const;
	void              SetFileName(const CString &newValue);

private:

	static int remapOldInconsistentShaderCapabilityLevels(int oldLevel);
	static int remapRecentInconsistentShaderCapabilityLevels(int oldLevel);

private:

	bool              Load_0000(Iff &iff);
	bool              Load_0001(Iff &iff);
	bool              Load_0002(Iff &iff);
	bool              Load_0003(Iff &iff);
	bool              Load_0004(Iff &iff);
	bool              Load_0005(Iff &iff);
	bool              Load_0006(Iff &iff);
	bool              Load_0007(Iff &iff);
	bool              Load_0008(Iff &iff);
	bool              Load_0009(Iff &iff);

public:

	CEffectNode          &m_effect;
	CString               m_phaseTag;
	std::vector<CString>  m_shaderCapabilityCompatibility;
	std::vector<CString>  m_optionTags;
	CString               m_fileName;
	BOOL                  m_castsShadows;
	BOOL                  m_collidable;
};

// ======================================================================

class CPassNode : public CNode
{
	friend class CStageNode;
	friend class CPixelShaderNode;
	friend class CFixedFunctionPipelineNode;
	friend class CVertexShaderNode;

public:
	CPassNode(CImplementationNode &newImplementation);
	CPassNode(CImplementationNode &newImplementation, const CPassNode &copyDataFrom, HTREEITEM after);
	virtual ~CPassNode();

	virtual bool       IsLocked(Whom whom) const;

	bool               Load(Iff &iff);
	bool               Save(Iff &iff) const;
	bool               Validate();
	void               Lint();

	void               Delete();

	void               AddFixedFunctionPipeline();
	void               AddFixedFunctionPipeline(const CFixedFunctionPipelineNode &copyFrom);
	void               RemoveFixedFunctionPipeline(CFixedFunctionPipelineNode &fixedFunctionPipelineNode);

	void               AddVertexShader();
	void               AddVertexShader(const CVertexShaderNode &copyFrom);
	void               RemoveVertexShader(CVertexShaderNode &vertexShaderNode);

	void               AddLocalStage();
	void               AddLocalStage(const CStageNode &stage, HTREEITEM after);
	void               RemoveStage(CStageNode &stage);

	void               AddLocalPixelShader();
	void               AddLocalPixelShader(const CPixelShaderNode &pixelShaderNode);
	void               RemovePixelShader(CPixelShaderNode &pixelShaderNode);

	CStageNode        *GetFirstStage();
	CStageNode        *GetNextStage(const CStageNode &stageNode);

	const CStageNode  *GetFirstStage() const;
	const CStageNode  *GetNextStage(const CStageNode &stageNode) const;

	CFixedFunctionPipelineNode        *GetFixedFunctionPipeline();
	const CFixedFunctionPipelineNode  *GetFixedFunctionPipeline() const;

	CVertexShaderNode                 *GetVertexShader();
	const CVertexShaderNode           *GetVertexShader() const;

	CPixelShaderNode                  *GetPixelShader();
	const CPixelShaderNode            *GetPixelShader() const;

public:

	int      m_textureStageCount;
	int      m_pixelShaderCount;

	CString  m_textureFactorTag;
	CString  m_textureFactor2Tag;
	CString  m_textureScrollTag;
	CString  m_shadeMode;
	BOOL     m_ditherEnable;
	BOOL     m_heat;
	CString  m_stencilWriteMask;
	CString  m_stencilMask;
	CString  m_stencilReferenceTag;
	CString  m_stencilCompareFunction;
	CString  m_stencilPassOperation;
	CString  m_stencilZFailOperation;
	CString  m_stencilFailOperation;
	CString  m_stencilCounterClockwiseCompareFunction;
	CString  m_stencilCounterClockwisePassOperation;
	CString  m_stencilCounterClockwiseZFailOperation;
	CString  m_stencilCounterClockwiseFailOperation;
	BOOL     m_stencilEnable;
	BOOL     m_stencilTwoSidedMode;
	CString  m_alphaTestFunction;
	CString  m_alphaTestTag;
	BOOL     m_alphaTestEnable;
	CString  m_alphaBlendOperation;
	CString  m_alphaBlendDestination;
	CString  m_alphaBlendSource;
	BOOL     m_alphaBlendEnable;
	CString  m_zCompare;
	BOOL     m_zWrite;
	BOOL     m_zEnable;
	BOOL     m_writeMaskA;
	BOOL     m_writeMaskR;
	BOOL     m_writeMaskG;
	BOOL     m_writeMaskB;
	CString  m_fogMode;
	CString  m_materialTag;

private:

	bool               Load_0000(Iff &iff);
	bool               Load_0001(Iff &iff);
	bool               Load_0002(Iff &iff);
	bool               Load_0003(Iff &iff);
	bool               Load_0004(Iff &iff);
	bool               Load_0005(Iff &iff);
	bool               Load_0006(Iff &iff);
	bool               Load_0007(Iff &iff);
	bool               Load_0008(Iff &iff);
	bool               Load_0009(Iff &iff);
	bool               Load_0010(Iff &iff);

private:

	CImplementationNode &implementation;
};

// ======================================================================

class CFixedFunctionPipelineNode : public CNode
{
	friend class CPassNode;

public:

	CFixedFunctionPipelineNode(CPassNode &pass);
	CFixedFunctionPipelineNode(CPassNode &newImplementation, const CFixedFunctionPipelineNode &copyDataFrom);
	virtual ~CFixedFunctionPipelineNode();

	virtual bool IsLocked(Whom whom) const;

	void Delete();
	bool Validate();
	bool Save(Iff &iff) const;
	bool Load(Iff &iff);

public:

	CString  m_lightingEmissiveColorSource;
	CString  m_lightingSpecularColorSource;
	CString  m_lightingDiffuseColorSource;
	CString  m_lightingAmbientColorSource;
	CString  m_deprecated_lightingMaterialTag;
	BOOL     m_lightingColorVertex;
	BOOL     m_lightingSpecularEnable;
	BOOL     m_lighting;

private:

	bool Load_0000(Iff &iff);
	bool Load_0001(Iff &iff);

	CPassNode &m_pass;
};

// ======================================================================

class CVertexShaderNode : public CNode
{
public:

	CVertexShaderNode(CPassNode &pass);
	CVertexShaderNode(CPassNode &newImplementation, const CVertexShaderNode &copyDataFrom);
	virtual ~CVertexShaderNode();

	virtual bool IsLocked(Whom whom) const;

	void Delete();
	bool Validate();
	bool Save(Iff &iff) const;
	bool Load(Iff &iff);

public:

	CString	 m_fileName;

private:

	CPassNode &m_pass;
};

// ======================================================================

class CStageNode : public CNode
{
public:
	CStageNode(CPassNode &newPass);
	CStageNode(CPassNode &newPass, const CStageNode &copyDataFrom, HTREEITEM after);
	virtual ~CStageNode();

	virtual bool     IsLocked(Whom whom) const;

	bool             Load(Iff &iff);
	bool             Load_0000(Iff &iff);
	bool             Load_0001(Iff &iff);
	bool             Save(Iff &iff) const;
	bool             Validate();

	void             Delete();

	CPassNode       &GetPass();
	const CPassNode &GetPass() const;

public:

	// ----------
	// Deprecated data

	CString	m_textureAddressU;
	CString	m_textureAddressV;
	CString	m_textureAddressW;
	CString	m_textureMipFilter;
	CString	m_textureMinificationFilter;
	CString	m_textureMagnificationFilter;

	// ----------

	CString	m_textureCoordinateGeneration;
	CString	m_textureCoordinateSetTag;
	CString	m_textureTag;
	CString	m_resultArgument;
	CString	m_alphaArgument0;
	CString	m_alphaArgument2;
	CString	m_alphaOperation;
	CString	m_alphaArgument1;
	CString	m_colorArgument0;
	CString	m_colorArgument2;
	CString	m_colorOperation;
	CString	m_colorArgument1;
	BOOL    m_colorArgument1AlphaReplicate;
	BOOL    m_colorArgument1Complement;
	BOOL    m_colorArgument2AlphaReplicate;
	BOOL    m_colorArgument2Complement;
	BOOL    m_colorArgument0AlphaReplicate;
	BOOL    m_colorArgument0Complement;
	BOOL    m_alphaArgument1Complement;
	BOOL    m_alphaArgument2Complement;
	BOOL    m_alphaArgument0Complement;

private:

	CPassNode &pass;
};

// ======================================================================

class CPixelShaderNode : public CNode
{
public:

	CPixelShaderNode(CPassNode &newPass);
	CPixelShaderNode(CPassNode &newPass, const CPixelShaderNode &copyDataFrom);
	virtual ~CPixelShaderNode();

	virtual bool     IsLocked(Whom whom) const;

	bool             Load(Iff &iff);
	bool             Save(Iff &iff) const;
	bool             Validate();

	void             Delete();

	CPassNode       &GetPass();
	const CPassNode &GetPass() const;

public:

	CString	  m_fileName;
	CString	  m_t0;
	CString	  m_t1;
	CString	  m_t2;
	CString	  m_t3;
	CString	  m_t4;
	CString	  m_t5;
	CString	  m_t6;
	CString	  m_t7;
	CString	  m_t8;
	CString	  m_t9;
	CString	  m_t10;
	CString	  m_t11;
	CString	  m_t12;
	CString	  m_t13;
	CString	  m_t14;
	CString	  m_t15;
	CString * m_textures[16];

	static int  ms_compiledIndex;

private:

	void setupTextureArray();

	bool Load_0000(Iff &iff);
	bool Load_0001(Iff &iff);

	bool Load_texture(Iff &iff);
	bool Load_texture_0000(Iff &iff);
	bool Load_texture_0001(Iff &iff);
	bool Load_texture_0002(Iff &iff);

private:

	CPassNode &m_pass;
};

// ======================================================================

class CMaterialNode : public CNode
{
public:
	CMaterialNode(const CString &name);
	virtual ~CMaterialNode();

	bool Load(Iff &iff);
	bool Save(Iff &iff) const;
	bool Validate();

public:

	CString  m_power;
	CString  m_emissiveBlue;
	CString  m_emissiveGreen;
	CString  m_emissiveRed;
	CString  m_emissiveAlpha;
	CString  m_specularBlue;
	CString  m_specularGreen;
	CString  m_specularRed;
	CString  m_specularAlpha;
	CString  m_ambientBlue;
	CString  m_ambientGreen;
	CString  m_ambientRed;
	CString  m_ambientAlpha;
	CString  m_diffuseBlue;
	CString  m_diffuseGreen;
	CString  m_diffuseRed;
	CString  m_diffuseAlpha;

	int referenceCount;
};

// ======================================================================

class CTextureNode : public CNode
{
public:
	CTextureNode(const CString &name);
	virtual ~CTextureNode();

	bool Load(Iff &iff, CString &name);
	bool Save(const CString &name, Iff &iff) const;
	bool Validate();

	bool Load_0000	( Iff & iff, CString & name );
	bool Load_0001	( Iff & iff, CString & name );
	bool Load_0002	( Iff & iff, CString & name );

public:

	BOOL    m_placeHolder;
	CString m_textureName;

	CString	m_addressU;
	CString	m_addressV;
	CString	m_addressW;

	CString	m_filterMip;
	CString	m_filterMin;
	CString	m_filterMag;

	int     m_anisotropy;

	int referenceCount;
};

// ======================================================================

class CTextureCoordinateSetNode : public CNode
{
public:
	CTextureCoordinateSetNode(const CString &name);
	virtual ~CTextureCoordinateSetNode();

	bool Load(Iff &iff);
	bool Save(Iff &iff) const;
	bool Validate();

public:
	CString m_textureCoordinateSetIndex;

	int referenceCount;
};

// ======================================================================

class CTextureFactorNode : public CNode
{
public:
	CTextureFactorNode(const CString &name);
	virtual ~CTextureFactorNode();

	bool Load(Iff &iff);
	bool Save(Iff &iff) const;
	bool Validate();

public:

	CString m_blue;
	CString m_green;
	CString m_red;
	CString m_alpha;

	int referenceCount;
};

// ======================================================================

class CTextureScrollNode : public CNode
{
public:

	CTextureScrollNode(const CString &name);
	virtual ~CTextureScrollNode();

	bool Load(Iff &iff);
	bool Save(Iff &iff) const;
	bool Validate();

public:

	CString  m_textureScroll1U;
	CString  m_textureScroll1V;
	CString  m_textureScroll2U;
	CString  m_textureScroll2V;

	int referenceCount;
};

// ======================================================================

class CAlphaReferenceValueNode : public CNode
{
public:
	CAlphaReferenceValueNode(const CString &name);
	virtual ~CAlphaReferenceValueNode();

	bool Load(Iff &iff);
	bool Save(Iff &iff) const;
	bool Validate();

public:

	CString m_alphaReferenceValue;

	int referenceCount;
};

// ======================================================================

class CStencilReferenceValueNode : public CNode
{
public:
	CStencilReferenceValueNode(const CString &name);
	virtual ~CStencilReferenceValueNode();

	bool Load(Iff &iff);
	bool Save(Iff &iff) const;
	bool Validate();

public:

	CString m_stencilReferenceValue;

	int referenceCount;
};

// ======================================================================

class CVertexShaderProgramNode : public CNode
{
public:

	CVertexShaderProgramNode(const CString &name, bool manual);
	virtual ~CVertexShaderProgramNode();

	void Load(const CString &name);
	bool Save();

public:
	int                   referenceCount;
	CString               m_absoluteFileName;
	CString               m_text;
	std::vector<CString>  m_textureCoordinateSetTags;
	int                   m_scroll;
	bool                  m_manuallyLoaded;
};

// ======================================================================

class CPixelShaderProgramNode : public CNode
{
public:

	CPixelShaderProgramNode(const CString &name, bool m_manuallyLoaded);
	virtual ~CPixelShaderProgramNode();

	void Load(const CString &name);
	bool Save();

public:
	int      referenceCount;
	CString	 m_absoluteFileName;
	CString	 m_text;
	int      m_exeLength;
	byte    *m_exe;
	int      m_compiledIndex;
	int      m_scroll;
	bool     m_manuallyLoaded;
};

// ======================================================================

class CVertexShaderProgramTreeNode : public CNode
{
public:

	CVertexShaderProgramTreeNode();
	virtual ~CVertexShaderProgramTreeNode();

	void Add(const char *name, bool manual);
	void Remove(const char *name, bool manual);

	void CompileAll(CVertexShaderProgramNode * start=NULL);
};

// ======================================================================

class CPixelShaderProgramTreeNode : public CNode
{
public:

	CPixelShaderProgramTreeNode();
	virtual ~CPixelShaderProgramTreeNode();

	void Add(const char *name, bool manual);
	void Remove(const char *name, bool manual);

	void CompileAll(CPixelShaderProgramNode * start=NULL);
};

// ======================================================================

class CIncludeNode : public CNode
{
public:

	CIncludeNode(const CString &name);
	virtual ~CIncludeNode();

	bool Save();

public:

	int                   referenceCount;
	CString               m_absoluteFileName;
	bool                  m_changed;
	CString               m_text;

	int                   m_scroll;
};

// ======================================================================

class CIncludeTreeNode : public CTaggedTreeNode<CIncludeNode>
{
public:

	CIncludeTreeNode();
	virtual ~CIncludeTreeNode();

	void SaveAll();
};

// ======================================================================

template <class T> void CTaggedTreeNode<T>::Add(const char *name)
{
	ASSERT(name);
	if (name[0])
	{
		// look for it
		ChildNode *node = static_cast<ChildNode *>(FindNamedChild(name));

		if (!node)
		{
			// create it if it wasn't found
			node  = new ChildNode(name);
			InsertItem(name, *node);

			// resort the children to be asthetically pleasing
			treeCtrl->SortChildren(treeItem);
		}

		// another pass refers to this material now
		++node->referenceCount;
	}
}

// ----------------------------------------------------------------------

template <class T>
void CTaggedTreeNode<T>::Remove(const char *name)
{
	ASSERT(name);
	if (name[0])
	{
		ChildNode *node = static_cast<ChildNode *>(FindNamedChild(name));

		ASSERT(node);
		if (--node->referenceCount == 0)
		{
			RemoveItem(*node);
			delete node;
		}
	}
}

// ----------------------------------------------------------------------

template <class T>
void CTaggedTreeNode<T>::GetChildTags(std::set<CString> &tags)
{
	for (HTREEITEM child = treeCtrl->GetChildItem(treeItem); child; child = treeCtrl->GetNextSiblingItem(child))
		tags.insert(treeCtrl->GetItemText(child));
}

// ======================================================================

#endif
