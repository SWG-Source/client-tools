//======================================================================
//
// SwgCuiChatRoomsWho.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiChatRoomsWho.h"

#include "UIButton.h"
#include "UIData.h"
#include "UIImageStyle.h"
#include "UIPage.h"
#include "UITable.h"
#include "UITableModelDefault.h"
#include "UIText.h"
#include "UIUtils.h"
#include "clientUserInterface/CuiActionManager.h"
#include "clientUserInterface/CuiActions.h"
#include "clientUserInterface/CuiChatManager.h"
#include "clientUserInterface/CuiChatRoomDataNode.h"
#include "clientUserInterface/CuiChatRoomManager.h"
#include "clientUserInterface/CuiChatRoomManagerStatusMessage.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiStringIds.h"
#include "sharedGame/SharedStringIds.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "swgClientUserInterface/SwgCuiChatRoomsBan.h"
#include "swgClientUserInterface/SwgCuiChatRoomsInvite.h"
#include <algorithm>
#include <map>
#include <vector>

//======================================================================

namespace SwgCuiChatRoomsWhoNamespace
{
	typedef stdvector<SwgCuiChatRoomsWho *>::fwd MediatorVector;
	MediatorVector s_mediators;

	typedef stdmap<Unicode::String, uint32>::fwd MemberMap;

	void insertMembers (MemberMap & members, const CuiChatRoomDataNode::AvatarVector & av, uint32 flag)
	{
		for (CuiChatRoomDataNode::AvatarVector::const_iterator it = av.begin (); it != av.end (); ++it)
		{
			const CuiChatAvatarId & avatar    = (*it);
			const ChatAvatarId & chatId       = avatar.chatId;
			const Unicode::String & shortName = CuiChatManager::getShortName (chatId);
			members.insert (std::make_pair (shortName, 0));
			members [shortName] |= flag;
		}
	}
}

using namespace SwgCuiChatRoomsWhoNamespace;

//----------------------------------------------------------------------

