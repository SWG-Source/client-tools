// ======================================================================
// PassView.cpp : implementation file
// ======================================================================

#include "FirstShaderBuilder.h"
#include "ShaderBuilder.h"
#include "PassView.h"

#include "Node.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
    
// ======================================================================

IMPLEMENT_DYNCREATE(CPassView, CFormView)

BEGIN_MESSAGE_MAP(CPassView, CFormView)
	//{{AFX_MSG_MAP(CPassView)
	ON_BN_CLICKED(IDC_PASS_ZENABLE_CHECK, OnZenable)
	ON_BN_CLICKED(IDC_PASS_ZWRITE_CHECK, OnZwrite)
	ON_BN_CLICKED(IDC_PASS_ALPHABLENDENABLE_CHECK, OnAlphablendenable)
	ON_BN_CLICKED(IDC_PASS_ALPHATESTENABLE_CHECK, OnAlphatestenable)
	ON_BN_CLICKED(IDC_PASS_DITHERENABLE_CHECK, OnDitherenable)
	ON_BN_CLICKED(IDC_PASS_HEAT_CHECK, OnHeat)
	ON_BN_CLICKED(IDC_PASS_STENCILENABLE_CHECK, OnStencilenable)
	ON_BN_CLICKED(IDC_PASS_STENCILTWOSIDEDMODE_CHECK, OnStenciltwosidedmode)
	ON_CBN_SELCHANGE(IDC_PASS_ZCOMPARE_COMBO, OnSelchangeZcomparefunction)
	ON_CBN_SELCHANGE(IDC_PASS_ALPHATEST_COMBO, OnSelchangeAlphatestfunction)
	ON_CBN_SELCHANGE(IDC_PASS_ALPHABLENDSOURCE_COMBO, OnSelchangeSourceblend)
	ON_CBN_SELCHANGE(IDC_PASS_ALPHABLENDDEST_COMBO, OnSelchangeDestinationblend)
	ON_CBN_SELCHANGE(IDC_PASS_ALPHABLENDOP_COMBO, OnSelchangeBlendoperation)
	ON_CBN_SELCHANGE(IDC_PASS_STENCILFAIL_COMBO, OnSelchangeStencilfail)
	ON_CBN_SELCHANGE(IDC_PASS_STENCILZFAIL_COMBO, OnSelchangeStencilzfail)
	ON_CBN_SELCHANGE(IDC_PASS_STENCILPASS_COMBO, OnSelchangeStencilpass)
	ON_CBN_SELCHANGE(IDC_PASS_STENCILFUNCTION_COMBO, OnSelchangeStencilfunction)
	ON_CBN_SELCHANGE(IDC_PASS_STENCILCCWFAIL, OnSelchangeStencilCounterclockwisefail)
	ON_CBN_SELCHANGE(IDC_PASS_STENCILCCWZFAIL, OnSelchangeStencilCounterclockwisezfail)
	ON_CBN_SELCHANGE(IDC_PASS_STENCILCCWPASS, OnSelchangeStencilCounterclockwisepass)
	ON_CBN_SELCHANGE(IDC_PASS_STENCILCCWFUNCTION, OnSelchangeStencilCounterclockwisefunction)
	ON_CBN_SELCHANGE(IDC_PASS_SHADEMODE_COMBO, OnSelchangeShademode)
	ON_CBN_SELCHANGE(IDC_PASS_FOGMODE_COMBO, OnSelchangeFogmode)
	ON_EN_CHANGE(IDC_PASS_ALPHATESTTAG_EDIT, OnChangeAlphareferencetag)
	ON_EN_CHANGE(IDC_PASS_STENCILREFERENCETAG_EDIT, OnChangeStencilreferencetag)
	ON_EN_CHANGE(IDC_PASS_STENCILMASK_EDIT, OnChangeStencilmask)
	ON_EN_CHANGE(IDC_PASS_STENCILWRITEMASK_EDIT, OnChangeStencilwritemask)
	ON_EN_CHANGE(IDC_PASS_TEXTUREFACTORTAG_EDIT, OnChangeTexturefactortag)
	ON_EN_CHANGE(IDC_PASS_TEXTUREFACTOR2TAG_EDIT, OnChangeTexturefactor2tag)
	ON_EN_CHANGE(IDC_PASS_LIGHTINGMATERIALTAG_EDIT, OnChangeLightingmaterialtag)
	ON_EN_CHANGE(IDC_PASS_TEXTURESCROLLTAG_EDIT, OnChangeTexturescrolltag)
	ON_EN_KILLFOCUS(IDC_PASS_ALPHATESTTAG_EDIT, OnKillfocusEdit1)
	ON_EN_KILLFOCUS(IDC_PASS_STENCILREFERENCETAG_EDIT, OnKillfocusEdit2)
	ON_EN_KILLFOCUS(IDC_PASS_TEXTUREFACTORTAG_EDIT, OnKillfocusEdit5)
	ON_EN_KILLFOCUS(IDC_PASS_TEXTUREFACTOR2TAG_EDIT, OnKillfocusEdit7)
	ON_EN_KILLFOCUS(IDC_PASS_LIGHTINGMATERIALTAG_EDIT, OnKillfocusEdit6)
	ON_EN_KILLFOCUS(IDC_PASS_TEXTURESCROLLTAG_EDIT, OnKillfocusTexturescrolltag)
	ON_BN_CLICKED(IDC_PASS_WRITEMASKR_CHECK, OnCheckwriteenabler)
	ON_BN_CLICKED(IDC_PASS_WRITEMASKG_CHECK, OnCheckwriteenableg)
	ON_BN_CLICKED(IDC_PASS_WRITEMASKB_CHECK, OnCheckwriteenableb)
	ON_BN_CLICKED(IDC_PASS_WRITEMASKA_CHECK, OnCheckwriteenablea)
	ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnToolTipNotify)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ======================================================================

