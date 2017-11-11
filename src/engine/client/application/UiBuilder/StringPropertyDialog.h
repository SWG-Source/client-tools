#if !defined(AFX_STRINGPROPERTYDIALOG_H__B282C9B6_3AC4_4471_80C7_077B4FD2749B__INCLUDED_)
#define AFX_STRINGPROPERTYDIALOG_H__B282C9B6_3AC4_4471_80C7_077B4FD2749B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// StringPropertyDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// StringPropertyDialog dialog

#include "PropertyDialog.h"

class StringPropertyDialog : public PropertyDialog
{
// Construction
public:

	enum FormatRestriction
	{
		 FR_AnyText
		,FR_Integer
	};

	StringPropertyDialog(CWnd* pParent, CPoint anchorPosition, const char *i_initialValue, FormatRestriction i_textFormat);   // standard constructor

// Dialog Data
	//{{AFX_DATA(StringPropertyDialog)
	enum { IDD = IDD_STRING_PROPERTY_DIALOG };
	CButton	m_okButton;
	CButton	m_cancelButton;
	CEdit	m_editControl;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(StringPropertyDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation

public:

	void getStringProperty(CString &o_value);

protected:

	CString	m_stringProperty;

	void _positionControls();
	virtual void _getStartupRect(CRect &o_rect);
	void _initStringProperty(const char *i_value);

	FormatRestriction m_textFormat;

	// Generated message map functions
	//{{AFX_MSG(StringPropertyDialog)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STRINGPROPERTYDIALOG_H__B282C9B6_3AC4_4471_80C7_077B4FD2749B__INCLUDED_)
