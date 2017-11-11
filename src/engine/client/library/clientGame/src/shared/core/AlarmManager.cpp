// ============================================================================
//
// AlarmManager.cpp
// Copyright Sony Online Entertainment, Inc.
//
// ============================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/AlarmManager.h"

#include "clientGame/AlarmManager_Alarm.h"
#include "clientAudio/Audio.h"
#include "clientGame/Game.h"
#include "clientUserInterface/CuiStringIdsAlarm.h"
#include "clientUserInterface/CuiStringVariablesData.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "clientUserInterface/CuiUtils.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ExitChain.h"
#include <map>
#include <vector>

//-----------------------------------------------------------------------------
namespace AlarmManagerNamespace
{
	int  getNextAlarmId();
	bool isAtMaxAlarms();

	typedef std::map<int, AlarmManager::Alarm> Alarms;
	typedef std::vector<std::string>           AlarmSounds;

	Alarms *     s_alarms;
	int          s_nextAlarmId = 0;
	bool         s_installed;
	unsigned int s_maxAlarms = 32;
	time_t const s_secondsPerMinute = 60;
	time_t const s_secondsPerHour = 60 * s_secondsPerMinute;
	time_t const s_secondsPerDay = 24 * s_secondsPerHour;
	AlarmSounds  s_alarmSounds;
}

using namespace AlarmManagerNamespace;

// ============================================================================
//
// AlarmManagerNamespace
//
// ============================================================================

//-----------------------------------------------------------------------------
int AlarmManagerNamespace::getNextAlarmId()
{
	int result = s_nextAlarmId;

	// Find an unused alarm id

	while (s_alarms->find(result) != s_alarms->end())
	{
		++s_nextAlarmId;

		result = s_nextAlarmId;
	}

	return result;
}

// ============================================================================
//
// AlarmManager
//
// ============================================================================

int const AlarmManager::invalidAlarmId = -1;

//-----------------------------------------------------------------------------
void AlarmManager::install()
{
	InstallTimer const installTimer("AlarmManager::install");

	DEBUG_FATAL(s_installed, ("AlarmManager::install() - Already installed."));

	s_installed = true;

	s_alarmSounds.clear();
	s_alarmSounds.push_back("sound/ui_alarm_clock1.snd");
	s_alarmSounds.push_back("sound/ui_alarm_clock2.snd");
	s_alarmSounds.push_back("sound/ui_alarm_clock3.snd");
	s_alarmSounds.push_back("sound/ui_alarm_clock4.snd");
	s_alarmSounds.push_back("sound/ui_alarm_clock5.snd");

	delete s_alarms;
	s_alarms = new Alarms;

	ExitChain::add(AlarmManager::remove, "AlarmManager::remove", 0, false);
}

//-----------------------------------------------------------------------------
void AlarmManager::remove()
{
	s_alarms->clear();
	delete s_alarms;
	s_alarms = NULL;

	s_alarmSounds.clear();

	s_installed = false;
}

