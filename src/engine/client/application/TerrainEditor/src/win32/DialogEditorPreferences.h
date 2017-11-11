//
// DialogEditorPreferences.cpp
// asommers 
//
// copyright 2000, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_DialogEditorPreferences_H
#define INCLUDED_DialogEditorPreferences_H

//-------------------------------------------------------------------

#include "resource.h"

//-------------------------------------------------------------------

class DialogEditorPreferences : public CDialog
{
protected:

	CToolTipCtrl m_tooltip;

protected:

	//{{AFX_MSG(DialogEditorPreferences)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

protected:

	//{{AFX_VIRTUAL(DialogEditorPreferences)
	protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

public:

//lint -save -e1925 

	//{{AFX_DATA(DialogEditorPreferences)
	enum { IDD = IDD_DIALOG_PREFERENCES };
	float	m_maxHeight;
	float	m_minHeight;
	//}}AFX_DATA

//lint -restore

public:

	explicit DialogEditorPreferences(CWnd* pParent = NULL);
};

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

//-------------------------------------------------------------------

#endif 
