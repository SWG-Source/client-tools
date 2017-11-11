// ======================================================================
//
// CuiDeleteSkillConfirmation.cpp
// Copyright Sony Online Entertainment
//
// ======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiDeleteSkillConfirmation.h"

#include "UIButton.h"
#include "UIMessage.h"
#include "UIText.h"
#include "UITextbox.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiSkillManager.h"
#include "sharedMessageDispatch/Transceiver.h"

#include "StringId.h"

//-----------------------------------------------------------------

namespace
{
	namespace Transceivers
	{
		MessageDispatch::Transceiver<const CuiDeleteSkillConfirmation::Message::DeleteSkillConfirmation::SkillName &, CuiDeleteSkillConfirmation::Message::DeleteSkillConfirmation>
			s_deleteSkillConfirmation;
	}
	
	StringId s_skillDeleteIncorrectText("ui", "skill_delete_incorrect_text");
	StringId s_skillDeleteYouEntered("ui", "skill_delete_you_entered");
	StringId s_skillDeleteRequiredYes("ui", "skill_delete_required_yes");
	StringId s_skillDeleteYouRequested("ui", "skill_delete_you_requested");	
	StringId s_skillDeleteSkill("ui", "skill_delete_skill");	
	StringId s_skillDeleteConfirm("ui", "skill_delete_confirm");	
	
}

// ======================================================================
//
// CuiDeleteSkillConfirmation
//
// ======================================================================

//-----------------------------------------------------------------

CuiDeleteSkillConfirmation::CuiDeleteSkillConfirmation(UIPage &page)
 : CuiMediator("CuiDeleteSkillConfirmation", page)
 , UIEventCallback()
 , m_okButton(NULL)
 , m_cancelButton(NULL)
 , m_skillTextBox(NULL)
 , m_instructionsText(NULL)
 , m_selectedSkill()
{
	setState(MS_closeable);
	setState(MS_closeDeactivates);

	getCodeDataObject(TUIButton, m_okButton, "buttonOk");
	registerMediatorObject(*m_okButton, true);

	getCodeDataObject(TUIButton, m_cancelButton, "buttonCancel");
	registerMediatorObject(*m_cancelButton, true);

	getCodeDataObject(TUITextbox, m_skillTextBox, "textBox");
	m_skillTextBox->SetLocalText(Unicode::emptyString);

	getCodeDataObject(TUIText, m_instructionsText, "textInstructions");
	m_instructionsText->Clear();
}

//-----------------------------------------------------------------

CuiDeleteSkillConfirmation::~CuiDeleteSkillConfirmation()
{
	m_okButton = NULL;
	m_cancelButton = NULL;
	m_skillTextBox = NULL;
	m_instructionsText = NULL;
}

//-----------------------------------------------------------------

void CuiDeleteSkillConfirmation::performActivate()
{
	setPointerInputActive(true);
	setKeyboardInputActive(true);
	setInputToggleActive(false);
}

//-----------------------------------------------------------------

void CuiDeleteSkillConfirmation::performDeactivate()
{
	setPointerInputActive(false);
	setKeyboardInputActive(false);
	setInputToggleActive(true);	
}

//-----------------------------------------------------------------

void CuiDeleteSkillConfirmation::OnButtonPressed(UIWidget *context)
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

void CuiDeleteSkillConfirmation::setSelectedSkill(std::string const &selectedSkill)
{
	DEBUG_WARNING(selectedSkill.empty(), ("Empty skill selected"));

	m_selectedSkill = selectedSkill;

	Unicode::String skillName;
	CuiSkillManager::localizeSkillName(m_selectedSkill, skillName);

	Unicode::String text;

	text += s_skillDeleteYouRequested.localize();
	text += s_skillDeleteSkill.localize() + skillName + Unicode::narrowToWide("\n\n");
	text += s_skillDeleteConfirm.localize();

	m_instructionsText->SetLocalText(text);
	m_skillTextBox->SetLocalText(Unicode::emptyString);
}

//-----------------------------------------------------------------

void CuiDeleteSkillConfirmation::validateText()
{
	if (!m_skillTextBox->GetLocalText().empty())
	{
		if (   Unicode::caseInsensitiveCompare(Unicode::narrowToWide("y"), m_skillTextBox->GetLocalText())
		    || Unicode::caseInsensitiveCompare(Unicode::narrowToWide("ye"), m_skillTextBox->GetLocalText())
		    || Unicode::caseInsensitiveCompare(Unicode::narrowToWide("yes"), m_skillTextBox->GetLocalText()))
		{
			// Exact match, request character deletion

			Transceivers::s_deleteSkillConfirmation.emitMessage(m_selectedSkill);
			deactivate();
		}
		else
		{
			// Match error, display an error message

			Unicode::String text;

			text += s_skillDeleteIncorrectText.localize();
			text += s_skillDeleteYouEntered.localize();
			text += m_skillTextBox->GetLocalText();
			text += Unicode::narrowToWide("\n");
			text += s_skillDeleteRequiredYes.localize();

			CuiMessageBox::createInfoBox(text, NULL, false);
		}
	}
}

// ======================================================================
