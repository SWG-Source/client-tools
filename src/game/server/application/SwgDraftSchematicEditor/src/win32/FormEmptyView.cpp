// ======================================================================
//
// FormEmptyView.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "FirstSwgDraftSchematicEditor.h"
#include "FormEmptyView.h"

// ======================================================================

IMPLEMENT_DYNCREATE(FormEmptyView, CFormView)

// ----------------------------------------------------------------------

FormEmptyView::FormEmptyView()
	: CFormView(FormEmptyView::IDD)
{
	//{{AFX_DATA_INIT(FormEmptyView)
	//}}AFX_DATA_INIT
}

// ----------------------------------------------------------------------

FormEmptyView::~FormEmptyView()
{
}

// ----------------------------------------------------------------------

void FormEmptyView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(FormEmptyView)
	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(FormEmptyView, CFormView)
	//{{AFX_MSG_MAP(FormEmptyView)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

#ifdef _DEBUG
void FormEmptyView::AssertValid() const
{
	CFormView::AssertValid();
}

void FormEmptyView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

// ======================================================================