SwgCuiChatRoomsWho::SwgCuiChatRoomsWho        (UIPage & page) :
CuiMediator      ("SwgCuiChatRoomsWho", page),
UIEventCallback  (),
m_buttonChatRoom (0),
m_buttonDeOp     (0),
m_buttonInvite   (0),
m_buttonKick     (0),
m_buttonOk       (0),
m_buttonOp       (0),
m_buttonUninvite (0),
m_buttonBan      (0),
m_buttonUnban    (0),
m_table          (0),
m_textName       (0),
m_textCreator    (0),
m_textModerated  (0),
m_textOwner      (0),
m_textPrivate    (0),
m_textTitle      (0),
m_roomId         (0),
m_pathIconPresent   (),
m_pathIconInvited   (),
m_pathIconModerator (),
m_pathIconBanned    (),
m_pathIconInvitedBanned (),
m_callback          (new MessageDispatch::Callback)
{
	getCodeDataObject (TUIButton, m_buttonChatRoom,    "buttonChatRoom");
	getCodeDataObject (TUIButton, m_buttonDeOp,        "buttonDeOp");
	getCodeDataObject (TUIButton, m_buttonInvite,      "buttonInvite");
	getCodeDataObject (TUIButton, m_buttonKick,        "buttonKick");
	getCodeDataObject (TUIButton, m_buttonOk,          "buttonOk");
	getCodeDataObject (TUIButton, m_buttonOp,          "buttonOp");
	getCodeDataObject (TUIButton, m_buttonUninvite,    "buttonUninvite");
	getCodeDataObject (TUIButton, m_buttonBan,         "buttonBan");
	getCodeDataObject (TUIButton, m_buttonUnban,       "buttonUnban");
	getCodeDataObject (TUITable,  m_table,             "table");
	getCodeDataObject (TUIText,   m_textName,          "textName");
	getCodeDataObject (TUIText,   m_textCreator,       "textCreator");
	getCodeDataObject (TUIText,   m_textModerated,     "textModerated");
	getCodeDataObject (TUIText,   m_textOwner,         "textOwner");
	getCodeDataObject (TUIText,   m_textPrivate,       "textPrivate");
	getCodeDataObject (TUIText,   m_textTitle,         "textTitle");

	{
		UIImageStyle * imageStyle = 0;
		getCodeDataObject (TUIImageStyle, imageStyle, "iconPresent");
		m_pathIconPresent   = Unicode::narrowToWide (imageStyle->GetFullPath ());
		getCodeDataObject (TUIImageStyle, imageStyle, "iconInvited");
		m_pathIconInvited   = Unicode::narrowToWide (imageStyle->GetFullPath ());
		getCodeDataObject (TUIImageStyle, imageStyle, "iconModerator");
		m_pathIconModerator = Unicode::narrowToWide (imageStyle->GetFullPath ());
		getCodeDataObject (TUIImageStyle, imageStyle, "iconBanned");
		m_pathIconBanned    = Unicode::narrowToWide (imageStyle->GetFullPath ());
		getCodeDataObject (TUIImageStyle, imageStyle, "iconInvitedBanned");
		m_pathIconInvitedBanned = Unicode::narrowToWide (imageStyle->GetFullPath ());
	}

	m_textName->SetPreLocalized      (true);
	m_textCreator->SetPreLocalized   (true);
	m_textModerated->SetPreLocalized (true);
	m_textOwner->SetPreLocalized     (true);
	m_textPrivate->SetPreLocalized   (true);
	m_textTitle->SetPreLocalized     (true);

	m_textName->Clear      ();
	m_textTitle->Clear     ();
	m_textCreator->Clear   ();
	m_textOwner->Clear     ();
	m_textPrivate->Clear   ();
	m_textModerated->Clear ();

	registerMediatorObject (*m_buttonChatRoom,  true);
	registerMediatorObject (*m_buttonDeOp,      true);
	registerMediatorObject (*m_buttonInvite,    true);
	registerMediatorObject (*m_buttonKick,      true);
	registerMediatorObject (*m_buttonOk,        true);
	registerMediatorObject (*m_buttonOp,        true);
	registerMediatorObject (*m_buttonUninvite,  true);
	registerMediatorObject (*m_buttonBan,       true);
	registerMediatorObject (*m_buttonUnban,     true);
	registerMediatorObject (*m_table,           true);

	setState (MS_closeable);
	setState (MS_closeDeactivates);

	s_mediators.push_back (this);
}

//----------------------------------------------------------------------

SwgCuiChatRoomsWho::~SwgCuiChatRoomsWho ()
{
	s_mediators.erase (std::remove (s_mediators.begin (), s_mediators.end (), this), s_mediators.end ());
	delete m_callback;
	m_callback = 0;
}

//----------------------------------------------------------------------

SwgCuiChatRoomsWho *    SwgCuiChatRoomsWho::createInto          (UIPage & parent)
{
	UIPage * const dupe = NON_NULL (UIPage::DuplicateInto (parent, "/pda.chatroomWho"));
	return new SwgCuiChatRoomsWho (*dupe);
}

//----------------------------------------------------------------------

void SwgCuiChatRoomsWho::performActivate           ()
{
	CuiManager::requestPointer (true);

	m_callback->connect   (*this, &SwgCuiChatRoomsWho::onRoomModified, static_cast<CuiChatRoomManager::Messages::Modified *>(0));
	onRoomModified (m_roomId);
	OnGenericSelectionChanged (m_table);
}

//----------------------------------------------------------------------

