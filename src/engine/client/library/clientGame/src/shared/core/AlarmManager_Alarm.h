// ============================================================================
// 
// AlarmManager_Alarm.h
// Copyright Sony Online Entertainment, Inc.
//
// ============================================================================

#ifndef INCLUDED_AlarmManager_Alarm_H
#define INCLUDED_AlarmManager_Alarm_H

#include "AlarmManager.h"

//-----------------------------------------------------------------------------
class AlarmManager::Alarm
{
public:

	Alarm();

	void   setId(int const id);
	void   setExpireTime(time_t const expireTime);
	void   setRecurTime(time_t const recurTime);
	void   setRecurTime(int const hours, int const minutes);
	void   setRecurring(bool const recurring);
	void   setMessage(Unicode::String const &message);
	void   setSoundId(int const soundId);

	int    getId() const;
	time_t getExpireTime() const;
	bool   isRecurring() const;
	void   getExpireTimeString(Unicode::String &expireTimeString) const;
	void   getTimeRemainingString(Unicode::String &timeRemainingString) const;
	void   getMessage(Unicode::String &message) const;
	int    getSoundId() const;

	bool   isExpired() const;
	void   snooze(int const hours, int const minutes);
	void   advance(); // Moves the alarm forward by the recurTime

private:

	int             m_id;
	time_t          m_expireTime;
	time_t          m_recurTime;
	bool            m_recurring;
	Unicode::String m_message;
	int             m_soundId;

	time_t getSeconds(int const hours, int const minutes);
};

// ============================================================================

#endif // INCLUDED_AlarmManager_Alarm_H
