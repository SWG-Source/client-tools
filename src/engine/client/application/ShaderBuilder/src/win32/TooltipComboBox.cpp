// TooltipComboBox.cpp : implementation file
//

/* NOTE, this code was obtained from CodeGuru, from the article at
 http://codeguru.earthweb.com/combobox//TooltipComboBox.html
*/

#include "FirstShaderBuilder.h"
#include "TooltipComboBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTooltipComboBox

CTooltipComboBox::CTooltipComboBox()
{
	m_nDroppedWidth = 0;
	m_nDroppedHeight = 0;
}

CTooltipComboBox::~CTooltipComboBox()
{
}


BEGIN_MESSAGE_MAP(CTooltipComboBox, CComboBox)
	//{{AFX_MSG_MAP(CTooltipComboBox)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipText)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipText)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTooltipComboBox message handlers

void CTooltipComboBox::OnLButtonDown(UINT nFlags, CPoint point) 
{
	UNREF(nFlags);
	UNREF(point);
	DisplayList( !m_lstCombo.IsWindowVisible( ) );
}

void CTooltipComboBox::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	OnLButtonDown( nFlags, point );
}

void CTooltipComboBox::DisplayList( BOOL bDisplay/* = TRUE*/ )
{
	if ( !bDisplay )	{
		m_lstCombo.ShowWindow( SW_HIDE );

		return;
	}

	CRect rc;
	GetWindowRect( rc );
	rc.top = rc.bottom;
	rc.right = rc.left + GetDroppedWidth( );
	rc.bottom = rc.top + GetDroppedHeight( );

	m_lstCombo.Display( rc );
}

int CTooltipComboBox::GetDroppedHeight() const
{
	return m_nDroppedHeight;
}

int CTooltipComboBox::GetDroppedWidth() const
{
	return m_nDroppedWidth;
}

int CTooltipComboBox::SetDroppedHeight( UINT nHeight )
{
	m_nDroppedHeight = nHeight;

	return m_nDroppedHeight;
}

int CTooltipComboBox::SetDroppedWidth( UINT nWidth )
{
	m_nDroppedWidth = nWidth;

	return m_nDroppedWidth;
}

BOOL CTooltipComboBox::GetDroppedState() const
{
	return m_lstCombo.IsWindowVisible( );
}

void CTooltipComboBox::PreSubclassWindow() 
{
	CRect rc( 0, 0, 10, 10 );

	DWORD dwStyle =   WS_BORDER 
						| LVS_REPORT 
						| LVS_NOCOLUMNHEADER 
						| LVS_SINGLESEL 
						| LVS_OWNERDATA
						;
	DWORD dwExStyle = WS_POPUP;

#if _MSC_VER < 1300
	m_lstCombo.CreateEx( 0, WC_LISTVIEW, NULL, dwStyle, rc, this, 0 );
#else
	m_lstCombo.CreateEx(dwExStyle,dwStyle,rc,this,0);
#endif

	DWORD dwStyleEx = m_lstCombo.GetExtendedStyle( );
	m_lstCombo.SetExtendedStyle( dwStyleEx | LVS_EX_FULLROWSELECT );
	m_lstCombo.Init( this );

	CRect rcAll;
	GetDroppedControlRect( &rcAll );
	GetWindowRect( &rc );
	SetDroppedWidth( rcAll.Width( ) );
	SetDroppedHeight( rcAll.Height( ) - rc.Height( ) );

	CComboBox::PreSubclassWindow( );

	EnableToolTips( );
}

int CTooltipComboBox::OnToolHitTest( CPoint point, TOOLINFO* pTI ) const
{
	UNREF(point);
	pTI->hwnd = m_hWnd;
	pTI->uId = ( UINT )1;
	pTI->lpszText = LPSTR_TEXTCALLBACK;

	CRect rc;
	GetWindowRect( &rc );
	ScreenToClient( &rc );
	pTI->rect = rc;

	return pTI->uId;
}

BOOL CTooltipComboBox::OnToolTipText( UINT id, NMHDR * pNMHDR, LRESULT * pResult )
{
	UNREF(id);
	// need to handle both ANSI and UNICODE versions of the message
	TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
	TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;
	CString sTipText;
	UINT nID = pNMHDR->idFrom;

	if( nID == 0 )	  	// Notification in NT from automatically
		return FALSE;   	// created tooltip

	sTipText = GetComboTip( );

#ifndef _UNICODE
	if (pNMHDR->code == TTN_NEEDTEXTA)
		lstrcpyn(pTTTA->szText, sTipText, 80);
	else
		_mbstowcsz(pTTTW->szText, sTipText, 80);
#else
	if (pNMHDR->code == TTN_NEEDTEXTA)
		_wcstombsz(pTTTA->szText, sTipText, 80);
	else
		lstrcpyn(pTTTW->szText, sTipText, 80);
#endif
	*pResult = 0;

	return TRUE;    // message was handled
}

CString CTooltipComboBox::GetComboTip( ) const
{
	return m_sTip;
}

void CTooltipComboBox::SetComboTip( CString sTip )
{
	m_sTip = sTip;
}

int CTooltipComboBox::SetItemTip( int nRow, CString sTip )
{
	return m_lstCombo.SetItemTip( nRow, sTip );
}
