// IncludeView.cpp : implementation file
//

#include "FirstShaderBuilder.h"
#include "shaderbuilder.h"
#include "IncludeView.h"

#include "Node.h"
#include "RecentDirectory.h"

#include <d3dx9.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CIncludeView

IMPLEMENT_DYNCREATE(CIncludeView, CFormView)

CIncludeView::CIncludeView()
	: CFormView(CIncludeView::IDD),
	m_font(0),
	m_includeNode(NULL)
{
	//{{AFX_DATA_INIT(CIncludeView)
	//}}AFX_DATA_INIT
}

CIncludeView::~CIncludeView()
{
	m_font->DeleteObject();
	delete m_font;
}

void CIncludeView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CIncludeView)
	DDX_Control(pDX, IDC_INCLUDE_SOURCE, m_source);
	DDX_Control(pDX, IDC_INCLUDE_FILENAME, m_fileName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CIncludeView, CFormView)
	//{{AFX_MSG_MAP(CIncludeView)
	ON_EN_CHANGE(IDC_INCLUDE_SOURCE, OnChangeSource)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIncludeView diagnostics

#ifdef _DEBUG
void CIncludeView::AssertValid() const
{
	CFormView::AssertValid();
}

void CIncludeView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CIncludeView message handlers


void CIncludeView::SetNode(CIncludeNode &node)
{
	m_includeNode = &node;

	m_fileName.SetWindowText(m_includeNode->m_absoluteFileName);
	m_source.SetWindowText(m_includeNode->m_text);
	m_source.LineScroll(m_includeNode->m_scroll);

#if _MSC_VER < 1300
	const DWORD INVALID_FILE_ATTRIBUTES = 0xffffffff;
#endif

	DWORD attributes = GetFileAttributes(m_includeNode->m_absoluteFileName);
	m_source.SetReadOnly(attributes == INVALID_FILE_ATTRIBUTES || (attributes &FILE_ATTRIBUTE_READONLY) != 0);
}

void CIncludeView::SwitchAway() 
{
	if (m_includeNode)
	{
		m_includeNode->m_scroll = m_source.GetFirstVisibleLine();
		m_includeNode = NULL;
	}
}

void CIncludeView::OnChangeSource() 
{
	m_source.GetWindowText(m_includeNode->m_text);
	m_includeNode->m_changed = true;
}

void CIncludeView::OnInitialUpdate() 
{
	CFormView::OnInitialUpdate();

	if (!m_font)
	{
		m_font = new CFont();
		m_font->CreateFont(
		   -14,                        // nHeight
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

	m_source.SetFont(m_font, FALSE);
}

void CIncludeView::OnSize(UINT nType, int cx, int cy) 
{
	CFormView::OnSize(nType, cx, cy);
	
	CFormView::OnSize(nType, cx, cy);

	if (cy < 600)
		cy = 600;
	if (m_includeNode)
	{
		RECT rect;
		m_source.GetWindowRect(&rect);
		m_source.MoveWindow(40, 110, cx - 80, cy - 160);
	}
	
}

BOOL CIncludeView::PreTranslateMessage(MSG* pMsg) 
{
	if ((pMsg->message == WM_KEYDOWN) && (pMsg->wParam == VK_TAB)) 
	{
		// get the char index of the caret position
		int nPos = LOWORD(m_source.CharFromPos(m_source.GetCaretPos()));

		// select zero chars
		m_source.SetSel(nPos, nPos);

		// then replace that selection with a TAB
		m_source.ReplaceSel("\t", TRUE);

		// no need to do a msg translation, so quit. that way no further processing gets done
		return TRUE;
	}

	//just let other massages to work normally
	return CFormView::PreTranslateMessage(pMsg);
}
