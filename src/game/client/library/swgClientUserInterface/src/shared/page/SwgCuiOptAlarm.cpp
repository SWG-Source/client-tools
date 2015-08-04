//======================================================================
//
// SwgCuiOptAlarm.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiOptAlarm.h"


#include "clientAudio/Audio.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiStringIdsAlarm.h"
#include "clientUserInterface/CuiStringVariablesData.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "clientUserInterface/CuiUtils.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "UIButton.h"
#include "UICheckbox.h"
#include "UIComboBox.h"
#include "UIData.h"
#include "UIList.h"
#include "UIMessage.h"
#include "UIText.h"
#include "UITextbox.h"
#include "clientGame/AlarmManager.h"

#include <list>
#include <vector>

//======================================================================

SwgCuiOptAlarm::SwgCuiOptAlarm (UIPage & page) :
SwgCuiOptBase             ("SwgCuiOptAlarm", page),
m_textAlarmEarthTime      (NULL),
m_textBoxAlarmInHours     (NULL),
m_textBoxAlarmInMinutes   (NULL),
m_comboBoxAlarmAtHour     (NULL),
m_comboBoxAlarmAtMinute   (NULL),
m_comboBoxAlarmSound      (NULL),
m_buttonTestAlarmSound    (NULL),
m_checkBoxRecurring       (NULL),
m_textAlarmMessage        (NULL),
m_buttonAddAlarm          (NULL),
m_buttonRemoveAlarm       (NULL),
m_listAlarms              (NULL),
m_checkBoxAlarmAt         (NULL),
m_checkBoxAlarmIn         (NULL),
m_soundId                 (),
m_callback                (new MessageDispatch::Callback)
{
	getCodeDataObject(TUIText,     m_textAlarmEarthTime,    "timeEarth");
	getCodeDataObject(TUITextbox,  m_textBoxAlarmInHours,   "textHours");	
	getCodeDataObject(TUITextbox,  m_textBoxAlarmInMinutes, "textMinutes");	
	getCodeDataObject(TUIComboBox, m_comboBoxAlarmAtHour,   "comboHours");	
	getCodeDataObject(TUIComboBox, m_comboBoxAlarmAtMinute, "comboMinutes");	
	getCodeDataObject(TUIComboBox, m_comboBoxAlarmSound,    "comboSound");	
	getCodeDataObject(TUIButton,   m_buttonTestAlarmSound,  "buttonTest");	
	getCodeDataObject(TUICheckbox, m_checkBoxRecurring,     "checkRecurring");	
	getCodeDataObject(TUITextbox,  m_textAlarmMessage,      "textMessage");	
	getCodeDataObject(TUIButton,   m_buttonAddAlarm,        "buttonAdd");	
	getCodeDataObject(TUIButton,   m_buttonRemoveAlarm,     "buttonRemove");	
	getCodeDataObject(TUIList,     m_listAlarms,            "listAlarms");	
	getCodeDataObject(TUICheckbox, m_checkBoxAlarmAt,       "checkAlarmAt");	
	getCodeDataObject(TUICheckbox, m_checkBoxAlarmIn,       "checkAlarmIn");	
	
	m_textBoxAlarmInHours->SetNumericInteger(true);
	m_textBoxAlarmInHours->SetNumericInteger(true);
	
	registerMediatorObject(*m_checkBoxAlarmIn,       true);
	registerMediatorObject(*m_checkBoxAlarmAt,       true);
	registerMediatorObject(*m_listAlarms,            true);
	registerMediatorObject(*m_buttonRemoveAlarm,     true);
	registerMediatorObject(*m_buttonAddAlarm,        true);
	registerMediatorObject(*m_textAlarmMessage,      true);
	registerMediatorObject(*m_checkBoxRecurring,     true);
	registerMediatorObject(*m_buttonTestAlarmSound,  true);
	registerMediatorObject(*m_comboBoxAlarmSound,    true);
	registerMediatorObject(*m_comboBoxAlarmAtMinute, true);
	registerMediatorObject(*m_comboBoxAlarmAtHour,   true);
	registerMediatorObject(*m_textBoxAlarmInHours,   true);
	registerMediatorObject(*m_textAlarmEarthTime,    true);

	// Fill out the hours

	{
		m_comboBoxAlarmAtHour->Clear();

		for (int i = 0; i < 24; ++i)
		{
			char militaryTime[256];
			sprintf(militaryTime, "%d", i);

			Unicode::String itemString;

			if (i == 0)
			{
				itemString += CuiStringIdsAlarm::alarm_midnight.localize();
			}
			else if (i == 12)
			{
				itemString += CuiStringIdsAlarm::alarm_noon.localize();
			}
			else
			{
				char civilianTime[256];

				if (i < 12)
				{
					sprintf(civilianTime, "%d", i);
					itemString += Unicode::narrowToWide(civilianTime);
					itemString += ' ';
					itemString += CuiStringIdsAlarm::alarm_am.localize();
				}
				else
				{
					sprintf(civilianTime, "%d", i - 12);
					itemString += Unicode::narrowToWide(civilianTime);
					itemString += ' ';
					itemString += CuiStringIdsAlarm::alarm_pm.localize();
				}
			}

			m_comboBoxAlarmAtHour->AddItem(itemString, militaryTime);
			m_comboBoxAlarmAtHour->SetSelectedIndex(0);
		}
	}
	
	// Fill out the minutes

	{
		m_comboBoxAlarmAtMinute->Clear();

		for (int i = 0; i < 60; ++i)
		{
			char text[256];
			sprintf(text, "%02d", i);

			m_comboBoxAlarmAtMinute->AddItem(Unicode::narrowToWide(text), text);
			m_comboBoxAlarmAtMinute->SetSelectedIndex(0);
		}
	}

	// Fill out the alarm sounds

	{
		m_comboBoxAlarmSound->Clear();

		for (unsigned int i = 1; i <= AlarmManager::getAlarmSounds().size(); ++i)
		{
			Unicode::String alarmNumberString;
			CuiStringVariablesData data;
			data.digit_i = static_cast<int>(i);

			CuiStringVariablesManager::process(CuiStringIdsAlarm::alarm_number, data, alarmNumberString);

			m_comboBoxAlarmSound->AddItem(alarmNumberString, "");
		}

		m_comboBoxAlarmSound->SetSelectedIndex(0);
	}

	m_checkBoxAlarmAt->SetChecked(false);
	m_checkBoxAlarmIn->SetChecked(true);

	// Default the alarms 2 hours in the future.

	m_textBoxAlarmInHours->SetLocalText(Unicode::narrowToWide("2"));
	m_textBoxAlarmInMinutes->SetLocalText(Unicode::narrowToWide("0"));

	m_comboBoxAlarmAtHour->SetSelectedIndex(2);
	m_comboBoxAlarmAtMinute->SetSelectedIndex(0);

	// Do not recur by default

	m_checkBoxRecurring->SetChecked(false);
}

