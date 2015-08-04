// ======================================================================
//
// PlaySkeletalAnimationAction.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/PlaySkeletalAnimationAction.h"

#include "clientAnimation/PlaybackScript.h"
#include "clientGame/CombatEffectsManager.h"
#include "clientGame/CreatureController.h"
#include "clientGame/CreatureInfo.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/PlaySkeletalAnimationActionTemplate.h"
#include "clientSkeletalAnimation/AnimationStateNameIdManager.h"
#include "clientSkeletalAnimation/AnimationStatePath.h"
#include "clientSkeletalAnimation/CallbackAnimationNotification.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/TransformAnimationResolver.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedObject/Object.h"
#include "swgSharedUtility/Postures.def"
#include "swgSharedUtility/States.def"

// ======================================================================

namespace PlaySkeletalAnimationActionNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	CreatureObject *getMountForObject(Object &potentialRider);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	float const  cs_mountCombatYawAdjustmentRadians = -45.0f * PI_OVER_180;  // mount faces 45 degrees to the left of target.

	int const    cs_standardAttackerActorIndex = 0;
	int const    cs_standardDefenderActorIndex = 1;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	std::string  s_actionName;

	AnimationStateNameId  s_combatAnimationStateId;
	AnimationStateNameId  s_pistolAnimationStateId;
	AnimationStateNameId  s_rifleAnimationStateId;
}

using namespace PlaySkeletalAnimationActionNamespace;

// ======================================================================
// namespace PlaySkeletalAnimationActionNamespace
// ======================================================================

CreatureObject *PlaySkeletalAnimationActionNamespace::getMountForObject(Object &potentialRider)
{
	ClientObject *const clientRider = potentialRider.asClientObject();
	if (!clientRider)
		return NULL;

	CreatureObject *const creatureRider = clientRider->asCreatureObject();
	if (!creatureRider)
		return NULL;

	if (!creatureRider->getState(States::RidingMount))
		return NULL;

	return creatureRider->getMountedCreature();
}

// ======================================================================
// class PlaySkeletalAnimationAction: INLINES
// ======================================================================

inline const PlaySkeletalAnimationActionTemplate &PlaySkeletalAnimationAction::getOurTemplate() const
{
	return *NON_NULL(safe_cast<const PlaySkeletalAnimationActionTemplate*>(getPlaybackActionTemplate()));
}

// ======================================================================
// clsas PlaySkeletalAnimationAction: PUBLIC STATIC
// ======================================================================

void PlaySkeletalAnimationAction::install()
{
	InstallTimer const installTimer("PlaySkeletalAnimationAction::install");

	s_combatAnimationStateId = AnimationStateNameIdManager::createId(ConstCharCrcLowerString("combat"));
	s_pistolAnimationStateId = AnimationStateNameIdManager::createId(ConstCharCrcLowerString("pistol"));
	s_rifleAnimationStateId  = AnimationStateNameIdManager::createId(ConstCharCrcLowerString("rifle"));
}

// ======================================================================
// class PlaySkeletalAnimationAction: public member functions
// ======================================================================

PlaySkeletalAnimationAction::PlaySkeletalAnimationAction(const PlaySkeletalAnimationActionTemplate &actionTemplate) :
	PlaybackAction(&actionTemplate),
	m_initialized(false),
	m_animationId(-1),
	m_animationIsAdd(false),
	m_notification(new CallbackAnimationNotification()),
	m_initialAppearance(NULL)
{
	//-- Fetch reference for this instance.
	m_notification->fetch();
}

// ----------------------------------------------------------------------

PlaySkeletalAnimationAction::~PlaySkeletalAnimationAction()
{
	m_notification->release();
	m_notification = 0; //lint !e423 // memory leak // No, this is reference counted.
}

// ----------------------------------------------------------------------

void PlaySkeletalAnimationAction::cleanup(PlaybackScript &script)
{
	//-- Stop the animation if requested.
	if (m_initialized && (m_animationId >= 0))
	{
		const PlaySkeletalAnimationActionTemplate &actionTemplate = getOurTemplate();
		if (actionTemplate.getStopAnimationOnAbort())
		{
			//-- Get the object.
			Object *const object = script.getActor(actionTemplate.getActorIndex());
			if (!object)
				return;

			//-- Ensure we have a skeletal appearance.
			SkeletalAppearance2 *const appearance = dynamic_cast<SkeletalAppearance2*>(object->getAppearance());
			if (!appearance)
			{
				DEBUG_WARNING(true, ("PlaySkeletalAnimationAction: can't stop action, actor index %d (id = %s) is not skeletal.", actionTemplate.getActorIndex(), object->getNetworkId().getValueString().c_str()));
				return;
			}

			appearance->getAnimationResolver().stopPrimaryAction(m_animationId, m_animationIsAdd);
		}
	}
}

