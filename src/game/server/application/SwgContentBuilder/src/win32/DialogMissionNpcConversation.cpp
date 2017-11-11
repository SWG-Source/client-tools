// ======================================================================
//
// DialogMissionNpcConversation.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "FirstSwgContentBuilder.h"
#include "DialogMissionNpcConversation.h"

#include "Conversation.h"

// ======================================================================

DialogMissionNpcConversation::DialogMissionNpcConversation(Conversation & conversation, ConversationType const conversationType, int const questIndex, CWnd* pParent /*=NULL*/)
	: CDialog(DialogMissionNpcConversation::IDD, pParent),
	m_conversation (conversation),
	m_conversationType (conversationType),
	m_questIndex (questIndex)
{
	//{{AFX_DATA_INIT(DialogMissionNpcConversation)
	m_notit = _T("");
	m_npc_1 = _T("");
	m_npc_2 = _T("");
	m_npc_3 = _T("");
	m_npc_4 = _T("");
	m_npc_5 = _T("");
	m_npc_6 = _T("");
	m_npc_backtowork = _T("");
	m_npc_failure = _T("");
	m_npc_noloc = _T("");
	m_npc_reward = _T("");
	m_npc_work = _T("");
	m_player_1 = _T("");
	m_player_2 = _T("");
	m_player_3 = _T("");
	m_player_4 = _T("");
	m_player_5 = _T("");
	m_editNext = _T("");
	m_editCantwork = _T("");
	m_editNotit = _T("");
	m_editNotyet = _T("");
	m_editNpc1 = _T("");
	m_editNpc2 = _T("");
	m_editNpc3 = _T("");
	m_editNpc4 = _T("");
	m_editNpc5 = _T("");
	m_editNpc6 = _T("");
	m_editNpcbacktowork = _T("");
	m_editNpcfailure = _T("");
	m_editnpcnoloc = _T("");
	m_editNpcreset = _T("");
	m_editNpcwork = _T("");
	m_editPlayer1 = _T("");
	m_editPlayer2 = _T("");
	m_editPlayer3 = _T("");
	m_editPlayer4 = _T("");
	m_editPlayer5 = _T("");
	m_editPlayerReset = _T("");
	m_editPlayerSorry = _T("");
	m_editNpcreward = _T("");
	//}}AFX_DATA_INIT
}

// ----------------------------------------------------------------------

