//
// ShaderGroupFrame.cpp
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "terraineditor.h"
#include "ShaderGroupFrame.h"
#include "ShaderView.h"
#include "ShaderTreeView.h"
#include "TerrainEditorDoc.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(ShaderGroupFrame, CMDIChildWnd)

//-------------------------------------------------------------------

ShaderGroupFrame::ShaderGroupFrame (void) :
	m_windowName (),
	m_wndToolBar (),
	splitter (),
	treeView (0),
	shaderView (0)
{
	m_windowName = "Shader Families";
}

//-------------------------------------------------------------------
	
ShaderGroupFrame::~ShaderGroupFrame()
{
}

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(ShaderGroupFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(ShaderGroupFrame)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

int ShaderGroupFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	//-- create tool bar
	if (!m_wndToolBar.CreateEx(this) ||
		!m_wndToolBar.LoadToolBar(IDR_SHADERGROUPFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY);

	TerrainEditorApp* app = static_cast<TerrainEditorApp*>(AfxGetApp());
	if(!app->RestoreWindowPosition(this,"ShaderGroupFrame"))
	{
		CRect mainRect;
		AfxGetApp()->GetMainWnd ()->GetClientRect (&mainRect);
		mainRect.right  -= 2; // allow for frame
		mainRect.bottom -= 64; // allow for toolbars, etc.
		SetWindowPos (&wndTop, 0, (mainRect.bottom/2), (mainRect.right*1/4), (mainRect.bottom/2), SWP_SHOWWINDOW);
	}
	
	return 0;
}

//-------------------------------------------------------------------

BOOL ShaderGroupFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
	UNREF (lpcs);

	if (!splitter.CreateStatic (this, 1, 2))
		return FALSE;

	if (!splitter.CreateView (0, 0, RUNTIME_CLASS (ShaderTreeView), CSize (100, 100), pContext))
		return FALSE;

	if (!splitter.CreateView (0, 1, RUNTIME_CLASS (ShaderView), CSize (100, 100), pContext))
		return FALSE;

	treeView   = static_cast<ShaderTreeView*> (splitter.GetPane (0, 0));
	shaderView = static_cast<ShaderView*>     (splitter.GetPane (0, 1));

	//-- don't chain back
//	return CMDIChildWnd::OnCreateClient(lpcs, pContext);

	return TRUE;
}

//-------------------------------------------------------------------

void ShaderGroupFrame::OnDestroy() 
{
	//-- tell document we're being destroyed
	static_cast<TerrainEditorDoc*> (GetActiveDocument ())->setShaderGroupFrame (0);

	TerrainEditorApp* app = static_cast<TerrainEditorApp*>(AfxGetApp());
	app->SaveWindowPosition(this,"ShaderGroupFrame");
	//-- chain to base 
	CMDIChildWnd::OnDestroy();
}

//-------------------------------------------------------------------

BOOL ShaderGroupFrame::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.lpszName = m_windowName;
	cs.style &= ~FWS_ADDTOTITLE;
	cs.style &= ~FWS_PREFIXTITLE;

	return CMDIChildWnd::PreCreateWindow(cs);
}

//-------------------------------------------------------------------

void ShaderGroupFrame::setSelectedShader (const char* name)
{
	//-- tell the mesh view about the newly selected object
	shaderView->loadShader (name);
}

//-------------------------------------------------------------------

void ShaderGroupFrame::addFamily (const CString& familyName, const ArrayList<CString*>& children)
{
	if (treeView)
		treeView->addFamily (familyName, children);
}

//-------------------------------------------------------------------

void ShaderGroupFrame::reset (void)
{
	if (treeView)
		treeView->reset ();
}

//-------------------------------------------------------------------
