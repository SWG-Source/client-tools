// ======================================================================
//
// DialogMinidump.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_DialogMinidump_H
#define INCLUDED_DialogMinidump_H

// ======================================================================

#include "Resource.h"

// ======================================================================

class DialogMinidump : public CDialog
{
public:

	DialogMinidump(CWnd* pParent = NULL);   

	//{{AFX_DATA(DialogMinidump)
	enum { IDD = IDD_DIALOG_MINIDUMP };
	int		m_radio;

	CString m_lblInfo;
	CString m_lblQuery;
	CString m_lblPleaseSend;
	CString m_lblNeverSend;

	CString m_lblBtnCancel;
	CString m_lblBtnNext;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(DialogMinidump)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(DialogMinidump)
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonProceed();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 

