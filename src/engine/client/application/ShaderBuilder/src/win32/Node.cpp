// ======================================================================
// Node.cpp
// ======================================================================

#include "FirstShaderBuilder.h"
#include "Node.h"

#include "LeftView.h"
#include "clientGraphics/ShaderCapability.h"
#include "clientGraphics/ShaderImplementation.h"
#include "sharedFile/Iff.h"
#include "MainFrm.h"
#include "Passview.h"
#include "StageView.h"
#include "TextureView.h"
#include "FixedFunctionPipelineView.h"
#include "sharedFile/TreeFile.h"
#include "ImplementationView.h"
#include "VertexShaderProgram.h"
#include "PixelShaderProgramView.h"

#include "sharedFoundation/Os.h"

#include <map>
#include <assert.h>

// ======================================================================

const Tag TAG_TAG  = TAG3(T,A,G);
const Tag TAG_TXM  = TAG3(T,X,M);

const Tag TAG_AREF = TAG(A,R,E,F);
const Tag TAG_ARVS = TAG(A,R,V,S);
const Tag TAG_CSHD = TAG(C,S,H,D);
const Tag TAG_EFCT = TAG(E,F,C,T);
const Tag TAG_FFPN = TAG(F,F,P,N);
const Tag TAG_IMPL = TAG(I,M,P,L);
const Tag TAG_MATL = TAG(M,A,T,L);
const Tag TAG_MATS = TAG(M,A,T,S);
const Tag TAG_OPTN = TAG(O,P,T,N);
const Tag TAG_PASS = TAG(P,A,S,S);
const Tag TAG_PEXE = TAG(P,E,X,E);
const Tag TAG_PFFP = TAG(P,F,F,P);
const Tag TAG_PIXL = TAG(P,I,X,L);
const Tag TAG_PPSH = TAG(P,P,S,H);
const Tag TAG_PSHP = TAG(P,S,H,P);
const Tag TAG_PSRC = TAG(P,S,R,C);
const Tag TAG_PTCH = TAG(P,T,C,H);
const Tag TAG_PTXM = TAG(P,T,X,M);
const Tag TAG_PVSH = TAG(P,V,S,H);
const Tag TAG_SCAP = TAG(S,C,A,P);
const Tag TAG_SREF = TAG(S,R,E,F);
const Tag TAG_SRVS = TAG(S,R,V,S);
const Tag TAG_SSHT = TAG(S,S,H,T);
const Tag TAG_STAG = TAG(S,T,A,G);
const Tag TAG_STGS = TAG(S,T,G,S);
const Tag TAG_TCSI = TAG(T,C,S,I);
const Tag TAG_TCSS = TAG(T,C,S,S);
const Tag TAG_TFCT = TAG(T,F,C,T);
const Tag TAG_TFNS = TAG(T,F,N,S);
const Tag TAG_TSNS = TAG(T,S,N,S);
const Tag TAG_TXMS = TAG(T,X,M,S);
const Tag TAG_VBUF = TAG(V,B,U,F);
const Tag TAG_VEXE = TAG(V,E,X,E);
const Tag TAG_VSHP = TAG(V,S,H,P);
const Tag TAG_VSRC = TAG(V,S,R,C);
const Tag TAG_VTAG = TAG(V,T,A,G);

Map CNode::ms_shadeMode;
Map CNode::ms_compare;
Map CNode::ms_blend;
Map CNode::ms_blendOp;
Map CNode::ms_materialSource;
Map CNode::ms_stencilOp;
Map CNode::ms_textureOp;
Map CNode::ms_textureArg;
Map CNode::ms_textureAddress;
Map CNode::ms_textureFilter;
Map CNode::ms_coordinateGeneration;
Map CNode::ms_textureIndex;
Map CNode::ms_fogMode;

int CPixelShaderNode::ms_compiledIndex;

// ======================================================================

Tag convertToTag(const char *buffer)
{
	Tag tag = 0;

	if (!buffer[0])
		return 0;

	int i;
	for (i = 0; i < 4 && *buffer; ++i, ++buffer)
		tag = (tag << 8) | static_cast<uint32>(*buffer);

	for (; i < 4; ++i)
		tag = (tag << 8) | static_cast<uint32>(' ');

	return tag;
}

void IffReadMap(Iff &iff, CString &string, const Map &m)
{
	const int8 value = iff.read_int8();

	Map::const_iterator end = m.end();
	for (Map::const_iterator i = m.begin(); i != end; ++i)
		if (i->second == value)
		{
			string = i->first;
			return;
		}

	assert(false);
}

void IffReadTag(Iff &iff, CString &string)
{
	uint32 value = iff.read_uint32();
	char buffer[5];

	// convert from tag to string
	for (int i = 3; i >= 0; --i, value >>= 8)
		buffer[i] = static_cast<char>(value & 0xff);
	buffer[4] = '\0';

	string = buffer;
}

void IffReadByte(Iff &iff, CString &string)
{
	const uint8 value = iff.read_uint8();
	char buffer[16];
	_itoa(value, buffer, 10);
	string = buffer;
}

void IffReadInt(Iff &iff, CString &string)
{
	const uint32 value = iff.read_uint32();
	char buffer[16];
	_itoa(value, buffer, 10);
	string = buffer;
}

void IffReadBool(Iff &iff, BOOL &b)
{
	b = iff.read_int8() ? true : false;
}

void IffReadFloat(Iff &iff, CString &string)
{
	const float value = iff.read_float();
	char buffer[16];
	sprintf(buffer, "%f", value);
	string = buffer;
}

void IffReadString(Iff &iff, CString &string)
{
	char buffer[8 * 1024];
	iff.read_string(buffer, sizeof(buffer));
	string = buffer;
}

// ======================================================================

CTreeCtrl         *CNode::treeCtrl;
CLeftView         *CNode::leftView;
CTemplateTreeNode *CTemplateTreeNode ::instance;

// ======================================================================

CNode::CNode(Type newType)
:
	type(newType),
	treeItem(NULL),
	children(),
	isRendering(false)
{
}

// ----------------------------------------------------------------------

CNode::~CNode()
{
	leftView->DestroyNode(this);

	// destroy all my children first
	Children::iterator end = children.end();
	for (Children::iterator i = children.begin(); i != end; ++i)
		delete i->second;

	// when closing the app, the TreeCtrl window is gone, and deleting items will cause assertions
	if (treeCtrl->GetSafeHwnd())
	{
		// remove my tree element
		treeCtrl->DeleteItem(treeItem);
		treeCtrl->Invalidate();
		treeItem = NULL;
	}
}

// ----------------------------------------------------------------------

void CNode::install()
{
	ms_textureOp["Select Argument 1"]                = ShaderImplementation::Pass::Stage:: TO_selectArg1;
	ms_textureOp["Select Argument 2"]                = ShaderImplementation::Pass::Stage:: TO_selectArg2;
	ms_textureOp["Modulate"]                         = ShaderImplementation::Pass::Stage:: TO_modulate;
	ms_textureOp["Modulate 2x"]                      = ShaderImplementation::Pass::Stage:: TO_modulate2x;
	ms_textureOp["Modulate 4x"]                      = ShaderImplementation::Pass::Stage:: TO_modulate4x;
	ms_textureOp["Add"]                              = ShaderImplementation::Pass::Stage:: TO_add;
	ms_textureOp["Add Signed"]                       = ShaderImplementation::Pass::Stage:: TO_addSigned;
	ms_textureOp["Add Signed 2x"]                    = ShaderImplementation::Pass::Stage:: TO_addSigned2x;
	ms_textureOp["Subtract"]                         = ShaderImplementation::Pass::Stage:: TO_subtract;
	ms_textureOp["Add Smooth"]                       = ShaderImplementation::Pass::Stage:: TO_addSmooth;
	ms_textureOp["Blend Diffuse Alpha"]              = ShaderImplementation::Pass::Stage:: TO_blendDiffuseAlpha;
	ms_textureOp["Blend Texture Alpha"]              = ShaderImplementation::Pass::Stage:: TO_blendTextureAlpha;
	ms_textureOp["Blend Factor Alpha"]               = ShaderImplementation::Pass::Stage:: TO_blendFactorAlpha;
	ms_textureOp["Blend Texture Alpha Premodulate"]  = ShaderImplementation::Pass::Stage:: TO_blendTextureAlphapm;
	ms_textureOp["Blend Current Alpha"]              = ShaderImplementation::Pass::Stage:: TO_blendCurrentAlpha;
	ms_textureOp["Premodulate"]                      = ShaderImplementation::Pass::Stage:: TO_premodulate;
	ms_textureOp["Modulate alpha add color"]         = ShaderImplementation::Pass::Stage:: TO_modulateAlpha_addColor;
	ms_textureOp["Modulate color add alpha"]         = ShaderImplementation::Pass::Stage:: TO_modulateColor_addAlpha;
	ms_textureOp["Modulate inverse alpha add color"] = ShaderImplementation::Pass::Stage:: TO_modulateInvalpha_addColor;
	ms_textureOp["Modulate inverse color add alpha"] = ShaderImplementation::Pass::Stage:: TO_modulateInvcolor_addAlpha;
	ms_textureOp["Bump Environment Map"]             = ShaderImplementation::Pass::Stage:: TO_bumpEnvMap;
	ms_textureOp["Bump Environment Map Luminance"]   = ShaderImplementation::Pass::Stage:: TO_bumpEnvMapLuminance;
	ms_textureOp["Dot Product 3"]                    = ShaderImplementation::Pass::Stage:: TO_dotProduct3;
	ms_textureOp["Multiply Add"]                     = ShaderImplementation::Pass::Stage:: TO_multiplyAdd;
	ms_textureOp["Lerp"]                             = ShaderImplementation::Pass::Stage:: TO_lerp;

	ms_textureArg["Current"]                         = ShaderImplementation::Pass::Stage:: TA_current;
	ms_textureArg["Diffuse"]                         = ShaderImplementation::Pass::Stage:: TA_diffuse;
	ms_textureArg["Specular"]                        = ShaderImplementation::Pass::Stage:: TA_specular;
	ms_textureArg["Temp"]                            = ShaderImplementation::Pass::Stage:: TA_temp;
	ms_textureArg["Texture"]                         = ShaderImplementation::Pass::Stage:: TA_texture;
	ms_textureArg["Texture Factor"]                  = ShaderImplementation::Pass::Stage:: TA_textureFactor;


	ms_textureAddress["Invalid"]                     = ShaderImplementation::Pass::Stage:: TA_invalid;
	ms_textureAddress["Wrap"]                        = ShaderImplementation::Pass::Stage:: TA_wrap;
	ms_textureAddress["Mirror"]                      = ShaderImplementation::Pass::Stage:: TA_mirror;
	ms_textureAddress["Clamp"]                       = ShaderImplementation::Pass::Stage:: TA_clamp;
	ms_textureAddress["Border"]                      = ShaderImplementation::Pass::Stage:: TA_border;
	ms_textureAddress["Mirror once"]                 = ShaderImplementation::Pass::Stage:: TA_mirrorOnce;

	ms_textureFilter["Invalid"]                      = ShaderImplementation::Pass::Stage:: TF_invalid;
	ms_textureFilter["None"]                         = ShaderImplementation::Pass::Stage:: TF_none;
	ms_textureFilter["Point"]                        = ShaderImplementation::Pass::Stage:: TF_point;
	ms_textureFilter["Linear"]                       = ShaderImplementation::Pass::Stage:: TF_linear;
	ms_textureFilter["Anisotropic"]                  = ShaderImplementation::Pass::Stage:: TF_anisotropic;
	ms_textureFilter["Flat cubic"]                   = ShaderImplementation::Pass::Stage:: TF_flatCubic;
	ms_textureFilter["Gaussian cubic"]               = ShaderImplementation::Pass::Stage:: TF_gaussianCubic;

	ms_coordinateGeneration["Pass Thru"]             = ShaderImplementation::Pass::Stage:: CG_passThru;
	ms_coordinateGeneration["CS Position"]           = ShaderImplementation::Pass::Stage:: CG_cameraSpacePosition;
	ms_coordinateGeneration["CS Normal"]             = ShaderImplementation::Pass::Stage:: CG_cameraSpaceNormal;
	ms_coordinateGeneration["CS Reflection Vector"]  = ShaderImplementation::Pass::Stage:: CG_cameraSpaceReflectionVector;
	ms_coordinateGeneration["Scroll 1"]              = ShaderImplementation::Pass::Stage:: CG_scroll1;
	ms_coordinateGeneration["Scroll 2"]              = ShaderImplementation::Pass::Stage:: CG_scroll2;

	ms_shadeMode["Flat"]                             = ShaderImplementation::Pass::SM_Flat;
	ms_shadeMode["Gouraud"]                          = ShaderImplementation::Pass::SM_Gouraud;

	ms_compare["Never"]                              = ShaderImplementation::Pass::C_Never;
	ms_compare["Less"]                               = ShaderImplementation::Pass::C_Less;
	ms_compare["Equal"]                              = ShaderImplementation::Pass::C_Equal;
	ms_compare["Less or Equal"]                      = ShaderImplementation::Pass::C_LessOrEqual;
	ms_compare["Greater"]                            = ShaderImplementation::Pass::C_Greater;
	ms_compare["Greater or Equal"]                   = ShaderImplementation::Pass::C_GreaterOrEqual;
	ms_compare["Not Equal"]                          = ShaderImplementation::Pass::C_NotEqual;
	ms_compare["Always"]                             = ShaderImplementation::Pass::C_Always;

	ms_blend["Zero"]                                 = ShaderImplementation::Pass::B_Zero;
	ms_blend["One"]                                  = ShaderImplementation::Pass::B_One;
	ms_blend["Source Color"]                         = ShaderImplementation::Pass::B_SourceColor;
	ms_blend["Inverse Source Color"]                 = ShaderImplementation::Pass::B_InverseSourceColor;
	ms_blend["Source Alpha"]                         = ShaderImplementation::Pass::B_SourceAlpha;
	ms_blend["Inverse Source Alpha"]                 = ShaderImplementation::Pass::B_InverseSourceAlpha;
	ms_blend["Destination Alpha"]                    = ShaderImplementation::Pass::B_DestinationAlpha;
	ms_blend["Inverse Destination Alpha"]            = ShaderImplementation::Pass::B_InverseDestinationAlpha;
	ms_blend["Destination Color"]                    = ShaderImplementation::Pass::B_DestinationColor;
	ms_blend["Inverse Destination Color"]            = ShaderImplementation::Pass::B_InverseDestinationColor;
	ms_blend["Source Alpha Saturate"]                = ShaderImplementation::Pass::B_SourceAlphaSaturate;

	ms_blendOp["Add"]                                = ShaderImplementation::Pass::BO_Add;
	ms_blendOp["Subtract"]                           = ShaderImplementation::Pass::BO_Subtract;
	ms_blendOp["Reverse Subtract"]                   = ShaderImplementation::Pass::BO_ReverseSubtract;
	ms_blendOp["Min"]                                = ShaderImplementation::Pass::BO_Min;
	ms_blendOp["Max"]                                = ShaderImplementation::Pass::BO_Max;

	ms_materialSource["Material"]                    = ShaderImplementation::Pass::FixedFunctionPipeline::MS_Material;
	ms_materialSource["Vertex color 0"]              = ShaderImplementation::Pass::FixedFunctionPipeline::MS_VertexColor0;
	ms_materialSource["Vertex color 1"]              = ShaderImplementation::Pass::FixedFunctionPipeline::MS_VertexColor1;

	ms_stencilOp["Keep"]                             = ShaderImplementation::Pass::SO_Keep;
	ms_stencilOp["Zero"]                             = ShaderImplementation::Pass::SO_Zero;
	ms_stencilOp["Replace"]                          = ShaderImplementation::Pass::SO_Replace;
	ms_stencilOp["Increment Saturate"]               = ShaderImplementation::Pass::SO_IncrementSaturate;
	ms_stencilOp["Decrement Saturate"]               = ShaderImplementation::Pass::SO_DecrementSaturate;
	ms_stencilOp["Invert"]                           = ShaderImplementation::Pass::SO_Invert;
	ms_stencilOp["Increment Wrap"]                   = ShaderImplementation::Pass::SO_IncrementWrap;
	ms_stencilOp["Decrement Wrap"]                   = ShaderImplementation::Pass::SO_DecrementWrap;

	ms_textureIndex["t0"]                            = 0;
	ms_textureIndex["t1"]                            = 1;
	ms_textureIndex["t2"]                            = 2;
	ms_textureIndex["t3"]                            = 3;
	ms_textureIndex["t4"]                            = 4;
	ms_textureIndex["t5"]                            = 5;
	ms_textureIndex["t6"]                            = 6;
	ms_textureIndex["t7"]                            = 7;
	ms_textureIndex["t8"]                            = 8;
	ms_textureIndex["t9"]                            = 9;
	ms_textureIndex["t10"]                           = 10;
	ms_textureIndex["t11"]                           = 11;
	ms_textureIndex["t12"]                           = 12;
	ms_textureIndex["t13"]                           = 13;
	ms_textureIndex["t14"]                           = 14;
	ms_textureIndex["t15"]                           = 15;

	ms_fogMode["Normal"]                          = ShaderImplementation::Pass::FM_Normal;
	ms_fogMode["Black"]                           = ShaderImplementation::Pass::FM_Black;
	ms_fogMode["White"]                           = ShaderImplementation::Pass::FM_White;
}

// ----------------------------------------------------------------------

void CNode::remove()
{
	ms_textureOp.clear();
	ms_textureArg.clear();
	ms_textureAddress.clear();
	ms_textureFilter.clear();
	ms_coordinateGeneration.clear();
	ms_shadeMode.clear();
	ms_compare.clear();
	ms_blend.clear();
	ms_blendOp.clear();
	ms_materialSource.clear();
	ms_stencilOp.clear();
}

// ----------------------------------------------------------------------

CNode::Type CNode::GetType() const
{
	return type;
}

// ----------------------------------------------------------------------

void CNode::GetName(CString &string) const
{
	string = treeCtrl->GetItemText(treeItem);
}

// ----------------------------------------------------------------------

HTREEITEM CNode::GetTreeItem() const
{
	return treeItem;
}

// ----------------------------------------------------------------------

bool CNode::IsLocked(Whom) const
{
	return false;
}

// ----------------------------------------------------------------------

void CNode::SetItemImages() const
{
	int base = 0;
	if (IsLocked(Either))
		base += 4;
	if (isRendering)
		base += 2;

	treeCtrl->SetItemImage(treeItem, base, base+1);
}

// ----------------------------------------------------------------------

void CNode::SetRendering(bool rendering) const
{
	isRendering = rendering;
	SetItemImages();
}

// ----------------------------------------------------------------------

void CNode::InsertItem(const char *name, CNode &node, HTREEITEM after)
{
	// insert as a child of this item
	node.treeItem = treeCtrl->InsertItem(name, 0, 1, treeItem, after);
	treeCtrl->Invalidate();
	children[node.treeItem] = &node;
}

// ----------------------------------------------------------------------

void CNode::RemoveItem(CNode &node)
{
	Children::iterator i = children.find(node.treeItem);
	ASSERT(i != children.end());
	children.erase(i);
	treeCtrl->DeleteItem(node.treeItem);
}

// ----------------------------------------------------------------------

CNode *CNode::FindNamedChild(const char *name)
{
	Children::iterator end = children.end();
	for (Children::iterator i = children.begin(); i != end; ++i)
	{
		CString itemName = treeCtrl->GetItemText(i->first);
		if (itemName == name)
			return i->second;
	}

	return NULL;
}

// ----------------------------------------------------------------------

CNode *CNode::Find(HTREEITEM item)
{
	// check me
	if (item == treeItem)
		return this;

	// check my children
	Children::iterator end = children.end();
	for (Children::iterator i = children.begin(); i != end; ++i)
	{
		CNode *result = i->second->Find(item);
		if (result)
			return result;
	}

	// not found
	return NULL;
}

// ----------------------------------------------------------------------

CNode *CNode::GetFirstChild()
{
	UNREF(type);
	HTREEITEM childPassItem = treeCtrl->GetChildItem(treeItem);
	if (!childPassItem)
		return false;
	Children::const_iterator childPassIterator = children.find(childPassItem);
	ASSERT(childPassIterator != children.end());
	return childPassIterator->second;
}

// ----------------------------------------------------------------------

CNode *CNode::GetNextChild(const CNode &node)
{
	UNREF(type);
	HTREEITEM nextPassItem = treeCtrl->GetNextSiblingItem(node.treeItem);
	if (!nextPassItem)
		return false;
	Children::const_iterator nextPass = children.find(nextPassItem);
	ASSERT(nextPass != children.end());
	return nextPass->second;
}

// ----------------------------------------------------------------------

const CNode *CNode::GetFirstChild() const
{
	UNREF(type);
	HTREEITEM childPassItem = treeCtrl->GetChildItem(treeItem);
	if (!childPassItem)
		return false;
	Children::const_iterator childPassIterator = children.find(childPassItem);
	ASSERT(childPassIterator != children.end());
	return childPassIterator->second;
}

// ----------------------------------------------------------------------

const CNode *CNode::GetNextChild(const CNode &node) const
{
	UNREF(type);
	HTREEITEM nextPassItem = treeCtrl->GetNextSiblingItem(node.treeItem);
	if (!nextPassItem)
		return false;
	Children::const_iterator nextPass = children.find(nextPassItem);
	ASSERT(nextPass != children.end());
	return nextPass->second;
}

// ----------------------------------------------------------------------

CNode *CNode::GetFirstChild(CNode::Type type)
{
	UNREF(type);
	HTREEITEM childPassItem = treeCtrl->GetChildItem(treeItem);
	if (!childPassItem)
		return false;
	Children::const_iterator childPassIterator = children.find(childPassItem);
	ASSERT(childPassIterator != children.end());
	ASSERT(childPassIterator->second->GetType() == type);
	return childPassIterator->second;
}

// ----------------------------------------------------------------------

CNode *CNode::GetNextChild(CNode::Type type, const CNode &node)
{
	UNREF(type);
	HTREEITEM nextPassItem = treeCtrl->GetNextSiblingItem(node.treeItem);
	if (!nextPassItem)
		return false;
	Children::const_iterator nextPass = children.find(nextPassItem);
	ASSERT(nextPass != children.end());
	ASSERT(nextPass->second->GetType() == type);
	return nextPass->second;
}

// ----------------------------------------------------------------------

const CNode *CNode::GetFirstChild(CNode::Type type) const
{
	UNREF(type);
	HTREEITEM childPassItem = treeCtrl->GetChildItem(treeItem);
	if (!childPassItem)
		return false;
	Children::const_iterator childPassIterator = children.find(childPassItem);
	ASSERT(childPassIterator != children.end());
	ASSERT(childPassIterator->second->GetType() == type);
	return childPassIterator->second;
}

// ----------------------------------------------------------------------

const CNode *CNode::GetNextChild(CNode::Type type, const CNode &node) const
{
	UNREF(type);
	HTREEITEM nextPassItem = treeCtrl->GetNextSiblingItem(node.treeItem);
	if (!nextPassItem)
		return false;
	Children::const_iterator nextPass = children.find(nextPassItem);
	ASSERT(nextPass != children.end());
	ASSERT(nextPass->second->GetType() == type);
	return nextPass->second;
}

// ======================================================================

CTemplateTreeNode::CTemplateTreeNode(CLeftView &newLeftView, CTreeCtrl &newTreeCtrl)
: CNode(Template),
	materialTreeNode(new CMaterialTreeNode()),
	textureTreeNode(new CTextureTreeNode()),
	textureCoordinateSetTreeNode(new CTextureCoordinateSetTreeNode()),
	textureFactorTreeNode(new CTextureFactorTreeNode()),
	textureScrollTreeNode(new CTextureScrollTreeNode()),
	alphaReferenceValueTreeNode(new CAlphaReferenceValueTreeNode()),
	stencilReferenceValueTreeNode(new CStencilReferenceValueTreeNode()),
	effectNode(new CEffectNode()),
	includeTreeNode(NULL),
	vertexShaderProgramTreeNode(NULL),
	pixelShaderProgramTreeNode(NULL)

{
	// save off the tree control
	leftView = &newLeftView;
	treeCtrl = &newTreeCtrl;

	// add a item for myself
	treeItem = treeCtrl->InsertItem("Shader Template");
	treeCtrl->SetItemImage(treeItem, 0, 1);

	// add the children to the tree
	InsertItem("Materials",                *materialTreeNode);
	InsertItem("Textures",                 *textureTreeNode);
	InsertItem("Texture Coordinate Sets",  *textureCoordinateSetTreeNode);
	InsertItem("Texture Factors",          *textureFactorTreeNode);
	InsertItem("Texture Scroll",           *textureScrollTreeNode);
	InsertItem("Alpha Reference Values",   *alphaReferenceValueTreeNode);
	InsertItem("Stencil Reference Values", *stencilReferenceValueTreeNode);
	InsertItem("Effect",                   *effectNode);

	// expand the shader template
	treeCtrl->Expand(treeItem, TVE_EXPAND);

	// remember me
	instance = this;

	includeTreeNode = new CIncludeTreeNode;
	vertexShaderProgramTreeNode = new CVertexShaderProgramTreeNode;
	pixelShaderProgramTreeNode = new CPixelShaderProgramTreeNode;
}