CPassView::CPassView()
: CFormView(CPassView::IDD),
	passNode(NULL)
{
	//{{AFX_DATA_INIT(CPassView)
	//}}AFX_DATA_INIT

	//fill in the per-item tooltip struct for the combo boxes
	m_blendtypeTooltipItems.push_back("Factor = (0,0,0,0)"); //Zero
	m_blendtypeTooltipItems.push_back("Factor = (1,1,1,1)"); //One
	m_blendtypeTooltipItems.push_back("Factor = (R(s),G(s),B(s),A(s))"); //Source Color
	m_blendtypeTooltipItems.push_back("Factor = (1-R(s),1-G(s),1-B(s),1-A(s))"); //Inverse Source Color
	m_blendtypeTooltipItems.push_back("Factor = (A(s),A(s),A(s),A(s))"); //Source Alpha
	m_blendtypeTooltipItems.push_back("Factor = (1-A(s),1-A(s),1-A(s),1-A(s))"); //Inverse Source Alpha
	m_blendtypeTooltipItems.push_back("Factor = (A(d),A(d),A(d),A(d))"); //Destination Alpha
	m_blendtypeTooltipItems.push_back("Factor = (1-A(d),1-A(d),1-A(d),1-A(d)"); //Inverse Destination Alpha
	m_blendtypeTooltipItems.push_back("Factor = (R(d),G(d),B(d),A(d)"); //Destination Color
	m_blendtypeTooltipItems.push_back("Factor = (1-R(d),1-G(d),1-B(d),1-A(d))"); //Inverse Destination Color
	m_blendtypeTooltipItems.push_back("Factor = (f,f,f,1), f = min(A(s), 1-A(d))"); //Source Alpha Saturate
}

// ----------------------------------------------------------------------

CPassView::~CPassView()
{
	passNode = NULL;
}

// ----------------------------------------------------------------------

