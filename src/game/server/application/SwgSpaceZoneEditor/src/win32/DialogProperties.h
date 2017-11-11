// ======================================================================
//
// DialogProperties.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_DialogProperties_H
#define INCLUDED_DialogProperties_H

// ======================================================================

#include "Resource.h"

// ======================================================================

class DialogProperties : public CDialog
{
public:

	DialogProperties(CString const & originalObjVars, CString const & currentObjVars, CString const & scripts);

	//{{AFX_DATA(DialogProperties)
	enum { IDD = IDD_DIALOG_PROPERTIES };
	CString	m_originalObjVars;
	CString	m_currentObjVars;
	CString	m_scripts;
	//}}AFX_DATA

protected:

	//{{AFX_VIRTUAL(DialogProperties)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(DialogProperties)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
