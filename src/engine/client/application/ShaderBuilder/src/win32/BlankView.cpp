// BlankView.cpp : implementation file
//

#include "FirstShaderBuilder.h"
#include "ShaderBuilder.h"
#include "BlankView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBlankView

IMPLEMENT_DYNCREATE(CBlankView, CFormView)

CBlankView::CBlankView()
	: CFormView(CBlankView::IDD)
{
	//{{AFX_DATA_INIT(CBlankView)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CBlankView::~CBlankView()
{
}

void CBlankView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBlankView)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBlankView, CFormView)
	//{{AFX_MSG_MAP(CBlankView)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBlankView diagnostics

#ifdef _DEBUG
void CBlankView::AssertValid() const
{
	CFormView::AssertValid();
}

void CBlankView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CBlankView message handlers

void CBlankView::OnSize(UINT nType, int cx, int cy) 
{
	CFormView::OnSize(nType, cx, cy);
	CFormView::ShowScrollBar(SB_VERT,FALSE);
	CFormView::ShowScrollBar(SB_HORZ,FALSE);
}