// ----------------------------------------------------------------------

CTemplateTreeNode::~CTemplateTreeNode()
{
	instance = NULL;

	delete includeTreeNode;
	delete vertexShaderProgramTreeNode;
	delete pixelShaderProgramTreeNode;
}

// ----------------------------------------------------------------------

CNode *CTemplateTreeNode::Find(HTREEITEM item)
{
	CNode *result = NULL;

	// look in the include tree
	result = includeTreeNode->Find(item);
	if (result)
		return result;

	// look in the vertex shader program tree
	result = vertexShaderProgramTreeNode->Find(item);
	if (result)
		return result;

	// look in the pixel shader program tree
	result = pixelShaderProgramTreeNode->Find(item);
	if (result)
		return result;

	return CNode::Find(item);
}

// ----------------------------------------------------------------------

CTemplateTreeNode *CTemplateTreeNode::GetInstance()
{
	return instance;
}

// ----------------------------------------------------------------------

bool CTemplateTreeNode::Load(Iff &iff)
{
	bool customized = false;

	if (iff.getCurrentName() == TAG_CSHD)
	{
		iff.enterForm(TAG_CSHD);
		if (iff.getCurrentName() == TAG_0000)
			iff.enterForm(TAG_0000);
		else
			if (iff.getCurrentName() == TAG_0001)
				iff.enterForm(TAG_0001);
			else
				DEBUG_FATAL(true, ("unknown customizable shader template version"));

		MessageBox(NULL, "Warning, loading customizable shader.  If you save it, you will lose data.", "Warning", MB_ICONERROR | MB_OK);
		customized = true;
	}

	iff.enterForm(TAG_SSHT);

		switch (iff.getCurrentName())
		{
			case TAG_0000:
				if (!Load_0000(iff))
					return false;
				break;

			case TAG_0001:
				if (!Load_0001(iff))
					return false;
				break;

			default:
				return false;

		}

	iff.exitForm(TAG_SSHT);

	if (customized)
	{
		iff.exitForm(true);
		iff.exitForm(true);
	}

	return true;
}

// ----------------------------------------------------------------------

bool CTemplateTreeNode::Load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);

		if (!materialTreeNode->Load(iff))
			return false;

		if (!textureTreeNode->Load(iff))
			return false;

		if (!textureCoordinateSetTreeNode->Load(iff))
			return false;

		if (!textureFactorTreeNode->Load(iff))
			return false;

		if (!alphaReferenceValueTreeNode->Load(iff))
			return false;

		if (!stencilReferenceValueTreeNode->Load(iff))
			return false;

		if (!effectNode->Load(iff))
			return false;

	iff.exitForm(TAG_0000);

	return true;
}

// ----------------------------------------------------------------------

bool CTemplateTreeNode::Load_0001(Iff &iff)
{
	iff.enterForm(TAG_0001);

		if (!effectNode->Load(iff))
			return false;

		if (!materialTreeNode->Load(iff))
			return false;

		if (!textureTreeNode->Load(iff))
			return false;

		if (!textureCoordinateSetTreeNode->Load(iff))
			return false;

		if (!textureFactorTreeNode->Load(iff))
			return false;

		if (!textureScrollTreeNode->Load(iff))
			return false;

		if (!alphaReferenceValueTreeNode->Load(iff))
			return false;

		if (!stencilReferenceValueTreeNode->Load(iff))
			return false;

	iff.exitForm(TAG_0001);

	return true;
}

// ----------------------------------------------------------------------

bool CTemplateTreeNode::Save(Iff &iff) const
{
	iff.insertForm(TAG_SSHT);
		iff.insertForm(TAG_0001);

			if (!effectNode->Save(iff, false))
				return false;

			if (!materialTreeNode->Save(iff))
				return false;

			if (!textureTreeNode->Save(iff))
				return false;

			if (!textureCoordinateSetTreeNode->Save(iff))
				return false;

			if (!textureFactorTreeNode->Save(iff))
				return false;

			if (!textureScrollTreeNode->Save(iff))
				return false;

			if (!alphaReferenceValueTreeNode->Save(iff))
				return false;

			if (!stencilReferenceValueTreeNode->Save(iff))
				return false;

		iff.exitForm(TAG_0001);
	iff.exitForm(TAG_SSHT);

	return true;
}

// ----------------------------------------------------------------------

bool CTemplateTreeNode::Validate()
{
	if (!materialTreeNode->Validate())
		return false;

	if (!textureTreeNode->Validate())
		return false;

	if (!textureCoordinateSetTreeNode->Validate())
		return false;

	if (!textureFactorTreeNode->Validate())
		return false;

	if (!alphaReferenceValueTreeNode->Validate())
		return false;

	if (!stencilReferenceValueTreeNode->Validate())
		return false;

	if (!effectNode->Validate())
		return false;

	return true;
}

// ----------------------------------------------------------------------

CMaterialTreeNode &CTemplateTreeNode::GetMaterialTree()
{
	return *materialTreeNode;
}

// ----------------------------------------------------------------------

CTextureTreeNode &CTemplateTreeNode::GetTextureTree()
{
	return *textureTreeNode;
}

// ----------------------------------------------------------------------

CTextureCoordinateSetTreeNode &CTemplateTreeNode::GetTextureCoordinateSetTree()
{
	return *textureCoordinateSetTreeNode;
}

// ----------------------------------------------------------------------

CTextureFactorTreeNode &CTemplateTreeNode::GetTextureFactorTree()
{
	return *textureFactorTreeNode;
}

// ----------------------------------------------------------------------

CTextureScrollTreeNode &CTemplateTreeNode::GetTextureScrollTree()
{
	return *textureScrollTreeNode;
}

// ----------------------------------------------------------------------

CAlphaReferenceValueTreeNode &CTemplateTreeNode::GetAlphaReferenceValueTree()
{
	return *alphaReferenceValueTreeNode;
}

// ----------------------------------------------------------------------

CStencilReferenceValueTreeNode &CTemplateTreeNode::GetStencilReferenceValueTree()
{
	return *stencilReferenceValueTreeNode;
}

// ----------------------------------------------------------------------

CEffectNode &CTemplateTreeNode::GetEffect()
{
	return *effectNode;
}

// ----------------------------------------------------------------------

CIncludeTreeNode &CTemplateTreeNode::GetIncludeTree()
{
	return *includeTreeNode;
}

// ----------------------------------------------------------------------

CVertexShaderProgramTreeNode &CTemplateTreeNode::GetVertexShaderProgramTree()
{
	return *vertexShaderProgramTreeNode;
}

// ----------------------------------------------------------------------

CPixelShaderProgramTreeNode &CTemplateTreeNode::GetPixelShaderProgramTree()
{
	return *pixelShaderProgramTreeNode;
}

// ----------------------------------------------------------------------

const CMaterialTreeNode &CTemplateTreeNode::GetMaterialTree() const
{
	return *materialTreeNode;
}

// ----------------------------------------------------------------------

const CTextureTreeNode &CTemplateTreeNode::GetTextureTree() const
{
	return *textureTreeNode;
}

// ----------------------------------------------------------------------

const CTextureCoordinateSetTreeNode &CTemplateTreeNode::GetTextureCoordinateSetTree() const
{
	return *textureCoordinateSetTreeNode;
}

// ----------------------------------------------------------------------

const CTextureFactorTreeNode &CTemplateTreeNode::GetTextureFactorTree() const
{
	return *textureFactorTreeNode;
}

// ----------------------------------------------------------------------

const CAlphaReferenceValueTreeNode &CTemplateTreeNode::GetAlphaReferenceValueTree() const
{
	return *alphaReferenceValueTreeNode;
}

// ----------------------------------------------------------------------

const CStencilReferenceValueTreeNode &CTemplateTreeNode::GetStencilReferenceValueTree() const
{
	return *stencilReferenceValueTreeNode;
}

// ----------------------------------------------------------------------

const CEffectNode &CTemplateTreeNode::GetEffect() const
{
	return *effectNode;
}

// ======================================================================

template <class T>
CTaggedTreeNode<T>::CTaggedTreeNode(Type newType)
: CNode(newType)
{
}

// ----------------------------------------------------------------------

template <class T>
CTaggedTreeNode<T>::~CTaggedTreeNode()
{
}

// ======================================================================

CMaterialTreeNode::CMaterialTreeNode()
: CTaggedTreeNode<CMaterialNode>(MaterialTree) // , Material, TAG_MATS)
{
}

// ----------------------------------------------------------------------

CMaterialTreeNode::~CMaterialTreeNode()
{
}

// ----------------------------------------------------------------------

CMaterialNode *CMaterialTreeNode::GetMaterial(const CString &name)
{
	return static_cast<CMaterialNode *>(FindNamedChild(name));
}

// ----------------------------------------------------------------------

bool CMaterialTreeNode::Load(Iff &iff)
{
	if (iff.enterForm(TAG_MATS, true))
	{
		iff.enterForm(TAG_0000);

			while (!iff.atEndOfForm())
			{
				CString name;
				iff.enterChunk(TAG_TAG);
					IffReadTag(iff, name);
				iff.exitChunk(TAG_TAG);

				CMaterialNode *node = GetMaterial(name);
				if (!node)
				{
					node = new CMaterialNode(name);
					InsertItem(name, *node);
				}

				if (!node->Load(iff))
					return false;
			}

		iff.exitForm(TAG_0000);
		iff.exitForm(TAG_MATS);
	}

	return true;
}

// ----------------------------------------------------------------------

bool CMaterialTreeNode::Save(Iff &iff) const
{
	if (GetFirstChild(Material))
	{
		iff.insertForm(TAG_MATS);
			iff.insertForm(TAG_0000);

				for (const CNode *node = GetFirstChild(Material); node; node = GetNextChild(Material, *node))
				{
					CString name;
					node->GetName(name);

					iff.insertChunk(TAG_TAG);
						iff.insertChunkData(convertToTag(name));
					iff.exitChunk(TAG_TAG);

					if (!static_cast<const ChildNode *>(node)->Save(iff))
						return false;
				}

			iff.exitForm(TAG_0000);
		iff.exitForm(TAG_MATS);
	}

	return true;
}

// ----------------------------------------------------------------------

bool CMaterialTreeNode::Validate()
{
	return true;
}

// ======================================================================

CTextureTreeNode::CTextureTreeNode()
: CTaggedTreeNode<CTextureNode>(TextureTree)
{
}

// ----------------------------------------------------------------------

CTextureTreeNode::~CTextureTreeNode()
{
}

// ----------------------------------------------------------------------

CTextureNode *CTextureTreeNode::GetTexture(const CString &name)
{
	return static_cast<CTextureNode *>(FindNamedChild(name));
}

// ----------------------------------------------------------------------

bool CTextureTreeNode::Load(Iff &iff)
{
	if (iff.enterForm(TAG_TXMS, true))
	{
		while (!iff.atEndOfForm())
		{
			CString name;
			CTextureNode *node = new CTextureNode(name);
			if (!node->Load(iff, name))
				return false;

			CTextureNode *existingNode = GetTexture(name);
			if (existingNode)
			{
				node->referenceCount = existingNode->referenceCount;
				RemoveItem(*existingNode);
				delete existingNode;
			}

			InsertItem(name, *node);
		}

		iff.exitForm(TAG_TXMS);
	}

	return true;
}

// ----------------------------------------------------------------------

bool CTextureTreeNode::Save(Iff &iff) const
{
	if (GetFirstChild(Texture))
	{
		iff.insertForm(TAG_TXMS);

			for (const CNode *node = GetFirstChild(Texture); node; node = GetNextChild(Texture, *node))
			{
				CString name;
				node->GetName(name);

				if (!static_cast<const CTextureNode *>(node)->Save(name, iff))
					return false;
			}

		iff.exitForm(TAG_TXMS);
	}

	return true;
}

// ----------------------------------------------------------------------

bool CTextureTreeNode::Validate()
{
	for (CNode *node = GetFirstChild(Texture); node; node = GetNextChild(Texture, *node))
	{
		CString name;
		node->GetName(name);

		if (!static_cast<CTextureNode *>(node)->Validate())
			return false;
	}
	return true;
}

// ======================================================================

CTextureCoordinateSetTreeNode::CTextureCoordinateSetTreeNode()
: CTaggedTreeNode<CTextureCoordinateSetNode>(TextureCoordinateSetTree)
{
}

// ----------------------------------------------------------------------

CTextureCoordinateSetTreeNode::~CTextureCoordinateSetTreeNode()
{
}

// ----------------------------------------------------------------------

CTextureCoordinateSetNode *CTextureCoordinateSetTreeNode::GetTextureCoordinateSet(const CString &name)
{
	return static_cast<CTextureCoordinateSetNode *>(FindNamedChild(name));
}

// ----------------------------------------------------------------------

bool CTextureCoordinateSetTreeNode::Load(Iff &iff)
{
	if (iff.enterForm(TAG_TCSS, true))
	{
		iff.enterChunk(TAG_0000);

			while (iff.getChunkLengthLeft())
			{
				CString name;
				IffReadTag(iff, name);

				CTextureCoordinateSetNode *node = GetTextureCoordinateSet(name);
				if (!node)
				{
					node = new CTextureCoordinateSetNode(name);
					InsertItem(name, *node);
				}

				if (!node->Load(iff))
					return false;
			}

		iff.exitChunk(TAG_0000);
		iff.exitForm(TAG_TCSS);
	}

	return true;
}

// ----------------------------------------------------------------------

bool CTextureCoordinateSetTreeNode::Save(Iff &iff) const
{
	if (GetFirstChild(TextureCoordinateSet))
	{
		iff.insertForm(TAG_TCSS);
			iff.insertChunk(TAG_0000);

				for (const CNode *node = GetFirstChild(TextureCoordinateSet); node; node = GetNextChild(TextureCoordinateSet, *node))
				{
					CString name;
					node->GetName(name);
					iff.insertChunkData(convertToTag(name));

					if (!static_cast<const CTextureCoordinateSetNode *>(node)->Save(iff))
						return false;
				}

			iff.exitChunk(TAG_0000);
		iff.exitForm(TAG_TCSS);
	}

	return true;
}

// ----------------------------------------------------------------------

bool CTextureCoordinateSetTreeNode::Validate()
{
	for (CNode *node = GetFirstChild(TextureCoordinateSet); node; node = GetNextChild(TextureCoordinateSet, *node))
	{
		CString name;
		node->GetName(name);

		if (!static_cast<CTextureCoordinateSetNode *>(node)->Validate())
			return false;
	}
	return true;
}

// ======================================================================

CTextureFactorTreeNode::CTextureFactorTreeNode()
: CTaggedTreeNode<CTextureFactorNode>(TextureFactorTree)
{
}

// ----------------------------------------------------------------------

CTextureFactorTreeNode::~CTextureFactorTreeNode()
{
}

// ----------------------------------------------------------------------

CTextureFactorNode *CTextureFactorTreeNode::GetTextureFactor(const CString &name)
{
	return static_cast<CTextureFactorNode *>(FindNamedChild(name));
}

// ----------------------------------------------------------------------

bool CTextureFactorTreeNode::Load(Iff &iff)
{
	if (iff.enterForm(TAG_TFNS, true))
	{
		iff.enterChunk(TAG_0000);

			while (iff.getChunkLengthLeft())
			{
				CString name;
				IffReadTag(iff, name);

				CTextureFactorNode *node = GetTextureFactor(name);
				if (!node)
				{
					node = new CTextureFactorNode(name);
					InsertItem(name, *node);
				}

				if (!node->Load(iff))
					return false;
			}

		iff.exitChunk(TAG_0000);
		iff.exitForm(TAG_TFNS);
	}

	return true;
}

// ----------------------------------------------------------------------

bool CTextureFactorTreeNode::Save(Iff &iff) const
{
	if (GetFirstChild(TextureFactor))
	{
		iff.insertForm(TAG_TFNS);
			iff.insertChunk(TAG_0000);

				for (const CNode *node = GetFirstChild(TextureFactor); node; node = GetNextChild(TextureFactor, *node))
				{
					CString name;
					node->GetName(name);
					iff.insertChunkData(convertToTag(name));

					if (!static_cast<const CTextureFactorNode *>(node)->Save(iff))
						return false;
				}

			iff.exitChunk(TAG_0000);
		iff.exitForm(TAG_TFNS);
	}

	return true;
}

// ----------------------------------------------------------------------

bool CTextureFactorTreeNode::Validate()
{
	for (CNode *node = GetFirstChild(TextureFactor); node; node = GetNextChild(TextureFactor, *node))
	{
		CString name;
		node->GetName(name);

		if (!static_cast<CTextureFactorNode *>(node)->Validate())
			return false;
	}
	return true;
}

// ======================================================================

CTextureScrollTreeNode::CTextureScrollTreeNode()
: CTaggedTreeNode<CTextureScrollNode>(TextureScrollTree)
{
}

// ----------------------------------------------------------------------

CTextureScrollTreeNode::~CTextureScrollTreeNode()
{
}

// ----------------------------------------------------------------------

CTextureScrollNode *CTextureScrollTreeNode::GetTextureScroll(const CString &name)
{
	return static_cast<CTextureScrollNode *>(FindNamedChild(name));
}

// ----------------------------------------------------------------------

bool CTextureScrollTreeNode::Load(Iff &iff)
{
	if (iff.enterForm(TAG_TSNS, true))
	{
		iff.enterChunk(TAG_0000);

			while (iff.getChunkLengthLeft())
			{
				CString name;
				IffReadTag(iff, name);

				CTextureScrollNode * node = GetTextureScroll(name);
				if (!node)
				{
					node = new CTextureScrollNode(name);
					InsertItem(name, *node);
				}

				if (!node->Load(iff))
					return false;
			}

		iff.exitChunk(TAG_0000);
		iff.exitForm(TAG_TSNS);
	}

	return true;
}

// ----------------------------------------------------------------------

bool CTextureScrollTreeNode::Save(Iff &iff) const
{
	if (GetFirstChild(TextureScroll))
	{
		iff.insertForm(TAG_TSNS);
			iff.insertChunk(TAG_0000);

				for (const CNode *node = GetFirstChild(TextureScroll); node; node = GetNextChild(TextureScroll, *node))
				{
					CString name;
					node->GetName(name);
					iff.insertChunkData(convertToTag(name));

					if (!static_cast<const CTextureScrollNode *>(node)->Save(iff))
						return false;
				}

			iff.exitChunk(TAG_0000);
		iff.exitForm(TAG_TSNS);
	}

	return true;
}

// ----------------------------------------------------------------------

bool CTextureScrollTreeNode::Validate()
{
	for (CNode *node = GetFirstChild(TextureScroll); node; node = GetNextChild(TextureScroll, *node))
	{
		CString name;
		node->GetName(name);

		if (!static_cast<CTextureScrollNode *>(node)->Validate())
			return false;
	}
	return true;
}

// ======================================================================

CAlphaReferenceValueTreeNode::CAlphaReferenceValueTreeNode()
: CTaggedTreeNode<CAlphaReferenceValueNode>(AlphaReferenceValueTree) // , AlphaReferenceValue, TAG_ARVS)
{
}

// ----------------------------------------------------------------------

CAlphaReferenceValueTreeNode::~CAlphaReferenceValueTreeNode()
{
}

// ----------------------------------------------------------------------

CAlphaReferenceValueNode *CAlphaReferenceValueTreeNode::GetAlphaReferenceValue(const CString &name)
{
	return static_cast<CAlphaReferenceValueNode *>(FindNamedChild(name));
}

// ----------------------------------------------------------------------

bool CAlphaReferenceValueTreeNode::Load(Iff &iff)
{
	if (iff.enterForm(TAG_ARVS, true))
	{
		iff.enterChunk(TAG_0000);

			while (iff.getChunkLengthLeft())
			{
				CString name;
				IffReadTag(iff, name);

				CAlphaReferenceValueNode *node = GetAlphaReferenceValue(name);
				if (!node)
				{
					node = new CAlphaReferenceValueNode(name);
					InsertItem(name, *node);
				}

				if (!node->Load(iff))
					return false;
			}

		iff.exitChunk(TAG_0000);
		iff.exitForm(TAG_ARVS);
	}

	return true;
}

// ----------------------------------------------------------------------

bool CAlphaReferenceValueTreeNode::Save(Iff &iff) const
{
	if (GetFirstChild(AlphaReferenceValue))
	{
		iff.insertForm(TAG_ARVS);
			iff.insertChunk(TAG_0000);

				for (const CNode *node = GetFirstChild(AlphaReferenceValue); node; node = GetNextChild(AlphaReferenceValue, *node))
				{
					CString name;
					node->GetName(name);
					iff.insertChunkData(convertToTag(name));

					if (!static_cast<const CAlphaReferenceValueNode *>(node)->Save(iff))
						return false;
				}

			iff.exitChunk(TAG_0000);
		iff.exitForm(TAG_ARVS);
	}

	return true;
}

// ----------------------------------------------------------------------

bool CAlphaReferenceValueTreeNode::Validate()
{
	for (CNode *node = GetFirstChild(AlphaReferenceValue); node; node = GetNextChild(AlphaReferenceValue, *node))
	{
		CString name;
		node->GetName(name);

		if (!static_cast<CAlphaReferenceValueNode *>(node)->Validate())
			return false;
	}
	return true;
}

// ======================================================================

CStencilReferenceValueTreeNode::CStencilReferenceValueTreeNode()
: CTaggedTreeNode<CStencilReferenceValueNode>(StencilReferenceValueTree) // , StencilReferenceValue, TAG_SRVS)
{
}

// ----------------------------------------------------------------------

CStencilReferenceValueTreeNode::~CStencilReferenceValueTreeNode()
{
}

// ----------------------------------------------------------------------

CStencilReferenceValueNode *CStencilReferenceValueTreeNode::GetStencilReferenceValue(const CString &name)
{
	return static_cast<CStencilReferenceValueNode *>(FindNamedChild(name));
}

// ----------------------------------------------------------------------

bool CStencilReferenceValueTreeNode::Load(Iff &iff)
{
	if (iff.enterForm(TAG_SRVS, true))
	{
		iff.enterChunk(TAG_0000);

			while (iff.getChunkLengthLeft())
			{
				CString name;
				IffReadTag(iff, name);

				CStencilReferenceValueNode *node = GetStencilReferenceValue(name);
				if (!node)
				{
					node = new CStencilReferenceValueNode(name);
					InsertItem(name, *node);
				}

				if (!node->Load(iff))
					return false;
			}

		iff.exitChunk(TAG_0000);
		iff.exitForm(TAG_SRVS);
	}

	return true;
}

// ----------------------------------------------------------------------

bool CStencilReferenceValueTreeNode::Save(Iff &iff) const
{
	if (GetFirstChild(StencilReferenceValue))
	{
		iff.insertForm(TAG_SRVS);
			iff.insertChunk(TAG_0000);

				for (const CNode *node = GetFirstChild(StencilReferenceValue); node; node = GetNextChild(StencilReferenceValue, *node))
				{
					CString name;
					node->GetName(name);
					iff.insertChunkData(convertToTag(name));

					if (!static_cast<const CStencilReferenceValueNode *>(node)->Save(iff))
						return false;
				}

			iff.exitChunk(TAG_0000);
		iff.exitForm(TAG_SRVS);
	}

	return true;
}

// ----------------------------------------------------------------------

bool CStencilReferenceValueTreeNode::Validate()
{
	for (CNode *node = GetFirstChild(StencilReferenceValue); node; node = GetNextChild(StencilReferenceValue, *node))
	{
		CString name;
		node->GetName(name);

		if (!static_cast<CStencilReferenceValueNode *>(node)->Validate())
			return false;
	}
	return true;
}

// ======================================================================

CEffectNode::CEffectNode()
: CNode(Effect),
	m_isLocal(true),
	m_fileName(""),
	m_containsPrecalculatedVertexLighting(false)
{
}

// ----------------------------------------------------------------------

CEffectNode::~CEffectNode()
{
}

// ----------------------------------------------------------------------

bool CEffectNode::IsLocal() const
{
	return m_isLocal;
}

// ----------------------------------------------------------------------

void CEffectNode::Localize()
{
	m_isLocal = true;
}

// ----------------------------------------------------------------------

bool CEffectNode::IsLocked(Whom) const
{
	return !IsLocal();
}

// ----------------------------------------------------------------------

const CString &CEffectNode::GetFileName() const
{
	return m_fileName;
}

