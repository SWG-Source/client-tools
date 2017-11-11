//
// HelpView.cpp
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "HelpView.h"

#include "Resource.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(HelpView, CHtmlView)

//-------------------------------------------------------------------

HelpView::HelpView() :
	CHtmlView ()
{
	//{{AFX_DATA_INIT(HelpView)
	//}}AFX_DATA_INIT
}

//-------------------------------------------------------------------

HelpView::~HelpView()
{
}

//-------------------------------------------------------------------

void HelpView::DoDataExchange(CDataExchange* pDX)
{
	CHtmlView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(HelpView)
	//}}AFX_DATA_MAP
}

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(HelpView, CHtmlView)
	//{{AFX_MSG_MAP(HelpView)
	ON_COMMAND(ID_BUTTON_BACK, OnButtonBack)
	ON_COMMAND(ID_BUTTON_FORWARD, OnButtonForward)
	ON_COMMAND(ID_BUTTON_HOME, OnButtonHome)
	ON_COMMAND(ID_BUTTON_STOP, OnButtonStop)
	ON_COMMAND(ID_BUTTON_REFRESH, OnButtonRefresh)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

#ifdef _DEBUG
void HelpView::AssertValid() const
{
	CHtmlView::AssertValid();
}

void HelpView::Dump(CDumpContext& dc) const
{
	CHtmlView::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

void HelpView::OnButtonBack() 
{
	// TODO: Add your command handler code here
	GoBack ();
}

//-------------------------------------------------------------------

void HelpView::OnButtonForward() 
{
	// TODO: Add your command handler code here
	GoForward ();
}

//-------------------------------------------------------------------

void HelpView::OnButtonHome() 
{
	// TODO: Add your command handler code here
	char buffer [1000];
	IGNORE_RETURN (GetCurrentDirectory (1000, buffer));

	CString path = buffer;
	path += "/web/terrainEditor.html";
	IGNORE_RETURN (path.Replace ("dev\\win32", "exe\\win32"));

	Navigate2 (path);
}

//-------------------------------------------------------------------

void HelpView::OnButtonStop() 
{
	// TODO: Add your command handler code here
	Stop ();
}

//-------------------------------------------------------------------

void HelpView::OnButtonRefresh() 
{
	// TODO: Add your command handler code here
	Refresh ();	
}

//-------------------------------------------------------------------

void HelpView::OnInitialUpdate() 
{
	OnButtonHome ();
}

//-------------------------------------------------------------------

