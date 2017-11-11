// ======================================================================
//
// DialogExtra.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_DialogExtra_H
#define INCLUDED_DialogExtra_H

// ======================================================================

#include "Resource.h"

class Quest;

// ======================================================================

class DialogExtra : public CDialog
{
public:

	DialogExtra(Quest * quest, CWnd* pParent = NULL);

	//{{AFX_DATA(DialogExtra)
	enum { IDD = IDD_DIALOG_EXTRA };
	CComboBox	m_comboSpawnAction4;
	CComboBox	m_comboSpawnAction3;
	CComboBox	m_comboSpawnAction2;
	CComboBox	m_comboSpawnAction1;
	CComboBox	m_comboNpc4Disposition;
	CComboBox	m_comboNpc3Disposition;
	CComboBox	m_comboNpc1Disposition;
	CComboBox	m_comboNpc2Disposition;
	CString	m_encounterDataTableName;
	CString	m_extraSpawnObjectTemplate;
	int		m_spawnDelay;
	int		m_spawnFrequency;
	int		m_spawnLimit;
	int		m_spawnRadius;
	CString	m_spawnScript;
	CString	m_extraNpc1Entry;
	CString	m_extraNpc2Entry;
	CString	m_extraNpc3Entry;
	CString	m_extraNpc4Entry;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(DialogExtra)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(DialogExtra)
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonBrowseencounterdatatable();
	afx_msg void OnButtonBrowseextraspawntemplate();
	afx_msg void OnButtonBrowsenpc1template();
	afx_msg void OnButtonBrowsenpc2template();
	afx_msg void OnButtonBrowsenpc3template();
	afx_msg void OnButtonBrowsenpc4template();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	Quest * const m_quest;
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