// ----------------------------------------------------------------------

void CEffectNode::SetFileName(const CString &fileName)
{
	m_fileName = fileName;
	m_isLocal = false;
}

// ----------------------------------------------------------------------

void CEffectNode::Reset()
{
	m_fileName.Empty();
	m_isLocal = true;
	while (GetFirstChild(Implementation))
		static_cast<CImplementationNode *>(GetFirstChild(Implementation))->Delete();
}

// ----------------------------------------------------------------------

bool CEffectNode::Load(Iff &iff)
{
	if (iff.enterChunk(TAG_NAME, true))
	{
		m_isLocal = false;

		// get the effect name
		char buffer[MAX_PATH];
		iff.read_string(buffer, sizeof(buffer));
		iff.exitChunk(TAG_NAME);

		// try to load the named effect
		CString namedIff = buffer;

		Iff newIff;
		if (!newIff.open(static_cast<LPCSTR>(namedIff), true))
		{
			char *b = strrchr(buffer, '\\');
			if (b)
			{
				char temp[256];
				strcpy(temp, "effect\\");
				strcat(temp, b+1);
				namedIff = temp;
			}

			if (!newIff.open(namedIff, true))
			{
				char buffer[256];
				sprintf(buffer, "Could not find effect file '%s'", namedIff);
				MessageBox(NULL, buffer, "Error", MB_ICONERROR | MB_OK);
				return false;
			}
		}
		if (!Load(newIff))
			return false;

		// set the file name and continue on
		SetFileName(buffer);
		return true;
	}
	else
		m_isLocal = true;

	iff.enterForm(TAG_EFCT);

		switch (iff.getCurrentName())
		{
			case TAG_0000:
				if (!Load_0000(iff))
					return false;
				break;

			case TAG_0001:
				if (!Load_0001(iff))
					return false;
				break;

			default:
				return false;
		}

		iff.exitForm(TAG_EFCT);
	return true;
}

// ----------------------------------------------------------------------

bool CEffectNode::Load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);

		iff.enterChunk(TAG_DATA);
			const int numberOfImplementations = iff.read_int8();
			m_containsPrecalculatedVertexLighting = false;
		iff.exitChunk(TAG_DATA);

		// load all the implementations
		for (int i = 0; i < numberOfImplementations; ++i)
		{
			CImplementationNode *implementationNode = new CImplementationNode(*this);
			if (!implementationNode->Load(iff))
				return false;
		}

	iff.exitForm(TAG_0000);

	return true;
}

// ----------------------------------------------------------------------

bool CEffectNode::Load_0001(Iff &iff)
{
	iff.enterForm(TAG_0001);

		iff.enterChunk(TAG_DATA);
			const int numberOfImplementations = iff.read_int8();
			m_containsPrecalculatedVertexLighting = iff.read_bool8();
		iff.exitChunk(TAG_DATA);

		// load all the implementations
		for (int i = 0; i < numberOfImplementations; ++i)
		{
			CImplementationNode *implementationNode = new CImplementationNode(*this);
			if (!implementationNode->Load(iff))
				return false;
		}

	iff.exitForm(TAG_0001);

	return true;
}

// ----------------------------------------------------------------------

bool CEffectNode::Save(Iff &iff, bool direct) const
{
	if (!direct && !m_isLocal)
	{
		iff.insertChunk(TAG_NAME);
			iff.insertChunkString(m_fileName);
		iff.exitChunk(TAG_NAME);
	}
	else
	{
		iff.insertForm(TAG_EFCT);
			iff.insertForm(TAG_0001);

				iff.insertChunk(TAG_DATA);
					iff.insertChunkData(static_cast<int8>(children.size()));
					iff.insertChunkData(static_cast<int8>(m_containsPrecalculatedVertexLighting ? 1 : 0));
				iff.exitChunk(TAG_DATA);

				// write out all the implementations
				int count = 0;
				for (const CImplementationNode *implementation = GetFirstImplementation(); implementation; implementation = GetNextImplementation(*implementation), ++count)
					if (!implementation->Save(iff, false))
						return false;

				ASSERT(count == static_cast<int>(children.size()));

			iff.exitForm(TAG_0001);
		iff.exitForm(TAG_EFCT);
	}

	return true;
}

// ----------------------------------------------------------------------

bool CEffectNode::Validate()
{
	int count = 0;
	for (CImplementationNode *implementation = const_cast<CImplementationNode*>(GetFirstImplementation()); implementation; implementation = const_cast<CImplementationNode*>(GetNextImplementation(*implementation)), ++count)
		if (!implementation->Validate())
			return false;
	return true;
}

// ----------------------------------------------------------------------

void CEffectNode::Lint()
{
	for (CImplementationNode *implementation = const_cast<CImplementationNode*>(GetFirstImplementation()); implementation; implementation = const_cast<CImplementationNode*>(GetNextImplementation(*implementation)))
		implementation->Lint();
}

// ----------------------------------------------------------------------

CImplementationNode &CEffectNode::AddLocalImplementation()
{
	return *(new CImplementationNode(*this));
}

// ----------------------------------------------------------------------

void CEffectNode::AddLocalImplementation(const CImplementationNode &implementation, HTREEITEM after)
{
	new CImplementationNode(*this, implementation, after);
}

// ----------------------------------------------------------------------

void CEffectNode::RemoveImplementation(CImplementationNode &implementation)
{
	RemoveItem(implementation);
}

// ----------------------------------------------------------------------

const CImplementationNode *CEffectNode::GetFirstImplementation() const
{
	return static_cast<const CImplementationNode *>(GetFirstChild(Implementation));
}

// ----------------------------------------------------------------------

const CImplementationNode *CEffectNode::GetNextImplementation(const CImplementationNode &ImplementationNode) const
{
	return static_cast<const CImplementationNode *>(GetNextChild(Implementation, ImplementationNode));
}

// ======================================================================

int CImplementationNode::remapOldInconsistentShaderCapabilityLevels(int oldLevel)
{
	if (oldLevel == ShaderCapability(1, 0, true)) return ShaderCapability(0, 2);
	if (oldLevel == ShaderCapability(1, 5, true)) return ShaderCapability(0, 3);
	if (oldLevel == ShaderCapability(2, 0))       return ShaderCapability(1, 1);
	if (oldLevel == ShaderCapability(2, 5, true)) return ShaderCapability(1, 4);
	if (oldLevel == ShaderCapability(3, 0, true)) return ShaderCapability(2, 0);
	WARNING(true, ("Could not remap old shader capability %d.%d", GetShaderCapabilityMajor(oldLevel), GetShaderCapabilityMinor(oldLevel)));
	return ShaderCapability(0,0);
}

// ----------------------------------------------------------------------

int CImplementationNode::remapRecentInconsistentShaderCapabilityLevels(int oldLevel)
{
	if (oldLevel == ShaderCapability(0, 2))       return ShaderCapability(0, 2);
	if (oldLevel == ShaderCapability(0, 3))       return ShaderCapability(0, 3);
	if (oldLevel == ShaderCapability(1, 0, true)) return ShaderCapability(1, 1);
	if (oldLevel == ShaderCapability(1, 4))       return ShaderCapability(1, 4);
	if (oldLevel == ShaderCapability(2, 0))       return ShaderCapability(2, 0);
	WARNING(true, ("Could not remap recent shader capability %d.%d", GetShaderCapabilityMajor(oldLevel), GetShaderCapabilityMinor(oldLevel)));
	return ShaderCapability(0,0);
}

// ======================================================================

CImplementationNode::CImplementationNode(CEffectNode &newEffect)
: CNode(Implementation),
	m_effect(newEffect),
	m_phaseTag(""),
	m_shaderCapabilityCompatibility(),
	m_optionTags(),
	m_fileName(""),
	m_castsShadows(true),
	m_collidable(true)
{
	m_effect.InsertItem("Implementation", *this);
}

// ----------------------------------------------------------------------

CImplementationNode::CImplementationNode(CEffectNode &newEffect, const CImplementationNode &copyDataFrom, HTREEITEM after)
: CNode(Implementation),
	m_effect(newEffect),
	m_phaseTag(copyDataFrom.m_phaseTag),
	m_shaderCapabilityCompatibility(copyDataFrom.m_shaderCapabilityCompatibility),
	m_optionTags(copyDataFrom.m_optionTags),
	m_fileName(copyDataFrom.m_fileName),
	m_castsShadows(copyDataFrom.m_castsShadows),
	m_collidable(copyDataFrom.m_collidable)
{
	m_effect.InsertItem("Implementation", *this, after);

	for (const CPassNode *pass = copyDataFrom.GetFirstPass(); pass; pass = copyDataFrom.GetNextPass(*pass))
		AddLocalPass(*pass, TVI_LAST);

	// see if the old node was expanded
	TVITEM tvItem;
	memset(&tvItem, 0, sizeof(tvItem));
	tvItem.hItem = copyDataFrom.treeItem;
	treeCtrl->GetItem(&tvItem);
	if (tvItem.state & TVIS_EXPANDED)
		treeCtrl->Expand(treeItem, TVE_EXPAND);
}

// ----------------------------------------------------------------------

CImplementationNode::~CImplementationNode()
{
}

// ----------------------------------------------------------------------

bool CImplementationNode::IsLocked(Whom whom) const
{
	return (whom != Self && m_effect.IsLocked(Self)) || (whom != Parent && m_fileName.IsEmpty() == FALSE);
}

// ----------------------------------------------------------------------

const CString &CImplementationNode::GetFileName() const
{
	return m_fileName;
}

// ----------------------------------------------------------------------

void CImplementationNode::SetFileName(const CString &fileName)
{
	m_fileName = fileName;
}

// ----------------------------------------------------------------------

bool CImplementationNode::Load(Iff &iff)
{
	if (iff.enterChunk(TAG_NAME, true))
	{
		// get the effect name
		char buffer[MAX_PATH];
		iff.read_string(buffer, sizeof(buffer));
		iff.exitChunk(TAG_NAME);

		// try to load the named effect
		CString namedIff = buffer;
		Iff newIff;
		if (!newIff.open(static_cast<LPCSTR>(namedIff), true))
		{
			char buffer[256];
			sprintf(buffer, "Could not find implementation file '%s'", namedIff);
			MessageBox(NULL, buffer, "Error", MB_ICONERROR | MB_OK);
			return false;
		}
		if (!Load(newIff))
			return false;

		// set the file name and continue on
		SetFileName(buffer);
		return true;
	}

	iff.enterForm(TAG_IMPL);

		switch (iff.getCurrentName())
		{
			case TAG_0000:
				if (!Load_0000(iff))
					return false;
				break;

			case TAG_0001:
				if (!Load_0001(iff))
					return false;
				break;

			case TAG_0002:
				if (!Load_0002(iff))
					return false;
				break;

			case TAG_0003:
				if (!Load_0003(iff))
					return false;
				break;

			case TAG_0004:
				if (!Load_0004(iff))
					return false;
				break;

			case TAG_0005:
				if (!Load_0005(iff))
					return false;
				break;

			case TAG_0006:
				if (!Load_0006(iff))
					return false;
				break;

			case TAG_0007:
				if (!Load_0007(iff))
					return false;
				break;

			case TAG_0008:
				if (!Load_0008(iff))
					return false;
				break;

			case TAG_0009:
				if (!Load_0009(iff))
					return false;
				break;

			default:
				assert(false);
				break;
		}

	iff.exitForm(TAG_IMPL);

	return true;
}

// ----------------------------------------------------------------------

bool CImplementationNode::Load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);

		iff.enterChunk(TAG_DATA);
			const int numberOfPasses = iff.read_int8();
		iff.exitChunk(TAG_DATA);

		// load all the passes
		for (int i = 0; i < numberOfPasses; ++i)
		{
			CPassNode *passNode = new CPassNode(*this);
			if (!passNode->Load(iff))
				return false;
		}

	iff.exitForm(TAG_0000);

	return true;
}

// ----------------------------------------------------------------------

bool CImplementationNode::Load_0001(Iff &iff)
{
	iff.enterForm(TAG_0001);

		iff.enterChunk(TAG_DATA);
			const int numberOfPasses = iff.read_int8();
			IGNORE_RETURN(iff.read_bool8());
		iff.exitChunk(TAG_DATA);

		// load all the passes
		for (int i = 0; i < numberOfPasses; ++i)
		{
			CPassNode *passNode = new CPassNode(*this);
			if (!passNode->Load(iff))
				return false;
		}

	iff.exitForm(TAG_0001);

	return true;
}

// ----------------------------------------------------------------------

bool CImplementationNode::Load_0002(Iff &iff)
{
	iff.enterForm(TAG_0002);

		iff.enterChunk(TAG_DATA);
			const int numberOfPasses = iff.read_int8();
			IffReadTag(iff, m_phaseTag);
		iff.exitChunk(TAG_DATA);

		// load all the passes
		for (int i = 0; i < numberOfPasses; ++i)
		{
			CPassNode *passNode = new CPassNode(*this);
			if (!passNode->Load(iff))
				return false;
		}

	iff.exitForm(TAG_0002);

	return true;
}

// ----------------------------------------------------------------------

bool CImplementationNode::Load_0003(Iff &iff)
{
	iff.enterForm(TAG_0003);

		iff.enterChunk(TAG_DATA);
			const int numberOfPasses = iff.read_int8();
			IffReadTag(iff, m_phaseTag);
		iff.exitChunk(TAG_DATA);

		if (iff.enterChunk(TAG_OPTN, true))
		{
			const int numberOfOptions = iff.getChunkLengthTotal(sizeof(Tag));
			for (int i = 0; i < numberOfOptions; ++i)
			{
				CString optionTag;
				IffReadTag(iff, optionTag);
				m_optionTags.push_back(optionTag);
			}
			iff.exitChunk(TAG_OPTN);
		}

		// load all the passes
		for (int i = 0; i < numberOfPasses; ++i)
		{
			CPassNode *passNode = new CPassNode(*this);
			if (!passNode->Load(iff))
				return false;
		}

	iff.exitForm(TAG_0003);

	return true;
}

// ----------------------------------------------------------------------

bool CImplementationNode::Load_0004(Iff &iff)
{
	iff.enterForm(TAG_0004);

		iff.enterChunk(TAG_SCAP);
		{
			const int numberOfEntries = iff.getChunkLengthTotal(sizeof(Tag));
			for (int i = 0; i < numberOfEntries; ++i)
			{
				int const value = remapOldInconsistentShaderCapabilityLevels(iff.read_int32());
				char buffer[16];
				sprintf(buffer, "%d.%d", GetShaderCapabilityMajor(value), GetShaderCapabilityMinor(value));
				m_shaderCapabilityCompatibility.push_back(CString(buffer));
			}
		}
		iff.exitChunk(TAG_SCAP);

		if (iff.enterChunk(TAG_OPTN, true))
		{
			const int numberOfOptions = iff.getChunkLengthTotal(sizeof(Tag));
			for (int i = 0; i < numberOfOptions; ++i)
			{
				CString optionTag;
				IffReadTag(iff, optionTag);
				m_optionTags.push_back(optionTag);
			}
			iff.exitChunk(TAG_OPTN);
		}

		iff.enterChunk(TAG_DATA);
			const int numberOfPasses = iff.read_int8();
			IffReadTag(iff, m_phaseTag);
		iff.exitChunk(TAG_DATA);

		// load all the passes
		for (int i = 0; i < numberOfPasses; ++i)
		{
			CPassNode *passNode = new CPassNode(*this);
			if (!passNode->Load(iff))
				return false;
		}

	const CPassNode *passNode = GetFirstPass();
	if (passNode)
	{
		if (passNode->m_alphaTestEnable || passNode->m_alphaBlendEnable)
		{
			m_collidable = false;
			m_castsShadows = false;
		}
		else
		{
			m_collidable = true;
			m_castsShadows = true;
		}
	}
	else
	{
		m_collidable = false;
		m_castsShadows = false;
	}

	iff.exitForm(TAG_0004);

	return true;
}

// ----------------------------------------------------------------------

bool CImplementationNode::Load_0005(Iff &iff)
{
	iff.enterForm(TAG_0005);

		iff.enterChunk(TAG_SCAP);
		{
			const int numberOfEntries = iff.getChunkLengthTotal(sizeof(Tag));
			for (int i = 0; i < numberOfEntries; ++i)
			{
				int const value = remapOldInconsistentShaderCapabilityLevels(iff.read_int32());
				char buffer[16];
				sprintf(buffer, "%d.%d", GetShaderCapabilityMajor(value), GetShaderCapabilityMinor(value));
				m_shaderCapabilityCompatibility.push_back(CString(buffer));
			}
		}
		iff.exitChunk(TAG_SCAP);

		if (iff.enterChunk(TAG_OPTN, true))
		{
			const int numberOfOptions = iff.getChunkLengthTotal(sizeof(Tag));
			for (int i = 0; i < numberOfOptions; ++i)
			{
				CString optionTag;
				IffReadTag(iff, optionTag);
				m_optionTags.push_back(optionTag);
			}
			iff.exitChunk(TAG_OPTN);
		}

		iff.enterChunk(TAG_DATA);
			const int numberOfPasses = iff.read_int8();
			IffReadTag(iff, m_phaseTag);
			IffReadBool(iff, m_castsShadows);
			IffReadBool(iff, m_collidable);
		iff.exitChunk(TAG_DATA);

		// load all the passes
		for (int i = 0; i < numberOfPasses; ++i)
		{
			CPassNode *passNode = new CPassNode(*this);
			if (!passNode->Load(iff))
				return false;
		}

	iff.exitForm(TAG_0005);

	return true;
}

// ----------------------------------------------------------------------

bool CImplementationNode::Load_0006(Iff &iff)
{
	iff.enterForm(TAG_0006);

		iff.enterChunk(TAG_SCAP);
		{
			const int numberOfEntries = iff.getChunkLengthTotal(sizeof(Tag));
			for (int i = 0; i < numberOfEntries; ++i)
			{
				int const value = remapRecentInconsistentShaderCapabilityLevels(iff.read_int32());
				char buffer[16];
				sprintf(buffer, "%d.%d", GetShaderCapabilityMajor(value), GetShaderCapabilityMinor(value));
				m_shaderCapabilityCompatibility.push_back(CString(buffer));
			}
		}
		iff.exitChunk(TAG_SCAP);

		if (iff.enterChunk(TAG_OPTN, true))
		{
			const int numberOfOptions = iff.getChunkLengthTotal(sizeof(Tag));
			for (int i = 0; i < numberOfOptions; ++i)
			{
				CString optionTag;
				IffReadTag(iff, optionTag);
				m_optionTags.push_back(optionTag);
			}
			iff.exitChunk(TAG_OPTN);
		}

		iff.enterChunk(TAG_DATA);
			const int numberOfPasses = iff.read_int8();
			IffReadTag(iff, m_phaseTag);
			IffReadBool(iff, m_castsShadows);
			IffReadBool(iff, m_collidable);
		iff.exitChunk(TAG_DATA);

		// load all the passes
		for (int i = 0; i < numberOfPasses; ++i)
		{
			CPassNode *passNode = new CPassNode(*this);
			if (!passNode->Load(iff))
				return false;
		}

	iff.exitForm(TAG_0006);

	return true;
}

// ----------------------------------------------------------------------

bool CImplementationNode::Load_0007(Iff &iff)
{
	iff.enterForm(TAG_0007);

		iff.enterChunk(TAG_SCAP);
		{
			const int numberOfEntries = iff.getChunkLengthTotal(sizeof(Tag));
			for (int i = 0; i < numberOfEntries; ++i)
			{
				int const value = iff.read_int32();
				char buffer[16];
				sprintf(buffer, "%d.%d", GetShaderCapabilityMajor(value), GetShaderCapabilityMinor(value));
				m_shaderCapabilityCompatibility.push_back(CString(buffer));
			}
		}
		iff.exitChunk(TAG_SCAP);

		if (iff.enterChunk(TAG_OPTN, true))
		{
			const int numberOfOptions = iff.getChunkLengthTotal(sizeof(Tag));
			for (int i = 0; i < numberOfOptions; ++i)
			{
				CString optionTag;
				IffReadTag(iff, optionTag);
				m_optionTags.push_back(optionTag);
			}
			iff.exitChunk(TAG_OPTN);
		}

		iff.enterChunk(TAG_DATA);
			const int numberOfPasses = iff.read_int8();
			IffReadTag(iff, m_phaseTag);
			IffReadBool(iff, m_castsShadows);
			IffReadBool(iff, m_collidable);
		iff.exitChunk(TAG_DATA);

		// load all the passes
		for (int i = 0; i < numberOfPasses; ++i)
		{
			CPassNode *passNode = new CPassNode(*this);
			if (!passNode->Load(iff))
				return false;
		}

	iff.exitForm(TAG_0007);

	return true;
}

// ----------------------------------------------------------------------

bool CImplementationNode::Load_0008(Iff &iff)
{
	iff.enterForm(TAG_0008);

		iff.enterChunk(TAG_SCAP);
		{
			const int numberOfEntries = iff.getChunkLengthTotal(sizeof(Tag));
			for (int i = 0; i < numberOfEntries; ++i)
			{
				int const value = iff.read_int32();
				char buffer[16];
				sprintf(buffer, "%d.%d", GetShaderCapabilityMajor(value), GetShaderCapabilityMinor(value));
				m_shaderCapabilityCompatibility.push_back(CString(buffer));
			}
		}
		iff.exitChunk(TAG_SCAP);

		if (iff.enterChunk(TAG_OPTN, true))
		{
			const int numberOfOptions = iff.getChunkLengthTotal(sizeof(Tag));
			for (int i = 0; i < numberOfOptions; ++i)
			{
				CString optionTag;
				IffReadTag(iff, optionTag);
				m_optionTags.push_back(optionTag);
			}
			iff.exitChunk(TAG_OPTN);
		}

		iff.enterChunk(TAG_DATA);
			const int numberOfPasses = iff.read_int8();
			IffReadTag(iff, m_phaseTag);
			IffReadBool(iff, m_castsShadows);
			IffReadBool(iff, m_collidable);
			int oldHeatBool = false;
			IffReadBool(iff, oldHeatBool);
		iff.exitChunk(TAG_DATA);

		// load all the passes
		for (int i = 0; i < numberOfPasses; ++i)
		{
			CPassNode *passNode = new CPassNode(*this);
			if (!passNode->Load(iff))
				return false;
		}

	iff.exitForm(TAG_0008);

	return true;
}

//----------------------------------------------------------------------

bool CImplementationNode::Load_0009(Iff &iff)
{
	//-- back-reved a version to remove heat from implementation
	iff.enterForm(TAG_0009);

		iff.enterChunk(TAG_SCAP);
		{
			const int numberOfEntries = iff.getChunkLengthTotal(sizeof(Tag));
			for (int i = 0; i < numberOfEntries; ++i)
			{
				int const value = iff.read_int32();
				char buffer[16];
				sprintf(buffer, "%d.%d", GetShaderCapabilityMajor(value), GetShaderCapabilityMinor(value));
				m_shaderCapabilityCompatibility.push_back(CString(buffer));
			}
		}
		iff.exitChunk(TAG_SCAP);

		if (iff.enterChunk(TAG_OPTN, true))
		{
			const int numberOfOptions = iff.getChunkLengthTotal(sizeof(Tag));
			for (int i = 0; i < numberOfOptions; ++i)
			{
				CString optionTag;
				IffReadTag(iff, optionTag);
				m_optionTags.push_back(optionTag);
			}
			iff.exitChunk(TAG_OPTN);
		}

		iff.enterChunk(TAG_DATA);
			const int numberOfPasses = iff.read_int8();
			IffReadTag(iff, m_phaseTag);
			IffReadBool(iff, m_castsShadows);
			IffReadBool(iff, m_collidable);
		iff.exitChunk(TAG_DATA);

		// load all the passes
		for (int i = 0; i < numberOfPasses; ++i)
		{
			CPassNode *passNode = new CPassNode(*this);
			if (!passNode->Load(iff))
				return false;
		}

	iff.exitForm(TAG_0009);

	return true;
}

// ----------------------------------------------------------------------

