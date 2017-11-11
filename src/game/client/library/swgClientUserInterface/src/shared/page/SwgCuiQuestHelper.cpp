// ======================================================================
//
// SwgCuiQuestHelper.cpp
// Copyright (c) 2005 Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiQuestHelper.h"

// ----------------------------------------------------------------------

#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientMissionObject.h"
#include "clientGame/ClientPlayerQuestObject.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/PlayerObject.h"
#include "clientGame/QuestJournalManager.h"
#include "clientGame/WorldSnapshot.h"
#include "clientGraphics/Camera.h"
#include "clientUserInterface/CuiActionManager.h"
#include "clientUserInterface/CuiActions.h"
#include "clientUserInterface/CuiInventoryManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiMissionManager.h"
#include "clientUserInterface/CuiPlayerQuestManager.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiSoundManager.h"
#include "clientUserInterface/CuiSounds.h"
#include "clientUserInterface/CuiUtils.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/Timer.h"
#include "sharedGame/GroundZoneManager.h"
#include "sharedGame/Quest.h"
#include "sharedGame/QuestManager.h"
#include "sharedGame/QuestTask.h"
#include "sharedGame/SharedBuildoutAreaManager.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/CachedNetworkId.h"
#include "sharedObject/Container.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/PortalProperty.h"
#include "sharedUtility/CurrentUserOptionManager.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"
#include "swgClientUserInterface/SwgCuiQuestHelper_Task.h"
#include "swgClientUserInterface/SwgCuiQuestJournal.h"

// ----------------------------------------------------------------------

#include "UIManager.h"
#include "UIMessage.h"
#include "UIComposite.h"
#include "UIEffector.h"
#include "UIText.h"

// ----------------------------------------------------------------------

#include <algorithm>
#include <vector>

// ======================================================================

namespace SwgCuiQuestHelperNamespace
{
	bool s_installed = false;
	uint32 s_questCrc = 0;

	bool const s_enableUpdateDebug = false;
	int const s_maxNumberOfTasks = 16;
	int const s_hideWaypointArrowDistance = 3;
	int const s_deltaWaypointElevation = 3;
	float const s_flashProgressTimeRemaining = 10.0f;
	float const s_minElevationIndicatorDistance = 100.0f;

	std::string const s_pathToLocationCommand("pathToLocation");
	std::string const s_pathClearCommand("pathClear");

	float const s_pathUpdatePeriod = 10.f;
	float const s_pathUpdateMinDistanceSquared = 100.f;
	float const s_minDistanceForFoundEntrance = 5.0f;

	Timer s_pathUpdateTimer(s_pathUpdatePeriod);
	Vector s_pathUpdateLastPosition;
	bool s_showingPath = false;

	Quest const * getActiveVisibleQuest(uint32 questCrc, PlayerObject const & playerObject);

	void showPath(const Vector& position);
	void clearPath(void);
}

using namespace SwgCuiQuestHelperNamespace;

// ======================================================================

SwgCuiQuestHelper::SwgCuiQuestHelper(UIPage & page)
 : SwgCuiLockableMediator("SwgCuiQuestHelper", page)
 , m_questHelperTasks(new QuestHelperTaskVector(s_maxNumberOfTasks, static_cast<Task *>(0)))
 , m_callback(new MessageDispatch::Callback)
 , m_comp(0)
 , m_sample(0)
 , m_questTitle(0)
 , m_questHighlight(0)
 , m_minSize()
 , m_lastQuestCrc(0)
 , m_lastMissionId(NetworkId::cms_invalid)
 , m_lastHelperTasksHeight(-1)
 , m_lastHelperTaskCount(0)
 , m_lastPlayerQuestId(NetworkId::cms_invalid)
{
	getCodeDataObject(TUIComposite, m_comp, "comp");
	getCodeDataObject(TUIComposite, m_sample, "sample");
	getCodeDataObject(TUIText, m_questTitle, "questTitle");
	getCodeDataObject(TUIEffector, m_questHighlight, "questHighlight");

	registerMediatorObject(getPage(), true);

	m_sample->SetVisible(false);

	m_minSize = getPage().GetMinimumSize();

}

// ----------------------------------------------------------------------

SwgCuiQuestHelper::~SwgCuiQuestHelper()
{
	clearQuestHelperTasks();

	delete m_questHelperTasks;
	m_questHelperTasks = 0;

	delete m_callback;
	m_callback = 0;

	m_comp = 0;
	m_questTitle = 0;
	m_questHighlight = 0;
	m_sample = 0;
}

// ----------------------------------------------------------------------

void SwgCuiQuestHelper::install()
{
	DEBUG_FATAL(s_installed, ("already installed."));
	s_installed = true;
}

// ----------------------------------------------------------------------

void SwgCuiQuestHelper::remove()
{
	DEBUG_FATAL (!s_installed, ("not installed\n"));
	s_installed = false;
}

