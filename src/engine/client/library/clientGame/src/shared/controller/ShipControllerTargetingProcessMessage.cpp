// ======================================================================
//
// ShipControllerTargetingProcessMessage.cpp
// Copyright 2004 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ShipControllerTargetingProcessMessage.h"

#include "clientGame/ClientShipTargeting.h"
#include "clientGame/CreatureObject.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedObject/NetworkIdManager.h"

void shipControllerTargetingProcessMessage(int const message, Object const * const owner, CreatureObject * const pilot)
{
	if (owner == 0)
	{
		return;
	}

	if (pilot == 0)
	{
		return;
	}

	switch (message)
	{
	case CM_spaceTargetNextOnScreenUnderVariableReticle:
		{
			Object const * const currentTarget = NetworkIdManager::getObjectById(pilot->getLookAtTarget());
			Object const * const target = ClientShipTargeting::getNextTargetOnScreenUnderVariableReticle(owner, currentTarget);
			if (target != 0)
				pilot->setLookAtTarget(target->getNetworkId());
		}
		break;

	case CM_spaceTargetPreviousOnScreenUnderVariableReticle:
		{
			Object const * const currentTarget = NetworkIdManager::getObjectById(pilot->getLookAtTarget());
			Object const * const target = ClientShipTargeting::getPreviousTargetOnScreenUnderVariableReticle(owner, currentTarget);
			if (target != 0)
				pilot->setLookAtTarget(target->getNetworkId());
		}
		break;

	case CM_spaceTargetClosestShip:
		{
			Object const * const target = ClientShipTargeting::closestShip(owner);
			if (target != 0)
				pilot->setLookAtTarget(target->getNetworkId());
		}
		break;

	case CM_spaceTargetClosestCapShip:
		{
			Object const * const target = ClientShipTargeting::closestCapShip(owner);
			if (target != 0)
				pilot->setLookAtTarget(target->getNetworkId());
		}
		break;

	case CM_spaceTargetUnderReticle:
		{
			Object const * const target = ClientShipTargeting::closestUnderReticle(owner);
			if (target != 0)
				pilot->setLookAtTarget(target->getNetworkId());
		}
		break;

	case CM_spaceTargetClosestPlayerShip:
		{
			Object const * const target = ClientShipTargeting::closestPlayer(owner);
			if (target != 0)
				pilot->setLookAtTarget(target->getNetworkId());
		}
		break;

	case CM_spaceTargetClosestNPCShip:
		{
			Object const * const target = ClientShipTargeting::closestNPC(owner);
			if (target != 0)
				pilot->setLookAtTarget(target->getNetworkId());
		}
		break;

	case CM_spaceTargetNext:
		{
			Object const * const currentTarget = NetworkIdManager::getObjectById(pilot->getLookAtTarget());
			Object const * const target = ClientShipTargeting::getNextAnything(owner, currentTarget);
			if (target != 0)
				pilot->setLookAtTarget(target->getNetworkId());
		}
		break;

	case CM_spaceTargetPrevious:
		{
			Object const * const currentTarget = NetworkIdManager::getObjectById(pilot->getLookAtTarget());
			Object const * const target = ClientShipTargeting::getPreviousAnything(owner, currentTarget);
			if (target != 0)
				pilot->setLookAtTarget(target->getNetworkId());
		}
		break;

	case CM_spaceTargetNextFriendly:
		{
			Object const * const currentTarget = NetworkIdManager::getObjectById(pilot->getLookAtTarget());
			Object const * const target = ClientShipTargeting::getNextFriendly(owner, currentTarget);
			if (target != 0)
				pilot->setLookAtTarget(target->getNetworkId());
		}
		break;

	case CM_spaceTargetPreviousFriendly:
		{
			Object const * const currentTarget = NetworkIdManager::getObjectById(pilot->getLookAtTarget());
			Object const * const target = ClientShipTargeting::getPreviousFriendly(owner, currentTarget);
			if (target != 0)
				pilot->setLookAtTarget(target->getNetworkId());
		}
		break;

	case CM_spaceTargetNextEnemy:
		{
			Object const * const currentTarget = NetworkIdManager::getObjectById(pilot->getLookAtTarget());
			Object const * const target = ClientShipTargeting::getNextEnemy(owner, currentTarget);
			if (target != 0)
				pilot->setLookAtTarget(target->getNetworkId());
		}
		break;

	case CM_spaceTargetPreviousEnemy:
		{
			Object const * const currentTarget = NetworkIdManager::getObjectById(pilot->getLookAtTarget());
			Object const * const target = ClientShipTargeting::getPreviousEnemy(owner, currentTarget);
			if (target != 0)
				pilot->setLookAtTarget(target->getNetworkId());
		}
		break;

	case CM_spaceTargetNextFriendlyPlayer:
		{
			Object const * const currentTarget = NetworkIdManager::getObjectById(pilot->getLookAtTarget());
			Object const * const target = ClientShipTargeting::getNextFriendlyPlayer(owner, currentTarget);
			if (target != 0)
				pilot->setLookAtTarget(target->getNetworkId());
		}
		break;

	case CM_spaceTargetPreviousFriendlyPlayer:
		{
			Object const * const currentTarget = NetworkIdManager::getObjectById(pilot->getLookAtTarget());
			Object const * const target = ClientShipTargeting::getPreviousFriendlyPlayer(owner, currentTarget);
			if (target != 0)
				pilot->setLookAtTarget(target->getNetworkId());
		}
		break;

	case CM_spaceTargetNextEnemyPlayer:
		{
			Object const * const currentTarget = NetworkIdManager::getObjectById(pilot->getLookAtTarget());
			Object const * const target = ClientShipTargeting::getNextEnemyPlayer(owner, currentTarget);
			if (target != 0)
				pilot->setLookAtTarget(target->getNetworkId());
		}
		break;

	case CM_spaceTargetPreviousEnemyPlayer:
		{
			Object const * const currentTarget = NetworkIdManager::getObjectById(pilot->getLookAtTarget());
			Object const * const target = ClientShipTargeting::getPreviousEnemyPlayer(owner, currentTarget);
			if (target != 0)
				pilot->setLookAtTarget(target->getNetworkId());
		}
		break;

	case CM_spaceTargetClosestEnemy:
		{
			Object const * const target = ClientShipTargeting::closestEnemy(owner);
			if (target != 0)
				pilot->setLookAtTarget(target->getNetworkId());
		}
		break;

	case CM_spaceTargetClosestEnemyPlayer:
		{
			Object const * const target = ClientShipTargeting::closestEnemyPlayer(owner);
			if (target != 0)
				pilot->setLookAtTarget(target->getNetworkId());
		}
		break;

	case CM_spaceTargetClosestFriend:
		{
			Object const * const target = ClientShipTargeting::closestFriend(owner);
			if (target != 0)
				pilot->setLookAtTarget(target->getNetworkId());
		}
		break;

	case CM_spaceTargetClosestFriendPlayer:
		{
			Object const * const target = ClientShipTargeting::closestFriendPlayer(owner);
			if (target != 0)
				pilot->setLookAtTarget(target->getNetworkId());
		}
		break;

	case CM_spaceTargetGroupLeader:
		{
			Object const * const target = ClientShipTargeting::getGroupLeader(owner);
			if (target != 0)
				pilot->setLookAtTarget(target->getNetworkId());
		}
		break;

	case CM_spaceTargetNextGroupMember:
		{
			Object const * const currentTarget = NetworkIdManager::getObjectById(pilot->getLookAtTarget());
			Object const * const target = ClientShipTargeting::getNextGroupMember(owner, currentTarget);
			if (target != 0)
				pilot->setLookAtTarget(target->getNetworkId());
		}
		break;

	case CM_spaceTargetPreviousGroupMember:
		{
			Object const * const currentTarget = NetworkIdManager::getObjectById(pilot->getLookAtTarget());
			Object const * const target = ClientShipTargeting::getPreviousGroupMember(owner, currentTarget);
			if (target != 0)
				pilot->setLookAtTarget(target->getNetworkId());
		}
		break;

	case CM_spaceTargetLastEnemyThatShotPlayer:
		{
			Object const * const lastAttacker = ClientShipTargeting::getLastAttacker();
			if (lastAttacker)
				pilot->setLookAtTarget(lastAttacker->getNetworkId());
		}
		break;

	default:
		break;
	}
}

// ======================================================================
