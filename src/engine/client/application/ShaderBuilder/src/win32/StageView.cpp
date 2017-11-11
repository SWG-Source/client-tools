// ======================================================================
// StageView.cpp : implementation file
// ======================================================================

#include "FirstShaderBuilder.h"
#include "ShaderBuilder.h"
#include "StageView.h"

#include "Node.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// ======================================================================

IMPLEMENT_DYNCREATE(CStageView, CFormView)

BEGIN_MESSAGE_MAP(CStageView, CFormView)
	//{{AFX_MSG_MAP(CStageView)
	ON_BN_CLICKED(IDC_STAGE_COLORARGUMENT0COMPLEMENT_CHECK, OnColorargument0complement)
	ON_BN_CLICKED(IDC_STAGE_COLORARGUMENT1COMPLEMENT_CHECK, OnColorargument1complement)
	ON_BN_CLICKED(IDC_STAGE_COLORARGUMENT2COMPLEMENT_CHECK, OnColorargument2complement)
	ON_BN_CLICKED(IDC_STAGE_ALPHAARGUMENT0COMPLEMENT_CHECK, OnAlphaargument0complement)
	ON_BN_CLICKED(IDC_STAGE_ALPHAARGUMENT1COMPLEMENT_CHECK, OnAlphaargument1complement)
	ON_BN_CLICKED(IDC_STAGE_ALPHAARGUMENT2COMPLEMENT_CHECK, OnAlphaargument2complement)
	ON_BN_CLICKED(IDC_STAGE_COLORARGUMENT0ALPHAREPLICATE_CHECK, OnColorargument0alphareplicate)
	ON_BN_CLICKED(IDC_STAGE_COLORARGUMENT1ALPHAREPLICATE_CHECK, OnColorargument1alphareplicate)
	ON_BN_CLICKED(IDC_STAGE_COLORARGUMENT2ALPHAREPLICATE_CHECK, OnColorargument2alphareplicate)
	ON_EN_CHANGE(IDC_STAGE_TEXTURECOORDINATESETTAG_EDIT, OnChangeTexturecoordinatesettag)
	ON_EN_CHANGE(IDC_STAGE_TEXTURETAG_EDIT, OnChangeTexturetag)
	ON_EN_KILLFOCUS(IDC_STAGE_TEXTURECOORDINATESETTAG_EDIT, OnKillfocusEdit3)
	ON_EN_KILLFOCUS(IDC_STAGE_TEXTURETAG_EDIT, OnKillfocusEdit4)
	ON_CBN_SELCHANGE(IDC_STAGE_COLORARGUMENT1_COMBO, OnSelchangeColorargument1)
	ON_CBN_SELCHANGE(IDC_STAGE_COLORARGUMENT2_COMBO, OnSelchangeColorargument2)
	ON_CBN_SELCHANGE(IDC_STAGE_ALPHAARGUMENT2_COMBO, OnSelchangeAlphaargument2)
	ON_CBN_SELCHANGE(IDC_STAGE_TEXTURECOORDINATEGENERATION_COMBO, OnSelchangeTexturecoordinategeneration)
	ON_CBN_SELCHANGE(IDC_STAGE_RESULTARG_COMBO, OnSelchangeResultargument)
	ON_CBN_SELCHANGE(IDC_STAGE_COLOROP_COMBO, OnSelchangeColoroperation)
	ON_CBN_SELCHANGE(IDC_STAGE_COLORARGUMENT0_COMBO, OnSelchangeColorargument0)
	ON_CBN_SELCHANGE(IDC_STAGE_ALPHAARGUMENT1_COMBO, OnSelchangeAlphaargument1)
	ON_CBN_SELCHANGE(IDC_STAGE_ALPHAOP_COMBO, OnSelchangeAlphaoperation)
	ON_CBN_SELCHANGE(IDC_STAGE_ALPHAARGUMENT0_COMBO, OnSelchangeAlphaargument0)
	ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnToolTipNotify)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ======================================================================

