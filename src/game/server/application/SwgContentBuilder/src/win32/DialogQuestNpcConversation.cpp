// ======================================================================
//
// DialogQuestNpcConversation.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "FirstSwgContentBuilder.h"
#include "DialogQuestNpcConversation.h"

#include "Conversation.h"

// ======================================================================

DialogQuestNpcConversation::DialogQuestNpcConversation(Conversation & conversation, ConversationType const conversationType, CString const & questType, int const questIndex, CWnd* pParent /*=NULL*/)
	: CDialog(DialogQuestNpcConversation::IDD, pParent),
	m_conversation (conversation),
	m_conversationType (conversationType),
	m_questType (questType),
	m_questIndex (questIndex)
{
	//{{AFX_DATA_INIT(DialogQuestNpcConversation)
	m_editDontknowyou = _T("");
	m_editGotowork = _T("");
	m_editNpcbreech = _T("");
	m_editNpcmore1 = _T("");
	m_editNpcmore2 = _T("");
	m_editNpcmore3 = _T("");
	m_editNpcsmuggle = _T("");
	m_editPlayermore2 = _T("");
	m_editPlayermore3 = _T("");
	m_dontknowyou = _T("");
	m_gotowork = _T("");
	m_npc_breech = _T("");
	m_npc_more_1 = _T("");
	m_npc_more_2 = _T("");
	m_npc_more_3 = _T("");
	m_npc_smuggle = _T("");
	m_player_more_1 = _T("");
	m_player_more_2 = _T("");
	m_player_more_3 = _T("");
	m_editNpcdropoff = _T("");
	m_editNpctakeme = _T("");
	m_editOtherescort = _T("");
	m_editPlayermore1 = _T("");
	m_npc_dropoff = _T("");
	m_npc_takeme = _T("");
	m_otherescort = _T("");
	m_editReturnWaypointDescription = _T("");
	m_editReturnWaypointName = _T("");
	m_editWaypointDescription = _T("");
	m_editWaypointName = _T("");
	m_returnWaypointName = _T("");
	m_returnWaypointDescription = _T("");
	m_waypointDescription = _T("");
	m_waypointName = _T("");
	//}}AFX_DATA_INIT
}

// ----------------------------------------------------------------------