void DialogMissionNpcConversation::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogMissionNpcConversation)
	DDX_Control(pDX, IDC_EDIT_NEXT, m_editCtrlNext);
	DDX_Control(pDX, IDC_EDIT_NOTYET, m_editCtrlNotyet);
	DDX_Control(pDX, IDC_EDIT_PLAYER_SORRY, m_editCtrlPlayersorry);
	DDX_Control(pDX, IDC_EDIT_NPC_RESET, m_editCtrlNpcreset);
	DDX_Control(pDX, IDC_EDIT_PLAYER_RESET, m_editCtrlPlayerreset);
	DDX_Control(pDX, IDC_EDIT_CANT_WORK, m_editCtrlCantwork);
	DDX_Control(pDX, IDC_EDIT_PLAYER_5, m_editCtrlPlayer5);
	DDX_Control(pDX, IDC_EDIT_PLAYER_4, m_editCtrlPlayer4);
	DDX_Control(pDX, IDC_EDIT_PLAYER_3, m_editCtrlPlayer3);
	DDX_Control(pDX, IDC_EDIT_NPC_6, m_editCtrlNpc6);
	DDX_Control(pDX, IDC_EDIT_NPC_5, m_editCtrlNpc5);
	DDX_Control(pDX, IDC_EDIT_NPC_4, m_editCtrlNpc4);
	DDX_Text(pDX, IDC_STATIC_NOTIT, m_notit);
	DDX_Text(pDX, IDC_STATIC_NPC_1, m_npc_1);
	DDX_Text(pDX, IDC_STATIC_NPC_2, m_npc_2);
	DDX_Text(pDX, IDC_STATIC_NPC_3, m_npc_3);
	DDX_Text(pDX, IDC_STATIC_NPC_4, m_npc_4);
	DDX_Text(pDX, IDC_STATIC_NPC_5, m_npc_5);
	DDX_Text(pDX, IDC_STATIC_NPC_6, m_npc_6);
	DDX_Text(pDX, IDC_STATIC_NPC_BACKTOWORK, m_npc_backtowork);
	DDX_Text(pDX, IDC_STATIC_NPC_FAILURE, m_npc_failure);
	DDX_Text(pDX, IDC_STATIC_NPC_NOLOC, m_npc_noloc);
	DDX_Text(pDX, IDC_STATIC_NPC_REWARD, m_npc_reward);
	DDX_Text(pDX, IDC_STATIC_NPC_WORK, m_npc_work);
	DDX_Text(pDX, IDC_STATIC_PLAYER_1, m_player_1);
	DDX_Text(pDX, IDC_STATIC_PLAYER_2, m_player_2);
	DDX_Text(pDX, IDC_STATIC_PLAYER_3, m_player_3);
	DDX_Text(pDX, IDC_STATIC_PLAYER_4, m_player_4);
	DDX_Text(pDX, IDC_STATIC_PLAYER_5, m_player_5);
	DDX_Text(pDX, IDC_EDIT_NEXT, m_editNext);
	DDX_Text(pDX, IDC_EDIT_CANT_WORK, m_editCantwork);
	DDX_Text(pDX, IDC_EDIT_NOTIT, m_editNotit);
	DDX_Text(pDX, IDC_EDIT_NOTYET, m_editNotyet);
	DDX_Text(pDX, IDC_EDIT_NPC_1, m_editNpc1);
	DDX_Text(pDX, IDC_EDIT_NPC_2, m_editNpc2);
	DDX_Text(pDX, IDC_EDIT_NPC_3, m_editNpc3);
	DDX_Text(pDX, IDC_EDIT_NPC_4, m_editNpc4);
	DDX_Text(pDX, IDC_EDIT_NPC_5, m_editNpc5);
	DDX_Text(pDX, IDC_EDIT_NPC_6, m_editNpc6);
	DDX_Text(pDX, IDC_EDIT_NPC_BACKTOWORK, m_editNpcbacktowork);
	DDX_Text(pDX, IDC_EDIT_NPC_FAILURE, m_editNpcfailure);
	DDX_Text(pDX, IDC_EDIT_NPC_NOLOC, m_editnpcnoloc);
	DDX_Text(pDX, IDC_EDIT_NPC_RESET, m_editNpcreset);
	DDX_Text(pDX, IDC_EDIT_NPC_WORK, m_editNpcwork);
	DDX_Text(pDX, IDC_EDIT_PLAYER_1, m_editPlayer1);
	DDX_Text(pDX, IDC_EDIT_PLAYER_2, m_editPlayer2);
	DDX_Text(pDX, IDC_EDIT_PLAYER_3, m_editPlayer3);
	DDX_Text(pDX, IDC_EDIT_PLAYER_4, m_editPlayer4);
	DDX_Text(pDX, IDC_EDIT_PLAYER_5, m_editPlayer5);
	DDX_Text(pDX, IDC_EDIT_PLAYER_RESET, m_editPlayerReset);
	DDX_Text(pDX, IDC_EDIT_PLAYER_SORRY, m_editPlayerSorry);
	DDX_Text(pDX, IDC_EDIT_NPC_REWARD, m_editNpcreward);
	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(DialogMissionNpcConversation, CDialog)
	//{{AFX_MSG_MAP(DialogMissionNpcConversation)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