// ----------------------------------------------------------------------

bool PlaySkeletalAnimationAction::update(float deltaTime, PlaybackScript &script)
{
	UNREF(deltaTime);

	//-- Handle first-call initialization activities.
	if (!m_initialized)
	{
		m_initialized = true;
		return initialize(script);
	}
	
	//-- Check if animation action has completed.
	const PlaySkeletalAnimationActionTemplate &actionTemplate = getOurTemplate();

	// Retrieve the appearance.
	SkeletalAppearance2 *const appearance = getActorAppearance(script, actionTemplate.getActorIndex());
	if (!appearance)
	{
		DEBUG_WARNING(true, ("failed to get appearance for action actor [%d], ending PlaySkeletalAnimationAction.", actionTemplate.getActorIndex()));
		return false;
	}

	if(m_initialAppearance != appearance)
	{
		return false;
	}

	const bool animationComplete = appearance->getAnimationResolver().hasActionCompleted(m_animationId, m_animationIsAdd);
#if 0
	DEBUG_REPORT_LOG(animationComplete, ("Animation action [%d] completed.\n", m_animationId));
#endif
	
	//-- There is nothing left for the script to do when the animation is complete.  Return false when the animation is done.
	return !animationComplete;
}

// ----------------------------------------------------------------------

CallbackAnimationNotification *PlaySkeletalAnimationAction::fetchNotification()
{
	m_notification->fetch();
	return m_notification;
}

// ======================================================================
// class PlaySkeletalAnimationAction: private static member functions
// ======================================================================

SkeletalAppearance2 *PlaySkeletalAnimationAction::getActorAppearance(PlaybackScript &script, int actorIndex)
{
	//-- Get the object.
	Object *const object = script.getActor(actorIndex);
	if (!object)
		return 0;

	//-- Return the object's appearance.
	return dynamic_cast<SkeletalAppearance2*>(object->getAppearance());
}

// ======================================================================
// class PlaySkeletalAnimationAction: private member functions
// ======================================================================

