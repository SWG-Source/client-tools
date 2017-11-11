//======================================================================
//
// GroundCombatActionManager.h
// copyright (c) 2005 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_GroundCombatActionManager_H
#define INCLUDED_GroundCombatActionManager_H

//======================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"
#include <vector>

//======================================================================

class NetworkId;
class Object;
class Vector;
class CreatureObject;

//======================================================================

class GroundCombatActionManager
{
public:
	typedef void (*attackCallback)(NetworkId const & target);
	typedef void (*defaultActionCallback)(Object & object, bool allowOpenOnly, int typeToUse, bool fromButton);

	typedef Watcher<Object> ObjectWatcher;
	typedef std::vector<ObjectWatcher> ObjectVector;

	enum ActionType
	{
		AT_none,
		AT_primaryAction,
		AT_primaryAttack,
		AT_primaryActionAndAttack,
		AT_secondaryAttack,
		AT_secondaryAttackFromToolbar,
		AT_walkForward,
		AT_toggleRepeatPrimaryAttack,
		AT_cancelRepeatPrimaryAttack
	};

public:
	static void install();

	static void registerPrimaryAttackCallback(attackCallback callback);
	static void registerSecondaryAttackCallback(attackCallback callback);

	static void registerDefaultActionCallback(defaultActionCallback callback);

	static void setCurrentSecondaryAction(std::string const & actionName, Object * associatedObject = NULL);

	static void attemptAction(ActionType const actionType);
	static void clearAction(ActionType const actionType);

	static void update(float const deltaTimeSecs, ObjectVector const & orderedTargets);

	static float getPrimaryThrottleTimer();  // A value from 0 to 1 indicating progress of primary timer
	static bool getRepeatAttackEnabled();

	static void setSecondaryAttackLocation(Vector location, const NetworkId cellId);
	static bool getSecondaryAttackLocation(Vector & location, NetworkId & cellId);
	static bool wantSecondaryAttackLocation();
	static void updateSecondaryReticleRange(const CreatureObject & creature);
	static bool isReticleLocationInRange(float distance);

	static bool getPrimaryAttackLocation(Vector & location, NetworkId & cellId);

private:
	GroundCombatActionManager();
	GroundCombatActionManager(GroundCombatActionManager const &);
	GroundCombatActionManager & operator=(const GroundCombatActionManager &);
};

//======================================================================

#endif
