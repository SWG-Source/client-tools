// ======================================================================
//
// ClientCommandChecks.h
// Copyright 2006 Sony Online Entertainment LLC (SOE)
// All rights reserved.
//
// ======================================================================

#ifndef INCLUDED_ClientCommandChecks_H
#define INCLUDED_ClientCommandChecks_H

class NetworkId;
class Object;
class CreatureObject;
class Command;

// ======================================================================

class ClientCommandChecks // static class
{
public:

	enum TestCode
	{
		TC_pass                     = 0x0001,
		TC_failAttackingSelf        = 0x0002,
		TC_failInvalidLocomotion    = 0x0004,
		TC_failPlayerLacksAbility   = 0x0008,
		TC_failNotEnoughAction      = 0x0010,
		TC_failInvalidTarget        = 0x0020,
		TC_failInvalidState         = 0x0040,
		TC_failInvalidPosture       = 0x0080,
		TC_failInvalidWeapon        = 0x0100,
		TC_failCooldown             = 0x0200,
		TC_failOutOfRange           = 0x0400,
		TC_failOther                = 0x0800,
		TC_failNoTarget             = 0x1000
	};

	static TestCode canCreatureExecuteCommand(bool primaryCommand, const std::string & commandName, CreatureObject const * creature, NetworkId const & targetId, Object const* actionObject = 0);
	static TestCode canCreatureFireHeavyWeapon(const std::string & commandName, CreatureObject const * creature);

	static bool doesLocomotionInvalidateCommand(Command const * command, CreatureObject const * creature);
	static bool doesPlayerHaveRequiredAbility(Command const * command, CreatureObject const * creature);
	static bool doesPlayerHaveEnoughAction(Command const * command, CreatureObject const * creature);
	static bool doesPlayerHaveEnoughMind(Command const * command, CreatureObject const * creature);
	static bool doesTargetInvalidateCommand(Command const * command, CreatureObject const * creature, NetworkId const & targetId, bool const forDefaultAction);
	static bool doesTargetRangeInvalidateCommand(Command const * command, CreatureObject const * creature, NetworkId const & targetId, Object const * actionObject);
	static bool doesStateInvalidateCommand(Command const * command, CreatureObject const * creature);
	static bool doesWeaponInvalidateCommand(Command const * command, CreatureObject const * creature);

	static bool hasCommandoHeavyWeaponActionCostException(Command const* command, CreatureObject const* creature);
	static bool canDoBeneficialCommandOnTarget(CreatureObject const * const targetCreature);
	static bool canDoBeneficialCommandOnTarget(NetworkId const & targetId);

private: // disabled

	ClientCommandChecks();
	ClientCommandChecks(ClientCommandChecks const &);
	ClientCommandChecks &operator =(ClientCommandChecks const &);
};

// ======================================================================

#endif // INCLUDED_ClientCommandChecks_H
