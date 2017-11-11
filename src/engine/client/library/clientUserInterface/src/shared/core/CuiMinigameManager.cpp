//======================================================================
//
// CuiMinigameManager.cpp
// copyright (c) 2008 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiMinigameManager.h"

#include "clientGame/Game.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedObject/Controller.h"
#include "sharedObject/Object.h"
#include "sharedUtility/ValueDictionary.h"
#include "sharedUtility/ValueTypeBase.h"
#include "sharedUtility/ValueTypeString.h"
#include "sharedUtility/ValueTypeSignedInt.h"
#include "sharedUtility/ValueTypeObjId.h"
#include "UnicodeUtils.h"

//----------------------------------------------------------------------

std::string const CuiMinigameManager::keyGame("game");
std::string const CuiMinigameManager::keyScore("score");
std::string const CuiMinigameManager::keyTableOid("table");
std::string const CuiMinigameManager::keyPlayerOid("player");

//----------------------------------------------------------------------

namespace CuiMinigameManagerNamespace
{
	typedef std::map<std::string, std::string> GameToMediatorMap;
	static GameToMediatorMap ms_gameToMediatorMap;

	template<typename ValueType, typename ContainerType>
	bool getValueFromDictionary(ValueDictionary const & data, std::string const & key, ValueType & returnValue)
	{
		DictionaryValueMap const & valueMap = data.get();
		DictionaryValueMap::const_iterator itr = valueMap.find(key);

		if(itr == valueMap.end() || itr->second == NULL)
		{
			return false;
		}

		if(itr->second->getType() == ContainerType::ms_type)
		{
			ContainerType * container = static_cast<ContainerType*>(itr->second);
			returnValue = container->getValue();
			return true;
		}
		return false;
	}
}

using namespace CuiMinigameManagerNamespace;

//======================================================================

void CuiMinigameManager::install ()
{

}

//----------------------------------------------------------------------

void CuiMinigameManager::remove ()
{

}

//----------------------------------------------------------------------

void CuiMinigameManager::registerMediatorForGame(std::string const & gameName, std::string const & mediatorName)
{
	std::string lowerName = Unicode::toLower(gameName);

	DEBUG_WARNING(ms_gameToMediatorMap.find(lowerName) != ms_gameToMediatorMap.end(),
		("Overwritting minigame mediator entry for %s. Old mediator: %s New Mediator %s",
			gameName.c_str(), ms_gameToMediatorMap.find(gameName)->second.c_str(), mediatorName.c_str()));

	ms_gameToMediatorMap[lowerName] = mediatorName;
}

//----------------------------------------------------------------------

bool CuiMinigameManager::unregisterMediatorForGame(std::string const & gameName)
{
	std::string lowerName = Unicode::toLower(gameName);

	GameToMediatorMap::iterator i = ms_gameToMediatorMap.find(lowerName);
	if(i != ms_gameToMediatorMap.end())
	{
		ms_gameToMediatorMap.erase(i);
		return true;
	}
	return false;
}

//----------------------------------------------------------------------

bool CuiMinigameManager::FindMediatorForGame(std::string const & gameName, std::string & returnMediatorName)
{
	std::string lowerName = Unicode::toLower(gameName);

	GameToMediatorMap::iterator i = ms_gameToMediatorMap.find(lowerName);
	if(i != ms_gameToMediatorMap.end())
	{
		returnMediatorName = i->second;
		return true;
	}
	return false;
}

//----------------------------------------------------------------------

bool CuiMinigameManager::getStringFromData(ValueDictionary const & data, std::string const & key, std::string & returnString)
{
	return getValueFromDictionary<std::string, ValueTypeString>(data, key, returnString);
}

//----------------------------------------------------------------------

bool CuiMinigameManager::getNetworkIdFromData(ValueDictionary const & data, std::string const & key, NetworkId & returnId)
{
	return getValueFromDictionary<NetworkId, ValueTypeObjId>(data, key, returnId);
}

//----------------------------------------------------------------------

bool CuiMinigameManager::getIntFromData(ValueDictionary const & data, std::string const & key, int & returnInt)
{
	return getValueFromDictionary<int, ValueTypeSignedInt>(data, key, returnInt);
}

//----------------------------------------------------------------------

bool CuiMinigameManager::startMinigame(ValueDictionary const & data)
{
	std::string gameName;
	if(!getStringFromData(data, keyGame, gameName))
	{
		DEBUG_WARNING(true, ("CuiMinigameManager::startMinigame failed to read game name from data"));
		return false;
	}

	std::string mediatorName;
	if(!FindMediatorForGame(gameName, mediatorName))
	{
		DEBUG_WARNING(true, ("CuiMinigameManager::startMinigame failed to find mediator for game %s", gameName.c_str()));
		return false;
	}

	CuiMediator * mediator = CuiMediatorFactory::activateInWorkspace(mediatorName.c_str());
	if(!mediator)
	{
		DEBUG_WARNING(true, ("CuiMinigameManager::startMinigame failed activate mediator %s", mediatorName.c_str()));
		return false;
	}

	CuiMinigameBase * minigame = dynamic_cast<CuiMinigameBase*>(mediator);
	if(minigame)
	{
		return minigame->readData(data);
	}

	return false;
}

//----------------------------------------------------------------------

bool CuiMinigameManager::closeMinigame(ValueDictionary const & data)
{
	std::string gameName;
	if(!getStringFromData(data, keyGame, gameName))
	{
		DEBUG_WARNING(true, ("CuiMinigameManager::closeMinigame failed to read game name from data"));
		return false;
	}

	std::string mediatorName;
	if(!FindMediatorForGame(gameName, mediatorName))
	{
		DEBUG_WARNING(true, ("CuiMinigameManager::closeMinigame failed to find mediator for game %s", gameName.c_str()));
		return false;
	}

	CuiMediatorFactory::deactivateInWorkspace(mediatorName.c_str());
	
	return true;
}

//----------------------------------------------------------------------

bool CuiMinigameManager::submitMinigameResult(ValueDictionary const & data)
{
	typedef ValueDictionary Payload;

	Controller * const controller = Game::getPlayer () ? Game::getPlayer ()->getController () : NULL;

	if (controller)
	{
		MessageQueueGenericValueType<Payload> * msg = new MessageQueueGenericValueType<Payload>(data);
		controller->appendMessage (CM_clientMinigameResult, 0.0f, msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_SERVER);
	}

	return false;
}

//----------------------------------------------------------------------

void CuiMinigameManager::addStandardScoreDataElements(ValueDictionary & data, std::string const & gameName, NetworkId const & table, int score)
{
	data.insert(keyGame, ValueTypeString(gameName));
	data.insert(keyTableOid, ValueTypeObjId(table));
	data.insert(keyScore, ValueTypeSignedInt(score));
	data.insert(keyPlayerOid, ValueTypeObjId(Game::getPlayerNetworkId()));
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------
//----------------------------------------------------------------------

bool CuiMinigameBase::readData(ValueDictionary const & data)
{
	UNREF(data);
	return true;
}

//======================================================================
