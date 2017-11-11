// ======================================================================
//
// ClientCommandChecks.cpp
// Copyright 2006 Sony Online Entertainment LLC (SOE)
// All rights reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientCommandChecks.h"

#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/PlayerCreatureController.h"
#include "clientGame/PlayerObject.h"
#include "clientGame/WeaponObject.h"
#include "clientUserInterface/CuiPreferences.h"
#include "sharedFoundation/Crc.h"
#include "sharedGame/CombatDataTable.h"
#include "sharedGame/Command.h"
#include "sharedGame/CommandChecks.h"
#include "sharedGame/CommandTable.h"
#include "sharedGame/ConfigSharedGame.h"
#include "sharedGame/GameObjectTypes.h"
#include "sharedGame/SharedCreatureObjectTemplate.h"
#include "sharedObject/NetworkIdManager.h"
#include "swgSharedUtility/Locomotions.h"
#include "swgSharedUtility/Postures.h"
#include "swgSharedUtility/States.h"

// ======================================================================

namespace ClientCommandChecksNamespace
{
	bool pvpCanAttack(NetworkId const & targetId);
	bool isJedi(NetworkId const & targetId);
	bool isCommando(CreatureObject const * targetCreature);
	bool isWeaponLightsaber(NetworkId const & targetId);
	bool isWeaponHeavyWeapon(NetworkId const & targetId);
	bool isDead(NetworkId const & targetId);
	bool isPlayer(NetworkId const & targetId);
	bool isAttackable(NetworkId const & targetId);

	// this is used for the commando heavy weapon cost action exception.
	// @todo remove this when the combat engine is unified client/server
	char const * const cs_commandoSkillTemplate = "commando_1a";
}

using namespace ClientCommandChecksNamespace;

// ======================================================================

ClientCommandChecks::TestCode ClientCommandChecks::canCreatureFireHeavyWeapon(const std::string & commandName, CreatureObject const * creature)
{
	bool const logClientCommandChecks = ConfigClientGame::getLogClientCommandChecks();
	UNREF(logClientCommandChecks);

	if ((creature->getVisualPosture() == Postures::Incapacitated) ||
		(creature->getVisualPosture() == Postures::Dead))
	{
		DEBUG_REPORT_LOG(logClientCommandChecks, ("HW: Can't execute command [%s].  Self is dead.\n", commandName.c_str()));
		return TC_failOther;
	}

	Command const & command = CommandTable::getCommand(Crc::normalizeAndCalculate(commandName.c_str()));

	if (!command.m_commandName.empty())
	{
		if (ClientCommandChecks::doesStateInvalidateCommand(&command, creature))
		{
			return TC_failInvalidState;
			DEBUG_REPORT_LOG(logClientCommandChecks, ("HW: Can't execute command [%s].  Player state is invalid.\n", command.m_commandName.c_str()));
		}
		else if (!ClientCommandChecks::hasCommandoHeavyWeaponActionCostException(&command, creature) && !ClientCommandChecks::doesPlayerHaveEnoughAction(&command, creature))
		{
			return TC_failNotEnoughAction;
			DEBUG_REPORT_LOG(logClientCommandChecks, ("HW: Can't execute command [%s].  Player doesn't have enough action points.\n", command.m_commandName.c_str()));
		}
	}

	return TC_pass;
}

// ----------------------------------------------------------------------

