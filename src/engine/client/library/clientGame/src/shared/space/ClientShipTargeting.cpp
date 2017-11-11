//=======================================================================
//
// ShipTargeting.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//=======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientShipTargeting.h"

#include "clientGame/ClientSharedObjectTemplateInterface.h"
#include "clientGame/ClientObject.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GroupObject.h"
#include "clientGame/ShipObject.h"
#include "clientGame/ShipStation.h"
#include "clientGame/TangibleObject.h"
#include "clientGraphics/Camera.h"
#include "clientUserInterface/CuiPreferences.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedUtility/CallbackReceiver.h"
#include "sharedUtility/Callback.h"

#include <vector>

//=======================================================================

namespace ClientShipTargetingNamespace
{
	class OnScreenVariableReticlePercentageOfScreen : public CallbackReceiver
	{
	public:
		OnScreenVariableReticlePercentageOfScreen();
		virtual ~OnScreenVariableReticlePercentageOfScreen();
		float getValue() const;
		virtual void performCallback();

	private:
		float m_value;
	};

	static const ClientSharedObjectTemplateInterface cs_sharedObjectTemplateInterface;
	static OnScreenVariableReticlePercentageOfScreen * s_onScreenVariableReticlePercentageOfScreen = 0;
	static bool m_variableReticleActive = false;

	ConstWatcher<Object> s_lastAttacker;

	void install();
	void remove();
	void activateVariableReticle(bool const active);

	bool alwaysTrueRelationship(Object const * actor, Object const * target);
	bool actorAndTargetAreFriends(Object const * actor, Object const * target);
	bool actorAndTargetAreEnemies(Object const * actor, Object const * target);
	bool actorAndTargetAreEnemiesAndTargetIsPlayer(Object const * actor, Object const * target);
	bool actorAndTargetAreFriendsAndTargetIsPlayer(Object const * actor, Object const * target);

	GroupObject const * getPlayerGroupObject();
	void gatherUniqueShipsFromGroup(GroupObject const * group, std::vector<NetworkId> & ships, NetworkId const & ToIgnore);

	float getOnScreenTargetFov();
}

//=======================================================================

ClientShipTargetingNamespace::OnScreenVariableReticlePercentageOfScreen::OnScreenVariableReticlePercentageOfScreen()
: CallbackReceiver()
, m_value(CuiPreferences::getVariableTargetingReticlePercentage())
{
}

// ----------------------------------------------------------------------

ClientShipTargetingNamespace::OnScreenVariableReticlePercentageOfScreen::~OnScreenVariableReticlePercentageOfScreen()
{
}

// ----------------------------------------------------------------------

float ClientShipTargetingNamespace::OnScreenVariableReticlePercentageOfScreen::getValue() const
{
	return m_value;
}

// ----------------------------------------------------------------------

void ClientShipTargetingNamespace::OnScreenVariableReticlePercentageOfScreen::performCallback()
{
	m_value = CuiPreferences::getVariableTargetingReticlePercentage();
}

//=======================================================================

void ClientShipTargetingNamespace::install()
{
	InstallTimer const installTimer("ClientShipTargetingNamespace::install");

	s_onScreenVariableReticlePercentageOfScreen = new ClientShipTargetingNamespace::OnScreenVariableReticlePercentageOfScreen;
}

// ----------------------------------------------------------------------

void ClientShipTargetingNamespace::remove()
{
	delete s_onScreenVariableReticlePercentageOfScreen;
	s_onScreenVariableReticlePercentageOfScreen = 0;
}

// ----------------------------------------------------------------------

void ClientShipTargetingNamespace::activateVariableReticle(bool const active)
{
	if (m_variableReticleActive == active)
	{
		return;
	}

	if (active)
	{
		// add callback if not already active
		if (!m_variableReticleActive)
		{
			CuiPreferences::getVariableTargetingReticlePercentageCallback().attachReceiver(*s_onScreenVariableReticlePercentageOfScreen);
		}
	}
	else
	{
		// remove callback
		if (m_variableReticleActive)
		{
			CuiPreferences::getVariableTargetingReticlePercentageCallback().detachReceiver(*s_onScreenVariableReticlePercentageOfScreen);
		}
	}

	m_variableReticleActive = active;
}

// ----------------------------------------------------------------------