bool CImplementationNode::Save(Iff &iff, bool direct) const
{
	if (!direct && m_fileName != "")
	{
		iff.insertChunk(TAG_NAME);
			iff.insertChunkString(m_fileName);
		iff.exitChunk(TAG_NAME);
	}
	else
	{
		iff.insertForm(TAG_IMPL);
			iff.insertForm(TAG_0009);

				{
					iff.insertChunk(TAG_SCAP);
						const int numberOfEntries = m_shaderCapabilityCompatibility.size();
						for (int i = 0; i < numberOfEntries; ++i)
						{
							CString const & value = m_shaderCapabilityCompatibility[i];
							int const dot = value.Find('.');
							int const major = (dot < 0) ? atoi(value) : atoi(value.Left(dot));
							int const minor = (dot < 0) ? 0           : atoi(value.Mid(dot + 1));
							iff.insertChunkData(static_cast<int32>(ShaderCapability(major, minor)));
						}
					iff.exitChunk(TAG_SCAP);
				}

				if (!m_optionTags.empty())
				{
					iff.insertChunk(TAG_OPTN);
						const int numberOfOptions = m_optionTags.size();
						for (int i = 0; i < numberOfOptions; ++i)
						{
							iff.insertChunkData(convertToTag(m_optionTags[i]));
						}
					iff.exitChunk(TAG_OPTN);
				}

				iff.insertChunk(TAG_DATA);
					iff.insertChunkData(static_cast<int8>(children.size()));
					iff.insertChunkData(convertToTag(m_phaseTag));
					iff.insertChunkData(static_cast<int8>(m_castsShadows ? 1 : 0));
					iff.insertChunkData(static_cast<int8>(m_collidable ? 1 : 0));
				iff.exitChunk(TAG_DATA);

				// write out all the passes
				int count = 0;
				for (const CPassNode *pass = GetFirstPass(); pass; pass = GetNextPass(*pass), ++count)
					if (!pass->Save(iff))
						return false;

				ASSERT(count == static_cast<int>(children.size()));

			iff.exitForm(TAG_0009);
		iff.exitForm(TAG_IMPL);
	}

	return true;
}

// ----------------------------------------------------------------------

bool CImplementationNode::Validate()
{
	if (m_phaseTag == "")
	{
		MessageBox(NULL, "Implementation node has no phase tag.", "Error", MB_OK | MB_ICONERROR);
		treeCtrl->Select(treeItem, TVGN_CARET);
		leftView->mainFrame->implementationView->m_phaseTag.SetFocus();
		return false;
	}

	if (m_shaderCapabilityCompatibility.empty())
	{
		MessageBox(NULL, "Implementation node has no shader capaibility levels set.", "Error", MB_OK | MB_ICONERROR);
		treeCtrl->Select(treeItem, TVGN_CARET);
		leftView->mainFrame->implementationView->m_shaderCapabilityCompatibility.SetFocus();
		return false;
	}

	const int numberOfEntries = m_shaderCapabilityCompatibility.size();
	for (int i = 0; i < numberOfEntries; ++i)
	{
		CString const & value = m_shaderCapabilityCompatibility[i];
		int const dot = value.Find('.');
		int const major = (dot < 0) ? atoi(value) : atoi(value.Left(dot));
		int const minor = (dot < 0) ? 0           : atoi(value.Mid(dot + 1));

		int const shaderCapability = ShaderCapability(major, minor);
		if (shaderCapability != ShaderCapability(0,2) && shaderCapability != ShaderCapability(0,3) && shaderCapability != ShaderCapability(1,1) && shaderCapability != ShaderCapability(1,4) && shaderCapability != ShaderCapability(2,0))
		{
			MessageBox(NULL, "Supported shader capabilities are: 0.2, 0.3,  1.1,  1.4,  2.0", "Error", MB_OK | MB_ICONERROR);
			treeCtrl->Select(treeItem, TVGN_CARET);
			leftView->mainFrame->implementationView->m_shaderCapabilityCompatibility.SetFocus();
			return false;
		}
	}

	int count = 0;
	for (CPassNode *pass = const_cast<CPassNode*>(GetFirstPass()); pass; pass = const_cast<CPassNode*>(GetNextPass(*pass)), ++count)
		if (!pass->Validate())
			return false;

	return true;
}

// ----------------------------------------------------------------------

void CImplementationNode::Lint()
{
	for (CPassNode *pass = const_cast<CPassNode*>(GetFirstPass()); pass; pass = const_cast<CPassNode*>(GetNextPass(*pass)))
		pass->Lint();
}

// ----------------------------------------------------------------------

void CImplementationNode::AddLocalPass()
{
	new CPassNode(*this);
}

// ----------------------------------------------------------------------

void CImplementationNode::AddLocalPass(const CPassNode &pass, HTREEITEM after)
{
	new CPassNode(*this, pass, after);
}

// ----------------------------------------------------------------------

void CImplementationNode::RemovePass(CPassNode &pass)
{
	RemoveItem(pass);
}

// ----------------------------------------------------------------------

void CImplementationNode::Delete()
{
	// delete all the stages
	for (Children::iterator i = children.begin(); i != children.end(); i = children.begin())
		static_cast<CPassNode *>(i->second)->Delete();

	m_effect.RemoveImplementation(*this);
	delete this;
}

// ----------------------------------------------------------------------

const CPassNode *CImplementationNode::GetFirstPass() const
{
	return static_cast<const CPassNode *>(GetFirstChild(Pass));
}

// ----------------------------------------------------------------------

const CPassNode *CImplementationNode::GetNextPass(const CPassNode &passNode) const
{
	return static_cast<const CPassNode *>(GetNextChild(Pass, passNode));
}

// ======================================================================

CPassNode::CPassNode(CImplementationNode &newImplementation)
: CNode(Pass),
	m_textureStageCount(0),
	m_pixelShaderCount(0),
	m_textureFactorTag(""),
	m_textureFactor2Tag(""),
	m_textureScrollTag(""),
	m_shadeMode("Gouraud"),
	m_ditherEnable(FALSE),
	m_heat(FALSE),
	m_stencilWriteMask("0"),
	m_stencilMask("0"),
	m_stencilReferenceTag(""),
	m_stencilCompareFunction("Greater"),
	m_stencilPassOperation("Keep"),
	m_stencilZFailOperation("Keep"),
	m_stencilFailOperation("Keep"),
	m_stencilCounterClockwiseCompareFunction("Greater"),
	m_stencilCounterClockwisePassOperation("Keep"),
	m_stencilCounterClockwiseZFailOperation("Keep"),
	m_stencilCounterClockwiseFailOperation("Keep"),
	m_stencilEnable(FALSE),
	m_stencilTwoSidedMode(FALSE),
	m_alphaTestFunction("Greater"),
	m_alphaTestTag(""),
	m_alphaTestEnable(FALSE),
	m_alphaBlendOperation("Add"),
	m_alphaBlendDestination("Inverse Source Alpha"),
	m_alphaBlendSource("Source Alpha"),
	m_alphaBlendEnable(FALSE),
	m_zCompare("Less or Equal"),
	m_zWrite(TRUE),
	m_zEnable(TRUE),
	m_writeMaskA(TRUE),
	m_writeMaskR(TRUE),
	m_writeMaskG(TRUE),
	m_writeMaskB(TRUE),
	m_fogMode("Normal"),
	m_materialTag(""),
	implementation(newImplementation)
{
	newImplementation.InsertItem("Pass", *this);
}

// ----------------------------------------------------------------------

CPassNode::CPassNode(CImplementationNode &newImplementation, const CPassNode &copyDataFrom, HTREEITEM after)
: CNode(Pass),
	m_textureStageCount(0),
	m_pixelShaderCount(0),
	m_textureFactorTag(copyDataFrom.m_textureFactorTag),
	m_textureFactor2Tag(copyDataFrom.m_textureFactor2Tag),
	m_textureScrollTag(copyDataFrom.m_textureScrollTag),
	m_shadeMode(copyDataFrom.m_shadeMode),
	m_ditherEnable(copyDataFrom.m_ditherEnable),
	m_heat(copyDataFrom.m_heat),
	m_stencilWriteMask(copyDataFrom.m_stencilWriteMask),
	m_stencilMask(copyDataFrom.m_stencilMask),
	m_stencilReferenceTag(copyDataFrom.m_stencilReferenceTag),
	m_stencilCompareFunction(copyDataFrom.m_stencilCompareFunction),
	m_stencilPassOperation(copyDataFrom.m_stencilPassOperation),
	m_stencilZFailOperation(copyDataFrom.m_stencilZFailOperation),
	m_stencilFailOperation(copyDataFrom.m_stencilFailOperation),
	m_stencilCounterClockwiseCompareFunction(copyDataFrom.m_stencilCounterClockwiseCompareFunction),
	m_stencilCounterClockwisePassOperation(copyDataFrom.m_stencilCounterClockwisePassOperation),
	m_stencilCounterClockwiseZFailOperation(copyDataFrom.m_stencilCounterClockwiseZFailOperation),
	m_stencilCounterClockwiseFailOperation(copyDataFrom.m_stencilCounterClockwiseFailOperation),
	m_stencilEnable(copyDataFrom.m_stencilEnable),
	m_stencilTwoSidedMode(copyDataFrom.m_stencilTwoSidedMode),
	m_alphaTestFunction(copyDataFrom.m_alphaTestFunction),
	m_alphaTestTag(copyDataFrom.m_alphaTestTag),
	m_alphaTestEnable(copyDataFrom.m_alphaTestEnable),
	m_alphaBlendOperation(copyDataFrom.m_alphaBlendOperation),
	m_alphaBlendDestination(copyDataFrom.m_alphaBlendDestination),
	m_alphaBlendSource(copyDataFrom.m_alphaBlendSource),
	m_alphaBlendEnable(copyDataFrom.m_alphaBlendEnable),
	m_zCompare(copyDataFrom.m_zCompare),
	m_zWrite(copyDataFrom.m_zWrite),
	m_zEnable(copyDataFrom.m_zEnable),
	m_writeMaskA(copyDataFrom.m_writeMaskA),
	m_writeMaskR(copyDataFrom.m_writeMaskR),
	m_writeMaskG(copyDataFrom.m_writeMaskG),
	m_writeMaskB(copyDataFrom.m_writeMaskB),
	m_fogMode(copyDataFrom.m_fogMode),
	m_materialTag(copyDataFrom.m_materialTag),
	implementation(newImplementation)
{
	CTemplateTreeNode::GetInstance()->GetTextureFactorTree().Add(m_textureFactorTag);
	CTemplateTreeNode::GetInstance()->GetTextureFactorTree().Add(m_textureFactor2Tag);
	CTemplateTreeNode::GetInstance()->GetTextureScrollTree().Add(m_textureScrollTag);
	CTemplateTreeNode::GetInstance()->GetAlphaReferenceValueTree().Add(m_alphaTestTag);
	CTemplateTreeNode::GetInstance()->GetStencilReferenceValueTree().Add(m_stencilReferenceTag);
	CTemplateTreeNode::GetInstance()->GetMaterialTree().Add(m_materialTag);

	newImplementation.InsertItem("Pass", *this, after);

	if (copyDataFrom.GetFixedFunctionPipeline())
		AddFixedFunctionPipeline(*copyDataFrom.GetFixedFunctionPipeline());
	else
		if (copyDataFrom.GetVertexShader())
			AddVertexShader(*copyDataFrom.GetVertexShader());

	if (copyDataFrom.GetPixelShader())
	{
		AddLocalPixelShader(*copyDataFrom.GetPixelShader());
	}
	else
	{
		for (const CStageNode *stage = copyDataFrom.GetFirstStage(); stage; stage = copyDataFrom.GetNextStage(*stage))
			AddLocalStage(*stage, TVI_LAST);
	}

	// see if the old node was expanded
	TVITEM tvItem;
	memset(&tvItem, 0, sizeof(tvItem));
	tvItem.hItem = copyDataFrom.treeItem;
	treeCtrl->GetItem(&tvItem);
	if (tvItem.state & TVIS_EXPANDED)
		treeCtrl->Expand(treeItem, TVE_EXPAND);
}

// ----------------------------------------------------------------------

CPassNode::~CPassNode()
{
}

// ----------------------------------------------------------------------

bool CPassNode::IsLocked(Whom) const
{
	return implementation.IsLocked(Either);
}

// ----------------------------------------------------------------------

bool CPassNode::Load(Iff &iff)
{
	iff.enterForm(TAG_PASS);

		switch (iff.getCurrentName())
		{
			case TAG_0000:
				if (!Load_0000(iff))
					return false;
				break;

			case TAG_0001:
				if (!Load_0001(iff))
					return false;
				break;

			case TAG_0002:
				if (!Load_0002(iff))
					return false;
				break;

			case TAG_0003:
				if (!Load_0003(iff))
					return false;
				break;

			case TAG_0004:
				if (!Load_0004(iff))
					return false;
				break;

			case TAG_0005:
				if (!Load_0005(iff))
					return false;
				break;

			case TAG_0006:
				if (!Load_0006(iff))
					return false;
				break;

			case TAG_0007:
				if (!Load_0007(iff))
					return false;
				break;

			case TAG_0008:
				if (!Load_0008(iff))
					return false;
				break;

			case TAG_0009:
				if (!Load_0009(iff))
					return false;
				break;

			case TAG_0010:
				if (!Load_0010(iff))
					return false;
				break;

			default:
				assert(false);
				break;
		}

	iff.exitForm(TAG_PASS);

	CTemplateTreeNode::GetInstance()->GetTextureFactorTree().Add(m_textureFactorTag);
	CTemplateTreeNode::GetInstance()->GetTextureFactorTree().Add(m_textureFactor2Tag);
	CTemplateTreeNode::GetInstance()->GetTextureScrollTree().Add(m_textureScrollTag);
	CTemplateTreeNode::GetInstance()->GetAlphaReferenceValueTree().Add(m_alphaTestTag);
	CTemplateTreeNode::GetInstance()->GetStencilReferenceValueTree().Add(m_stencilReferenceTag);
	CTemplateTreeNode::GetInstance()->GetMaterialTree().Add(m_materialTag);

	return true;
}

// ----------------------------------------------------------------------

bool CPassNode::Load_0000(Iff &iff)
{
	AddFixedFunctionPipeline();
	CFixedFunctionPipelineNode *ff = GetFixedFunctionPipeline();

	iff.enterForm(TAG_0000);
		iff.enterChunk(TAG_DATA);

			const int numberOfStages = iff.read_int8();

			IffReadMap (iff, m_shadeMode, ms_shadeMode);

			IffReadBool(iff, m_ditherEnable);

			IffReadBool(iff, m_zEnable);
			IffReadBool(iff, m_zWrite);
			IffReadMap (iff, m_zCompare, ms_compare);

			IffReadBool(iff, m_alphaBlendEnable);
			IffReadMap (iff, m_alphaBlendOperation, ms_blendOp);
			IffReadMap (iff, m_alphaBlendSource, ms_blend);
			IffReadMap (iff, m_alphaBlendDestination, ms_blend);

			IffReadBool(iff, m_alphaTestEnable);
			IffReadTag (iff, m_alphaTestTag);
			IffReadMap (iff, m_alphaTestFunction, ms_compare);

			byte mask = iff.read_uint8();
			m_writeMaskA = ((mask >> 3) & 1) != 0;
			m_writeMaskR = ((mask >> 2) & 1) != 0;
			m_writeMaskG = ((mask >> 1) & 1) != 0;
			m_writeMaskB = ((mask >> 0) & 1) != 0;

			IffReadTag (iff, m_textureFactorTag);

			IffReadBool(iff, ff->m_lighting);
			IffReadBool(iff, ff->m_lightingSpecularEnable);
			IffReadBool(iff, ff->m_lightingColorVertex);
			IffReadTag (iff, m_materialTag);
			IffReadMap (iff, ff->m_lightingAmbientColorSource, ms_materialSource);
			IffReadMap (iff, ff->m_lightingDiffuseColorSource, ms_materialSource);
			IffReadMap (iff, ff->m_lightingSpecularColorSource, ms_materialSource);
			IffReadMap (iff, ff->m_lightingEmissiveColorSource, ms_materialSource);

			IffReadBool(iff, m_stencilEnable);
			IffReadTag (iff, m_stencilReferenceTag);
			IffReadMap (iff, m_stencilCompareFunction, ms_compare);
			IffReadMap (iff, m_stencilPassOperation, ms_stencilOp);
			IffReadMap (iff, m_stencilZFailOperation, ms_stencilOp);
			IffReadMap (iff, m_stencilFailOperation, ms_stencilOp);
			IffReadInt (iff, m_stencilWriteMask);
			IffReadInt (iff, m_stencilMask);

		iff.exitChunk(TAG_DATA);

		for (int i = 0; i < numberOfStages; ++i)
		{
			CStageNode *stageNode = new CStageNode(*this);
			if (!stageNode->Load(iff))
				return false;
			++m_textureStageCount;
		}

	iff.exitForm(TAG_0000);
	return true;
}

// ----------------------------------------------------------------------

bool CPassNode::Load_0001(Iff &iff)
{
	AddFixedFunctionPipeline();
	CFixedFunctionPipelineNode *ff = GetFixedFunctionPipeline();

	iff.enterForm(TAG_0001);
		iff.enterChunk(TAG_DATA);

			const int numberOfStages = iff.read_int8();

			CString pixelShaderTag;
			IffReadTag (iff, pixelShaderTag);

			IffReadMap (iff, m_shadeMode, ms_shadeMode);

			IffReadBool(iff, m_ditherEnable);

			IffReadBool(iff, m_zEnable);
			IffReadBool(iff, m_zWrite);
			IffReadMap (iff, m_zCompare, ms_compare);

			IffReadBool(iff, m_alphaBlendEnable);
			IffReadMap (iff, m_alphaBlendOperation, ms_blendOp);
			IffReadMap (iff, m_alphaBlendSource, ms_blend);
			IffReadMap (iff, m_alphaBlendDestination, ms_blend);

			IffReadBool(iff, m_alphaTestEnable);
			IffReadTag (iff, m_alphaTestTag);
			IffReadMap (iff, m_alphaTestFunction, ms_compare);

			byte mask = iff.read_uint8();
			m_writeMaskA = ((mask >> 3) & 1) != 0;
			m_writeMaskR = ((mask >> 2) & 1) != 0;
			m_writeMaskG = ((mask >> 1) & 1) != 0;
			m_writeMaskB = ((mask >> 0) & 1) != 0;

			IffReadTag (iff, m_textureFactorTag);

			IffReadBool(iff, ff->m_lighting);
			IffReadBool(iff, ff->m_lightingSpecularEnable);
			IffReadBool(iff, ff->m_lightingColorVertex);
			IffReadTag (iff, m_materialTag);
			IffReadMap (iff, ff->m_lightingAmbientColorSource, ms_materialSource);
			IffReadMap (iff, ff->m_lightingDiffuseColorSource, ms_materialSource);
			IffReadMap (iff, ff->m_lightingSpecularColorSource, ms_materialSource);
			IffReadMap (iff, ff->m_lightingEmissiveColorSource, ms_materialSource);

			IffReadBool(iff, m_stencilEnable);
			IffReadTag (iff, m_stencilReferenceTag);
			IffReadMap (iff, m_stencilCompareFunction, ms_compare);
			IffReadMap (iff, m_stencilPassOperation, ms_stencilOp);
			IffReadMap (iff, m_stencilZFailOperation, ms_stencilOp);
			IffReadMap (iff, m_stencilFailOperation, ms_stencilOp);
			IffReadInt (iff, m_stencilWriteMask);
			IffReadInt (iff, m_stencilMask);

		iff.exitChunk(TAG_DATA);

		for (int i = 0; i < numberOfStages; ++i)
		{
			CStageNode *stageNode = new CStageNode(*this);
			if (!stageNode->Load(iff))
				return false;
			++m_textureStageCount;
		}

	iff.exitForm(TAG_0001);

	return true;
}

// ----------------------------------------------------------------------

bool CPassNode::Load_0002(Iff &iff)
{
	AddFixedFunctionPipeline();
	CFixedFunctionPipelineNode *ff = GetFixedFunctionPipeline();

	iff.enterForm(TAG_0002);
		iff.enterChunk(TAG_DATA);

			const bool pixelShader    = iff.read_int8() == 0 ? false : true;
			const int  numberOfStages = iff.read_int8();

			IffReadMap (iff, m_shadeMode, ms_shadeMode);
			IffReadMap (iff, m_fogMode, ms_fogMode);

			IffReadBool(iff, m_ditherEnable);

			IffReadBool(iff, m_zEnable);
			IffReadBool(iff, m_zWrite);
			IffReadMap (iff, m_zCompare, ms_compare);

			IffReadBool(iff, m_alphaBlendEnable);
			IffReadMap (iff, m_alphaBlendOperation, ms_blendOp);
			IffReadMap (iff, m_alphaBlendSource, ms_blend);
			IffReadMap (iff, m_alphaBlendDestination, ms_blend);

			IffReadBool(iff, m_alphaTestEnable);
			IffReadTag (iff, m_alphaTestTag);
			IffReadMap (iff, m_alphaTestFunction, ms_compare);

			byte mask = iff.read_uint8();
			m_writeMaskA = ((mask >> 3) & 1) != 0;
			m_writeMaskR = ((mask >> 2) & 1) != 0;
			m_writeMaskG = ((mask >> 1) & 1) != 0;
			m_writeMaskB = ((mask >> 0) & 1) != 0;

			IffReadTag (iff, m_textureFactorTag);

			IffReadBool(iff, ff->m_lighting);
			IffReadBool(iff, ff->m_lightingSpecularEnable);
			IffReadBool(iff, ff->m_lightingColorVertex);
			IffReadTag (iff, m_materialTag);
			IffReadMap (iff, ff->m_lightingAmbientColorSource, ms_materialSource);
			IffReadMap (iff, ff->m_lightingDiffuseColorSource, ms_materialSource);
			IffReadMap (iff, ff->m_lightingSpecularColorSource, ms_materialSource);
			IffReadMap (iff, ff->m_lightingEmissiveColorSource, ms_materialSource);

			IffReadBool(iff, m_stencilEnable);
			IffReadTag (iff, m_stencilReferenceTag);
			IffReadMap (iff, m_stencilCompareFunction, ms_compare);
			IffReadMap (iff, m_stencilPassOperation, ms_stencilOp);
			IffReadMap (iff, m_stencilZFailOperation, ms_stencilOp);
			IffReadMap (iff, m_stencilFailOperation, ms_stencilOp);
			IffReadInt (iff, m_stencilWriteMask);
			IffReadInt (iff, m_stencilMask);

		iff.exitChunk(TAG_DATA);

		if (pixelShader)
		{
			CPixelShaderNode *pixelShader = new CPixelShaderNode(*this);
			if (!pixelShader->Load(iff))
				return false;
			++m_pixelShaderCount;
		}
		else
			for (int i = 0; i < numberOfStages; ++i)
			{
				CStageNode *stageNode = new CStageNode(*this);
				if (!stageNode->Load(iff))
					return false;
				++m_textureStageCount;
			}

	iff.exitForm(TAG_0002);

	return true;
}

// ----------------------------------------------------------------------

bool CPassNode::Load_0003(Iff &iff)
{
	AddFixedFunctionPipeline();
	CFixedFunctionPipelineNode *ff = GetFixedFunctionPipeline();

	iff.enterForm(TAG_0003);
		iff.enterChunk(TAG_DATA);

			const bool pixelShader    = iff.read_int8() == 0 ? false : true;
			const int  numberOfStages = iff.read_int8();

			IffReadMap (iff, m_shadeMode, ms_shadeMode);
			IffReadMap (iff, m_fogMode, ms_fogMode);

			IffReadBool(iff, m_ditherEnable);

			IffReadBool(iff, m_zEnable);
			IffReadBool(iff, m_zWrite);
			IffReadMap (iff, m_zCompare, ms_compare);

			IffReadBool(iff, m_alphaBlendEnable);
			IffReadMap (iff, m_alphaBlendOperation, ms_blendOp);
			IffReadMap (iff, m_alphaBlendSource, ms_blend);
			IffReadMap (iff, m_alphaBlendDestination, ms_blend);

			IffReadBool(iff, m_alphaTestEnable);
			IffReadTag (iff, m_alphaTestTag);
			IffReadMap (iff, m_alphaTestFunction, ms_compare);

			byte mask = iff.read_uint8();
			m_writeMaskA = ((mask >> 3) & 1) != 0;
			m_writeMaskR = ((mask >> 2) & 1) != 0;
			m_writeMaskG = ((mask >> 1) & 1) != 0;
			m_writeMaskB = ((mask >> 0) & 1) != 0;

			IffReadTag (iff, m_textureFactorTag);

			IffReadBool(iff, ff->m_lighting);
			IffReadBool(iff, ff->m_lightingSpecularEnable);
			IffReadBool(iff, ff->m_lightingColorVertex);
			IffReadTag (iff, m_materialTag);
			IffReadMap (iff, ff->m_lightingAmbientColorSource, ms_materialSource);
			IffReadMap (iff, ff->m_lightingDiffuseColorSource, ms_materialSource);
			IffReadMap (iff, ff->m_lightingSpecularColorSource, ms_materialSource);
			IffReadMap (iff, ff->m_lightingEmissiveColorSource, ms_materialSource);

			IffReadBool(iff, m_stencilEnable);
			IffReadTag (iff, m_stencilReferenceTag);
			IffReadMap (iff, m_stencilCompareFunction, ms_compare);
			IffReadMap (iff, m_stencilPassOperation, ms_stencilOp);
			IffReadMap (iff, m_stencilZFailOperation, ms_stencilOp);
			IffReadMap (iff, m_stencilFailOperation, ms_stencilOp);
			IffReadInt (iff, m_stencilWriteMask);
			IffReadInt (iff, m_stencilMask);

			CString junkVertexShaderTag;
			IffReadTag (iff, junkVertexShaderTag);

		iff.exitChunk(TAG_DATA);

		if (pixelShader)
		{
			CPixelShaderNode *pixelShader = new CPixelShaderNode(*this);
			if (!pixelShader->Load(iff))
				return false;
			++m_pixelShaderCount;
		}
		else
			for (int i = 0; i < numberOfStages; ++i)
			{
				CStageNode *stageNode = new CStageNode(*this);
				if (!stageNode->Load(iff))
					return false;
				++m_textureStageCount;
			}

	iff.exitForm(TAG_0003);

	return true;
}