ClientCommandChecks::TestCode ClientCommandChecks::canCreatureExecuteCommand(bool primaryCommand, const std::string& commandName, CreatureObject const* creature, NetworkId const& targetId, Object const* actionObject)
{
	bool const logClientCommandChecks = ConfigClientGame::getLogClientCommandChecks();
	UNREF(logClientCommandChecks);

	TestCode result = TC_failOther;

	if (!creature->isDead() && !creature->isIncapacitated())
	{
		result = TC_pass;
		
		Command const& command = CommandTable::getCommand(Crc::normalizeAndCalculate(commandName.c_str()));

		if (!command.m_commandName.empty())
		{
			if (ClientCommandChecks::doesLocomotionInvalidateCommand(&command, creature))
			{
				result = TC_failInvalidLocomotion;
				DEBUG_REPORT_LOG(logClientCommandChecks, ("Can't execute command [%s].  Locomotion not valid.\n", command.m_commandName.c_str()));
			}
			else if (!ClientCommandChecks::doesPlayerHaveRequiredAbility(&command, creature))
			{
				result = TC_failPlayerLacksAbility;
				DEBUG_REPORT_LOG(logClientCommandChecks, ("Can't execute command [%s].  Player doesn't have required ability.\n", command.m_commandName.c_str()));
			}
			else if (!ClientCommandChecks::doesPlayerHaveEnoughAction(&command, creature))
			{
				result = TC_failNotEnoughAction;
				DEBUG_REPORT_LOG(logClientCommandChecks, ("Can't execute command [%s].  Player doesn't have enough action points.\n", command.m_commandName.c_str()));
			}
			else if (!ClientCommandChecks::doesPlayerHaveEnoughMind(&command, creature))
			{
				result = TC_failOther;
				DEBUG_REPORT_LOG(logClientCommandChecks, ("Can't execute command [%s].  Player doesn't have enough mind points.\n", command.m_commandName.c_str()));
			}
			else if (ClientCommandChecks::doesTargetInvalidateCommand(&command, creature, targetId, false))
			{
				result = targetId == NetworkId::cms_invalid ? TC_failNoTarget : TC_failInvalidTarget;
				DEBUG_REPORT_LOG(logClientCommandChecks, ("Can't execute command [%s].  Target is invalid (0x%04X).\n", command.m_commandName.c_str(), result));
			}
			else if (ClientCommandChecks::doesTargetRangeInvalidateCommand(&command, creature, targetId, actionObject))
			{
				result = TC_failOutOfRange;
				DEBUG_REPORT_LOG(logClientCommandChecks, ("Can't execute command [%s].  Target range is invalid.\n", command.m_commandName.c_str()));
			}
			else if (ClientCommandChecks::doesStateInvalidateCommand(&command, creature))
			{
				result = TC_failInvalidState;
				DEBUG_REPORT_LOG(logClientCommandChecks, ("Can't execute command [%s].  Player state is invalid.\n", command.m_commandName.c_str()));
			}
			else if (ClientCommandChecks::doesWeaponInvalidateCommand(&command, creature))
			{
				result = TC_failInvalidWeapon;
				DEBUG_REPORT_LOG(logClientCommandChecks, ("Can't execute command [%s].  Player weapon is invalid.\n", command.m_commandName.c_str()));
			}
			else if (!ClientCommandQueue::canEnqueueCombatCommand(primaryCommand, command))
			{
				result = TC_failCooldown;
				DEBUG_REPORT_LOG(logClientCommandChecks, ("Can't execute command [%s].  Cooldown/execute timer not expired.\n", command.m_commandName.c_str()));
			}
		}
	}

	return result;
}

// ----------------------------------------------------------------------

bool ClientCommandChecks::doesLocomotionInvalidateCommand(Command const * command, CreatureObject const * creature)
{
	Locomotions::Enumerator locomotion = creature->getLocomotion();

	CreatureController const * creatureController = safe_cast<CreatureController const *>(creature->getController());
	float currentSpeed = 0.0f; 
	if (creatureController)
		currentSpeed = creatureController->getCurrentSpeed();
	
	return !CommandChecks::isLocomotionValidForCommand(command, locomotion, currentSpeed);
}

// ----------------------------------------------------------------------

bool ClientCommandChecks::doesPlayerHaveRequiredAbility(Command const * command, CreatureObject const * creature)
{
	bool result = false;
	
	if ("unskilled" == command->m_characterAbility || "" == command->m_characterAbility)
		result = true;
	else
		result = creature->hasCommand(command->m_characterAbility);

	return result;
}

// ----------------------------------------------------------------------

