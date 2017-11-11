// ======================================================================
//
// WarningFrame.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "FirstSwgConversationEditor.h"
#include "WarningFrame.h"

#include "SwgConversationEditorDoc.h"
#include "WarningView.h"

// ======================================================================

IMPLEMENT_DYNCREATE(WarningFrame, CMDIChildWnd)

WarningFrame::WarningFrame () :
	CMDIChildWnd (),
	m_windowName ("Output")
{
}

WarningFrame::~WarningFrame()
{
}

// ======================================================================

BEGIN_MESSAGE_MAP(WarningFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(WarningFrame)
	ON_WM_DESTROY()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ======================================================================

void WarningFrame::OnDestroy() 
{
	CMDIChildWnd::OnDestroy();
	
	//-- tell document we're being destroyed
	if (GetActiveDocument ())
		safe_cast<SwgConversationEditorDoc*> (GetActiveDocument ())->setWarningFrame (0);
}

// ----------------------------------------------------------------------

BOOL WarningFrame::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.lpszName = m_windowName;
	cs.style &= ~FWS_ADDTOTITLE;
	cs.style &= ~FWS_PREFIXTITLE;
	
	return CMDIChildWnd::PreCreateWindow(cs);
}

// ----------------------------------------------------------------------

int WarningFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	CRect mainRect;
	AfxGetApp()->GetMainWnd ()->GetClientRect (&mainRect);
	mainRect.right  -= 4; // allow for frame
	mainRect.bottom -= 64; // allow for toolbars, etc.
	
	IGNORE_RETURN (SetWindowPos (&wndTop, 0, mainRect.bottom * 3 / 4, mainRect.right, mainRect.bottom / 4, SWP_SHOWWINDOW));
	
	return 0;
}

// ----------------------------------------------------------------------

void WarningFrame::clear ()
{
	safe_cast<WarningView *> (GetActiveView ())->clear ();
}

// ----------------------------------------------------------------------

void WarningFrame::add (int const type, CString const & message, ConversationItem const * const conversationItem)
{
	safe_cast<WarningView *> (GetActiveView ())->add (type, message, conversationItem);
}

// ----------------------------------------------------------------------

void WarningFrame::ActivateFrame(int nCmdShow) 
{
	//-- tell the document about it
	safe_cast<SwgConversationEditorDoc*> (GetActiveDocument ())->setWarningFrame (this);

	CMDIChildWnd::ActivateFrame(nCmdShow);
}

// ======================================================================

