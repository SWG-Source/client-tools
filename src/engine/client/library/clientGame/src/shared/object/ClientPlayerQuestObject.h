// ClientPlayerQuestObject.h
// Copyright 2009, Sony Online Entertainment Inc., all rights reserved. 
// Author: Matt Boudreaux

#ifndef	_INCLUDED_ClientPlayerQuestObject_H
#define	_INCLUDED_ClientPlayerQuestObject_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaVector.h"
#include "clientGame/TangibleObject.h"

class ClientWaypointObject;
class SharedPlayerQuestObjectTemplate;
class Token;

//-----------------------------------------------------------------------

class ClientPlayerQuestObject : public TangibleObject
{
public:
	enum TaskStatus
	{
		TS_None = 0,
		TS_Current,
		TS_Completed,
		TS_Failed,
		TS_Inactive,

		TS_TotalStatus
	};

	ClientPlayerQuestObject(const SharedPlayerQuestObjectTemplate *);
	~ClientPlayerQuestObject();

	void  containedByModified(const NetworkId & oldValue, const NetworkId & newValue, bool isLocal);

	virtual void getObjectInfo(stdmap<std::string, stdmap<std::string, Unicode::String>::fwd >::fwd & propertyMap) const;

	std::string const & getQuestTitle() const;
	std::string const & getQuestDescription() const;

	std::string const & getTaskTitle(int index) const;
	std::string const & getTaskDescription(int index) const;

	std::string const getTaskData(int index) const;
	int               getTaskDataCount() const;

	int getTotalTasks() const;
	int getQuestDifficulty() const;

	std::pair<int, int> const & getTaskCounterPair(int index) const;

	bool hasWaypoint(int index) const;
	ClientWaypointObject * getLocalWaypointObject(int index);

	int const getTaskStatus(int index) const;

	bool isCompleted() const;
	bool isRecipe() const;

	void OnStatusChanged      ();

	std::string const & getRewardData() const;
	std::string const & getCreatorName() const;

private:
	struct Callbacks
	{
		template <typename U> struct DefaultCallback
		{
			void modified (ClientPlayerQuestObject & target, const U & old, const U & value, bool isLocal) const;
		};

		typedef DefaultCallback<int>									TaskCountChanged;
	};

	ClientPlayerQuestObject & operator = (const ClientPlayerQuestObject & rhs);
	ClientPlayerQuestObject(const ClientPlayerQuestObject & source);

	// Callbacks
	void tasksOnChanged		  ();
	void tasksOnErase         (const unsigned int index, const Unicode::String & value);
	void tasksOnInsert        (const unsigned int index, const Unicode::String & value);
	void tasksOnSet           (const unsigned int index, const Unicode::String & oldValue, const Unicode::String & newValue);

	void waypointOnChanged    ();

	void createClientWaypointObjects();

	
	Archive::AutoDeltaVector<Unicode::String, ClientPlayerQuestObject>								m_tasks;
	Archive::AutoDeltaVector<std::pair<int, int>, ClientPlayerQuestObject>							m_taskCounters;
	Archive::AutoDeltaVector<int, ClientPlayerQuestObject>										    m_taskStatus;
	Archive::AutoDeltaVector<std::string>															m_waypoints;
	Archive::AutoDeltaVariable<std::string>															m_rewards;
	Archive::AutoDeltaVariable<std::string>															m_creatorName;
	Archive::AutoDeltaVariable<bool>																m_completed;
	Archive::AutoDeltaVariable<bool>																m_recipe;
	Archive::AutoDeltaVariable<std::string>															m_title;
	Archive::AutoDeltaVariable<std::string>															m_description;
	Archive::AutoDeltaVariable<NetworkId>															m_creator;
	Archive::AutoDeltaVariable<int>																	m_totalTasks;
	Archive::AutoDeltaVariable<int>																	m_difficulty;

	Archive::AutoDeltaVariable<std::string>															m_taskTitle1;
	Archive::AutoDeltaVariable<std::string>															m_taskDescription1;
	Archive::AutoDeltaVariable<std::string>															m_taskTitle2;
	Archive::AutoDeltaVariable<std::string>															m_taskDescription2;
	Archive::AutoDeltaVariable<std::string>															m_taskTitle3;
	Archive::AutoDeltaVariable<std::string>															m_taskDescription3;
	Archive::AutoDeltaVariable<std::string>															m_taskTitle4;
	Archive::AutoDeltaVariable<std::string>															m_taskDescription4;
	Archive::AutoDeltaVariable<std::string>															m_taskTitle5;
	Archive::AutoDeltaVariable<std::string>															m_taskDescription5;
	Archive::AutoDeltaVariable<std::string>															m_taskTitle6;
	Archive::AutoDeltaVariable<std::string>															m_taskDescription6;
	Archive::AutoDeltaVariable<std::string>															m_taskTitle7;
	Archive::AutoDeltaVariable<std::string>															m_taskDescription7;
	Archive::AutoDeltaVariable<std::string>															m_taskTitle8;
	Archive::AutoDeltaVariable<std::string>															m_taskDescription8;
	Archive::AutoDeltaVariable<std::string>															m_taskTitle9;
	Archive::AutoDeltaVariable<std::string>															m_taskDescription9;
	Archive::AutoDeltaVariable<std::string>															m_taskTitle10;
	Archive::AutoDeltaVariable<std::string>															m_taskDescription10;
	Archive::AutoDeltaVariable<std::string>															m_taskTitle11;
	Archive::AutoDeltaVariable<std::string>															m_taskDescription11;
	Archive::AutoDeltaVariable<std::string>															m_taskTitle12;
	Archive::AutoDeltaVariable<std::string>															m_taskDescription12;

	std::vector<Archive::AutoDeltaVariable<std::string> *>											m_taskTitles;
	std::vector<Archive::AutoDeltaVariable<std::string> *>											m_taskDescriptions;

	std::vector<ClientWaypointObject *>														m_clientWaypoints;

	bool																							m_initWaypoints;

};

//-----------------------------------------------------------------------

inline std::string const & ClientPlayerQuestObject::getQuestTitle() const
{
	return m_title.get();
}

//-----------------------------------------------------------------------

inline std::string const & ClientPlayerQuestObject::getQuestDescription() const
{
	return m_description.get();
}

//-----------------------------------------------------------------------

inline int ClientPlayerQuestObject::getTotalTasks() const
{
	return m_totalTasks.get();
}

//-----------------------------------------------------------------------

inline int ClientPlayerQuestObject::getQuestDifficulty() const
{
	return m_difficulty.get();
}

//-----------------------------------------------------------------------

inline std::pair<int, int> const & ClientPlayerQuestObject::getTaskCounterPair(int index) const
{
	return m_taskCounters.get()[index];
}

//-----------------------------------------------------------------------

inline bool ClientPlayerQuestObject::hasWaypoint(int index) const
{
	return !(m_waypoints.get()[index].empty());
}

//-----------------------------------------------------------------------

inline int const ClientPlayerQuestObject::getTaskStatus(int index) const
{
	return m_taskStatus.get()[index];
}

//-----------------------------------------------------------------------

inline bool ClientPlayerQuestObject::isCompleted() const
{
	return m_completed.get();
}

//-----------------------------------------------------------------------

inline bool ClientPlayerQuestObject::isRecipe() const
{
	return m_recipe.get();
}

//-----------------------------------------------------------------------

inline int ClientPlayerQuestObject::getTaskDataCount() const
{
	return static_cast<int>(m_tasks.size());
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ClientPlayerQuestObject_H