bool ClientCommandChecks::doesPlayerHaveEnoughAction(Command const* command, CreatureObject const* creature)
{
	float actionCost = CombatDataTable::getActionCost(command->m_commandHash);

	if (creature && actionCost > 0)
	{
		// What follows is a direct port from the server combat.scriptlib file function:
		// int[] getActionCost(obj_id self, weapon_data weaponData, dictionary actionData)
		float expertiseActionCostMod = 0;

		// If the ability adds damage from the weapon then we use the action cost reducer from using that kind of weapon...
		float percentAddFromWeapon = CombatDataTable::getPercentAddFromWeapon(command->m_commandHash);
		if (creature->getCurrentWeapon() && percentAddFromWeapon > 0.0f)
		{
			WeaponObject::WeaponType type = creature->getCurrentWeapon()->getWeaponType();
			char tempBuf[32];
			sprintf(tempBuf, "expertise_action_weapon_%d", type);
			expertiseActionCostMod += creature->getModValue(tempBuf, false);
		}

		expertiseActionCostMod += creature->getModValue("expertise_action_all", false);

		std::string specialLine = CombatDataTable::getSpecialLine(command->m_commandHash);
		if (specialLine != "")
			expertiseActionCostMod += creature->getModValue("expertise_action_line_" + specialLine, false);

		std::string actionName = command->m_commandName;
		if (actionName != "")
			expertiseActionCostMod += creature->getModValue("expertise_action_single_" + actionName, false);

		actionCost = actionCost - (actionCost * (expertiseActionCostMod/100.0f));
		if (actionCost < 0)
			actionCost = 0;
	}

	if (actionCost <= creature->getAttribute(Attributes::Action))
		return true;

	return false;
}

// ----------------------------------------------------------------------

// Commandos don't check for action cost when firing heavy weapons
// @todo remove this when the combat engine is unified client/server
bool ClientCommandChecks::hasCommandoHeavyWeaponActionCostException(Command const * command, CreatureObject const * creature)
{
	UNREF(command);
	return isCommando(creature);
}

// ----------------------------------------------------------------------

bool ClientCommandChecks::doesPlayerHaveEnoughMind(Command const * command, CreatureObject const * creature)
{
	return CommandChecks::isEnoughMindPointsForCommand(command, creature->getAttribute(Attributes::Mind));
}

// ----------------------------------------------------------------------

bool ClientCommandChecks::doesTargetInvalidateCommand(Command const * command, CreatureObject const * creature, NetworkId const & targetId, bool const forDefaultAction)
{
	UNREF(creature);

	// enforces rules in command_table, and combat_data

	// Any target is valid.
	if(command->m_targetType == Command::CTT_All)
		return false;

	// if not for default action determination and the target type is optional (like heavy weapons) ...
	if (!forDefaultAction && command->m_targetType == Command::CTT_Optional)
	{
		// if auto-aim is turned on and the target is set, then we should validate the target
		if (!CuiPreferences::getAutoAimToggle() || targetId == NetworkId::cms_invalid)
			return false;

		// if the command's valid target type is friend but the target cannot be helped (i.e. command will be applied to self instead), do not validate the target
		if (CombatDataTable::getValidTarget(command->m_commandHash) == CombatDataTable::VTT_friend && !canDoBeneficialCommandOnTarget(targetId))
			return false;
	}

	// Command doesn't want a target and there isn't one... don't check target type beyond this....
	if ((command->m_targetType == Command::CTT_Location || command->m_targetType == Command::CTT_None) && targetId == NetworkId::cms_invalid)
		return false;

	// We're trying to get a location--don't invalidate target, since we don't really care about our target.
	if((command->m_targetType == Command::CTT_Location))
		return false;

	bool isInvalidated = false;
	if ((command->m_targetType == Command::CTT_None && targetId != NetworkId::cms_invalid) ||
	    (command->m_targetType == Command::CTT_Required && targetId == NetworkId::cms_invalid))
	{
		isInvalidated = true;
	}
	else
	{
		//now check combat_data rules

		int const validTargetType = CombatDataTable::getValidTarget(command->m_commandHash);

		if (CombatDataTable::VTT_none != validTargetType && isDead(targetId) && CombatDataTable::VTT_dead != validTargetType)
		{
			//can only do action on target that is dead if combat_data.validTarget == VTT_dead
			//or if we don't actually care about the target at all
			isInvalidated = true;
		}
		else
		{
			switch (CombatDataTable::getValidTarget(command->m_commandHash))
			{
				case CombatDataTable::VTT_mob:
					isInvalidated = !CommandChecks::isMob(targetId) && !CommandChecks::isVehicle(targetId);
					break;

				case CombatDataTable::VTT_creature:
					isInvalidated = !CommandChecks::isMonster(targetId);
					break;

				case CombatDataTable::VTT_npc:
					isInvalidated = !CommandChecks::isNpc(targetId);
					break;

				case CombatDataTable::VTT_droid:
					isInvalidated = !CommandChecks::isDroid(targetId);
					break;

				case CombatDataTable::VTT_pvp:
					isInvalidated = !pvpCanAttack(targetId);
					break;

				case CombatDataTable::VTT_jedi:
					if (isPlayer(targetId))
					{
						isInvalidated = !isJedi(targetId);
					}
					else
					{
						isInvalidated = !isWeaponLightsaber(targetId);
					}
					break;

				case CombatDataTable::VTT_dead:
					isInvalidated = !isDead(targetId);
					break;

				case CombatDataTable::VTT_friend:
					isInvalidated = !(targetId == NetworkId::cms_invalid || canDoBeneficialCommandOnTarget(targetId));
					break;

				case CombatDataTable::VTT_standard:
					isInvalidated = !isAttackable(targetId);
					break;
					
				default:
					//unknown type, assume it *can* be executed
					isInvalidated = false;
			}
		}
	}

	return isInvalidated;
}