void CPassView::OnInitialUpdate() 
{
	CFormView::OnInitialUpdate();
	EnableToolTips(true);   // enable tool tips for view

	//add the per-item tooltip to math-intensive combo boxes
	for(int i = 0; i < m_alphaBlendDestination.GetCount() && i < m_alphaBlendSource.GetCount() && i < static_cast<int>(m_blendtypeTooltipItems.size()); ++i)
	{
		m_alphaBlendDestination.SetItemTip(i, m_blendtypeTooltipItems[i].c_str());
		m_alphaBlendSource.SetItemTip(i, m_blendtypeTooltipItems[i].c_str());
	}

	m_textureFactorTag.SetLimitText(4);
	m_textureFactor2Tag.SetLimitText(4);
	m_stencilReferenceTag.SetLimitText(4);
	m_alphaTestTag.SetLimitText(4);
	m_materialTag.SetLimitText(4);
	m_textureScrollTag.SetLimitText(4);
}

// ----------------------------------------------------------------------

void CPassView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPassView)
	DDX_Control(pDX, IDC_PASS_HEAT_CHECK, m_heat);
	DDX_Control(pDX, IDC_PASS_FOGMODE_COMBO, m_fogMode);
	DDX_Control(pDX, IDC_PASS_WRITEMASKA_CHECK, m_writeMaskA);
	DDX_Control(pDX, IDC_PASS_WRITEMASKB_CHECK, m_writeMaskB);
	DDX_Control(pDX, IDC_PASS_WRITEMASKG_CHECK, m_writeMaskG);
	DDX_Control(pDX, IDC_PASS_WRITEMASKR_CHECK, m_writeMaskR);
	DDX_Control(pDX, IDC_PASS_TEXTUREFACTORTAG_EDIT, m_textureFactorTag);
	DDX_Control(pDX, IDC_PASS_TEXTUREFACTOR2TAG_EDIT, m_textureFactor2Tag);
	DDX_Control(pDX, IDC_PASS_SHADEMODE_COMBO, m_shadeMode);
	DDX_Control(pDX, IDC_PASS_DITHERENABLE_CHECK, m_ditherEnable);
	DDX_Control(pDX, IDC_PASS_STENCILWRITEMASK_EDIT, m_stencilWriteMask);
	DDX_Control(pDX, IDC_PASS_STENCILMASK_EDIT, m_stencilMask);
	DDX_Control(pDX, IDC_PASS_STENCILREFERENCETAG_EDIT, m_stencilReferenceTag);
	DDX_Control(pDX, IDC_PASS_STENCILFUNCTION_COMBO, m_stencilCompareFunction);
	DDX_Control(pDX, IDC_PASS_STENCILPASS_COMBO, m_stencilPassOperation);
	DDX_Control(pDX, IDC_PASS_STENCILZFAIL_COMBO, m_stencilZFailOperation);
	DDX_Control(pDX, IDC_PASS_STENCILFAIL_COMBO, m_stencilFailOperation);
	DDX_Control(pDX, IDC_PASS_STENCILCCWFUNCTION, m_stencilCounterClockwiseCompareFunction);
	DDX_Control(pDX, IDC_PASS_STENCILCCWPASS, m_stencilCounterClockwisePassOperation);
	DDX_Control(pDX, IDC_PASS_STENCILCCWZFAIL, m_stencilCounterClockwiseZFailOperation);
	DDX_Control(pDX, IDC_PASS_STENCILCCWFAIL, m_stencilCounterClockwiseFailOperation);
	DDX_Control(pDX, IDC_PASS_STENCILENABLE_CHECK, m_stencilEnable);
	DDX_Control(pDX, IDC_PASS_STENCILTWOSIDEDMODE_CHECK, m_stencilTwoSidedMode);
	DDX_Control(pDX, IDC_PASS_ALPHATEST_COMBO, m_alphaTestFunction);
	DDX_Control(pDX, IDC_PASS_ALPHATESTTAG_EDIT, m_alphaTestTag);
	DDX_Control(pDX, IDC_PASS_ALPHATESTENABLE_CHECK, m_alphaTestEnable);
	DDX_Control(pDX, IDC_PASS_ALPHABLENDOP_COMBO, m_alphaBlendOperation);
	DDX_Control(pDX, IDC_PASS_ALPHABLENDDEST_COMBO, m_alphaBlendDestination);
	DDX_Control(pDX, IDC_PASS_ALPHABLENDSOURCE_COMBO, m_alphaBlendSource);
	DDX_Control(pDX, IDC_PASS_ALPHABLENDENABLE_CHECK, m_alphaBlendEnable);
	DDX_Control(pDX, IDC_PASS_ZCOMPARE_COMBO, m_zCompare);
	DDX_Control(pDX, IDC_PASS_ZWRITE_CHECK, m_zWrite);
	DDX_Control(pDX, IDC_PASS_ZENABLE_CHECK, m_zEnable);
	DDX_Control(pDX, IDC_PASS_LIGHTINGMATERIALTAG_EDIT, m_materialTag);
	DDX_Control(pDX, IDC_PASS_TEXTURESCROLLTAG_EDIT, m_textureScrollTag);
	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

