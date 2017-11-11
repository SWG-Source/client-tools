// ======================================================================
//
// ClientIncubatorManager.cpp
// copyright (c) 2006 Sony Online Entertainment
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientIncubatorManager.h"

#include "clientGame/ContainerInterface.h"
#include "clientGame/Game.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/Object.h"

// ======================================================================

namespace ClientIncubatorManagerNamespace
{
	const int ms_numSlots = 4;
	NetworkId ms_slotNetworkId[4];
	// JU_TODO: move to datatable
	const std::string ms_enzyme1TemplateName = "object/tangible/loot/beast/shared_enzyme_1.iff";
	const std::string ms_enzyme2TemplateName = "object/tangible/loot/beast/shared_enzyme_2.iff";
	const std::string ms_enzyme3TemplateName = "object/tangible/loot/beast/shared_enzyme_3.iff";
	// JU_TODO: end move
	bool ms_installed = false;
}

using namespace ClientIncubatorManagerNamespace;

// ======================================================================

void ClientIncubatorManager::install ()
{
	InstallTimer const installTimer("ClientIncubatorManager::install");

	DEBUG_FATAL(ms_installed, ("Already Installed"));
	resetSlots();

	ms_installed = true;
	ExitChain::add(ClientIncubatorManager::remove, "ClientIncubatorManager::remove", 0, false);
}

//----------------------------------------------------------------------

void ClientIncubatorManager::remove ()
{
	DEBUG_FATAL(!ms_installed, ("Not Installed"));
	ms_installed = false;
}

//----------------------------------------------------------------------

const NetworkId& ClientIncubatorManager::getNetworkId(ClientIncubatorManager::Slot slot)
{
	if(slot != ClientIncubatorManager::slot_invalid && slot < ClientIncubatorManager::slot_size)
	{
		return ms_slotNetworkId[slot];
	}

	return NetworkId::cms_invalid;
}

//----------------------------------------------------------------------

void ClientIncubatorManager::setNetworkId(ClientIncubatorManager::Slot slot, const NetworkId& networkId)
{
	if(slot != ClientIncubatorManager::slot_invalid && slot < ClientIncubatorManager::slot_size)
	{
		ms_slotNetworkId[slot] = networkId;
	}
}

//----------------------------------------------------------------------

ClientIncubatorManager::Slot ClientIncubatorManager::getSlotIndexForNetworkId(const NetworkId& networkId)
{
	if(networkId != NetworkId::cms_invalid)
	{
		int i;
		for(i = 0; i < ms_numSlots; ++i)
		{
			if(networkId == ms_slotNetworkId[i])
			{
				return static_cast<ClientIncubatorManager::Slot>(i);
			}
		}
	}
	return ClientIncubatorManager::slot_invalid;
}

//----------------------------------------------------------------------

bool ClientIncubatorManager::canEnzymeGoInSlot(const Object* obj, ClientIncubatorManager::Slot slot)
{
	// JU_TODO: move logic to datatable
	if(obj)
	{
		// make sure object is on the character
		if (!ContainerInterface::isNestedWithin(*obj, Game::getPlayerNetworkId()))
			return false;

		const std::string templateName(obj->getObjectTemplateName());
		switch(slot)
		{
		case ClientIncubatorManager::slot_slot1:
			return (templateName == ms_enzyme1TemplateName);
			break;
		case ClientIncubatorManager::slot_slot2:
			return (templateName == ms_enzyme2TemplateName);
			break;
		case ClientIncubatorManager::slot_slot3:
			return (templateName == ms_enzyme1TemplateName);
			break;
		case ClientIncubatorManager::slot_slot4:
			return (templateName == ms_enzyme3TemplateName);
			break;
		};
	}
	return false;
	// JU_TODO: end move
}

//----------------------------------------------------------------------

void ClientIncubatorManager::resetSlots()
{
	int i;
	for(i = 0; i < ms_numSlots; ++i)
	{
		ms_slotNetworkId[i] = NetworkId::cms_invalid;
	}
}


// =====================================================================