// ----------------------------------------------------------------------

bool ClientCommandChecks::doesTargetRangeInvalidateCommand(Command const * command, CreatureObject const * creature, NetworkId const & targetId, Object const * actionObject)
{
	if (!creature)
		return true;

	// range is always valid if target not allowed
	if (command->m_targetType == Command::CTT_None || command->m_targetType == Command::CTT_All || command->m_targetType == Command::CTT_Location)
		return false;

	Object * targetObj = NetworkIdManager::getObjectById(targetId);

	// range is always valid if no target
	if (!targetObj)
		return false;

	// if a target is optional we don't need a range check ...
	if (command->m_targetType == Command::CTT_Optional)
	{
		// if the valid target type is none
		if (CombatDataTable::getValidTarget(command->m_commandHash) == CombatDataTable::VTT_none)
			return false;

		// if the valid target type is friend and the target cannot be helped (self is the target in this case)
		if (CombatDataTable::getValidTarget(command->m_commandHash) == CombatDataTable::VTT_friend && !canDoBeneficialCommandOnTarget(targetId))
			return false;
	}

	bool weaponCheckLast = false;

	WeaponObject const* weapon = dynamic_cast<WeaponObject const*>(actionObject);
	if (!weapon)
		weapon = creature->getCurrentWeapon();
	else
		weaponCheckLast = true;

	float minWeaponRange = weapon ? weapon->getMinRange() : -1.0f;
	float maxWeaponRange = weapon ? weapon->getMaxRange() : -1.0f;

	float minRange, maxRange;
	CommandChecks::getRangeForCommand(command, minWeaponRange, maxWeaponRange, weaponCheckLast, minRange, maxRange);

	float const collisionSphereDistance = creature->getDistanceBetweenCollisionSpheres_w(*targetObj);;

	if (minRange > 0 && collisionSphereDistance < minRange)
		return true;

	if (maxRange > 0 && collisionSphereDistance > maxRange)
		return true;

	return false;
}

// ----------------------------------------------------------------------