#ifdef _DEBUG
void CPassView::AssertValid() const
{
	CFormView::AssertValid();
}

// ----------------------------------------------------------------------

void CPassView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

// ----------------------------------------------------------------------

void CPassView::SetNode(CPassNode &node)
{
	passNode = &node;

#define SET(a) a.SetCheck(node.a)

	SET(m_ditherEnable);
	SET(m_heat);
//	SET(m_lightingColorVertex);
//	SET(m_lightingSpecularEnable);
//	SET(m_lighting);
	SET(m_stencilEnable);
	SET(m_stencilTwoSidedMode);
	SET(m_alphaTestEnable);
	SET(m_alphaBlendEnable);
	SET(m_zWrite);
	SET(m_zEnable);
	SET(m_writeMaskA);
	SET(m_writeMaskR);
	SET(m_writeMaskG);
	SET(m_writeMaskB);

#undef SET
#define SET(a) do { if (node.a == "") a.SetCurSel(-1); else a.SelectString(-1, node.a); } while (0)

	SET(m_shadeMode);
	SET(m_fogMode);
//	SET(m_lightingEmissiveColorSource);
//	SET(m_lightingSpecularColorSource);
//	SET(m_lightingDiffuseColorSource);
//	SET(m_lightingAmbientColorSource);
	SET(m_stencilCompareFunction);
	SET(m_stencilPassOperation);
	SET(m_stencilZFailOperation);
	SET(m_stencilFailOperation);
	SET(m_stencilCounterClockwiseCompareFunction);
	SET(m_stencilCounterClockwisePassOperation);
	SET(m_stencilCounterClockwiseZFailOperation);
	SET(m_stencilCounterClockwiseFailOperation);
	SET(m_alphaTestFunction);
	SET(m_alphaBlendOperation);
	SET(m_alphaBlendDestination);
	SET(m_alphaBlendSource);
	SET(m_zCompare);

#undef SET
#define SET(a) a.SetWindowText(node.a)

	SET(m_textureFactorTag);
	SET(m_textureFactor2Tag);
	SET(m_stencilWriteMask);
	SET(m_stencilMask);
	SET(m_stencilReferenceTag);
	SET(m_alphaTestTag);
	SET(m_materialTag);
	SET(m_textureScrollTag);

#undef SET

	if (passNode->IsLocked(CNode::Either))	
		EnableWindow(FALSE);
	else
		EnableWindow(TRUE);
}

// ----------------------------------------------------------------------

void CPassView::OnZenable() 
{
	passNode->m_zEnable = m_zEnable.GetCheck();	
}

// ----------------------------------------------------------------------

void CPassView::OnZwrite() 
{
	passNode->m_zWrite = m_zWrite.GetCheck();
}

// ----------------------------------------------------------------------

void CPassView::OnAlphablendenable() 
{
	passNode->m_alphaBlendEnable = m_alphaBlendEnable.GetCheck();
}

// ----------------------------------------------------------------------

void CPassView::OnAlphatestenable() 
{
	passNode->m_alphaTestEnable = m_alphaTestEnable.GetCheck();
}

// ----------------------------------------------------------------------

void CPassView::OnDitherenable() 
{
	passNode->m_ditherEnable = m_ditherEnable.GetCheck();
}

