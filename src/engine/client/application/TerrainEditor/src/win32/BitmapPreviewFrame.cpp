//
// BitmapPreviewFrame.cpp
//
// copyright 2004, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "BitmapPreviewFrame.h"

#include "BitmapPreviewView.h"
#include "TerrainEditor.h"
#include "TerrainEditorDoc.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(BitmapPreviewFrame, CMDIChildWnd)

//-------------------------------------------------------------------

BitmapPreviewFrame::BitmapPreviewFrame () :
	CMDIChildWnd (),
	m_windowName ()
{
	m_windowName = "Bitmap Preview";
}

//-------------------------------------------------------------------

BitmapPreviewFrame::~BitmapPreviewFrame()
{
}

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(BitmapPreviewFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(BitmapPreviewFrame)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

int BitmapPreviewFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	TerrainEditorApp* app = static_cast<TerrainEditorApp*>(AfxGetApp());
	if(!app->RestoreWindowPosition(this,"BitmapPreviewFrame"))
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

void BitmapPreviewFrame::OnDestroy() 
{
	//-- chain to base 
	CMDIChildWnd::OnDestroy();
	
	TerrainEditorApp* app = static_cast<TerrainEditorApp*>(AfxGetApp());
	app->SaveWindowPosition(this,"BitmapPreviewFrame");

	//-- tell document we're being destroyed
	static_cast<TerrainEditorDoc*> (GetActiveDocument ())->setBitmapPreviewFrame (0);
}

//-------------------------------------------------------------------

BOOL BitmapPreviewFrame::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.lpszName = m_windowName;
	
	return CMDIChildWnd::PreCreateWindow(cs);
}

//-------------------------------------------------------------------

void BitmapPreviewFrame::updateBitmap (const Image& image, float low, float high, float gain)
{
	safe_cast<BitmapPreviewView*> (GetActiveView ())->updateBitmap (image, low, high, gain);
}

//-------------------------------------------------------------------