// ----------------------------------------------------------------------

bool CPassNode::Load_0004(Iff &iff)
{
	iff.enterForm(TAG_0004);
		iff.enterChunk(TAG_DATA);

			const bool pixelShader    = iff.read_int8() == 0 ? false : true;
			const int  numberOfStages = iff.read_int8();

			IffReadMap (iff, m_shadeMode, ms_shadeMode);
			IffReadMap (iff, m_fogMode, ms_fogMode);

			IffReadBool(iff, m_ditherEnable);

			IffReadBool(iff, m_zEnable);
			IffReadBool(iff, m_zWrite);
			IffReadMap (iff, m_zCompare, ms_compare);

			IffReadBool(iff, m_alphaBlendEnable);
			IffReadMap (iff, m_alphaBlendOperation, ms_blendOp);
			IffReadMap (iff, m_alphaBlendSource, ms_blend);
			IffReadMap (iff, m_alphaBlendDestination, ms_blend);

			IffReadBool(iff, m_alphaTestEnable);
			IffReadTag (iff, m_alphaTestTag);
			IffReadMap (iff, m_alphaTestFunction, ms_compare);

			byte mask = iff.read_uint8();
			m_writeMaskA = ((mask >> 3) & 1) != 0;
			m_writeMaskR = ((mask >> 2) & 1) != 0;
			m_writeMaskG = ((mask >> 1) & 1) != 0;
			m_writeMaskB = ((mask >> 0) & 1) != 0;

			IffReadTag (iff, m_textureFactorTag);

			IffReadBool(iff, m_stencilEnable);
			IffReadTag (iff, m_stencilReferenceTag);
			IffReadMap (iff, m_stencilCompareFunction, ms_compare);
			IffReadMap (iff, m_stencilPassOperation, ms_stencilOp);
			IffReadMap (iff, m_stencilZFailOperation, ms_stencilOp);
			IffReadMap (iff, m_stencilFailOperation, ms_stencilOp);
			IffReadInt (iff, m_stencilWriteMask);
			IffReadInt (iff, m_stencilMask);

		iff.exitChunk(TAG_DATA);

		if (iff.getCurrentName() == TAG_PVSH)
		{
			CVertexShaderNode *vertexShaderNode = new CVertexShaderNode(*this);
			if (!vertexShaderNode->Load(iff))
				return false;
		}
		else
			if (iff.getCurrentName() == TAG_PFFP)
			{
				CFixedFunctionPipelineNode *fixedFunctionPipelineNode = new CFixedFunctionPipelineNode(*this);
				if (!fixedFunctionPipelineNode->Load(iff))
					return false;

				m_materialTag = fixedFunctionPipelineNode->m_deprecated_lightingMaterialTag;
			}
			else
				assert(false);

		if (pixelShader)
		{
			CPixelShaderNode *pixelShader = new CPixelShaderNode(*this);
			if (!pixelShader->Load(iff))
				return false;
			++m_pixelShaderCount;
		}
		else
			for (int i = 0; i < numberOfStages; ++i)
			{
				CStageNode *stageNode = new CStageNode(*this);
				if (!stageNode->Load(iff))
					return false;
				++m_textureStageCount;
			}

	iff.exitForm(TAG_0004);

	return true;
}

// ----------------------------------------------------------------------

bool CPassNode::Load_0005(Iff &iff)
{
	iff.enterForm(TAG_0005);
		iff.enterChunk(TAG_DATA);

			const int  numberOfStages = iff.read_int8();

			IffReadMap (iff, m_shadeMode, ms_shadeMode);
			IffReadMap (iff, m_fogMode, ms_fogMode);

			IffReadBool(iff, m_ditherEnable);

			IffReadBool(iff, m_zEnable);
			IffReadBool(iff, m_zWrite);
			IffReadMap (iff, m_zCompare, ms_compare);

			IffReadBool(iff, m_alphaBlendEnable);
			IffReadMap (iff, m_alphaBlendOperation, ms_blendOp);
			IffReadMap (iff, m_alphaBlendSource, ms_blend);
			IffReadMap (iff, m_alphaBlendDestination, ms_blend);

			IffReadBool(iff, m_alphaTestEnable);
			IffReadTag (iff, m_alphaTestTag);
			IffReadMap (iff, m_alphaTestFunction, ms_compare);

			byte mask = iff.read_uint8();
			m_writeMaskA = ((mask >> 3) & 1) != 0;
			m_writeMaskR = ((mask >> 2) & 1) != 0;
			m_writeMaskG = ((mask >> 1) & 1) != 0;
			m_writeMaskB = ((mask >> 0) & 1) != 0;

			IffReadTag (iff, m_textureFactorTag);

			IffReadBool(iff, m_stencilEnable);
			IffReadTag (iff, m_stencilReferenceTag);
			IffReadMap (iff, m_stencilCompareFunction, ms_compare);
			IffReadMap (iff, m_stencilPassOperation, ms_stencilOp);
			IffReadMap (iff, m_stencilZFailOperation, ms_stencilOp);
			IffReadMap (iff, m_stencilFailOperation, ms_stencilOp);
			IffReadInt (iff, m_stencilWriteMask);
			IffReadInt (iff, m_stencilMask);

		iff.exitChunk(TAG_DATA);

		if (iff.getCurrentName() == TAG_PVSH)
		{
			CVertexShaderNode *vertexShaderNode = new CVertexShaderNode(*this);
			if (!vertexShaderNode->Load(iff))
				return false;
		}
		else
			if (iff.getCurrentName() == TAG_PFFP)
			{
				CFixedFunctionPipelineNode *fixedFunctionPipelineNode = new CFixedFunctionPipelineNode(*this);
				if (!fixedFunctionPipelineNode->Load(iff))
					return false;

				m_materialTag = fixedFunctionPipelineNode->m_deprecated_lightingMaterialTag;
			}
			else
				assert(false);

		if (numberOfStages == 0 && !iff.atEndOfForm() && iff.getCurrentName() == TAG_PPSH)
		{
			CPixelShaderNode *pixelShader = new CPixelShaderNode(*this);
			if (!pixelShader->Load(iff))
				return false;
			++m_pixelShaderCount;
		}
		else
			for (int i = 0; i < numberOfStages; ++i)
			{
				CStageNode *stageNode = new CStageNode(*this);
				if (!stageNode->Load(iff))
					return false;
				++m_textureStageCount;
			}

	iff.exitForm(TAG_0005);

	return true;
}

// ----------------------------------------------------------------------

bool CPassNode::Load_0006(Iff &iff)
{
	iff.enterForm(TAG_0006);
		iff.enterChunk(TAG_DATA);

			const int  numberOfStages = iff.read_int8();

			IffReadMap (iff, m_shadeMode, ms_shadeMode);
			IffReadMap (iff, m_fogMode, ms_fogMode);

			IffReadBool(iff, m_ditherEnable);

			IffReadBool(iff, m_zEnable);
			IffReadBool(iff, m_zWrite);
			IffReadMap (iff, m_zCompare, ms_compare);

			IffReadBool(iff, m_alphaBlendEnable);
			IffReadMap (iff, m_alphaBlendOperation, ms_blendOp);
			IffReadMap (iff, m_alphaBlendSource, ms_blend);
			IffReadMap (iff, m_alphaBlendDestination, ms_blend);

			IffReadBool(iff, m_alphaTestEnable);
			IffReadTag (iff, m_alphaTestTag);
			IffReadMap (iff, m_alphaTestFunction, ms_compare);

			byte mask = iff.read_uint8();
			m_writeMaskA = ((mask >> 3) & 1) != 0;
			m_writeMaskR = ((mask >> 2) & 1) != 0;
			m_writeMaskG = ((mask >> 1) & 1) != 0;
			m_writeMaskB = ((mask >> 0) & 1) != 0;

			IffReadTag (iff, m_textureFactorTag);

			IffReadBool(iff, m_stencilEnable);
			IffReadTag (iff, m_stencilReferenceTag);
			IffReadMap (iff, m_stencilCompareFunction, ms_compare);
			IffReadMap (iff, m_stencilPassOperation, ms_stencilOp);
			IffReadMap (iff, m_stencilZFailOperation, ms_stencilOp);
			IffReadMap (iff, m_stencilFailOperation, ms_stencilOp);
			IffReadInt (iff, m_stencilWriteMask);
			IffReadInt (iff, m_stencilMask);

			IffReadTag (iff, m_materialTag);

		iff.exitChunk(TAG_DATA);

		if (iff.getCurrentName() == TAG_PVSH)
		{
			CVertexShaderNode *vertexShaderNode = new CVertexShaderNode(*this);
			if (!vertexShaderNode->Load(iff))
				return false;
		}
		else
			if (iff.getCurrentName() == TAG_PFFP)
			{
				CFixedFunctionPipelineNode *fixedFunctionPipelineNode = new CFixedFunctionPipelineNode(*this);
				if (!fixedFunctionPipelineNode->Load(iff))
					return false;
			}
			else
				assert(false);

		if (numberOfStages == 0 && !iff.atEndOfForm() && iff.getCurrentName() == TAG_PPSH)
		{
			CPixelShaderNode *pixelShader = new CPixelShaderNode(*this);
			if (!pixelShader->Load(iff))
				return false;
			++m_pixelShaderCount;
		}
		else
			for (int i = 0; i < numberOfStages; ++i)
			{
				CStageNode *stageNode = new CStageNode(*this);
				if (!stageNode->Load(iff))
					return false;
				++m_textureStageCount;
			}

	iff.exitForm(TAG_0006);

	return true;
}

// ----------------------------------------------------------------------

bool CPassNode::Load_0007(Iff &iff)
{
	iff.enterForm(TAG_0007);
		iff.enterChunk(TAG_DATA);

			const int  numberOfStages = iff.read_int8();

			IffReadMap (iff, m_shadeMode, ms_shadeMode);
			IffReadMap (iff, m_fogMode, ms_fogMode);

			IffReadBool(iff, m_ditherEnable);

			IffReadBool(iff, m_zEnable);
			IffReadBool(iff, m_zWrite);
			IffReadMap (iff, m_zCompare, ms_compare);

			IffReadBool(iff, m_alphaBlendEnable);
			IffReadMap (iff, m_alphaBlendOperation, ms_blendOp);
			IffReadMap (iff, m_alphaBlendSource, ms_blend);
			IffReadMap (iff, m_alphaBlendDestination, ms_blend);

			IffReadBool(iff, m_alphaTestEnable);
			IffReadTag (iff, m_alphaTestTag);
			IffReadMap (iff, m_alphaTestFunction, ms_compare);

			byte mask = iff.read_uint8();
			m_writeMaskA = ((mask >> 3) & 1) != 0;
			m_writeMaskR = ((mask >> 2) & 1) != 0;
			m_writeMaskG = ((mask >> 1) & 1) != 0;
			m_writeMaskB = ((mask >> 0) & 1) != 0;

			IffReadTag (iff, m_textureFactorTag);
			IffReadTag (iff, m_textureFactor2Tag);

			IffReadBool(iff, m_stencilEnable);
			IffReadTag (iff, m_stencilReferenceTag);
			IffReadMap (iff, m_stencilCompareFunction, ms_compare);
			IffReadMap (iff, m_stencilPassOperation, ms_stencilOp);
			IffReadMap (iff, m_stencilZFailOperation, ms_stencilOp);
			IffReadMap (iff, m_stencilFailOperation, ms_stencilOp);
			IffReadInt (iff, m_stencilWriteMask);
			IffReadInt (iff, m_stencilMask);

			IffReadTag (iff, m_materialTag);

		iff.exitChunk(TAG_DATA);

		if (iff.getCurrentName() == TAG_PVSH)
		{
			CVertexShaderNode *vertexShaderNode = new CVertexShaderNode(*this);
			if (!vertexShaderNode->Load(iff))
				return false;
		}
		else
			if (iff.getCurrentName() == TAG_PFFP)
			{
				CFixedFunctionPipelineNode *fixedFunctionPipelineNode = new CFixedFunctionPipelineNode(*this);
				if (!fixedFunctionPipelineNode->Load(iff))
					return false;
			}
			else
				assert(false);

		if (numberOfStages == 0 && !iff.atEndOfForm() && iff.getCurrentName() == TAG_PPSH)
		{
			CPixelShaderNode *pixelShader = new CPixelShaderNode(*this);
			if (!pixelShader->Load(iff))
				return false;
			++m_pixelShaderCount;
		}
		else
			for (int i = 0; i < numberOfStages; ++i)
			{
				CStageNode *stageNode = new CStageNode(*this);
				if (!stageNode->Load(iff))
					return false;
				++m_textureStageCount;
			}

	iff.exitForm(TAG_0007);

	return true;
}

// ----------------------------------------------------------------------

bool CPassNode::Load_0008(Iff &iff)
{
	iff.enterForm(TAG_0008);
		iff.enterChunk(TAG_DATA);

			const int  numberOfStages = iff.read_int8();

			IffReadMap (iff, m_shadeMode, ms_shadeMode);
			IffReadMap (iff, m_fogMode, ms_fogMode);

			IffReadBool(iff, m_ditherEnable);

			IffReadBool(iff, m_zEnable);
			IffReadBool(iff, m_zWrite);
			IffReadMap (iff, m_zCompare, ms_compare);

			IffReadBool(iff, m_alphaBlendEnable);
			IffReadMap (iff, m_alphaBlendOperation, ms_blendOp);
			IffReadMap (iff, m_alphaBlendSource, ms_blend);
			IffReadMap (iff, m_alphaBlendDestination, ms_blend);

			IffReadBool(iff, m_alphaTestEnable);
			IffReadTag (iff, m_alphaTestTag);
			IffReadMap (iff, m_alphaTestFunction, ms_compare);

			byte mask = iff.read_uint8();
			m_writeMaskA = ((mask >> 3) & 1) != 0;
			m_writeMaskR = ((mask >> 2) & 1) != 0;
			m_writeMaskG = ((mask >> 1) & 1) != 0;
			m_writeMaskB = ((mask >> 0) & 1) != 0;

			IffReadTag (iff, m_textureFactorTag);
			IffReadTag (iff, m_textureFactor2Tag);
			IffReadTag (iff, m_textureScrollTag);

			IffReadBool(iff, m_stencilEnable);
			IffReadTag (iff, m_stencilReferenceTag);
			IffReadMap (iff, m_stencilCompareFunction, ms_compare);
			IffReadMap (iff, m_stencilPassOperation, ms_stencilOp);
			IffReadMap (iff, m_stencilZFailOperation, ms_stencilOp);
			IffReadMap (iff, m_stencilFailOperation, ms_stencilOp);
			IffReadInt (iff, m_stencilWriteMask);
			IffReadInt (iff, m_stencilMask);

			IffReadTag (iff, m_materialTag);

		iff.exitChunk(TAG_DATA);

		if (iff.getCurrentName() == TAG_PVSH)
		{
			CVertexShaderNode *vertexShaderNode = new CVertexShaderNode(*this);
			if (!vertexShaderNode->Load(iff))
				return false;
		}
		else
			if (iff.getCurrentName() == TAG_PFFP)
			{
				CFixedFunctionPipelineNode *fixedFunctionPipelineNode = new CFixedFunctionPipelineNode(*this);
				if (!fixedFunctionPipelineNode->Load(iff))
					return false;
			}
			else
				assert(false);

		if (numberOfStages == 0 && !iff.atEndOfForm() && iff.getCurrentName() == TAG_PPSH)
		{
			CPixelShaderNode *pixelShader = new CPixelShaderNode(*this);
			if (!pixelShader->Load(iff))
				return false;
			++m_pixelShaderCount;
		}
		else
			for (int i = 0; i < numberOfStages; ++i)
			{
				CStageNode *stageNode = new CStageNode(*this);
				if (!stageNode->Load(iff))
					return false;
				++m_textureStageCount;
			}

	iff.exitForm(TAG_0008);

	return true;
}

// ----------------------------------------------------------------------

bool CPassNode::Load_0009(Iff &iff)
{
	iff.enterForm(TAG_0009);
		iff.enterChunk(TAG_DATA);

			const int  numberOfStages = iff.read_int8();

			IffReadMap (iff, m_shadeMode, ms_shadeMode);
			IffReadMap (iff, m_fogMode, ms_fogMode);

			IffReadBool(iff, m_ditherEnable);

			IffReadBool(iff, m_zEnable);
			IffReadBool(iff, m_zWrite);
			IffReadMap (iff, m_zCompare, ms_compare);

			IffReadBool(iff, m_alphaBlendEnable);
			IffReadMap (iff, m_alphaBlendOperation, ms_blendOp);
			IffReadMap (iff, m_alphaBlendSource, ms_blend);
			IffReadMap (iff, m_alphaBlendDestination, ms_blend);

			IffReadBool(iff, m_alphaTestEnable);
			IffReadTag (iff, m_alphaTestTag);
			IffReadMap (iff, m_alphaTestFunction, ms_compare);

			byte mask = iff.read_uint8();
			m_writeMaskA = ((mask >> 3) & 1) != 0;
			m_writeMaskR = ((mask >> 2) & 1) != 0;
			m_writeMaskG = ((mask >> 1) & 1) != 0;
			m_writeMaskB = ((mask >> 0) & 1) != 0;

			IffReadTag (iff, m_textureFactorTag);
			IffReadTag (iff, m_textureFactor2Tag);
			IffReadTag (iff, m_textureScrollTag);

			IffReadBool(iff, m_stencilEnable);
			IffReadBool(iff, m_stencilTwoSidedMode);
			IffReadTag (iff, m_stencilReferenceTag);
			IffReadMap (iff, m_stencilCompareFunction, ms_compare);
			IffReadMap (iff, m_stencilPassOperation, ms_stencilOp);
			IffReadMap (iff, m_stencilZFailOperation, ms_stencilOp);
			IffReadMap (iff, m_stencilFailOperation, ms_stencilOp);
			IffReadMap (iff, m_stencilCounterClockwiseCompareFunction, ms_compare);
			IffReadMap (iff, m_stencilCounterClockwisePassOperation, ms_stencilOp);
			IffReadMap (iff, m_stencilCounterClockwiseZFailOperation, ms_stencilOp);
			IffReadMap (iff, m_stencilCounterClockwiseFailOperation, ms_stencilOp);
			IffReadInt (iff, m_stencilWriteMask);
			IffReadInt (iff, m_stencilMask);

			IffReadTag (iff, m_materialTag);

		iff.exitChunk(TAG_DATA);

		if (iff.getCurrentName() == TAG_PVSH)
		{
			CVertexShaderNode *vertexShaderNode = new CVertexShaderNode(*this);
			if (!vertexShaderNode->Load(iff))
				return false;
		}
		else
			if (iff.getCurrentName() == TAG_PFFP)
			{
				CFixedFunctionPipelineNode *fixedFunctionPipelineNode = new CFixedFunctionPipelineNode(*this);
				if (!fixedFunctionPipelineNode->Load(iff))
					return false;
			}
			else
				assert(false);

		if (numberOfStages == 0 && !iff.atEndOfForm() && iff.getCurrentName() == TAG_PPSH)
		{
			CPixelShaderNode *pixelShader = new CPixelShaderNode(*this);
			if (!pixelShader->Load(iff))
				return false;
			++m_pixelShaderCount;
		}
		else
			for (int i = 0; i < numberOfStages; ++i)
			{
				CStageNode *stageNode = new CStageNode(*this);
				if (!stageNode->Load(iff))
					return false;
				++m_textureStageCount;
			}

	iff.exitForm(TAG_0009);

	return true;
}

// ----------------------------------------------------------------------

bool CPassNode::Load_0010(Iff &iff)
{
	iff.enterForm(TAG_0010);
		iff.enterChunk(TAG_DATA);

			const int  numberOfStages = iff.read_int8();

			IffReadMap (iff, m_shadeMode, ms_shadeMode);
			IffReadMap (iff, m_fogMode, ms_fogMode);

			IffReadBool(iff, m_ditherEnable);
			IffReadBool(iff, m_heat);

			IffReadBool(iff, m_zEnable);
			IffReadBool(iff, m_zWrite);
			IffReadMap (iff, m_zCompare, ms_compare);

			IffReadBool(iff, m_alphaBlendEnable);
			IffReadMap (iff, m_alphaBlendOperation, ms_blendOp);
			IffReadMap (iff, m_alphaBlendSource, ms_blend);
			IffReadMap (iff, m_alphaBlendDestination, ms_blend);

			IffReadBool(iff, m_alphaTestEnable);
			IffReadTag (iff, m_alphaTestTag);
			IffReadMap (iff, m_alphaTestFunction, ms_compare);

			byte mask = iff.read_uint8();
			m_writeMaskA = ((mask >> 3) & 1) != 0;
			m_writeMaskR = ((mask >> 2) & 1) != 0;
			m_writeMaskG = ((mask >> 1) & 1) != 0;
			m_writeMaskB = ((mask >> 0) & 1) != 0;

			IffReadTag (iff, m_textureFactorTag);
			IffReadTag (iff, m_textureFactor2Tag);
			IffReadTag (iff, m_textureScrollTag);

			IffReadBool(iff, m_stencilEnable);
			IffReadBool(iff, m_stencilTwoSidedMode);
			IffReadTag (iff, m_stencilReferenceTag);
			IffReadMap (iff, m_stencilCompareFunction, ms_compare);
			IffReadMap (iff, m_stencilPassOperation, ms_stencilOp);
			IffReadMap (iff, m_stencilZFailOperation, ms_stencilOp);
			IffReadMap (iff, m_stencilFailOperation, ms_stencilOp);
			IffReadMap (iff, m_stencilCounterClockwiseCompareFunction, ms_compare);
			IffReadMap (iff, m_stencilCounterClockwisePassOperation, ms_stencilOp);
			IffReadMap (iff, m_stencilCounterClockwiseZFailOperation, ms_stencilOp);
			IffReadMap (iff, m_stencilCounterClockwiseFailOperation, ms_stencilOp);
			IffReadInt (iff, m_stencilWriteMask);
			IffReadInt (iff, m_stencilMask);

			IffReadTag (iff, m_materialTag);

		iff.exitChunk(TAG_DATA);

		if (iff.getCurrentName() == TAG_PVSH)
		{
			CVertexShaderNode *vertexShaderNode = new CVertexShaderNode(*this);
			if (!vertexShaderNode->Load(iff))
				return false;
		}
		else
			if (iff.getCurrentName() == TAG_PFFP)
			{
				CFixedFunctionPipelineNode *fixedFunctionPipelineNode = new CFixedFunctionPipelineNode(*this);
				if (!fixedFunctionPipelineNode->Load(iff))
					return false;
			}
			else
				assert(false);

		if (numberOfStages == 0 && !iff.atEndOfForm() && iff.getCurrentName() == TAG_PPSH)
		{
			CPixelShaderNode *pixelShader = new CPixelShaderNode(*this);
			if (!pixelShader->Load(iff))
				return false;
			++m_pixelShaderCount;
		}
		else
			for (int i = 0; i < numberOfStages; ++i)
			{
				CStageNode *stageNode = new CStageNode(*this);
				if (!stageNode->Load(iff))
					return false;
				++m_textureStageCount;
			}

	iff.exitForm(TAG_0010);

	return true;
}

// ----------------------------------------------------------------------

