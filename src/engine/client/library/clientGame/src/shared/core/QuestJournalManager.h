//======================================================================
//
// QuestJournalManager.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_QuestJournalManager_H
#define INCLUDED_QuestJournalManager_H

//======================================================================

#include "Unicode.h"

class ClientWaypointObject;
class Location;
class NetworkId;

//----------------------------------------------------------------------

class QuestJournalManager
{
public:

	enum QuestType
	{
		QT_quest,
		QT_mission,
		QT_playerQuest
	};

	struct Messages
	{
		struct CountersChanged
		{
			typedef bool Payload;
		};

		struct LocationsChanged
		{
			typedef bool Payload;
		};

		struct TimersChanged
		{
			typedef bool Payload;
		};
	};

	struct QuestDataKey
	{
		uint32 questCrc;
		int taskId;

		QuestDataKey(uint32 crc, int id)
		: questCrc(crc),
		  taskId(id) 
		{}

		bool operator <(QuestDataKey const & rhs) const
		{
			if(questCrc < rhs.questCrc)
				return true;
			else if (questCrc > rhs.questCrc)
				return false;
			else
				return taskId < rhs.taskId;
		}

	private:
		QuestDataKey();
	};

	struct CounterData
	{
		Unicode::String sourceName;
		int counter;
		int counterMax;

		CounterData(Unicode::String const & source, int c,	int cMax) : sourceName(source), counter(c), counterMax(cMax) {}

	private:
		CounterData();
	};
	typedef stdvector<CounterData>::fwd CounterDataVector;

	struct TimerData
	{
		Unicode::String sourceName;
		int playedTimeTimerEnd;
		float timeLeft;

		TimerData(Unicode::String const & source, int t, float tMax) : sourceName(source), playedTimeTimerEnd(t), timeLeft(tMax) {}

	private:
		TimerData();
	};
	typedef stdvector<TimerData>::fwd TimerDataVector;

	static void install();
	static void remove();

	static void setCounter(std::string const & questName, int taskId, Unicode::String const & sourceName, int counter, int counterMax);
	static stdvector<CounterData>::fwd getCounters(uint32 questCrc, int taskId);
	static bool getCounterValues(uint32 questCrc, int taskId, int & counter, int & counterMax);

	static void setLocation(std::string const & questName, int taskId, Location const & location);
	static Location const * const getLocation(uint32 questCrc, int taskId);

	static void setTimer(std::string const & questName, int taskId, Unicode::String const & sourceName, int timerLength);
	static stdvector<TimerData>::fwd getTimers(uint32 questCrc, int taskId);
	static void clearTimerData();

	static void update(float deltaTimeSecs);
	static void toggleQuestWaypoint(uint32 questCrc, int taskId);
	static void activateQuestWaypoint(uint32 questCrc, int taskId);
	static ClientWaypointObject const * getQuestWaypoint(uint32 questCrc, int taskId);

	static QuestType getSelectedQuestType();
	static void setSelectedQuestCrc(uint32 questCrc);
	static uint32 getSelectedQuestCrc();
	static void setSelectedMissionNid(NetworkId const & missionId);
	static NetworkId const & getSelectedMissionNid();
	static void setSelectedPlayerQuestId(NetworkId const & playerQuestId);
	static NetworkId const & getSelectedPlayerQuestId();
};

//======================================================================

#endif