void SwgCuiChatRoomsWho::performDeactivate         ()
{
	CuiManager::requestPointer (false);
	m_callback->disconnect (*this, &SwgCuiChatRoomsWho::onRoomModified, static_cast<CuiChatRoomManager::Messages::Modified *>(0));	

	CuiWorkspace * const ws = getContainingWorkspace ();
	if (ws)
	{
		SwgCuiChatRoomsInvite * const inviteMediator = SwgCuiChatRoomsInvite::findMediatorForRoom (*ws, m_roomId);
		SwgCuiChatRoomsBan * const banMediator       = SwgCuiChatRoomsBan::findMediatorForRoom    (*ws, m_roomId);

		if (inviteMediator)
			inviteMediator->deactivate ();
		if (banMediator)
			banMediator->deactivate ();
	}
}

//----------------------------------------------------------------------

void SwgCuiChatRoomsWho::OnButtonPressed           (UIWidget *context )
{
	if (m_buttonChatRoom      == context)
	{
		Unicode::String param;
		UIUtils::FormatInteger (param, m_roomId);
		CuiActionManager::performAction (CuiActions::chatRoomBrowser, param);
	}

	else if (m_buttonDeOp     == context)
	{
		op (false);
	}

	else if (m_buttonInvite   == context)
	{
		invite ();
	}

	else if (m_buttonKick     == context)
	{
		kick ();
	}

	else if (m_buttonOk       == context)
	{
		deactivate ();
	}

	else if (m_buttonOp       == context)
	{
		op (true);
	}

	else if (m_buttonUninvite == context)
	{
		uninvite ();
	}

	else if (m_buttonBan == context)
	{
		ban ();
	}

	else if (m_buttonUnban == context)
	{
		unban ();
	}
}

//----------------------------------------------------------------------

void SwgCuiChatRoomsWho::OnGenericSelectionChanged (UIWidget *context )
{
	if (context == m_table)
	{
		const CuiChatRoomDataNode * const node = CuiChatRoomManager::findRoomNode (m_roomId);
		if (!node)
		{
			WARNING (true, ("SwgCuiChatRoomsWho for unknown room [%d] closing", m_roomId));
//			deactivate ();
			return;
		}

		UITableModelDefault * const model = safe_cast<UITableModelDefault *>(m_table->GetTableModel ());
		NOT_NULL (model);
		const long selectedRow = m_table->GetLastSelectedRow ();
		Unicode::String selectedAvatar;	
		model->GetValueAtText (selectedRow, 0, selectedAvatar);

		const ChatAvatarId & selfId = CuiChatManager::getSelfAvatarId ();
		const bool playerIsOperator = (node->data.owner == selfId) || (node->data.creator == selfId) || node->isModerator (selfId.getFullName ());
		
		if (selectedAvatar.empty ())
		{
			m_buttonDeOp->SetVisible     (false);
			m_buttonOp->SetVisible       (false);
			m_buttonKick->SetVisible     (false);
			m_buttonUninvite->SetVisible (false);
			m_buttonUnban->SetVisible    (false);
		}
		else
		{
			const std::string & shortName = Unicode::wideToNarrow (selectedAvatar);

			const bool isMember    = node->isMember    (shortName);
			const bool isInvited   = node->isInvitee   (shortName);
			const bool isModerator = node->isModerator (shortName);
			const bool isBanned    = node->isBanned    (shortName);

			m_buttonDeOp->SetVisible     (playerIsOperator && isModerator);
			m_buttonOp->SetVisible       (playerIsOperator && !isModerator);

			m_buttonKick->SetVisible     (playerIsOperator && isMember);
			m_buttonUninvite->SetVisible (playerIsOperator && isInvited && !isMember);

			m_buttonBan->SetVisible      (!isBanned && playerIsOperator);
			m_buttonUnban->SetVisible    (isBanned && playerIsOperator);
		}

		m_buttonInvite->SetVisible (playerIsOperator);
	}
}

//----------------------------------------------------------------------

ChatAvatarId SwgCuiChatRoomsWho::getSelectedAvatar  () const
{
	UITableModelDefault * const model = safe_cast<UITableModelDefault *>(m_table->GetTableModel ());
	NOT_NULL (model);

	const long selectedRow = m_table->GetLastSelectedRow ();
	Unicode::String selectedAvatarName;	
	model->GetValueAtText (selectedRow, 0, selectedAvatarName);

	ChatAvatarId chatAvatarId;
	CuiChatManager::constructChatAvatarId (Unicode::wideToNarrow (selectedAvatarName), chatAvatarId);
	return chatAvatarId;
}