// ----------------------------------------------------------------------

void CPassView::OnHeat() 
{	
	passNode->m_heat = m_heat.GetCheck();
}

// ----------------------------------------------------------------------

void CPassView::OnStencilenable() 
{
	passNode->m_stencilEnable = m_stencilEnable.GetCheck();
}

// ----------------------------------------------------------------------

void CPassView::OnStenciltwosidedmode() 
{
	passNode->m_stencilTwoSidedMode = m_stencilTwoSidedMode.GetCheck();
}

// ----------------------------------------------------------------------

void CPassView::OnSelchangeZcomparefunction() 
{
	m_zCompare.GetWindowText(passNode->m_zCompare);
}

// ----------------------------------------------------------------------

void CPassView::OnSelchangeSourceblend() 
{
	m_alphaBlendSource.GetWindowText(passNode->m_alphaBlendSource);
}

// ----------------------------------------------------------------------

void CPassView::OnSelchangeDestinationblend() 
{
	m_alphaBlendDestination.GetWindowText(passNode->m_alphaBlendDestination);
}

// ----------------------------------------------------------------------

void CPassView::OnSelchangeBlendoperation() 
{
	m_alphaBlendOperation.GetWindowText(passNode->m_alphaBlendOperation);
}

// ----------------------------------------------------------------------

void CPassView::OnSelchangeAlphatestfunction() 
{
	m_alphaTestFunction.GetWindowText(passNode->m_alphaTestFunction);
}

// ----------------------------------------------------------------------

void CPassView::OnSelchangeShademode() 
{
	m_shadeMode.GetWindowText(passNode->m_shadeMode);
}

// ----------------------------------------------------------------------

void CPassView::OnSelchangeStencilfail() 
{
	m_stencilFailOperation.GetWindowText(passNode->m_stencilFailOperation);
}

// ----------------------------------------------------------------------

void CPassView::OnSelchangeStencilzfail() 
{
	m_stencilZFailOperation.GetWindowText(passNode->m_stencilZFailOperation);
}

// ----------------------------------------------------------------------

void CPassView::OnSelchangeStencilpass() 
{
	m_stencilPassOperation.GetWindowText(passNode->m_stencilPassOperation);
}

// ----------------------------------------------------------------------

void CPassView::OnSelchangeStencilfunction() 
{
	m_stencilCompareFunction.GetWindowText(passNode->m_stencilCompareFunction);
}

// ----------------------------------------------------------------------

void CPassView::OnSelchangeStencilCounterclockwisefail() 
{
	m_stencilCounterClockwiseFailOperation.GetWindowText(passNode->m_stencilCounterClockwiseFailOperation);
}

// ----------------------------------------------------------------------

void CPassView::OnSelchangeStencilCounterclockwisezfail() 
{
	m_stencilCounterClockwiseZFailOperation.GetWindowText(passNode->m_stencilCounterClockwiseZFailOperation);
}

// ----------------------------------------------------------------------

void CPassView::OnSelchangeStencilCounterclockwisepass() 
{
	m_stencilCounterClockwisePassOperation.GetWindowText(passNode->m_stencilCounterClockwisePassOperation);
}

// ----------------------------------------------------------------------

void CPassView::OnSelchangeStencilCounterclockwisefunction() 
{
	m_stencilCounterClockwiseCompareFunction.GetWindowText(passNode->m_stencilCounterClockwiseCompareFunction);
}

// ----------------------------------------------------------------------

void CPassView::OnChangeAlphareferencetag() 
{
	CString s;
	m_alphaTestTag.GetWindowText(s);
	UpdateTaggedTree(CTemplateTreeNode::GetInstance()->GetAlphaReferenceValueTree(), passNode->m_alphaTestTag, s);
}

// ----------------------------------------------------------------------

void CPassView::OnChangeTexturefactortag() 
{
	CString s;
	m_textureFactorTag.GetWindowText(s);
	UpdateTaggedTree(CTemplateTreeNode::GetInstance()->GetTextureFactorTree(), passNode->m_textureFactorTag, s);
}

