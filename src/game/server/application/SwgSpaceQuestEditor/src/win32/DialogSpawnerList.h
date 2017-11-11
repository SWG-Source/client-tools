// ======================================================================
//
// DialogSpawnerList.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_DialogSpawnerList_H
#define INCLUDED_DialogSpawnerList_H

// ======================================================================

#include "Resource.h"

// ======================================================================

class DialogSpawnerList : public CDialog
{
public:

	DialogSpawnerList(CString const & spawnerList);

	CString const & getSpawnerList() const;

	//{{AFX_DATA(DialogSpawnerList)
	enum { IDD = IDD_DIALOG_NAVPOINTLIST };
	CListBox	m_spawnerListBox;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(DialogSpawnerList)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    
	virtual void OnOK();
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(DialogSpawnerList)
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

	CString m_spawnerList;
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
