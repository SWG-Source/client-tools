// ClientPlayerQuestObject.cpp
// Copyright 2009, Sony Online Entertainment Inc., all rights reserved. 
// Author: Matt Boudreaux

//-----------------------------------------------------------------------

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientPlayerQuestObject.h"

#include "sharedFoundation/DebugInfoManager.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/Production.h"
#include "sharedGame/SharedPlayerQuestObjectTemplate.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/ContainedByProperty.h"
#include "sharedObject/Container.h"
#include "sharedObject/AlterScheduler.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/SlotIdManager.h"
#include "sharedObject/SlottedContainer.h"
#include "sharedObject/VolumeContainer.h"
#include "clientGame/ClientWaypointObject.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/Game.h"
#include "clientUserInterface/CuiSystemMessageManager.h"

//-----------------------------------------------------------------------

namespace ClientPlayerQuestObjectNamespace
{

	std::string const & ms_debugInfoSectionName = "ClientPlayerQuestObject";
}

using namespace ClientPlayerQuestObjectNamespace;

//-----------------------------------------------------------------------

void ClientPlayerQuestObject::Callbacks::TaskCountChanged::modified(ClientPlayerQuestObject & /*target*/, const int & /*old*/, const int & /*value*/, bool /*isLocal*/) const
{

}

//-----------------------------------------------------------------------

ClientPlayerQuestObject::ClientPlayerQuestObject(const SharedPlayerQuestObjectTemplate * tmpl)
: TangibleObject(tmpl),
m_tasks(),
m_taskCounters(),
m_taskStatus(),
m_waypoints(),
m_rewards(),
m_creatorName(),
m_completed(),
m_recipe(),
m_title(),
m_description(),
m_creator(),
m_totalTasks(),
m_difficulty(),
m_taskTitle1(),
m_taskDescription1(),
m_taskTitle2(),
m_taskDescription2(),
m_taskTitle3(),
m_taskDescription3(),
m_taskTitle4(),
m_taskDescription4(),
m_taskTitle5(),
m_taskDescription5(),
m_taskTitle6(),
m_taskDescription6(),
m_taskTitle7(),
m_taskDescription7(),
m_taskTitle8(),
m_taskDescription8(),
m_taskTitle9(),
m_taskDescription9(),
m_taskTitle10(),
m_taskDescription10(),
m_taskTitle11(),
m_taskDescription11(),
m_taskTitle12(),
m_taskDescription12(),
m_taskTitles(),
m_taskDescriptions(),
m_initWaypoints(false)
{
	addSharedVariable_np(m_tasks);
	addSharedVariable_np(m_taskCounters);
	addSharedVariable_np(m_taskStatus);
	addSharedVariable_np(m_waypoints);
	addSharedVariable_np(m_rewards);
	addSharedVariable_np(m_creatorName);
	addSharedVariable_np(m_completed);
	addSharedVariable_np(m_recipe);
	addSharedVariable(m_title);
	addSharedVariable(m_description);
	addSharedVariable(m_creator);
	addSharedVariable(m_totalTasks);
	addSharedVariable(m_difficulty);
	addSharedVariable(m_taskTitle1);
	addSharedVariable(m_taskDescription1);
	addSharedVariable(m_taskTitle2);
	addSharedVariable(m_taskDescription2);
	addSharedVariable(m_taskTitle3);
	addSharedVariable(m_taskDescription3);
	addSharedVariable(m_taskTitle4);
	addSharedVariable(m_taskDescription4);
	addSharedVariable(m_taskTitle5);
	addSharedVariable(m_taskDescription5);
	addSharedVariable(m_taskTitle6);
	addSharedVariable(m_taskDescription6);
	addSharedVariable(m_taskTitle7);
	addSharedVariable(m_taskDescription7);
	addSharedVariable(m_taskTitle8);
	addSharedVariable(m_taskDescription8);
	addSharedVariable(m_taskTitle9);
	addSharedVariable(m_taskDescription9);
	addSharedVariable(m_taskTitle10);
	addSharedVariable(m_taskDescription10);
	addSharedVariable(m_taskTitle11);
	addSharedVariable(m_taskDescription11);
	addSharedVariable(m_taskTitle12);
	addSharedVariable(m_taskDescription12);

	m_taskTitles.push_back(&m_taskTitle1);
	m_taskTitles.push_back(&m_taskTitle2);
	m_taskTitles.push_back(&m_taskTitle3);
	m_taskTitles.push_back(&m_taskTitle4);
	m_taskTitles.push_back(&m_taskTitle5);
	m_taskTitles.push_back(&m_taskTitle6);
	m_taskTitles.push_back(&m_taskTitle7);
	m_taskTitles.push_back(&m_taskTitle8);
	m_taskTitles.push_back(&m_taskTitle9);
	m_taskTitles.push_back(&m_taskTitle10);
	m_taskTitles.push_back(&m_taskTitle11);
	m_taskTitles.push_back(&m_taskTitle12);

	m_taskDescriptions.push_back(&m_taskDescription1);
	m_taskDescriptions.push_back(&m_taskDescription2);
	m_taskDescriptions.push_back(&m_taskDescription3);
	m_taskDescriptions.push_back(&m_taskDescription4);
	m_taskDescriptions.push_back(&m_taskDescription5);
	m_taskDescriptions.push_back(&m_taskDescription6);
	m_taskDescriptions.push_back(&m_taskDescription7);
	m_taskDescriptions.push_back(&m_taskDescription8);
	m_taskDescriptions.push_back(&m_taskDescription9);
	m_taskDescriptions.push_back(&m_taskDescription10);
	m_taskDescriptions.push_back(&m_taskDescription11);
	m_taskDescriptions.push_back(&m_taskDescription12);

	m_taskStatus.setOnChanged(this, &ClientPlayerQuestObject::OnStatusChanged);
}

