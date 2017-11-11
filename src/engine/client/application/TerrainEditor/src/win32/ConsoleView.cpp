//
// ConsoleView.cpp
// asommers 10-9-2000
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "ConsoleView.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(ConsoleView, CEditView)

//-------------------------------------------------------------------

ConsoleView::ConsoleView() :
	CEditView (),
	m_font (0)
{
}

//-------------------------------------------------------------------

ConsoleView::~ConsoleView()
{
	m_font->DeleteObject ();
	delete m_font;
}

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(ConsoleView, CEditView)
	//{{AFX_MSG_MAP(ConsoleView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

#ifdef _DEBUG
void ConsoleView::AssertValid() const
{
	CEditView::AssertValid();
}

void ConsoleView::Dump(CDumpContext& dc) const
{
	CEditView::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

void ConsoleView::setConsoleMessage (const CString& newMessage)
{
	//-- set new message
	GetEditCtrl ().SetWindowText (newMessage);
	GetEditCtrl ().LineScroll (GetEditCtrl ().GetLineCount ());
}

//-------------------------------------------------------------------

void ConsoleView::OnInitialUpdate() 
{
	CEditView::OnInitialUpdate();

	if (!m_font)
	{
		m_font = new CFont ();
		m_font->CreateFont (
		   -12,                        // nHeight
		   0,                         // nWidth
		   0,                         // nEscapement
		   0,                         // nOrientation
		   FW_NORMAL,                 // nWeight
		   FALSE,                     // bItalic
		   FALSE,                     // bUnderline
		   0,                         // cStrikeOut
		   ANSI_CHARSET,              // nCharSet
		   OUT_DEFAULT_PRECIS,        // nOutPrecision
		   CLIP_DEFAULT_PRECIS,       // nClipPrecision
		   DEFAULT_QUALITY,           // nQuality
		   DEFAULT_PITCH | FF_MODERN, // nPitchAndFamily
		   "Courier New");                // lpszFacename
	}
	
	//-- make sure no one can edit this
	IGNORE_RETURN (GetEditCtrl ().SetReadOnly ());
	IGNORE_RETURN (GetEditCtrl ().SetFont (m_font, FALSE));
}

//-------------------------------------------------------------------

BOOL ConsoleView::PreCreateWindow(CREATESTRUCT& cs) 
{
	//-- make sure this is a multi-line control
	cs.style |= ES_MULTILINE;
	cs.style |= WS_VSCROLL;
	cs.style |= WS_HSCROLL;
	
	return CEditView::PreCreateWindow(cs);
}

//-------------------------------------------------------------------

