#if !defined(AFX_TOOLTIPCOMBOBOX_H__41214ECF_3100_11D5_AB89_000000000000__INCLUDED_)
#define AFX_TOOLTIPCOMBOBOX_H__41214ECF_3100_11D5_AB89_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TooltipComboBox.h : header file
//

/* NOTE, this code was obtained from CodeGuru, from the article at
 http://codeguru.earthweb.com/combobox//TooltipComboBox.html
*/

/////////////////////////////////////////////////////////////////////////////
// CTooltipComboBox window
#include "TooltipListCtrl.h"

class CTooltipComboBox : public CComboBox
{
// Construction
public:
	CTooltipComboBox();

// Attributes
public:
\
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTooltipComboBox)
	protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL
	virtual int OnToolHitTest( CPoint point, TOOLINFO* pTI ) const;
	BOOL OnToolTipText( UINT id, NMHDR * pNMHDR, LRESULT * pResult );

// Implementation
public:
	CString GetComboTip( ) const;
	void SetComboTip( CString sTip );
	int SetItemTip( int nRow, CString sTip );
	BOOL GetDroppedState( ) const;
	int GetDroppedHeight( ) const;
	int GetDroppedWidth( ) const;
	int SetDroppedHeight( UINT nHeight );
	int SetDroppedWidth( UINT nWidth );
	void DisplayList( BOOL bDisplay = TRUE );
	virtual ~CTooltipComboBox();

protected:
	int m_nDroppedHeight;
	int m_nDroppedWidth;
	CTooltipListCtrl m_lstCombo;

	// Generated message map functions
protected:
	CString m_sTip;
	//{{AFX_MSG(CTooltipComboBox)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TOOLTIPCOMBOBOX_H__41214ECF_3100_11D5_AB89_000000000000__INCLUDED_)
