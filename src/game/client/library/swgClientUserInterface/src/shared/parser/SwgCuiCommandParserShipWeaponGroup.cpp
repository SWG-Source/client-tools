// ======================================================================
//
// SwgCuiCommandParserShipWeaponGroup.cpp
// Copyright Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiCommandParserShipWeaponGroup.h"

#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/ShipObject.h"
#include "clientGame/ShipWeaponGroup.h"
#include "clientGame/ShipWeaponGroupManager.h"
#include "clientGame/ShipWeaponGroupsForShip.h" 
#include "clientUserInterface/CuiSoundManager.h"
#include "clientUserInterface/CuiSounds.h"
#include "sharedGame/ShipComponentWeaponManager.h"
#include "sharedObject/NetworkIdManager.h"

// ======================================================================

namespace SwgCuiCommandParserShipWeaponGroupNamespace
{
	namespace CommandNames
	{
		const char * const s_info               = "info";
		const char * const s_defaults           = "defaults";
		const char * const s_add                = "add";
		const char * const s_remove             = "remove";
		const char * const s_clear              = "clear";
		const char * const s_fire               = "fire";
		const char * const s_fireStop           = "fireStop";
		const char * const s_fireProjectile     = "fireProjectile";
		const char * const s_fireProjectileStop = "fireProjectileStop";
		const char * const s_previousProjectile = "previousProjectile";
		const char * const s_nextProjectile     = "nextProjectile";
		const char * const s_fireMissile        = "fireMissile";
		const char * const s_fireCountermeasures = "fireCountermeasures";
		const char * const s_previousMissile    = "previousMissile";
		const char * const s_nextMissile        = "nextMissile";
		const char * const s_fireTractor        = "fireTractor";
		const char * const s_stopTractor        = "stopTractor";
		const char * const s_toggleTractor      = "toggleTractor";
	}

	const CommandParser::CmdInfo cmds[] =
	{
		{CommandNames::s_info,                  0, "[all]", "Show all WeaponGroup info for ship(s)"},
		{CommandNames::s_defaults,              0, "[all]", "Reset defaults for current or all ships' weapon groups"},
		{CommandNames::s_add,                   2, "<group> <weapon>", "Add a weapon to a group"},
		{CommandNames::s_remove,                2, "<group> <weapon>", "Remove a weapon from a group"},
		{CommandNames::s_clear,                 1, "<group>", "Clear a weapon group"},
		{CommandNames::s_fire,                  1, "<group>", "Fire weapons in the specified projectile group."},
		{CommandNames::s_fireStop,              1, "<group>", "Stop firing weapons in the specified projectile group."},
		{CommandNames::s_fireProjectile,        0, "[next]", "Fire weapons in the current projectile group."},
		{CommandNames::s_fireProjectileStop,    0, "[next]", "Stop firing weapons in the current projectile group."},
		{CommandNames::s_previousProjectile,    0, "", "Select previous projectile group."},
		{CommandNames::s_nextProjectile,        0, "", "Select next projectile group."},
		{CommandNames::s_fireMissile,           0, "[next]", "Fire weapons in the current missile group."},
		{CommandNames::s_fireCountermeasures,   0, "", "Fire countermeasures."},
		{CommandNames::s_previousMissile,       0, "", "Select previous missile group."},
		{CommandNames::s_nextMissile,           0, "", "Select next missile group."},
		{CommandNames::s_fireTractor,           0, "", "Fire the ship's tractor beam."},
		{CommandNames::s_stopTractor,           0, "", "Stop firing the ship's tractor beam."},
		{CommandNames::s_toggleTractor,         0, "", "Toggle firing the ship's tractor beam."},
		{"", 0, "", ""} // this must be last
	};
}

//-----------------------------------------------------------------

SwgCuiCommandParserShipWeaponGroup::SwgCuiCommandParserShipWeaponGroup()
 : CommandParser ("WeaponGroup", 0, "...", "Weapon Group commands", 0)
{
	createDelegateCommands(SwgCuiCommandParserShipWeaponGroupNamespace::cmds);
} 

//-----------------------------------------------------------------