bool ClientShipTargetingNamespace::alwaysTrueRelationship(Object const *, Object const *)
{
	return true;
}

// ----------------------------------------------------------------------

bool ClientShipTargetingNamespace::actorAndTargetAreFriends(Object const * const actor, Object const * const target)
{
	return !ClientShipTargetingNamespace::actorAndTargetAreEnemies(actor, target);
}

// ----------------------------------------------------------------------

bool ClientShipTargetingNamespace::actorAndTargetAreEnemies(Object const * const /*actor*/, Object const * const target)
{
	if (target != 0)
	{
		ClientObject const * const clientObject = target->asClientObject();
		if (clientObject != 0)
		{
			TangibleObject const * const tangibleObject = clientObject->asTangibleObject();
			if (tangibleObject != 0)
			{
				return tangibleObject->isEnemy();
			}
		}
	}
	return false;
}

// ----------------------------------------------------------------------

bool ClientShipTargetingNamespace::actorAndTargetAreEnemiesAndTargetIsPlayer(Object const * const actor, Object const * const target)
{
	if (ClientShipTargetingNamespace::actorAndTargetAreEnemies(actor, target))
	{
		return ShipTargeting::isPlayerShip(cs_sharedObjectTemplateInterface, target);
	}

	return false;
}

// ----------------------------------------------------------------------

bool ClientShipTargetingNamespace::actorAndTargetAreFriendsAndTargetIsPlayer(Object const * const actor, Object const * const target)
{
	if (ClientShipTargetingNamespace::actorAndTargetAreFriends(actor, target))
	{
		return ShipTargeting::isPlayerShip(cs_sharedObjectTemplateInterface, target);
	}

	return false;
}

// ----------------------------------------------------------------------

GroupObject const * ClientShipTargetingNamespace::getPlayerGroupObject()
{
	CreatureObject const * const player = Game::getPlayerCreature();

	if (player != 0)
	{
		CachedNetworkId const & groupId = player->getGroup();
		return safe_cast<const GroupObject *>(groupId.getObject());
	}

	return 0;
}

// ----------------------------------------------------------------------

float ClientShipTargetingNamespace::getOnScreenTargetFov()
{
	Camera const * const camera = Game::getCamera();

	if (camera != 0)
	{
		float const reticlePercentageOfScreen = s_onScreenVariableReticlePercentageOfScreen->getValue();
		return camera->getVerticalFieldOfView() * reticlePercentageOfScreen;
	}

	// should never reach here
	return 0.5f;
}

// ----------------------------------------------------------------------

void ClientShipTargetingNamespace::gatherUniqueShipsFromGroup(GroupObject const * const group, std::vector<NetworkId> & ships, NetworkId const & ToIgnore)
{
	typedef std::set<NetworkId> UniqueSet;
	UniqueSet uniqueSet;

	GroupObject::GroupMemberVector const & members = group->getGroupMembers();
	GroupObject::GroupMemberVector::const_iterator ii = members.begin();
	GroupObject::GroupMemberVector::const_iterator iiEnd = members.end();

	for (; ii != iiEnd; ++ii)
	{
		if (ii->first != ToIgnore)
		{
			NetworkId const & shipId = group->getShipFromMember(ii->first);
			UniqueSet::const_iterator ii = uniqueSet.find(shipId);
			if (ii != uniqueSet.end())
			{
				ships.push_back(shipId);
				uniqueSet.insert(shipId);
			}
		}
	}
}

//=======================================================================

using namespace ClientShipTargetingNamespace;

//=======================================================================

void ClientShipTargeting::install()
{
	InstallTimer const installTimer("ClientShipTargeting::install");

	ClientShipTargetingNamespace::install();
	ExitChain::add(remove, "ClientShipTargetingNamespace::remove");
}

// ----------------------------------------------------------------------

void ClientShipTargeting::activateVariableReticle(bool const active)
{
	ClientShipTargetingNamespace::activateVariableReticle(active);
}

// ----------------------------------------------------------------------

Object * ClientShipTargeting::closestShip(Object const * const targetingObject)
{
	return ShipTargeting::closestShipInWorld(cs_sharedObjectTemplateInterface, targetingObject);
}

// ----------------------------------------------------------------------

Object * ClientShipTargeting::closestCapShip(Object const * const targetingObject)
{
	return ShipTargeting::closestCapShipInWorld(cs_sharedObjectTemplateInterface, targetingObject);
}