//-----------------------------------------------------------------------

ClientPlayerQuestObject::~ClientPlayerQuestObject()
{
	std::vector<ClientWaypointObject const *>::size_type i = 0;
	for(; i < m_clientWaypoints.size(); ++i)
	{
		if(m_clientWaypoints[i])
			delete m_clientWaypoints[i];
	}
	m_clientWaypoints.clear();
}

//-----------------------------------------------------------------------

void ClientPlayerQuestObject::containedByModified(const NetworkId & oldValue, const NetworkId & newValue, bool isLocal)
{
	TangibleObject::containedByModified(oldValue, newValue, isLocal);
}

//-----------------------------------------------------------------------

void ClientPlayerQuestObject::getObjectInfo(std::map<std::string, std::map<std::string, Unicode::String> > & propertyMap) const
{
#if PRODUCTION == 0
	/**
	When adding a variable to this class, please add it here.  Variable that aren't easily displayable are still listed, for tracking purposes.
	*/
	TangibleObject::getObjectInfo(propertyMap);

#else
	UNREF(propertyMap);
#endif
}

//-----------------------------------------------------------------------

void ClientPlayerQuestObject::tasksOnSet(const unsigned int /*index*/, const Unicode::String &/*oldValue*/, const Unicode::String &/*newValue*/)
{

}

//-----------------------------------------------------------------------

void ClientPlayerQuestObject::tasksOnInsert(const unsigned int /*index*/, const Unicode::String &/*value*/)
{

}

//-----------------------------------------------------------------------

void ClientPlayerQuestObject::tasksOnErase(const unsigned int /*index*/, const Unicode::String &/*value*/)
{

}

//-----------------------------------------------------------------------

void ClientPlayerQuestObject::tasksOnChanged()
{

}

//-----------------------------------------------------------------------

std::string const & ClientPlayerQuestObject::getTaskTitle(int index) const
{
	return m_taskTitles[index]->get();
}

//-----------------------------------------------------------------------

std::string const & ClientPlayerQuestObject::getTaskDescription(int index) const
{
	return m_taskDescriptions[index]->get();
}

//-----------------------------------------------------------------------

std::string const ClientPlayerQuestObject::getTaskData(int index) const
{
	return Unicode::wideToNarrow(m_tasks[index]);
}