bool PlaySkeletalAnimationAction::initialize(PlaybackScript &script)
{
	const PlaySkeletalAnimationActionTemplate &actionTemplate = getOurTemplate();

	//-- Get the object.
	Object *const object = script.getActor(actionTemplate.getActorIndex());
	if (!object)
	{
		DEBUG_WARNING(PlaybackScriptNamespace::s_verbosePlaybackScriptWarnings, ("PlaySkeletalAnimationAction::initialize() [%s] skipping, actor index [%d] is not set for script.", 
			script.getPlaybackScriptTemplateName(), actionTemplate.getActorIndex()));
		return false;
	}

	// Costumed characters aren't allowed to do Combat animations currently.
	CreatureObject * creatureObj = dynamic_cast<CreatureObject*>(object);
	if(creatureObj && creatureObj->isInCombat() && creatureObj->isUsingAlternateAppearance()) 
		return false;

	//-- Tell the notification about the object.  This allows the notification to keep a watcher for it and prevents any access to deleted data.
	m_notification->associateObject(object);

	//-- Ensure we have a skeletal appearance.
	SkeletalAppearance2 *const appearance = dynamic_cast<SkeletalAppearance2*>(object->getAppearance());
	if (!appearance)
	{
		DEBUG_WARNING(PlaybackScriptNamespace::s_verbosePlaybackScriptWarnings, ("PlaySkeletalAnimationAction::initialize() skipping, actor index [%d], object template=[%s] is not skeletal.", actionTemplate.getActorIndex(), object->getObjectTemplateName()));
		return false;
	}

	m_initialAppearance = appearance;

	//-- Retrieve the action name.
	const bool gsvResult = script.getStringVariable(actionTemplate.getActionNameVariable(), s_actionName);
	if (!gsvResult)
	{
#ifdef _DEBUG
		char buffer[5];
		ConvertTagToString(actionTemplate.getActionNameVariable(), buffer);
		DEBUG_WARNING(true, ("PlaySkeletalAnimationAction::initialize() skipping, script action variable [%s] not present.", buffer));
#endif
		return false;
	}

	//-- Retrieve the new posture.
	int newPosture = -1;
	if (actionTemplate.doPostureChange())
	{
		IGNORE_RETURN(script.getIntVariable(actionTemplate.getPostureVariable(), newPosture));
	}

	//-- Skip action if the action is an empty string.
	if (s_actionName.empty())
	{
		// Skip animation playback, do posture changes and use animation traversal logic.

		//-- Handle posture changes.
		if (actionTemplate.doPostureChange())
			handlePostureChange(*object, newPosture, false);

		// The action is done.
		return false;
	}

	//-- Get mount-related info.
	CreatureObject *const mountObject = getMountForObject(*object);
	bool const isActorRiding          = (mountObject != NULL);

	//-- Play actions.
	Object             *faceTrackTargetObject = 0;
	CreatureController *creatureController    = 0;

	if (actionTemplate.doFaceTracking() && !isActorRiding)
	{
		// Get the target object for face tracking.
		faceTrackTargetObject = script.getActor(actionTemplate.getFaceTrackTargetActorIndex());

		// Get the controller for the object that will face track.
		creatureController = dynamic_cast<CreatureController*>(object->getController());
		DEBUG_REPORT_LOG(!creatureController, ("PSAA: skipping face tracking on object [%s] since it doesn't have a CreatureController.\n", object->getNetworkId().getValueString().c_str()));
	}

	if (faceTrackTargetObject && creatureController)
	{
		//-- Tell the creature to play animation and face track the target.
		creatureController->playAnimationWithFaceTracking(CrcLowerString(s_actionName.c_str()), *faceTrackTargetObject, m_animationId, m_animationIsAdd, m_notification);
	}
	else
	{
		//-- Simply play the action on the character.
		appearance->getAnimationResolver().playAction(CrcLowerString(s_actionName.c_str()), m_animationId, m_animationIsAdd, m_notification);
	}

	//-- Handle posture change.
	if (actionTemplate.doPostureChange())
		handlePostureChange(*object, newPosture, true);

	//-- Handle combat moves that require the mount to orient 45 degrees left of target.
	//   We do this to allow the player to use right-handed-only attacks on the defender
	//   and still look reasonably good.
	if (isActorRiding)
	{
		// Only do the mount orientation if the rider/mount combo is not moving.
		CreatureController const *const riderController = dynamic_cast<CreatureController const*>(object->getController());
		bool const doMountOrient = (riderController != NULL) ? (riderController->getCurrentSpeed() <= 0.0f) : true;

		if (doMountOrient)
		{
			NOT_NULL(mountObject);
			makeMountOrientForCombatAsNecessary(script, actionTemplate, *object, *mountObject);
		}
	}

	int attackerActionNameCrc = 0;
	if (script.getIntVariable(actionTemplate.getSpecialAttackEffectVariable(), attackerActionNameCrc))
	{
		CombatEffectsManager::createSpecialAttackEffect( *object, attackerActionNameCrc );
	}

	//-- Instruct script to continue since we're in the process of playing an animation and we want to wait for it to complete.
	return true;
}

// ----------------------------------------------------------------------

void PlaySkeletalAnimationAction::handlePostureChange(Object &actor, int newPosture, bool skipTraversal) const
{
	//-- Get the creature controller.
	CreatureController *const controller = dynamic_cast<CreatureController*>(actor.getController());
	if (!controller)
	{
		DEBUG_WARNING(true, ("posture change can only occur on appearances with CreatureController-derived controllers."));
		return;
	}

	//-- If newPosture is negative, no posture change should occur.
	if (newPosture < 0)
	{
		// No posture change specified.
		return;
	}

	//-- Tell creature controller to change underlying looping animation state to
	//   the state implied by this (possibly different) posture.  If its a new state,
	//   no traversals will occur.
	const bool skipWithDelay = true;
	controller->changeAnimationStatePath(newPosture, skipTraversal, skipWithDelay);
	
	//-- Tell the creature object about its new state as specified by the server.
	CreatureObject *const creatureObject = safe_cast<CreatureObject*>(&actor);
	NOT_NULL(creatureObject);

#ifdef _DEBUG
	Postures::Enumerator posture = static_cast<Postures::Enumerator>(newPosture);

	CreatureInfo::PostureState postureState = CreatureInfo::PS_incapacitatedByPlayAnimationAction;
	bool setPostureState = false;

	switch (posture)
	{
		case Postures::Incapacitated:
			postureState    = CreatureInfo::PS_incapacitatedByPlayAnimationAction;
			setPostureState = true;
			break;

		case Postures::Dead:
			postureState    = CreatureInfo::PS_deadByPlayAnimationAction;
			setPostureState = true;
			break;
			
		default:
			// do nothing.
			break;
	}

	if (setPostureState)
		IGNORE_RETURN(CreatureInfo::setCreaturePostureState(creatureObject->getNetworkId(), postureState));
#endif

	creatureObject->setVisualPosture(static_cast<int8>(newPosture));
}

