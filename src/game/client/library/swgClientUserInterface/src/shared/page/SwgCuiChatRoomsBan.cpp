//======================================================================
//
// SwgCuiChatRoomsBan.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiChatRoomsBan.h"

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

SwgCuiChatRoomsBan::SwgCuiChatRoomsBan     (UIPage & page) :
CuiMediator     ("SwgCuiChatRoomsBan", page),
UIEventCallback (),
m_buttonOk      (0),
m_buttonCancel  (0),
m_textbox       (0),
m_textRoom      (0),
m_roomId        (0)
{
	getCodeDataObject (TUIButton,  m_buttonOk,     "buttonOk");
	getCodeDataObject (TUIButton,  m_buttonCancel, "buttonCancel");
	getCodeDataObject (TUITextbox, m_textbox,      "textbox");
	getCodeDataObject (TUIText,    m_textRoom,     "textRoom");

	registerMediatorObject (*m_buttonOk,     true);
	registerMediatorObject (*m_buttonCancel, true);

	m_textRoom->SetPreLocalized (true);

	setState (MS_closeable);
}

//----------------------------------------------------------------------

SwgCuiChatRoomsBan::~SwgCuiChatRoomsBan ()
{
}

//----------------------------------------------------------------------

void SwgCuiChatRoomsBan::performActivate ()
{
	m_textbox->SetFocus ();
}

//----------------------------------------------------------------------

void SwgCuiChatRoomsBan::OnButtonPressed   (UIWidget *context )
{
	if (context == m_buttonOk)
	{
		const Unicode::String & value = m_textbox->GetLocalText ();

		ChatAvatarId cid;
		CuiChatManager::constructChatAvatarId (Unicode::wideToNarrow (value), cid);

		Unicode::String result;
		if (!CuiChatRoomManager::setBanned (m_roomId, cid.getFullName (), true, result))
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

SwgCuiChatRoomsBan * SwgCuiChatRoomsBan::createInto  (UIPage & parent)
{
	UIPage * const dupe = NON_NULL (UIPage::DuplicateInto (parent, "/pda.chatroomBan"));
	return new SwgCuiChatRoomsBan (*dupe);
}

//----------------------------------------------------------------------

SwgCuiChatRoomsBan * SwgCuiChatRoomsBan::findMediatorForRoom       (CuiWorkspace & ws, uint32 roomId)
{
	SwgCuiChatRoomsBan * mediator = safe_cast<SwgCuiChatRoomsBan *>(ws.findMediatorByType (typeid (SwgCuiChatRoomsBan)));
	
	if (mediator && (mediator->m_roomId == roomId || roomId == 0))
		return mediator;

	return 0;
}

//----------------------------------------------------------------------

SwgCuiChatRoomsBan * SwgCuiChatRoomsBan::getMediatorForRoom        (CuiWorkspace & ws, uint32 roomId, const std::string & name)
{
	SwgCuiChatRoomsBan * mediator = findMediatorForRoom (ws, roomId);
	
	if (!mediator)
	{
		mediator = SwgCuiChatRoomsBan::createInto (ws.getPage ());
		NOT_NULL (mediator);
		mediator->setSettingsAutoSizeLocation (true, true);
		ws.addMediator (*mediator);
		//-- start off near the cursor
		ws.positionMediator (*mediator);
	}
	
	mediator->setRoomId (roomId, name);
	mediator->activate ();
	ws.focusMediator (*mediator, true);
	mediator->setEnabled (true);

	return mediator;
}

//----------------------------------------------------------------------

void SwgCuiChatRoomsBan::setRoomId                 (uint32 roomId, const std::string & name)
{
	m_roomId = roomId;
	m_textbox->SetText (Unicode::narrowToWide (name));

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
