//======================================================================
//
// QuestJournalManager.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/QuestJournalManager.h"

#include "clientAudio/Audio.h"
#include "clientAudio/SoundId.h"
#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientWaypointObject.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/PlayerObject.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedGame/Quest.h"
#include "sharedGame/QuestManager.h"
#include "sharedGame/QuestTask.h"
#include "sharedMessageDispatch/Transceiver.h"

#include <map>
#include <vector>

namespace QuestJournalManagerNamespace
{
	typedef std::multimap<QuestJournalManager::QuestDataKey, QuestJournalManager::CounterData> CounterMap;
	CounterMap ms_counters;

	typedef std::map<QuestJournalManager::QuestDataKey, Location> LocationMap;
	LocationMap ms_locations;

	typedef std::multimap<QuestJournalManager::QuestDataKey, QuestJournalManager::TimerData> TimerMap;
	TimerMap ms_timers;

	bool ms_playingAlarmSound = false;

	char const * const cms_alarmSoundPath = "sound/amb_alarm_air_raid_lp.snd";

	std::vector<TimerMap::iterator> ms_iteratorsToErase;

	namespace Transceivers
	{
		MessageDispatch::Transceiver<const QuestJournalManager::Messages::CountersChanged::Payload &, QuestJournalManager::Messages::CountersChanged>
			countersChanged;

		MessageDispatch::Transceiver<const QuestJournalManager::Messages::LocationsChanged::Payload &, QuestJournalManager::Messages::LocationsChanged>
			locationsChanged;

		MessageDispatch::Transceiver<const QuestJournalManager::Messages::TimersChanged::Payload &, QuestJournalManager::Messages::TimersChanged>
			timersChanged;
	}

	float const cms_questTimerAlarmTime = 10.0f;

	void startAlarmSound()
	{
//		if(ms_alarmId.isValid())
//			Audio::stopSound(ms_alarmId);
//		ms_alarmId = Audio::playSound(cms_alarmSoundPath, NULL);
		ms_playingAlarmSound = true;
	}

	void stopAlarmSound()
	{
//		if(ms_alarmId.isValid())
//		{
//			Audio::stopSound(ms_alarmId);
//			ms_alarmId.invalidate();
//		}
		ms_playingAlarmSound = false;
	}

	uint32 ms_selectedQuestCrc = 0;
	NetworkId ms_selectedMissionNid;
	QuestJournalManager::QuestType ms_questType = QuestJournalManager::QT_quest;

	NetworkId ms_selectedPlayerQuestId;
}

using namespace QuestJournalManagerNamespace;

//----------------------------------------------------------------------

void QuestJournalManager::install()
{
	InstallTimer const installTimer("QuestJournalManager::install");

	ms_counters.clear();
	ms_locations.clear();
	ms_timers.clear();
}

//----------------------------------------------------------------------

void QuestJournalManager::remove()
{
	ms_counters.clear();
	ms_locations.clear();
	ms_timers.clear();
	return;
}

//----------------------------------------------------------------------

void QuestJournalManager::setCounter(std::string const & questName, int const taskId, Unicode::String const & sourceName, int const counter, int const counterMax)
{
	uint32 questCrc = Crc::calculate(questName.c_str());
	QuestDataKey key(questCrc, taskId);
	CounterData data(sourceName, counter, counterMax);

	std::pair<CounterMap::iterator, CounterMap::iterator> const range = ms_counters.equal_range(key);
	std::vector<QuestJournalManager::CounterData> result;
	bool foundMatch = false;
	for(CounterMap::iterator i = range.first; i != range.second && !foundMatch; ++i)
	{
		CounterData & d = i->second;
		if(d.sourceName == data.sourceName)
		{
			d.counter = data.counter;
			d.counterMax = data.counterMax;
			foundMatch = true;
		}
	}

	if(!foundMatch)
		IGNORE_RETURN(ms_counters.insert(std::make_pair(key, data)));

	Transceivers::countersChanged.emitMessage(true);
}

//----------------------------------------------------------------------

std::vector<QuestJournalManager::CounterData> QuestJournalManager::getCounters(uint32 const questCrc, int const taskId)
{
	std::pair<CounterMap::iterator, CounterMap::iterator> const range = ms_counters.equal_range(QuestDataKey(questCrc, taskId));
	std::vector<QuestJournalManager::CounterData> result;
	for(CounterMap::iterator i = range.first; i != range.second; ++i)
	{
		result.push_back(i->second);
	}

	if(Game::getSinglePlayer())
	{
		result.push_back(CounterData(Unicode::narrowToWide("@ui:ok"), 3, 10));
	}

	return result;
}