bool CPassNode::Save(Iff &iff) const
{
	iff.insertForm(TAG_PASS);
		iff.insertForm(TAG_0010);
			iff.insertChunk(TAG_DATA);

				if (m_pixelShaderCount)
				{
					iff.insertChunkData(static_cast<int8>(0));
				}
				else
				{
					iff.insertChunkData(static_cast<int8>(children.size()-1));
				}

				iff.insertChunkData(ms_shadeMode[m_shadeMode]);
				iff.insertChunkData(ms_fogMode[m_fogMode]);

				iff.insertChunkData(static_cast<int8>(m_ditherEnable ? 1 : 0));
				iff.insertChunkData(static_cast<int8>(m_heat ? 1 : 0));

				iff.insertChunkData(static_cast<int8>(m_zEnable ? 1 : 0));
				iff.insertChunkData(static_cast<int8>(m_zWrite ? 1 : 0));
				iff.insertChunkData(ms_compare[m_zCompare]);

				iff.insertChunkData(static_cast<int8>(m_alphaBlendEnable ? 1 : 0));
				iff.insertChunkData(ms_blendOp[m_alphaBlendOperation]);
				iff.insertChunkData(ms_blend[m_alphaBlendSource]);
				iff.insertChunkData(ms_blend[m_alphaBlendDestination]);

				iff.insertChunkData(static_cast<int8>(m_alphaTestEnable ? 1 : 0));
				iff.insertChunkData(convertToTag(m_alphaTestTag));
				iff.insertChunkData(ms_compare[m_alphaTestFunction]);

				byte mask = 0;
				mask |= m_writeMaskA ? (1 << 3) : 0;
				mask |= m_writeMaskR ? (1 << 2) : 0;
				mask |= m_writeMaskG ? (1 << 1) : 0;
				mask |= m_writeMaskB ? (1 << 0) : 0;
				iff.insertChunkData(mask);

				iff.insertChunkData(convertToTag(m_textureFactorTag));
				iff.insertChunkData(convertToTag(m_textureFactor2Tag));
				iff.insertChunkData(convertToTag(m_textureScrollTag));

				iff.insertChunkData(static_cast<int8>(m_stencilEnable ? 1 : 0));
				iff.insertChunkData(static_cast<int8>(m_stencilTwoSidedMode ? 1 : 0));
				iff.insertChunkData(convertToTag(m_stencilReferenceTag));
				iff.insertChunkData(ms_compare[m_stencilCompareFunction]);
				iff.insertChunkData(ms_stencilOp[m_stencilPassOperation]);
				iff.insertChunkData(ms_stencilOp[m_stencilZFailOperation]);
				iff.insertChunkData(ms_stencilOp[m_stencilFailOperation]);
				iff.insertChunkData(ms_compare[m_stencilCounterClockwiseCompareFunction]);
				iff.insertChunkData(ms_stencilOp[m_stencilCounterClockwisePassOperation]);
				iff.insertChunkData(ms_stencilOp[m_stencilCounterClockwiseZFailOperation]);
				iff.insertChunkData(ms_stencilOp[m_stencilCounterClockwiseFailOperation]);
				iff.insertChunkData(static_cast<uint32>(atoi(m_stencilWriteMask)));
				iff.insertChunkData(static_cast<uint32>(atoi(m_stencilMask)));

				iff.insertChunkData(convertToTag(m_materialTag));

			iff.exitChunk(TAG_DATA);


			if (GetFixedFunctionPipeline())
			{
				if (!GetFixedFunctionPipeline()->Save(iff))
					return false;
			}
			else
				if (GetVertexShader())
				{
					if (!GetVertexShader()->Save(iff))
						return false;
				}
				else
					ASSERT(false);

			if (m_pixelShaderCount)
			{
				if (!GetPixelShader()->Save(iff))
					return false;
			}
			else
			{
				int count = 0;
				for (const CStageNode *stage = GetFirstStage(); stage; stage = GetNextStage(*stage), ++count)
					if(!stage->Save(iff))
						return false;

				// make sure we wrote out as many as we said we would
				ASSERT(count == static_cast<int>(children.size()-1));
			}

		iff.exitForm(TAG_0010);
	iff.exitForm(TAG_PASS);

	return true;
}

// ----------------------------------------------------------------------

bool CPassNode::Validate()
{
	//check for alpha testing enabled without an alpha reference tag
	if(m_alphaTestEnable && m_alphaTestTag == "")
	{
		MessageBox(NULL, "Alpha testing enabled, but no alpha reference tag defined.", "Error", MB_OK | MB_ICONERROR);
		treeCtrl->Select(treeItem, TVGN_CARET);
		leftView->mainFrame->passView->m_alphaTestTag.SetFocus();
		return false;
	}

	//check for stencil enabled without a stencil reference tag
	if(m_stencilEnable && m_stencilReferenceTag == "")
	{
		MessageBox(NULL, "Stencil enabled, but no stencil reference tag defined.", "Error", MB_OK | MB_ICONERROR);
		treeCtrl->Select(treeItem, TVGN_CARET);
		leftView->mainFrame->passView->m_stencilReferenceTag.SetFocus();
		return false;
	}

	if (!GetFixedFunctionPipeline()  && !GetVertexShader())
	{
		MessageBox(NULL, "Pass needs either fixed functin pipeline or vertex shader.", "Error", MB_OK | MB_ICONERROR);
		treeCtrl->Select(treeItem, TVGN_CARET);
		leftView->mainFrame->passView->m_zEnable.SetFocus();
		return false;
	}

	int count = 0;
	for (CStageNode *stage = const_cast<CStageNode*>(GetFirstStage()); stage; stage = const_cast<CStageNode*>(GetNextStage(*stage)), ++count)
		if(!stage->Validate())
			return false;

	return true;
}

// ----------------------------------------------------------------------

void CPassNode::Lint()
{
	if (m_stencilCounterClockwiseCompareFunction != "Greater" || m_stencilCounterClockwisePassOperation != "Keep" || m_stencilCounterClockwiseZFailOperation != "Keep" || m_stencilCounterClockwiseFailOperation != "Keep")
	{
		if (MessageBox(NULL, "Stencil counter clockwise values are not the default.  Fix?", "Lint", MB_YESNO) == IDYES)
		{
			CTemplateTreeNode::GetInstance()->GetEffect().Localize();
			m_stencilCounterClockwiseCompareFunction = "Greater";
			m_stencilCounterClockwisePassOperation = "Keep";
			m_stencilCounterClockwiseZFailOperation = "Keep";
			m_stencilCounterClockwiseFailOperation = "Keep";
		}
	}

	if (m_alphaTestTag != "" && m_alphaTestTag != "A000")
	{
		if (MessageBox(NULL, "Alpha test tag set, but is not A000.  Fix?", "Lint", MB_YESNO) == IDYES)
		{
			CTemplateTreeNode::GetInstance()->GetEffect().Localize();
			CTemplateTreeNode::GetInstance()->GetAlphaReferenceValueTree().Remove(m_alphaTestTag);
			m_alphaTestTag = "A000";
			CTemplateTreeNode::GetInstance()->GetAlphaReferenceValueTree().Add(m_alphaTestTag);
		}
	}
}

// ----------------------------------------------------------------------

void CPassNode::Delete()
{
	CTemplateTreeNode::GetInstance()->GetTextureFactorTree().Remove(m_textureFactorTag);
	CTemplateTreeNode::GetInstance()->GetTextureFactorTree().Remove(m_textureFactor2Tag);
	CTemplateTreeNode::GetInstance()->GetTextureScrollTree().Remove(m_textureScrollTag);
	CTemplateTreeNode::GetInstance()->GetAlphaReferenceValueTree().Remove(m_alphaTestTag);
	CTemplateTreeNode::GetInstance()->GetStencilReferenceValueTree().Remove(m_stencilReferenceTag);
	CTemplateTreeNode::GetInstance()->GetMaterialTree().Remove(m_materialTag);

	// delete all the stages
	for (Children::iterator i = children.begin(); i != children.end(); i = children.begin())
	{
		switch (i->second->GetType())
		{
			case FixedFunctionPipeline:
				static_cast<CFixedFunctionPipelineNode *>(i->second)->Delete();
				break;

			case VertexShader:
				static_cast<CVertexShaderNode *>(i->second)->Delete();
				break;

			case Stage:
				static_cast<CStageNode *>(i->second)->Delete();
				break;

			case PixelShader:
				static_cast<CPixelShaderNode *>(i->second)->Delete();
				break;
		}
	}

	implementation.RemovePass(*this);
	delete this;
}

// ----------------------------------------------------------------------

void CPassNode::AddLocalStage()
{
	assert(m_pixelShaderCount == 0);
	++m_textureStageCount;
	new CStageNode(*this);
}

// ----------------------------------------------------------------------

void CPassNode::AddLocalStage(const CStageNode &stage, HTREEITEM after)
{
	assert(m_pixelShaderCount == 0);
	++m_textureStageCount;
	new CStageNode(*this, stage, after);
}

// ----------------------------------------------------------------------

void CPassNode::RemoveStage(CStageNode &stage)
{
	--m_textureStageCount;
	RemoveItem(stage);
}

// ----------------------------------------------------------------------

void CPassNode::AddLocalPixelShader()
{
	assert(m_textureStageCount == 0);
	++m_pixelShaderCount;
	new CPixelShaderNode(*this);
}

// ----------------------------------------------------------------------

void CPassNode::AddLocalPixelShader(const CPixelShaderNode &pixelShader)
{
	assert(m_textureStageCount == 0);
	++m_pixelShaderCount;
	new CPixelShaderNode(*this, pixelShader);
}

// ----------------------------------------------------------------------

void CPassNode::RemovePixelShader(CPixelShaderNode &pixelShader)
{
	--m_pixelShaderCount;
	RemoveItem(pixelShader);
}

// ----------------------------------------------------------------------

CStageNode *CPassNode::GetFirstStage()
{
	for (CNode *node = GetFirstChild(); node; node = GetNextChild(*node))
		if (node->GetType() == Stage)
			return static_cast<CStageNode *>(node);

	return NULL;
}

// ----------------------------------------------------------------------

CStageNode *CPassNode::GetNextStage(const CStageNode &stageNode)
{
	return static_cast<CStageNode *>(GetNextChild(Stage, stageNode));
}

// ----------------------------------------------------------------------

const CStageNode *CPassNode::GetFirstStage() const
{
	for (const CNode *node = GetFirstChild(); node; node = GetNextChild(*node))
		if (node->GetType() == Stage)
			return static_cast<const CStageNode *>(node);

	return NULL;
}

// ----------------------------------------------------------------------

const CStageNode *CPassNode::GetNextStage(const CStageNode &stageNode) const
{
	return static_cast<const CStageNode *>(GetNextChild(Stage, stageNode));
}

// ----------------------------------------------------------------------

void CPassNode::AddFixedFunctionPipeline()
{
	assert(GetFixedFunctionPipeline() == 0);
	assert(GetVertexShader() == 0);
	new CFixedFunctionPipelineNode(*this);
}

// ----------------------------------------------------------------------

void CPassNode::AddFixedFunctionPipeline(const CFixedFunctionPipelineNode &copyFrom)
{
	assert(GetFixedFunctionPipeline() == 0);
	assert(GetVertexShader() == 0);
	new CFixedFunctionPipelineNode(*this, copyFrom);
}

// ----------------------------------------------------------------------

void CPassNode::RemoveFixedFunctionPipeline(CFixedFunctionPipelineNode &fixedFunctionPipelineNode)
{
	RemoveItem(fixedFunctionPipelineNode);
}

// ----------------------------------------------------------------------

CFixedFunctionPipelineNode *CPassNode::GetFixedFunctionPipeline()
{
	CNode *node = GetFirstChild();
	if (node && node->GetType() == FixedFunctionPipeline)
		return static_cast<CFixedFunctionPipelineNode *>(node);
	return NULL;
}

// ----------------------------------------------------------------------

const CFixedFunctionPipelineNode *CPassNode::GetFixedFunctionPipeline() const
{
	const CNode *node = GetFirstChild();
	if (node && node->GetType() == FixedFunctionPipeline)
		return static_cast<const CFixedFunctionPipelineNode *>(node);
	return NULL;
}

// ----------------------------------------------------------------------

void CPassNode::AddVertexShader()
{
	assert(GetFixedFunctionPipeline() == 0);
	assert(GetVertexShader() == 0);
	new CVertexShaderNode(*this);
}

// ----------------------------------------------------------------------

void CPassNode::AddVertexShader(const CVertexShaderNode &vertexShaderNode)
{
	assert(GetFixedFunctionPipeline() == 0);
	assert(GetVertexShader() == 0);
	new CVertexShaderNode(*this, vertexShaderNode);
}

// ----------------------------------------------------------------------

void CPassNode::RemoveVertexShader(CVertexShaderNode &vertexShaderNode)
{
	RemoveItem(vertexShaderNode);
}

// ----------------------------------------------------------------------

CVertexShaderNode *CPassNode::GetVertexShader()
{
	CNode *node = GetFirstChild();
	if (node && node->GetType() == VertexShader)
		return static_cast<CVertexShaderNode *>(node);
	return NULL;
}

// ----------------------------------------------------------------------

const CVertexShaderNode *CPassNode::GetVertexShader() const
{
	const CNode *node = GetFirstChild();
	if (node && node->GetType() == VertexShader)
		return static_cast<const CVertexShaderNode *>(node);
	return NULL;
}

// ----------------------------------------------------------------------

CPixelShaderNode *CPassNode::GetPixelShader()
{
	for (CNode *node = GetFirstChild(); node; node = GetNextChild(*node))
		if (node->GetType() == PixelShader)
			return static_cast<CPixelShaderNode *>(node);

	return NULL;
}

// ----------------------------------------------------------------------

const CPixelShaderNode *CPassNode::GetPixelShader() const
{
	for (const CNode *node = GetFirstChild(); node; node = GetNextChild(*node))
		if (node->GetType() == PixelShader)
			return static_cast<const CPixelShaderNode *>(node);

	return NULL;
}

// ======================================================================

CFixedFunctionPipelineNode::CFixedFunctionPipelineNode(CPassNode &pass)
: CNode(FixedFunctionPipeline),
	m_lightingEmissiveColorSource("Material"),
	m_lightingSpecularColorSource("Material"),
	m_lightingDiffuseColorSource("Material"),
	m_lightingAmbientColorSource("Material"),
	m_deprecated_lightingMaterialTag(""),
	m_lightingColorVertex(FALSE),
	m_lightingSpecularEnable(FALSE),
	m_lighting(TRUE),
	m_pass(pass)
{
	m_pass.InsertItem("Fixed Function Pipeline", *this, TVI_FIRST);
}

// ----------------------------------------------------------------------

CFixedFunctionPipelineNode::CFixedFunctionPipelineNode(CPassNode &pass, const CFixedFunctionPipelineNode &copyDataFrom)
: CNode(FixedFunctionPipeline),
	m_lightingEmissiveColorSource(copyDataFrom.m_lightingEmissiveColorSource),
	m_lightingSpecularColorSource(copyDataFrom.m_lightingSpecularColorSource),
	m_lightingDiffuseColorSource(copyDataFrom.m_lightingDiffuseColorSource),
	m_lightingAmbientColorSource(copyDataFrom.m_lightingAmbientColorSource),
	m_deprecated_lightingMaterialTag(copyDataFrom.m_deprecated_lightingMaterialTag),
	m_lightingColorVertex(copyDataFrom.m_lightingColorVertex),
	m_lightingSpecularEnable(copyDataFrom.m_lightingSpecularEnable),
	m_lighting(copyDataFrom.m_lighting),
	m_pass(pass)
{
	m_pass.InsertItem("Fixed Function Pipeline", *this, TVI_FIRST);
}

// ----------------------------------------------------------------------

CFixedFunctionPipelineNode::~CFixedFunctionPipelineNode()
{
}

// ----------------------------------------------------------------------

bool CFixedFunctionPipelineNode::IsLocked(Whom) const
{
	return m_pass.IsLocked(Either);
}

// ----------------------------------------------------------------------

void CFixedFunctionPipelineNode::Delete()
{
	m_pass.RemoveFixedFunctionPipeline(*this);
	delete this;
}

// ----------------------------------------------------------------------

bool CFixedFunctionPipelineNode::Validate()
{
#if 0
	// @todo material has moved to the pass
	//check for lighting without a material tag
	if(m_lighting && m_lightingMaterialTag == "")
	{
		MessageBox(NULL, "Lighting enabled, but no material tag defined.", "Error", MB_OK | MB_ICONERROR);
		leftView->mainFrame->fixedFunctionPipelineView->m_lightingMaterialTag.SetFocus();
		treeCtrl->Select(treeItem, TVGN_CARET);
		return false;
	}
#endif

	return true;
}

// ----------------------------------------------------------------------

bool CFixedFunctionPipelineNode::Save(Iff &iff) const
{
	iff.insertForm(TAG_PFFP);
		iff.insertChunk(TAG_0001);

				iff.insertChunkData(static_cast<int8>(m_lighting ? 1 : 0));
				iff.insertChunkData(static_cast<int8>(m_lightingSpecularEnable ? 1 : 0));
				iff.insertChunkData(static_cast<int8>(m_lightingColorVertex ? 1 : 0));
				iff.insertChunkData(ms_materialSource[m_lightingAmbientColorSource]);
				iff.insertChunkData(ms_materialSource[m_lightingDiffuseColorSource]);
				iff.insertChunkData(ms_materialSource[m_lightingSpecularColorSource]);
				iff.insertChunkData(ms_materialSource[m_lightingEmissiveColorSource]);

		iff.exitChunk(TAG_0001);
	iff.exitForm(TAG_PFFP);

	return true;
}

// ----------------------------------------------------------------------

bool CFixedFunctionPipelineNode::Load(Iff &iff)
{
	iff.enterForm(TAG_PFFP);

		switch (iff.getCurrentName())
		{
			case TAG_0000:
				Load_0000(iff);
				break;

			case TAG_0001:
				Load_0001(iff);
				break;

			default:
				return false;
				break;
		}

	iff.exitForm(TAG_PFFP);

	return true;
}

// ----------------------------------------------------------------------

bool CFixedFunctionPipelineNode::Load_0000(Iff &iff)
{
	iff.enterChunk(TAG_0000);

		IffReadBool(iff, m_lighting);
		IffReadBool(iff, m_lightingSpecularEnable);
		IffReadBool(iff, m_lightingColorVertex);
		IffReadTag (iff, m_deprecated_lightingMaterialTag);
		IffReadMap (iff, m_lightingAmbientColorSource, ms_materialSource);
		IffReadMap (iff, m_lightingDiffuseColorSource, ms_materialSource);
		IffReadMap (iff, m_lightingSpecularColorSource, ms_materialSource);
		IffReadMap (iff, m_lightingEmissiveColorSource, ms_materialSource);

	iff.exitChunk(TAG_0000);

	return true;
}

// ----------------------------------------------------------------------

bool CFixedFunctionPipelineNode::Load_0001(Iff &iff)
{
	iff.enterChunk(TAG_0001);

		IffReadBool(iff, m_lighting);
		IffReadBool(iff, m_lightingSpecularEnable);
		IffReadBool(iff, m_lightingColorVertex);
		IffReadMap (iff, m_lightingAmbientColorSource, ms_materialSource);
		IffReadMap (iff, m_lightingDiffuseColorSource, ms_materialSource);
		IffReadMap (iff, m_lightingSpecularColorSource, ms_materialSource);
		IffReadMap (iff, m_lightingEmissiveColorSource, ms_materialSource);

	iff.exitChunk(TAG_0001);

	return true;
}

// ======================================================================

CVertexShaderNode::CVertexShaderNode(CPassNode &pass)
: CNode(VertexShader),
	m_fileName("vertex_program/"),
	m_pass(pass)
{
	m_pass.InsertItem("Vertex Shader", *this, TVI_FIRST);
	CTemplateTreeNode::GetInstance()->GetVertexShaderProgramTree().Add(m_fileName, false);
}

// ----------------------------------------------------------------------

CVertexShaderNode::CVertexShaderNode(CPassNode &pass, const CVertexShaderNode &copyDataFrom)
: CNode(VertexShader),
	m_fileName(copyDataFrom.m_fileName),
	m_pass(pass)
{
	m_pass.InsertItem("Vertex Shader", *this, TVI_FIRST);
	CTemplateTreeNode::GetInstance()->GetVertexShaderProgramTree().Add(m_fileName, false);
}

// ----------------------------------------------------------------------

CVertexShaderNode::~CVertexShaderNode()
{
}

// ----------------------------------------------------------------------

bool CVertexShaderNode::IsLocked(Whom) const
{
	return m_pass.IsLocked(Either);
}

// ----------------------------------------------------------------------

void CVertexShaderNode::Delete()
{
	CTemplateTreeNode::GetInstance()->GetVertexShaderProgramTree().Remove(m_fileName, false);
	m_pass.RemoveVertexShader(*this);
	delete this;
}

// ----------------------------------------------------------------------

bool CVertexShaderNode::Validate()
{
	return false;
}

// ----------------------------------------------------------------------

bool CVertexShaderNode::Save(Iff &iff) const
{
	iff.insertForm(TAG_PVSH);
		iff.insertChunk(TAG_0000);
			iff.insertChunkString(m_fileName);
		iff.exitChunk(TAG_0000);
	iff.exitForm(TAG_PVSH);
	return true;
}

// ----------------------------------------------------------------------

bool CVertexShaderNode::Load(Iff &iff)
{
	CString old = m_fileName;

	iff.enterForm(TAG_PVSH);
		iff.enterChunk(TAG_0000);
			IffReadString(iff, m_fileName);
		iff.exitChunk(TAG_0000);
	iff.exitForm(TAG_PVSH);


	CTemplateTreeNode::GetInstance()->GetVertexShaderProgramTree().Add(m_fileName, false);
	CTemplateTreeNode::GetInstance()->GetVertexShaderProgramTree().Remove(old, false);
	return true;
}

// ======================================================================

CStageNode::CStageNode(CPassNode &newPass)
: CNode(Stage),
	// ----------
	// Deprecated Data

	m_textureMipFilter("Invalid"),
	m_textureMinificationFilter("Invalid"),
	m_textureMagnificationFilter("Invalid"),
	m_textureAddressW("Invalid"),
	m_textureAddressV("Invalid"),
	m_textureAddressU("Invalid"),

	// ----------

	m_textureCoordinateGeneration("Pass Thru"),
	m_textureCoordinateSetTag(""),
	m_textureTag(""),
	m_resultArgument("Current"),
	m_alphaArgument0("Current"),
	m_alphaArgument2("Current"),
	m_alphaOperation("Modulate"),
	m_alphaArgument1("Texture"),
	m_colorArgument0("Current"),
	m_colorArgument2("Current"),
	m_colorOperation("Modulate"),
	m_colorArgument1("Texture"),
	m_colorArgument1AlphaReplicate(FALSE),
	m_colorArgument1Complement(FALSE),
	m_colorArgument2AlphaReplicate(FALSE),
	m_colorArgument2Complement(FALSE),
	m_colorArgument0AlphaReplicate(FALSE),
	m_colorArgument0Complement(FALSE),
	m_alphaArgument1Complement(FALSE),
	m_alphaArgument2Complement(FALSE),
	m_alphaArgument0Complement(FALSE),
	pass(newPass)
{
	newPass.InsertItem("Stage", *this);
}

// ----------------------------------------------------------------------

CStageNode::CStageNode(CPassNode &newPass, const CStageNode &stage, HTREEITEM after)
: CNode(Stage),
	// ----------
	// Deprecated data

	m_textureMipFilter(stage.m_textureMipFilter),
	m_textureMinificationFilter(stage.m_textureMinificationFilter),
	m_textureMagnificationFilter(stage.m_textureMagnificationFilter),
	m_textureAddressW(stage.m_textureAddressW),
	m_textureAddressV(stage.m_textureAddressV),
	m_textureAddressU(stage.m_textureAddressU),

	// ----------

	m_textureCoordinateGeneration(stage.m_textureCoordinateGeneration),
	m_textureCoordinateSetTag(stage.m_textureCoordinateSetTag),
	m_textureTag(stage.m_textureTag),
	m_resultArgument(stage.m_resultArgument),
	m_alphaArgument0(stage.m_alphaArgument0),
	m_alphaArgument2(stage.m_alphaArgument2),
	m_alphaOperation(stage.m_alphaOperation),
	m_alphaArgument1(stage.m_alphaArgument1),
	m_colorArgument0(stage.m_colorArgument0),
	m_colorArgument2(stage.m_colorArgument2),
	m_colorOperation(stage.m_colorOperation),
	m_colorArgument1(stage.m_colorArgument1),
	m_colorArgument1AlphaReplicate(stage.m_colorArgument1AlphaReplicate),
	m_colorArgument1Complement(stage.m_colorArgument1Complement),
	m_colorArgument2AlphaReplicate(stage.m_colorArgument2AlphaReplicate),
	m_colorArgument2Complement(stage.m_colorArgument2Complement),
	m_colorArgument0AlphaReplicate(stage.m_colorArgument0AlphaReplicate),
	m_colorArgument0Complement(stage.m_colorArgument0Complement),
	m_alphaArgument1Complement(stage.m_alphaArgument1Complement),
	m_alphaArgument2Complement(stage.m_alphaArgument2Complement),
	m_alphaArgument0Complement(stage.m_alphaArgument0Complement),
	pass(newPass)
{
	CTemplateTreeNode::GetInstance()->GetTextureTree().Add(m_textureTag);
	CTemplateTreeNode::GetInstance()->GetTextureCoordinateSetTree().Add(m_textureCoordinateSetTag);

	newPass.InsertItem("Stage", *this, after);
}

