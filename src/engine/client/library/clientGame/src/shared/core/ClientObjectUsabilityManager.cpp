//======================================================================
//
// ClientObjectUsabilityManager.cpp
// copyright (c) Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientObjectUsabilityManager.h"

//======================================================================

#include "clientGame/ObjectAttributeManager.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/TangibleObject.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedGame/GameObjectTypes.h"
#include "sharedGame/ObjectUsabilityManager.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedUtility/DataTable.h"

//----------------------------------------------------------------------

namespace ClientObjectUsabilityManagerNamespace
{
	bool s_installed = false;
	
	typedef std::map<NetworkId, bool> WearableInfo;
	WearableInfo s_isWearableObjectMap;
	
	typedef std::map<NetworkId, bool> WearableInfo;
	WearableInfo s_playerCanWearMap;

	const char * const cms_combatLevelAttributeName = "healing_combat_level_required";

	std::set<uint32> s_doNotShowAsWearableGOTTypeSet;
}

using namespace ClientObjectUsabilityManagerNamespace;

//----------------------------------------------------------------------

void ClientObjectUsabilityManager::install()
{
	InstallTimer const installTimer("ClientObjectUsabilityManager::install");
	
	DEBUG_FATAL(s_installed, ("ClientObjectUsabilityManager::install: already installed"));
	s_installed = true;
	
	//do not show as wearables table
	{
		char const * const doNotShowAsWearableTableFileName = "datatables/appearance/do_not_show_as_wearable.iff";
		Iff iffDoNotShowAsWearable;
		if (iffDoNotShowAsWearable.open(doNotShowAsWearableTableFileName, true))
		{
			DataTable doNotShowAsWearableDataTable;
			doNotShowAsWearableDataTable.load(iffDoNotShowAsWearable);

			int const numberOfRows = doNotShowAsWearableDataTable.getNumRows();

			for (int row = 0; row < numberOfRows; ++row)
			{
				std::string const & gotTypeName = doNotShowAsWearableDataTable.getStringValue(0, row);

				// Turn it into a GOT type
				int gotType;
				if(GameObjectTypes::getTypeByName(gotTypeName, gotType))
				{
					s_doNotShowAsWearableGOTTypeSet.insert(gotType);
				}
				else
					WARNING(true, ("ClientObjectUsabilityManager encountered unknown GOT type [%s]", gotTypeName.c_str()));
			}
		}
		else
			WARNING(true, ("ClientObjectUsabilityManager table missing [%s]", doNotShowAsWearableTableFileName));
	}

	ExitChain::add(ClientObjectUsabilityManager::remove, "ClientObjectUsabilityManager::install");
}

//----------------------------------------------------------------------

void ClientObjectUsabilityManager::remove()
{
	s_installed = false;
	s_isWearableObjectMap.clear();
	s_playerCanWearMap.clear();
}

//----------------------------------------------------------------------

void ClientObjectUsabilityManager::reset()
{
	s_playerCanWearMap.clear();
}

//----------------------------------------------------------------------

bool ClientObjectUsabilityManager::canWear(CreatureObject const * const user, TangibleObject const * const target)
{
	DEBUG_FATAL(!s_installed, ("ClientObjectUsabilityManager::install: not installed"));

	bool use = true;
	
	if (user && target) 
	{
		NetworkId const & targetId = target->getNetworkId();
		WearableInfo::const_iterator const itWear = s_playerCanWearMap.find(targetId);

		if (itWear == s_playerCanWearMap.end()) 
		{
			ObjectTemplate const * const objectTemplate = target->getObjectTemplate();
			if (objectTemplate)
			{
				SharedObjectTemplate const * const sharedObjectTemplate = objectTemplate->asSharedObjectTemplate();
				if (sharedObjectTemplate)
				{
					std::string sharedObjectTemplateName(sharedObjectTemplate->getName());
					
					IGNORE_RETURN(sharedObjectTemplateName.erase(0, sharedObjectTemplateName.find_last_of('/') + 1));
					IGNORE_RETURN(sharedObjectTemplateName.erase(sharedObjectTemplateName.find_last_of('.'), static_cast<unsigned>(std::string::npos)));
					
					char const * const sharedName = sharedObjectTemplateName.c_str();
					TemporaryCrcString const crcName(sharedName, true);
					uint32 const crc = crcName.getCrc();
					
					if (ObjectUsabilityManager::isWearable(crc)) 
					{
						use = ObjectUsabilityManager::canWear(crc, user->getSpecies(), user->getGender());
					}

					// update the map.
					s_playerCanWearMap[ targetId ] = use;
				}
			}
		}
		else
		{
			use = itWear->second;
		}
	}
	
	return use;
}

