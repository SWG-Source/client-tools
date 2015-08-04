// AlphaReferenceView.cpp : implementation file
//

#include "FirstShaderBuilder.h"
#include "ShaderBuilder.h"
#include "AlphaReferenceView.h"

#include "Node.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAlphaReferenceView

IMPLEMENT_DYNCREATE(CAlphaReferenceView, CFormView)

CAlphaReferenceView::CAlphaReferenceView()
	: CFormView(CAlphaReferenceView::IDD),
	alphaReferenceValueNode(0)
{
	//{{AFX_DATA_INIT(CAlphaReferenceView)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CAlphaReferenceView::~CAlphaReferenceView()
{
}

void CAlphaReferenceView::OnInitialUpdate() 
{
	CFormView::OnInitialUpdate();
	EnableToolTips(true);   // enable tool tips for view
}

void CAlphaReferenceView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAlphaReferenceView)
	DDX_Control(pDX, IDC_ALPHAREFERENCE_VALUE_SLIDER, m_slider);
	DDX_Control(pDX, IDC_ALPHAREFERENCE_VALUE_EDIT, m_alphaReferenceValue);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAlphaReferenceView, CFormView)
	//{{AFX_MSG_MAP(CAlphaReferenceView)
	ON_EN_CHANGE(IDC_ALPHAREFERENCE_VALUE_EDIT, OnChangeAlphareferencevalue)
	ON_WM_HSCROLL()
	ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnToolTipNotify)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAlphaReferenceView diagnostics

#ifdef _DEBUG
void CAlphaReferenceView::AssertValid() const
{
	CFormView::AssertValid();
}

void CAlphaReferenceView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CAlphaReferenceView message handlers

void CAlphaReferenceView::SetNode(CAlphaReferenceValueNode &node)
{
	alphaReferenceValueNode = &node;
	m_alphaReferenceValue.SetWindowText(node.m_alphaReferenceValue);


	m_slider.SetRange (0, 255);
	m_slider.SetPos (static_cast<int>(atoi(node.m_alphaReferenceValue)));
}

void CAlphaReferenceView::OnChangeAlphareferencevalue() 
{
	m_alphaReferenceValue.GetWindowText(alphaReferenceValueNode->m_alphaReferenceValue);
	m_slider.SetPos (static_cast<int>(atoi(alphaReferenceValueNode->m_alphaReferenceValue)));
}

void CAlphaReferenceView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{	// TODO: Add your message handler code here and/or call default

	
	CFormView::OnHScroll(nSBCode, nPos, pScrollBar);
	UpdateData(true);
	
	int value = m_slider.GetPos();
	char buffer[8];
	_itoa(value, buffer, 10);
	m_alphaReferenceValue.SetWindowText(buffer);
}

BOOL CAlphaReferenceView::OnToolTipNotify(UINT id, NMHDR *pNMHDR, LRESULT *pResult)
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
