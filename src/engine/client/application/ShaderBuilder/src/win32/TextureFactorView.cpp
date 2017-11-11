// TextureFactorView.cpp : implementation file
//

#include "FirstShaderBuilder.h"
#include "ShaderBuilder.h"
#include "TextureFactorView.h"

#include "Node.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTextureFactorView

IMPLEMENT_DYNCREATE(CTextureFactorView, CFormView)

CTextureFactorView::CTextureFactorView()
	: CFormView(CTextureFactorView::IDD),
	textureFactorNode(0),
	m_once(false)
{
	//{{AFX_DATA_INIT(CTextureFactorView)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CTextureFactorView::~CTextureFactorView()
{
}

void CTextureFactorView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTextureFactorView)
	DDX_Control(pDX, IDC_TEXTUREFACTOR_ALPHA_EDIT, m_alpha);
	DDX_Control(pDX, IDC_TEXTUREFACTOR_RED_EDIT, m_red);
	DDX_Control(pDX, IDC_TEXTUREFACTOR_GREEN_EDIT, m_green);
	DDX_Control(pDX, IDC_TEXTUREFACTOR_BLUE_EDIT, m_blue);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTextureFactorView, CFormView)
	//{{AFX_MSG_MAP(CTextureFactorView)
	ON_EN_CHANGE(IDC_TEXTUREFACTOR_ALPHA_EDIT, OnChangeAlpha)
	ON_EN_CHANGE(IDC_TEXTUREFACTOR_RED_EDIT, OnChangeRed)
	ON_EN_CHANGE(IDC_TEXTUREFACTOR_GREEN_EDIT, OnChangeGreen)
	ON_EN_CHANGE(IDC_TEXTUREFACTOR_BLUE_EDIT, OnChangeBlue)
	ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnToolTipNotify)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_USERCOLORCHANGE, OnColorChange)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTextureFactorView diagnostics

#ifdef _DEBUG
void CTextureFactorView::AssertValid() const
{
	CFormView::AssertValid();
}

void CTextureFactorView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTextureFactorView message handlers

void CTextureFactorView::SetNode(CTextureFactorNode &node)
{
	textureFactorNode = &node;

#define SET(a)  a.SetWindowText(node.a)

	SET(m_alpha);
	SET(m_red);
	SET(m_green);
	SET(m_blue);

#undef SET
	SetButtonColor();
}

void CTextureFactorView::OnChangeAlpha() 
{
	m_alpha.GetWindowText(textureFactorNode->m_alpha);
}

void CTextureFactorView::OnChangeRed() 
{
	m_red.GetWindowText(textureFactorNode->m_red);
	SetButtonColor();
}

void CTextureFactorView::OnChangeGreen() 
{
	m_green.GetWindowText(textureFactorNode->m_green);
	SetButtonColor();
}

void CTextureFactorView::OnChangeBlue() 
{
	m_blue.GetWindowText(textureFactorNode->m_blue);
	SetButtonColor();
}

void CTextureFactorView::OnInitialUpdate() 
{
	CFormView::OnInitialUpdate();
	EnableToolTips(true);   // enable tool tips for view

	if (!m_once)
	{
		m_colorButton.SubclassDlgItem(IDC_TEXTUREFACTOR_BROWSE_BUTTON,this);
		m_once = true;
	}

	m_alpha.SetLimitText(3);
	m_red.SetLimitText(3);
	m_green.SetLimitText(3);
	m_blue.SetLimitText(3);
}

void CTextureFactorView::SetButtonColor()
{
	const int r = atoi(textureFactorNode->m_red);
	const int g = atoi(textureFactorNode->m_green);
	const int b = atoi(textureFactorNode->m_blue);
	m_colorButton.currentcolor = RGB(r,g,b);
	m_colorButton.Invalidate();
}

LONG CTextureFactorView::OnColorChange(UINT id, LONG value)
{
	static_cast<void>(id);

	char t[8];
	sprintf(t, "%d", GetRValue(value));
	m_red.SetWindowText(t);

	sprintf(t, "%d", GetGValue(value));
	m_green.SetWindowText(t);

	sprintf(t, "%d", GetBValue(value));
	m_blue.SetWindowText(t);

	return 0;
}

BOOL CTextureFactorView::OnToolTipNotify(UINT id, NMHDR *pNMHDR, LRESULT *pResult)
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