//-----------------------------------------------------------------------------
int AlarmManager::addAlarmAt(int const hour, int const minute, Unicode::String const &message, int const soundId, bool const recurring)
{
	int result = AlarmManager::invalidAlarmId;

	if (isFull())
	{
		CuiStringVariablesData data;
		data.digit_i = s_maxAlarms;

		Unicode::String alarmMaximumReachedString;
		CuiStringVariablesManager::process(CuiStringIdsAlarm::alarm_maximum_reached, data, alarmMaximumReachedString);

		CuiSystemMessageManager::sendFakeSystemMessage(alarmMaximumReachedString);
	}
	else if ((hour < 0) || (hour > 23) ||
	         (minute < 0) || (minute > 59))
	{
		CuiSystemMessageManager::sendFakeSystemMessage(CuiStringIdsAlarm::alarm_at_invalid.localize());
	}
	else if ((soundId >= 0) &&
	         (soundId <= getSoundCount()))
	{
		AlarmManager::Alarm alarm;

		// Get the time when the current day started

		time_t const currentSystemTime = CuiUtils::GetSystemSeconds();
		time_t const timeZoneDifferenceFromUTC = CuiUtils::GetTimeZoneDifferenceFromUTC();
		time_t const daysFromTheBeginning = ((currentSystemTime - timeZoneDifferenceFromUTC) / s_secondsPerDay);
		time_t const systemSecondsAtStartOfDay = timeZoneDifferenceFromUTC + daysFromTheBeginning * s_secondsPerDay;

		if ((systemSecondsAtStartOfDay + (static_cast<time_t >(hour - 1) * s_secondsPerHour) + (static_cast<time_t>(minute) * s_secondsPerMinute)) < currentSystemTime)
		{
			// The expire time is sometime tomorrow

			time_t const expireTime = systemSecondsAtStartOfDay + s_secondsPerDay + (static_cast<size_t>(hour - 1) * s_secondsPerHour) + (static_cast<size_t>(minute) * s_secondsPerMinute);

			alarm.setExpireTime(expireTime);
			alarm.setRecurTime(s_secondsPerDay);
		}
		else
		{
			// The expire time is sometime today

			time_t const expireTime = systemSecondsAtStartOfDay + (static_cast<size_t>(hour - 1) * s_secondsPerHour) + (static_cast<size_t>(minute) * s_secondsPerMinute);

			alarm.setExpireTime(expireTime);
			alarm.setRecurTime(s_secondsPerDay);
		}

		alarm.setId(getNextAlarmId());
		alarm.setMessage(message);
		alarm.setSoundId(soundId);
		alarm.setRecurring(recurring);

		IGNORE_RETURN(s_alarms->insert(std::make_pair(alarm.getId(), alarm)));
		result = alarm.getId();
	}
	else
	{
		DEBUG_FATAL(true, ("Invalid alarm sound id: %d", soundId));
	}

	return result;
}

//-----------------------------------------------------------------------------
int AlarmManager::addAlarmIn(int const hours, int const minutes, Unicode::String const &message, int const soundId, bool const recurring)
{
	int result = AlarmManager::invalidAlarmId;

	if (isFull())
	{
		CuiStringVariablesData data;
		data.digit_i = s_maxAlarms;

		Unicode::String alarmMaximumReachedString;
		CuiStringVariablesManager::process(CuiStringIdsAlarm::alarm_maximum_reached, data, alarmMaximumReachedString);

		CuiSystemMessageManager::sendFakeSystemMessage(alarmMaximumReachedString);
	}
	else if ((hours < 0) || (hours > 8760) ||
		(minutes < 0) || (minutes > 60))
	{
		CuiSystemMessageManager::sendFakeSystemMessage(CuiStringIdsAlarm::alarm_in_invalid.localize());
	}
	else if((soundId >= 0) &&
	        (soundId <= getSoundCount()))
	{
		// Set the alarm parameters

		AlarmManager::Alarm alarm;

		alarm.setId(getNextAlarmId());
		alarm.snooze(hours, minutes);
		alarm.setRecurTime(hours, minutes);
		alarm.setMessage(message);
		alarm.setSoundId(soundId);
		alarm.setRecurring(recurring);

		IGNORE_RETURN(s_alarms->insert(std::make_pair(alarm.getId(), alarm)));
		result = alarm.getId();
	}
	else
	{
		DEBUG_FATAL(true, ("Invalid alarm sound id: %d", soundId));
	}

	return result;
}

//-----------------------------------------------------------------------------
bool AlarmManager::removeAlarm(int const alarmId)
{
	bool result = false;
	Alarms::iterator iterAlarms = s_alarms->find(alarmId);

	if (iterAlarms != s_alarms->end())
	{
		result = true;

		// Remove the alarm

		s_alarms->erase(iterAlarms);
	}

	return result;
}

//-----------------------------------------------------------------------------
bool AlarmManager::snoozeAlarm(int const alarmId, int const hours, int const minutes)
{
	bool result = false;
	Alarms::iterator iterAlarms = s_alarms->find(alarmId);

	if (iterAlarms != s_alarms->end())
	{
		result = true;

		iterAlarms->second.snooze(hours, minutes);
	}
	else
	{
		// Invalid alarmId

		CuiStringVariablesData data;
		data.digit_i = alarmId;

		Unicode::String alarmIdInvalidString;
		CuiStringVariablesManager::process(CuiStringIdsAlarm::alarm_id_invalid, data, alarmIdInvalidString);

		CuiSystemMessageManager::sendFakeSystemMessage(alarmIdInvalidString);
	}

	return result;
}

