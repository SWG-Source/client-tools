//
// HelpFrame.cpp
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "HelpFrame.h"
#include "TerrainEditorDoc.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(HelpFrame, CMDIChildWnd)

//-------------------------------------------------------------------

HelpFrame::HelpFrame() :
	CMDIChildWnd (),
	m_windowName (),
	m_wndToolBar ()
{
	m_windowName = _T("Help");
}

HelpFrame::~HelpFrame()
{
}

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(HelpFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(HelpFrame)
	ON_WM_DESTROY()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

void HelpFrame::OnDestroy() 
{
	//-- tell document we're being destroyed
	static_cast<TerrainEditorDoc*> (GetActiveDocument ())->setHelpFrame (0);

	CMDIChildWnd::OnDestroy();
}

//-------------------------------------------------------------------

int HelpFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	//-- create tool bar
	if (!m_wndToolBar.CreateEx(this) ||
		!m_wndToolBar.LoadToolBar(IDR_HELPVIEW))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY);

	return 0;
}

//-------------------------------------------------------------------

BOOL HelpFrame::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.lpszName = m_windowName;
	cs.style &= ~FWS_ADDTOTITLE;
	cs.style &= ~FWS_PREFIXTITLE;
	
	return CMDIChildWnd::PreCreateWindow(cs);
}

//-------------------------------------------------------------------

