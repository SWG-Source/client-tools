// ======================================================================
//
// DialogMissionNpcConversation.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_DialogMissionNpcConversation_H
#define INCLUDED_DialogMissionNpcConversation_H

// ======================================================================

#include "Resource.h"

class Conversation;

// ======================================================================

class DialogMissionNpcConversation : public CDialog
{
public:

	enum ConversationType
	{
		CT_terse,
		CT_normal,
		CT_verbose
	};

public:

	DialogMissionNpcConversation(Conversation & conversation, ConversationType conversationType, int questIndex, CWnd* pParent = NULL);   // standard constructor

	//{{AFX_DATA(DialogMissionNpcConversation)
	enum { IDD = IDD_DIALOG_MISSIONCONVERSATION };
	CEdit	m_editCtrlNext;
	CEdit	m_editCtrlNotyet;
	CEdit	m_editCtrlPlayersorry;
	CEdit	m_editCtrlNpcreset;
	CEdit	m_editCtrlPlayerreset;
	CEdit	m_editCtrlCantwork;
	CEdit	m_editCtrlPlayer5;
	CEdit	m_editCtrlPlayer4;
	CEdit	m_editCtrlPlayer3;
	CEdit	m_editCtrlNpc6;
	CEdit	m_editCtrlNpc5;
	CEdit	m_editCtrlNpc4;
	CString	m_notit;
	CString	m_npc_1;
	CString	m_npc_2;
	CString	m_npc_3;
	CString	m_npc_4;
	CString	m_npc_5;
	CString	m_npc_6;
	CString	m_npc_backtowork;
	CString	m_npc_failure;
	CString	m_npc_noloc;
	CString	m_npc_reward;
	CString	m_npc_work;
	CString	m_player_1;
	CString	m_player_2;
	CString	m_player_3;
	CString	m_player_4;
	CString	m_player_5;
	CString	m_editNext;
	CString	m_editCantwork;
	CString	m_editNotit;
	CString	m_editNotyet;
	CString	m_editNpc1;
	CString	m_editNpc2;
	CString	m_editNpc3;
	CString	m_editNpc4;
	CString	m_editNpc5;
	CString	m_editNpc6;
	CString	m_editNpcbacktowork;
	CString	m_editNpcfailure;
	CString	m_editnpcnoloc;
	CString	m_editNpcreset;
	CString	m_editNpcwork;
	CString	m_editPlayer1;
	CString	m_editPlayer2;
	CString	m_editPlayer3;
	CString	m_editPlayer4;
	CString	m_editPlayer5;
	CString	m_editPlayerReset;
	CString	m_editPlayerSorry;
	CString	m_editNpcreward;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(DialogMissionNpcConversation)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(DialogMissionNpcConversation)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	CToolTipCtrl m_tooltip;

	Conversation & m_conversation;
	ConversationType const m_conversationType;
	int const              m_questIndex;
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