bool ClientCommandChecks::doesStateInvalidateCommand(Command const * command, CreatureObject const * creature)
{
	for (States::Enumerator i = 0; i < States::NumberOfStates; ++i)
	{
		// check permission
		if (creature->getState(i) && !command->m_statePermissions[i])
		{
			return true;
		}

		// check required
		if (!creature->getState(i) && command->m_stateRequired[i])
		{
			return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------

bool ClientCommandChecks::doesWeaponInvalidateCommand(Command const * command, CreatureObject const * creature)
{
	if(creature && creature->getPlayerObject())
	{
		// If we have a player and that player has their default attack overridden, then we don't care about our weapon.
		// We just want to send the override command.
		if(!creature->getPlayerObject()->getDefaultAttackOverride().empty())
			return false;
	}
	//a weapon of some sort is required
	if (!creature->getCurrentWeapon())
		return true;

	WeaponObject const * weapon = creature->getCurrentWeapon();

	return !WeaponObject::weaponTypeSatisfies(weapon->getWeaponType(), command->m_weaponTypesValid, command->m_weaponTypesInvalid);
}

// ----------------------------------------------------------------------

bool ClientCommandChecks::canDoBeneficialCommandOnTarget(CreatureObject const * const targetCreature)
{
	if (targetCreature && !targetCreature->isAttackable() && targetCreature->canHelp())
		return true;

	return false;
}

// ----------------------------------------------------------------------

bool ClientCommandChecks::canDoBeneficialCommandOnTarget(NetworkId const & targetId)
{
	Object *targetObj = NetworkIdManager::getObjectById(targetId);

	if (targetObj)
		return ClientCommandChecks::canDoBeneficialCommandOnTarget(CreatureObject::asCreatureObject(targetObj));

	return false;
}

// ======================================================================

bool ClientCommandChecksNamespace::pvpCanAttack(NetworkId const & targetId)
{
	bool result = false;

	Object *targetObj = NetworkIdManager::getObjectById(targetId);
	if (targetObj)
	{
		CreatureObject const * const targetCreature = CreatureObject::asCreatureObject(targetObj);

		if (targetCreature)
			result = targetCreature->isAttackable();
	}
	
	return result;
}

// ----------------------------------------------------------------------

bool ClientCommandChecksNamespace::isJedi(NetworkId const & targetId)
{
	bool result = false;

	Object *targetObj = NetworkIdManager::getObjectById(targetId);
	if (targetObj)
	{
		CreatureObject const * const targetCreature   = CreatureObject::asCreatureObject(targetObj);

		if (targetCreature)
		{
			const PlayerObject * targetPlayer = targetCreature->getPlayerObject();

			if (targetPlayer)
				result = targetPlayer->isJedi();
		}
	}

	return result;
}

// ----------------------------------------------------------------------

bool ClientCommandChecksNamespace::isCommando(CreatureObject const * targetCreature)
{
	bool result = false;
	if (targetCreature)
	{
		PlayerObject const * targetPlayer = targetCreature->getPlayerObject();
		if (targetPlayer)
			result = (strcmp(targetPlayer->getSkillTemplate().c_str(), cs_commandoSkillTemplate) == 0);
	}
	return result;
}

// ----------------------------------------------------------------------

bool ClientCommandChecksNamespace::isWeaponLightsaber(NetworkId const & targetId)
{
	bool result = false;

	Object * targetObj = NetworkIdManager::getObjectById(targetId);
	if (targetObj)
	{
		CreatureObject const * const targetCreature = CreatureObject::asCreatureObject(targetObj);

		if (targetCreature)
		{
			WeaponObject const * targetWeapon = targetCreature->getCurrentWeapon();
			
			if (targetWeapon)
				result = targetWeapon->isLightsaberWeapon();
		}
	}

	return result;
}

// ----------------------------------------------------------------------

bool ClientCommandChecksNamespace::isWeaponHeavyWeapon(NetworkId const & targetId)
{
	bool result = false;

	Object *targetObj = NetworkIdManager::getObjectById(targetId);
	if (targetObj)
	{
		CreatureObject const * const targetCreature = CreatureObject::asCreatureObject(targetObj);

		if (targetCreature)
		{
			WeaponObject const * targetWeapon = targetCreature->getCurrentWeapon();

			if (targetWeapon)
				result = targetWeapon->isHeavyWeapon();
		}
	}

	return result;
}

// ----------------------------------------------------------------------

bool ClientCommandChecksNamespace::isDead(NetworkId const & targetId)
{
	bool result = false;

	Object *targetObj = NetworkIdManager::getObjectById(targetId);
	if (targetObj)
	{
		CreatureObject const * const targetCreature = CreatureObject::asCreatureObject(targetObj);

		if (targetCreature)
			result = (targetCreature->isDead() || targetCreature->isIncapacitated());
	}

	return result;
}

// ----------------------------------------------------------------------

bool ClientCommandChecksNamespace::isPlayer(NetworkId const & targetId)
{
	bool result = false;

	Object *targetObj = NetworkIdManager::getObjectById(targetId);
	if (targetObj)
	{
		CreatureObject const * const targetCreature = CreatureObject::asCreatureObject(targetObj);

		if (targetCreature)
			result = targetCreature->isPlayer();
	}

	return result;
}

// ----------------------------------------------------------------------

bool ClientCommandChecksNamespace::isAttackable(NetworkId const & targetId)
{
	bool result = false;

	Object *targetObj = NetworkIdManager::getObjectById(targetId);
	if (targetObj)
	{
		TangibleObject * const targetTangible = dynamic_cast<TangibleObject *>(targetObj);

		if (targetTangible)
		{
			result = targetTangible->isAttackable();
		}
	}
	return result;
}

// ======================================================================
