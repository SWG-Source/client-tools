#if !defined(AFX_PAGEFILEDIALOG_H__1042B582_5A42_4D08_9C8D_6B8954B4F99E__INCLUDED_)
#define AFX_PAGEFILEDIALOG_H__1042B582_5A42_4D08_9C8D_6B8954B4F99E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PageFileDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// PageFileDialog dialog

class PageFileDialog : public CDialog
{
// Construction
public:
	PageFileDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(PageFileDialog)
	enum { IDD = IDD_PAGEFILE };
	CEdit	m_pageFile;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(PageFileDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(PageFileDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnNewfile();
	afx_msg void OnParent();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PAGEFILEDIALOG_H__1042B582_5A42_4D08_9C8D_6B8954B4F99E__INCLUDED_)
