// ======================================================================
//
// SwgCuiCommandParserShip.cpp
// Copyright Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiCommandParserShip.h"

#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/ProjectileManager.h"
#include "clientGame/ResourceTypeManager.h"
#include "clientGame/ShipHitEffectsManagerClient.h"
#include "clientGame/ShipObject.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/Production.h"
#include "sharedGame/ShipComponentDescriptor.h"
#include "sharedMath/AxialBox.h"
#include "sharedNetworkMessages/MessageQueueShipHit.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedRandom/Random.h"
#include "swgClientUserInterface/SwgCuiCommandParserShipWeaponGroup.h"

// ======================================================================

namespace SwgCuiCommandParserShipNamespace
{
	namespace CommandNames
	{
		const char * const s_info = "info";
		const char * const ms_destroyComponent = "destroyComponent";
		const char * const ms_destroyShip = "destroyShip";
		const char * const ms_chassisSetHp = "chassisSetHp";
		const char * const ms_componentSetHp = "componentSetHp";
		const char * const ms_componentSetFlags = "componentSetFlags";
		const char * const ms_pseudoInstallComponent = "pseudoInstallComponent";
		const char * const ms_purgeComponent = "purgeComponent";
		const char * const ms_startShipDestruction = "startShipDestruction";
		const char * const ms_projectileMe = "projectileMe";
		const char * const ms_hit = "hit";
		const char * const ms_cargo = "cargo";
	}

	const CommandParser::CmdInfo cmds[] =
	{
		{CommandNames::s_info,                  0,  "",  "Show ship info"},
#if PRODUCTION==0
		{CommandNames::ms_destroyComponent, 2, "<chassis slot> <severity>", "destroy a component"},
		{CommandNames::ms_destroyShip, 1, "<severity>", "destroy a ship visually"},
		{CommandNames::ms_chassisSetHp, 2, "<cur> <max>", "set chassis hp"},
		{CommandNames::ms_componentSetHp, 3, "<slot> <cur> <max>", "set component hp"},
		{CommandNames::ms_componentSetFlags, 2, "<slot> <flags>", "set component flags"},
		{CommandNames::ms_pseudoInstallComponent, 2, "<component> <slot>", "pseudo install"},
		{CommandNames::ms_purgeComponent, 1, "<slot>", "purge component"},
		{CommandNames::ms_startShipDestruction, 1, "<severity>", "start ship destruction"},
		{CommandNames::ms_projectileMe, 0, "<angle>", "projectile me"},
		{CommandNames::ms_hit, 3, "<type> <int> <prevint> [<x> <y> <z>]", "hit me"},
		{CommandNames::ms_cargo, 2, "<cur> <max>", "set cargo"},
#endif
		{"", 0, "", ""} // this must be last
	};
}

using namespace SwgCuiCommandParserShipNamespace;

//-----------------------------------------------------------------

SwgCuiCommandParserShip::SwgCuiCommandParserShip()
 : CommandParser ("Ship", 0, "...", "Ship commands", NULL)
{
	createDelegateCommands(SwgCuiCommandParserShipNamespace::cmds);
	IGNORE_RETURN(addSubCommand(new SwgCuiCommandParserShipWeaponGroup)); //lint !e1524 //owns
} 

//-----------------------------------------------------------------