void DialogQuestNpcConversation::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogQuestNpcConversation)
	DDX_Control(pDX, IDC_EDIT_PLAYER_MORE_3, m_editCtrlPlayermore3);
	DDX_Control(pDX, IDC_EDIT_PLAYER_MORE_2, m_editCtrlPlayermore2);
	DDX_Control(pDX, IDC_EDIT_PLAYER_MORE_1, m_editCtrlPlayermore1);
	DDX_Control(pDX, IDC_EDIT_OTHERESCORT, m_editCtrlOtherescort);
	DDX_Control(pDX, IDC_EDIT_NPC_MORE_3, m_editCtrlNpcmore3);
	DDX_Control(pDX, IDC_EDIT_NPC_MORE_2, m_editCtrlNpcmore2);
	DDX_Control(pDX, IDC_EDIT_NPC_MORE_1, m_editCtrlNpcmore1);
	DDX_Control(pDX, IDC_EDIT_NPC_TAKEME, m_editCtrlNpctakeme);
	DDX_Control(pDX, IDC_EDIT_NPC_DROPOFF, m_editCtrlNpcdropoff);
	DDX_Control(pDX, IDC_EDIT_NPC_SMUGGLE, m_editCtrlNpcsmuggle);
	DDX_Control(pDX, IDC_EDIT_GOTOWORK, m_editCtrlGotowork);
	DDX_Text(pDX, IDC_EDIT_DONTKNOWYOU, m_editDontknowyou);
	DDX_Text(pDX, IDC_EDIT_GOTOWORK, m_editGotowork);
	DDX_Text(pDX, IDC_EDIT_NPC_BREECH, m_editNpcbreech);
	DDX_Text(pDX, IDC_EDIT_NPC_MORE_1, m_editNpcmore1);
	DDX_Text(pDX, IDC_EDIT_NPC_MORE_2, m_editNpcmore2);
	DDX_Text(pDX, IDC_EDIT_NPC_MORE_3, m_editNpcmore3);
	DDX_Text(pDX, IDC_EDIT_NPC_SMUGGLE, m_editNpcsmuggle);
	DDX_Text(pDX, IDC_EDIT_PLAYER_MORE_2, m_editPlayermore2);
	DDX_Text(pDX, IDC_EDIT_PLAYER_MORE_3, m_editPlayermore3);
	DDX_Text(pDX, IDC_STATIC_DONTKNOWYOU, m_dontknowyou);
	DDX_Text(pDX, IDC_STATIC_GOTOWORK, m_gotowork);
	DDX_Text(pDX, IDC_STATIC_NPC_BREECH, m_npc_breech);
	DDX_Text(pDX, IDC_STATIC_NPC_MORE_1, m_npc_more_1);
	DDX_Text(pDX, IDC_STATIC_NPC_MORE_2, m_npc_more_2);
	DDX_Text(pDX, IDC_STATIC_NPC_MORE_3, m_npc_more_3);
	DDX_Text(pDX, IDC_STATIC_NPC_SMUGGLE, m_npc_smuggle);
	DDX_Text(pDX, IDC_STATIC_PLAYER_MORE_1, m_player_more_1);
	DDX_Text(pDX, IDC_STATIC_PLAYER_MORE_2, m_player_more_2);
	DDX_Text(pDX, IDC_STATIC_PLAYER_MORE_3, m_player_more_3);
	DDX_Text(pDX, IDC_EDIT_NPC_DROPOFF, m_editNpcdropoff);
	DDX_Text(pDX, IDC_EDIT_NPC_TAKEME, m_editNpctakeme);
	DDX_Text(pDX, IDC_EDIT_OTHERESCORT, m_editOtherescort);
	DDX_Text(pDX, IDC_EDIT_PLAYER_MORE_1, m_editPlayermore1);
	DDX_Text(pDX, IDC_STATIC_NPC_DROPOFF, m_npc_dropoff);
	DDX_Text(pDX, IDC_STATIC_NPC_TAKEME, m_npc_takeme);
	DDX_Text(pDX, IDC_STATIC_OTHERESCORT, m_otherescort);
	DDX_Text(pDX, IDC_EDIT_RETURN_WAYPOINT_DESCRIPTION, m_editReturnWaypointDescription);
	DDX_Text(pDX, IDC_EDIT_RETURN_WAYPOINT_NAME, m_editReturnWaypointName);
	DDX_Text(pDX, IDC_EDIT_WAYPOINT_DESCRIPTION, m_editWaypointDescription);
	DDX_Text(pDX, IDC_EDIT_WAYPOINT_NAME, m_editWaypointName);
	DDX_Text(pDX, IDC_STATIC_RETURN_WAYPOIINT_NAME, m_returnWaypointName);
	DDX_Text(pDX, IDC_STATIC_RETURN_WAYPOINT_DESCRIPTION, m_returnWaypointDescription);
	DDX_Text(pDX, IDC_STATIC_WAYPOINT_DESCRIPTION, m_waypointDescription);
	DDX_Text(pDX, IDC_STATIC_WAYPOINT_NAME, m_waypointName);
	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(DialogQuestNpcConversation, CDialog)
	//{{AFX_MSG_MAP(DialogQuestNpcConversation)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

