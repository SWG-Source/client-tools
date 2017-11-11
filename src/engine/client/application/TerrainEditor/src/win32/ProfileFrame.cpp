//
// ProfileFrame.cpp
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "ProfileFrame.h"

#include "ProfileView.h"
#include "TerrainEditor.h"
#include "TerrainEditorDoc.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(ProfileFrame, CMDIChildWnd)

//-------------------------------------------------------------------

ProfileFrame::ProfileFrame() :
	CMDIChildWnd (),
	m_windowName ()
{
	m_windowName = _T("Profile");
}

//-------------------------------------------------------------------

ProfileFrame::~ProfileFrame()
{
}

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(ProfileFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(ProfileFrame)
	ON_WM_DESTROY()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

void ProfileFrame::OnDestroy() 
{
	//-- tell document we're being destroyed
	static_cast<TerrainEditorDoc*> (GetActiveDocument ())->setProfileFrame (0);

	TerrainEditorApp* app = static_cast<TerrainEditorApp*>(AfxGetApp());
	app->SaveWindowPosition(this,"ProfileFrame");
	CMDIChildWnd::OnDestroy();
}

//-------------------------------------------------------------------

BOOL ProfileFrame::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.lpszName = m_windowName;
	cs.style &= ~FWS_ADDTOTITLE;
	cs.style &= ~FWS_PREFIXTITLE;
	
	return CMDIChildWnd::PreCreateWindow(cs);
}

//-------------------------------------------------------------------

int ProfileFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	TerrainEditorApp* app = static_cast<TerrainEditorApp*>(AfxGetApp());
	if(!app->RestoreWindowPosition(this,"ProfileFrame"))
	{
		CRect mainRect;
		AfxGetApp()->GetMainWnd ()->GetClientRect (&mainRect);
		mainRect.right  -= 2; // allow for frame
		mainRect.bottom -= 64; // allow for toolbars, etc.
	
		IGNORE_RETURN (SetWindowPos (&wndTop, 0, (mainRect.bottom*1)/2, (mainRect.right*3/4), (mainRect.bottom*1/4), SWP_SHOWWINDOW));
	}

	return 0;
}

//-------------------------------------------------------------------

void ProfileFrame::update (const TerrainGenerator* generator)
{
	//-- route to view
	if (GetActiveView ())
		static_cast<ProfileView*> (GetActiveView ())->update (generator);
}

//-------------------------------------------------------------------

