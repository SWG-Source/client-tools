#if !defined(AFX_TOOLTIPLISTCTRL_H__41214ED5_3100_11D5_AB89_000000000000__INCLUDED_)
#define AFX_TOOLTIPLISTCTRL_H__41214ED5_3100_11D5_AB89_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TooltipListCtrl.h : header file
//

#include <afxtempl.h> 

/* NOTE, this code was obtained from CodeGuru, from the article at
 http://codeguru.earthweb.com/combobox//TooltipComboBox.html
*/

/////////////////////////////////////////////////////////////////////////////
// CTooltipListCtrl window

class CTooltipListCtrl : public CListCtrl
{
// Construction
public:
	CTooltipListCtrl();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTooltipListCtrl)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL
	virtual int OnToolHitTest( CPoint point, TOOLINFO* pTI ) const;

	BOOL OnToolTipText( UINT id, NMHDR * pNMHDR, LRESULT * pResult );

// Implementation
public:
	CString GetItemTip( int nRow ) const;
	int SetItemTip( int nRow, CString sTip );
	void Display( CRect rc );
	void Init( CComboBox *pComboParent );
	virtual ~CTooltipListCtrl();

protected:
	int m_nLastItem;
	CComboBox *m_pComboParent;
	CMap< int, int &, CString, CString & > m_mpItemToTip;

	// Generated message map functions
protected:
	//{{AFX_MSG(CTooltipListCtrl)
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TOOLTIPLISTCTRL_H__41214ED5_3100_11D5_AB89_000000000000__INCLUDED_)