//----------------------------------------------------------------------

bool ClientObjectUsabilityManager::isWearable(TangibleObject const * const target)
{
	bool wearable = false;

	if (target) 
	{
		NetworkId const & targetId = target->getNetworkId();

		WearableInfo::const_iterator const itWear = s_isWearableObjectMap.find(targetId);
		if (itWear == s_isWearableObjectMap.end()) 
		{
			int gotType = target->getGameObjectType();
			if(s_doNotShowAsWearableGOTTypeSet.find(gotType) != s_doNotShowAsWearableGOTTypeSet.end())
			{
				// I specifically am not caching the value here because the operation required to search
				// through the set is not expensive (unlike below where expensive string and crc ops are required).
				// I don't want the size of this cache to be that big. -- ARH
				return false;
			}

			ObjectTemplate const * const objectTemplate = target->getObjectTemplate();
			if (objectTemplate)
			{
				SharedObjectTemplate const * const sharedObjectTemplate = objectTemplate->asSharedObjectTemplate();
				if (sharedObjectTemplate)
				{
					std::string sharedObjectTemplateName(sharedObjectTemplate->getName());
					
					IGNORE_RETURN(sharedObjectTemplateName.erase(0, sharedObjectTemplateName.find_last_of('/') + 1));
					IGNORE_RETURN(sharedObjectTemplateName.erase(sharedObjectTemplateName.find_last_of('.'), static_cast<unsigned>(std::string::npos)));
					
					char const * const sharedName = sharedObjectTemplateName.c_str();
					TemporaryCrcString const crcName(sharedName, true);
					uint32 const crc = crcName.getCrc();
					
					wearable = ObjectUsabilityManager::isWearable(crc);
					
					// update the map.
					s_isWearableObjectMap[ targetId ] = wearable;
				}
			}
		}
		else
		{
			wearable = itWear->second;
		}
	}

	return wearable;
}


//----------------------------------------------------------------------

bool ClientObjectUsabilityManager::isUsable(TangibleObject const * const target)
{
	if(!target)
		return false;

	ObjectAttributeManager::AttributeVector av;
	ObjectAttributeManager::getAttributes(target->getNetworkId(), av, false, false);

	ObjectAttributeManager::AttributePair const * const combatLevelAttributePair = ObjectAttributeManager::findAttribute(av, cms_combatLevelAttributeName);
	return (combatLevelAttributePair != NULL);
}

//----------------------------------------------------------------------

bool ClientObjectUsabilityManager::canUse(CreatureObject const * const user, TangibleObject const * const target)
{
	if(!user || !target)
		return true;
	ObjectAttributeManager::AttributeVector av;
	ObjectAttributeManager::getAttributes(target->getNetworkId(), av, false, false);

	ObjectAttributeManager::AttributePair const * const combatLevelAttributePair = ObjectAttributeManager::findAttribute(av, cms_combatLevelAttributeName);
	if(combatLevelAttributePair)
	{
		int16 const combatLevel = user->getLevel();
		std::string const & requiredLevelStr = Unicode::wideToNarrow(combatLevelAttributePair->second);
		int const requiredLevel = atoi(requiredLevelStr.c_str());
		if(requiredLevel > combatLevel)
			return false;
	}

	return true;
}

//----------------------------------------------------------------------