// ----------------------------------------------------------------------

void SwgCuiQuestHelper::performActivate()
{
	m_callback->connect(*this, &SwgCuiQuestHelper::onCurrentQuestChanged, static_cast<PlayerObject::Messages::CurrentQuestChanged *>(0));
	m_callback->connect(*this, &SwgCuiQuestHelper::onAddedToContainer, static_cast<ClientObject::Messages::AddedToContainer *>     (0));
	m_callback->connect(*this, &SwgCuiQuestHelper::onRemovedFromContainer, static_cast<ClientObject::Messages::RemovedFromContainer *> (0));

	setIsUpdating(true);
}

// ----------------------------------------------------------------------

void SwgCuiQuestHelper::performDeactivate()
{
	m_callback->disconnect(*this, &SwgCuiQuestHelper::onCurrentQuestChanged, static_cast<PlayerObject::Messages::CurrentQuestChanged *>(0));
	m_callback->disconnect(*this, &SwgCuiQuestHelper::onAddedToContainer, static_cast<ClientObject::Messages::AddedToContainer *>     (0));
	m_callback->disconnect(*this, &SwgCuiQuestHelper::onRemovedFromContainer, static_cast<ClientObject::Messages::RemovedFromContainer *> (0));

	clearPath();

	setIsUpdating(false);
}

// ----------------------------------------------------------------------

SwgCuiQuestHelper::Task * SwgCuiQuestHelper::getQuestHelperTask(uint index)
{
	FATAL(index < 0 || index >= s_maxNumberOfTasks,
		("Invalid task index [%d]. Max supported [%d].", index, s_maxNumberOfTasks));

	Task * questHelperTask = 0;

	if (m_questHelperTasks->at(index) == 0)
	{
		UIComposite * const dupe = NON_NULL(safe_cast<UIComposite *>(m_sample->DuplicateObject()));

		if (!dupe)
			return 0;

		m_comp->AddChild(dupe);

		dupe->Link();
		dupe->SetVisible(true);

#ifdef _DEBUG
		dupe->SetName(UINarrowString(FormattedString<64>().sprintf("task%02d", index)));
#endif

		questHelperTask = new Task(*dupe);
		questHelperTask->fetch();
		questHelperTask->activate();

		m_questHelperTasks->at(index) = questHelperTask;
	}
	else
	{
		questHelperTask = m_questHelperTasks->at(index);
	}

	return questHelperTask;
}

// ----------------------------------------------------------------------

void SwgCuiQuestHelper::removeExtraQuestHelperTasks(uint helperTaskCount)
{
	for (int i = helperTaskCount; i < s_maxNumberOfTasks; ++i)
	{
		if (!m_questHelperTasks->at(i))
			break;

		m_questHelperTasks->at(i)->release();
		m_questHelperTasks->at(i) = 0;
	}
}

// ----------------------------------------------------------------------

void SwgCuiQuestHelper::clearQuestHelperTasks()
{
	std::for_each(m_questHelperTasks->begin(), m_questHelperTasks->end(), ReleaserFunctor());
	m_questHelperTasks->clear();

	m_comp->Clear();
}

// ----------------------------------------------------------------------

void SwgCuiQuestHelper::update(float deltaTimeSecs)
{
	PlayerObject * const playerObject = Game::getPlayerObject();
	bool showHelper = false;

	DEBUG_REPORT_PRINT(s_enableUpdateDebug, ("** Quest Helper Debug Info **\n"));

	s_pathUpdateTimer.updateNoReset( deltaTimeSecs );

	if (playerObject && Game::getPlayerCreature())
	{
		if (getQuestCrc() == 0 && playerObject->getCurrentQuest() != 0)
		{
			//-- set the current quest based on the value persisted in the player object
			QuestJournalManager::setSelectedQuestCrc(playerObject->getCurrentQuest());
			setupData(false, false);
		}

		QuestJournalManager::QuestType const questType = QuestJournalManager::getSelectedQuestType();
		if(questType == QuestJournalManager::QT_quest)
		{
			Quest const * const quest = getActiveVisibleQuest(getQuestCrc(), *playerObject);
			if (quest)
			{
				showHelper = true;
				updateHelperWithQuestData(playerObject, quest);
			}
			else
			{
				clearPath();
			}
		}
		else if (questType == QuestJournalManager::QT_mission)
		{
			NetworkId const & missionId = QuestJournalManager::getSelectedMissionNid();
			bool hasValidMission = false;

			if (missionId.isValid())
			{
				hasValidMission = updateHelperWithMissionData(playerObject, missionId);
				showHelper = hasValidMission;
			}

			if (!hasValidMission)
			{
				//-- select a quest or clear the helper
				QuestJournalManager::setSelectedQuestCrc(playerObject->getCurrentQuest());
				setupData(false, false);
			}
		}
		else if (questType == QuestJournalManager::QT_playerQuest)
		{
			// Do player quest update stuff here.

			NetworkId const & playerQuestId = QuestJournalManager::getSelectedPlayerQuestId();
			bool validPlayerQuest = false;

			if(playerQuestId.isValid())
			{
				validPlayerQuest = updateHelperWithPlayerQuestData(playerObject, playerQuestId);
				showHelper = validPlayerQuest;
			}

			if(!validPlayerQuest)
			{
				//-- select a quest or clear the helper
				QuestJournalManager::setSelectedQuestCrc(playerObject->getCurrentQuest());
				setupData(false, false);
			}
		}
	}

	getPage().SetVisible(showHelper && CuiPreferences::getShowQuestHelper());
}

