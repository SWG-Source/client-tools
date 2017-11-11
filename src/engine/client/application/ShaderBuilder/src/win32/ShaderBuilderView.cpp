// ShaderBuilderView.cpp : implementation of the CShaderBuilderView class
//

#include "FirstShaderBuilder.h"
#include "ShaderBuilder.h"

#include "ShaderBuilderDoc.h"
#include "ShaderBuilderView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CShaderBuilderView

IMPLEMENT_DYNCREATE(CShaderBuilderView, CView)

BEGIN_MESSAGE_MAP(CShaderBuilderView, CView)
	//{{AFX_MSG_MAP(CShaderBuilderView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CShaderBuilderView construction/destruction

CShaderBuilderView::CShaderBuilderView()
{
	// TODO: add construction code here

}

CShaderBuilderView::~CShaderBuilderView()
{
}

BOOL CShaderBuilderView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CShaderBuilderView drawing

void CShaderBuilderView::OnDraw(CDC* pDC)
{
	UNREF(pDC);
	CShaderBuilderDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here
}

/////////////////////////////////////////////////////////////////////////////
// CShaderBuilderView diagnostics

#ifdef _DEBUG
void CShaderBuilderView::AssertValid() const
{
	CView::AssertValid();
}

void CShaderBuilderView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CShaderBuilderDoc* CShaderBuilderView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CShaderBuilderDoc)));
	return (CShaderBuilderDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CShaderBuilderView message handlers
