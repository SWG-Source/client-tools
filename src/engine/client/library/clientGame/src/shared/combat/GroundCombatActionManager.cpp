// ======================================================================
//
// GroundCombatActionManager.cpp
// Copyright 2006 Sony Online Entertainment LLC (SOE)
// All rights reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/GroundCombatActionManager.h"
#include "clientAudio/Audio.h"
#include "clientGame/CellObject.h"
#include "clientGame/ClientCombatActionInfo.h"
#include "clientGame/ClientCombatPlaybackManager.h"
#include "clientGame/ClientCommandChecks.h"
#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientEffectTemplateList.h"
#include "clientGame/ClientEffectTemplate.h"
#include "clientGame/ClientEffect.h"
#include "clientGame/CreatureController.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/ShipObject.h"
#include "clientGame/ShipStation.h"
#include "clientGame/WeaponObject.h"
#include "clientGraphics/Camera.h"
#include "clientGraphics/DebugPrimitive.h"
#include "clientObject/ReticleManager.h"
#include "clientSkeletalAnimation/ConfigClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/TransformAnimationResolver.h"
#include "clientUserInterface/CuiCombatManager.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiTextManager.h"
#include "sharedCollision/CollideParameters.h"
#include "sharedCollision/CollisionInfo.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/Timer.h"
#include "sharedFoundation/Watcher.h"
#include "sharedGame/CombatDataTable.h"
#include "sharedGame/Command.h"
#include "sharedGame/CommandChecks.h"
#include "sharedGame/CommandTable.h"
#include "sharedMath/VectorArgb.h"
#include "sharedObject/Controller.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedGame/CombatTimingTable.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "swgSharedUtility/Postures.h"

// ======================================================================

namespace GroundCombatActionManagerNamespace
{
	void remove();
	void doWeaponReloadVisuals(CreatureObject * player, std::string const & weaponType);

	std::string getRandomElementFromString(char const delimiter, std::string const & stringToParse);
	std::string checkForAnimWildcard(char const wildcard, std::string const & inString);
	float doClientAnimation(CreatureObject * const player, NetworkId const & targetId, std::string const & actionName, Object * associatedObject = NULL);

	CachedNetworkId findBestTargetForAction(CreatureObject const * const player, GroundCombatActionManager::ObjectVector const & orderedTargets, std::string const & attackName, bool attemptDefaultAction, bool &performedPrimaryAction);
	bool doAutoAimLOSCheck(Vector startPosition, const Object *intendedTargetObj);

#ifdef _DEBUG
	bool s_drawAutoAimLOSChecks = false;
#endif

	Timer s_throttleTimer(0.0f);
	Timer s_weaponReloadTimer(0.0f);
	Timer s_postureTransitionTimer(0.0f);
	NetworkId s_postureTransitionTargetNetworkId = NetworkId::cms_invalid;
	bool s_wasPrimaryPostureTransitionTimerUsed = false;
	bool s_wasSecondaryPostureTransitionTimerUsed = false;


	int s_shotsTillWeaponReload = 0;
	bool s_attemptPrimaryAttack = false;
    bool s_attemptSecondaryAttack = false;
	bool s_secondaryAttackFromToolbar = false;
	bool s_attemptWalkForward = false;
	bool s_cancelAutoRun = false;
	bool s_attemptDefaultAction = false;

	bool s_repeatPrimaryAttack = false;

	bool s_executedSecondaryAttack = false;

	bool s_wantSecondaryAttackLoc = false;
	bool s_secondaryAttackLocValid = false;
	float s_secondaryAttackLocMinRange;
	float s_secondaryAttackLocMaxRange;
	Vector s_secondaryAttackLoc;
	NetworkId s_secondaryAttackLocCell;

	GroundCombatActionManager::attackCallback s_primaryAttackCallback = 0;
	GroundCombatActionManager::attackCallback s_secondaryAttackCallback = 0;

	GroundCombatActionManager::defaultActionCallback s_defaultActionCallback = 0;

	std::string s_secondaryActionName;
	Watcher<Object> s_secondaryActionObject;

	MessageDispatch::Callback s_callback;

	class Listener
	{
	public:
		Listener();
		~Listener();
		void onStateChanged(CreatureObject::Messages::StatesChanged::Payload const & creature);
		void onCurrentWeaponChanged(CreatureObject::Messages::CurrentWeaponChanged::Payload const & creature);

	private:
		Listener(Listener const &);
		Listener & operator=(const Listener &);

	private:
		bool m_isInCombat;
	};

	Listener s_listener;
	
	std::map<uint32, ClientEffectTemplate const *> s_idToReloadClientEffectTemplateMap;

	char const * const s_notEnoughActionSound = "sound/item_fusioncutter_end.snd";
	StringId const s_notEnoughActionString("combat_effects", "action_too_tired");
	VectorArgb const s_goldenrod(255, 218, 165, 32);

	int s_failedTestCodesShownPrimary = 0;
	int s_failedTestCodesShownSecondary = 0;
	void doCommandFailure(ClientObject &obj, NetworkId const & targetId, ClientCommandChecks::TestCode code, int & codesShown);
	
	int const totalWildcards = 2;
	std::string const s_wildcardReplacements[totalWildcards] = { "medium", "light" };
	ConstCharCrcString const heavyWeaponCooldown("heavy_weapon");
}

// ======================================================================

void GroundCombatActionManager::install()
{
#ifdef _DEBUG
	DebugFlags::registerFlag(GroundCombatActionManagerNamespace::s_drawAutoAimLOSChecks, "SharedCollision", "ClientAutoAimLOSCheck");
#endif

	ExitChain::add(GroundCombatActionManagerNamespace::remove, "GroundCombatActionManagerNamespace::remove");

	GroundCombatActionManagerNamespace::s_callback.connect(GroundCombatActionManagerNamespace::s_listener,
		&GroundCombatActionManagerNamespace::Listener::onStateChanged,
		static_cast<CreatureObject::Messages::StatesChanged*>(0));

	GroundCombatActionManagerNamespace::s_callback.connect(GroundCombatActionManagerNamespace::s_listener,
		&GroundCombatActionManagerNamespace::Listener::onCurrentWeaponChanged,
		static_cast<CreatureObject::Messages::CurrentWeaponChanged*>(0));
}

// ======================================================================

void GroundCombatActionManagerNamespace::remove()
{
#ifdef _DEBUG
	DebugFlags::unregisterFlag(s_drawAutoAimLOSChecks);
#endif

	GroundCombatActionManagerNamespace::s_callback.disconnect(GroundCombatActionManagerNamespace::s_listener,
		&GroundCombatActionManagerNamespace::Listener::onStateChanged,
		static_cast<CreatureObject::Messages::StatesChanged*>(0));
	GroundCombatActionManagerNamespace::s_callback.disconnect(GroundCombatActionManagerNamespace::s_listener,
		&GroundCombatActionManagerNamespace::Listener::onCurrentWeaponChanged,
		static_cast<CreatureObject::Messages::CurrentWeaponChanged*>(0));

	for (std::map<uint32, ClientEffectTemplate const *>::iterator i = s_idToReloadClientEffectTemplateMap.begin(); i != s_idToReloadClientEffectTemplateMap.end(); ++i)
	{
		i->second->release();
	}
}