BOOL DialogMissionNpcConversation::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_tooltip.Create(this);
	m_tooltip.Activate(TRUE);

	m_tooltip.AddTool (GetDlgItem (IDC_EDIT_NPC_1), "Npc greets Player and offers mission");
	m_tooltip.AddTool (GetDlgItem (IDC_EDIT_PLAYER_1), "Player accepts mission");
	m_tooltip.AddTool (GetDlgItem (IDC_EDIT_NPC_2), "Npc is happy Player has accepted mission (convo ends)");
	m_tooltip.AddTool (GetDlgItem (IDC_EDIT_PLAYER_2), "Player refuses mission");
	m_tooltip.AddTool (GetDlgItem (IDC_EDIT_NPC_3), "Npc is angry Player refused mission (convo ends)");
	m_tooltip.AddTool (GetDlgItem (IDC_EDIT_PLAYER_3), "Player asks for more information");
	m_tooltip.AddTool (GetDlgItem (IDC_EDIT_NPC_4), "Npc gives more information");
	m_tooltip.AddTool (GetDlgItem (IDC_EDIT_PLAYER_4), "Player has another question");
	m_tooltip.AddTool (GetDlgItem (IDC_EDIT_NPC_5), "Npc answers next question");
	m_tooltip.AddTool (GetDlgItem (IDC_EDIT_PLAYER_5), "Player has a 3rd Question");
	m_tooltip.AddTool (GetDlgItem (IDC_EDIT_NPC_6), "Npc answers 3rd Question");
	m_tooltip.AddTool (GetDlgItem (IDC_EDIT_NPC_6), "Npc answers 3rd Question");
	m_tooltip.AddTool (GetDlgItem (IDC_EDIT_NPC_REWARD), "Npc says Player completed mission and came back");
	m_tooltip.AddTool (GetDlgItem (IDC_EDIT_CANT_WORK), "Npc says Player has too many missions and can't accept another one");
	m_tooltip.AddTool (GetDlgItem (IDC_EDIT_NPC_WORK), "Npc says Player is on the mission and came back");
	m_tooltip.AddTool (GetDlgItem (IDC_EDIT_PLAYER_RESET), "Player wants to clear mission");
	m_tooltip.AddTool (GetDlgItem (IDC_EDIT_NPC_RESET), "Npc clears mission and says come back later");
	m_tooltip.AddTool (GetDlgItem (IDC_EDIT_PLAYER_SORRY), "Player will continue mission");
	m_tooltip.AddTool (GetDlgItem (IDC_EDIT_NPC_BACKTOWORK), "Npc says get back to work");
	m_tooltip.AddTool (GetDlgItem (IDC_EDIT_NOTIT), "Npc says Player has not delivered the right item");
	m_tooltip.AddTool (GetDlgItem (IDC_EDIT_NOTYET), "Npc says Player is gated and cannot take these missions yet");
	m_tooltip.AddTool (GetDlgItem (IDC_EDIT_NEXT), "Npc says Player has done all the missions");
	m_tooltip.AddTool (GetDlgItem (IDC_EDIT_NPC_NOLOC), "Npc says, I can't find a mission for you right now, come back later");
	m_tooltip.AddTool (GetDlgItem (IDC_EDIT_NPC_FAILURE), "Npc says, You failed this mission");
 
	// TODO: Add extra initialization here
	m_notit.Format ("notit_%i", m_questIndex);
	m_npc_1.Format ("npc_1_%i", m_questIndex);
	m_npc_2.Format ("npc_2_%i", m_questIndex);
	m_npc_3.Format ("npc_3_%i", m_questIndex);
	m_npc_4.Format ("npc_4_%i", m_questIndex);
	m_npc_5.Format ("npc_5_%i", m_questIndex);
	m_npc_6.Format ("npc_6_%i", m_questIndex);
	m_npc_backtowork.Format ("npc_backtowork_%i", m_questIndex);
	m_npc_failure.Format ("npc_failure_%i", m_questIndex);
	m_npc_noloc.Format ("npc_noloc_%i", m_questIndex);
	m_npc_reward.Format ("npc_reward_%i", m_questIndex);
	m_npc_work.Format ("npc_work_%i", m_questIndex);
	m_player_1.Format ("player_1_%i", m_questIndex);
	m_player_2.Format ("player_2_%i", m_questIndex);
	m_player_3.Format ("player_3_%i", m_questIndex);
	m_player_4.Format ("player_4_%i", m_questIndex);
	m_player_5.Format ("player_5_%i", m_questIndex);

	switch (m_conversationType)
	{
	case CT_terse:
		m_editCtrlPlayer5.EnableWindow (false);
		m_editCtrlPlayer4.EnableWindow (false);
		m_editCtrlPlayer3.EnableWindow (false);
		m_editCtrlNpc6.EnableWindow (false);
		m_editCtrlNpc5.EnableWindow (false);
		m_editCtrlNpc4.EnableWindow (false);
		break;

	case CT_normal:
		m_editCtrlPlayer5.EnableWindow (false);
		m_editCtrlPlayer4.EnableWindow (false);
		m_editCtrlNpc6.EnableWindow (false);
		m_editCtrlNpc5.EnableWindow (false);
		break;
	}

	if (m_questIndex != 1)
	{
		m_editCtrlNext.EnableWindow (false);
		m_editCtrlNotyet.EnableWindow (false);
		m_editCtrlPlayersorry.EnableWindow (false);
		m_editCtrlNpcreset.EnableWindow (false);
		m_editCtrlPlayerreset.EnableWindow (false);
		m_editCtrlCantwork.EnableWindow (false);
	}

	//-- fill out strings
	CString key;
	m_editNext = m_conversation.getString (std::string ("next")).c_str ();
	m_editCantwork = m_conversation.getString (std::string ("cant_work")).c_str ();
	key.Format ("notit_%i", m_questIndex);
	m_editNotit = m_conversation.getString (std::string (key)).c_str ();
	m_editNotyet = m_conversation.getString (std::string ("notyet")).c_str ();
	key.Format ("npc_1_%i", m_questIndex);
	m_editNpc1 = m_conversation.getString (std::string (key)).c_str ();
	key.Format ("npc_2_%i", m_questIndex);
	m_editNpc2 = m_conversation.getString (std::string (key)).c_str ();
	key.Format ("npc_3_%i", m_questIndex);
	m_editNpc3 = m_conversation.getString (std::string (key)).c_str ();
	key.Format ("npc_4_%i", m_questIndex);
	m_editNpc4 = m_conversation.getString (std::string (key)).c_str ();
	key.Format ("npc_5_%i", m_questIndex);
	m_editNpc5 = m_conversation.getString (std::string (key)).c_str ();
	key.Format ("npc_6_%i", m_questIndex);
	m_editNpc6 = m_conversation.getString (std::string (key)).c_str ();
	key.Format ("npc_backtowork_%i", m_questIndex);
	m_editNpcbacktowork = m_conversation.getString (std::string (key)).c_str ();
	key.Format ("npc_failure_%i", m_questIndex);
	m_editNpcfailure = m_conversation.getString (std::string (key)).c_str ();
	key.Format ("npc_noloc_%i", m_questIndex);
	m_editnpcnoloc = m_conversation.getString (std::string (key)).c_str ();
	m_editNpcreset = m_conversation.getString (std::string ("npc_reset")).c_str ();
	key.Format ("npc_reward_%i", m_questIndex);
	m_editNpcreward = m_conversation.getString (std::string (key)).c_str ();
	key.Format ("npc_work_%i", m_questIndex);
	m_editNpcwork = m_conversation.getString (std::string (key)).c_str ();
	key.Format ("player_1_%i", m_questIndex);
	m_editPlayer1 = m_conversation.getString (std::string (key)).c_str ();
	key.Format ("player_2_%i", m_questIndex);
	m_editPlayer2 = m_conversation.getString (std::string (key)).c_str ();
	key.Format ("player_3_%i", m_questIndex);
	m_editPlayer3 = m_conversation.getString (std::string (key)).c_str ();
	key.Format ("player_4_%i", m_questIndex);
	m_editPlayer4 = m_conversation.getString (std::string (key)).c_str ();
	key.Format ("player_5_%i", m_questIndex);
	m_editPlayer5 = m_conversation.getString (std::string (key)).c_str ();
	m_editPlayerReset = m_conversation.getString (std::string ("player_reset")).c_str ();
	m_editPlayerSorry = m_conversation.getString (std::string ("player_sorry")).c_str ();
	
	UpdateData (false);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ----------------------------------------------------------------------

