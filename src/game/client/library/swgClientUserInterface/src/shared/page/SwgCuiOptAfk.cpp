//======================================================================
//
// SwgCuiOptAfk.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiOptAfk.h"

#include "clientGame/AwayFromKeyBoardManager.h"
#include "UICheckbox.h"
#include "UIComboBox.h"
#include "UIData.h"
#include "UIPage.h"
#include "UIText.h"

//======================================================================

SwgCuiOptAfk::SwgCuiOptAfk (UIPage & page)
 : SwgCuiOptBase ("SwgCuiOptAfk", page)
 , m_textAwayFromKeyBoardAutomaticResponse(NULL)
 , m_checkBoxAutoAwayFromKeyboardEnabled(NULL)
 , m_comboAwayFromKeyBoard(NULL)
{
	getCodeDataObject(TUICheckbox,  m_checkBoxAutoAwayFromKeyboardEnabled, "checkAutoAfk");
	registerMediatorObject(*m_checkBoxAutoAwayFromKeyboardEnabled, true);

	getCodeDataObject(TUIText,  m_textAwayFromKeyBoardAutomaticResponse, "textAfkAutomaticResponse");
	registerMediatorObject(*m_textAwayFromKeyBoardAutomaticResponse, true);

	getCodeDataObject(TUIComboBox,  m_comboAwayFromKeyBoard, "comboAfk");
	registerMediatorObject(*m_comboAwayFromKeyBoard, true);
	m_comboAwayFromKeyBoard->SetEnabled(true);
	m_comboAwayFromKeyBoard->Clear();

	for (int i = AwayFromKeyBoardManager::getAutomaticAwayFromKeyBoardMinutesMin(); i <= AwayFromKeyBoardManager::getAutomaticAwayFromKeyBoardMinutesMax(); ++i)
	{
		char text[256];
		snprintf(text, sizeof(text), "%d", i);
		m_comboAwayFromKeyBoard->AddItem(Unicode::narrowToWide(text), text);
	}
}

//----------------------------------------------------------------------

SwgCuiOptAfk::~SwgCuiOptAfk ()
{
	m_textAwayFromKeyBoardAutomaticResponse = NULL;
	m_checkBoxAutoAwayFromKeyboardEnabled = NULL;
	m_comboAwayFromKeyBoard = NULL;
}

//----------------------------------------------------------------------

void SwgCuiOptAfk::performActivate ()
{
	SwgCuiOptBase::performActivate ();
}

//----------------------------------------------------------------------

void SwgCuiOptAfk::queryWidgetValues()
{
	SwgCuiOptBase::queryWidgetValues();

	m_textAwayFromKeyBoardAutomaticResponse->SetLocalText(AwayFromKeyBoardManager::getAutomaticResponseMessage());
	m_checkBoxAutoAwayFromKeyboardEnabled->SetChecked(AwayFromKeyBoardManager::isAutoAwayFromKeyBoardEnabled());

	for (int i = 0; i < m_comboAwayFromKeyBoard->GetItemCount(); ++i)
	{
		std::string name;

		if (m_comboAwayFromKeyBoard->GetIndexName(i, name))
		{
			if (atoi(name.c_str()) == AwayFromKeyBoardManager::getAutomaticAwayFromKeyBoardMinutes())
			{
				m_comboAwayFromKeyBoard->SetSelectedIndex(i);
				break;
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiOptAfk::performDeactivate ()
{
	SwgCuiOptBase::performDeactivate ();

	AwayFromKeyBoardManager::setAutoAwayFromKeyBoardEnabled(m_checkBoxAutoAwayFromKeyboardEnabled->IsChecked());
	Unicode::String const & defaultMessage = AwayFromKeyBoardManager::getAutomaticResponseMessageDefault();
	Unicode::String const & currentMessage = m_textAwayFromKeyBoardAutomaticResponse->GetLocalText();
	if (defaultMessage != currentMessage)	
		AwayFromKeyBoardManager::setAutomaticResponseMessage(Unicode::getTrim(currentMessage));
	else
		AwayFromKeyBoardManager::setAutomaticResponseMessage(Unicode::emptyString);
	
	std::string selectedIndexText;

	if ((m_comboAwayFromKeyBoard->GetSelectedIndex() >= 0) &&
	     m_comboAwayFromKeyBoard->GetIndexName(m_comboAwayFromKeyBoard->GetSelectedIndex(), selectedIndexText))
	{
		AwayFromKeyBoardManager::setAutomaticAwayFromKeyBoardMinutes(atoi(selectedIndexText.c_str()));
	}
}

//----------------------------------------------------------------------

void SwgCuiOptAfk::resetDefaults(bool confirmed)
{
	SwgCuiOptBase::resetDefaults(confirmed);

	if (confirmed)
	{
		AwayFromKeyBoardManager::setAutoAwayFromKeyBoardEnabled(AwayFromKeyBoardManager::isAutoAwayFromKeyBoardEnabledDefault());
		AwayFromKeyBoardManager::setAutomaticAwayFromKeyBoardMinutes(AwayFromKeyBoardManager::getAutomaticAwayFromKeyBoardMinutesDefault());
		AwayFromKeyBoardManager::setAutomaticResponseMessage(Unicode::emptyString);

		queryWidgetValues();
	}
}

//----------------------------------------------------------------------
void SwgCuiOptAfk::OnCheckboxSet(UIWidget *context)
{
	if (context == m_checkBoxAutoAwayFromKeyboardEnabled)
	{
		m_comboAwayFromKeyBoard->SetEnabled(true);
	}
}

//----------------------------------------------------------------------
void SwgCuiOptAfk::OnCheckboxUnset(UIWidget *context)
{
	if (context == m_checkBoxAutoAwayFromKeyboardEnabled)
	{
		m_comboAwayFromKeyBoard->SetEnabled(false);
	}
}

//======================================================================