CStageView::CStageView()
: CFormView(CStageView::IDD),
	stageNode(NULL)
{
	//{{AFX_DATA_INIT(CStageView)
	//}}AFX_DATA_INIT

	//fill in the per-item tooltip struct for the combo boxes
	m_operationTooltipText.push_back("S = Arg1"); //Select Argument 1
	m_operationTooltipText.push_back("S = Arg2"); //Select Argument 2
	m_operationTooltipText.push_back("S = Arg1 * Arg2"); //Modulate
	m_operationTooltipText.push_back("S = (Arg1 * Arg2) << 1"); //Modulate 2x
	m_operationTooltipText.push_back("S = (Arg1 * Arg2) << 2"); //Modulate 4x
	m_operationTooltipText.push_back("S = Arg1 + Arg2"); //Add
	m_operationTooltipText.push_back("S = Arg1 + Arg2 - 0.5"); //Add Signed
	m_operationTooltipText.push_back("S = (Arg1 + Arg2 - 0.5) << 1"); //Add Signed 2x
	m_operationTooltipText.push_back("S = Arg1 - Arg2"); //Subtract
	m_operationTooltipText.push_back("S = Arg1 + Arg2 - Arg1 * Arg2"); //Add Smooth
	m_operationTooltipText.push_back("S = Arg1 * (Alpha) + Arg2 * (1 - Alpha)"); //Blend Diffuse Alpha
	m_operationTooltipText.push_back("S = Arg1 * (Alpha) + Arg2 * (1 - Alpha)"); //Blend Texture Alpha
	m_operationTooltipText.push_back("S = Arg1 * (Alpha) + Arg2 * (1 - Alpha)"); //Blend Factor Alpha
	m_operationTooltipText.push_back("S = Arg1 + Arg2 * (1 - Alpha)"); //Blend Texture Alpha Premodulate
	m_operationTooltipText.push_back("S = Arg1 * (Alpha) + Arg2 * (1 - Alpha)"); //Blend Current Alpha
	m_operationTooltipText.push_back("Modulate the texture with the next texture stage"); //Premodulate
	m_operationTooltipText.push_back("S = Arg1(rgb) + Arg1(a) * Arg2(rgb)"); //Modulate alpha add color
	m_operationTooltipText.push_back("S = Arg1(rgb) * Arg2(rgb) + Arg1(a)"); //Modulate color add alpha
	m_operationTooltipText.push_back("S = (1 - Arg1(a)) * Arg2(rgb) + Arg1(rgb)"); //Modulate inverse alpha add color
	m_operationTooltipText.push_back("S = (1 - Arg1(rgb)) * ARg2(rgb) + Arg1(a)"); //Modulate inverse color add alpha
	m_operationTooltipText.push_back("Perform per-pixel bump mapping, without luminance"); //Bump Environment Map
	m_operationTooltipText.push_back("Perform per-pixel bump mapping, with luminance"); //Bump Environment Map Luminance
	m_operationTooltipText.push_back("S = Arg1(r) * Arg2(r) + Arg1(g) * Arg2(g) + Arg1(b) * Arg2(b)"); //Dot Product 3
	m_operationTooltipText.push_back("S = Arg1 + Arg2 * Arg0"); //Multiply Add
	m_operationTooltipText.push_back("S = Arg1 * Arg2 + (1 - Arg1) * Arg0"); //Lerp
}

// ----------------------------------------------------------------------

CStageView::~CStageView()
{
	stageNode = NULL;
}

// ----------------------------------------------------------------------

void CStageView::OnInitialUpdate() 
{
	CFormView::OnInitialUpdate();
	m_textureCoordinateSetTag.SetLimitText(4);
	m_textureTag.SetLimitText(4);
	EnableToolTips(true);   // enable tool tips for view
	
	//add the per-item tooltip to math-intensive combo boxes
	for(int i = 0; i < m_colorOperation.GetCount() && i < m_alphaOperation.GetCount() && i < static_cast<int>(m_operationTooltipText.size()); ++i)
	{
		m_colorOperation.SetItemTip(i, m_operationTooltipText[i].c_str());
		m_alphaOperation.SetItemTip(i, m_operationTooltipText[i].c_str());
	}
}

// ----------------------------------------------------------------------

