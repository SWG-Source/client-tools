//===================================================================
//
// DialogFind.h
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_DialogFind_H
#define INCLUDED_DialogFind_H

//===================================================================

#include "Resource.h"

//===================================================================

class DialogFind : public CDialog
{
public:

	DialogFind(CWnd* pParent = NULL);   

	//{{AFX_DATA(DialogFind)
	enum { IDD = IDD_DIALOG_FIND };
	CString	m_name;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(DialogFind)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(DialogFind)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//===================================================================

//{{AFX_INSERT_LOCATION}}

//===================================================================

#endif 
