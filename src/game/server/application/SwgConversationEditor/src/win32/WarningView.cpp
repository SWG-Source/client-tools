// ======================================================================
//
// WarningFrame.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "FirstSwgConversationEditor.h"
#include "WarningView.h"

#include "ConversationFrame.h"
#include "Resource.h"
#include "SwgConversationEditor.h"

// ======================================================================

IMPLEMENT_DYNCREATE(WarningView, CListView)

WarningView::WarningView() :
	m_imageListSet (false),
	m_imageList ()
{
}

WarningView::~WarningView()
{
}

// ======================================================================

BEGIN_MESSAGE_MAP(WarningView, CListView)
	//{{AFX_MSG_MAP(WarningView)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ======================================================================

void WarningView::OnDraw (CDC * /*pDC*/)
{
}

// ======================================================================

#ifdef _DEBUG
void WarningView::AssertValid() const
{
	CListView::AssertValid();
}

void WarningView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}
#endif //_DEBUG

// ----------------------------------------------------------------------

void WarningView::OnInitialUpdate() 
{
	CListView::OnInitialUpdate();
	
	if (!m_imageListSet)
	{
		IGNORE_RETURN (m_imageList.Create (IDB_BITMAP_WARNINGVIEW, 16, 1, RGB (255,255,255)));
		IGNORE_RETURN (GetListCtrl ().SetImageList (&m_imageList, LVSIL_SMALL));

		m_imageListSet = true;
	}

	IGNORE_RETURN (GetListCtrl ().InsertColumn (0, "Description", LVCFMT_LEFT, 1024, 0));
}

// ----------------------------------------------------------------------

BOOL WarningView::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.style = cs.style | LVS_REPORT;
	
	return CListView::PreCreateWindow(cs);
}

// ----------------------------------------------------------------------

void WarningView::clear ()
{
	GetListCtrl ().DeleteAllItems ();
}

// ----------------------------------------------------------------------

void WarningView::add (int const type, CString const & message, ConversationItem const * const conversationItem)
{
	int const index = GetListCtrl ().GetItemCount ();

	IGNORE_RETURN (GetListCtrl ().InsertItem (index, message, type));
	IGNORE_RETURN (GetListCtrl ().SetItemData (index, reinterpret_cast<DWORD> (conversationItem)));
}

// ----------------------------------------------------------------------

void WarningView::OnDblclk (NMHDR * const /*pNMHDR*/, LRESULT * const pResult) 
{
	POSITION position = GetListCtrl ().GetFirstSelectedItemPosition ();

	if (position)
	{
		int const item = GetListCtrl ().GetNextSelectedItem (position);

		ConversationItem const * const conversationItem = reinterpret_cast<ConversationItem const *> (GetListCtrl ().GetItemData (item));

		SwgConversationEditorDoc * const document = safe_cast<SwgConversationEditorDoc *> (GetDocument ());
		safe_cast<SwgConversationEditorApp *> (AfxGetApp ())->onViewConversation (document);

		if (document->getConversationFrame ())
			document->getConversationFrame ()->setView (conversationItem);
	}
	
	*pResult = 0;
}

// ======================================================================

