// ======================================================================
//
// EmptyView.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "FirstSwgConversationEditor.h"
#include "EmptyView.h"

#include "SwgConversationEditor.h"

// ======================================================================

IMPLEMENT_DYNCREATE(EmptyView, CFormView)

// ======================================================================

EmptyView::EmptyView()
	: CFormView(EmptyView::IDD)
{
	//{{AFX_DATA_INIT(EmptyView)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

// ----------------------------------------------------------------------

EmptyView::~EmptyView()
{
}

// ----------------------------------------------------------------------

void EmptyView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(EmptyView)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(EmptyView, CFormView)
	//{{AFX_MSG_MAP(EmptyView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

#ifdef _DEBUG
void EmptyView::AssertValid() const
{
	CFormView::AssertValid();
}

void EmptyView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

// ======================================================================
