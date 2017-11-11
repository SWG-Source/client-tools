//======================================================================
//
// SwgCuiChatRoomsInvite.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiChatRoomsInvite.h"

#include "UIData.h"
#include "UIPage.h"
#include "UIButton.h"
#include "UIText.h"
#include "UITextbox.h"
#include "clientUserInterface/CuiWorkspace.h"
#include "clientUserInterface/CuiChatRoomManager.h"
#include "clientUserInterface/CuiChatManager.h"
#include "clientUserInterface/CuiChatRoomDataNode.h"
#include "clientUserInterface/CuiChatAvatarId.h"
#include "clientUserInterface/CuiMessageBox.h"

//======================================================================

SwgCuiChatRoomsInvite::SwgCuiChatRoomsInvite     (UIPage & page) :
CuiMediator         ("SwgCuiChatRoomsInvite", page),
UIEventCallback     (),
m_buttonInvite      (0),
m_buttonInviteGroup (0),
m_buttonCancel      (0),
m_textbox           (0),
m_textRoom          (0),
m_roomId            (0)
{
	getCodeDataObject (TUIButton,  m_buttonInvite,      "buttonInvite");
	getCodeDataObject (TUIButton,  m_buttonInviteGroup, "buttonInviteGroup");
	getCodeDataObject (TUIButton,  m_buttonCancel,      "buttonCancel");
	getCodeDataObject (TUITextbox, m_textbox,           "textbox");
	getCodeDataObject (TUIText,    m_textRoom,          "textRoom");

	registerMediatorObject (*m_buttonInvite,      true);
	registerMediatorObject (*m_buttonInviteGroup, true);
	registerMediatorObject (*m_buttonCancel,      true);

	m_textRoom->SetPreLocalized (true);

	setState (MS_closeable);
//	setState (MS_closeDeactivates);
}

//----------------------------------------------------------------------

SwgCuiChatRoomsInvite::~SwgCuiChatRoomsInvite ()
{
}

//----------------------------------------------------------------------

void SwgCuiChatRoomsInvite::performActivate ()
{
	m_textbox->SetFocus ();
}

//----------------------------------------------------------------------

void SwgCuiChatRoomsInvite::OnButtonPressed   (UIWidget *context )
{
	if (context == m_buttonInvite)
	{
		const Unicode::String & value = m_textbox->GetLocalText ();

		ChatAvatarId cid;
		CuiChatManager::constructChatAvatarId (Unicode::wideToNarrow (value), cid);

		Unicode::String result;
		if (!CuiChatRoomManager::setInvited (m_roomId, cid.getFullName (), true, result))
		{
			CuiMessageBox::createInfoBox (result);
			return;
		}

		deactivate ();
	}

	else if (context == m_buttonInviteGroup)
	{
		const Unicode::String & value = m_textbox->GetLocalText ();

		ChatAvatarId cid;
		CuiChatManager::constructChatAvatarId (Unicode::wideToNarrow (value), cid);

		Unicode::String result;
		if (!CuiChatRoomManager::inviteGroup (m_roomId, cid.getFullName (), result))
		{
			CuiMessageBox::createInfoBox (result);
			return;
		}

		deactivate ();
	}

	else if (context == m_buttonCancel)
		deactivate ();
}

//----------------------------------------------------------------------

SwgCuiChatRoomsInvite * SwgCuiChatRoomsInvite::createInto  (UIPage & parent)
{
	UIPage * const dupe = NON_NULL (UIPage::DuplicateInto (parent, "/pda.chatroomInvite"));
	return new SwgCuiChatRoomsInvite (*dupe);
}

//----------------------------------------------------------------------

SwgCuiChatRoomsInvite * SwgCuiChatRoomsInvite::findMediatorForRoom       (CuiWorkspace & ws, uint32 roomId)
{
	SwgCuiChatRoomsInvite * mediator = safe_cast<SwgCuiChatRoomsInvite *>(ws.findMediatorByType (typeid (SwgCuiChatRoomsInvite)));
	
	if (mediator && (mediator->m_roomId == roomId || roomId == 0))
		return mediator;

	return 0;
}

//----------------------------------------------------------------------

SwgCuiChatRoomsInvite * SwgCuiChatRoomsInvite::getMediatorForRoom        (CuiWorkspace & ws, uint32 roomId)
{
	SwgCuiChatRoomsInvite * mediator = findMediatorForRoom (ws, 0);
	
	if (!mediator)
	{
		mediator = SwgCuiChatRoomsInvite::createInto (ws.getPage ());
		NOT_NULL (mediator);
		mediator->setSettingsAutoSizeLocation (true, true);
		ws.addMediator (*mediator);
		//-- start off near the cursor
		ws.positionMediator (*mediator);
	}
	
	mediator->setRoomId (roomId);
	mediator->activate ();
	ws.focusMediator (*mediator, true);
	mediator->setEnabled (true);

	return mediator;
}

//----------------------------------------------------------------------

void SwgCuiChatRoomsInvite::setRoomId                 (uint32 roomId)
{
	m_roomId = roomId;
	m_textbox->SetText (Unicode::emptyString);

	const CuiChatRoomDataNode * const node = CuiChatRoomManager::findRoomNode (m_roomId);
	if (node)
	{
		std::string shortPath;
		CuiChatRoomManager::getRoomShortPath (*node, shortPath);
		m_textRoom->SetLocalText (Unicode::narrowToWide (shortPath));
	}
	else
		m_textRoom->Clear ();

}
	
//======================================================================