//----------------------------------------------------------------------

bool QuestJournalManager::getCounterValues(uint32 questCrc, int taskId, int & counter, int & counterMax)
{
	bool result = false;
	counter = 0;
	counterMax = 0;

	std::pair<CounterMap::iterator, CounterMap::iterator> const range = ms_counters.equal_range(QuestDataKey(questCrc, taskId));

	for (CounterMap::iterator i = range.first; i != range.second; ++i)
	{
		counter += i->second.counter;
		counterMax += i->second.counterMax;
		result = true;
	}

	return result;
}

//----------------------------------------------------------------------

void QuestJournalManager::setLocation(std::string const & questName, int const taskId, Location const & location)
{
	uint32 questCrc = Crc::calculate(questName.c_str());

	ms_locations[QuestDataKey(questCrc, taskId)] = location;

	DEBUG_REPORT_LOG(false, ("Location: [%-8.2f %-8.2f %-8.2f] [%s] [%s]\n",
		 location.getCoordinates().x, location.getCoordinates().y, location.getCoordinates().z, location.getSceneId(), location.getCell().getValueString().c_str()));

	Transceivers::locationsChanged.emitMessage(true);
}

//----------------------------------------------------------------------

Location const * const QuestJournalManager::getLocation(uint32 questCrc, int taskId)
{
	LocationMap::iterator it = ms_locations.find(QuestDataKey(questCrc, taskId));

	if (it != ms_locations.end())
	{
		return &it->second;
	}

	return NULL;
}

//----------------------------------------------------------------------

void QuestJournalManager::setTimer(std::string const & questName, int taskId, Unicode::String const & sourceName, int playedTimeTimerEnd)
{
	uint32 questCrc = Crc::calculate(questName.c_str());
	QuestDataKey key(questCrc, taskId);
	PlayerObject const * const playerObject = Game::getPlayerObject();
	if(!playerObject)
		return;

	int timeLeft = static_cast<int>(static_cast<unsigned int>(playedTimeTimerEnd) - playerObject->getPlayedTime());
	timeLeft = std::max(0, timeLeft);
	TimerData data(sourceName, playedTimeTimerEnd, static_cast<float>(timeLeft));

	std::pair<TimerMap::iterator, TimerMap::iterator> const range = ms_timers.equal_range(key);
	std::vector<QuestJournalManager::TimerData> result;
	bool foundMatch = false;
	for(TimerMap::iterator i = range.first; i != range.second && !foundMatch; ++i)
	{
		TimerData & d = i->second;
		if(d.sourceName == data.sourceName)
		{
			d.playedTimeTimerEnd = data.playedTimeTimerEnd;
			d.timeLeft = data.timeLeft;
			foundMatch = true;
		}
	}

	if(!foundMatch)
		IGNORE_RETURN(ms_timers.insert(std::make_pair(key, data)));

	Transceivers::timersChanged.emitMessage(true);
}

//----------------------------------------------------------------------

std::vector<QuestJournalManager::TimerData> QuestJournalManager::getTimers(uint32 questCrc, int const taskId)
{
	std::pair<TimerMap::iterator, TimerMap::iterator> const range = ms_timers.equal_range(QuestDataKey(questCrc, taskId));
	std::vector<QuestJournalManager::TimerData> result;
	for(TimerMap::iterator i = range.first; i != range.second; ++i)
	{
		result.push_back(i->second);
	}
	return result;
}

//----------------------------------------------------------------------

void QuestJournalManager::clearTimerData()
{
	ms_timers.clear();
	stopAlarmSound();
}

//----------------------------------------------------------------------

void QuestJournalManager::update(float const )
{
	bool alarmSoundShouldPlay = false;
	ms_iteratorsToErase.clear();

	PlayerObject * const player = Game::getPlayerObject();
	if(!player)
	{
		stopAlarmSound();
		return;
	}

	for(TimerMap::iterator i = ms_timers.begin(); i != ms_timers.end(); ++i)
	{
		QuestDataKey dataKey = i->first;
		TimerData & td = i->second;
		td.timeLeft = static_cast<float>(static_cast<unsigned int>(td.playedTimeTimerEnd) - player->getPlayedTime());

		DEBUG_REPORT_LOG(false, ("timeLeft = %f ('%s' - %d:%d) [%s]\n",
			td.timeLeft,
			QuestManager::getQuestName(dataKey.questCrc).c_str(),
			dataKey.questCrc,
			dataKey.taskId,
			player->questHasActiveQuestTask(dataKey.questCrc, dataKey.taskId) ? "active" : "inactive")
			);

		if (td.timeLeft > 0.0f && td.timeLeft <= cms_questTimerAlarmTime && player->questHasActiveQuestTask(dataKey.questCrc, dataKey.taskId))
		{
			alarmSoundShouldPlay = true;
		}
		
		if (td.timeLeft <= 0.0f)
		{
			ms_iteratorsToErase.push_back(i);
		}
	}

	for(std::vector<TimerMap::iterator>::iterator j = ms_iteratorsToErase.begin(); j != ms_iteratorsToErase.end(); ++j)
	{
		ms_timers.erase(*j);
	}
	ms_iteratorsToErase.clear();

	
	if(alarmSoundShouldPlay)
	{
		if(!ms_playingAlarmSound)
		{
			CuiSystemMessageManager::sendFakeSystemMessage (CuiStringIds::quest_10_seconds_left.localize());
			startAlarmSound();
		}
	}
	else
	{
		stopAlarmSound();
	}
}

