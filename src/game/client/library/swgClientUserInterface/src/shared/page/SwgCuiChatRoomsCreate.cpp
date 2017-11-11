//======================================================================
//
// SwgCuiChatRoomsCreate.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiChatRoomsCreate.h"

#include "UIButton.h"
#include "UICheckbox.h"
#include "UIData.h"
#include "UIPage.h"
#include "UIText.h"
#include "UITextbox.h"
#include "clientUserInterface/CuiChatRoomManager.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiStringIdsChatRoom.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "sharedGame/TextManager.h"

//======================================================================

SwgCuiChatRoomsCreate::SwgCuiChatRoomsCreate     (UIPage & page) :
CuiMediator      ("SwgCuiChatRoomsCreate", page),
UIEventCallback  (),
m_buttonCancel   (0),
m_buttonOk       (0),
m_textPath       (0),
m_textboxName    (0),
m_textboxTitle   (0),
m_checkPrivate   (0),
m_checkModerated (0)
{
	getCodeDataObject (TUIButton,   m_buttonCancel,    "buttonCancel");
	getCodeDataObject (TUIButton,   m_buttonOk,        "buttonOk");

	getCodeDataObject (TUIText,     m_textPath,        "textPath");

	getCodeDataObject (TUITextbox,  m_textboxName,     "textboxName");
	getCodeDataObject (TUITextbox,  m_textboxTitle,    "textboxTitle");

	getCodeDataObject (TUICheckbox, m_checkPrivate,    "checkPrivate");
	getCodeDataObject (TUICheckbox, m_checkModerated,  "checkModerated");

	m_textPath->SetPreLocalized (true);

	setState (MS_closeable);
	setState (MS_closeDeactivates);
}

//----------------------------------------------------------------------

SwgCuiChatRoomsCreate::~SwgCuiChatRoomsCreate ()
{
	m_buttonCancel   = 0;
	m_buttonOk       = 0;
	m_textPath       = 0;
	m_textboxName    = 0;
	m_textboxTitle   = 0;
	m_checkPrivate   = 0;
	m_checkModerated = 0;
}

//----------------------------------------------------------------------

void  SwgCuiChatRoomsCreate::performActivate           ()
{
	CuiManager::requestPointer (true);

	m_buttonCancel->AddCallback  (this);
	m_buttonOk->AddCallback      (this);

	setRootPath (std::string ());
}

//----------------------------------------------------------------------

void  SwgCuiChatRoomsCreate::performDeactivate         ()
{
	m_buttonCancel->RemoveCallback  (this);
	m_buttonOk->RemoveCallback      (this);

	setRootPath (std::string ());

	CuiManager::requestPointer (false);
}

//----------------------------------------------------------------------

void  SwgCuiChatRoomsCreate::OnButtonPressed           (UIWidget *context )
{
	if (context == m_buttonCancel)
	{
		deactivate ();
	}
	else if (context == m_buttonOk)
	{
		ok ();
	}
}

//----------------------------------------------------------------------

SwgCuiChatRoomsCreate * SwgCuiChatRoomsCreate::createInto          (UIPage & parent)
{
	UIPage * const dupe = NON_NULL (UIPage::DuplicateInto (parent, "/pda.chatroomCreate"));
	dupe->Center ();
	return new SwgCuiChatRoomsCreate (*dupe);

}

//----------------------------------------------------------------------

void SwgCuiChatRoomsCreate::ok ()
{
	const std::string & roomName = Unicode::getTrim (Unicode::wideToNarrow (m_textboxName->GetLocalText ()));

	if (roomName.empty ())
	{
		CuiMessageBox::createInfoBox (CuiStringIdsChatRoom::create_err_no_name.localize ());
		return;
	}

	if (roomName [0] == '.' || roomName.find ("..") != std::string::npos)
	{
		CuiMessageBox::createInfoBox (CuiStringIdsChatRoom::create_err_invalid_name.localize ());
		return;
	}

	for (std::string::const_iterator it = roomName.begin (); it != roomName.end (); ++it)
	{
		const char c = *it;

		if (!isalnum (c) && c != '.')
		{
			CuiMessageBox::createInfoBox (CuiStringIdsChatRoom::create_err_invalid_name.localize ());
			return;
		}
	}

	const std::string & rootPath = Unicode::wideToNarrow (m_textPath->GetLocalText ());
	const std::string & roomPath  = rootPath + "." + roomName;

	// Check the room path for profanity

	if (!TextManager::isAppropriateText(Unicode::narrowToWide(roomPath)))
	{
		CuiSystemMessageManager::sendFakeSystemMessage(CuiStringIdsChatRoom::create_err_profane_name.localize());
		return;
	}
	
	const Unicode::String & roomTitle = Unicode::getTrim (m_textboxTitle->GetLocalText ());

	/*
	if (roomTitle.empty ())
	{
		CuiMessageBox::createInfoBox (CuiStringIdsChatRoom::create_err_no_title.localize ());
		return;
	}
*/

	// Check the room title for profanity

	if (!TextManager::isAppropriateText(roomTitle))
	{
		CuiSystemMessageManager::sendFakeSystemMessage(CuiStringIdsChatRoom::create_err_profane_title.localize());
		return;
	}

	CuiChatRoomManager::createRoom (roomPath, m_checkModerated->IsChecked (), !m_checkPrivate->IsChecked (), roomTitle);

	deactivate ();
}

//----------------------------------------------------------------------

void SwgCuiChatRoomsCreate::setRootPath (const std::string & rootPath)
{
	m_textPath->SetLocalText     (Unicode::narrowToWide (rootPath));

	m_textboxName->SetLocalText  (Unicode::String ());
	m_textboxTitle->SetLocalText (Unicode::String ());

	m_checkPrivate->SetChecked   (true);
	m_checkModerated->SetChecked (false);

	m_textboxName->SetFocus ();
}

//======================================================================
