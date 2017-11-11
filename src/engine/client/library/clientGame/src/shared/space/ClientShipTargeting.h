//======================================================================
//
// ClientShipTargeting.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ClientShipTargeting_H
#define INCLUDED_ClientShipTargeting_H

#include "sharedGame/ShipTargeting.h"

//======================================================================

class Object;

//----------------------------------------------------------------------

class ClientShipTargeting : public ShipTargeting
{
public:
	static void install();
	static void activateVariableReticle(bool const active);

	static Object * closestShip(Object const * targetingObject);
	static Object * closestCapShip(Object const * targetingObject);
	static Object * closestUnderReticle(Object const * targetingObject);
	static Object * closestPlayer(Object const * targetingObject);
	static Object * closestNPC(Object const * targetingObject);
	static Object * closestEnemy(Object const * targetingObject);
	static Object * closestEnemyPlayer(Object const * targetingObject);
	static Object * closestFriend(Object const * targetingObject);
	static Object * closestFriendPlayer(Object const * targetingObject);

	static Object * getNextAnything(Object const * targetingObject, Object const * currentTarget);
	static Object * getPreviousAnything(Object const * targetingObject, Object const * currentTarget);

	static Object * getNextFriendly(Object const * targetingObject, Object const * currentTarget);
	static Object * getPreviousFriendly(Object const * targetingObject, Object const * currentTarget);

	static Object * getNextEnemy(Object const * targetingObject, Object const * currentTarget);
	static Object * getPreviousEnemy(Object const * targetingObject, Object const * currentTarget);

	static Object * getNextFriendlyPlayer(Object const * targetingObject, Object const * currentTarget);
	static Object * getPreviousFriendlyPlayer(Object const * targetingObject, Object const * currentTarget);

	static Object * getNextEnemyPlayer(Object const * targetingObject, Object const * currentTarget);
	static Object * getPreviousEnemyPlayer(Object const * targetingObject, Object const * currentTarget);

	static int getOnScreenVariableReticleRadius();
	static Object * getNextTargetOnScreenUnderVariableReticle(Object const * targetingObject, Object const * currentTarget);
	static Object * getPreviousTargetOnScreenUnderVariableReticle(Object const * targetingObject, Object const * currentTarget);

	static Object * getGroupLeader(Object const * targetingObject);
	static Object * getNextGroupMember(Object const * targetingObject, Object const * currentTarget);
	static Object * getPreviousGroupMember(Object const * targetingObject, Object const * currentTarget);

	static void onPlayerShipHit(Object const *attacker);
	static Object const *getLastAttacker();

private:

	ClientShipTargeting();
	ClientShipTargeting(ClientShipTargeting const & copy);
	ClientShipTargeting & operator=(ClientShipTargeting const & copy);

};

//======================================================================

#endif