// ----------------------------------------------------------------------

void SwgCuiQuestHelper::updateHelperWithQuestData(PlayerObject * const playerObject, Quest const * const quest)
{
	CreatureObject const * const playerCreature = Game::getPlayerCreature();
	if (!playerCreature)
	{
		return;
	}

	uint helperTaskCount = 0;
	bool const sameQuest = getQuestCrc() == m_lastQuestCrc;
	bool taskTitleChanged = false;

	if (!sameQuest)
	{
		m_questTitle->SetLocalText(quest->getJournalEntryTitle().localize());
		highlightQuest(QH_quest, QHS_none);
	}

	int const numberOfTasks = quest->getNumberOfTasks();

	DEBUG_REPORT_PRINT(s_enableUpdateDebug, ("Number of tasks = %d\n", numberOfTasks));

	bool showedPath = false;

	for (int i = 0; i < numberOfTasks; ++i)
	{
		QuestTask const * const questTask = quest->getTask(i);

		if (!questTask)
		{
			DEBUG_REPORT_PRINT(true, ("Invalid quest task %s:%d\n", quest->getName().getString(), i));
			continue;
		}

		int const taskId = questTask->getId();

		DEBUG_REPORT_PRINT(s_enableUpdateDebug, ("taskId = %d\n", taskId));
		DEBUG_REPORT_PRINT(s_enableUpdateDebug, ("i = %d\n", i));

		if (questTask && questTask->isVisible() && playerObject->questHasActiveQuestTask(getQuestCrc(), taskId))
		{
			Task * questHelperTask = getQuestHelperTask(helperTaskCount++);

			if (!questHelperTask)
			{
				DEBUG_REPORT_PRINT(true, ("Failed to get helper task %s:%d\n", quest->getName().getString(), helperTaskCount));
				continue;
			}

			bool const sameHelperTask = questHelperTask->isSameHelperTask(getQuestCrc(), taskId);

			if (!sameHelperTask)
			{
				questHelperTask->setQuestTaskData(getQuestCrc(), taskId);
				questHelperTask->setTaskTitle(questTask->getJournalEntryTitle().localize());
				taskTitleChanged = true;
			}

			//-- check if this quest has a dynamic waypoint
			Location const * const dynamicWaypointLocation = QuestJournalManager::getLocation(getQuestCrc(), taskId);

			//-- if it does, use it instead of the static one defined in the data table
			Location const waypointLocation = dynamicWaypointLocation ? *dynamicWaypointLocation : questTask->getWaypointLocation();

			showedPath = setupWaypointForTask(questTask, questHelperTask, waypointLocation, sameHelperTask, questTask->isInteriorWaypoint(), dynamicWaypointLocation);

			//-- timer	
			{
				QuestJournalManager::TimerDataVector const timers = QuestJournalManager::getTimers(getQuestCrc(), taskId);
				QuestJournalManager::TimerDataVector::const_iterator i = timers.begin();
				QuestJournalManager::TimerDataVector::const_iterator iEnd = timers.end();

				//-- the helper only supports one timer so use the first one
				if (i != iEnd)
				{
					DEBUG_REPORT_PRINT(s_enableUpdateDebug, ("hasTimer\n"));

					QuestJournalManager::TimerData const data = *i;

					if (!sameHelperTask || (sameHelperTask && questHelperTask->timerValueChanged(data.timeLeft)))
					{
						questHelperTask->setTimerValue(data.timeLeft);

						Unicode::String timeWide;
						IGNORE_RETURN(CuiUtils::FormatTimeDuration(timeWide,
							static_cast<unsigned int>(data.timeLeft), false, false, true, true, true));

						questHelperTask->setTaskProgress(timeWide, data.timeLeft <= s_flashProgressTimeRemaining ? Task::PH_flash : Task::PH_none);
					}
					continue;
				}
			}

			//-- counter
			{
				int counter, counterMax;

				//-- show the sum of the counters for each task
				if (QuestJournalManager::getCounterValues(getQuestCrc(), taskId, counter, counterMax))
				{
					DEBUG_REPORT_PRINT(s_enableUpdateDebug, ("hasCounter\n"));

					bool const hasCounter = questHelperTask->hasCounter();

					if (!sameHelperTask || (sameHelperTask && questHelperTask->counterValuesChanged(counter, counterMax)))
					{
						questHelperTask->setCounterValues(counter, counterMax);

						std::string const result = FormattedString<256>().sprintf("%d/%d", counter, counterMax);

						questHelperTask->setTaskProgress(Unicode::narrowToWide(result.c_str()), Task::PH_fadeFull);

						//-- highlight the task when the counter changes
						if (sameHelperTask && hasCounter && counter != 0)
							questHelperTask->highlightTask(Task::TH_task, Task::THS_counter);
					}
					continue;
				}
			}
		}
	}

	if (s_showingPath && !showedPath)
	{
		clearPath();
	}

	removeExtraQuestHelperTasks(helperTaskCount);

	finishSharedHelperUpdate(taskTitleChanged, sameQuest, helperTaskCount, getQuestCrc(), NetworkId::cms_invalid);
}

