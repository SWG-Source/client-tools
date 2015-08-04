// UnicodeTesterDlg.h : header file
//

#if !defined(AFX_TESTERDLG_H__47AD596A_5873_410D_BEB0_6449F7632321__INCLUDED_)
#define AFX_TESTERDLG_H__47AD596A_5873_410D_BEB0_6449F7632321__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CUnicodeTesterDlg dialog

class CUnicodeTesterDlg : public CDialog
{
// Construction
public:
	CUnicodeTesterDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CUnicodeTesterDlg)
	enum { IDD = IDD_TESTER_DIALOG };
	CListCtrl	m_listCharData;
	CListCtrl	m_listBlocks;
	CString	m_curFilename;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUnicodeTesterDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CUnicodeTesterDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButtonLoad();
	virtual void OnCancel();
	afx_msg void OnSelchangeTab2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButtonChangeFile();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESTERDLG_H__47AD596A_5873_410D_BEB0_6449F7632321__INCLUDED_)

