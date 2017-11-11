// ======================================================================
//
// DialogResource.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_DialogResource_H
#define INCLUDED_DialogResource_H

// ======================================================================

#include "Resource.h"

// ======================================================================

class DialogResource : public CDialog
{
public:

	DialogResource(CWnd* pParent = NULL);   

	//{{AFX_DATA(DialogResource)
	enum { IDD = IDD_DIALOG_RESOURCE };
	CButton	m_okButton;
	CTreeCtrl	m_treeCtrl;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(DialogResource)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(DialogResource)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	CString m_selection;

private:

	CTreeCtrl & GetTreeCtrl ();
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 