//-----------------------------------------------------------------------------
int AlarmManager::getSoundCount()
{
	return static_cast<int>(getAlarmSounds().size());
}

//-----------------------------------------------------------------------------
void AlarmManager::alter(float const deltaTime)
{
	UNREF(deltaTime);

	Alarms::iterator iterAlarms = s_alarms->begin();

	for (; iterAlarms != s_alarms->end(); ++iterAlarms)
	{
		if (iterAlarms->second.isExpired())
		{
			// Signal the alarm

			CuiStringVariablesData alarmIdData;
			alarmIdData.digit_i = iterAlarms->second.getId();

			Unicode::String alarmExpiredString;
			CuiStringVariablesManager::process(CuiStringIdsAlarm::alarm_expired, alarmIdData, alarmExpiredString);

			alarmExpiredString.append(1, ' ');

			Unicode::String alarmMessage;
			iterAlarms->second.getMessage(alarmMessage);
			alarmExpiredString.append(alarmMessage);

			CuiSystemMessageManager::sendFakeSystemMessage(alarmExpiredString);

			if ((iterAlarms->second.getSoundId() >= 0) &&
			    (iterAlarms->second.getSoundId() < getSoundCount()))
			{
				// Play a sound

				unsigned int const soundIndex = static_cast<unsigned int>(iterAlarms->second.getSoundId());

				std::string const &alarmSoundPath = s_alarmSounds[soundIndex];

				Audio::playSound(alarmSoundPath.c_str(), NULL);
			}

			// Remove the alarm from the list if it does not repeat daily

			if (iterAlarms->second.isRecurring())
			{
				iterAlarms->second.advance();
			}
			else
			{
				s_alarms->erase(iterAlarms);
			}

			Game::gameOptionChanged();

			break;
		}
	}
}

//-----------------------------------------------------------------------------
void AlarmManager::getAlarms(AlarmIdVector &alarms)
{
	alarms.clear();
	alarms.reserve(s_alarms->size());

	Alarms::const_iterator iterAlarms = s_alarms->begin();

	for (; iterAlarms != s_alarms->end(); ++iterAlarms)
	{
		alarms.push_back(iterAlarms->first);
	}
}

//-----------------------------------------------------------------------------
int AlarmManager::getAlarmCount()
{
	return static_cast<int>(s_alarms->size());
}

//-----------------------------------------------------------------------------
bool AlarmManager::isAlarmIdValid(int const alarmId)
{
	bool result = false;
	Alarms::iterator iterAlarms = s_alarms->find(alarmId);

	if (iterAlarms != s_alarms->end())
	{
		result = true;
	}

	return result;
}

//-----------------------------------------------------------------------------
void AlarmManager::getAlarmExpireTimeString(int const alarmId, Unicode::String &alarmExpireTimeString)
{
	Alarms::iterator iterAlarms = s_alarms->find(alarmId);

	if (iterAlarms != s_alarms->end())
	{
		iterAlarms->second.getExpireTimeString(alarmExpireTimeString);
	}
	else
	{
		alarmExpireTimeString.clear();
	}
}

//-----------------------------------------------------------------------------
void AlarmManager::getAlarmMessage(int const alarmId, Unicode::String &alarmMessage)
{
	Alarms::iterator iterAlarms = s_alarms->find(alarmId);

	if (iterAlarms != s_alarms->end())
	{
		iterAlarms->second.getMessage(alarmMessage);
	}
	else
	{
		alarmMessage.clear();
	}
}

//-----------------------------------------------------------------------------
bool AlarmManager::isFull()
{
	return (s_alarms->size() == s_maxAlarms);
}

//-----------------------------------------------------------------------------
int AlarmManager::getMaxAlarmCount()
{
	return s_maxAlarms;
}

//-----------------------------------------------------------------------------
AlarmManager::StringVector const &AlarmManager::getAlarmSounds()
{
	return s_alarmSounds;
}

//-----------------------------------------------------------------------------
int AlarmManager::getSecondsPerMinute()
{
	return s_secondsPerMinute;
}

//-----------------------------------------------------------------------------
int AlarmManager::getSecondsPerHour()
{
	return s_secondsPerHour;
}

//-----------------------------------------------------------------------------
int AlarmManager::getSecondsPerDay()
{
	return s_secondsPerDay;
}

// ============================================================================