// ----------------------------------------------------------------------

void GroundCombatActionManagerNamespace::doWeaponReloadVisuals(CreatureObject * const player, std::string const & weaponType)
{
	if (player == 0)
		return;

	if ((player->getVisualPosture() == Postures::Incapacitated) ||
		(player->getVisualPosture() == Postures::Dead))
		return;
	std::string const weaponReloadClientEffectName = CombatTimingTable::getWeaponReloadClientEffect(weaponType);

	if (!weaponReloadClientEffectName.empty())
	{
		CrcLowerString const name(weaponReloadClientEffectName.c_str());
		std::map<uint32, ClientEffectTemplate const *>::iterator i = s_idToReloadClientEffectTemplateMap.find(name.getCrc());
		ClientEffectTemplate const * clientEffectTemplate;
		if (i != s_idToReloadClientEffectTemplateMap.end())
			clientEffectTemplate = i->second;
		else
		{			
			clientEffectTemplate = ClientEffectTemplateList::fetch(name);
			s_idToReloadClientEffectTemplateMap.insert(std::make_pair(name.getCrc(), clientEffectTemplate));
		}
		
		if (clientEffectTemplate != 0)
		{
			CellProperty const * const cellProperty = player->getParentCell();
			Vector const & position_c = player->getPosition_c();

			ClientEffect * const clientEffect = clientEffectTemplate->createClientEffect(cellProperty, position_c, Vector::unitY);
			clientEffect->execute();
			delete clientEffect;
		}
	}

	std::string const weaponReloadClientAnimationName = CombatTimingTable::getWeaponReloadClientAnimation(weaponType);

	if (!weaponReloadClientAnimationName.empty())
	{
		//-- Play the animation action
		SkeletalAppearance2 * const appearance = dynamic_cast<SkeletalAppearance2 *>(player->getAppearance ());

		if (appearance)
		{
			int animationId = 0;
			bool animationIsAdd = false;

			appearance->getAnimationResolver().playAction(CrcLowerString(weaponReloadClientAnimationName.c_str()), animationId, animationIsAdd, 0);
		}
	}
}

// ----------------------------------------------------------------------

GroundCombatActionManagerNamespace::Listener::Listener()
: m_isInCombat(false)
{
}

// ----------------------------------------------------------------------

GroundCombatActionManagerNamespace::Listener::~Listener()
{
}

// ----------------------------------------------------------------------

void GroundCombatActionManagerNamespace::Listener::onStateChanged(CreatureObject::Messages::StatesChanged::Payload const & creature)
{
	CreatureObject * const player = Game::getPlayerCreature();

	if ((player != 0) && (player == &creature))
	{
		bool const isInCombat = creature.isInCombat();

		if (m_isInCombat != isInCombat)
		{
			if (!isInCombat)
			{
				WeaponObject const * const weaponObject = player->getCurrentWeapon();
				if (weaponObject != 0)
				{
					std::string const weaponType(WeaponObject::getWeaponTypeString(weaponObject->getWeaponType()));
					float const weaponReloadTimeSeconds = CombatTimingTable::getWeaponReloadTimeSeconds(weaponType);

					doWeaponReloadVisuals(player, weaponType);

					s_weaponReloadTimer.reset();
					s_weaponReloadTimer.setExpireTime(weaponReloadTimeSeconds);
					s_shotsTillWeaponReload = 0;
				}
			}
		}

		m_isInCombat = isInCombat;
	}
}

// ----------------------------------------------------------------------

void GroundCombatActionManagerNamespace::Listener::onCurrentWeaponChanged(CreatureObject::Messages::CurrentWeaponChanged::Payload const & creature)
{
	if (GroundCombatActionManager::wantSecondaryAttackLocation())
		GroundCombatActionManager::updateSecondaryReticleRange(creature);
}

// ----------------------------------------------------------------------

#ifdef _DEBUG
#define LOS_DEBUG_LINE(_startPos_, _endPos_) \
	if (s_drawAutoAimLOSChecks) \
		camera->addDebugPrimitive(new Line3dDebugPrimitive(Line3dDebugPrimitive::S_z, Transform::identity, _startPos_, _endPos_, VectorArgb::solidRed))
#else
#define LOS_DEBUG_LINE(_startPos_, _endPos_)
#endif

// ----------------------------------------------------------------------
// Checks from the input position to 5 points on the object (corners and middle)
bool GroundCombatActionManagerNamespace::doAutoAimLOSCheck(Vector startPosition, const Object *intendedTargetObj)
{
	CreatureObject *player = Game::getPlayerCreature();

	// Line of sight check...
	uint16 firstCollisionFlags = ClientWorld::CF_terrain
		| ClientWorld::CF_terrainFlora
		| ClientWorld::CF_tangible
		| ClientWorld::CF_tangibleNotTargetable
		| ClientWorld::CF_tangibleFlora
		| ClientWorld::CF_interiorObjects
		| ClientWorld::CF_interiorGeometry
		| ClientWorld::CF_childObjects;
	CollisionInfo firstCinfo;
	bool collided = true;
	Camera const * camera = Game::getCamera();
	UNREF(camera);

	if (intendedTargetObj->getCollisionSphereExtent_w().getRadius() > 0)
	{
		Vector endPos = intendedTargetObj->getCollisionSphereExtent_w().getCenter();
		real appearanceRadius = intendedTargetObj->getCollisionSphereExtent_w().getRadius();

		Vector playerUp = player->rotate_o2w(Vector(0, 1, 0));
		Vector tryAdjH = endPos - startPosition;
		tryAdjH = tryAdjH.cross(playerUp);
		tryAdjH.normalize();
		tryAdjH *= appearanceRadius;

		Vector tryAdjV = endPos - startPosition;
		tryAdjV = tryAdjV.cross(tryAdjH);
		tryAdjV.normalize();
		tryAdjV *= appearanceRadius;

		LOS_DEBUG_LINE(startPosition, endPos);
		if (!ClientWorld::collide(player->getParentCell(), startPosition, endPos, CollideParameters::cms_default, firstCinfo, firstCollisionFlags))
			return true;

		Vector tryPos;
		tryPos = endPos + tryAdjH + tryAdjV;
		LOS_DEBUG_LINE(startPosition, tryPos);
		if (!ClientWorld::collide(player->getParentCell(), startPosition, tryPos, CollideParameters::cms_default, firstCinfo, firstCollisionFlags))
			return true;

		tryPos = endPos - tryAdjH + tryAdjV;
		LOS_DEBUG_LINE(startPosition, tryPos);
		if (!ClientWorld::collide(player->getParentCell(), startPosition, tryPos, CollideParameters::cms_default, firstCinfo, firstCollisionFlags))
			return true;

		tryPos = endPos + tryAdjH - tryAdjV;
		LOS_DEBUG_LINE(startPosition, tryPos);
		if (!ClientWorld::collide(player->getParentCell(), startPosition, tryPos, CollideParameters::cms_default, firstCinfo, firstCollisionFlags))
			return true;

		tryPos = endPos - tryAdjH - tryAdjV;
		LOS_DEBUG_LINE(startPosition, tryPos);
		if (!ClientWorld::collide(player->getParentCell(), startPosition, tryPos, CollideParameters::cms_default, firstCinfo, firstCollisionFlags))
			return true;
	}
	else
	{
		Vector endPos = intendedTargetObj->getPosition_w();
		LOS_DEBUG_LINE(startPosition, endPos);
		collided = ClientWorld::collide(player->getParentCell(), startPosition, endPos, CollideParameters::cms_default, firstCinfo, firstCollisionFlags);
			return true;
	}
	return false;
}