// ----------------------------------------------------------------------

void CPassView::OnChangeTexturefactor2tag() 
{
	CString s;
	m_textureFactor2Tag.GetWindowText(s);
	UpdateTaggedTree(CTemplateTreeNode::GetInstance()->GetTextureFactorTree(), passNode->m_textureFactor2Tag, s);
}

// ----------------------------------------------------------------------

void CPassView::OnChangeStencilreferencetag() 
{
	CString s;
	m_stencilReferenceTag.GetWindowText(s);
	UpdateTaggedTree(CTemplateTreeNode::GetInstance()->GetStencilReferenceValueTree(), passNode->m_stencilReferenceTag, s);
}

// ----------------------------------------------------------------------

void CPassView::OnChangeStencilmask() 
{
	m_stencilMask.GetWindowText(passNode->m_stencilMask);
}

// ----------------------------------------------------------------------

void CPassView::OnChangeStencilwritemask() 
{
	m_stencilWriteMask.GetWindowText(passNode->m_stencilWriteMask);
}

// ----------------------------------------------------------------------

void CPassView::OnKillfocusEdit1() 
{
	Pad4(m_alphaTestTag);
}

// ----------------------------------------------------------------------

void CPassView::OnKillfocusEdit2() 
{
	Pad4(m_stencilReferenceTag);
}

// ----------------------------------------------------------------------

void CPassView::OnKillfocusEdit5() 
{
	Pad4(m_textureFactorTag);
}

// ----------------------------------------------------------------------

void CPassView::OnKillfocusEdit7() 
{
	Pad4(m_textureFactor2Tag);
}

// ----------------------------------------------------------------------

void CPassView::OnCheckwriteenabler() 
{
	passNode->m_writeMaskR = m_writeMaskR.GetCheck();	
}

// ----------------------------------------------------------------------

void CPassView::OnCheckwriteenableg() 
{
	passNode->m_writeMaskG = m_writeMaskG.GetCheck();	
}

// ----------------------------------------------------------------------

void CPassView::OnCheckwriteenableb() 
{
	passNode->m_writeMaskB = m_writeMaskB.GetCheck();	
}

// ----------------------------------------------------------------------

void CPassView::OnCheckwriteenablea() 
{
	passNode->m_writeMaskA = m_writeMaskA.GetCheck();	
}

// ----------------------------------------------------------------------

void CPassView::OnSelchangeFogmode() 
{
	m_fogMode.GetWindowText(passNode->m_fogMode);
}

// ----------------------------------------------------------------------

BOOL CPassView::OnToolTipNotify(UINT id, NMHDR *pNMHDR, LRESULT *pResult)
{
	UNREF(pResult);
	UNREF(id);
	TOOLTIPTEXT *pTTT = reinterpret_cast<TOOLTIPTEXT *>(pNMHDR);
	UINT nID = pNMHDR->idFrom;
	if (pTTT->uFlags & TTF_IDISHWND)
	{
		// idFrom is actually the HWND of the tool
		nID = ::GetDlgCtrlID((HWND)nID);
		if(nID)
		{
			pTTT->lpszText = MAKEINTRESOURCE(nID);
			pTTT->hinst = AfxGetResourceHandle();
			return TRUE;
		}
	}
	return FALSE;
}

void CPassView::OnKillfocusEdit6() 
{
	Pad4(m_materialTag);
}

void CPassView::OnChangeLightingmaterialtag() 
{
	CString s;
	m_materialTag.GetWindowText(s);
	UpdateTaggedTree(CTemplateTreeNode::GetInstance()->GetMaterialTree(), passNode->m_materialTag, s);
}

void CPassView::OnKillfocusTexturescrolltag() 
{
	Pad4(m_textureScrollTag);
}

void CPassView::OnChangeTexturescrolltag() 
{
	CString s;
	m_textureScrollTag.GetWindowText(s);
	UpdateTaggedTree(CTemplateTreeNode::GetInstance()->GetTextureScrollTree(), passNode->m_textureScrollTag, s);
}

// ======================================================================
