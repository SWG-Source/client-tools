// ======================================================================
//
// SwgCuiQuestHelper.h
// Copyright (c) 2005 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiQuestHelper_H
#define INCLUDED_SwgCuiQuestHelper_H

// ======================================================================

#include "clientGame/ClientObject.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"
#include "swgClientUserInterface/SwgCuiLockableMediator.h"
// ----------------------------------------------------------------------

class Location;
class PlayerObject;
class Quest;
class QuestTask;
class UIComposite;
class UIEffector;
class UIPage;
class UIText;

// ----------------------------------------------------------------------

namespace MessageDispatch
{
	class Callback;
}

// ======================================================================

class SwgCuiQuestHelper : 
public SwgCuiLockableMediator 
{
	class Task;
	typedef stdvector<Task *>::fwd QuestHelperTaskVector;

	enum QuestHighlight
	{
		QH_none,
		QH_title,
		QH_quest
	};

	enum QuestHighlightSound
	{
		QHS_none,
		QHS_stepCompleted
	};

public:
	static void install();
	static void remove();

public:
	SwgCuiQuestHelper(UIPage & page);
	~SwgCuiQuestHelper();

	void update(float deltaTimeSecs);
	void setupData(bool checkActiveVisible = true, bool sendToServer = true);
	uint32 getQuestCrc() const;
	void onCurrentQuestChanged(PlayerObject const & player);
	bool OnMessage (UIWidget * context, UIMessage const & msg);

	// Missions are in the datapad so we will monitor additions/removals
	void onAddedToContainer(ClientObject::Messages::ContainerMsg const & msg);
	void onRemovedFromContainer(ClientObject::Messages::ContainerMsg const & msg);

protected:
	virtual void performActivate();
	virtual void performDeactivate();

private:
	Task * getQuestHelperTask(uint index);
	void removeExtraQuestHelperTasks(uint helperTaskCount);
	void clearQuestHelperTasks();
	void updateHelperWithQuestData(PlayerObject * const playerObject, Quest const * const quest);
	long resizeHelperTasks(boolean forceResize);
	void highlightQuest(QuestHighlight questHighlight, QuestHighlightSound questHighlightSound);
	bool updateHelperWithMissionData(PlayerObject const * const playerObject, NetworkId const & missionId);
	void finishSharedHelperUpdate(bool taskTitleChanged, bool sameQuest, uint const helperTaskCount, uint32 const questCrc, NetworkId const & missionId, bool isPlayerQuest = false);
	bool setupWaypointForTask(QuestTask const * questTask, Task * questHelperTask, Location const & waypointLocation, bool sameHelperTask, bool isInteriorWaypoint, Location const * dynamicWaypointLocation);
	bool updateHelperWithPlayerQuestData(PlayerObject const * const PlayerObject, NetworkId const & playerQuestId);

private:
	QuestHelperTaskVector * m_questHelperTasks;
	MessageDispatch::Callback * m_callback;
	UIComposite * m_comp;
	UIComposite * m_sample;
	UIText * m_questTitle;
	UIEffector * m_questHighlight;
	UISize m_minSize;
	uint32 m_lastQuestCrc;
	NetworkId m_lastMissionId;
	long m_lastHelperTasksHeight;
	uint m_lastHelperTaskCount;
	NetworkId m_lastPlayerQuestId;

private: //-- disabled
	SwgCuiQuestHelper();
	SwgCuiQuestHelper(SwgCuiQuestHelper const & rhs);
	SwgCuiQuestHelper & operator= (SwgCuiQuestHelper const & rhs);
};

// ======================================================================

#endif