// ----------------------------------------------------------------------

bool SwgCuiQuestHelper::updateHelperWithMissionData(PlayerObject const * const /*playerObject*/, NetworkId const & missionId)
{
	Object const * const o = NetworkIdManager::getObjectById(missionId);
	ClientMissionObject const * const m = dynamic_cast<ClientMissionObject const *>(o);
	if (!m)
	{
		return false;
	}

	bool const sameMission = missionId == m_lastMissionId;
	if (!sameMission)
	{
		m_questTitle->SetLocalText(m->getTitle().localize());
	}

	bool showedPath = false;

	Task * questHelperTask = getQuestHelperTask(0);
	if (questHelperTask)
	{
		questHelperTask->setTaskTitle(StringId::decodeString(Unicode::narrowToWide(m->getTargetName())));
		questHelperTask->setQuestTaskData(0, 0);
	}

	ClientWaypointObject const * const waypointLocation = m->getLocalWaypoint();
	if (waypointLocation && waypointLocation->isWaypointVisible())
	{
		Location waypointLocation(waypointLocation->getLocation(), waypointLocation->getCell(), Crc::calculate(waypointLocation->getPlanetName().c_str()));
		showedPath = setupWaypointForTask(NULL, questHelperTask, waypointLocation, false, false, NULL);
	}

	if (s_showingPath && !showedPath)
	{
		clearPath();
	}

	removeExtraQuestHelperTasks(1);

	finishSharedHelperUpdate(false, false, 1, 0, missionId);

	return true;
}

// ----------------------------------------------------------------------

void SwgCuiQuestHelper::finishSharedHelperUpdate(bool const taskTitleChanged, bool const sameQuest, uint const helperTaskCount, uint32 const questCrc, NetworkId const & missionId, bool isPlayerQuest)
{
	bool const forceResize = false;
	long const helperTasksHeight = resizeHelperTasks(forceResize);

	//-- auto resize based on the number of quest helper tasks
	if (m_lastHelperTasksHeight != helperTasksHeight)
	{
		long const height = m_minSize.y + helperTasksHeight;

		//-- prevent vertical resizing
		UISize const maxSize(getPage().GetMaximumSize().x, height);
		UISize const minSize(getPage().GetMinimumSize().x, height);

		getPage().SetMaximumSize(maxSize);
		getPage().SetMinimumSize(minSize);
		getPage().SetMaximumSize(maxSize);

		getPage().SetHeight(height);
	}

	//-- check for step completion
	if (sameQuest && (taskTitleChanged || helperTaskCount < m_lastHelperTaskCount))
	{
		highlightQuest(QH_title, QHS_stepCompleted);
	}

	m_lastQuestCrc = questCrc;
	
	if(!isPlayerQuest)
		m_lastMissionId = missionId;
	else
		m_lastPlayerQuestId = missionId;

	m_lastHelperTasksHeight = helperTasksHeight;
	m_lastHelperTaskCount = helperTaskCount;
}

// ----------------------------------------------------------------------

