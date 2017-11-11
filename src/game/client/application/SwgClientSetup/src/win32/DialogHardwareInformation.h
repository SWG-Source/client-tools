// ======================================================================
//
// DialogHardwareInformation.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_DialogHardwareInformation_H
#define INCLUDED_DialogHardwareInformation_H

// ======================================================================

#include "Resource.h"

// ======================================================================

class DialogHardwareInformation : public CDialog
{
public:

	DialogHardwareInformation(CWnd* pParent = NULL);   

	//{{AFX_DATA(DialogHardwareInformation)
	enum { IDD = IDD_DIALOG_HARDWAREINFORMATION };
	int		m_radio;

	CString	m_lblSendHardwareInfo;
	CString m_lblQuery;
	CString m_lblPleaseSend;
	CString m_lblPleaseDoNotSend;

	CString m_lblBtnCancel;
	CString m_lblBtnNext;

	//}}AFX_DATA

	//{{AFX_VIRTUAL(DialogHardwareInformation)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(DialogHardwareInformation)
	afx_msg void OnButtonProceed();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