// ----------------------------------------------------------------------

CStageNode::~CStageNode()
{
}

// ----------------------------------------------------------------------

bool CStageNode::IsLocked(Whom) const
{
	return pass.IsLocked(Either);
}

// ----------------------------------------------------------------------

CPassNode &CStageNode::GetPass()
{
	return pass;
}

// ----------------------------------------------------------------------

const CPassNode &CStageNode::GetPass() const
{
	return pass;
}

// ----------------------------------------------------------------------

bool CStageNode::Load(Iff &iff)
{
	iff.enterForm(TAG_STAG);
	Tag versionTag = iff.getCurrentName();

	switch (versionTag)
	{
		case TAG_0000:
			if (!Load_0000(iff))
				return false;
			break;

		case TAG_0001:
			if (!Load_0001(iff))
				return false;
			break;
	}

	iff.exitForm(TAG_STAG);

	CTemplateTreeNode::GetInstance()->GetTextureTree().Add(m_textureTag);
	CTemplateTreeNode::GetInstance()->GetTextureCoordinateSetTree().Add(m_textureCoordinateSetTag);

	// ----------
	// Fixup - We've just loaded the stage. If the texture is missing wrap/filter info,
	// fill it with the stage's info.

	CTemplateTreeNode * pTemplate = CTemplateTreeNode::GetInstance();
	CTextureNode * pTexture = pTemplate->GetTextureTree().GetTexture(m_textureTag);

	if(pTexture)
	{
		// ----------
		// Part 1 - Try to use the values from the stage

		if(pTexture->m_filterMip == "Invalid") pTexture->m_filterMip = m_textureMipFilter;
		if(pTexture->m_filterMag == "Invalid") pTexture->m_filterMag = m_textureMagnificationFilter;
		if(pTexture->m_filterMin == "Invalid") pTexture->m_filterMin = m_textureMinificationFilter;

		if(pTexture->m_addressU == "Invalid") pTexture->m_addressU = m_textureAddressU;
		if(pTexture->m_addressV == "Invalid") pTexture->m_addressV = m_textureAddressV;
		if(pTexture->m_addressW == "Invalid") pTexture->m_addressW = m_textureAddressW;

		// ----------
		// Part 2 - If the texture still has invalid wrap/filter modes,
		// which could possibly happen if we have an old texture and a new stage,
		// set them to the defaults.

		if(pTexture->m_filterMip == "Invalid") pTexture->m_filterMip = "Linear";
		if(pTexture->m_filterMin == "Invalid") pTexture->m_filterMin = "Linear";
		if(pTexture->m_filterMag == "Invalid") pTexture->m_filterMag = "Linear";

		if(pTexture->m_addressU == "Invalid") pTexture->m_addressU = "Wrap";
		if(pTexture->m_addressV == "Invalid") pTexture->m_addressV = "Wrap";
		if(pTexture->m_addressW == "Invalid") pTexture->m_addressW = "Wrap";
	}

	// ----------

	return true;
}


bool CStageNode::Load_0000(Iff &iff)
{
	iff.enterChunk(TAG_0000);

		IffReadMap (iff, m_colorOperation,               ms_textureOp);
		IffReadMap (iff, m_colorArgument0,               ms_textureArg);
		IffReadBool(iff, m_colorArgument0Complement);
		IffReadBool(iff, m_colorArgument0AlphaReplicate);

		IffReadMap (iff, m_colorArgument1,               ms_textureArg);
		IffReadBool(iff, m_colorArgument1Complement);
		IffReadBool(iff, m_colorArgument1AlphaReplicate);

		IffReadMap (iff, m_colorArgument2,               ms_textureArg);
		IffReadBool(iff, m_colorArgument2Complement);
		IffReadBool(iff, m_colorArgument2AlphaReplicate);

		IffReadMap (iff, m_alphaOperation,               ms_textureOp);
		IffReadMap (iff, m_alphaArgument0,               ms_textureArg);
		IffReadBool(iff, m_alphaArgument0Complement);

		IffReadMap (iff, m_alphaArgument1,               ms_textureArg);
		IffReadBool(iff, m_alphaArgument1Complement);

		IffReadMap (iff, m_alphaArgument2,               ms_textureArg);
		IffReadBool(iff, m_alphaArgument2Complement);

		IffReadMap (iff, m_resultArgument,               ms_textureArg);

		IffReadTag (iff, m_textureTag);
		IffReadTag (iff, m_textureCoordinateSetTag);

		// ----------
		// Deprecated data

		IffReadMap (iff, m_textureAddressU, ms_textureAddress);
		IffReadMap (iff, m_textureAddressV, ms_textureAddress);
		IffReadMap (iff, m_textureAddressW, ms_textureAddress);
		IffReadMap (iff, m_textureMipFilter, ms_textureFilter);
		IffReadMap (iff, m_textureMinificationFilter, ms_textureFilter);
		IffReadMap (iff, m_textureMagnificationFilter, ms_textureFilter);

		// ----------

		IffReadMap (iff, m_textureCoordinateGeneration, ms_coordinateGeneration);

	iff.exitChunk(TAG_0000);

	return true;
}

// ----------

bool CStageNode::Load_0001(Iff &iff)
{
	iff.enterChunk(TAG_0001);

		IffReadMap (iff, m_colorOperation,               ms_textureOp);
		IffReadMap (iff, m_colorArgument0,               ms_textureArg);
		IffReadBool(iff, m_colorArgument0Complement);
		IffReadBool(iff, m_colorArgument0AlphaReplicate);

		IffReadMap (iff, m_colorArgument1,               ms_textureArg);
		IffReadBool(iff, m_colorArgument1Complement);
		IffReadBool(iff, m_colorArgument1AlphaReplicate);

		IffReadMap (iff, m_colorArgument2,               ms_textureArg);
		IffReadBool(iff, m_colorArgument2Complement);
		IffReadBool(iff, m_colorArgument2AlphaReplicate);

		IffReadMap (iff, m_alphaOperation,               ms_textureOp);
		IffReadMap (iff, m_alphaArgument0,               ms_textureArg);
		IffReadBool(iff, m_alphaArgument0Complement);

		IffReadMap (iff, m_alphaArgument1,               ms_textureArg);
		IffReadBool(iff, m_alphaArgument1Complement);

		IffReadMap (iff, m_alphaArgument2,               ms_textureArg);
		IffReadBool(iff, m_alphaArgument2Complement);

		IffReadMap (iff, m_resultArgument,               ms_textureArg);

		IffReadTag (iff, m_textureTag);
		IffReadTag (iff, m_textureCoordinateSetTag);
		IffReadMap (iff, m_textureCoordinateGeneration, ms_coordinateGeneration);

	iff.exitChunk(TAG_0001);

	return true;
}

// ----------------------------------------------------------------------

bool CStageNode::Save(Iff &iff) const
{
	iff.insertForm(TAG_STAG);
		iff.insertChunk(TAG_0000);

			iff.insertChunkData(ms_textureOp[m_colorOperation]);
			iff.insertChunkData(ms_textureArg[m_colorArgument0]);
			iff.insertChunkData(static_cast<int8>(m_colorArgument0Complement? 1 : 0));
			iff.insertChunkData(static_cast<int8>(m_colorArgument0AlphaReplicate? 1 : 0));
			iff.insertChunkData(ms_textureArg[m_colorArgument1]);
			iff.insertChunkData(static_cast<int8>(m_colorArgument1Complement? 1 : 0));
			iff.insertChunkData(static_cast<int8>(m_colorArgument1AlphaReplicate? 1 : 0));
			iff.insertChunkData(ms_textureArg[m_colorArgument2]);
			iff.insertChunkData(static_cast<int8>(m_colorArgument2Complement? 1 : 0));
			iff.insertChunkData(static_cast<int8>(m_colorArgument2AlphaReplicate? 1 : 0));

			iff.insertChunkData(ms_textureOp[m_alphaOperation]);
			iff.insertChunkData(ms_textureArg[m_alphaArgument0]);
			iff.insertChunkData(static_cast<int8>(m_alphaArgument0Complement? 1 : 0));
			iff.insertChunkData(ms_textureArg[m_alphaArgument1]);
			iff.insertChunkData(static_cast<int8>(m_alphaArgument1Complement? 1 : 0));
			iff.insertChunkData(ms_textureArg[m_alphaArgument2]);
			iff.insertChunkData(static_cast<int8>(m_alphaArgument2Complement? 1 : 0));

			iff.insertChunkData(ms_textureArg[m_resultArgument]);

			iff.insertChunkData(convertToTag(m_textureTag));
			iff.insertChunkData(convertToTag(m_textureCoordinateSetTag));

			// ----------
			// Deprecated data

			iff.insertChunkData(ms_textureAddress[m_textureAddressU]);
			iff.insertChunkData(ms_textureAddress[m_textureAddressV]);
			iff.insertChunkData(ms_textureAddress[m_textureAddressW]);
			iff.insertChunkData(ms_textureFilter[m_textureMipFilter]);
			iff.insertChunkData(ms_textureFilter[m_textureMinificationFilter]);
			iff.insertChunkData(ms_textureFilter[m_textureMagnificationFilter]);

			// ----------

			iff.insertChunkData(ms_coordinateGeneration[m_textureCoordinateGeneration]);

		iff.exitChunk(TAG_0000);
	iff.exitForm(TAG_STAG);

	return true;
}

// ----------------------------------------------------------------------

void CStageNode::Delete()
{
	CTemplateTreeNode::GetInstance()->GetTextureTree().Remove(m_textureTag);
	CTemplateTreeNode::GetInstance()->GetTextureCoordinateSetTree().Remove(m_textureCoordinateSetTag);

	pass.RemoveStage(*this);
	delete this;
}

// ----------------------------------------------------------------------

bool CStageNode::Validate()
{
	//check for no texture factor tag in the parent pass if we use it in this stage
	if(pass.m_textureFactorTag == "")
	{
		if(m_colorArgument1 == "Texture Factor")
		{
			MessageBox(NULL, "Color argument 1 of a stage uses a texture factor, but parent pass does not have a texture factor tag.", "Error", MB_OK | MB_ICONERROR);
			treeCtrl->Select(pass.GetTreeItem(), TVGN_CARET);
			leftView->mainFrame->passView->m_textureFactorTag.SetFocus();
			return false;
		}
		else if(m_colorArgument2 == "Texture Factor")
		{
			MessageBox(NULL, "Color argument 2 of a stage uses a texture factor, but parent pass does not have a texture factor tag.", "Error", MB_OK | MB_ICONERROR);
			treeCtrl->Select(pass.GetTreeItem(), TVGN_CARET);
			leftView->mainFrame->passView->m_textureFactorTag.SetFocus();
			return false;
		}
		else if(m_alphaArgument1 == "Texture Factor")
		{
			MessageBox(NULL, "Alpha argument 1 of a stage uses a texture factor, but parent pass does not have a texture factor tag.", "Error", MB_OK | MB_ICONERROR);
			treeCtrl->Select(pass.GetTreeItem(), TVGN_CARET);
			leftView->mainFrame->passView->m_textureFactorTag.SetFocus();
			return false;
		}
		else if(m_alphaArgument1 == "Texture Factor")
		{
			MessageBox(NULL, "Alpha argument 2 of a stage uses a texture factor, but parent pass does not have a texture factor tag.", "Error", MB_OK | MB_ICONERROR);
			treeCtrl->Select(pass.GetTreeItem(), TVGN_CARET);
			leftView->mainFrame->passView->m_textureFactorTag.SetFocus();
			return false;
		}
	}

	//check for no texture tag if we use textures in this stage
	if(m_textureTag == "")
	{
		if(m_colorArgument1 == "Texture")
		{
			MessageBox(NULL, "Color argument 1 of a stage uses a texture, but does not have a texture tag.", "Error", MB_OK | MB_ICONERROR);
			treeCtrl->Select(treeItem, TVGN_CARET);
			leftView->mainFrame->stageView->m_textureTag.SetFocus();
			return false;
		}
		if(m_colorArgument2 == "Texture")
		{
			MessageBox(NULL, "Color argument 2 of a stage uses a texture, but does not have a texture tag.", "Error", MB_OK | MB_ICONERROR);
			treeCtrl->Select(treeItem, TVGN_CARET);
			leftView->mainFrame->stageView->m_textureTag.SetFocus();
			return false;
		}
		if(m_alphaArgument1 == "Texture")
		{
			MessageBox(NULL, "Alpha argument 1 of a stage uses a texture, but does not have a texture tag.", "Error", MB_OK | MB_ICONERROR);
			treeCtrl->Select(treeItem, TVGN_CARET);
			leftView->mainFrame->stageView->m_textureTag.SetFocus();
			return false;
		}
		if(m_alphaArgument1 == "Texture")
		{
			MessageBox(NULL, "Alpha argument 2 of a stage uses a texture, but does not have a texture tag.", "Error", MB_OK | MB_ICONERROR);
			treeCtrl->Select(treeItem, TVGN_CARET);
			leftView->mainFrame->stageView->m_textureTag.SetFocus();
			return false;
		}
	}

	//check for no texture coordiate set tag if we use textures in this stage
	if(m_textureCoordinateSetTag == "")
	{
		if(m_colorArgument1 == "Texture")
		{
			MessageBox(NULL, "Color argument 1 of a stage uses a texture, but does not have a texture coordinate set tag.", "Error", MB_OK | MB_ICONERROR);
			treeCtrl->Select(treeItem, TVGN_CARET);
			leftView->mainFrame->stageView->m_textureCoordinateSetTag.SetFocus();
			return false;
		}
		if(m_colorArgument2 == "Texture")
		{
			MessageBox(NULL, "Color argument 2 of a stage uses a texture, but does not have a texture coordinate set tag.", "Error", MB_OK | MB_ICONERROR);
			treeCtrl->Select(treeItem, TVGN_CARET);
			leftView->mainFrame->stageView->m_textureCoordinateSetTag.SetFocus();
			return false;
		}
		if(m_alphaArgument1 == "Texture")
		{
			MessageBox(NULL, "Alpha argument 1 of a stage uses a texture, but does not have a texture coordinate set tag.", "Error", MB_OK | MB_ICONERROR);
			treeCtrl->Select(treeItem, TVGN_CARET);
			leftView->mainFrame->stageView->m_textureCoordinateSetTag.SetFocus();
			return false;
		}
		if(m_alphaArgument1 == "Texture")
		{
			MessageBox(NULL, "Alpha argument 2 of a stage uses a texture, but does not have a texture coordinate set tag.", "Error", MB_OK | MB_ICONERROR);
			treeCtrl->Select(treeItem, TVGN_CARET);
			leftView->mainFrame->stageView->m_textureCoordinateSetTag.SetFocus();
			return false;
		}
	}

	return true;
}

// ======================================================================

CMaterialNode::CMaterialNode(const CString &)
: CNode(Material),
	m_power("0"),
	m_emissiveBlue("0.0"),
	m_emissiveGreen("0.0"),
	m_emissiveRed("0.0"),
	m_emissiveAlpha("0.0"),
	m_specularBlue("1.0"),
	m_specularGreen("1.0"),
	m_specularRed("1.0"),
	m_specularAlpha("1.0"),
	m_ambientBlue("1.0"),
	m_ambientGreen("1.0"),
	m_ambientRed("1.0"),
	m_ambientAlpha("1.0"),
	m_diffuseBlue("1.0"),
	m_diffuseGreen("1.0"),
	m_diffuseRed("1.0"),
	m_diffuseAlpha("1.0"),
	referenceCount(0)
{
}

// ----------------------------------------------------------------------

CMaterialNode::~CMaterialNode()
{
}

// ----------------------------------------------------------------------

bool CMaterialNode::Load(Iff &iff)
{
	iff.enterChunk(TAG_MATL);

		IffReadFloat(iff, m_ambientAlpha);
		IffReadFloat(iff, m_ambientRed);
		IffReadFloat(iff, m_ambientGreen);
		IffReadFloat(iff, m_ambientBlue);
		IffReadFloat(iff, m_diffuseAlpha);
		IffReadFloat(iff, m_diffuseRed);
		IffReadFloat(iff, m_diffuseGreen);
		IffReadFloat(iff, m_diffuseBlue);
		IffReadFloat(iff, m_emissiveAlpha);
		IffReadFloat(iff, m_emissiveRed);
		IffReadFloat(iff, m_emissiveGreen);
		IffReadFloat(iff, m_emissiveBlue);
		IffReadFloat(iff, m_specularAlpha);
		IffReadFloat(iff, m_specularRed);
		IffReadFloat(iff, m_specularGreen);
		IffReadFloat(iff, m_specularBlue);
		IffReadFloat(iff, m_power);

	iff.exitChunk(TAG_MATL);

	return true;
}

// ----------------------------------------------------------------------

bool CMaterialNode::Save(Iff &iff) const
{
	iff.insertChunk(TAG_MATL);

		iff.insertChunkData(static_cast<float>(atof(m_ambientAlpha)));
		iff.insertChunkData(static_cast<float>(atof(m_ambientRed)));
		iff.insertChunkData(static_cast<float>(atof(m_ambientGreen)));
		iff.insertChunkData(static_cast<float>(atof(m_ambientBlue)));
		iff.insertChunkData(static_cast<float>(atof(m_diffuseAlpha)));
		iff.insertChunkData(static_cast<float>(atof(m_diffuseRed)));
		iff.insertChunkData(static_cast<float>(atof(m_diffuseGreen)));
		iff.insertChunkData(static_cast<float>(atof(m_diffuseBlue)));
		iff.insertChunkData(static_cast<float>(atof(m_emissiveAlpha)));
		iff.insertChunkData(static_cast<float>(atof(m_emissiveRed)));
		iff.insertChunkData(static_cast<float>(atof(m_emissiveGreen)));
		iff.insertChunkData(static_cast<float>(atof(m_emissiveBlue)));
		iff.insertChunkData(static_cast<float>(atof(m_specularAlpha)));
		iff.insertChunkData(static_cast<float>(atof(m_specularRed)));
		iff.insertChunkData(static_cast<float>(atof(m_specularGreen)));
		iff.insertChunkData(static_cast<float>(atof(m_specularBlue)));
		iff.insertChunkData(static_cast<float>(atof(m_power)));

	iff.exitChunk(TAG_MATL);

	return true;
}

// ----------------------------------------------------------------------

bool CMaterialNode::Validate()
{
	return true;
}

// ======================================================================

CTextureNode::CTextureNode(const CString &)
: CNode(Texture),
	m_placeHolder(FALSE),
	m_textureName(""),
	referenceCount(0),
	m_addressU("Wrap"),
	m_addressV("Wrap"),
	m_addressW("Wrap"),
	m_filterMip("Linear"),
	m_filterMin("Linear"),
	m_filterMag("Linear"),
	m_anisotropy(0)
{
}

// ----------------------------------------------------------------------

CTextureNode::~CTextureNode()
{
}

// ----------------------------------------------------------------------

bool CTextureNode::Load(Iff &iff, CString &name)
{
	iff.enterForm(TAG_TXM);

		Tag versionTag = iff.getCurrentName();

		switch (versionTag)
		{
			case TAG_0000:
				if (!Load_0000(iff,name))
					return false;
				break;

			case TAG_0001:
				if (!Load_0001(iff,name))
					return false;
				break;

			case TAG_0002:
				if (!Load_0002(iff,name))
					return false;
				break;

			default:
				DEBUG_FATAL(true, ("unknown version"));
		}

	iff.exitForm(TAG_TXM);

	return true;
}

// ----------------------------------------------------------------------

bool CTextureNode::Load_0000(Iff & iff, CString & name)
{
	iff.enterForm(TAG_0000);

		iff.enterChunk(TAG_DATA);
			IffReadBool(iff, m_placeHolder);
			IffReadTag (iff, name);

			m_addressU = "Invalid";
			m_addressV = "Invalid";
			m_addressW = "Invalid";
			m_filterMip = "Invalid";
			m_filterMin = "Invalid";
			m_filterMag = "Invalid";

		iff.exitChunk(TAG_DATA);

		if (!m_placeHolder)
		{
			iff.enterChunk(TAG_NAME);
				IffReadString(iff, m_textureName);
			iff.exitChunk(TAG_NAME);
		}

	iff.exitForm(TAG_0000);

	return true;
}

// ----------------------------------------------------------------------

bool CTextureNode::Load_0001(Iff & iff, CString & name)
{
	iff.enterForm(TAG_0001);

		iff.enterChunk(TAG_DATA);
			IffReadTag (iff, name);
			IffReadBool(iff, m_placeHolder);
			IffReadMap (iff, m_addressU,	ms_textureAddress);
			IffReadMap (iff, m_addressV,	ms_textureAddress);
			IffReadMap (iff, m_addressW,	ms_textureAddress);
			IffReadMap (iff, m_filterMip,	ms_textureFilter);
			IffReadMap (iff, m_filterMin,	ms_textureFilter);
			IffReadMap (iff, m_filterMag,	ms_textureFilter);
		iff.exitChunk(TAG_DATA);

		if (!m_placeHolder)
		{
			iff.enterChunk(TAG_NAME);
				IffReadString(iff, m_textureName);
			iff.exitChunk(TAG_NAME);
		}

	iff.exitForm(TAG_0001);

	return true;
}

// ----------------------------------------------------------------------

bool CTextureNode::Load_0002(Iff & iff, CString & name)
{
	iff.enterForm(TAG_0002);

		iff.enterChunk(TAG_DATA);
			IffReadTag (iff, name);
			IffReadBool(iff, m_placeHolder);
			IffReadMap (iff, m_addressU,	ms_textureAddress);
			IffReadMap (iff, m_addressV,	ms_textureAddress);
			IffReadMap (iff, m_addressW,	ms_textureAddress);
			IffReadMap (iff, m_filterMip,	ms_textureFilter);
			IffReadMap (iff, m_filterMin,	ms_textureFilter);
			IffReadMap (iff, m_filterMag,	ms_textureFilter);
			m_anisotropy = iff.read_uint8();

		iff.exitChunk(TAG_DATA);

		if (!m_placeHolder)
		{
			iff.enterChunk(TAG_NAME);
				IffReadString(iff, m_textureName);
			iff.exitChunk(TAG_NAME);
		}

	iff.exitForm(TAG_0002);

	return true;
}

// ----------------------------------------------------------------------

bool CTextureNode::Save(const CString &name, Iff &iff) const
{
	iff.insertForm(TAG_TXM);
		iff.insertForm(TAG_0002);

			iff.insertChunk(TAG_DATA);
				iff.insertChunkData(convertToTag(name));
				iff.insertChunkData(static_cast<int8>(m_placeHolder ? 1 : 0));
				iff.insertChunkData(ms_textureAddress[m_addressU]);
				iff.insertChunkData(ms_textureAddress[m_addressV]);
				iff.insertChunkData(ms_textureAddress[m_addressW]);
				iff.insertChunkData(ms_textureFilter[m_filterMip]);
				iff.insertChunkData(ms_textureFilter[m_filterMin]);
				iff.insertChunkData(ms_textureFilter[m_filterMag]);
				iff.insertChunkData(static_cast<uint8>(m_anisotropy));
			iff.exitChunk(TAG_DATA);

			if (!m_placeHolder)
			{
				iff.insertChunk(TAG_NAME);
					iff.insertChunkString(m_textureName);
				iff.exitChunk(TAG_NAME);
			}

		iff.exitForm(TAG_0002);
	iff.exitForm(TAG_TXM);

	return true;
}

// ----------------------------------------------------------------------

bool CTextureNode::Validate()
{
	if(m_textureName == "" && !m_placeHolder)
	{
		MessageBox(NULL, "Texture field blank, but not marked as a placeholder texture.", "Error", MB_OK | MB_ICONERROR);
		treeCtrl->Select(treeItem, TVGN_CARET);
		leftView->mainFrame->textureView->m_textureName.SetFocus();
		return false;
	}
	return true;
}

// ======================================================================

CTextureCoordinateSetNode::CTextureCoordinateSetNode(const CString &)
: CNode(TextureCoordinateSet),
	m_textureCoordinateSetIndex("0"),
	referenceCount(0)
{
}

// ----------------------------------------------------------------------

CTextureCoordinateSetNode::~CTextureCoordinateSetNode()
{
}

// ----------------------------------------------------------------------

bool CTextureCoordinateSetNode::Load(Iff &iff)
{
	IffReadByte(iff, m_textureCoordinateSetIndex);
	return true;
}

// ----------------------------------------------------------------------

bool CTextureCoordinateSetNode::Save(Iff &iff) const
{
	iff.insertChunkData(static_cast<uint8>(atoi(m_textureCoordinateSetIndex)));
	return true;
}

// ----------------------------------------------------------------------

