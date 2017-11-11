// ======================================================================
//
// DialogRewards.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_DialogDialogRewards_H
#define INCLUDED_DialogDialogRewards_H

// ======================================================================

#include "Resource.h"

class Quest;

// ======================================================================

class DialogRewards : public CDialog
{
public:

	DialogRewards(Quest * quest, CWnd* pParent = NULL);

	//{{AFX_DATA(DialogRewards)
	enum { IDD = IDD_DIALOG_REWARD };
	CComboBox	m_comboFactionReward4;
	CComboBox	m_comboFactionReward3;
	CComboBox	m_comboFactionReward2;
	CComboBox	m_comboFactionReward1;
	int		m_credits;
	CString	m_reward1ObjectTemplate;
	CString	m_reward2ObjectTemplate;
	CString	m_reward3ObjectTemplate;
	CString	m_reward4ObjectTemplate;
	CString	m_reward1Objvar;
	CString	m_reward2Objvar;
	CString	m_reward3Objvar;
	CString	m_reward4Objvar;
	CString	m_reward4ObjvarValue;
	CString	m_reward3ObjvarValue;
	CString	m_reward2ObjvarValue;
	CString	m_reward1ObjvarValue;
	int		m_reward1FactionValue;
	int		m_reward2FactionValue;
	int		m_reward3FactionValue;
	int		m_reward4FactionValue;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(DialogRewards)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(DialogRewards)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnButtonBrowseobject1();
	afx_msg void OnButtonBrowseobject2();
	afx_msg void OnButtonBrowseobject3();
	afx_msg void OnButtonBrowseobject4();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	Quest * const m_quest;
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 