//----------------------------------------------------------------------

ClientWaypointObject const * QuestJournalManager::getQuestWaypoint(uint32 questCrc, int taskId)
{
	Quest const * const q = QuestManager::getQuest(questCrc);
	if(!q)
		return NULL;
	QuestTask const * const t = q->getTask(taskId);
	if(!t)
		return NULL;

	Location const & loc = t->getWaypointLocation();

	ClientWaypointObject const * questWaypoint = NULL;

	ClientWaypointObject::ConstWaypointVector const & wps = ClientWaypointObject::getRegularWaypoints();
	for(ClientWaypointObject::ConstWaypointVector::const_iterator i = wps.begin(); i != wps.end(); ++i)
	{
		ConstWatcher<ClientWaypointObject> const & wpWatcher = *i;
		ClientWaypointObject const * const wp = wpWatcher.getPointer();
		if(wp)
		{
			std::string const & wpPlanetName = wp->getPlanetName();
			Vector const & wpLocation = wp->getLocation();

			std::string const & planetName = loc.getSceneId();
			Vector const & location = loc.getCoordinates();

			if(wpPlanetName == planetName)
			{
				if(wpLocation.withinEpsilon(location, 0.1f))
				{
					questWaypoint = wp;
					break;
				}
			}
		}
	}

	return questWaypoint;
}

//----------------------------------------------------------------------

void QuestJournalManager::toggleQuestWaypoint(uint32 questCrc, int taskId)
{
	ClientWaypointObject const * const questWaypoint = getQuestWaypoint(questCrc, taskId);

	if (questWaypoint)
	{
		if ((Game::getSceneId() == questWaypoint->getPlanetName()))
			ClientCommandQueue::enqueueCommand(questWaypoint->isWaypointActive() ? "deactivate" : "activate",
				questWaypoint->getNetworkId(), Unicode::emptyString);
	}
}

//----------------------------------------------------------------------

void QuestJournalManager::activateQuestWaypoint(uint32 questCrc, int taskId)
{
	ClientWaypointObject const * const questWaypoint = getQuestWaypoint(questCrc, taskId);

	if (questWaypoint)
	{
		if ((Game::getSceneId() == questWaypoint->getPlanetName()) && !questWaypoint->isWaypointActive())
			ClientCommandQueue::enqueueCommand("activate", questWaypoint->getNetworkId(), Unicode::emptyString);
	}
}

//----------------------------------------------------------------------

QuestJournalManager::QuestType QuestJournalManager::getSelectedQuestType()
{
	return ms_questType;
}

//----------------------------------------------------------------------

void QuestJournalManager::setSelectedQuestCrc(uint32 const questCrc)
{
	ms_selectedQuestCrc = questCrc;
	ms_questType = QT_quest;
}

//----------------------------------------------------------------------

uint32 QuestJournalManager::getSelectedQuestCrc()
{
	return ms_selectedQuestCrc;
}

//----------------------------------------------------------------------

void QuestJournalManager::setSelectedMissionNid(NetworkId const & missionId)
{
	ms_selectedMissionNid = missionId;
	ms_questType = QT_mission;
}

//----------------------------------------------------------------------

NetworkId const & QuestJournalManager::getSelectedMissionNid()
{
	return ms_selectedMissionNid;
}

//----------------------------------------------------------------------

void QuestJournalManager::setSelectedPlayerQuestId(NetworkId const & playerQuestId)
{
	ms_selectedPlayerQuestId = playerQuestId;
	ms_questType = QT_playerQuest;
}

//----------------------------------------------------------------------

NetworkId const & QuestJournalManager::getSelectedPlayerQuestId()
{
	return ms_selectedPlayerQuestId;
}

//----------------------------------------------------------------------