// ======================================================================
//
// DialogFinish.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_DialogFinish_H
#define INCLUDED_DialogFinish_H

// ======================================================================

#include "Resource.h"

// ======================================================================
class DialogFinish : public CDialog
{
public:

	DialogFinish(CWnd* pParent = NULL);   

	//{{AFX_DATA(DialogFinish)
	enum { IDD = IDD_DIALOG_FINISH };
	CString m_information;

	CString m_lblBtnFinish;

	//}}AFX_DATA

	//{{AFX_VIRTUAL(DialogFinish)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(DialogFinish)
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonProceed();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
