// ============================================================================
// 
// AlarmManager_Alarm.cpp
// Copyright Sony Online Entertainment, Inc.
//
// ============================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/AlarmManager_Alarm.h"

#include "clientUserInterface/CuiStringIdsAlarm.h"
#include "clientUserInterface/CuiStringVariablesData.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "clientUserInterface/CuiUtils.h"

// ============================================================================
//
// AlarmManager::Alarm
//
// ============================================================================

//-----------------------------------------------------------------------------
AlarmManager::Alarm::Alarm()
 : m_id(AlarmManager::invalidAlarmId)
 , m_expireTime(CuiUtils::GetSystemSeconds())
 , m_recurTime(0)
 , m_recurring(false)
 , m_message()
 , m_soundId(-1)
{
}

//-----------------------------------------------------------------------------
void AlarmManager::Alarm::advance()
{
	m_expireTime += m_recurTime;
}

//-----------------------------------------------------------------------------
void AlarmManager::Alarm::setId(int const id)
{
	m_id = id;
}

//-----------------------------------------------------------------------------
void AlarmManager::Alarm::setExpireTime(time_t const expireTime)
{
	m_expireTime = expireTime;
}

//-----------------------------------------------------------------------------
void AlarmManager::Alarm::setRecurTime(time_t const recurTime)
{
	m_recurTime = recurTime;
}

//-----------------------------------------------------------------------------
void AlarmManager::Alarm::setRecurTime(int const hours, int const minutes)
{
	m_recurTime = getSeconds(hours, minutes);
}

//-----------------------------------------------------------------------------
void AlarmManager::Alarm::setRecurring(bool const recurring)
{
	m_recurring = recurring;
}

//-----------------------------------------------------------------------------
void AlarmManager::Alarm::setMessage(Unicode::String const &message)
{
	m_message = message;
}

//-----------------------------------------------------------------------------
void AlarmManager::Alarm::setSoundId(int const soundId)
{
	m_soundId = soundId;
}

//-----------------------------------------------------------------------------
int AlarmManager::Alarm::getId() const
{
	return m_id;
}

//-----------------------------------------------------------------------------
time_t AlarmManager::Alarm::getExpireTime() const
{
	return m_expireTime;
}

//-----------------------------------------------------------------------------
bool AlarmManager::Alarm::isRecurring() const
{
	return m_recurring;
}

//-----------------------------------------------------------------------------
int AlarmManager::Alarm::getSoundId() const
{
	return m_soundId;
}

//-----------------------------------------------------------------------------
void AlarmManager::Alarm::getMessage(Unicode::String &message) const
{
	message = m_message;
}

//-----------------------------------------------------------------------------
void AlarmManager::Alarm::getExpireTimeString(Unicode::String &expireTimeString) const
{
	expireTimeString.clear();

	// Alarm id

	expireTimeString.append(CuiStringIdsAlarm::alarm_id_expires.localize());

	// Expire time

	Unicode::String alarmExpireTimeString;
	IGNORE_RETURN(CuiUtils::FormatDate(alarmExpireTimeString, getExpireTime()));

	expireTimeString.append(1, ' ');
	expireTimeString.append(alarmExpireTimeString);

	// Remaining time

	Unicode::String timeRemainingString;
	getTimeRemainingString(timeRemainingString);

	CuiStringVariablesData remainingTimeData;
	remainingTimeData.targetName = timeRemainingString;

	Unicode::String alarmTimeRemainingString;
	CuiStringVariablesManager::process(CuiStringIdsAlarm::alarm_remaining, remainingTimeData, alarmTimeRemainingString);

	expireTimeString.append(1, ' ');
	expireTimeString.append(alarmTimeRemainingString);

	// Repeat

	if (isRecurring())
	{
		expireTimeString.append(1, ' ');
		expireTimeString.append(CuiStringIdsAlarm::alarm_recurring.localize());
	}

	// Message

	expireTimeString.append(1, ' ');
	expireTimeString.append(m_message);
}

//-----------------------------------------------------------------------------
void AlarmManager::Alarm::getTimeRemainingString(Unicode::String &timeRemainingString) const
{
	timeRemainingString.clear();

	time_t const seconds = getExpireTime() - CuiUtils::GetSystemSeconds();
	bool const showDays = false;
	bool const showHours = true;
	bool const showMins = true;
	bool const showSecs = false;
	bool const shortHand = true;

	IGNORE_RETURN(CuiUtils::FormatTimeDuration(timeRemainingString, seconds, showDays, showHours, showMins, showSecs, shortHand));
}

//-----------------------------------------------------------------------------
void AlarmManager::Alarm::snooze(int const hours, int const minutes)
{
	// Push the start time back

	m_expireTime += getSeconds(hours, minutes);;
}

//-----------------------------------------------------------------------------
bool AlarmManager::Alarm::isExpired() const
{
	return (CuiUtils::GetSystemSeconds() >= getExpireTime());
}

//-----------------------------------------------------------------------------
time_t AlarmManager::Alarm::getSeconds(int const hours, int const minutes)
{
	return static_cast<time_t>(minutes) * AlarmManager::getSecondsPerMinute() + static_cast<time_t>(hours) * AlarmManager::getSecondsPerHour();
}

// ============================================================================