// ----------------------------------------------------------------------

void PlaySkeletalAnimationAction::makeMountOrientForCombatAsNecessary(PlaybackScript &script, PlaySkeletalAnimationActionTemplate const &actionTemplate, Object &riderObject, CreatureObject &mountObject)
{
	//-- Ensure the actor for this role is in the "attacker" slot.
	bool const animatingActorIsAttacker = (actionTemplate.getActorIndex() == cs_standardAttackerActorIndex);
	if (!animatingActorIsAttacker)
		return;

	//-- Ensure there is a valid "defender" target.
	Object const *const defenderObject = (script.getSupportedActorCount() >= 2) ? script.getActor(cs_standardDefenderActorIndex) : NULL;
	if (!defenderObject)
		return;

	//-- Ensure the rider's combat state path includes "combat" and doesn't include pistol or rifle.
	// ... first get CreatureController for rider.
	CreatureController const *riderCreatureController = dynamic_cast<CreatureController const*>(riderObject.getController());
	if (!riderCreatureController)
		return;

	// ... get the current AnimationStatePath for the rider.
	AnimationStatePath const *const riderPath = riderCreatureController->getCurrentAnimationStatePath();
	if (!riderPath)
		return;

	// ... check for presence of combat, pistol and rifle.
	bool hasCombat = false;
	bool hasPistol = false;
	bool hasRifle  = false;

	int const stateCount = riderPath->getPathLength();
	for (int stateIndex = 0; stateIndex < stateCount; ++stateIndex)
	{
		AnimationStateNameId const &stateNameId = riderPath->getState(stateIndex);
		if (stateNameId == s_combatAnimationStateId)
			hasCombat = true;
		else if (stateNameId == s_pistolAnimationStateId)
			hasPistol = true;
		else if (stateNameId == s_rifleAnimationStateId)
			hasRifle = true;
	}

	// ... we only apply mount orienting when in combat.
	if (!hasCombat)
		return;

	// ... we only apply mount orienting in combat when we are not using ranged combat.
	if (hasPistol || hasRifle)
		return;

	//-- Find a world position for the mount to face that is 45 degrees to the left of the defender.
	// ... get defender position relative to mount object space (mos).
	Vector const defenderPosition_mos = mountObject.rotateTranslate_w2o(defenderObject->getPosition_w());
	Vector defenderDirection_mos = defenderPosition_mos;
	if (!defenderDirection_mos.normalize())
		return;

	// ... calculate target mount-object-relative theta for correct mount heading.
	float const newMountTargetTheta_mos = defenderDirection_mos.theta() + cs_mountCombatYawAdjustmentRadians;
	float const radius = 5.0f;

	// ... do a simple local-space planar yaw around X-Z axis using newMountTargetTheta_mos as the angle of rotation.
	Vector const newMountTargetFacing_mos(radius * sin(newMountTargetTheta_mos), 0.0f, radius * cos(newMountTargetTheta_mos));

	// ... convert mount-object-relative target position to world space.
	Vector const newMountTargetFacing_w = mountObject.rotateTranslate_o2w(newMountTargetFacing_mos);

	//-- Get mount's CreatureController, tell it to face the new direction.
	CreatureController *const mountCreatureController = dynamic_cast<CreatureController*>(mountObject.getController());
	if (mountCreatureController)
		mountCreatureController->face(newMountTargetFacing_w);
}
// ----------------------------------------------------------------------

void PlaySkeletalAnimationAction::debugDump() const
{
	DEBUG_REPORT_LOG(true, ("PSAA:[BEGIN DUMP].\n"));
	DEBUG_REPORT_LOG(true, ("PSAA: initialized %d\n", m_initialized));
	DEBUG_REPORT_LOG(true, ("PSAA: animation id %d\n", m_animationId));
	DEBUG_REPORT_LOG(true, ("PSAA: is add: %d\n", m_animationIsAdd));
}

// ======================================================================
