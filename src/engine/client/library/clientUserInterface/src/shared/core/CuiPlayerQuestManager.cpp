//======================================================================
//
// CuiPlayerQuestManager.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiPlayerQuestManager.h"

#include "clientGame/ClientObject.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"

#include "sharedFoundation/NetworkId.h"
#include "sharedObject/CachedNetworkId.h"
#include "sharedObject/VolumeContainer.h"

#include <vector>
//======================================================================

namespace CuiPlayerQuestManagerNamespace
{
	bool ms_installed = false;
	std::vector<NetworkId> ms_activePlayerQuests;
}

using namespace CuiPlayerQuestManagerNamespace;

void CuiPlayerQuestManager::install()
{
	DEBUG_FATAL(ms_installed, ("ms_installed was true."));
	ms_installed = true;
}

void CuiPlayerQuestManager::remove()
{
	DEBUG_FATAL(!ms_installed, ("ms_installed was false."));
	ms_installed = false;
}

std::vector<NetworkId> const & CuiPlayerQuestManager::getActivePlayerQuests()
{
	ms_activePlayerQuests.clear();

	CreatureObject * const playerCreature = Game::getPlayerCreature();
	if(playerCreature)
	{
		ClientObject * const datapad = playerCreature->getDatapadObject();
		if(datapad)
		{
			VolumeContainer * datapadVC = ContainerInterface::getVolumeContainer(*datapad);
			if(datapadVC)
			{
				for(ContainerIterator iter = datapadVC->begin(); iter != datapadVC->end(); ++iter)
				{
					CachedNetworkId const & item = *iter;
					Object * itemObj = item.getObject();

					VolumeContainer * controlDevice = ContainerInterface::getVolumeContainer(*itemObj);
					if(controlDevice)
					{
						for(ContainerIterator controlIter = controlDevice->begin(); controlIter != controlDevice->end(); ++controlIter)
						{
							CachedNetworkId const & deviceitem = *controlIter;
							Object * deviceItemObj = deviceitem.getObject();
							ClientPlayerQuestObject * questObj = dynamic_cast<ClientPlayerQuestObject *>(deviceItemObj);
							if(questObj)
								ms_activePlayerQuests.push_back(questObj->getNetworkId());
						}
					}
				}
			}
		}
	}

	return ms_activePlayerQuests;
}