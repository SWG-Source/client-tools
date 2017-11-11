#if !defined(AFX_PROPERTYDIALOG_H__B089BBD2_7140_447B_8C29_1D541A5FE393__INCLUDED_)
#define AFX_PROPERTYDIALOG_H__B089BBD2_7140_447B_8C29_1D541A5FE393__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PropertyDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// PropertyDialog dialog

class PropertyDialog : public CDialog
{
// Construction
public:
	PropertyDialog(int ID, CPoint anchorPosition, CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(PropertyDialog)
	enum { IDD = IDD_PROPERTY_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(PropertyDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	virtual void OnOK();
	virtual void OnCancel(); 
	//}}AFX_VIRTUAL

// Implementation
protected:

	virtual void _getStartupRect(CRect &o_rect)=0;

	CPoint m_anchorPosition;

	// Generated message map functions
	//{{AFX_MSG(PropertyDialog)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROPERTYDIALOG_H__B089BBD2_7140_447B_8C29_1D541A5FE393__INCLUDED_)
