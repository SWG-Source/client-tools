// ======================================================================
//
// ChildFrame.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "FirstSwgDraftSchematicEditor.h"
#include "ChildFrame.h"

#include "DraftSchematicTreeView.h"
#include "FormAttributeView.h"
#include "FormEmptyView.h"
#include "FormPropertyView.h"
#include "FormSlotView.h"
#include "MainFrame.h"

// ======================================================================

IMPLEMENT_DYNCREATE(ChildFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(ChildFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(ChildFrame)
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

ChildFrame::ChildFrame() :
	CMDIChildWnd (),
	m_initialized (false),
	m_splitter (),
	m_treeView (0),
	m_emptyViewId (0),
	m_emptyView (0),
	m_propertyViewId (0),
	m_propertyView (0),
	m_slotViewId (0),
	m_slotView (0),
	m_attributeViewId (0),
	m_attributeView (0)
{
	m_splitter.setLocked (true);
}

// ----------------------------------------------------------------------

ChildFrame::~ChildFrame()
{
}

// ----------------------------------------------------------------------

BOOL ChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	cs.style |= WS_MAXIMIZE;

	if( !CMDIChildWnd::PreCreateWindow(cs) )
		return FALSE;

	return TRUE;
}

// ----------------------------------------------------------------------

#ifdef _DEBUG
void ChildFrame::AssertValid() const
{
	CMDIChildWnd::AssertValid();
}

void ChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWnd::Dump(dc);
}

#endif //_DEBUG

// ----------------------------------------------------------------------

BOOL ChildFrame::OnCreateClient(LPCREATESTRUCT const lpcs, CCreateContext * const pContext) 
{
	if (!m_splitter.CreateStatic (this, 1, 2))
		return FALSE;

	m_emptyViewId = m_splitter.AddView (0, 1, RUNTIME_CLASS (FormEmptyView), pContext);
	m_emptyView = safe_cast<FormEmptyView *> (m_splitter.GetView (m_emptyViewId));
	m_propertyViewId = m_splitter.AddView (0, 1, RUNTIME_CLASS (FormPropertyView), pContext);
	m_propertyView = safe_cast<FormPropertyView *> (m_splitter.GetView (m_propertyViewId));
	m_slotViewId = m_splitter.AddView (0, 1, RUNTIME_CLASS (FormSlotView), pContext);
	m_slotView = safe_cast<FormSlotView *> (m_splitter.GetView (m_slotViewId));
	m_attributeViewId = m_splitter.AddView (0, 1, RUNTIME_CLASS (FormAttributeView), pContext);
	m_attributeView = safe_cast<FormAttributeView *> (m_splitter.GetView (m_attributeViewId));

	m_splitter.CreateView (0, 0, RUNTIME_CLASS (DraftSchematicTreeView), CSize (0, 0), pContext);
	m_treeView = safe_cast<DraftSchematicTreeView *> (m_splitter.GetPane (0, 0));

	CRect rect;
	GetWindowRect (&rect);

	int x = 0;
	x = max (m_emptyView->GetTotalSize ().cx, x);
	x = max (m_propertyView->GetTotalSize ().cx, x);
	x = max (m_slotView->GetTotalSize ().cx, x);
	x = max (m_attributeView->GetTotalSize ().cx, x);
	int y = 0;
	y = max (m_emptyView->GetTotalSize ().cy, y);
	y = max (m_propertyView->GetTotalSize ().cy, y);
	y = max (m_slotView->GetTotalSize ().cy, y);
	y = max (m_attributeView->GetTotalSize ().cy, y);
	m_splitter.SetColumnInfo (0, rect.Width () - (x + 28), y);

	m_initialized = true;

	return TRUE;
}

// ----------------------------------------------------------------------

void ChildFrame::OnSize(UINT nType, int cx, int cy) 
{
	if (m_initialized)
	{
		int x = 0;
		x = max (m_emptyView->GetTotalSize ().cx, x);
		x = max (m_propertyView->GetTotalSize ().cx, x);
		x = max (m_slotView->GetTotalSize ().cx, x);
		x = max (m_attributeView->GetTotalSize ().cx, x);
		x = cx - (x + 28);
		if (x > 28)
			m_splitter.SetColumnInfo (0, x, 0);
	}

	CMDIChildWnd::OnSize(nType, cx, cy);
}

// ----------------------------------------------------------------------

void ChildFrame::setViewEmpty ()
{
	m_splitter.ShowView (m_emptyViewId);
}

// ----------------------------------------------------------------------

void ChildFrame::setViewProperty ()
{
	m_splitter.ShowView (m_propertyViewId);
	m_propertyView->update ();
}

// ----------------------------------------------------------------------

void ChildFrame::setViewSlots ()
{
	m_splitter.ShowView (m_slotViewId);
	m_slotView->setItemData (m_treeView->getSelectedItemData ());
}

// ----------------------------------------------------------------------

void ChildFrame::setViewAttributes ()
{
	m_splitter.ShowView (m_attributeViewId);
	m_attributeView->setItemData (m_treeView->getSelectedItemData ());
}

// ----------------------------------------------------------------------

void ChildFrame::OnSetFocus(CWnd* pOldWnd) 
{
	CMDIChildWnd::OnSetFocus(pOldWnd);

	safe_cast<SwgDraftSchematicEditorDoc *> (GetActiveDocument ())->updateTpfTabs (GetActiveDocument ()->GetPathName ());
}

// ----------------------------------------------------------------------

BOOL ChildFrame::DestroyWindow() 
{
	safe_cast<MainFrame *> (AfxGetMainWnd ())->clearTpfTabs ();
	
	return CMDIChildWnd::DestroyWindow();
}

// ======================================================================