// ----------------------------------------------------------------------

CachedNetworkId GroundCombatActionManagerNamespace::findBestTargetForAction(CreatureObject const * const player, GroundCombatActionManager::ObjectVector const & orderedTargets, std::string const & attackName, bool attemptDefaultAction, bool &performedPrimaryAction)
{
	CachedNetworkId targetId;
	Object * defaultActionTarget = 0;
	performedPrimaryAction = false;

	GroundCombatActionManager::ObjectVector::const_iterator ii = orderedTargets.begin();
	GroundCombatActionManager::ObjectVector::const_iterator iiEnd = orderedTargets.end();

	Command const& command = CommandTable::getCommand(Crc::normalizeAndCalculate(attackName.c_str()));

	for (; ii != iiEnd; ++ii)
	{
		GroundCombatActionManager::ObjectWatcher const potentialTarget = *ii;

		if (ii == orderedTargets.begin())
		{
			defaultActionTarget = potentialTarget;
		}

		if (potentialTarget != 0)
		{
			NetworkId potentialTargetId = potentialTarget->getNetworkId();

			if (potentialTargetId.isValid())
			{
				if (!ClientCommandChecks::doesTargetInvalidateCommand(&command, player, potentialTargetId, true))
				{
					attemptDefaultAction = false;
					targetId = *potentialTarget;
					break;
				}
				else if(attemptDefaultAction)
				{
					//we are supposed to be attempting a default action, and the first object is unattackable.  break out of this loop,
					//because we don't want to attack *through* a default action target that's not attackable.
					break;
				}
			}
		}
	}

	if ((attemptDefaultAction) && (defaultActionTarget != 0))
	{
		ClientObject const * const defaultActionTargetCO = defaultActionTarget->asClientObject();
		TangibleObject const * const defaultActionTargetTO = (defaultActionTargetCO != 0) ? defaultActionTargetCO->asTangibleObject() : 0;

		bool const isPlayer = (defaultActionTargetTO != 0) ? defaultActionTargetTO->isPlayer() : false;
		bool const isFriendly = (defaultActionTargetTO != 0) ? !defaultActionTargetTO->isEnemy() : true;

		if ((s_defaultActionCallback != 0) && (!(isPlayer && isFriendly)))
		{
			s_defaultActionCallback(*defaultActionTarget, false, 0, false);
			performedPrimaryAction = true;
			targetId = NetworkId::cms_invalid;
		}
	}

	if (command.m_targetType == Command::CTT_None)
	{
		targetId = NetworkId::cms_invalid;
		return targetId;
	}

	// If auto aim is on you'll shoot your intended target IF you don't use a default action...
	if (CuiPreferences::getAutoAimToggle() && !performedPrimaryAction)
	{
		targetId = NetworkId::cms_invalid;
		CachedNetworkId intendedTargetId(player->getIntendedTarget());
		Object *intendedTargetObj = NetworkIdManager::getObjectById(intendedTargetId);

		if (intendedTargetObj == 0)
			return targetId;

		// See if mouse happens to be over our intended (first object in ordered object list)
		ii = orderedTargets.begin();
		iiEnd = orderedTargets.end();
		if (ii != iiEnd)
		{
			GroundCombatActionManager::ObjectWatcher const potentialTarget = *ii;
			if (potentialTarget && potentialTarget->getNetworkId() == intendedTargetId)
			{
				return intendedTargetId;
			}
		}

		// Do world collision checks to see if we have LOS from character to object....
		Vector startPos;
		Vector endPos = intendedTargetObj->getPosition_w();
		if (player->getAppearance())
		{
			startPos = player->getCollisionSphereExtent_w().getCenter();
			startPos.y += player->getCollisionSphereExtent_w().getRadius();

			Vector tryAdjH = endPos - startPos;
			tryAdjH.normalize();
			tryAdjH = tryAdjH.cross(Vector(0, 1, 0)) * 0.5f;

			if (doAutoAimLOSCheck(startPos-tryAdjH, intendedTargetObj) ||
				doAutoAimLOSCheck(startPos+tryAdjH, intendedTargetObj))
				return intendedTargetId;
		}
		else
		{
			if (doAutoAimLOSCheck(player->getPosition_w(), intendedTargetObj))
				return intendedTargetId;
		}
	}

	return targetId;
}

// ----------------------------------------------------------------------

std::string GroundCombatActionManagerNamespace::getRandomElementFromString(char const delimiter, std::string const & stringToParse)
{
	std::string result = stringToParse;

	std::string::size_type loc = stringToParse.find(delimiter);
	if (std::string::npos != loc)
	{
		// there are options, pick one randomly
		std::vector<int> locations;
		locations.push_back(loc);
		while (std::string::npos != (loc = stringToParse.find(delimiter, loc+1)))
		{
			locations.push_back(loc);
		}

		const int numElements = locations.size() + 1;
		const uint32 choice = Random::random(0, numElements-1);

		int beginLoc = 0;
		std::string::size_type len = locations[0];
		if (0 != choice)
		{
			beginLoc = locations[choice-1]+1;
			if (locations.size() == choice)
			{
				len = stringToParse.size() - locations[choice-1];
			}
			else
			{
				len = locations[choice] - locations[choice-1] - 1;
			}
		}
		result = stringToParse.substr(beginLoc, len);
	}

	return result;
}

std::string GroundCombatActionManagerNamespace::checkForAnimWildcard(char const wildcard, std::string const & inString)
{
	int loc = inString.find(wildcard);

	if(loc == std::string::npos)
		return inString;

	std::string newString = inString.substr(0, loc);
	
	newString += s_wildcardReplacements[Random::random(0, totalWildcards-1)];

	return newString;
}

using namespace GroundCombatActionManagerNamespace;

// ======================================================================

void GroundCombatActionManager::registerPrimaryAttackCallback(attackCallback callback)
{
	s_primaryAttackCallback = callback;
}

// ----------------------------------------------------------------------

void GroundCombatActionManager::registerSecondaryAttackCallback(attackCallback callback)
{
	s_secondaryAttackCallback = callback;
}

// ----------------------------------------------------------------------

void GroundCombatActionManager::registerDefaultActionCallback(defaultActionCallback callback)
{
	s_defaultActionCallback = callback;
}

// ----------------------------------------------------------------------

void GroundCombatActionManager::setCurrentSecondaryAction(std::string const & actionName, Object * associatedObject)
{
	s_secondaryActionName = actionName;
	s_secondaryActionObject = associatedObject;
}

// ----------------------------------------------------------------------

