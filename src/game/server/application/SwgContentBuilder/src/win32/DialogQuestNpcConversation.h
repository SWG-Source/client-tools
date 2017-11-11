// ======================================================================
//
// DialogQuestNpcConversation.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_DialogQuestNpcConversation_H
#define INCLUDED_DialogQuestNpcConversation_H

// ======================================================================

#include "Resource.h"

class Conversation;

// ======================================================================

class DialogQuestNpcConversation : public CDialog
{
public:

	enum ConversationType
	{
		CT_terse,
		CT_normal,
		CT_extended,
		CT_verbose
	};

public:

	DialogQuestNpcConversation(Conversation & conversation, ConversationType conversationType, CString const & questType, int questIndex, CWnd* pParent = NULL);   // standard constructor

	//{{AFX_DATA(DialogQuestNpcConversation)
	enum { IDD = IDD_DIALOG_QUESTCONVERSATION };
	CEdit	m_editCtrlPlayermore3;
	CEdit	m_editCtrlPlayermore2;
	CEdit	m_editCtrlPlayermore1;
	CEdit	m_editCtrlOtherescort;
	CEdit	m_editCtrlNpcmore3;
	CEdit	m_editCtrlNpcmore2;
	CEdit	m_editCtrlNpcmore1;
	CEdit	m_editCtrlNpctakeme;
	CEdit	m_editCtrlNpcdropoff;
	CEdit	m_editCtrlNpcsmuggle;
	CEdit	m_editCtrlGotowork;
	CString	m_editDontknowyou;
	CString	m_editGotowork;
	CString	m_editNpcbreech;
	CString	m_editNpcmore1;
	CString	m_editNpcmore2;
	CString	m_editNpcmore3;
	CString	m_editNpcsmuggle;
	CString	m_editPlayermore2;
	CString	m_editPlayermore3;
	CString	m_dontknowyou;
	CString	m_gotowork;
	CString	m_npc_breech;
	CString	m_npc_more_1;
	CString	m_npc_more_2;
	CString	m_npc_more_3;
	CString	m_npc_smuggle;
	CString	m_player_more_1;
	CString	m_player_more_2;
	CString	m_player_more_3;
	CString	m_editNpcdropoff;
	CString	m_editNpctakeme;
	CString	m_editOtherescort;
	CString	m_editPlayermore1;
	CString	m_npc_dropoff;
	CString	m_npc_takeme;
	CString	m_otherescort;
	CString	m_editReturnWaypointDescription;
	CString	m_editReturnWaypointName;
	CString	m_editWaypointDescription;
	CString	m_editWaypointName;
	CString	m_returnWaypointName;
	CString	m_returnWaypointDescription;
	CString	m_waypointDescription;
	CString	m_waypointName;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(DialogQuestNpcConversation)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(DialogQuestNpcConversation)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	CToolTipCtrl m_tooltip;

	Conversation & m_conversation;
	ConversationType const m_conversationType;
	CString const m_questType;
	int const m_questIndex;
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 