bool SwgCuiQuestHelper::setupWaypointForTask(QuestTask const * const questTask, Task * const questHelperTask, Location const & waypointLocation, bool const sameHelperTask, bool const /*isInteriorWaypoint*/, Location const * const dynamicWaypointLocation)
{
	bool showedPath = false;
	CreatureObject const * const playerCreature = Game::getPlayerCreature();
	Object const * const player = Game::getPlayer();
	if (!playerCreature || !player || !questHelperTask)
	{
		return showedPath;
	}

	bool firstTaskWithWaypointFound = false;
	std::string const & sceneName   = Game::getSceneId();
	bool const inSpace              = Game::isSpace();

	Vector waypointVector = GroundZoneManager::getRelativePositionFromPlayer(sceneName.c_str(), playerCreature->getPosition_w(), waypointLocation.getCoordinates());

	//-- check for entrance waypoint, while we are in the world point to entrance
	if (questTask && questTask->hasEntranceWaypoint() && playerCreature->isInWorldCell())
	{
		std::string const & sceneName = Game::getSceneId();
		Vector const waypointEntranceVector = GroundZoneManager::getRelativePositionFromPlayer(sceneName.c_str(), playerCreature->getPosition_w(), questTask->getWaypointEntranceLocation().getCoordinates());
		bool const inSpace = Game::isSpace();

		Camera const * const camera = Game::getCamera();
		Vector const & cameraPos_w = camera->getPosition_w();
		Vector player_pos_w = playerCreature->getPosition_w();

		//-- only update the waypoint if the player position or the camera moved
		if (questHelperTask->cameraPositionChanged(cameraPos_w) || (questHelperTask->playerPositionChanged(player_pos_w)))
		{
			if (!inSpace)
				player_pos_w.y = waypointVector.y;

			int const distance = static_cast<int>(waypointEntranceVector.magnitudeBetween(player_pos_w));

			//-- have we reached the entrance? if not use entrance waypoint
			if (distance > s_minDistanceForFoundEntrance)
			{
				waypointVector = GroundZoneManager::getRelativePositionFromPlayer(sceneName.c_str(), playerCreature->getPosition_w(), questTask->getWaypointEntranceLocation().getCoordinates());
			}
		}
	} 
		
	bool hasTraversableWaypointInScene = false;
	if (questTask)
	{
		hasTraversableWaypointInScene = (questTask->hasWaypoint() || dynamicWaypointLocation)
			&& Game::calculateNonInstanceSceneId(waypointLocation.getSceneId()) == Game::getSceneId()
			&& SharedBuildoutAreaManager::isTraversable(sceneName, waypointLocation.getCoordinates(), playerCreature->getPosition_w());
	}
	else
	{
		hasTraversableWaypointInScene = Game::calculateNonInstanceSceneId(waypointLocation.getSceneId()) == Game::getSceneId()
			&& SharedBuildoutAreaManager::isTraversable(sceneName, waypointLocation.getCoordinates(), playerCreature->getPosition_w());
	}

	Task::IndicatorImage indicatorImage = hasTraversableWaypointInScene ? questHelperTask->getIndicatorImage() : Task::II_taskDot;

	//-- waypoint arrow
	{
		//-- verify a waypoint is defined and is for the current scene
		if (hasTraversableWaypointInScene)
		{
			DEBUG_REPORT_PRINT(s_enableUpdateDebug, ("hasWaypoint\n"));

			//-- determine the rotation of the waypoint arrow image
			Camera const * const camera = Game::getCamera();
			Vector const & cameraPos_w = camera->getPosition_w();

			//-- only update the waypoint if the task changed or the camera moved
			if (!sameHelperTask || (sameHelperTask && questHelperTask->cameraPositionChanged(cameraPos_w)))
			{
				questHelperTask->setCameraPosition(cameraPos_w);

				//-- make sure the indicator is an arrow unless currently showing elevation
				if (indicatorImage != Task::II_waypointAbove && indicatorImage != Task::II_waypointBelow)
					indicatorImage = Task::II_waypointArrow;

				Vector waypointPos = waypointVector;
				float theta = 0;

				if (inSpace)
				{
					Vector delta_vector = waypointPos - cameraPos_w;
					Vector const delta_in_camera = camera->rotate_w2o(delta_vector);
					bool const normalized = delta_vector.normalize();
					theta = atan2(delta_in_camera.x, delta_in_camera.y);
					float const dotProduct = camera->getObjectFrameK_w().dot(delta_vector);

					if (!normalized || acos(dotProduct) < std::min(camera->getVerticalFieldOfView(), camera->getHorizontalFieldOfView()) / 2.f)
					{
						indicatorImage = Task::II_waypointSpace;
					}
				}
				else
				{
					waypointPos.y = cameraPos_w.y;
					const float camera_angle  = camera->getObjectFrameK_w ().theta ();
					const Vector delta_vector = waypointPos - cameraPos_w;
					const float target_angle  = delta_vector.theta ();
					theta = target_angle - camera_angle;
				}

				questHelperTask->setWaypointRotation(theta / PI_TIMES_2 - 0.25f);
			}
		}
	}

	//-- waypoint distance
	{
		if (hasTraversableWaypointInScene)
		{
			Vector player_pos_w = playerCreature->getPosition_w();

			if (!sameHelperTask || (sameHelperTask && questHelperTask->playerPositionChanged(player_pos_w)))
			{
				questHelperTask->setPlayerPosition(player_pos_w);

				float const player_y = player_pos_w.y;

				//-- only test x-z distance in the ground game
				if (!inSpace)
					player_pos_w.y = waypointVector.y;

				int const distance = static_cast<int>(waypointVector.magnitudeBetween(player_pos_w));

				questHelperTask->setWaypointDistance(
					Unicode::narrowToWide(FormattedString<64>().sprintf("%dm", distance)));

				if (indicatorImage != Task::II_waypointSpace)
				{
					bool const canShowElevationIndicator = !inSpace && distance < s_minElevationIndicatorDistance
						&& !playerCreature->isInWorldCell() && (questTask && questTask->isInteriorWaypoint());

					if (canShowElevationIndicator && player_y < waypointVector.y - s_deltaWaypointElevation)
					{
						indicatorImage = Task::II_waypointAbove;
					}
					else if (canShowElevationIndicator && player_y > waypointVector.y + s_deltaWaypointElevation)
					{
						indicatorImage = Task::II_waypointBelow;
					}
					else
					{
						indicatorImage = Task::II_waypointArrow;
					}
				}
			}
		}
	}

	//-- path to location
	{
		if (hasTraversableWaypointInScene && Game::isTutorial() && !Game::isSpace())
		{
			bool forcePathUpdate = false;
			
			showedPath = s_showingPath;  //-- even if we don't update it, we want to pretend we did for later

			//-- force update the ribbon path on the first new task with a waypoint
			if (!firstTaskWithWaypointFound)
			{
				firstTaskWithWaypointFound = true;
				forcePathUpdate = !sameHelperTask;
			}

			if (forcePathUpdate || s_pathUpdateTimer.isExpired())
			{
				s_pathUpdateTimer.reset();

				if (forcePathUpdate || playerCreature->getPosition_w().magnitudeBetweenSquared(s_pathUpdateLastPosition) > s_pathUpdateMinDistanceSquared)
				{
					s_pathUpdateLastPosition = playerCreature->getPosition_w();

					showPath(waypointVector);

					showedPath = true;
				}
			}
		}
	}

	//-- task indicator image
	questHelperTask->setIndicatorImage(indicatorImage);

	return showedPath;
}