//----------------------------------------------------------------------

SwgCuiOptAlarm::~SwgCuiOptAlarm()
{
	m_textAlarmEarthTime = NULL;
	m_textBoxAlarmInHours = NULL;
	m_textBoxAlarmInMinutes = NULL;
	m_comboBoxAlarmAtHour = NULL;
	m_comboBoxAlarmAtMinute = NULL;
	m_comboBoxAlarmSound = NULL;
	m_buttonTestAlarmSound = NULL;
	m_checkBoxRecurring = NULL;
	m_textAlarmMessage = NULL;
	m_buttonAddAlarm = NULL;
	m_buttonRemoveAlarm = NULL;
	m_listAlarms = NULL;
	m_checkBoxAlarmAt = NULL;
	m_checkBoxAlarmIn = NULL;

	delete m_callback;
	m_callback = NULL;
}

//-----------------------------------------------------------------

void SwgCuiOptAlarm::performActivate()
{
	setIsUpdating(true);
}

//-----------------------------------------------------------------

void SwgCuiOptAlarm::performDeactivate()
{
	setIsUpdating(false);
}

//-----------------------------------------------------------------

void SwgCuiOptAlarm::queryWidgetValues()
{
	SwgCuiOptBase::queryWidgetValues();

	// Load up all the current alarms

	AlarmManager::AlarmIdVector alarms;
	AlarmManager::getAlarms(alarms);
	updateAlarms(alarms);
}

