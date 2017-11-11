// ======================================================================
//
// DialogStringIdList.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_DialogStringIdList_H
#define INCLUDED_DialogStringIdList_H

// ======================================================================

#include "Resource.h"

// ======================================================================

class DialogStringIdList : public CDialog
{
public:

	DialogStringIdList(CString const & missionTemplateType, StringList const & stringIdList);

	StringList const & getStringIdList() const;

	//{{AFX_DATA(DialogStringIdList)
	enum { IDD = IDD_DIALOG_STRINGIDLIST };
	CListBox	m_stringIdListBox;
	CString	m_stringId;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(DialogStringIdList)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    
	virtual void OnOK();
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(DialogStringIdList)
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonAdd();
	afx_msg void OnButtonRemove();
	afx_msg void OnButtonRemoveall();
	afx_msg void OnSelchangeListStringid();
	afx_msg void OnButtonChange();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	CString const m_missionTemplateType;
	StringList m_stringIdList;
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