void GroundCombatActionManager::attemptAction(ActionType const actionType)
{
	switch (actionType)
	{
		case AT_primaryAttack:
			{
				s_attemptPrimaryAttack = true;
			}
			break;

		case AT_secondaryAttack:
			{
				s_attemptSecondaryAttack = true;
				s_executedSecondaryAttack = false;
				s_wantSecondaryAttackLoc = false;
				s_secondaryAttackLocValid = false;
			}
			break;

		case AT_secondaryAttackFromToolbar:
			{
				s_attemptSecondaryAttack = true;
				s_secondaryAttackFromToolbar = true;
				s_executedSecondaryAttack = false;
				s_wantSecondaryAttackLoc = false;
				s_secondaryAttackLocValid = false;
			}
			break;

		case AT_primaryAction:
			{
				s_attemptDefaultAction = true;
			}
			break;

		case AT_primaryActionAndAttack:
			{
				s_attemptPrimaryAttack = true;
				s_attemptDefaultAction = true;
			}
			break;

		case AT_walkForward:
			{
				//when we start going forward, pull us out of auto run
				s_cancelAutoRun = true;
				s_attemptWalkForward = true;
			}
			break;
		case AT_toggleRepeatPrimaryAttack:
			{
				if (s_repeatPrimaryAttack)
				{
					s_repeatPrimaryAttack = false;
				}
				else
				{
					CreatureObject *player = Game::getPlayerCreature();
					if (!CuiPreferences::getAutoAimToggle() || player->getIntendedTarget() != NetworkId::cms_invalid)
						s_repeatPrimaryAttack = true;

					if(player->getPrimaryActionOverridden())
					{
						std::string const & commandName = player->getCurrentPrimaryActionName();
						uint32 primaryCmdHash = Crc::normalizeAndCalculate(commandName.c_str());
						Command const& cmd = CommandTable::getCommand(primaryCmdHash);

						if(cmd.m_targetType == Command::CTT_None)
							s_repeatPrimaryAttack = true;
					}
				}
				return;
			}
			break;
		case AT_cancelRepeatPrimaryAttack:
			{
				s_repeatPrimaryAttack = false;
				return;
			}
			break;
		default:
			break;
	}

	CreatureObject * const player = Game::getPlayerCreature();
	if (s_attemptPrimaryAttack || s_attemptDefaultAction)
	{
		if(player)
		{
			NetworkId id = player->getTargetUnderCursor();
			if (CuiPreferences::getTargetNothingUntargets() &&
				!CuiPreferences::getAutoAimToggle() &&
				id == NetworkId::cms_invalid)
			{
				player->setIntendedTarget(id);
				player->setLookAtTarget(id);

			}

		}
	}
	// automatically set the intended target for actions if the current intended target is not appropriate
	if (s_attemptPrimaryAttack || s_attemptDefaultAction || s_attemptSecondaryAttack)
	{
		if (player && player->getLookAtTarget().isValid())
		{
			// if the intended target is not valid, set it to the lookAt target
			if (!player->getIntendedTarget().isValid())
				player->setIntendedTarget(player->getLookAtTarget());
			else
			{
				Object const * const intendedTarget = player->getIntendedTarget().getObject();

				if (intendedTarget)
				{
					CreatureObject const * const creatureObject = dynamic_cast<CreatureObject const *>(intendedTarget);

					// if the intended target is dead, switch it to the lookAt target
					if (creatureObject && creatureObject->isDead())
						player->setIntendedTarget(player->getLookAtTarget());
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

void GroundCombatActionManager::clearAction(ActionType const actionType)
{
	switch(actionType)
	{
		case AT_primaryAttack:
			{
				s_failedTestCodesShownPrimary = 0;
				s_attemptPrimaryAttack = false;
			}
			break;

		case AT_secondaryAttack:
			{
				s_failedTestCodesShownSecondary = 0;
				s_attemptSecondaryAttack = false;
				s_executedSecondaryAttack = false;
				s_wantSecondaryAttackLoc = false;
				s_secondaryAttackLocValid = false;
			}
			break;
		case AT_secondaryAttackFromToolbar:
			{
				s_failedTestCodesShownSecondary = 0;
				s_attemptSecondaryAttack = false;
				s_secondaryAttackFromToolbar = false;
				s_executedSecondaryAttack = false;
				s_wantSecondaryAttackLoc = false;
				s_secondaryAttackLocValid = false;
			}
			break;
		case AT_primaryAction:
			{
				s_failedTestCodesShownPrimary = 0;
				s_attemptDefaultAction = false;
			}
			break;

		case AT_primaryActionAndAttack:
			{
				s_failedTestCodesShownPrimary = 0;
				s_attemptPrimaryAttack = false;
				s_attemptDefaultAction = false;
			}
			break;

		case AT_walkForward:
			{
				s_attemptWalkForward = false;
			}
			break;

		default:
			break;
	}
}

// ----------------------------------------------------------------------

void GroundCombatActionManager::update(float const deltaTimeSecs, ObjectVector const & orderedTargets)
{
	s_throttleTimer.updateNoReset(deltaTimeSecs);
	s_weaponReloadTimer.updateNoReset(deltaTimeSecs);
	s_postureTransitionTimer.updateNoReset(deltaTimeSecs);

	if (!s_weaponReloadTimer.isExpired())
	{
		if(s_secondaryAttackFromToolbar)
		{
			// kill it, otherwise it will que up
			clearAction(AT_secondaryAttackFromToolbar);
		}
		return;
	}

	CreatureObject * const player = Game::getPlayerCreature();

	if (s_postureTransitionTargetNetworkId.isValid() && player)
	{
		Object *postureTransitionTargetObj = NetworkIdManager::getObjectById(s_postureTransitionTargetNetworkId);
		if (!postureTransitionTargetObj)
		{
			s_postureTransitionTargetNetworkId = NetworkId::cms_invalid;
		}
		else
		{
			CreatureController *playerController = dynamic_cast<CreatureController *>(player->getController());
			if (postureTransitionTargetObj && playerController)
				playerController->overrideAnimationTarget(postureTransitionTargetObj, true, CrcLowerString::empty);
		}
	}

	if (!s_postureTransitionTimer.isExpired())
		return;

	if (player == 0)
		return;

	//we were waiting for posture transition that has now completed

	//-- also have to check the command that the weapon will enqueue
	std::string const & commandName = player->getCurrentPrimaryActionName();
	uint32 primaryCmdHash = Crc::normalizeAndCalculate(commandName.c_str());
	Command const& cmd = CommandTable::getCommand(primaryCmdHash);
	
	if (Game::isSpace())
	{
		if (player->getShipStation() == ShipStation::ShipStation_None)
		{
			if (s_attemptPrimaryAttack || s_attemptDefaultAction)
			{				
				Object * const defaultActionTarget = (!orderedTargets.empty()) ? orderedTargets.front().getPointer() : 0;
				if ((defaultActionTarget) && (s_defaultActionCallback != 0))
				{
					s_defaultActionCallback(*defaultActionTarget, false, 0, false);
				}
			}
			s_attemptPrimaryAttack = false;
			s_attemptDefaultAction = false;

			if (s_attemptWalkForward)
			{
				CreatureObject * player = Game::getPlayerCreature();
				Controller * const controller = (player != 0) ? player->getController() : 0;

				if (controller != 0)
				{
					if (s_cancelAutoRun)
					{
						controller->appendMessage(CM_cancelAutoRun, 0.0f);
						s_cancelAutoRun = false;
					}

					controller->appendMessage(CM_walk, 0.0f);
				}
			}
		}

		return;
	}

	// Update the zip bar with the actual cooldown remaining. The max time may not be 100%
	// accurate as the server may have adjusted our total cooldown based on skills etc.
	if(!cmd.m_commandName.empty() && cmd.m_addToCombatQueue)
	{
		float maxTime = 1.0f;		
		float timeLeft = static_cast<float>(ClientCommandQueue::getCooldownRemainingForCommand(cmd));
		WeaponObject const * const weaponObject = player->getCurrentWeapon();
		if (player->getPrimaryActionOverridden())
		{
			maxTime = std::max(cmd.m_warmTime + cmd.m_execTime + cmd.m_coolTime, timeLeft);
		}
		else if (weaponObject != 0)
		{
			maxTime = weaponObject->getAttackSpeed();
		}

		s_throttleTimer.setExpireTime(maxTime);
		s_throttleTimer.setElapsedTime(maxTime - timeLeft);
	}

	bool doPrimaryAttack = (s_attemptPrimaryAttack || s_wasPrimaryPostureTransitionTimerUsed) && s_throttleTimer.isExpired();


	// If repeat attack is turned on, go ahead and try to use a primary attack even if the player didn't
	// explicitly ask for it.
	if (s_repeatPrimaryAttack && s_throttleTimer.isExpired() && !s_attemptPrimaryAttack)
	{
		CachedNetworkId intendedTargetId = player->getIntendedTarget();
		Object *intendedTargetObj = NetworkIdManager::getObjectById(intendedTargetId);
		CreatureObject *intendedTargetCreature = dynamic_cast<CreatureObject *>(intendedTargetObj);
		// Stop auto-attacking if ...
		if (player->isIncapacitated() || player->isDead())
			s_repeatPrimaryAttack = false;
		if (intendedTargetCreature && (intendedTargetCreature->isDead() || intendedTargetCreature->isIncapacitated()))
			s_repeatPrimaryAttack = false;
		if (s_repeatPrimaryAttack)
			doPrimaryAttack = true;
	}


	if (doPrimaryAttack && (cmd.m_commandName.empty() || !ClientCommandQueue::canEnqueueCombatCommand(true,cmd)))
	{
		doPrimaryAttack = false;
	}

	if (!doPrimaryAttack && s_attemptDefaultAction == true)
	{
		std::string const & commandName = player->getCurrentPrimaryActionName();
		bool performedPrimaryAction = false;
		CachedNetworkId targetId = findBestTargetForAction(player, orderedTargets, commandName, s_attemptDefaultAction, performedPrimaryAction);
		s_attemptDefaultAction = false;
	}

	if (doPrimaryAttack || s_wasPrimaryPostureTransitionTimerUsed)
	{
		std::string const & commandName = player->getCurrentPrimaryActionName();
		bool const primaryActionIsOverridden = player->getPrimaryActionOverridden();

		bool performedPrimaryAction = false;
		CachedNetworkId targetId;
		if (s_wasPrimaryPostureTransitionTimerUsed && s_postureTransitionTargetNetworkId.isValid())
		{
			targetId = s_postureTransitionTargetNetworkId;
		}
		else 
		{
			targetId = findBestTargetForAction(player, orderedTargets, commandName, s_attemptDefaultAction, performedPrimaryAction);
		}

		s_attemptDefaultAction = false;

		bool firstTargetIsUnattackable = false;
		if (!orderedTargets.empty())
		{
			Object *frontObj = orderedTargets.front().getPointer();
			if (frontObj)
			{
				TangibleObject * const frontObjTangible = dynamic_cast<TangibleObject *>(frontObj);
				if (frontObjTangible)
				{
					firstTargetIsUnattackable = !frontObjTangible->isAttackable();

					CreatureObject * const frontObjCreature = frontObjTangible->asCreatureObject();
					if(frontObjCreature && frontObjCreature->isDead())
						firstTargetIsUnattackable = true;
				}
			}
		}

		bool shouldAttack = true;
		if (performedPrimaryAction)
		{
			s_attemptPrimaryAttack = false;
			shouldAttack = false;
		}
		
		Command const & command = CommandTable::getCommand(Crc::normalizeAndCalculate(commandName.c_str()));
		bool const heavyWeapon = (command.m_coolGroup == heavyWeaponCooldown.getCrc());
		
		// If we are trying to use a heavy weapon then we don't want this check to occur. There is a special check a little bit
		// further down (actually, right after this code) that will do the proper checks since Heavy Weapon cost can vary if you
		// are a commando.
		ClientCommandChecks::TestCode tc = ClientCommandChecks::canCreatureExecuteCommand(true, commandName, player, targetId);
		if (tc != ClientCommandChecks::TC_pass)
		{

			if(tc == ClientCommandChecks::TC_failNotEnoughAction && heavyWeapon)
			{
				// Don't do anything. We failed for action while trying to use a heavy weapon. Give the checks below
				// a chance to check for commando special heavy weapon expertise exemption.
			}
			else if (tc != ClientCommandChecks::TC_failNoTarget)
			{
				// do not fail the command when no target was set as that causes excessive combat logs
				//   when the player is clicking to try and hit a target
				doCommandFailure(*player, targetId, tc, s_failedTestCodesShownPrimary);
				shouldAttack = false;
			}
			else
				shouldAttack = false;
		}

		WeaponObject const * const weaponObject = player->getCurrentWeapon();
		if (shouldAttack && (weaponObject != 0))
		{
			if (primaryActionIsOverridden && command.m_targetType == Command::CTT_None && tc == ClientCommandChecks::TC_pass)
			{
				shouldAttack = true;
			}
			else if (primaryActionIsOverridden && player->getPrimaryActionWantsGroundReticule())
			{
				//mimic the heavy weapons behavior
				if (CuiPreferences::getAutoAimToggle())
				{
					shouldAttack = (targetId.isValid() && targetId == player->getIntendedTarget() && (tc == ClientCommandChecks::TC_pass));
				}				
				else
				{
					shouldAttack = (ReticleManager::getReticleCurrentlyValid() && (tc == ClientCommandChecks::TC_pass));
				}
			}
			else if (!CuiPreferences::getAutoAimToggle() && weaponObject->isDirectionalTargetting())
			{			
				ClientCommandChecks::TestCode tc = ClientCommandChecks::canCreatureFireHeavyWeapon(commandName, player);
				if(tc == ClientCommandChecks::TC_failNotEnoughAction)
				{
					doCommandFailure(*player, targetId, tc, s_failedTestCodesShownPrimary);
				}
				shouldAttack = !firstTargetIsUnattackable && (tc == ClientCommandChecks::TC_pass);
			}
			else if (!CuiPreferences::getAutoAimToggle() && weaponObject->isGroundTargetting())
			{
				ClientCommandChecks::TestCode tc = ClientCommandChecks::canCreatureFireHeavyWeapon(commandName, player);
				if(tc == ClientCommandChecks::TC_failNotEnoughAction)
				{
					doCommandFailure(*player, targetId, tc, s_failedTestCodesShownPrimary);
				}
				shouldAttack = ReticleManager::getReticleCurrentlyValid() && !firstTargetIsUnattackable && (tc == ClientCommandChecks::TC_pass);
			}
			else if (CuiPreferences::getAutoAimToggle())
			{
				ClientCommandChecks::TestCode tc = ClientCommandChecks::canCreatureFireHeavyWeapon(commandName, player);
				if(tc == ClientCommandChecks::TC_failNotEnoughAction)
				{
					doCommandFailure(*player, targetId, tc, s_failedTestCodesShownPrimary);
				}
				shouldAttack = (targetId.isValid() && targetId == player->getIntendedTarget() && (tc == ClientCommandChecks::TC_pass));
			}
			else
			{
				shouldAttack = !firstTargetIsUnattackable && targetId.isValid();
			}
		}
		else
		{
			shouldAttack = false;
		}

		if (s_wasPrimaryPostureTransitionTimerUsed || shouldAttack)
		{						
			std::string const weaponType(WeaponObject::getWeaponTypeString(weaponObject->getWeaponType()));
			int   const maximumShotsTillWeaponReload = CombatTimingTable::getMaximumShotsTillWeaponReload(weaponType);
			float const weaponReloadTimeSeconds      = CombatTimingTable::getWeaponReloadTimeSeconds(weaponType);

			bool const canFireShot = s_shotsTillWeaponReload < maximumShotsTillWeaponReload;

			if (canFireShot)
			{
				if (!CuiPreferences::getAutoAimToggle() && targetId.isValid())
					player->setLookAtTarget(targetId);
	
				//kick off client animation (function handles rather or not it should be kicked off)
				const float postureTransitionDelay = doClientAnimation(player, targetId, commandName);

				if (0.0f < postureTransitionDelay && !s_wasPrimaryPostureTransitionTimerUsed)
				{
					//we need to wait for the posture transition to complete before firing the animation
					s_postureTransitionTimer.reset();
					s_postureTransitionTimer.setExpireTime(postureTransitionDelay);
					s_wasPrimaryPostureTransitionTimerUsed = true;
					s_postureTransitionTargetNetworkId = targetId;
					return;
				}

				if (s_primaryAttackCallback != 0)
				{
					if(!s_attemptSecondaryAttack)
					{
						s_failedTestCodesShownPrimary = 0;
						s_primaryAttackCallback(targetId);
					}
				}

				s_throttleTimer.reset();

				if (primaryActionIsOverridden)
				{
					if (CombatDataTable::getCancelsAutoAttack(primaryCmdHash))
					{
						s_repeatPrimaryAttack = false;
					}

					//This is a guess as to what the cooldown time will be since skills etc can make the
					//actual cooldown time different from what appears in the command table
					float commandSpacingTime = command.m_warmTime + command.m_execTime + command.m_coolTime;
					s_throttleTimer.setExpireTime(commandSpacingTime);
				}
				else if (weaponObject != 0)
				{
					s_throttleTimer.setExpireTime(weaponObject->getAttackSpeed());
				}

				++s_shotsTillWeaponReload;
			}

			if (maximumShotsTillWeaponReload != 0 && s_shotsTillWeaponReload >= maximumShotsTillWeaponReload)
			{
				s_weaponReloadTimer.reset();
				s_weaponReloadTimer.setExpireTime(weaponReloadTimeSeconds);
				s_shotsTillWeaponReload = 0;

				doWeaponReloadVisuals(player, weaponType);
			}

			if (!CombatTimingTable::isContinuous(weaponType))
			{
				s_attemptPrimaryAttack = false;
				s_attemptSecondaryAttack = false;
				s_secondaryAttackFromToolbar = false;
			}
		}
	}

	if (s_wasSecondaryPostureTransitionTimerUsed || (s_attemptSecondaryAttack && !s_executedSecondaryAttack))
	{
		const uint32 secondaryActionNameHash = Crc::normalizeAndCalculate(s_secondaryActionName.c_str());
		Command const & command = CommandTable::getCommand(secondaryActionNameHash);

		bool performedPrimaryAction = false;
		CachedNetworkId targetId;
		if (s_wasSecondaryPostureTransitionTimerUsed)
		{
			targetId = s_postureTransitionTargetNetworkId;
		}
		else
		{
			targetId = findBestTargetForAction(player, orderedTargets, s_secondaryActionName, false, performedPrimaryAction);
		}

		ClientCommandChecks::TestCode tc = ClientCommandChecks::canCreatureExecuteCommand(false, s_secondaryActionName, player, targetId, s_secondaryActionObject.getPointer());

		if (command.m_targetType == Command::CTT_Location && tc == ClientCommandChecks::TC_pass)
		{
			if (!s_wantSecondaryAttackLoc)
			{
				s_secondaryAttackLocValid = false;
				s_wantSecondaryAttackLoc = true;
				updateSecondaryReticleRange(*player);
				return;
			}
			if (s_wantSecondaryAttackLoc && !s_secondaryAttackLocValid)
			{
				return;
			}
		}


		if (tc != ClientCommandChecks::TC_pass)
		{
			doCommandFailure(*player, targetId, tc, s_failedTestCodesShownSecondary);

			if (s_secondaryAttackFromToolbar)
			{
				s_wasPrimaryPostureTransitionTimerUsed = false;
				s_wasSecondaryPostureTransitionTimerUsed = false;
				s_postureTransitionTargetNetworkId = NetworkId::cms_invalid;
				clearAction(AT_secondaryAttackFromToolbar);
				return;
			}
		}
		else
		{
			if (!CuiPreferences::getAutoAimToggle() && targetId.isValid())
				player->setLookAtTarget(targetId);

			//kick off client animation (function handles rather or not it should be kicked off)
			const float fPostureTransitionDelay = doClientAnimation(player, targetId, s_secondaryActionName, s_secondaryActionObject);

			if (0.0f < fPostureTransitionDelay && !s_wasSecondaryPostureTransitionTimerUsed)
			{
				//we need to wait for the posture transition to complete before firing the animation
				s_postureTransitionTimer.reset();
				s_postureTransitionTimer.setExpireTime(fPostureTransitionDelay);
				s_wasSecondaryPostureTransitionTimerUsed = true;
				s_postureTransitionTargetNetworkId = targetId;
				return;
			}

			if (s_secondaryAttackCallback != 0)
			{
				// Stop attacking if the secondary attack wants us to...
				if (secondaryActionNameHash != 0 && CombatDataTable::getCancelsAutoAttack(secondaryActionNameHash))
				{
					s_repeatPrimaryAttack = false;
					clearAction(AT_primaryAttack);
				}

				s_failedTestCodesShownSecondary = 0;
				s_secondaryAttackCallback(targetId);
			}


			if(s_secondaryAttackFromToolbar)
			{
				clearAction(AT_secondaryAttackFromToolbar);
			}
			else
			{
				s_executedSecondaryAttack = true;
			}
			s_wantSecondaryAttackLoc = false;
			s_secondaryAttackLocValid = false;
		}
	}

	s_wasPrimaryPostureTransitionTimerUsed = false;
	s_wasSecondaryPostureTransitionTimerUsed = false;
	s_postureTransitionTargetNetworkId = NetworkId::cms_invalid;

	if (s_attemptWalkForward)
	{
		CreatureObject * player = Game::getPlayerCreature();
		Controller * const controller = (player != 0) ? player->getController() : 0;

		if (controller != 0)
		{
			if (s_cancelAutoRun)
			{
				controller->appendMessage(CM_cancelAutoRun, 0.0f);
				s_cancelAutoRun = false;
			}
			controller->appendMessage(CM_walk, 0.0f);
		}
	}
}

// ======================================================================

//do client-side animation, if attack type warrants it
float GroundCombatActionManagerNamespace::doClientAnimation(CreatureObject * const player, NetworkId const & targetId, const std::string & actionName, Object * associatedObject)
{
	uint32 const actionNameHash = Crc::normalizeAndCalculate (actionName.c_str());

	//only animate on client if combat data tells us to
	if (!CombatDataTable::isActionClientAnim(actionNameHash))
	{
		return 0.0f;
	}

	if(!player)
		return 0.0f;


	bool requiresTarget = false;

	Command const & command = CommandTable::getCommand(actionNameHash);
	
// TODO: def'd out is what we believe to be the valid change - significant data is affected - holding off for now
#if 0
	const int32 validTargetForActionName = CombatDataTable::getValidTarget(actionNameHash);
	requiresTarget = command.m_targetType != Command::CTT_Optional 
		&& (command.m_targetType == Command::CTT_Required || validTargetForActionName != CombatDataTable::VTT_none);
#else
	requiresTarget = command.m_targetType == Command::CTT_Required || CombatDataTable::getValidTarget(actionNameHash) != CombatDataTable::VTT_none;

#endif
// TODO: end def

	
	CreatureController *controller = (static_cast<CreatureController*>(player->getController()));
	if(requiresTarget && controller && !controller->areVisualsInCombat())
	{
		controller->forceVisualsIntoCombat();
		return ConfigClientSkeletalAnimation::getBlendTime();//Wait for the blend to finish
	}

	Object * playerWeapon = player->getCurrentWeapon();
	
	if (NULL != associatedObject)
	{
		playerWeapon = associatedObject;
	}

	if (NULL == playerWeapon)
		return 0.0f;

	std::string weaponType = "unknown";
	WeaponObject * const weaponObj = dynamic_cast<WeaponObject*>(playerWeapon);

	if (NULL != weaponObj)
	{
		weaponType = WeaponObject::getWeaponTypeString(weaponObj->getWeaponType());
	}
	
	std::string actionAnimation = CombatDataTable::getActionAnimationData(actionNameHash, weaponType.c_str());

	actionAnimation = getRandomElementFromString(',', actionAnimation); //returns input if no delims found

	actionAnimation = checkForAnimWildcard('^', actionAnimation); // Check for wildcards.

	// get the defender, put him in combat mode
	TangibleObject * const defender = dynamic_cast<TangibleObject *>(CachedNetworkId (targetId).getObject ());
	CreatureObject * const defenderCreature = defender ? defender->asCreatureObject () : NULL;

	//@todo NPE_TODO: hacktastical: if the defender has a "saberBlock" buff, and a lightsaber equiped, and we're firing a ranged weapon, then they'll see the shot blocked
		//  in the future, we'll moving all combat logic to client so it can know the outcome of the attack.
	ClientCombatActionInfo::DefenderDefense defenderDisposition = ClientCombatActionInfo::DD_hit;
	if (weaponObj && defenderCreature)
	{
		if (defenderCreature->getCurrentWeapon() && defenderCreature->getCurrentWeapon()->isLightsaberWeapon() //defender has a lightsaber
			&& defenderCreature && defenderCreature->hasBuff(Crc::normalizeAndCalculate("saberBlock")) //and the defender has the saberBlock buff
			&& weaponObj->isRangedWeapon()) //we are firing a ranged weapon
		{
			defenderDisposition = ClientCombatActionInfo::DD_lightsaberBlock;
		}
	}
	
	bool attackerUseLocation = false;
	Vector attackerTargetLocation;
	NetworkId attackerTargetCell;

	if(weaponObj->isGroundTargetting())
	{
		attackerUseLocation = true;
		attackerTargetLocation = ReticleManager::getLastGroundReticlePoint();
		attackerTargetCell = ReticleManager::getLastGroundReticleCell();
		if(attackerTargetCell != NetworkId::cms_invalid)
		{
			CellObject *cellObject = safe_cast<CellObject *>(NetworkIdManager::getObjectById(attackerTargetCell));
			if(cellObject)
				attackerTargetLocation = cellObject->getTransform_o2w().rotateTranslate_p2l(attackerTargetLocation);
		}
	}

	//-- Setup an attack.
	uint32 const  actionId                = Crc::normalizeAndCalculate (actionAnimation.c_str ());
	int           attackerPostureEndIndex = static_cast<int>(player->getVisualPosture());

	if(weaponObj 
		&& weaponObj->isMeleeWeapon()
		&& CombatDataTable::getHitType(actionNameHash) != CombatDataTable::HT_nonAttack
	)
	{
		player->setVisualPosture(0);
		attackerPostureEndIndex = 0;   //When attacking with melee weapons, you always stand up first
	}

	int const     attackerTrailBits       = CombatDataTable::getTrailBits(actionNameHash);
	int const     attackerClientEffectId  = 0;
	int           defenderEndPostureIndex = defenderCreature ? static_cast<int>(defenderCreature->getVisualPosture()) : 0;
	int const     defenderClientEffectId  = 0;
	int const     defenderHitLocation     = 0;
	int const     damageAmount            = 0;

	ClientCombatActionInfo const actionInfo(player,  //have this
											playerWeapon,	 //have this
											attackerPostureEndIndex, 
											attackerTrailBits, 
											attackerClientEffectId, 
											actionNameHash, 
											attackerUseLocation,
											attackerTargetLocation,
											attackerTargetCell,
											actionId, 
											defender, //have this
											defenderEndPostureIndex, 
											defenderDisposition, 
											defenderClientEffectId, 
											defenderHitLocation, 
											damageAmount); //setting to 100, or 0 (depending on if we want the defender to look like he was hit)

	// submit combat action for playback
	ClientCombatPlaybackManager::handleCombatAction(actionInfo);

	return 0.0f;
}

// ======================================================================

float GroundCombatActionManager::getPrimaryThrottleTimer()
{
	return s_throttleTimer.getRemainingRatio();
}

// ----------------------------------------------------------------------

void GroundCombatActionManagerNamespace::doCommandFailure(ClientObject & obj, NetworkId const & targetId, ClientCommandChecks::TestCode code, int & shownCodes)
{
	if (shownCodes & code)
		return;
	shownCodes |= code;

	const Object * const target = NetworkIdManager::getObjectById(targetId);
	Vector position_wTarget;
	if (target)
		position_wTarget = target->getPosition_w();

	switch (code)
	{
	case ClientCommandChecks::TC_failAttackingSelf:
		CuiCombatManager::sendFakeCombatSpam(obj.getNetworkId(), obj.getPosition_w(), targetId, position_wTarget, NetworkId::cms_invalid, 0, 0, Unicode::narrowToWide("@cbt_spam:shoot_self"));
		break;
	case ClientCommandChecks::TC_failInvalidLocomotion:
		break;
	case ClientCommandChecks::TC_failPlayerLacksAbility:
		CuiCombatManager::sendFakeCombatSpam(obj.getNetworkId(), obj.getPosition_w(), targetId, position_wTarget, NetworkId::cms_invalid, 0, 0, Unicode::narrowToWide("@cbt_spam:dont_have_ability"));
		break;
	case ClientCommandChecks::TC_failNotEnoughAction:
		CuiCombatManager::sendFakeCombatSpam(obj.getNetworkId(), obj.getPosition_w(), targetId, position_wTarget, NetworkId::cms_invalid, 0, 0, Unicode::narrowToWide("@cbt_spam:not_enough_action"));

		Audio::playSound(s_notEnoughActionSound, NULL);			
		obj.addFlyText(s_notEnoughActionString.localize(), 1.0, s_goldenrod, 1.5, CuiTextManagerTextEnqueueInfo::TW_starwars);
		break;
	case ClientCommandChecks::TC_failInvalidTarget:
		CuiCombatManager::sendFakeCombatSpam(obj.getNetworkId(), obj.getPosition_w(), targetId, position_wTarget, NetworkId::cms_invalid, 0, 0, Unicode::narrowToWide("@cbt_spam:invalid_target"));
		break;
	case ClientCommandChecks::TC_failInvalidPosture:
		CuiCombatManager::sendFakeCombatSpam(obj.getNetworkId(), obj.getPosition_w(), targetId, position_wTarget, NetworkId::cms_invalid, 0, 0, Unicode::narrowToWide("@cbt_spam:not_req_posture_0"));
		break;
	case ClientCommandChecks::TC_failInvalidState:
		CuiCombatManager::sendFakeCombatSpam(obj.getNetworkId(), obj.getPosition_w(), targetId, position_wTarget, NetworkId::cms_invalid, 0, 0, Unicode::narrowToWide("@spam:cant_do_it_state"));
		break;
	case ClientCommandChecks::TC_failInvalidWeapon:
		CuiCombatManager::sendFakeCombatSpam(obj.getNetworkId(), obj.getPosition_w(), targetId, position_wTarget, NetworkId::cms_invalid, 0, 0, Unicode::narrowToWide("@cbt_spam:invalid_weapon_single"));
		break;
	case ClientCommandChecks::TC_failCooldown:
		break;
	case ClientCommandChecks::TC_failOutOfRange:
		CuiCombatManager::sendFakeCombatSpam(obj.getNetworkId(), obj.getPosition_w(), targetId, position_wTarget, NetworkId::cms_invalid, 0, 0, Unicode::narrowToWide("@cbt_spam:out_of_range"), CuiCombatManager::COMBAT_RESULT_OUT_OF_RANGE);
		break;
	default:
		break;
	}
}

// ----------------------------------------------------------------------

bool GroundCombatActionManager::getRepeatAttackEnabled()
{
	return s_repeatPrimaryAttack;
}

// ----------------------------------------------------------------------

void GroundCombatActionManager::setSecondaryAttackLocation(Vector location, const NetworkId cellId)
{
	s_secondaryAttackLoc = location;
	s_secondaryAttackLocCell = cellId;
	s_secondaryAttackLocValid = true;
}

// ----------------------------------------------------------------------

bool GroundCombatActionManager::getSecondaryAttackLocation(Vector & location, NetworkId & cellId)
{
	location = s_secondaryAttackLoc;
	cellId = s_secondaryAttackLocCell;
	return s_secondaryAttackLocValid;
}

// ----------------------------------------------------------------------

bool GroundCombatActionManager::wantSecondaryAttackLocation()
{
	return s_wantSecondaryAttackLoc;
}

// ----------------------------------------------------------------------

bool GroundCombatActionManager::getPrimaryAttackLocation(Vector & location, NetworkId & cellId)
{
	location = ReticleManager::getLastGroundReticlePoint();
	cellId = ReticleManager::getLastGroundReticleCell();
	return ReticleManager::getReticleCurrentlyValid();
}

// ----------------------------------------------------------------------

void GroundCombatActionManager::updateSecondaryReticleRange(const CreatureObject & creature)
{
	const uint32 secondaryActionNameHash = Crc::normalizeAndCalculate(s_secondaryActionName.c_str());
	Command const & command = CommandTable::getCommand(secondaryActionNameHash);
	if (command.m_targetType == Command::CTT_Location)
	{
		bool weaponCheckLast = false;

		WeaponObject const* weapon = dynamic_cast<WeaponObject const*>(s_secondaryActionObject.getPointer());
		if (!weapon)
			weapon = creature.getCurrentWeapon();
		else
			weaponCheckLast = true;

		float minWeaponRange = weapon ? weapon->getMinRange() : -1.0f;
		float maxWeaponRange = weapon ? weapon->getMaxRange() : -1.0f;
		if (weapon)
			CommandChecks::getRangeForCommand(&command, minWeaponRange, maxWeaponRange, weaponCheckLast, s_secondaryAttackLocMinRange, s_secondaryAttackLocMaxRange);
		else
			CommandChecks::getRangeForCommand(&command, -1, -1, false, s_secondaryAttackLocMinRange, s_secondaryAttackLocMaxRange);
	}
}

// ----------------------------------------------------------------------

bool GroundCombatActionManager::isReticleLocationInRange(float distance)
{
	if (wantSecondaryAttackLocation())
	{
		if (s_secondaryAttackLocMinRange > 0 && distance < s_secondaryAttackLocMinRange)
			return false;
		if (s_secondaryAttackLocMaxRange > 0 && distance > s_secondaryAttackLocMaxRange)
			return false;
	}
	else
	{
		CreatureObject * const player = Game::getPlayerCreature ();

		if(!player)
		{
			return false;
		}
		else if(player->getPrimaryActionOverridden())
		{
			std::string const & commandName = player->getCurrentPrimaryActionName();
			Command const & command = CommandTable::getCommand(Crc::normalizeAndCalculate(commandName.c_str()));
			WeaponObject const* weapon = player->getCurrentWeapon();

			float minWeaponRange = weapon ? weapon->getMinRange() : -1.0f;
			float maxWeaponRange = weapon ? weapon->getMaxRange() : -1.0f;

			float minRange = -1.0f;
			float maxRange = -1.0f;
			CommandChecks::getRangeForCommand(&command, minWeaponRange, maxWeaponRange, false, minRange, maxRange);

			if((minRange > 0.0f && distance < minRange)
				|| maxRange > 0.0f && distance > maxRange)
			{
				return false;
			}
		}
		else
		{
			if(!player->getCurrentWeapon() || (player->getCurrentWeapon()->isGroundTargetting() && (player->getCurrentWeapon()->getMaxRange() < distance)))
				return false;
		}
	}

	return true;
}

// ======================================================================