//----------------------------------------------------------------------

void SwgCuiOptAlarm::OnButtonPressed(UIWidget *context)
{
	SwgCuiOptBase::OnButtonPressed (context);

	if (context == m_buttonTestAlarmSound)
	{
		if (!AlarmManager::getAlarmSounds().empty())
		{
			int const selectedIndex = m_comboBoxAlarmSound->GetSelectedIndex();

			if ((m_comboBoxAlarmSound->GetItemCount() > 0) &&
			    (selectedIndex >= 0) &&
			    (selectedIndex < static_cast<int>(AlarmManager::getAlarmSounds().size())))
			{
				std::string const &sound = AlarmManager::getAlarmSounds()[static_cast<unsigned int>(selectedIndex)];

				Audio::stopSound(m_soundId);
				m_soundId = Audio::playSound(sound.c_str(), NULL);
			}
		}
	}
	else if (context == m_buttonAddAlarm)
	{
		addNewAlarm();
	}
	else if (context == m_buttonRemoveAlarm)
	{
		removeAlarms();
	}
}

//----------------------------------------------------------------------

void SwgCuiOptAlarm::OnCheckboxSet(UIWidget *context)
{
	SwgCuiOptBase::OnCheckboxSet (context);

	if (context == m_checkBoxAlarmAt)
	{
		m_checkBoxAlarmIn->SetChecked(false);
		m_textBoxAlarmInHours->SetEnabled(false);
		m_textBoxAlarmInMinutes->SetEnabled(false);

		m_comboBoxAlarmAtHour->SetEnabled(true);
		m_comboBoxAlarmAtMinute->SetEnabled(true);
	}
	else if (context == m_checkBoxAlarmIn)
	{
		m_checkBoxAlarmAt->SetChecked(false);
		m_comboBoxAlarmAtHour->SetEnabled(false);
		m_comboBoxAlarmAtMinute->SetEnabled(false);

		m_textBoxAlarmInHours->SetEnabled(true);
		m_textBoxAlarmInMinutes->SetEnabled(true);
	}
}

//----------------------------------------------------------------------

bool SwgCuiOptAlarm::OnMessage(UIWidget *context, const UIMessage &message)
{
	if (!SwgCuiOptBase::OnMessage (context, message))
		return false;

	bool result = true;

	switch (message.Type)
	{
		case UIMessage::KeyDown:
			{
				if (message.Keystroke == UIMessage::Enter)
				{
					if (context == m_textAlarmMessage)
					{
						addNewAlarm();

						result = false;
					}
				}
				else if (message.Keystroke == UIMessage::Delete)
				{
					if (context == m_listAlarms)
					{
						removeAlarms();

						result = false;
					}
				}
			}
			break;
	}

	return result;
}

//----------------------------------------------------------------------

void SwgCuiOptAlarm::update(float deltaTimeSecs)
{
	SwgCuiOptBase::update (deltaTimeSecs);

	Unicode::String earthTime;
	IGNORE_RETURN(CuiUtils::FormatDate(earthTime, CuiUtils::GetSystemSeconds()));
	
	m_textAlarmEarthTime->SetText(earthTime);
}

//----------------------------------------------------------------------

void SwgCuiOptAlarm::updateAlarms(AlarmManager::AlarmIdVector const &alarms)
{
	m_listAlarms->Clear();

	if (alarms.empty())
	{
		m_listAlarms->AddRow(CuiStringIdsAlarm::alarm_none.localize(), "-1");
		m_buttonRemoveAlarm->SetEnabled(false);
	}
	else
	{
		AlarmManager::AlarmIdVector::const_iterator iterAlarms = alarms.begin();
		int count = 1;

		for (; iterAlarms != alarms.end(); ++iterAlarms)
		{
			int const alarmId = (*iterAlarms);
			Unicode::String alarmExpireTimeString;

			AlarmManager::getAlarmExpireTimeString(alarmId, alarmExpireTimeString);

			char text[256];
			sprintf(text, "%2d ", count);

			Unicode::String localizedString;
			localizedString += Unicode::narrowToWide(text);
			localizedString += alarmExpireTimeString;

			sprintf(text, "%d", alarmId);
			m_listAlarms->AddRow(localizedString, text);
			++count;
		}

		m_buttonRemoveAlarm->SetEnabled(true);
	}
}

