// ======================================================================
//
// DialogSpaceMobile.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_DialogSpaceMobile_H
#define INCLUDED_DialogSpaceMobile_H

// ======================================================================

#include "Resource.h"

// ======================================================================

class DialogSpaceMobile : public CDialog
{
public:

	DialogSpaceMobile(CString const & spaceMobile);

	CString const & getSpaceMobile() const;

	//{{AFX_DATA(DialogSpaceMobile)
	enum { IDD = IDD_DIALOG_SPACEMOBILE };
	CListBox	m_spaceMobileListBox;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(DialogSpaceMobile)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(DialogSpaceMobile)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnDblclkListSpacemobile();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	CString m_spaceMobile;
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