void CStageView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CStageView)
	DDX_Control(pDX, IDC_STAGE_COLORARGUMENT0ALPHAREPLICATE_CHECK, m_colorArgument0AlphaReplicate);
	DDX_Control(pDX, IDC_STAGE_COLORARGUMENT2ALPHAREPLICATE_CHECK, m_colorArgument2AlphaReplicate);
	DDX_Control(pDX, IDC_STAGE_COLORARGUMENT1ALPHAREPLICATE_CHECK, m_colorArgument1AlphaReplicate);
	DDX_Control(pDX, IDC_STAGE_COLORARGUMENT0COMPLEMENT_CHECK, m_colorArgument0Complement);
	DDX_Control(pDX, IDC_STAGE_COLORARGUMENT2COMPLEMENT_CHECK, m_colorArgument2Complement);
	DDX_Control(pDX, IDC_STAGE_ALPHAARGUMENT0COMPLEMENT_CHECK, m_alphaArgument0Complement);
	DDX_Control(pDX, IDC_STAGE_ALPHAARGUMENT2COMPLEMENT_CHECK, m_alphaArgument2Complement);
	DDX_Control(pDX, IDC_STAGE_ALPHAARGUMENT1COMPLEMENT_CHECK, m_alphaArgument1Complement);
	DDX_Control(pDX, IDC_STAGE_COLORARGUMENT1COMPLEMENT_CHECK, m_colorArgument1Complement);
	DDX_Control(pDX, IDC_STAGE_TEXTURECOORDINATESETTAG_EDIT, m_textureCoordinateSetTag);
	DDX_Control(pDX, IDC_STAGE_TEXTURETAG_EDIT, m_textureTag);
	DDX_Control(pDX, IDC_STAGE_TEXTURECOORDINATEGENERATION_COMBO, m_textureCoordinateGeneration);
	DDX_Control(pDX, IDC_STAGE_RESULTARG_COMBO, m_resultArgument);
	DDX_Control(pDX, IDC_STAGE_ALPHAARGUMENT0_COMBO, m_alphaArgument0);
	DDX_Control(pDX, IDC_STAGE_ALPHAARGUMENT2_COMBO, m_alphaArgument2);
	DDX_Control(pDX, IDC_STAGE_ALPHAOP_COMBO, m_alphaOperation);
	DDX_Control(pDX, IDC_STAGE_ALPHAARGUMENT1_COMBO, m_alphaArgument1);
	DDX_Control(pDX, IDC_STAGE_COLORARGUMENT0_COMBO, m_colorArgument0);
	DDX_Control(pDX, IDC_STAGE_COLORARGUMENT2_COMBO, m_colorArgument2);
	DDX_Control(pDX, IDC_STAGE_COLOROP_COMBO, m_colorOperation);
	DDX_Control(pDX, IDC_STAGE_COLORARGUMENT1_COMBO, m_colorArgument1);
	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

#ifdef _DEBUG
void CStageView::AssertValid() const
{
	CFormView::AssertValid();
}

// ----------------------------------------------------------------------

void CStageView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

// ----------------------------------------------------------------------

static void stop()
{
	__asm int 3;
}

// Set a combo box's value to the string in the node.
#define SET(a) \
do \
{ \
	if (node.a == "") \
		a.SetCurSel(-1); \
	else \
		a.SelectString(-1, node.a); \
	CString value; \
	a.GetWindowText(value); \
	if (node.a != value) \
		stop(); \
} while (0)
	
// ----------

void CStageView::SetNode(CStageNode &node)
{
	stageNode = &node;

	SET(m_textureCoordinateGeneration);
	SET(m_resultArgument);
	SET(m_alphaArgument0);
	SET(m_alphaArgument2);
	SET(m_alphaOperation);
	SET(m_alphaArgument1);
	SET(m_colorArgument0);
	SET(m_colorArgument2);
	SET(m_colorOperation);
	SET(m_colorArgument1);

#undef SET
#define SET(a) a.SetWindowText(node.a)

	SET(m_textureCoordinateSetTag);
	SET(m_textureTag);

#undef SET
#define SET(a) a.SetCheck(node.a ? 1 : 0)

	SET(m_colorArgument1AlphaReplicate);
	SET(m_colorArgument1Complement);
	SET(m_colorArgument2AlphaReplicate);
	SET(m_colorArgument2Complement);
	SET(m_colorArgument0AlphaReplicate);
	SET(m_colorArgument0Complement);
	SET(m_alphaArgument1Complement);
	SET(m_alphaArgument2Complement);
	SET(m_alphaArgument0Complement);

#undef SET

	if (stageNode->IsLocked(CNode::Either))	
		EnableWindow(FALSE);
	else
		EnableWindow(TRUE);
}

// ----------------------------------------------------------------------

void CStageView::OnSelchangeColorargument1()
{
	m_colorArgument1.GetWindowText(stageNode->m_colorArgument1);
}

// ----------------------------------------------------------------------

void CStageView::OnSelchangeColoroperation()
{
	m_colorOperation.GetWindowText(stageNode->m_colorOperation);
}

