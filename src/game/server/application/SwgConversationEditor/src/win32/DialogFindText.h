// ======================================================================
//
// DialogFindText.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_DialogFindText_H
#define INCLUDED_DialogFindText_H

// ======================================================================

#include "Resource.h"

// ======================================================================

class DialogFindText : public CDialog
{
public:

	DialogFindText();

	//{{AFX_DATA(DialogFindText)
	enum { IDD = IDD_EDIT_FINDTEXT };
	CButton	m_okButton;
	BOOL	m_matchCase;
	CString	m_text;
	BOOL	m_wholeWord;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(DialogFindText)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(DialogFindText)
	virtual void OnOK();
	afx_msg void OnChangeEditText();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