bool CTextureCoordinateSetNode::Validate()
{
	return true;
}

// ======================================================================

CTextureFactorNode::CTextureFactorNode(const CString &)
: CNode(TextureFactor),
	m_blue("255"),
	m_green("255"),
	m_red("255"),
	m_alpha("255"),
	referenceCount(0)
{
}

// ----------------------------------------------------------------------

CTextureFactorNode::~CTextureFactorNode()
{
}

// ----------------------------------------------------------------------

bool CTextureFactorNode::Load(Iff &iff)
{
	const uint32 value = iff.read_uint32();
	const uint32 alpha = (value >> 24) & 0xff;
	const uint32 red   = (value >> 16) & 0xff;
	const uint32 green = (value >>  8) & 0xff;
	const uint32 blue  = (value >>  0) & 0xff;

	char buffer[8];

	_itoa(alpha, buffer, 10);
	m_alpha = buffer;

	_itoa(red, buffer, 10);
	m_red = buffer;

	_itoa(green, buffer, 10);
	m_green = buffer;

	_itoa(blue, buffer, 10);
	m_blue = buffer;

	return true;
}

// ----------------------------------------------------------------------

bool CTextureFactorNode::Save(Iff &iff) const
{
	const uint32 alpha = atoi(m_alpha);
	const uint32 red   = atoi(m_red);
	const uint32 green = atoi(m_green);
	const uint32 blue  = atoi(m_blue);
	const uint32 value = (alpha << 24) | (red << 16) | (green << 8) | (blue << 0);

	iff.insertChunkData(value);

	return true;
}

// ----------------------------------------------------------------------

bool CTextureFactorNode::Validate()
{
	return true;
}

// ======================================================================

CTextureScrollNode::CTextureScrollNode(const CString &)
: CNode(TextureScroll),
	m_textureScroll1U(""),
	m_textureScroll1V(""),
	m_textureScroll2U(""),
	m_textureScroll2V(""),
	referenceCount(0)
{
}

// ----------------------------------------------------------------------

CTextureScrollNode::~CTextureScrollNode()
{
}

// ----------------------------------------------------------------------

bool CTextureScrollNode::Load(Iff &iff)
{
	IffReadFloat(iff, m_textureScroll1U);
	IffReadFloat(iff, m_textureScroll1V);
	IffReadFloat(iff, m_textureScroll2U);
	IffReadFloat(iff, m_textureScroll2V);

	return true;
}

// ----------------------------------------------------------------------

bool CTextureScrollNode::Save(Iff &iff) const
{
	iff.insertChunkData(static_cast<float>(atof(m_textureScroll1U)));
	iff.insertChunkData(static_cast<float>(atof(m_textureScroll1V)));
	iff.insertChunkData(static_cast<float>(atof(m_textureScroll2U)));
	iff.insertChunkData(static_cast<float>(atof(m_textureScroll2V)));

	return true;
}

// ----------------------------------------------------------------------

bool CTextureScrollNode::Validate()
{
	return true;
}

// ======================================================================

CAlphaReferenceValueNode::CAlphaReferenceValueNode(const CString &)
: CNode(AlphaReferenceValue),
	m_alphaReferenceValue("0"),
	referenceCount(0)
{
}

// ----------------------------------------------------------------------

CAlphaReferenceValueNode::~CAlphaReferenceValueNode()
{
}

// ----------------------------------------------------------------------

bool CAlphaReferenceValueNode::Load(Iff &iff)
{
	IffReadByte(iff, m_alphaReferenceValue);
	return true;
}

// ----------------------------------------------------------------------

bool CAlphaReferenceValueNode::Save(Iff &iff) const
{
	iff.insertChunkData(static_cast<uint8>(atoi(m_alphaReferenceValue)));
	return true;
}

// ----------------------------------------------------------------------

bool CAlphaReferenceValueNode::Validate()
{
	return true;
}

// ======================================================================

CStencilReferenceValueNode::CStencilReferenceValueNode(const CString &)
: CNode(StencilReferenceValue),
	m_stencilReferenceValue("0"),
	referenceCount(0)
{
}

// ----------------------------------------------------------------------

CStencilReferenceValueNode::~CStencilReferenceValueNode()
{
}

// ----------------------------------------------------------------------

bool CStencilReferenceValueNode::Load(Iff &iff)
{
	IffReadInt(iff, m_stencilReferenceValue);
	return true;
}

// ----------------------------------------------------------------------

bool CStencilReferenceValueNode::Save(Iff &iff) const
{
	iff.insertChunkData(static_cast<uint32>(atoi(m_stencilReferenceValue)));
	return true;
}

// ----------------------------------------------------------------------

bool CStencilReferenceValueNode::Validate()
{
	return true;
}

// ======================================================================

CPixelShaderNode::CPixelShaderNode(CPassNode &newPass)
: CNode(PixelShader),
	m_fileName("pixel_program/"),
	m_pass(newPass)
{
	setupTextureArray();
	m_pass.InsertItem("Pixel Shader", *this);
	CTemplateTreeNode::GetInstance()->GetPixelShaderProgramTree().Add(m_fileName, false);
}

// ----------------------------------------------------------------------

CPixelShaderNode::CPixelShaderNode(CPassNode &newPass, const CPixelShaderNode &copyDataFrom)
: CNode(PixelShader),
	m_fileName(copyDataFrom.m_fileName),
	m_t0(copyDataFrom.m_t0),
	m_t1(copyDataFrom.m_t1),
	m_t2(copyDataFrom.m_t2),
	m_t3(copyDataFrom.m_t3),
	m_t4(copyDataFrom.m_t4),
	m_t5(copyDataFrom.m_t5),
	m_t6(copyDataFrom.m_t6),
	m_t7(copyDataFrom.m_t7),
	m_t8(copyDataFrom.m_t8),
	m_t9(copyDataFrom.m_t9),
	m_t10(copyDataFrom.m_t10),
	m_t11(copyDataFrom.m_t11),
	m_t12(copyDataFrom.m_t12),
	m_t13(copyDataFrom.m_t13),
	m_t14(copyDataFrom.m_t14),
	m_t15(copyDataFrom.m_t15),
	m_pass(newPass)
{
	setupTextureArray();

	m_pass.InsertItem("Pixel Shader", *this);

	CTemplateTreeNode::GetInstance()->GetPixelShaderProgramTree().Add(m_fileName, false);

	CTextureTreeNode &textureTreeNode = CTemplateTreeNode::GetInstance()->GetTextureTree();
	for (int i = 0; i < 16; ++i)
		textureTreeNode.Add(*m_textures[i]);
}

// ----------------------------------------------------------------------

CPixelShaderNode::~CPixelShaderNode()
{
}

// ----------------------------------------------------------------------

void CPixelShaderNode::setupTextureArray()
{
	m_textures[0]  = &m_t0;
	m_textures[1]  = &m_t1;
	m_textures[2]  = &m_t2;
	m_textures[3]  = &m_t3;
	m_textures[4]  = &m_t4;
	m_textures[5]  = &m_t5;
	m_textures[6]  = &m_t6;
	m_textures[7]  = &m_t7;
	m_textures[8]  = &m_t8;
	m_textures[9]  = &m_t9;
	m_textures[10] = &m_t10;
	m_textures[11] = &m_t11;
	m_textures[12] = &m_t12;
	m_textures[13] = &m_t13;
	m_textures[14] = &m_t14;
	m_textures[15] = &m_t15;
}

// ----------------------------------------------------------------------

bool CPixelShaderNode::IsLocked(Whom) const
{
	return m_pass.IsLocked(Either);
}

// ----------------------------------------------------------------------

bool CPixelShaderNode::Load(Iff &iff)
{
	iff.enterForm(TAG_PPSH);

		switch (iff.getCurrentName())
		{
			case TAG_0000:
				return false;
				break;

			case TAG_0001:
				if (!Load_0001(iff))
					return false;
				break;

			default:
				return false;
				break;
		}

	iff.exitForm(TAG_PPSH);
	return true;
}

// ----------------------------------------------------------------------

bool CPixelShaderNode::Load_0001(Iff &iff)
{
	CString old = m_fileName;

	iff.enterForm(TAG_0001);

		iff.enterChunk(TAG_DATA);
			const int numberOfTextureSamplers = iff.read_int8();
			IffReadString(iff, m_fileName);
		iff.exitChunk(TAG_DATA);

		for (int i = 0; i < numberOfTextureSamplers; ++i)
		{
			if (!Load_texture(iff))
				return false;
		}

	CTemplateTreeNode::GetInstance()->GetPixelShaderProgramTree().Add(m_fileName, false);
	CTemplateTreeNode::GetInstance()->GetPixelShaderProgramTree().Remove(old, false);

	iff.exitForm(TAG_0001);
	return true;
}

// ----------------------------------------------------------------------

bool CPixelShaderNode::Save(Iff &iff) const
{
	iff.insertForm(TAG_PPSH);
		iff.insertForm(TAG_0001);

			int i = 0;
			int8 count = 0;
			for (i = 0; i < 16; ++i)
				if (!m_textures[i]->IsEmpty())
					++count;

			iff.insertChunk(TAG_DATA);
				iff.insertChunkData(count);
				iff.insertChunkString(m_fileName);
			iff.exitChunk(TAG_DATA);

			for (i = 0; i < 16; ++i)
				if (!m_textures[i]->IsEmpty())
				{
					iff.insertForm(TAG_PTXM);
						iff.insertChunk(TAG_0002);

							iff.insertChunkData(static_cast<int8>(i));
							iff.insertChunkData(convertToTag(*(m_textures[i])));

						iff.exitChunk(TAG_0002);
					iff.exitForm(TAG_PTXM);
				}

		iff.exitForm(TAG_0001);
	iff.exitForm(TAG_PPSH);
	return true;
}

// ----------------------------------------------------------------------

bool CPixelShaderNode::Load_texture(Iff &iff)
{
	iff.enterForm(TAG_PTXM);

		Tag versionTag = iff.getCurrentName();

		switch (versionTag)
		{
			case TAG_0000:
				if (!Load_texture_0000(iff))
					return false;
				break;

			case TAG_0001:
				if (!Load_texture_0001(iff))
					return false;
				break;

			case TAG_0002:
				if (!Load_texture_0002(iff))
					return false;
				break;

			default:
				DEBUG_FATAL(true, ("unknown version\n"));
				break;
		}

	iff.exitForm(TAG_PTXM);

	return true;
}

// ----------------------------------------------------------------------

bool CPixelShaderNode::Load_texture_0000(Iff &iff)
{
	iff.enterChunk(TAG_0000);

		int textureIndex = iff.read_int8();
		IffReadTag (iff, *m_textures[textureIndex]);
		CTemplateTreeNode::GetInstance()->GetTextureTree().Add(*m_textures[textureIndex]);

		IGNORE_RETURN(iff.read_int32()); // old m_textureCoordinateSetTag

		CString	textureAddressU;
		CString	textureAddressV;
		CString	textureAddressW;
		CString	textureMipFilter;
		CString	textureMinificationFilter;
		CString	textureMagnificationFilter;

		IffReadMap (iff, textureAddressU, ms_textureAddress);
		IffReadMap (iff, textureAddressV, ms_textureAddress);
		IffReadMap (iff, textureAddressW, ms_textureAddress);
		IffReadMap (iff, textureMipFilter, ms_textureFilter);
		IffReadMap (iff, textureMinificationFilter, ms_textureFilter);
		IffReadMap (iff, textureMagnificationFilter, ms_textureFilter);

		CTextureNode * textureNode = CTemplateTreeNode::GetInstance()->GetTextureTree().GetTexture(*m_textures[textureIndex]);

		// ----------
		// Part 1 - Try to use the values from the stage

		if(textureNode->m_filterMip == "Invalid") textureNode->m_filterMip = textureMipFilter;
		if(textureNode->m_filterMin == "Invalid") textureNode->m_filterMin = textureMinificationFilter;
		if(textureNode->m_filterMag == "Invalid") textureNode->m_filterMag = textureMagnificationFilter;

		if(textureNode->m_addressU == "Invalid") textureNode->m_addressU = textureAddressU;
		if(textureNode->m_addressV == "Invalid") textureNode->m_addressV = textureAddressV;
		if(textureNode->m_addressW == "Invalid") textureNode->m_addressW = textureAddressW;

		// ----------
		// Part 2 - If the texture still has invalid wrap/filter modes,
		// which could possibly happen if we have an old texture and a new stage,
		// set them to the defaults.

		if(textureNode->m_filterMip == "Invalid") textureNode->m_filterMip = "Linear";;
		if(textureNode->m_filterMin == "Invalid") textureNode->m_filterMin = "Linear";
		if(textureNode->m_filterMag == "Invalid") textureNode->m_filterMag = "Linear";

		if(textureNode->m_addressU == "Invalid") textureNode->m_addressU = "Wrap";
		if(textureNode->m_addressV == "Invalid") textureNode->m_addressV = "Wrap";
		if(textureNode->m_addressW == "Invalid") textureNode->m_addressW = "Wrap";

	iff.exitChunk(TAG_0000, true);

	return true;
}

// ----------------------------------------------------------------------

bool CPixelShaderNode::Load_texture_0001(Iff &iff)
{
	iff.enterChunk(TAG_0001);

		int textureIndex = iff.read_int8();
		IffReadTag (iff, *m_textures[textureIndex]);
		CTemplateTreeNode::GetInstance()->GetTextureTree().Add(*m_textures[textureIndex]);

	iff.exitChunk(TAG_0001, true);

	return true;
}

// ----------------------------------------------------------------------

bool CPixelShaderNode::Load_texture_0002(Iff &iff)
{
	iff.enterChunk(TAG_0002);

		int textureIndex = iff.read_int8();
		IffReadTag (iff, *m_textures[textureIndex]);
		CTemplateTreeNode::GetInstance()->GetTextureTree().Add(*m_textures[textureIndex]);

	iff.exitChunk(TAG_0002);

	return true;
}

// ----------------------------------------------------------------------

bool CPixelShaderNode::Validate()
{
	return true;
}

// ----------------------------------------------------------------------

void CPixelShaderNode::Delete()
{
	CTemplateTreeNode::GetInstance()->GetPixelShaderProgramTree().Remove(m_fileName, false);

	m_pass.RemovePixelShader(*this);

	CTextureTreeNode &textureTreeNode = CTemplateTreeNode::GetInstance()->GetTextureTree();
	for (int i = 0; i < 16; ++i)
		textureTreeNode.Remove(*m_textures[i]);

	delete this;
}

// ----------------------------------------------------------------------

CPassNode &CPixelShaderNode::GetPass()
{
	return m_pass;
}

// ----------------------------------------------------------------------

const CPassNode &CPixelShaderNode::GetPass() const
{
	return m_pass;
}

// ======================================================================

CVertexShaderProgramNode::CVertexShaderProgramNode(const CString &name, bool manual)
: CNode(VertexShaderProgram),
	referenceCount(0),
	m_absoluteFileName(),
	m_text(),
	m_scroll(0),
	m_manuallyLoaded(manual)
{
	Load(name);
}

// ----------------------------------------------------------------------

CVertexShaderProgramNode::~CVertexShaderProgramNode()
{
	// remove all the old tcs tags
	if (!m_manuallyLoaded && CTemplateTreeNode::GetInstance())
	{
		CTextureCoordinateSetTreeNode &treeNode = CTemplateTreeNode::GetInstance()->GetTextureCoordinateSetTree();

		for (uint j = 0; j < m_textureCoordinateSetTags.size(); ++j)
			treeNode.Remove(m_textureCoordinateSetTags[j]);
	}
}

// ----------------------------------------------------------------------

void CVertexShaderProgramNode::Load(const CString &name)
{
	AbstractFile *file = TreeFile::open(name, AbstractFile::PriorityData, true);
	if (file)
	{
		char buffer[Os::MAX_PATH_LENGTH];
		TreeFile::getPathName(name, buffer, sizeof(buffer));
		m_absoluteFileName = buffer;

		int const length = file->length();
		file->read(m_text.GetBufferSetLength(length), length);

		delete file;
	}
}

// ----------------------------------------------------------------------

bool CVertexShaderProgramNode::Save()
{
	return Os::writeFile(m_absoluteFileName, static_cast<char const *>(m_text), m_text.GetLength());
}

// ======================================================================

CPixelShaderProgramNode::CPixelShaderProgramNode(const CString &name, bool manuallyLoaded)
: CNode(PixelShaderProgram),
	referenceCount(0),
	m_absoluteFileName(),
	m_text(),
	m_exeLength(0),
	m_exe(NULL),
	m_compiledIndex(0),
	m_scroll(0),
	m_manuallyLoaded(manuallyLoaded)
{
	Load(name);
}

// ----------------------------------------------------------------------

CPixelShaderProgramNode::~CPixelShaderProgramNode()
{
	delete [] m_exe;
}

// ----------------------------------------------------------------------

void CPixelShaderProgramNode::Load(const CString &name)
{
	Iff iff;

	if (iff.open(name, true))
	{
		char buffer[512];
		TreeFile::getPathName(name, buffer, sizeof(buffer));
		m_absoluteFileName = buffer;

		iff.enterForm(TAG_PSHP);
			iff.enterForm(TAG_0000);

				iff.enterChunk(TAG_PSRC);
					IffReadString(iff, m_text);
				iff.exitChunk(TAG_PSRC);

				iff.enterChunk(TAG_PEXE);
				m_exeLength = iff.getChunkLengthLeft();
				m_exe = new byte[m_exeLength];
				iff.read_uint8(m_exeLength, m_exe);
				m_compiledIndex = ++CPixelShaderNode::ms_compiledIndex;
				iff.exitChunk(TAG_PEXE);

			iff.exitForm(TAG_0000);
		iff.exitForm(TAG_PSHP);
	}
}

// ----------------------------------------------------------------------

bool CPixelShaderProgramNode::Save()
{
	Iff iff(1024);

	iff.insertForm(TAG_PSHP);
		iff.insertForm(TAG_0000);

			iff.insertChunk(TAG_PSRC);
				iff.insertChunkString(m_text);
			iff.exitChunk(TAG_PSRC);

			iff.insertChunk(TAG_PEXE);
				iff.insertChunkData(m_exe, m_exeLength);
			iff.exitChunk(TAG_PEXE);

		iff.exitForm(TAG_0000);
	iff.exitForm(TAG_PSHP);

	return iff.write(m_absoluteFileName, true);
}

// ======================================================================

CVertexShaderProgramTreeNode::CVertexShaderProgramTreeNode()
: CNode(VertexShaderProgramTree)
{
	treeItem = treeCtrl->InsertItem("Vertex Shader Program Tree");
	treeCtrl->SetItemImage(treeItem, 0, 1);
}

// ----------------------------------------------------------------------

CVertexShaderProgramTreeNode::~CVertexShaderProgramTreeNode()
{
}

// ----------------------------------------------------------------------

void CVertexShaderProgramTreeNode::Add(const char *name, bool manual)
{
	ASSERT(name);
	if (name[0])
	{
		// look for it
		CVertexShaderProgramNode *node = static_cast<CVertexShaderProgramNode *>(FindNamedChild(name));

		if (!node)
		{
			// create it if it wasn't found
			node  = new CVertexShaderProgramNode(name, manual);
			InsertItem(name, *node);

			// resort the children to be asthetically pleasing
			treeCtrl->SortChildren(treeItem);
		}
		else
		{
			if (manual && node->m_manuallyLoaded)
				return;
		}

		// another pass refers to this material now
		++node->referenceCount;

		if (manual)
			node->m_manuallyLoaded = true;
	}
}

// ----------------------------------------------------------------------

void CVertexShaderProgramTreeNode::Remove(const char *name, bool manual)
{
	ASSERT(name);
	if (name[0])
	{
		CVertexShaderProgramNode *node = static_cast<CVertexShaderProgramNode *>(FindNamedChild(name));

		ASSERT(node);

		if (manual)
		{
			if (!node->m_manuallyLoaded)
				return;
			if (node->referenceCount > 1)
				node->m_manuallyLoaded = false;
		}

		if (--node->referenceCount == 0)
		{
			RemoveItem(*node);
			delete node;
		}
	}
}

// ----------------------------------------------------------------------

void CVertexShaderProgramTreeNode::CompileAll(CVertexShaderProgramNode * start)
{
	if (!start)
		start = static_cast<CVertexShaderProgramNode *>(GetFirstChild(VertexShaderProgram));

	for (CVertexShaderProgramNode *node = start; node; node = static_cast<CVertexShaderProgramNode *>(GetNextChild(VertexShaderProgram, *node)))
	{
		treeCtrl->Select(node->GetTreeItem(), TVGN_CARET);
		if (!leftView->mainFrame->vertexShaderProgramView->Compile())
			return;
	}
}

// ======================================================================

CPixelShaderProgramTreeNode::CPixelShaderProgramTreeNode()
: CNode(PixelShaderProgramTree)
{
	treeItem = treeCtrl->InsertItem("Pixel Shader Program Tree");
	treeCtrl->SetItemImage(treeItem, 0, 1);
}

// ----------------------------------------------------------------------

CPixelShaderProgramTreeNode::~CPixelShaderProgramTreeNode()
{
}

// ----------------------------------------------------------------------

void CPixelShaderProgramTreeNode::Add(const char *name, bool manual)
{
	ASSERT(name);
	if (name[0])
	{
		// look for it
		CPixelShaderProgramNode *node = static_cast<CPixelShaderProgramNode *>(FindNamedChild(name));

		if (!node)
		{
			// create it if it wasn't found
			node  = new CPixelShaderProgramNode(name, manual);
			InsertItem(name, *node);

			// resort the children to be asthetically pleasing
			treeCtrl->SortChildren(treeItem);
		}
		else
		{
			if (manual && node->m_manuallyLoaded)
				return;
		}

		// another pass refers to this material now
		++node->referenceCount;

		if (manual)
			node->m_manuallyLoaded = true;
	}
}

// ----------------------------------------------------------------------

void CPixelShaderProgramTreeNode::Remove(const char *name, bool manual)
{
	ASSERT(name);
	if (name[0])
	{
		CPixelShaderProgramNode *node = static_cast<CPixelShaderProgramNode *>(FindNamedChild(name));

		ASSERT(node);

		if (manual)
		{
			if (!node->m_manuallyLoaded)
				return;
			if (node->referenceCount > 1)
				node->m_manuallyLoaded = false;
		}

		if (--node->referenceCount == 0)
		{
			RemoveItem(*node);
			delete node;
		}
	}
}

// ----------------------------------------------------------------------

void CPixelShaderProgramTreeNode::CompileAll(CPixelShaderProgramNode *start)
{
	if (!start)
		start = static_cast<CPixelShaderProgramNode *>(GetFirstChild(PixelShaderProgram));

	for (CPixelShaderProgramNode *node = start; node; node = static_cast<CPixelShaderProgramNode *>(GetNextChild(PixelShaderProgram, *node)))
	{
		treeCtrl->Select(node->GetTreeItem(), TVGN_CARET);
		if (!leftView->mainFrame->pixelShaderProgramView->Compile())
			return;
	}
}

// ======================================================================

CIncludeNode::CIncludeNode(const CString &name)
:	CNode(Include),
	referenceCount(0),
	m_absoluteFileName(name),
	m_changed(false),
	m_text(),
	m_scroll(0)
{
	AbstractFile *file = TreeFile::open(name, AbstractFile::PriorityData, true);

	if (file)
	{
		char buffer[512];
		TreeFile::getPathName(name, buffer, sizeof(buffer));
		m_absoluteFileName = buffer;

		const int length = file->length();
		char *text = new char[length + 1];
		file->read(text, file->length());
		text[length] = '\0';
		m_text = text;
		file->close();
		delete file;
		delete [] text;
	}
}

CIncludeNode::~CIncludeNode()
{
}

bool CIncludeNode::Save()
{
	if (m_changed && !Os::writeFile(m_absoluteFileName, static_cast<const char *>(m_text), m_text.GetLength()))
		return false;

	m_changed = false;
	return true;
}

// ======================================================================

CIncludeTreeNode::CIncludeTreeNode()
: CTaggedTreeNode<CIncludeNode>(IncludeTree)
{
	treeItem = treeCtrl->InsertItem("Include Tree");
	treeCtrl->SetItemImage(treeItem, 0, 1);
}

CIncludeTreeNode::~CIncludeTreeNode()
{
}

void CIncludeTreeNode::SaveAll()
{
	for (CIncludeNode *node = static_cast<CIncludeNode *>(GetFirstChild(Include)); node; node = static_cast<CIncludeNode *>(GetNextChild(Include, *node)))
	{
		if (!node->Save())
		{
			treeCtrl->Select(node->GetTreeItem(), TVGN_CARET);
			MessageBox(NULL, "Save failed.", "Error", MB_OK | MB_ICONERROR);
			return;
		}
	}
}

// ======================================================================