// ----------------------------------------------------------------------

void SwgCuiQuestHelper::setupData(bool checkActiveVisible, bool sendToServer)
{
	uint32 questCrc = QuestJournalManager::getSelectedQuestCrc();
	uint32 const oldCrc = s_questCrc;

	PlayerObject * const playerObject = Game::getPlayerObject();
	if(!playerObject)
		return;

	if (checkActiveVisible)
	{
		//-- ignore the set request unless it's a visible and active quest
		if (getActiveVisibleQuest(questCrc, *playerObject))
			s_questCrc = questCrc;
	}
	else
	{
		s_questCrc = questCrc;
	}

	if (s_questCrc != oldCrc && sendToServer)
	{
		playerObject->setCurrentQuest(s_questCrc);
	}
}

// ----------------------------------------------------------------------

uint32 SwgCuiQuestHelper::getQuestCrc() const
{
	return s_questCrc;
}

// ----------------------------------------------------------------------

long SwgCuiQuestHelper::resizeHelperTasks(boolean forceResize)
{
	long helperTasksHeight = 0;

	for (int i = 0; i < s_maxNumberOfTasks; ++i)
	{
		if (!m_questHelperTasks->at(i))
			break;

		m_questHelperTasks->at(i)->sizeToContent(forceResize);

		helperTasksHeight += m_questHelperTasks->at(i)->getHeight();
	}

	return helperTasksHeight;
}

// ----------------------------------------------------------------------

void SwgCuiQuestHelper::highlightQuest(QuestHighlight questHighlight, QuestHighlightSound questHighlightSound)
{
	switch (questHighlight)
	{
	case QH_none:
		break;
	case QH_title:
		m_questTitle->SetBackgroundOpacity(1.0);
		UIManager::gUIManager().ExecuteEffector(m_questHighlight, m_questTitle, false);
		break;
	case QH_quest:
		getPage().SetBackgroundOpacity(0.75);
		UIManager::gUIManager().ExecuteEffector(m_questHighlight, &getPage(), false);
		break;
	default:
		DEBUG_WARNING(true, ("No highlight effect defined!"));
		break;
	}

	switch (questHighlightSound)
	{
	case QHS_none:
		break;
	case QHS_stepCompleted:
		CuiSoundManager::play(CuiSounds::npe2_quest_step_completed);
		break;
	default:
		DEBUG_WARNING(true, ("No highlight sound defined!"));
		break;
	}
}

// ----------------------------------------------------------------------

void SwgCuiQuestHelper::onCurrentQuestChanged(PlayerObject const & player)
{
	if (&player == Game::getPlayerObject())
	{
		QuestJournalManager::setSelectedQuestCrc(player.getCurrentQuest());
		setupData(false, false);
	}
}

// ----------------------------------------------------------------------