BOOL DialogQuestNpcConversation::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_tooltip.Create(this);
	m_tooltip.Activate(TRUE);

	// m_tooltip.AddTool(GetDlgItem(IDC_<name>), <string-table-id>);
	m_tooltip.AddTool (GetDlgItem (IDC_EDIT_NPC_BREECH), "Npc says something to attract the player");
	m_tooltip.AddTool (GetDlgItem (IDC_EDIT_GOTOWORK), "Npc says that you haven't brought the right item");
	m_tooltip.AddTool (GetDlgItem (IDC_EDIT_NPC_SMUGGLE), "Npc says, \"Thanks for delivering this\"");
	m_tooltip.AddTool (GetDlgItem (IDC_EDIT_NPC_TAKEME), "Npc says, \"Lead me to wherever\"");
	m_tooltip.AddTool (GetDlgItem (IDC_EDIT_NPC_DROPOFF), "Npc says, \"Thanks for leading me here\"");
	m_tooltip.AddTool (GetDlgItem (IDC_EDIT_OTHERESCORT), "Npc says, \"I'm the wrong escort Npc\"");
	m_tooltip.AddTool (GetDlgItem (IDC_EDIT_PLAYER_MORE_1), "Player says, \"Tell me more\"");
	m_tooltip.AddTool (GetDlgItem (IDC_EDIT_NPC_MORE_1), "Npc gives more info");
	m_tooltip.AddTool (GetDlgItem (IDC_EDIT_PLAYER_MORE_2), "Player says, \"Tell me even more\"");
	m_tooltip.AddTool (GetDlgItem (IDC_EDIT_NPC_MORE_2), "Npc gives even more info");
	m_tooltip.AddTool (GetDlgItem (IDC_EDIT_PLAYER_MORE_3), "Player says, \"Tell me a bit more\"");
	m_tooltip.AddTool (GetDlgItem (IDC_EDIT_NPC_MORE_3), "Npc gives a bit more info");
	m_tooltip.AddTool (GetDlgItem (IDC_EDIT_DONTKNOWYOU), "Npc says this to non-mission players");
	
	// TODO: Add extra initialization here
	m_dontknowyou.Format ("dontknowyou_%i", m_questIndex);
	m_gotowork.Format ("gotowork_%i", m_questIndex);
	m_npc_breech.Format ("npc_breech_%i", m_questIndex);
	m_npc_more_1.Format ("npc_more_1_%i", m_questIndex);
	m_npc_more_2.Format ("npc_more_2_%i", m_questIndex);
	m_npc_more_3.Format ("npc_more_3_%i", m_questIndex);
	m_npc_smuggle.Format ("npc_smuggle_%i", m_questIndex);
	m_player_more_1.Format ("player_more_1_%i", m_questIndex);
	m_player_more_2.Format ("player_more_2_%i", m_questIndex);
	m_player_more_3.Format ("player_more_3_%i", m_questIndex);
	m_npc_dropoff.Format ("npc_dropoff_%i", m_questIndex);
	m_npc_takeme.Format ("npc_takeme_%i", m_questIndex);
	m_otherescort.Format ("otherescort_%i", m_questIndex);
	m_waypointName.Format ("waypoint_name_%i", m_questIndex);
	m_waypointDescription.Format ("waypoint_description_%i", m_questIndex);
	m_returnWaypointName.Format ("return_waypoint_name_%i", m_questIndex);
	m_returnWaypointDescription.Format ("return_waypoint_description_%i", m_questIndex);

	if (!(m_questType == "deliver" || m_questType == "smuggle" || m_questType == "retrieve"))
	{
		m_editCtrlNpcsmuggle.EnableWindow (false);
		m_editCtrlGotowork.EnableWindow (false);
	}

	if (!(m_questType == "arrest" || m_questType == "escort" || m_questType == "rescue"))
	{
		m_editCtrlNpcdropoff.EnableWindow (false);
		m_editCtrlNpctakeme.EnableWindow (false);
		m_editCtrlOtherescort.EnableWindow (false);
	}

	switch (m_conversationType)
	{
	case CT_terse:
		m_editCtrlNpcmore1.EnableWindow (false);
		m_editCtrlNpcmore2.EnableWindow (false);
		m_editCtrlNpcmore3.EnableWindow (false);
		m_editCtrlPlayermore1.EnableWindow (false);
		m_editCtrlPlayermore2.EnableWindow (false);
		m_editCtrlPlayermore3.EnableWindow (false);
		break;

	case CT_normal:
		m_editCtrlNpcmore2.EnableWindow (false);
		m_editCtrlNpcmore3.EnableWindow (false);
		m_editCtrlPlayermore2.EnableWindow (false);
		m_editCtrlPlayermore3.EnableWindow (false);
		break;

	case CT_extended:
		m_editCtrlNpcmore3.EnableWindow (false);
		m_editCtrlPlayermore3.EnableWindow (false);
		break;
	}

	//-- fill out strings
	CString key;
	key.Format ("dontknowyou_%i", m_questIndex);
	m_editDontknowyou = m_conversation.getString (std::string (key)).c_str ();
	key.Format ("gotowork_%i", m_questIndex);
	m_editGotowork = m_conversation.getString (std::string (key)).c_str ();
	key.Format ("npc_breech_%i", m_questIndex);
	m_editNpcbreech = m_conversation.getString (std::string (key)).c_str ();
	key.Format ("npc_more_1_%i", m_questIndex);
	m_editNpcmore1 = m_conversation.getString (std::string (key)).c_str ();
	key.Format ("npc_more_2_%i", m_questIndex);
	m_editNpcmore2 = m_conversation.getString (std::string (key)).c_str ();
	key.Format ("npc_more_3_%i", m_questIndex);
	m_editNpcmore3 = m_conversation.getString (std::string (key)).c_str ();
	key.Format ("npc_smuggle_%i", m_questIndex);
	m_editNpcsmuggle = m_conversation.getString (std::string (key)).c_str ();
	key.Format ("player_more_1_%i", m_questIndex);
	m_editPlayermore1 = m_conversation.getString (std::string (key)).c_str ();
	key.Format ("player_more_2_%i", m_questIndex);
	m_editPlayermore2 = m_conversation.getString (std::string (key)).c_str ();
	key.Format ("player_more_3_%i", m_questIndex);
	m_editPlayermore3 = m_conversation.getString (std::string (key)).c_str ();
	key.Format ("npc_dropoff_%i", m_questIndex);
	m_editNpcdropoff = m_conversation.getString (std::string (key)).c_str ();
	key.Format ("npc_takeme_%i", m_questIndex);
	m_editNpctakeme = m_conversation.getString (std::string (key)).c_str ();
	key.Format ("otherescort_%i", m_questIndex);
	m_editOtherescort = m_conversation.getString (std::string (key)).c_str ();
	key.Format ("waypoint_name_%i", m_questIndex);
	m_editWaypointName = m_conversation.getString (std::string (key)).c_str ();
	key.Format ("waypoint_description_%i", m_questIndex);
	m_editWaypointDescription = m_conversation.getString (std::string (key)).c_str ();
	key.Format ("return_waypoint_name_%i", m_questIndex);
	m_editReturnWaypointName = m_conversation.getString (std::string (key)).c_str ();
	key.Format ("return_waypoint_description_%i", m_questIndex);
	m_editReturnWaypointDescription = m_conversation.getString (std::string (key)).c_str ();

	UpdateData (false);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ----------------------------------------------------------------------

