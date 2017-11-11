#if !defined(AFX_OUTPUTDIALOG_H__0D3311C6_9C7A_422E_9834_68186DE6C613__INCLUDED_)
#define AFX_OUTPUTDIALOG_H__0D3311C6_9C7A_422E_9834_68186DE6C613__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OutputDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// OutputDialog dialog

class OutputDialog : public CDialog
{
	BOOL       initialized;

// Construction
public:
	OutputDialog(CWnd* pParent = NULL);   // standard constructor

	void clearMessage (void);
	void addMessage (const char* message);

// Dialog Data
	//{{AFX_DATA(OutputDialog)
	enum { IDD = IDD_OUTPUT };
	CEdit	m_outputWindow;
	CString	m_output;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(OutputDialog)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(OutputDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OUTPUTDIALOG_H__0D3311C6_9C7A_422E_9834_68186DE6C613__INCLUDED_)
