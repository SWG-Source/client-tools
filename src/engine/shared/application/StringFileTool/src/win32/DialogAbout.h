// ======================================================================
//
// DialogAbout.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_DialogAbout_H
#define INCLUDED_DialogAbout_H

// ======================================================================

#include "Resource.h"

// ======================================================================

class DialogAbout : public CDialog
{
public:

	DialogAbout();

	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(DialogAbout)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

// ======================================================================

#endif