void DialogQuestNpcConversation::OnOK() 
{
	UpdateData (true);

	CString key;
	key.Format ("dontknowyou_%i", m_questIndex);
	m_conversation.setString (std::string (key), std::string (m_editDontknowyou));
	key.Format ("gotowork_%i", m_questIndex);
	m_conversation.setString (std::string (key), std::string (m_editGotowork));
	key.Format ("npc_breech_%i", m_questIndex);
	m_conversation.setString (std::string (key), std::string (m_editNpcbreech));
	key.Format ("npc_more_1_%i", m_questIndex);
	m_conversation.setString (std::string (key), std::string (m_editNpcmore1));
	key.Format ("npc_more_2_%i", m_questIndex);
	m_conversation.setString (std::string (key), std::string (m_editNpcmore2));
	key.Format ("npc_more_3_%i", m_questIndex);
	m_conversation.setString (std::string (key), std::string (m_editNpcmore3));
	key.Format ("npc_smuggle_%i", m_questIndex);
	m_conversation.setString (std::string (key), std::string (m_editNpcsmuggle));
	key.Format ("player_more_1_%i", m_questIndex);
	m_conversation.setString (std::string (key), std::string (m_editPlayermore1));
	key.Format ("player_more_2_%i", m_questIndex);
	m_conversation.setString (std::string (key), std::string (m_editPlayermore2));
	key.Format ("player_more_3_%i", m_questIndex);
	m_conversation.setString (std::string (key), std::string (m_editPlayermore3));
	key.Format ("npc_dropoff_%i", m_questIndex);
	m_conversation.setString (std::string (key), std::string (m_editNpcdropoff));
	key.Format ("npc_takeme_%i", m_questIndex);
	m_conversation.setString (std::string (key), std::string (m_editNpctakeme));
	key.Format ("otherescort_%i", m_questIndex);
	m_conversation.setString (std::string (key), std::string (m_editOtherescort));
	key.Format ("waypoint_name_%i", m_questIndex);
	m_conversation.setString (std::string (key), std::string (m_editWaypointName));
	key.Format ("waypoint_description_%i", m_questIndex);
	m_conversation.setString (std::string (key), std::string (m_editWaypointDescription));
	key.Format ("return_waypoint_name_%i", m_questIndex);
	m_conversation.setString (std::string (key), std::string (m_editReturnWaypointName));
	key.Format ("return_waypoint_description_%i", m_questIndex);
	m_conversation.setString (std::string (key), std::string (m_editReturnWaypointDescription));
	
	CDialog::OnOK();
}

// ----------------------------------------------------------------------

BOOL DialogQuestNpcConversation::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	m_tooltip.RelayEvent(pMsg);
	
	return CDialog::PreTranslateMessage(pMsg);
}

// ======================================================================

