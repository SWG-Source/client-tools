// ======================================================================
// SplitterView.cpp : implementation file
// ======================================================================

#include "FirstShaderBuilder.h"
#include "ShaderBuilder.h"
#include "SplitterView.h"

#include "LeftView.h"
#include "RenderView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// ======================================================================

IMPLEMENT_DYNCREATE(CSplitterView, CView)

BEGIN_MESSAGE_MAP(CSplitterView, CView)
	//{{AFX_MSG_MAP(CSplitterView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ======================================================================

CSplitterView::CSplitterView()
{
}

// ----------------------------------------------------------------------

CSplitterView::~CSplitterView()
{
}

// ----------------------------------------------------------------------

void CSplitterView::OnDraw(CDC* pDC)
{
	UNREF(pDC);
	CDocument* pDoc = GetDocument();
	UNREF(pDoc);
}

// ----------------------------------------------------------------------

#ifdef _DEBUG
void CSplitterView::AssertValid() const
{
	CView::AssertValid();
}

// ----------------------------------------------------------------------

void CSplitterView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

// ----------------------------------------------------------------------

int CSplitterView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	m_wndSplitter.CreateStatic(this, 2, 1);
	CCreateContext *pContext = (CCreateContext*) lpCreateStruct->lpCreateParams;
	m_wndSplitter.CreateView(0,0,RUNTIME_CLASS(CLeftView), CSize(150,400), pContext);
	m_wndSplitter.CreateView(1,0,RUNTIME_CLASS(CRenderView), CSize(150,150), pContext);


	// the render view needs to know about the tree view, and vice-versa
	CLeftView   *leftView   = static_cast<CLeftView *>(m_wndSplitter.GetPane(0,0));
	CRenderView *renderView = static_cast<CRenderView *>(m_wndSplitter.GetPane(1,0));
	renderView->leftView = leftView;
	leftView->renderView = renderView;

	return 0;
}

// ----------------------------------------------------------------------

void CSplitterView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);

	const int borderWidth  = GetSystemMetrics(SM_CXEDGE);	
	const int borderHeight = GetSystemMetrics(SM_CYEDGE);	

	m_wndSplitter.MoveWindow(-borderWidth,-borderHeight,cx+(2*borderWidth),cy+(2*borderHeight));
	m_wndSplitter.RecalcLayout();
}

// ======================================================================