// ----------------------------------------------------------------------

Object * ClientShipTargeting::closestUnderReticle(Object const * const targetingObject)
{
	return ShipTargeting::closestUnderReticleInWorld(cs_sharedObjectTemplateInterface, targetingObject);
}

// ----------------------------------------------------------------------

Object * ClientShipTargeting::closestPlayer(Object const * const targetingObject)
{
	return ShipTargeting::closestPlayerInWorld(cs_sharedObjectTemplateInterface, targetingObject);
}

// ----------------------------------------------------------------------

Object * ClientShipTargeting::closestNPC(Object const * const targetingObject)
{
	return ShipTargeting::closestNPCInWorld(cs_sharedObjectTemplateInterface, targetingObject);
}

// ----------------------------------------------------------------------

Object * ClientShipTargeting::closestEnemy(Object const * const targetingObject)
{
	return closestShipInWorldWithRelationShip(cs_sharedObjectTemplateInterface, targetingObject, actorAndTargetAreEnemies);
}

// ----------------------------------------------------------------------

Object * ClientShipTargeting::closestEnemyPlayer(Object const * const targetingObject)
{
	return closestShipInWorldWithRelationShip(cs_sharedObjectTemplateInterface, targetingObject, actorAndTargetAreEnemiesAndTargetIsPlayer);
}

// ----------------------------------------------------------------------

Object * ClientShipTargeting::closestFriend(Object const * const targetingObject)
{
	return closestShipInWorldWithRelationShip(cs_sharedObjectTemplateInterface, targetingObject, actorAndTargetAreFriends);
}

// ----------------------------------------------------------------------

Object * ClientShipTargeting::closestFriendPlayer(Object const * const targetingObject)
{
	return closestShipInWorldWithRelationShip(cs_sharedObjectTemplateInterface, targetingObject, actorAndTargetAreFriendsAndTargetIsPlayer);
}

// ----------------------------------------------------------------------

Object * ClientShipTargeting::getNextAnything(Object const * const targetingObject, Object const * const currentTarget)
{
	ShipTargeting::ObjectVector resultObjects;
	ShipTargeting::allShipsWithRelationshipInWorld(cs_sharedObjectTemplateInterface, targetingObject, alwaysTrueRelationship, resultObjects);
	return getNextTarget(resultObjects, currentTarget);
}

// ----------------------------------------------------------------------

Object * ClientShipTargeting::getPreviousAnything(Object const * const targetingObject, Object const * const currentTarget)
{
	ShipTargeting::ObjectVector resultObjects;
	ShipTargeting::allShipsWithRelationshipInWorld(cs_sharedObjectTemplateInterface, targetingObject, alwaysTrueRelationship, resultObjects);
	return getPreviousTarget(resultObjects, currentTarget);
}

// ----------------------------------------------------------------------

Object * ClientShipTargeting::getNextFriendly(Object const * const targetingObject, Object const * const currentTarget)
{
	ShipTargeting::ObjectVector resultObjects;
	ShipTargeting::allShipsWithRelationshipInWorld(cs_sharedObjectTemplateInterface, targetingObject, actorAndTargetAreFriends, resultObjects);
	return getNextTarget(resultObjects, currentTarget);
}

// ----------------------------------------------------------------------

Object * ClientShipTargeting::getPreviousFriendly(Object const * const targetingObject, Object const * const currentTarget)
{
	ShipTargeting::ObjectVector resultObjects;
	ShipTargeting::allShipsWithRelationshipInWorld(cs_sharedObjectTemplateInterface, targetingObject, actorAndTargetAreFriends, resultObjects);
	return getPreviousTarget(resultObjects, currentTarget);
}

// ----------------------------------------------------------------------

Object * ClientShipTargeting::getNextEnemy(Object const * const targetingObject, Object const * const currentTarget)
{
	ShipTargeting::ObjectVector resultObjects;
	ShipTargeting::allShipsWithRelationshipInWorld(cs_sharedObjectTemplateInterface, targetingObject, actorAndTargetAreEnemies, resultObjects);
	return getNextTarget(resultObjects, currentTarget);
}

// ----------------------------------------------------------------------

