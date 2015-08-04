// ======================================================================
//
// DialogContact.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_DialogContact_H
#define INCLUDED_DialogContact_H

// ======================================================================

#include "Resource.h"

// ======================================================================
class DialogContact : public CDialog
{
public:

	DialogContact(CWnd* pParent = NULL); 

	//{{AFX_DATA(DialogContact)
	enum { IDD = IDD_DIALOG_CONTACT };
	int		m_radio;
	
	CString m_lblSOEContactInfo;
	CString m_lblSOEContactQuery;
	CString m_lblPleaseContact;
	CString m_lblPleaseDoNotContact;

	CString m_lblBtnCancel;
	CString m_lblBtnNext;

	//}}AFX_DATA

	//{{AFX_VIRTUAL(DialogContact)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    
	//}}AFX_VIRTUAL
	
protected:

	//{{AFX_MSG(DialogContact)
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonProceed();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
