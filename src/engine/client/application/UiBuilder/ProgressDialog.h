#if !defined(AFX_PROGRESSDIALOG_H__B93DCE94_2145_4D48_A875_D5FD5E1A524F__INCLUDED_)
#define AFX_PROGRESSDIALOG_H__B93DCE94_2145_4D48_A875_D5FD5E1A524F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProgressDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// ProgressDialog dialog

class ProgressDialog : public CDialog
{
// Construction
public:
	ProgressDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(ProgressDialog)
	enum { IDD = IDD_PROGRESS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ProgressDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(ProgressDialog)
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROGRESSDIALOG_H__B93DCE94_2145_4D48_A875_D5FD5E1A524F__INCLUDED_)