Object * ClientShipTargeting::getPreviousEnemy(Object const * const targetingObject, Object const * const currentTarget)
{
	ShipTargeting::ObjectVector resultObjects;
	ShipTargeting::allShipsWithRelationshipInWorld(cs_sharedObjectTemplateInterface, targetingObject, actorAndTargetAreEnemies, resultObjects);
	return getPreviousTarget(resultObjects, currentTarget);
}

// ----------------------------------------------------------------------

Object * ClientShipTargeting::getNextFriendlyPlayer(Object const * const targetingObject, Object const * const currentTarget)
{
	ShipTargeting::ObjectVector resultObjects;
	ShipTargeting::allShipsWithRelationshipInWorld(cs_sharedObjectTemplateInterface, targetingObject, actorAndTargetAreFriendsAndTargetIsPlayer, resultObjects);
	return getNextTarget(resultObjects, currentTarget);
}

// ----------------------------------------------------------------------

Object * ClientShipTargeting::getPreviousFriendlyPlayer(Object const * const targetingObject, Object const * const currentTarget)
{
	ShipTargeting::ObjectVector resultObjects;
	ShipTargeting::allShipsWithRelationshipInWorld(cs_sharedObjectTemplateInterface, targetingObject, actorAndTargetAreFriendsAndTargetIsPlayer, resultObjects);
	return getPreviousTarget(resultObjects, currentTarget);
}

// ----------------------------------------------------------------------

Object * ClientShipTargeting::getNextEnemyPlayer(Object const * const targetingObject, Object const * const currentTarget)
{
	ShipTargeting::ObjectVector resultObjects;
	ShipTargeting::allShipsWithRelationshipInWorld(cs_sharedObjectTemplateInterface, targetingObject, actorAndTargetAreEnemiesAndTargetIsPlayer, resultObjects);
	return getNextTarget(resultObjects, currentTarget);
}

// ----------------------------------------------------------------------

Object * ClientShipTargeting::getPreviousEnemyPlayer(Object const * const targetingObject, Object const * const currentTarget)
{
	ShipTargeting::ObjectVector resultObjects;
	ShipTargeting::allShipsWithRelationshipInWorld(cs_sharedObjectTemplateInterface, targetingObject, actorAndTargetAreEnemiesAndTargetIsPlayer, resultObjects);
	return getPreviousTarget(resultObjects, currentTarget);
}

// ----------------------------------------------------------------------

int ClientShipTargeting::getOnScreenVariableReticleRadius()
{
	Camera const * const camera = Game::getCamera();

	if (camera != 0)
	{
		float const viewportHeight = static_cast<float>(camera->getViewportHeight());
		float const reticlePercentageOfScreen = s_onScreenVariableReticlePercentageOfScreen->getValue();
		float const targetDiameter = viewportHeight * reticlePercentageOfScreen;
		return static_cast<int>(targetDiameter * 0.5f);
	}

	// should never reach here while in game
	return 1;
}

// ----------------------------------------------------------------------

Object * ClientShipTargeting::getNextTargetOnScreenUnderVariableReticle(Object const * const targetingObject, Object const * const currentTarget)
{
	ShipTargeting::ObjectVector objectsInFov;
	float const onScreenTargetFov = ClientShipTargetingNamespace::getOnScreenTargetFov();
	ShipTargeting::allShipsInWorldUnderDefinedReticle(cs_sharedObjectTemplateInterface, targetingObject, onScreenTargetFov, objectsInFov);
	return getNextTarget(objectsInFov, currentTarget);
}

// ----------------------------------------------------------------------

Object * ClientShipTargeting::getPreviousTargetOnScreenUnderVariableReticle(Object const * const targetingObject, Object const * const currentTarget)
{
	ShipTargeting::ObjectVector objectsInFov;
	float const onScreenTargetFov = ClientShipTargetingNamespace::getOnScreenTargetFov();
	ShipTargeting::allShipsInWorldUnderDefinedReticle(cs_sharedObjectTemplateInterface, targetingObject, onScreenTargetFov, objectsInFov);
	return getPreviousTarget(objectsInFov, currentTarget);
}

// ----------------------------------------------------------------------

