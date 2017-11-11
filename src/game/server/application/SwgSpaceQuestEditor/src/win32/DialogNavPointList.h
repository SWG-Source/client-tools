// ======================================================================
//
// DialogNavPointList.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_DialogNavPointList_H
#define INCLUDED_DialogNavPointList_H

// ======================================================================

#include "Resource.h"

// ======================================================================

class DialogNavPointList : public CDialog
{
public:

	DialogNavPointList(CString const & navPointList);

	CString const & getNavPointList() const;

	//{{AFX_DATA(DialogNavPointList)
	enum { IDD = IDD_DIALOG_NAVPOINTLIST };
	CListBox	m_navPointListBox;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(DialogNavPointList)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    
	virtual void OnOK();
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(DialogNavPointList)
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

	CString m_navPointList;
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