bool SwgCuiCommandParserShipWeaponGroup::performParsing(const NetworkId &, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node)
{
	UNREF(node);
	UNREF(originalCommand);

	CreatureObject * const player = Game::getPlayerCreature();
	if (player == NULL)
		return true;

	//-----------------------------------------------------------------
	
	if (isCommand(argv[0], SwgCuiCommandParserShipWeaponGroupNamespace::CommandNames::s_info))
	{
		bool all = false;
		
		if (argv.size() > 1)
		{
			if (argv[1] == Unicode::narrowToWide("all"))
				all = true;
		}

		ShipWeaponGroupManager::ShipWeaponGroupsForShipVector shipWeaponGroupsForShipVector;

		if (!all)
		{
			ShipObject const * const ship = player->getPilotedShip();
			if (ship == NULL)
			{
				result += Unicode::narrowToWide("(unlocalized) Invalid ship");
				return true;
			}

			shipWeaponGroupsForShipVector.push_back(&ShipWeaponGroupManager::getShipWeaponGroupsForShip(ship->getNetworkId()));
		}
		else
		{
			ShipWeaponGroupManager::getAllShipWeaponGroupsForShip(shipWeaponGroupsForShipVector);

			if (shipWeaponGroupsForShipVector.empty())
			{
				result += Unicode::narrowToWide("(unlocalized) No ship groups");
				return true;
			}
		}

		for (ShipWeaponGroupManager::ShipWeaponGroupsForShipVector::const_iterator it = shipWeaponGroupsForShipVector.begin(); it != shipWeaponGroupsForShipVector.end(); ++it)
		{
			ShipWeaponGroupsForShip const * const shipWeaponGroupsForShip = NON_NULL(*it);
			shipWeaponGroupsForShip->printInfo(result);
		}

		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand(argv[0], SwgCuiCommandParserShipWeaponGroupNamespace::CommandNames::s_defaults))
	{
		if (argv.size() > 1)
		{
			if (argv[1] == Unicode::narrowToWide("all"))
			{
				ShipWeaponGroupManager::resetDefaults();
				return true;
			}
		}
		
		ShipObject const * const ship = player->getPilotedShip();
		if (ship == NULL)
		{
			result += Unicode::narrowToWide("(unlocalized) Invalid ship");
			return true;
		}

		ShipWeaponGroupManager::resetDefaultsForShip(ship->getNetworkId(), true);
		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand(argv[0], SwgCuiCommandParserShipWeaponGroupNamespace::CommandNames::s_add))
	{
		ShipObject const * const ship = player->getPilotedShip();
		if (ship == NULL)
		{
			result += Unicode::narrowToWide("(unlocalized) Invalid ship");
			return true;
		}

		int const groupId = atoi(Unicode::wideToNarrow(argv[1]).c_str());
		int const weaponIndex = atoi(Unicode::wideToNarrow(argv[2]).c_str());

		ShipWeaponGroupsForShip & shipWeaponGroupsForShip = ShipWeaponGroupManager::getShipWeaponGroupsForShip(ship->getNetworkId());

		if (groupId < 0 || groupId >= shipWeaponGroupsForShip.getGroupCount())
		{
			result += Unicode::narrowToWide("(unlocalized) Invalid group id");
			return true;
		}

		if (weaponIndex < 0 || weaponIndex >= ShipChassisSlotType::cms_numWeaponIndices)
		{
			result += Unicode::narrowToWide("(unlocalized) Invalid weapon id.");
			return true;
		}

		if (!ship->isSlotInstalled(weaponIndex + static_cast<int>(ShipChassisSlotType::SCST_weapon_first)))
			result += Unicode::narrowToWide("(unlocalized) Note: weapon slot is not currently installed.");

		ShipWeaponGroup & shipWeaponGroup = shipWeaponGroupsForShip.getGroup(groupId);

		if (shipWeaponGroup.hasWeapon(weaponIndex))
			result += Unicode::narrowToWide("(unlocalized) Weapon slot is already grouped.");
		else
			shipWeaponGroup.addWeapon(weaponIndex, ship->isMissile(weaponIndex), ship->isCountermeasure(weaponIndex));

		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand(argv[0], SwgCuiCommandParserShipWeaponGroupNamespace::CommandNames::s_remove))
	{
		ShipObject const * const ship = player->getPilotedShip();
		if (ship == NULL)
		{
			result += Unicode::narrowToWide("(unlocalized) Invalid ship");
			return true;
		}

		int const groupId = atoi(Unicode::wideToNarrow(argv[1]).c_str());
		int const weaponIndex = atoi(Unicode::wideToNarrow(argv[2]).c_str());

		ShipWeaponGroupsForShip & shipWeaponGroupsForShip = ShipWeaponGroupManager::getShipWeaponGroupsForShip(ship->getNetworkId());

		if (groupId < 0 || groupId >= shipWeaponGroupsForShip.getGroupCount())
		{
			result += Unicode::narrowToWide("(unlocalized) Invalid group id");
			return true;
		}

		if (weaponIndex < 0 || weaponIndex >= ShipChassisSlotType::cms_numWeaponIndices)
		{
			result += Unicode::narrowToWide("(unlocalized) Invalid weapon id.");
			return true;
		}

		if (!ship->isSlotInstalled(weaponIndex + static_cast<int>(ShipChassisSlotType::SCST_weapon_first)))
			result += Unicode::narrowToWide("(unlocalized) Note: weapon slot is not currently installed.");

		ShipWeaponGroup & shipWeaponGroup = shipWeaponGroupsForShip.getGroup(groupId);

		if (!shipWeaponGroup.hasWeapon(weaponIndex))
			result += Unicode::narrowToWide("(unlocalized) Weapon slot is not grouped.");
		else
			shipWeaponGroup.removeWeapon(weaponIndex);

		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand(argv[0], SwgCuiCommandParserShipWeaponGroupNamespace::CommandNames::s_clear))
	{
		ShipObject const * const ship = player->getPilotedShip();
		if (ship == NULL)
		{
			result += Unicode::narrowToWide("(unlocalized) Invalid ship");
			return true;
		}

		int const groupId = atoi(Unicode::wideToNarrow(argv[1]).c_str());

		ShipWeaponGroupsForShip & shipWeaponGroupsForShip = ShipWeaponGroupManager::getShipWeaponGroupsForShip(ship->getNetworkId());

		if (groupId < 0 || groupId >= shipWeaponGroupsForShip.getGroupCount())
		{
			result += Unicode::narrowToWide("(unlocalized) Invalid group id");
			return true;
		}

		ShipWeaponGroup & shipWeaponGroup = shipWeaponGroupsForShip.getGroup(groupId);
		shipWeaponGroup.clear();
		return true;
	}
	
	//----------------------------------------------------------------------
	
	else if (isCommand(argv[0], SwgCuiCommandParserShipWeaponGroupNamespace::CommandNames::s_fire))
	{
		int const groupId = atoi(Unicode::wideToNarrow(argv[1]).c_str());
		IGNORE_RETURN(ShipWeaponGroupManager::fireWeaponGroupForCurrentShip(groupId));
		return true;
	}

	//----------------------------------------------------------------------
	
	else if (isCommand(argv[0], SwgCuiCommandParserShipWeaponGroupNamespace::CommandNames::s_fireStop))
	{
		int const groupId = atoi(Unicode::wideToNarrow(argv[1]).c_str());
		IGNORE_RETURN(ShipWeaponGroupManager::stopFiringWeaponGroupForCurrentShip(groupId));
		return true;
	}

	//----------------------------------------------------------------------
	
	else if (isCommand(argv[0], SwgCuiCommandParserShipWeaponGroupNamespace::CommandNames::s_fireProjectile))
	{
		IGNORE_RETURN(ShipWeaponGroupManager::fireCurrentWeaponGroupForShip(ShipWeaponGroupManager::GT_Projectile));

		if (argv.size() > 1)
		{
			//-- cycle forward
			ShipWeaponGroupManager::findCurrentGroup(ShipWeaponGroupManager::GT_Projectile, false);
		}

		return true;
	}

	//----------------------------------------------------------------------
	
	else if (isCommand(argv[0], SwgCuiCommandParserShipWeaponGroupNamespace::CommandNames::s_fireProjectileStop))
	{
		IGNORE_RETURN(ShipWeaponGroupManager::stopFiringCurrentWeaponGroupForShip(ShipWeaponGroupManager::GT_Projectile));
		return true;
	}

	//----------------------------------------------------------------------
	
	else if (isCommand(argv[0], SwgCuiCommandParserShipWeaponGroupNamespace::CommandNames::s_previousProjectile))
	{
		if (ShipWeaponGroupManager::findCurrentGroup(ShipWeaponGroupManager::GT_Projectile, true))
			CuiSoundManager::restart(CuiSounds::select_popup);

		return true;
	}

	//----------------------------------------------------------------------
	
	else if (isCommand(argv[0], SwgCuiCommandParserShipWeaponGroupNamespace::CommandNames::s_nextProjectile))
	{
		if (ShipWeaponGroupManager::findCurrentGroup(ShipWeaponGroupManager::GT_Projectile, false))
			CuiSoundManager::restart(CuiSounds::select_popup);
		return true;
	}

	//----------------------------------------------------------------------
	
	else if (isCommand(argv[0], SwgCuiCommandParserShipWeaponGroupNamespace::CommandNames::s_fireMissile))
	{
		IGNORE_RETURN(ShipWeaponGroupManager::fireCurrentWeaponGroupForShip(ShipWeaponGroupManager::GT_Missile));

		if (argv.size() > 1)
		{
			//-- cycle forward
			ShipWeaponGroupManager::findCurrentGroup(ShipWeaponGroupManager::GT_Missile, false);
		}

		return true;
	}
	
	//----------------------------------------------------------------------

	else if (isCommand(argv[0], SwgCuiCommandParserShipWeaponGroupNamespace::CommandNames::s_fireCountermeasures))
	{
		IGNORE_RETURN(ShipWeaponGroupManager::fireCountermeasure());
		return true;
	}

	//----------------------------------------------------------------------
	
	else if (isCommand(argv[0], SwgCuiCommandParserShipWeaponGroupNamespace::CommandNames::s_previousMissile))
	{
		if (ShipWeaponGroupManager::findCurrentGroup(ShipWeaponGroupManager::GT_Missile, true))
			CuiSoundManager::restart(CuiSounds::select_popup);
		return true;
	}
	
	//----------------------------------------------------------------------
	
	else if (isCommand(argv[0], SwgCuiCommandParserShipWeaponGroupNamespace::CommandNames::s_nextMissile))
	{
		if (ShipWeaponGroupManager::findCurrentGroup(ShipWeaponGroupManager::GT_Missile, false))
			CuiSoundManager::restart(CuiSounds::select_popup);
		return true;
	}

	//----------------------------------------------------------------------
	
	else if (isCommand(argv[0], SwgCuiCommandParserShipWeaponGroupNamespace::CommandNames::s_fireTractor))
	{
		ShipObject * const ship = player->getPilotedShip();
		if (ship == NULL)
		{
			result += Unicode::narrowToWide("(unlocalized) Invalid ship");
			return true;
		}
		
		for(int slot = ShipChassisSlotType::SCST_weapon_first; slot <= ShipChassisSlotType::SCST_weapon_last; ++slot)
		{
			if(ship->isSlotInstalled(slot))
			{
				int const weaponIndex = ShipChassisSlotType::getWeaponIndex(static_cast<ShipChassisSlotType::Type>(slot));
				if(ship->isBeamWeapon(weaponIndex))
				{
					uint32 const componentCrc = ship->getComponentCrc(slot);
					if(ShipComponentWeaponManager::isTractor(static_cast<uint32>(componentCrc)))
					{
						ship->fireWeapon(weaponIndex);
						break;
					}
				}
			}
		}

		return true;
	}

	//----------------------------------------------------------------------
	
	else if (isCommand(argv[0], SwgCuiCommandParserShipWeaponGroupNamespace::CommandNames::s_stopTractor))
	{
		ShipObject * const ship = player->getPilotedShip();
		if (ship == NULL)
		{
			result += Unicode::narrowToWide("(unlocalized) Invalid ship");
			return true;
		}

		for(int slot = ShipChassisSlotType::SCST_weapon_first; slot <= ShipChassisSlotType::SCST_weapon_last; ++slot)
		{
			if(ship->isSlotInstalled(slot))
			{
				int const weaponIndex = ShipChassisSlotType::getWeaponIndex(static_cast<ShipChassisSlotType::Type>(slot));
				if(ship->isBeamWeapon(weaponIndex))
				{
					uint32 const componentCrc = ship->getComponentCrc(slot);
					if(ShipComponentWeaponManager::isTractor(static_cast<uint32>(componentCrc)))
					{
						ship->stopFiringWeapon(weaponIndex, true);
						break;
					}
				}
			}
		}

		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand(argv[0], SwgCuiCommandParserShipWeaponGroupNamespace::CommandNames::s_toggleTractor))
	{
		ShipObject * const ship = player->getPilotedShip();
		if (ship == NULL)
		{
			result += Unicode::narrowToWide("(unlocalized) Invalid ship");
			return true;
		}

		for(int slot = ShipChassisSlotType::SCST_weapon_first; slot <= ShipChassisSlotType::SCST_weapon_last; ++slot)
		{
			if(ship->isSlotInstalled(slot))
			{
				int const weaponIndex = ShipChassisSlotType::getWeaponIndex(static_cast<ShipChassisSlotType::Type>(slot));
				if(ship->isBeamWeapon(weaponIndex))
				{
					uint32 const componentCrc = ship->getComponentCrc(slot);
					if(ShipComponentWeaponManager::isTractor(static_cast<uint32>(componentCrc)))
					{
						if(ship->isWeaponFiring(weaponIndex))
						{
							ship->stopFiringWeapon(weaponIndex, true);
						}
						else
						{
							ship->fireWeapon(weaponIndex);
						}
						break;
					}
				}
			}
		}

		return true;
	}

	//----------------------------------------------------------------------

	return false;
}

// ======================================================================