//----------------------------------------------------------------------

void SwgCuiChatRoomsWho::resetTable ()
{
	UITableModelDefault * const model = safe_cast<UITableModelDefault *>(m_table->GetTableModel ());
	NOT_NULL (model);

	const long oldSelectedRow = m_table->GetLastSelectedRow ();
	Unicode::String oldSelectedAvatar;	
	model->GetValueAtText (oldSelectedRow, 0, oldSelectedAvatar);
	long newSelectedRow = -1;  // no selection

	model->Attach (0);
	m_table->SetTableModel (0);

	// Clear the table data
	model->ClearData ();

	// Populate the table if we have data
	const CuiChatRoomDataNode * const node = CuiChatRoomManager::findRoomNode (m_roomId);
	if (node)
	{
		MemberMap members;

		const uint32 B_present   = 1;
		const uint32 B_invited   = 2;
		const uint32 B_moderator = 4;
		const uint32 B_banned    = 8;
		
		insertMembers (members, node->getMembers    (), B_present);
		insertMembers (members, node->getInvitees   (), B_invited);
		insertMembers (members, node->getModerators (), B_moderator);
		insertMembers (members, node->getBanned     (), B_banned);

		int index = 0;
		for (MemberMap::const_iterator it = members.begin (); it != members.end (); ++it, ++index)
		{
			const Unicode::String & shortName = (*it).first;
			const uint32 flags                = (*it).second;

			model->AppendCell (0, Unicode::wideToNarrow (shortName).c_str (), shortName);

			const bool isPresent   = (flags & B_present)   != 0;
			const bool isInvited   = (flags & B_invited)   != 0;
			const bool isModerator = (flags & B_moderator) != 0;
			const bool isBanned    = (flags & B_banned)    != 0;

			model->AppendCell (1, 0, isPresent   ? m_pathIconPresent   : Unicode::emptyString);

			Unicode::String inviteIconPath;

			if (isBanned)
			{
				//- this is an error state
				if (isInvited)
					inviteIconPath = m_pathIconInvitedBanned;
				else
					inviteIconPath = m_pathIconBanned;
			}
			else if (isInvited)
				inviteIconPath = m_pathIconInvited;

			model->AppendCell (2, 0, inviteIconPath);
			model->AppendCell (3, 0, isModerator ? m_pathIconModerator : Unicode::emptyString);

			if (oldSelectedAvatar == shortName)
				newSelectedRow = index;
		}
	}
	else
	{
		WARNING (true, ("SwgCuiChatRoomsWho::resetTable for unknown room [%d] closing", m_roomId));
//		deactivate ();
	}

	m_table->SetTableModel (model);
	model->Detach (0);

	m_table->SelectRow (newSelectedRow);
}

//----------------------------------------------------------------------

void SwgCuiChatRoomsWho::resetInfo ()
{
	const CuiChatRoomDataNode * const node = CuiChatRoomManager::findRoomNode (m_roomId);
	if (node)
	{
		m_textName->SetLocalText      (Unicode::narrowToWide (node->data.path));
		m_textTitle->SetLocalText     (node->data.title);
		m_textCreator->SetLocalText   (CuiChatManager::getShortName (node->data.creator));
		m_textOwner->SetLocalText     (CuiChatManager::getShortName (node->data.creator));
		m_textPrivate->SetLocalText   (node->data.roomType == CHAT_ROOM_PRIVATE ? SharedStringIds::yes.localize () : SharedStringIds::no.localize ());
		m_textModerated->SetLocalText (node->data.moderated != 0 ? SharedStringIds::yes.localize () : SharedStringIds::no.localize ());
	}
	else
	{
		WARNING (true, ("SwgCuiChatRoomsWho::resetInfo for unknown room [%d] closing", m_roomId));
//		deactivate ();

		// Clear the fields
		m_textName->Clear      ();
		m_textTitle->Clear     ();
		m_textCreator->Clear   ();
		m_textOwner->Clear     ();
		m_textPrivate->Clear   ();
		m_textModerated->Clear ();
	}
}

