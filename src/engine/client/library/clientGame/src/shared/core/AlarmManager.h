// ============================================================================
// 
// AlarmManager.h
// Copyright Sony Online Entertainment, Inc.
//
// ============================================================================

#ifndef INCLUDED_AlarmManager_H
#define INCLUDED_AlarmManager_H

//-----------------------------------------------------------------------------
// This handles the in-game alarm the user can set to notify themself that they 
// have been doing something for a certain period of time.
//-----------------------------------------------------------------------------
class AlarmManager
{
public:

	static void install();

public:

	typedef stdvector<Unicode::String>::fwd UnicodeStringVector;
	typedef stdvector<int>::fwd             AlarmIdVector;
	typedef stdvector<std::string>::fwd     StringVector;

	static int const invalidAlarmId;

	static int  addAlarmAt(int const hour, int const minute, Unicode::String const &message, int const soundId, bool const recurring);
	static int  addAlarmIn(int const hours, int const minutes, Unicode::String const &message, int const soundId, bool const recurring);
	static bool removeAlarm(int const alarmId);
	static bool snoozeAlarm(int const alarmId, int const hours, int const minutes);
	static void alter(float const deltaTime);
	static bool isAlarmIdValid(int const alarmId);
	static bool isFull();
	static int  getMaxAlarmCount();

	static void getAlarms(AlarmIdVector &alarms);
	static int  getAlarmCount();
	static void getAlarmExpireTimeString(int const alarmId, Unicode::String &alarmExpireTimeString);
	static void getAlarmMessage(int const alarmId, Unicode::String &alarmMessage);
	static int  getSoundCount();

	static int   getSecondsPerMinute();
	static int   getSecondsPerHour();
	static int   getSecondsPerDay();

	static StringVector const &getAlarmSounds();

	class Alarm;

private:

	static void remove();

	// Disable

	AlarmManager();
	~AlarmManager();
	AlarmManager(AlarmManager const &);
	AlarmManager &operator =(AlarmManager const &);
};

// ============================================================================

#endif // INCLUDED_AlarmManager_H
