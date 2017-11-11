// ======================================================================
//
// DialogSpaceMobileList.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_DialogSpaceMobileList_H
#define INCLUDED_DialogSpaceMobileList_H

// ======================================================================

#include "Resource.h"

// ======================================================================

class DialogSpaceMobileList : public CDialog
{
public:

	DialogSpaceMobileList(CString const & spaceMobileList);

	CString const & getSpaceMobileList() const;

	//{{AFX_DATA(DialogSpaceMobileList)
	enum { IDD = IDD_DIALOG_SPACEMOBILELIST };
	CListBox	m_spaceMobileListBox;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(DialogSpaceMobileList)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    
	virtual void OnOK();
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(DialogSpaceMobileList)
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonAdd();
	afx_msg void OnButtonDown();
	afx_msg void OnButtonRemove();
	afx_msg void OnButtonRemoveall();
	afx_msg void OnButtonUp();
	afx_msg void OnButtonEdit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	CString m_spaceMobileList;
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
