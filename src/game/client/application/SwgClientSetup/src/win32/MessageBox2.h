// MessageBox2.h : header file

#ifndef INCLUDED_MessageBox2_H
#define INCLUDED_MessageBox2_H

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// MessageBox2 dialog

class MessageBox2 : public CDialog
{
public:

	static void install(TCHAR const * registryFolder);

// Construction
public:
	MessageBox2(CString const & text, CWnd* pParent = NULL);

	void setOkayButton(CString const & text);
	void setCancelButton(CString const & text);
	void setWebButton(CString const & text, CString const & url);
	void setDoNotShowAgainCheckBox(CString const & text, CString const & registryKey);

// Dialog Data
	//{{AFX_DATA(MessageBox2)
	enum { IDD = IDD_MESSAGEBOX2 };
	CButton	m_cancel;
	CButton	m_okay;
	CButton	m_doNotAskAgain;
	CButton	m_goToWebPage;
	CString	m_message;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(MessageBox2)
	public:
	virtual int DoModal();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(MessageBox2)
	virtual void OnCancel();
	afx_msg void OnWeb();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CString m_okayText;
	CString m_cancelText;
	CString m_webText;
	CString m_url;
	CString m_doNotShowText;
	CString m_registryKey;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif 