void SwgCuiQuestHelper::onAddedToContainer(ClientObject::Messages::ContainerMsg const & msg)
{
	// See if something was just added to the datapad
	ClientObject const * const container = msg.first;
	if (container == CuiInventoryManager::getPlayerDatapad())
	{
		// See if the added object was a non-space mission object
		ClientObject const * const addedObject = msg.second;
		ClientMissionObject const * const missionObject = dynamic_cast<ClientMissionObject const *>(addedObject);
		if (missionObject && !missionObject->isSpaceMission())
		{
			QuestJournalManager::setSelectedMissionNid(missionObject->getNetworkId());
			return;
		}
		
		Container const * objectContainer = ContainerInterface::getContainer(*addedObject);
		if(!objectContainer)
			return;
		for(ContainerConstIterator iter = objectContainer->begin(); iter != objectContainer->end(); ++iter)
		{
			CachedNetworkId const & item = (*iter);
			Object const * itemObj = item.getObject();
			ClientPlayerQuestObject const * const playerQuestObject = dynamic_cast<ClientPlayerQuestObject const *>(itemObj);
			if(playerQuestObject)
			{
				QuestJournalManager::setSelectedPlayerQuestId(playerQuestObject->getNetworkId());
				return;
			}
		}

	}
}

// ----------------------------------------------------------------------

void SwgCuiQuestHelper::onRemovedFromContainer(ClientObject::Messages::ContainerMsg const & msg)
{
	// See if something was just removed to the datapad
	ClientObject const * const container = msg.first;
	if (container == CuiInventoryManager::getPlayerDatapad())
	{
		// See if the removed object was the currently selected mission object
		ClientObject const * const removedObject = msg.second;
		if (removedObject && (removedObject->getNetworkId() == QuestJournalManager::getSelectedMissionNid()))
		{
			QuestJournalManager::setSelectedMissionNid(NetworkId::cms_invalid);

			// We are clearing a mission, see if we can update the helper with a quest
			PlayerObject const * const player = Game::getPlayerObject();
			if (player && (player->getCurrentQuest() != 0))
			{
				QuestJournalManager::setSelectedQuestCrc(player->getCurrentQuest());
				setupData(false, false);
			}
			else
			{
				// The user doesn't have a selected quest, so look for another mission
				std::vector<NetworkId> const & activeMissions = CuiMissionManager::getActiveMissions();
				for(std::vector<NetworkId>::const_iterator i = activeMissions.begin(); i != activeMissions.end(); ++i)
				{
					NetworkId const & activeMissionId = (*i);
					Object const * const obj = NetworkIdManager::getObjectById(activeMissionId);
					ClientMissionObject const * const missionObj = dynamic_cast<ClientMissionObject const *>(obj);
					if(missionObj && !missionObj->isSpaceMission())
					{
						QuestJournalManager::setSelectedMissionNid(activeMissionId);
						break;
					}
				}
			}
		}
		else if(removedObject && (removedObject->getNetworkId() == QuestJournalManager::getSelectedPlayerQuestId()))
		{
			QuestJournalManager::setSelectedPlayerQuestId(NetworkId::cms_invalid);
			// We are clearing a player quest, see if we can update the helper with a quest
			PlayerObject const * const player = Game::getPlayerObject();
			if (player && (player->getCurrentQuest() != 0))
			{
				QuestJournalManager::setSelectedQuestCrc(player->getCurrentQuest());
				setupData(false, false);
			}
			else
			{
				// The user doesn't have a selected quest, so look for another player quest
				std::vector<NetworkId> const & activePQ = CuiPlayerQuestManager::getActivePlayerQuests();
				for(std::vector<NetworkId>::const_iterator i = activePQ.begin(); i != activePQ.end(); ++i)
				{
					NetworkId const & activePQID = (*i);
					Object const * const obj = NetworkIdManager::getObjectById(activePQID);
					ClientPlayerQuestObject const * const playerQuestObj = dynamic_cast<ClientPlayerQuestObject const *>(obj);
					if(playerQuestObj)
					{
						QuestJournalManager::setSelectedPlayerQuestId(activePQID);
						break;
					}
				}
			}
		}
	}
}

// ======================================================================

Quest const * SwgCuiQuestHelperNamespace::getActiveVisibleQuest(uint32 questCrc, PlayerObject const & playerObject)
{
	if (!playerObject.questHasActiveQuest(questCrc))
		return false;

	Quest const * quest = QuestManager::getQuest(questCrc);

	if (quest && quest->isVisible())
		return quest;

	return NULL;
}

// ----------------------------------------------------------------------