Object * ClientShipTargeting::getGroupLeader(Object const * const /*targetingObject*/)
{
	GroupObject const * group = getPlayerGroupObject();

	if (group != 0)
	{
		NetworkId const & leaderId = group->getLeader();

		CreatureObject const * const player = Game::getPlayerCreature();
		if (player != 0)
		{
			if (player->getNetworkId() == leaderId)
			{
				return 0;
			}
		}

		Object * const leader = NetworkIdManager::getObjectById(group->getShipFromMember(leaderId));
		return leader;
	}
	return 0;
}

// ----------------------------------------------------------------------

Object * ClientShipTargeting::getNextGroupMember(Object const * const targetingObject, Object const * const currentTarget)
{
	GroupObject const * group = getPlayerGroupObject();

	if (group != 0)
	{
		GroupObject::GroupMemberVector const & members = group->getGroupMembers();

		if (currentTarget != 0)
		{
			// find the next person in your group not youself
			CreatureObject const * const player = Game::getPlayerCreature();
			NetworkId const & playerId = (player != 0) ? player->getNetworkId() : NetworkId::cms_invalid;

			std::vector<NetworkId> otherShips;
			gatherUniqueShipsFromGroup(group, otherShips, playerId);

			std::vector<NetworkId>::const_iterator ii = std::find(otherShips.begin(), otherShips.end(), currentTarget->getNetworkId());
			if (ii != otherShips.end())
			{
				std::vector<NetworkId>::const_iterator next = ii;
				++next;
				if (next == otherShips.end())
				{
					next = otherShips.begin();
				}

				return (NetworkIdManager::getObjectById(*next));
			}
		}

		// no one was previously targeted that was in the group so grab the leader
		Object * const leader = getGroupLeader(targetingObject);

		if (leader != 0)
		{
			return leader;
		}

		// we are the leader so NULL was returned.  find the second person in the
		// group.  groups always have at least 2 people in them
		GroupObject::GroupMemberVector::const_iterator ii = members.begin();

		++ii;
		if (ii != members.end())
		{
			NetworkId const & shipId = group->getShipFromMember(ii->first);
			return(NetworkIdManager::getObjectById(shipId));
		}
	}

	return 0;
}

// ----------------------------------------------------------------------

Object * ClientShipTargeting::getPreviousGroupMember(Object const * const targetingObject, Object const * const currentTarget)
{
	GroupObject const * group = getPlayerGroupObject();

	if (group != 0)
	{
		GroupObject::GroupMemberVector const & members = group->getGroupMembers();

		if (currentTarget != 0)
		{
			// find the next person in your group not youself
			CreatureObject const * const player = Game::getPlayerCreature();
			NetworkId const & playerId = (player != 0) ? player->getNetworkId() : NetworkId::cms_invalid;

			std::vector<NetworkId> otherShips;
			gatherUniqueShipsFromGroup(group, otherShips, playerId);

			std::vector<NetworkId>::const_iterator ii = std::find(otherShips.begin(), otherShips.end(), currentTarget->getNetworkId());
			if (ii != otherShips.end())
			{
				std::vector<NetworkId>::const_iterator next = ii;
				if (next == otherShips.begin())
				{
					next = otherShips.end();
				}

				--next;

				return (NetworkIdManager::getObjectById(*next));
			}
		}

		// no one was previously targeted that was in the group so grab the leader
		Object * const leader = getGroupLeader(targetingObject);

		if (leader != 0)
		{
			return leader;
		}

		// we are the leader so NULL was returned.  find the second person in the
		// group.  groups always have at least 2 people in them
		GroupObject::GroupMemberVector::const_reverse_iterator ii = members.rbegin();

		if (ii != members.rend())
		{
			NetworkId const & shipId = group->getShipFromMember(ii->first);
			return(NetworkIdManager::getObjectById(shipId));
		}
	}

	return 0;
}

// ----------------------------------------------------------------------

void ClientShipTargeting::onPlayerShipHit(Object const *attacker)
{
	s_lastAttacker = attacker;

	// Target the attacker if the player creature is in a ship station and doesn't already have a target
	CreatureObject * const playerCreature = Game::getPlayerCreature();
	if (attacker && playerCreature && playerCreature->getLookAtTarget() == NetworkId::cms_invalid && playerCreature->getShipStation() != ShipStation::ShipStation_None)
		playerCreature->setLookAtTarget(attacker->getNetworkId());
}

// ----------------------------------------------------------------------

Object const *ClientShipTargeting::getLastAttacker()
{
	return s_lastAttacker.getPointer();
}

//=======================================================================