//----------------------------------------------------------------------

void SwgCuiOptAlarm::addNewAlarm()
{
	// Add a new alarm

	Unicode::String const &message = m_textAlarmMessage->GetLocalText();
	bool const recurring = m_checkBoxRecurring->IsChecked();
	int const soundId    = static_cast<int>(m_comboBoxAlarmSound->GetSelectedIndex());

	if (m_checkBoxAlarmIn->IsChecked())
	{
		const std::string & hoursString   = Unicode::wideToNarrow(m_textBoxAlarmInHours->GetLocalText());
		const std::string & minutesString = Unicode::wideToNarrow(m_textBoxAlarmInMinutes->GetLocalText());

		int hours = atoi(hoursString.c_str());
		int minutes = atoi(minutesString.c_str());

		if (AlarmManager::addAlarmIn(hours, minutes, message, soundId, recurring) != AlarmManager::invalidAlarmId)
		{
			AlarmManager::AlarmIdVector alarms;
			AlarmManager::getAlarms(alarms);
			updateAlarms(alarms);

			// Make sure the item that was inserted is selected and visible

			int const newRow = AlarmManager::getAlarmCount() - 1;

			if (newRow >= 0)
			{
				m_listAlarms->SelectRow(newRow);
				m_listAlarms->ScrollToRow(newRow);
			}

			// Clear the alarm message

			//m_textAlarmMessage->SetLocalText(Unicode::emptyString);
		}
		else
		{
			CuiMessageBox::createInfoBox(CuiStringIdsAlarm::alarm_in_invalid.localize(), NULL, false);
		}
	}
	else if (m_checkBoxAlarmAt->IsChecked())
	{
		int const hour = static_cast<int>(m_comboBoxAlarmAtHour->GetSelectedIndex());
		int const minute = static_cast<int>(m_comboBoxAlarmAtMinute->GetSelectedIndex());

		if (AlarmManager::addAlarmAt(hour, minute, message, soundId, recurring) != AlarmManager::invalidAlarmId)
		{
			AlarmManager::AlarmIdVector alarms;
			AlarmManager::getAlarms(alarms);
			updateAlarms(alarms);

			// Make sure the item that was inserted is selected and visible

			int const newRow = AlarmManager::getAlarmCount() - 1;

			if (newRow >= 0)
			{
				m_listAlarms->SelectRow(newRow);
				m_listAlarms->ScrollToRow(newRow);
			}

			// Clear the alarm message

			//m_textAlarmMessage->SetLocalText(Unicode::emptyString);
		}
		else
		{
			CuiMessageBox::createInfoBox(CuiStringIdsAlarm::alarm_at_invalid.localize(), NULL, false);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiOptAlarm::removeAlarms()
{
	// Build a list of the selected items

	typedef std::list<int> RemoveAlarmList;
	RemoveAlarmList removeAlarmList;

	for (int i = 0; i < m_listAlarms->GetRowCount(); ++i)
	{
		if (m_listAlarms->IsRowSelected(i))
		{
			std::string alarmIdString;

			if (m_listAlarms->GetText(i, alarmIdString))
			{
				int const alarmId = atoi(alarmIdString.c_str());

				removeAlarmList.push_back(alarmId);
			}
		}
	}

	// Remove all the selected alarms

	RemoveAlarmList::iterator iterRemoveAlarmList = removeAlarmList.begin();

	for (; iterRemoveAlarmList != removeAlarmList.end(); ++iterRemoveAlarmList)
	{
		int const alarmId = (*iterRemoveAlarmList);

		if (!AlarmManager::removeAlarm(alarmId))
		{
			DEBUG_WARNING(true, ("Unable to remove alarm %d", alarmId));
		}
	}

	AlarmManager::AlarmIdVector alarms;
	AlarmManager::getAlarms(alarms);
	updateAlarms(alarms);
}

//======================================================================