void SwgCuiQuestHelperNamespace::showPath(const Vector& position_w)
{
	CellProperty const * cell = CellProperty::getWorldCellProperty();

	//-- check buildout
	NetworkId cellid = WorldSnapshot::findClosestCellIdFromWorldPosition(position_w);
	
	if (cellid != NetworkId::cms_invalid) 
	{
		Object const * const object = NetworkIdManager::getObjectById(cellid);
		if (object) 
		{
			cell = object->getCellProperty();
		}
	}
	
	//-- check client sphere tree
	if (!cell || cell->isWorldCell()) 
	{
		Object const * const cellObject = ClientWorld::findClosestCellObjectFromWorldPosition(position_w);
		if (cellObject) 
		{
			cell = cellObject->getCellProperty();
			cellid = cellObject->getNetworkId();
		}
	}

	//-- see if I'm in a building, and ask the building if it knows where this point is
	if(!cell || cell->isWorldCell())
	{
		Object *playerObj = Game::getPlayer();
		if(playerObj)
		{
			const PortalProperty * portal = playerObj->getParentCell()->getPortalProperty();
			if (portal) 
			{
				Vector const position_l = portal->getOwner().rotateTranslate_w2o(position_w);
				
				CellProperty const * const localCell = const_cast<PortalProperty *>(portal)->findContainingCell(position_l);
				if(localCell)
				{				
					NetworkId const & containingCellId = localCell->getOwner().getNetworkId();
					
					if (containingCellId.isValid())
					{
						cell = localCell;
						cellid = containingCellId;
					}
				}
			}
		}
	}

	//-- rotate into object space
	Object const & owner = cell->getOwner();
	Vector const & position_l = owner.rotateTranslate_w2o(position_w);

	//-- x y z cell_id
	char buf[256];
	snprintf(buf, 255, "%5.3f %5.3f %5.3f %s", position_l.x, position_l.y, position_l.z, cellid.getValueString().c_str());

	Unicode::String resultParams = Unicode::narrowToWide(buf);
	ClientCommandQueue::enqueueCommand(s_pathToLocationCommand, NetworkId::cms_invalid, resultParams);

	s_showingPath = true;
}

// ----------------------------------------------------------------------

void SwgCuiQuestHelperNamespace::clearPath(void)
{
	if (s_showingPath)
	{
		Unicode::String resultParams;
		ClientCommandQueue::enqueueCommand(s_pathClearCommand, NetworkId::cms_invalid, resultParams);
	
		s_showingPath = false;
	}
}

// ----------------------------------------------------------------------

bool SwgCuiQuestHelper::OnMessage (UIWidget * const context, UIMessage const & msg)
{
	UNREF(context);
	UNREF(msg);
	if (msg.Type == UIMessage::LeftMouseDoubleClick)
	{
		IGNORE_RETURN(CuiActionManager::performAction (CuiActions::questJournal, Unicode::emptyString));
	}
	else if (msg.Type == UIMessage::RightMouseUp)
	{
		generateLockablePopup(context, msg);
	}

	return true;
}

// ----------------------------------------------------------------------

bool SwgCuiQuestHelper::updateHelperWithPlayerQuestData(PlayerObject const * const /*playerObject*/, NetworkId const & playerQuestId)
{
	Object * o = NetworkIdManager::getObjectById(playerQuestId);
	ClientPlayerQuestObject * pq = dynamic_cast<ClientPlayerQuestObject *>(o);
	if (!pq)
	{
		return false;
	}
	
	// Update task info here.
	//bool const samePQ = playerQuestId == m_lastPlayerQuestId;
	//if (!samePQ)
	//{
		UIString PQTitle = Unicode::narrowToWide(pq->getQuestTitle());
		m_questTitle->SetLocalText(PQTitle);
	//}
	
	if(pq->isCompleted())
		return false;

	bool showedPath = false;
	int totalActiveTasks = 0;
	for(int i = 0; i < pq->getTotalTasks(); ++i)
	{
		if(pq->getTaskStatus(i) == ClientPlayerQuestObject::TS_Completed || pq->getTaskStatus(i) == ClientPlayerQuestObject::TS_Inactive)
			continue;

		Task * questHelperTask = getQuestHelperTask(totalActiveTasks);
		if (questHelperTask)
		{
			++totalActiveTasks;

			questHelperTask->setTaskTitle(StringId::decodeString(Unicode::narrowToWide(pq->getTaskTitle(i))));
			questHelperTask->setQuestTaskData(0, 0);

			std::pair<int, int> const & counterPair = pq->getTaskCounterPair(i);

			if(counterPair.first < 0 || counterPair.second < 0)
				continue;

			questHelperTask->setCounterValues(counterPair.first, counterPair.second);

			std::string const result = FormattedString<256>().sprintf("%d/%d", counterPair.first, counterPair.second);

			questHelperTask->setTaskProgress(Unicode::narrowToWide(result.c_str()), Task::PH_none);

			ClientWaypointObject * waypointLocation = pq->getLocalWaypointObject(i);
			if (waypointLocation && waypointLocation->isWaypointVisible())
			{
				waypointLocation->setWaypointActive(true);
				Location waypointLocation(waypointLocation->getLocation(), waypointLocation->getCell(), Crc::calculate(waypointLocation->getPlanetName().c_str()));
				showedPath = setupWaypointForTask(NULL, questHelperTask, waypointLocation, false, false, NULL);
			}

			if (s_showingPath && !showedPath)
			{
				clearPath();
			}
		}
	}

	removeExtraQuestHelperTasks(totalActiveTasks);

	finishSharedHelperUpdate(false, false, totalActiveTasks, 0, playerQuestId, true);

	return true;
}

// ======================================================================