bool SwgCuiCommandParserShip::performParsing(const NetworkId &, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node)
{
	UNREF(node);
	UNREF(originalCommand);

	CreatureObject * const player = Game::getPlayerCreature();
	if (player == NULL)
		return true;

	//-----------------------------------------------------------------
	
	if (isCommand(argv[0], CommandNames::s_info))
	{
		ShipObject const * targetShip = NULL;
		
		if (argv.size() > 1)
		{
			targetShip = dynamic_cast<ShipObject const *>(NetworkIdManager::getObjectById(NetworkId(Unicode::wideToNarrow(argv[1]))));
		}
		else
		{
			targetShip = dynamic_cast<ShipObject const *>(player->getLookAtTarget().getObject());
			
			if (targetShip == NULL)
				targetShip = player->getPilotedShip();
		}
		
		if (targetShip == NULL)
		{
			result += Unicode::narrowToWide("(unlocalized) Invalid ship");
			return true;
		}
		
		result += Unicode::narrowToWide("Ship name: ");		
		result += targetShip->getLocalizedName();
		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand(argv[0], CommandNames::ms_destroyComponent))
	{		
		ShipObject* const ship = dynamic_cast<ShipObject *>(player->getLookAtTarget().getObject());
		if (!ship)
			return true;
				
		int const chassisSlot = ShipChassisSlotType::getTypeFromName(Unicode::wideToNarrow(argv[1]));
		float const severity = static_cast<float>(atof(Unicode::wideToNarrow(argv[2]).c_str()));
		
		ship->handleComponentDestruction(chassisSlot, severity);
		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand(argv[0], CommandNames::ms_destroyShip))
	{
		ShipObject* const ship = dynamic_cast<ShipObject *>(player->getLookAtTarget().getObject());
		if (!ship)
			return true;
						
		float const severity = static_cast<float>(atof(Unicode::wideToNarrow(argv[1]).c_str()));

		ship->handleFinalShipDestruction(severity);
		
		if (ship != Game::getPlayerContainingShip())
			ship->kill();

		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand(argv[0], CommandNames::ms_startShipDestruction))
	{
		ShipObject* const ship = dynamic_cast<ShipObject *>(player->getLookAtTarget().getObject());
		if (!ship)
			return true;
						
		float const severity = static_cast<float>(atof(Unicode::wideToNarrow(argv[1]).c_str()));

		ship->startShipDestructionSequence(severity);
		ship->setShipKillableByDestructionSequence(true);
		
		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand(argv[0], CommandNames::ms_chassisSetHp))
	{
		ShipObject* const ship = dynamic_cast<ShipObject *>(player->getLookAtTarget().getObject());
		if (!ship)
			return true;

		float const cur = static_cast<float>(atof(Unicode::wideToNarrow(argv[1]).c_str()));
		float const max = static_cast<float>(atof(Unicode::wideToNarrow(argv[2]).c_str()));

		ship->clientSetChassisHitPoints(cur, max);
		return true;
	}
	
	//----------------------------------------------------------------------

	else if (isCommand(argv[0], CommandNames::ms_componentSetHp))
	{
		ShipObject* const ship = dynamic_cast<ShipObject *>(player->getLookAtTarget().getObject());
		if (!ship)
			return true;

		int const chassisSlot = ShipChassisSlotType::getTypeFromName(Unicode::wideToNarrow(argv[1]));
		float const cur = static_cast<float>(atof(Unicode::wideToNarrow(argv[2]).c_str()));
		float const max = static_cast<float>(atof(Unicode::wideToNarrow(argv[3]).c_str()));

		IGNORE_RETURN(ship->clientSetComponentHitPoints(chassisSlot, cur, max));
		return true;
	}
	
	//----------------------------------------------------------------------
	
	else if (isCommand(argv[0], CommandNames::ms_componentSetFlags))
	{
		ShipObject* const ship = dynamic_cast<ShipObject *>(player->getLookAtTarget().getObject());
		if (!ship)
			return true;

		int const chassisSlot = ShipChassisSlotType::getTypeFromName(Unicode::wideToNarrow(argv[1]));
		int const flags = atoi(Unicode::wideToNarrow(argv[2]).c_str());

		IGNORE_RETURN(ship->clientSetComponentFlags(chassisSlot, flags));

		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand(argv[0], CommandNames::ms_pseudoInstallComponent))
	{
		ShipObject * const ship                = dynamic_cast<ShipObject *>(player->getLookAtTarget().getObject());
		std::string const & componentName      = Unicode::wideToNarrow (argv [1]);
		std::string const & slotName           = Unicode::wideToNarrow (argv [2]);
		
		if (ship == NULL)
		{
			result += Unicode::narrowToWide ("no ship");
			return true;
		}
		
		uint32 const componentCrc = Crc::normalizeAndCalculate (componentName.c_str ());

		ShipComponentDescriptor const * const shipComponentDescriptor = ShipComponentDescriptor::findShipComponentDescriptorByCrc (componentCrc);
		if (shipComponentDescriptor == NULL)
		{
			result += Unicode::narrowToWide ("Invalid component name");
			return true;
		}

		ShipChassisSlotType::Type const shipChassisSlotType = ShipChassisSlotType::getTypeFromName (slotName);

		if (shipChassisSlotType == ShipChassisSlotType::SCST_num_types)
		{
			result += Unicode::narrowToWide ("not a valid slot name");
			return true;
		}

		if (!ship->clientPseudoInstallComponent (static_cast<int>(shipChassisSlotType), componentCrc))
			result += Unicode::narrowToWide ("Installation Failed... check warnings");
		else		
			result += Unicode::narrowToWide ("Installation Success");

		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand(argv[0], CommandNames::ms_purgeComponent))
	{
		ShipObject * const ship                = dynamic_cast<ShipObject *>(player->getLookAtTarget().getObject());
		std::string const & slotName           = Unicode::wideToNarrow (argv [1]);
		
		if (ship == NULL)
		{
			result += Unicode::narrowToWide ("no ship");
			return true;
		}
		
		ShipChassisSlotType::Type shipChassisSlotType = ShipChassisSlotType::getTypeFromName (slotName);
		
		if (shipChassisSlotType == ShipChassisSlotType::SCST_num_types)
		{
			result += Unicode::narrowToWide ("not a valid slot name");
			return true;
		}
		
		if (!ship->isSlotInstalled (shipChassisSlotType))
		{
			result += Unicode::narrowToWide ("That slot is not filled");
			return true;
		}
		
		ship->clientPurgeComponent (static_cast<int>(shipChassisSlotType));
		result += Unicode::narrowToWide ("Purge Success");
		
		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand(argv[0], CommandNames::ms_projectileMe))
	{
		ShipObject * const ship = dynamic_cast<ShipObject *>(player->getLookAtTarget().getObject());
		if (NULL == ship)
			return true;

		float angle = 0.0f;

		if (argv.size() > 1)
		{
			angle = static_cast<float>(atof(Unicode::wideToNarrow(argv[1]).c_str()));
		}
		else
		{
			angle = Random::randomReal(0.0f, PI_TIMES_2);
		}
 
		ClientObject * const obj = safe_cast<ClientObject *>(ship->getObjectTemplate()->createObject());

		obj->setTransform_o2w(ship->getTransform_o2w());

		float const projectileTravelDistance = std::max(100.0f, ship->getTangibleExtent().getRadius() * 2.0f);

		//-- travel time is projectileTravelDistance meters @200 m/s
		obj->move_o(Vector::unitZ * (ship->getCurrentSpeed() * (projectileTravelDistance / 200.0f)));
		obj->pitch_o(angle);
		obj->move_o(Vector::unitZ * (-projectileTravelDistance));

		int const weaponIndex = 0;
		int const projectileIndex = 1;

		ProjectileManager::createClientProjectile(obj, weaponIndex, projectileIndex, 200.0f, 20.0f);

		delete obj;

		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand(argv[0], CommandNames::ms_hit))
	{
		ShipObject * const ship = dynamic_cast<ShipObject *>(player->getLookAtTarget().getObject());
		if (NULL == ship)
			return true;

		int const type = atoi(Unicode::wideToNarrow(argv[1]).c_str());
		float const integrity = static_cast<float>(atof(Unicode::wideToNarrow(argv[2]).c_str()));
		float const previousIntegrity = static_cast<float>(atof(Unicode::wideToNarrow(argv[3]).c_str()));

		Vector up_w = Vector::unitY;

		if (argv.size() > 4)
		{
			up_w.x = static_cast<float>(atof(Unicode::wideToNarrow(argv[4]).c_str()));

			if (argv.size() > 5)
			{
				up_w.y = static_cast<float>(atof(Unicode::wideToNarrow(argv[5]).c_str()));

				if (argv.size() > 6)
				{
					up_w.z = static_cast<float>(atof(Unicode::wideToNarrow(argv[6]).c_str()));
				}
			}
		}

		MessageQueueShipHit const msg(up_w, Vector::zero, type, integrity, previousIntegrity);

		ShipHitEffectsManagerClient::handleShipHit(*ship, msg);

		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand(argv[0], CommandNames::ms_cargo))
	{
		ShipObject * const ship = dynamic_cast<ShipObject *>(player->getLookAtTarget().getObject());
		if (NULL == ship)
			return true;

		ShipObject::NetworkIdIntMap const & contents = ship->getCargoHoldContents();

		FormattedString<128> fstr;
		{
			ShipObject::NetworkIdIntMap::const_iterator const end = contents.end();
			for (ShipObject::NetworkIdIntMap::const_iterator it = contents.begin(); it != contents.end(); ++it)
			{
				NetworkId const & resourceId = (*it).first;
				int const amount = (*it).second;

				Unicode::String displayLabel;
				if (!ResourceTypeManager::createTypeDisplayLabel(resourceId, displayLabel))
					displayLabel = Unicode::narrowToWide(".ERROR.");

				result += Unicode::narrowToWide(fstr.sprintf("    %10s (%s) : %3d\n", resourceId.getValueString().c_str(), Unicode::wideToNarrow(displayLabel).c_str(), amount));
			}
		}

		return true;
	}
	
	return false;
}

// ======================================================================
