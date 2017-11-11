// TextureCoordinateSetView.cpp : implementation file
//

#include "FirstShaderBuilder.h"
#include "ShaderBuilder.h"
#include "TextureCoordinateSetView.h"

#include "Node.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTextureCoordinateSetView

IMPLEMENT_DYNCREATE(CTextureCoordinateSetView, CFormView)

CTextureCoordinateSetView::CTextureCoordinateSetView()
	: CFormView(CTextureCoordinateSetView::IDD),
	textureCoordinateSetNode(0)
{
	//{{AFX_DATA_INIT(CTextureCoordinateSetView)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CTextureCoordinateSetView::~CTextureCoordinateSetView()
{
}

void CTextureCoordinateSetView::OnInitialUpdate() 
{
	CFormView::OnInitialUpdate();
	EnableToolTips(true);   // enable tool tips for view
}

void CTextureCoordinateSetView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTextureCoordinateSetView)
	DDX_Control(pDX, IDC_TEXTURECOORDINATESET_INDEX_COMBO, m_textureCoordinateSetIndex);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTextureCoordinateSetView, CFormView)
	//{{AFX_MSG_MAP(CTextureCoordinateSetView)
	ON_CBN_SELCHANGE(IDC_TEXTURECOORDINATESET_INDEX_COMBO, OnSelchangeTexturecoordinatesetindex)
	ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnToolTipNotify)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTextureCoordinateSetView diagnostics

#ifdef _DEBUG
void CTextureCoordinateSetView::AssertValid() const
{
	CFormView::AssertValid();
}

void CTextureCoordinateSetView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTextureCoordinateSetView message handlers

void CTextureCoordinateSetView::SetNode(CTextureCoordinateSetNode &node)
{
	textureCoordinateSetNode = &node;
	m_textureCoordinateSetIndex.SelectString(-1, node.m_textureCoordinateSetIndex);
}

void CTextureCoordinateSetView::OnSelchangeTexturecoordinatesetindex() 
{
	m_textureCoordinateSetIndex.GetWindowText(textureCoordinateSetNode->m_textureCoordinateSetIndex);
}

/////////////////////////////////////////////////////////////////////////////

BOOL CTextureCoordinateSetView::OnToolTipNotify(UINT id, NMHDR *pNMHDR, LRESULT *pResult)
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

/////////////////////////////////////////////////////////////////////////////
