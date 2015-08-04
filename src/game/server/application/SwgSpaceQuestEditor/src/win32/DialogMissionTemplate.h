// ======================================================================
//
// DialogMissionTemplate.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_DialogMissionTemplate_H
#define INCLUDED_DialogMissionTemplate_H

// ======================================================================

#include "Resource.h"

// ======================================================================

class DialogMissionTemplate : public CDialog
{
public:

	DialogMissionTemplate(CWnd* pParent = NULL);

	CString const & getResult() const;

	//{{AFX_DATA(DialogMissionTemplate)
	enum { IDD = IDD_DIALOG_MISSIONTEMPLATE };
	CListBox	m_missionTemplateListBox;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(DialogMissionTemplate)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual void OnOK();
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(DialogMissionTemplate)
	virtual BOOL OnInitDialog();
	afx_msg void OnDblclkListMissiontemplate();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	CString m_result;
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