//----------------------------------------------------------------------

void SwgCuiChatRoomsWho::onRoomModified            (const uint32 & roomId)
{
	if (roomId != m_roomId)
		return;

	if (!CuiChatRoomManager::isRoomEntered (roomId))
	{
		WARNING (true, ("SwgCuiChatRoomsWho::onRoomModified for room [%d] not entered closing", m_roomId));
//		deactivate ();
		return;
	}

	resetInfo  ();
	resetTable ();
}

//----------------------------------------------------------------------

SwgCuiChatRoomsWho * SwgCuiChatRoomsWho::getMediatorForRoom (uint32 roomId)
{
	for (MediatorVector::const_iterator it = s_mediators.begin (); it != s_mediators.end (); ++it)
	{
		SwgCuiChatRoomsWho * const mediator = *it;

		if (mediator->m_roomId == roomId)
			return mediator;
	}

	return 0;
}

//----------------------------------------------------------------------

void SwgCuiChatRoomsWho::setRoomId                 (uint32 roomId)
{
	m_roomId = roomId;
	if (isActive ())
		onRoomModified (m_roomId);
}

//----------------------------------------------------------------------

void SwgCuiChatRoomsWho::op (bool b)
{
	const ChatAvatarId & avatar = getSelectedAvatar ();
	if (avatar.name.empty ())
		return;

	Unicode::String result;
	if (!CuiChatRoomManager::setModerator (m_roomId, avatar.getFullName (), b, result))
		CuiMessageBox::createInfoBox (result);
}

//----------------------------------------------------------------------

void SwgCuiChatRoomsWho::uninvite ()
{
	const ChatAvatarId & avatar = getSelectedAvatar ();
	if (avatar.name.empty ())
		return;

	Unicode::String result;
	if (!CuiChatRoomManager::setInvited (m_roomId, avatar.getFullName (), false, result))
		CuiMessageBox::createInfoBox (result);
}

//----------------------------------------------------------------------

void SwgCuiChatRoomsWho::kick ()
{
	const ChatAvatarId & avatar = getSelectedAvatar ();
	if (avatar.name.empty ())
		return;

	Unicode::String result;
	if (!CuiChatRoomManager::kick (m_roomId, avatar.getFullName (), result))
		CuiMessageBox::createInfoBox (result);
}

//----------------------------------------------------------------------

void SwgCuiChatRoomsWho::invite ()
{
	CuiWorkspace * const ws = getContainingWorkspace ();
	if (ws)
		SwgCuiChatRoomsInvite::getMediatorForRoom (*ws, m_roomId);
}

//----------------------------------------------------------------------

void SwgCuiChatRoomsWho::ban ()
{
	CuiWorkspace * const ws = getContainingWorkspace ();
	if (ws)
	{
		const ChatAvatarId & avatar = getSelectedAvatar ();
		
		if (avatar.name.empty ())
			SwgCuiChatRoomsBan::getMediatorForRoom (*ws, m_roomId, std::string ());
		else
			SwgCuiChatRoomsBan::getMediatorForRoom (*ws, m_roomId, Unicode::wideToNarrow (CuiChatManager::getShortName (avatar)));
	}
}

//----------------------------------------------------------------------

void SwgCuiChatRoomsWho::unban ()
{
	const ChatAvatarId & avatar = getSelectedAvatar ();
	if (avatar.name.empty ())
		return;

	Unicode::String result;
	if (!CuiChatRoomManager::setBanned (m_roomId, avatar.getFullName (), false, result))
		CuiMessageBox::createInfoBox (result);
}

//======================================================================