// ----------------------------------------------------------------------

void CStageView::OnSelchangeColorargument2()
{
	m_colorArgument2.GetWindowText(stageNode->m_colorArgument2);
}

// ----------------------------------------------------------------------

void CStageView::OnSelchangeColorargument0()
{
	m_colorArgument0.GetWindowText(stageNode->m_colorArgument0);
}

// ----------------------------------------------------------------------

void CStageView::OnSelchangeAlphaargument1()
{
	m_alphaArgument1.GetWindowText(stageNode->m_alphaArgument1);
}

// ----------------------------------------------------------------------

void CStageView::OnSelchangeAlphaoperation()
{
	m_alphaOperation.GetWindowText(stageNode->m_alphaOperation);
}

// ----------------------------------------------------------------------

void CStageView::OnSelchangeAlphaargument2()
{
	m_alphaArgument2.GetWindowText(stageNode->m_alphaArgument2);
}

// ----------------------------------------------------------------------

void CStageView::OnSelchangeAlphaargument0()
{
	m_alphaArgument0.GetWindowText(stageNode->m_alphaArgument0);
}

// ----------------------------------------------------------------------

void CStageView::OnSelchangeResultargument()
{
	m_resultArgument.GetWindowText(stageNode->m_resultArgument);
}

// ----------------------------------------------------------------------

void CStageView::OnSelchangeTexturecoordinategeneration()
{
	m_textureCoordinateGeneration.GetWindowText(stageNode->m_textureCoordinateGeneration);
}

// ----------------------------------------------------------------------

void CStageView::OnChangeTexturetag() 
{
	CString s;
	m_textureTag.GetWindowText(s);
	UpdateTaggedTree(CTemplateTreeNode::GetInstance()->GetTextureTree(), stageNode->m_textureTag, s);	
}

// ----------------------------------------------------------------------

void CStageView::OnChangeTexturecoordinatesettag() 
{
	CString s;
	m_textureCoordinateSetTag.GetWindowText(s);
	UpdateTaggedTree(CTemplateTreeNode::GetInstance()->GetTextureCoordinateSetTree(), stageNode->m_textureCoordinateSetTag, s);	
}

// ----------------------------------------------------------------------

void CStageView::OnKillfocusEdit3() 
{
	Pad4(m_textureCoordinateSetTag);
}

// ----------------------------------------------------------------------

void CStageView::OnKillfocusEdit4() 
{
	Pad4(m_textureTag);
}

// ----------------------------------------------------------------------

void CStageView::OnColorargument1complement() 
{
	stageNode->m_colorArgument1Complement = m_colorArgument1Complement.GetCheck();
}

// ----------------------------------------------------------------------

void CStageView::OnColorargument2complement() 
{
	stageNode->m_colorArgument2Complement = m_colorArgument2Complement.GetCheck();
}

// ----------------------------------------------------------------------

void CStageView::OnColorargument0complement() 
{
	stageNode->m_colorArgument0Complement = m_colorArgument0Complement.GetCheck();
}

// ----------------------------------------------------------------------

void CStageView::OnAlphaargument1complement() 
{
	stageNode->m_alphaArgument1Complement = m_alphaArgument1Complement.GetCheck();
}

// ----------------------------------------------------------------------

void CStageView::OnAlphaargument2complement() 
{
	stageNode->m_alphaArgument2Complement = m_alphaArgument2Complement.GetCheck();
}

// ----------------------------------------------------------------------

void CStageView::OnAlphaargument0complement() 
{
	stageNode->m_alphaArgument0Complement = m_alphaArgument0Complement.GetCheck();
}

// ----------------------------------------------------------------------

void CStageView::OnColorargument1alphareplicate() 
{
	stageNode->m_colorArgument1AlphaReplicate = m_colorArgument1AlphaReplicate.GetCheck();
}

// ----------------------------------------------------------------------

void CStageView::OnColorargument2alphareplicate() 
{
	stageNode->m_colorArgument2AlphaReplicate = m_colorArgument2AlphaReplicate.GetCheck();
}

// ----------------------------------------------------------------------

void CStageView::OnColorargument0alphareplicate() 
{
	stageNode->m_colorArgument0AlphaReplicate = m_colorArgument0AlphaReplicate.GetCheck();
}

// ----------------------------------------------------------------------

BOOL CStageView::OnToolTipNotify(UINT id, NMHDR *pNMHDR, LRESULT *pResult)
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

// ======================================================================
