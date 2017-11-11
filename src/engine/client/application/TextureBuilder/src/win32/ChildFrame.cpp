// ======================================================================
//
// ChildFrame.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "FirstTextureBuilder.h"
#include "ChildFrame.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// ======================================================================
// class ChildFrame
// ======================================================================

ChildFrame::ChildFrame()
:	CMDIChildWnd()
{
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

	if( !CMDIChildWnd::PreCreateWindow(cs) )
		return FALSE;

	return TRUE;
}

// ----------------------------------------------------------------------

IMPLEMENT_DYNCREATE(ChildFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(ChildFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(ChildFrame)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

#ifdef _DEBUG
void ChildFrame::AssertValid() const
{
	CMDIChildWnd::AssertValid();
}
#endif

// ----------------------------------------------------------------------

#ifdef _DEBUG
void ChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWnd::Dump(dc);
}

#endif

// ======================================================================