//-----------------------------------------------------------------------

void ClientPlayerQuestObject::createClientWaypointObjects()
{
	std::vector<ClientWaypointObject const *>::size_type j = 0;
	for(; j < m_clientWaypoints.size(); ++j)
	{
		if(m_clientWaypoints[j])
			delete m_clientWaypoints[j];
	}
	m_clientWaypoints.clear();

	std::vector<std::string>::size_type i = 0;
	for(; i < m_waypoints.get().size(); ++i)
	{
		std::string const & current = m_waypoints.get()[i];

		if(current.empty() || m_taskStatus[i] == ClientPlayerQuestObject::TS_Completed)
		{
			m_clientWaypoints.push_back(NULL);
			continue;
		}

		std::string::size_type start = 0;
		std::string::size_type loc = current.find(' ', start);
		float xyz[3];
		int floatIndex = 0;
		while(loc != std::string::npos)
		{
			std::string token = current.substr(start, (loc - start));
			xyz[floatIndex] = static_cast<float>(atof(token.c_str()));
			++floatIndex;
			start = loc + 1;
			loc = current.find(' ', start);
		}

		std::string sceneId = current.substr(start, (current.size() - start));

		ClientWaypointObject * waypoint = ClientWaypointObject::createClientWaypoint(Unicode::emptyString, sceneId, Vector(xyz[0], xyz[1] , xyz[2]), "purple", true);

		if(waypoint)
			m_clientWaypoints.push_back(waypoint);
	}

	// Somehow we have no waypoints and no strings representing waypoints. Fill the vector with NULL pointers to avoid
	// possibly accessing some bad memory.
	if(m_clientWaypoints.empty())
	{
		std::vector<int>::size_type i = 0;
		for(; i < m_taskStatus.size(); ++i)
		{
			m_clientWaypoints.push_back(NULL);
		}
	}
}

//-----------------------------------------------------------------------

ClientWaypointObject * ClientPlayerQuestObject::getLocalWaypointObject(int index)
{
	if(m_waypoints.empty() || m_taskStatus[index] == ClientPlayerQuestObject::TS_Completed)
		return NULL;

	if(m_clientWaypoints.empty() && !m_initWaypoints)
	{
		createClientWaypointObjects();
		m_initWaypoints = true;
	}

	if(static_cast<unsigned int>(index) > m_clientWaypoints.size() || static_cast<unsigned int>(index) > m_waypoints.size())
		return NULL;

	if(m_clientWaypoints[index] == NULL && m_waypoints.get()[index].empty())
		return NULL;
	else if (m_clientWaypoints[index])
		return m_clientWaypoints[index];
	else
	{
		// Somehow we don't have a waypoint created for this item, but we have a valid string to create a waypoint from?
		createClientWaypointObjects();
		
		if(static_cast<unsigned int>(index) >= m_clientWaypoints.size())
			return NULL;
		else
			return m_clientWaypoints[index];

	}

	return NULL;
}

//-----------------------------------------------------------------------

void ClientPlayerQuestObject::waypointOnChanged()
{
	createClientWaypointObjects();
}

//-----------------------------------------------------------------------

void ClientPlayerQuestObject::OnStatusChanged()
{
	std::vector<int>::size_type i = 0;
	for(; i < m_taskStatus.size(); ++i)
	{
		if(m_taskStatus[i] == ClientPlayerQuestObject::TS_Completed)
		{
			if(!m_initWaypoints)
			{
				createClientWaypointObjects();
				m_initWaypoints = true;
			}

			if(m_clientWaypoints[i] != NULL)
			{
				delete m_clientWaypoints[i];
				m_clientWaypoints[i] = NULL;
			}
		}
	}
}

//-----------------------------------------------------------------------

std::string const & ClientPlayerQuestObject::getRewardData() const
{
	return m_rewards.get();
}

//-----------------------------------------------------------------------

std::string const & ClientPlayerQuestObject::getCreatorName() const
{
	return m_creatorName.get();
}