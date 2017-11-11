// EffectView.cpp : implementation file
//

#include "FirstShaderBuilder.h"
#include "ShaderBuilder.h"
#include "EffectView.h"

#include "Node.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEffectView

IMPLEMENT_DYNCREATE(CEffectView, CFormView)

CEffectView::CEffectView()
	: CFormView(CEffectView::IDD),
	m_effectNode(NULL)
{
	//{{AFX_DATA_INIT(CEffectView)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CEffectView::~CEffectView()
{
}

void CEffectView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEffectView)
	DDX_Control(pDX, IDC_EFFECT_PRELIT, m_effectPrelit);
	DDX_Control(pDX, IDC_NAME, m_name);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEffectView, CFormView)
	//{{AFX_MSG_MAP(CEffectView)
	ON_BN_CLICKED(IDC_EFFECT_PRELIT, OnEffectPrelit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEffectView diagnostics

#ifdef _DEBUG
void CEffectView::AssertValid() const
{
	CFormView::AssertValid();
}

void CEffectView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CEffectView message handlers

void CEffectView::SetNode(CEffectNode &effectNode)
{
	m_effectNode = &effectNode;

	// set the file name correctly
	if (effectNode.IsLocal())
	{
		CString newText = "Local effect:\t";
		newText += effectNode.GetFileName();
		m_name.SetWindowText(newText);
	}
	else
	{
		CString newText = "Named effect:\t";
		newText += effectNode.GetFileName();
		m_name.SetWindowText(newText);
	}

	if (effectNode.IsLocked(CNode::Either))	
		EnableWindow(FALSE);
	else
		EnableWindow(TRUE);

	m_effectPrelit.SetCheck(m_effectNode->m_containsPrecalculatedVertexLighting);
}

void CEffectView::OnEffectPrelit() 
{
	// TODO: Add your control notification handler code here
	m_effectNode->m_containsPrecalculatedVertexLighting = m_effectPrelit.GetCheck() ? true : false;
}
