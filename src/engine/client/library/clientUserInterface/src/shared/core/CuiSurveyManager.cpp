//======================================================================
//
// CuiSurveyManager.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiSurveyManager.h"

#include "clientGame/ClientCommandQueue.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "clientUserInterface/CuiStringIdsResource.h"
#include "sharedFoundation/NetworkId.h"

#include <map>


//======================================================================

NetworkId                            CuiSurveyManager::m_currentTool;
std::map<NetworkId, Unicode::String> CuiSurveyManager::m_toolToResourceMap;

//======================================================================

void CuiSurveyManager::install ()
{
	m_currentTool = NetworkId::cms_invalid;
	m_toolToResourceMap.clear();
}

//----------------------------------------------------------------------

void CuiSurveyManager::remove ()
{
	m_currentTool = NetworkId::cms_invalid;
	m_toolToResourceMap.clear();
}

//----------------------------------------------------------------------

void CuiSurveyManager::doSurvey()
{
	if(m_currentTool == NetworkId::cms_invalid)
	{
		CuiSystemMessageManager::sendFakeSystemMessage(CuiStringIdsResource::survey_notool.localize());
		return;
	}
	std::map<NetworkId, Unicode::String>::iterator i = m_toolToResourceMap.find(m_currentTool);
	if(i == m_toolToResourceMap.end())
	{
		CuiSystemMessageManager::sendFakeSystemMessage(CuiStringIdsResource::survey_noresource.localize());
		return;
	}

	IGNORE_RETURN(ClientCommandQueue::enqueueCommand("requestSurvey", m_currentTool, i->second));
}

//----------------------------------------------------------------------

void CuiSurveyManager::doGetSample()
{
	if(m_currentTool == NetworkId::cms_invalid)
	{
		CuiSystemMessageManager::sendFakeSystemMessage(CuiStringIdsResource::survey_notool.localize());
		return;
	}
	std::map<NetworkId, Unicode::String>::iterator i = m_toolToResourceMap.find(m_currentTool);
	if(i == m_toolToResourceMap.end())
	{
		CuiSystemMessageManager::sendFakeSystemMessage(CuiStringIdsResource::survey_noresource.localize());
		return;
	}

	IGNORE_RETURN(ClientCommandQueue::enqueueCommand("requestCoreSample", m_currentTool, i->second));
}

//----------------------------------------------------------------------

void CuiSurveyManager::setCurrentTool(const NetworkId& toolId)
{
	m_currentTool = toolId;
}

//----------------------------------------------------------------------

const NetworkId& CuiSurveyManager::getCurrentTool()
{
	return m_currentTool;
}

//----------------------------------------------------------------------

void CuiSurveyManager::setDefaultResourceForTool(const NetworkId& toolId, const Unicode::String& resourceName)
{
	m_toolToResourceMap[toolId] = resourceName;
}


//----------------------------------------------------------------------

Unicode::String CuiSurveyManager::getDefaultResourceForTool(const NetworkId& toolId)
{
	std::map<NetworkId, Unicode::String>::iterator i = m_toolToResourceMap.find(toolId);
	if(i != m_toolToResourceMap.end())
		return i->second;
	else
		return Unicode::emptyString;
}

//======================================================================
