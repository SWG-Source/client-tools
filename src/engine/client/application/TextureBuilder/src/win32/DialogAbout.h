// ======================================================================
//
// DialogAbout.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef DIALOG_ABOUT_H
#define DIALOG_ABOUT_H

// ======================================================================

#include "resource.h"

// ======================================================================

class CAboutDlg : public CDialog
{
public:

	CAboutDlg();

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// Dialog Data
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	/*
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA
	*/

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// ClassWizard generated virtual function overrides
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// ClassWizard generated message handlers
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

// ======================================================================

#endif