void DialogMissionNpcConversation::OnOK() 
{
	UpdateData (true);

	CString key;
	m_conversation.setString (std::string ("next"), std::string (m_editNext));
	m_conversation.setString (std::string ("cant_work"), std::string (m_editCantwork));
	key.Format ("notit_%i", m_questIndex);
	m_conversation.setString (std::string (key), std::string (m_editNotit));
	m_conversation.setString (std::string ("notyet"), std::string (m_editNotyet));
	key.Format ("npc_1_%i", m_questIndex);
	m_conversation.setString (std::string (key), std::string (m_editNpc1));
	key.Format ("npc_2_%i", m_questIndex);
	m_conversation.setString (std::string (key), std::string (m_editNpc2));
	key.Format ("npc_3_%i", m_questIndex);
	m_conversation.setString (std::string (key), std::string (m_editNpc3));
	key.Format ("npc_4_%i", m_questIndex);
	m_conversation.setString (std::string (key), std::string (m_editNpc4));
	key.Format ("npc_5_%i", m_questIndex);
	m_conversation.setString (std::string (key), std::string (m_editNpc5));
	key.Format ("npc_6_%i", m_questIndex);
	m_conversation.setString (std::string (key), std::string (m_editNpc6));
	key.Format ("npc_backtowork_%i", m_questIndex);
	m_conversation.setString (std::string (key), std::string (m_editNpcbacktowork));
	key.Format ("npc_failure_%i", m_questIndex);
	m_conversation.setString (std::string (key), std::string (m_editNpcfailure));
	key.Format ("npc_noloc_%i", m_questIndex);
	m_conversation.setString (std::string (key), std::string (m_editnpcnoloc));
	m_conversation.setString (std::string ("npc_reset"), std::string (m_editNpcreset));
	key.Format ("npc_reward_%i", m_questIndex);
	m_conversation.setString (std::string (key), std::string (m_editNpcreward));
	key.Format ("npc_work_%i", m_questIndex);
	m_conversation.setString (std::string (key), std::string (m_editNpcwork));
	key.Format ("player_1_%i", m_questIndex);
	m_conversation.setString (std::string (key), std::string (m_editPlayer1));
	key.Format ("player_2_%i", m_questIndex);
	m_conversation.setString (std::string (key), std::string (m_editPlayer2));
	key.Format ("player_3_%i", m_questIndex);
	m_conversation.setString (std::string (key), std::string (m_editPlayer3));
	key.Format ("player_4_%i", m_questIndex);
	m_conversation.setString (std::string (key), std::string (m_editPlayer4));
	key.Format ("player_5_%i", m_questIndex);
	m_conversation.setString (std::string (key), std::string (m_editPlayer5));
	m_conversation.setString (std::string ("player_reset"), std::string (m_editPlayerReset));
	m_conversation.setString (std::string ("player_sorry"), std::string (m_editPlayerSorry));
	
	CDialog::OnOK();
}

// ----------------------------------------------------------------------

BOOL DialogMissionNpcConversation::PreTranslateMessage(MSG* pMsg) 
{
	m_tooltip.RelayEvent(pMsg);
	
	return CDialog::PreTranslateMessage(pMsg);
}

// ======================================================================

