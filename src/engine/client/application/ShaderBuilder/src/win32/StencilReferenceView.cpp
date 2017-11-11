// StencilReferenceView.cpp : implementation file
//

#include "FirstShaderBuilder.h"
#include "ShaderBuilder.h"
#include "StencilReferenceView.h"

#include "Node.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CStencilReferenceView

IMPLEMENT_DYNCREATE(CStencilReferenceView, CFormView)

CStencilReferenceView::CStencilReferenceView()
	: CFormView(CStencilReferenceView::IDD),
	stencilReferenceValueNode(0)
{
	//{{AFX_DATA_INIT(CStencilReferenceView)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CStencilReferenceView::~CStencilReferenceView()
{
}

void CStencilReferenceView::OnInitialUpdate() 
{
	CFormView::OnInitialUpdate();
	EnableToolTips(true);   // enable tool tips for view
}

void CStencilReferenceView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CStencilReferenceView)
	DDX_Control(pDX, IDC_STENCILREFERENCE_VALUE_EDIT, m_stencilReferenceValue);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CStencilReferenceView, CFormView)
	//{{AFX_MSG_MAP(CStencilReferenceView)
	ON_EN_CHANGE(IDC_STENCILREFERENCE_VALUE_EDIT, OnChangeStencilreferencevalue)
	ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnToolTipNotify)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStencilReferenceView diagnostics

#ifdef _DEBUG
void CStencilReferenceView::AssertValid() const
{
	CFormView::AssertValid();
}

void CStencilReferenceView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CStencilReferenceView message handlers

void CStencilReferenceView::SetNode(CStencilReferenceValueNode &node)
{
	stencilReferenceValueNode = &node;
	m_stencilReferenceValue.SetWindowText(node.m_stencilReferenceValue);
}


void CStencilReferenceView::OnChangeStencilreferencevalue() 
{
	m_stencilReferenceValue.GetWindowText(stencilReferenceValueNode->m_stencilReferenceValue);
}


BOOL CStencilReferenceView::OnToolTipNotify(UINT id, NMHDR *pNMHDR, LRESULT *pResult)
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