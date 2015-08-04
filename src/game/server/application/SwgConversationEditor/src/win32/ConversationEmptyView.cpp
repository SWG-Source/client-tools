// ======================================================================
//
// ConversationEmptyView.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "FirstSwgConversationEditor.h"
#include "ConversationEmptyView.h"

#include "SwgConversationEditor.h"

// ======================================================================

IMPLEMENT_DYNCREATE(ConversationEmptyView, CFormView)

// ======================================================================

ConversationEmptyView::ConversationEmptyView()
	: CFormView(ConversationEmptyView::IDD)
{
	//{{AFX_DATA_INIT(ConversationEmptyView)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

// ----------------------------------------------------------------------

ConversationEmptyView::~ConversationEmptyView()
{
}

// ----------------------------------------------------------------------

void ConversationEmptyView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ConversationEmptyView)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(ConversationEmptyView, CFormView)
	//{{AFX_MSG_MAP(ConversationEmptyView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

#ifdef _DEBUG
void ConversationEmptyView::AssertValid() const
{
	CFormView::AssertValid();
}

void ConversationEmptyView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

// ======================================================================
