// ======================================================================
//
// SwgCuiDeleteAvatarConfirmation.cpp
// Copyright Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiDeleteAvatarConfirmation.h"

#include "StringId.h"
#include "UIButton.h"
#include "UIMessage.h"
#include "UIText.h"
#include "UITextbox.h"
#include "clientUserInterface/CuiLoginManager.h"
#include "clientUserInterface/CuiLoginManagerClusterInfo.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "sharedMessageDispatch/Transceiver.h"

//-----------------------------------------------------------------

namespace
{
	namespace Transceivers
	{
		MessageDispatch::Transceiver<const SwgCuiDeleteAvatarConfirmation::Message::DeleteAvatarConfirmation::Info &, SwgCuiDeleteAvatarConfirmation::Message::DeleteAvatarConfirmation>
			s_deleteAvatarConfirmation;
	}
	
	StringId s_avatarDeleteRequest("ui", "avatar_delete_request");
	StringId s_avatarDeleteName("ui", "avatar_delete_name");
	StringId s_avatarDeleteGalaxy("ui", "avatar_delete_galaxy");
	StringId s_avatarDeleteConfirm("ui", "avatar_delete_confirm");	
	StringId s_avatarDeleteNotMatch("ui", "avatar_delete_notmatch");
	StringId s_avatarDeleteYouEntered("ui", "avatar_delete_you_entered");
	StringId s_avatarDeleteRequired("ui", "avatar_delete_required");		
}

// ======================================================================
//
// SwgCuiDeleteAvatarConfirmation
//
// ======================================================================

//-----------------------------------------------------------------

SwgCuiDeleteAvatarConfirmation::SwgCuiDeleteAvatarConfirmation(UIPage &page)
 : CuiMediator("SwgCuiDeleteAvatarConfirmation", page)
 , UIEventCallback()
 , m_callBack(new MessageDispatch::Callback)
 , m_okButton(NULL)
 , m_cancelButton(NULL)
{
	setState(MS_closeable);
	setState(MS_closeDeactivates);

	getCodeDataObject(TUIButton, m_okButton, "buttonOk");
	registerMediatorObject(*m_okButton, true);

	getCodeDataObject(TUIButton, m_cancelButton, "buttonCancel");
	registerMediatorObject(*m_cancelButton, true);

	getCodeDataObject(TUITextbox, m_avatarTextBox, "textBoxAvatar");
	registerMediatorObject(*m_avatarTextBox, true);
	m_avatarTextBox->SetLocalText(Unicode::emptyString);

	getCodeDataObject(TUIText, m_instructionsText, "textInstructions");
	registerMediatorObject(*m_instructionsText, true);
	m_instructionsText->SetLocalText(Unicode::emptyString);
}

//-----------------------------------------------------------------

SwgCuiDeleteAvatarConfirmation::~SwgCuiDeleteAvatarConfirmation()
{
	delete m_callBack;
	m_callBack = NULL;

	m_okButton = NULL;
	m_cancelButton = NULL;
	m_avatarTextBox = NULL;
	m_instructionsText = NULL;
}

//-----------------------------------------------------------------

void SwgCuiDeleteAvatarConfirmation::performActivate()
{
}

//-----------------------------------------------------------------

void SwgCuiDeleteAvatarConfirmation::performDeactivate()
{
}

//-----------------------------------------------------------------

void SwgCuiDeleteAvatarConfirmation::OnButtonPressed(UIWidget *context)
{
	if (context == m_okButton)
	{
		validateText();
	}
	else if (context == m_cancelButton)
	{
		deactivate();
	}
}

//-----------------------------------------------------------------

void SwgCuiDeleteAvatarConfirmation::setAvatarInfo(CuiLoginManagerAvatarInfo const &avatarInfo)
{
	m_avatarInfo = avatarInfo;

	const CuiLoginManager::ClusterInfo * const clusterInfo = CuiLoginManager::findClusterInfo(avatarInfo.clusterId);

	if (clusterInfo != NULL)
	{
		Unicode::String text;

		text += s_avatarDeleteRequest.localize();
		text += s_avatarDeleteName.localize() + avatarInfo.name + Unicode::narrowToWide("\n");
		text += s_avatarDeleteGalaxy.localize() + Unicode::narrowToWide(clusterInfo->name) + Unicode::narrowToWide("\n\n");
		text += s_avatarDeleteConfirm.localize();
		//This is intentionally left english because it must match what the user types
		text += Unicode::narrowToWide("delete ") + avatarInfo.name;

		m_instructionsText->SetLocalText(text);
	}
	else
	{
		deactivate();
	}
}

//-----------------------------------------------------------------

void SwgCuiDeleteAvatarConfirmation::validateText()
{
	if (!m_avatarTextBox->GetLocalText().empty())
	{
		Unicode::String verfiyText;

		verfiyText += Unicode::narrowToWide("delete ");
		verfiyText += m_avatarInfo.name;

		if (m_avatarTextBox->GetLocalText() == verfiyText)
		{
			// Exact match, request character deletion

			Transceivers::s_deleteAvatarConfirmation.emitMessage(m_avatarInfo);
			deactivate();

			// Clear the text

			m_avatarTextBox->SetLocalText(Unicode::emptyString);
		}
		else
		{
			// Match error, display an error message

			Unicode::String text;

			text += s_avatarDeleteNotMatch.localize();
			text += s_avatarDeleteYouEntered.localize();
			text += m_avatarTextBox->GetLocalText();
			text += Unicode::narrowToWide("\n");
			text += s_avatarDeleteRequired.localize();
			text += verfiyText;

			CuiMessageBox::createInfoBox(text, NULL, false);
		}
	}
}

// ======================================================================
