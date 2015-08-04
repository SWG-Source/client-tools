//
// FractalPreviewFrame.cpp
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "FractalPreviewFrame.h"

#include "FractalPreviewView.h"
#include "TerrainEditor.h"
#include "TerrainEditorDoc.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(FractalPreviewFrame, CMDIChildWnd)

//-------------------------------------------------------------------

FractalPreviewFrame::FractalPreviewFrame () :
	CMDIChildWnd (),
	m_windowName ()
{
	m_windowName = "Fractal Preview";
}

//-------------------------------------------------------------------

FractalPreviewFrame::~FractalPreviewFrame()
{
}

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(FractalPreviewFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(FractalPreviewFrame)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

int FractalPreviewFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	TerrainEditorApp* app = static_cast<TerrainEditorApp*>(AfxGetApp());
	if(!app->RestoreWindowPosition(this,"FractalPreviewFrame"))
	{
		CRect mainRect;
		AfxGetApp()->GetMainWnd ()->GetClientRect (&mainRect);
		mainRect.right  -= 2; // allow for frame
		mainRect.bottom -= 64; // allow for toolbars, etc.
		
		IGNORE_RETURN (SetWindowPos (&wndTop, ((mainRect.right*2)/3) - 192, 0, 192, 192, SWP_SHOWWINDOW));
	}
	return 0;
}

//-------------------------------------------------------------------

void FractalPreviewFrame::OnDestroy() 
{
	//-- chain to base 
	CMDIChildWnd::OnDestroy();
	
	TerrainEditorApp* app = static_cast<TerrainEditorApp*>(AfxGetApp());
	app->SaveWindowPosition(this,"FractalPreviewFrame");

	//-- tell document we're being destroyed
	static_cast<TerrainEditorDoc*> (GetActiveDocument ())->setFractalPreviewFrame (0);
}

//-------------------------------------------------------------------

BOOL FractalPreviewFrame::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.lpszName = m_windowName;
	
	return CMDIChildWnd::PreCreateWindow(cs);
}

//-------------------------------------------------------------------

void FractalPreviewFrame::updateBitmap (const MultiFractal& multiFractal, float low, float high)
{
	safe_cast<FractalPreviewView*> (GetActiveView ())->updateBitmap (multiFractal, low, high);
}

//-------------------------------------------------------------------

