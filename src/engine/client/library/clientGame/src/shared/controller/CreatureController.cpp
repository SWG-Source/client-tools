// ======================================================================
//
// CreatureController.cpp
// Portions Copyright 1999, Bootprint Entertainment, Inc.
// Portions Copyright 2001, 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/CreatureController.h"

#include "clientAnimation/PlaybackScript.h"
#include "clientAnimation/PlaybackScriptManager.h"
#include "clientGame/ClientCombatPlaybackManager.h"
#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientGameAppearanceEvents.h"
#include "clientGame/ClientEffectManager.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureInfo.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GamePlaybackScript.h"
#include "clientGame/LightsaberAppearance.h"
#include "clientGame/PlayerMusicManager.h"
#include "clientGame/PlayerObject.h"
#include "clientGame/ShipStation.h"
#include "clientSkeletalAnimation/AnimationPostureMapper.h"
#include "clientSkeletalAnimation/AnimationPriorityMap.h"
#include "clientSkeletalAnimation/AnimationStateNameId.h"
#include "clientSkeletalAnimation/AnimationStateNameIdManager.h"
#include "clientSkeletalAnimation/AnimationStatePath.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/StateHierarchyAnimationController.h"
#include "clientSkeletalAnimation/TransformAnimationResolver.h"
#include "clientUserInterface/CuiActionManager.h"
#include "clientUserInterface/CuiActions.h"
#include "clientUserInterface/CuiCombatManager.h"
#include "clientUserInterface/CuiMenuInfoTypes.h"
#include "clientUserInterface/CuiModifierManager.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiRadialMenuManager.h"
#include "clientUserInterface/CuiRecipeManager.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/Footprint.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedGame/GameObjectTypes.h"
#include "sharedGame/SharedCreatureObjectTemplate.h"
#include "sharedMath/Quaternion.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedNetworkMessages/MessageQueueNetworkId.h"
#include "sharedNetworkMessages/MessageQueuePosture.h"
#include "sharedNetworkMessages/MessageQueueSitOnObject.h"
#include "sharedNetworkMessages/MessageQueueSlowDownEffect.h"
#include "sharedNetworkMessages/MessageQueueString.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedTerrain/TerrainObject.h"
#include "sharedTerrain/WaterTypeManager.h"
#include "swgSharedUtility/Postures.def"
#include "swgSharedUtility/States.def"

#include <list>
#include <cstdio>

// ======================================================================

const Tag TAG_ATRG = TAG(A,T,R,G);

/// Minimum yaw rotation rate (in radians per second) required for rotation animations.
const float cs_minAnimationYawRate = 10.0f * PI_OVER_180;

/// The time duration to continue turning in the last direction of turn after a turn appears to have stopped.
const float cs_continueTurningTime = 0.5f;

// ======================================================================

/*namespace PlayerCreatureControllerNamespace
{
#if PRODUCTION == 0
	extern bool s_usePlayerServerSpeed;
#endif
	extern float ms_playerServerMovementSpeed;
}
using namespace PlayerCreatureControllerNamespace;*/

namespace CreatureControllerNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef std::vector<const PlaybackScript *>  ConstPlaybackScriptVector;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	// If true, logs changes that affect the animation state.
	// @todo move these into ConfigClientGame.
	bool s_logAnimationStatePathActivity;
	bool s_printTargetSource;

	AnimationStateNameId       s_ridingStateId;
	AnimationStateNameId       s_swimStateId;
	AnimationStateNameId       s_swimIncapacitatedStateId;
	int                        s_locomotionPriority;

	ConstPlaybackScriptVector  s_playbackScripts;

	namespace CommandNames
	{
		const std::string stand           = "stand";
		const std::string sit             = "sit";
		const std::string kneel           = "kneel";
		const std::string prone           = "prone";
		const std::string setMoodInternal = "setMoodInternal";

		const std::string combatAttitudeEvasive    = "combatAttitudeEvasive";
		const std::string combatAttitudeNeutral    = "combatAttitudeNeutral";
		const std::string combatAttitudeAggressive = "combatAttitudeAggressive";

	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	inline const CreatureObject* getCreatureObject (const Object* const object)
	{
		NOT_NULL (object);
		return safe_cast<const CreatureObject*> (object);
	}

	inline CreatureObject* getCreatureObject (Object* const object)
	{
		NOT_NULL (object);
		return safe_cast<CreatureObject*> (object);
	}
}

using namespace CreatureControllerNamespace;

// ======================================================================

AnimationStateNameId CreatureController::ms_combatStateId;
AnimationStateNameId CreatureController::ms_aimedStateId;
AnimationStateNameId CreatureController::ms_sittingOnChairStateId;

// ======================================================================
// class CreatureController: pubic static member functions
// ======================================================================

void CreatureController::install ()
{
	InstallTimer const installTimer("CreatureController::install ");

	//-- Lookup animation state IDs by name.
	ms_combatStateId           = AnimationStateNameIdManager::createId(ConstCharCrcLowerString("combat"));
	ms_aimedStateId            = AnimationStateNameIdManager::createId(ConstCharCrcLowerString("aimed"));
	ms_sittingOnChairStateId   = AnimationStateNameIdManager::createId(ConstCharCrcLowerString("sitting_chair"));
	s_ridingStateId            = AnimationStateNameIdManager::createId(ConstCharCrcLowerString("riding"));
	s_swimStateId              = AnimationStateNameIdManager::createId(ConstCharCrcLowerString("swimming"));
	s_swimIncapacitatedStateId = AnimationStateNameIdManager::createId(ConstCharCrcLowerString("incapacitated_water"));

	//-- Lookup animation priority values.
	s_locomotionPriority = AnimationPriorityMap::getPriorityValue(ConstCharCrcLowerString("locomotion"));

	DebugFlags::registerFlag(s_logAnimationStatePathActivity,  "ClientGame/CreatureController", "logAnimationStatePathActivity");
	DebugFlags::registerFlag(s_printTargetSource, "ClientGame/CreatureController", "printTargetSource");
}

// ======================================================================
// class CreatureController: public member functions
// ======================================================================

CreatureController::CreatureController (CreatureObject* newOwner) :
	ClientController (newOwner),
	m_appearanceIsSkeletal (false),
	m_animationControllerInitialized (false),
	m_currentAnimationStatePath (new AnimationStatePath ()),
	m_previousAnimationStatePath (new AnimationStatePath ()),
	m_useHeldItemState (false),
	m_heldItemStateId (new AnimationStateNameId),
	m_overrideAnimationTargetWatcher (0),
	m_isOverrideTargetForCombat (false),
	m_suppressStandRequest (false),
	m_face (false),
	m_facePosition_w (),
	m_previousTheta_w (0.0f),
	m_previousYawDirection (0),
	m_continueTurningTimer (0.0f),
	m_isFirstAlter (true),
	m_faceTracking (false),
	m_faceTrackingTarget (new Watcher<Object>(0)),
	m_faceTrackingAnimationId (-1),
	m_faceTrackingAnimationIsAdd (false),
	m_isSwimming (false),
	m_inCombatState (false)
{
}

//-------------------------------------------------------------------

CreatureController::~CreatureController (void)
{
	delete m_faceTrackingTarget;
	delete m_overrideAnimationTargetWatcher;
	delete m_heldItemStateId;
	delete m_currentAnimationStatePath;
	delete m_previousAnimationStatePath;
}

//-------------------------------------------------------------------

void CreatureController::forceVisualsIntoCombat()
{
	setControllerToCurrentState(true);
}
	
//-------------------------------------------------------------------

float CreatureController::realAlter (float deltaTime)
{
	NP_PROFILER_AUTO_BLOCK_DEFINE ("CreatureController::realAlter");

	//-- Handle face tracking.
	if (m_faceTracking)
	{
		handleFaceTracking ();
	}

	//-- Get creature info.
	CreatureObject* const creatureObject         = safe_cast<CreatureObject*> (getOwner ());
	bool const            isRidingMount          = creatureObject ? creatureObject->isRidingMount () : false;
	CreatureObject* const movementCreatureObject = isRidingMount ? NON_NULL(creatureObject)->getMountedCreature() : creatureObject;

	if (!movementCreatureObject || !creatureObject)
	{
		DEBUG_WARNING (true, ("CreatureController::realAlter (): skipping rest of alter because owner is NULL, chaining up to parent class."));
		return ClientController::realAlter (deltaTime);
	}

	//--  face target if necessary
	if (m_face)
	{
		const Vector facing_o = movementCreatureObject->rotateTranslate_w2o (m_facePosition_w);

		m_face = getCurrentSpeed () == 0.f
					&& fabs (facing_o.theta ()) > convertDegreesToRadians (2.f)
					&& !creatureObject->isRidingMount();

		if (m_face)
		{
			const float turnRate = movementCreatureObject->getMaximumTurnRate (0.f);
			const float maximumYawThisFrame = convertDegreesToRadians (turnRate) * deltaTime;
			const float yaw = clamp (-maximumYawThisFrame, facing_o.theta (), maximumYawThisFrame);
			movementCreatureObject->yaw_o (yaw);
		}
	}

	float baseAlterResult = ClientController::realAlter (deltaTime);
	UNREF (baseAlterResult);

	//-- initialize controller if necessary
	if (!m_animationControllerInitialized)
	{
		initializeAnimationController ();
		m_animationControllerInitialized = true;
	}

	//-- Get the animation target.
	Object* animationTarget = 0;
	bool    inCombatState   = false;

	getAnimationTarget (animationTarget, inCombatState);

	//-- set the animation controller to the desired state
	setControllerToCurrentState (inCombatState);

	//-- set appearance target.
	setAppearanceTarget (animationTarget);

	//-- If combat state has changed since last frame, handle special
	//   processing of owner's equipment (e.g. turn on/off lightsabers.).
	if ((inCombatState && !m_inCombatState) || (!inCombatState && m_inCombatState))
	{
		processCombatStateChange(inCombatState);
		m_inCombatState = inCombatState;
	}

	//-- Apply animation-driven locomotion that has occurred since last frame.
	if (shouldApplyAnimationDrivenLocomotion())
		applyAnimationDrivenLocomotion(deltaTime);

	//-- Clear the override animation target.
	if (m_overrideAnimationTargetWatcher)
		*m_overrideAnimationTargetWatcher = 0;

	//-- Handle swimming.
	bool   onTerrain;
	Vector terrainNormal;
	float terrainHeight = 0.0f;
	float waterHeight   = 0.0f;
	bool isBurning = false;

	const bool terrainCalculated = calculateTerrainAndSwimmingInfo(onTerrain, terrainNormal, m_isSwimming, terrainHeight, waterHeight, isBurning);
	bool const isHoveringVehicle = (creatureObject->getGameObjectType () == SharedObjectTemplate::GOT_vehicle_hover || creatureObject->getGameObjectType() == SharedObjectTemplate::GOT_vehicle_hover_ai);

	//@todo: this snap to terrain needs to happen for all creatures, not just vehicles
	if(terrainCalculated)
	{
		const Vector & position = movementCreatureObject->getPosition_w ();
		if(isHoveringVehicle)
		{
			if (m_isSwimming)
				movementCreatureObject->move_p (Vector::unitY * (waterHeight - position.y));
			else
				movementCreatureObject->move_p (Vector::unitY * (terrainHeight - position.y));
		}
		else 
		{
			Footprint *const footprint = movementCreatureObject->getFootprint();
			if(footprint)
			{
				if(isBurning) // make everything but vehicles walk on lava
				{
					m_isSwimming = false;
					footprint->setSwimHeight(movementCreatureObject->getSwimHeight() * 0.2f);
					movementCreatureObject->move_p(Vector::unitY * (waterHeight - position.y));
				}
				else
				{
					footprint->setSwimHeight(movementCreatureObject->getSwimHeight());	
				}
			}
		}
	}

	if (!isRidingMount)
	{
		//-- align to terrain
		const SharedCreatureObjectTemplate*	const sharedCreatureObjectTemplate = safe_cast<const SharedCreatureObjectTemplate*> (creatureObject->getObjectTemplate ());
		if (sharedCreatureObjectTemplate)
		{
			const Transform& startTransform = creatureObject->getTransform_o2w ();

			Object const * const containedByObject = ContainerInterface::getContainedByObject(*creatureObject);

			Vector up_w;
			if (containedByObject)
				up_w = containedByObject->getObjectFrameJ_w();				
			else
				up_w = Vector::unitY;

			Transform endTransform = startTransform;
			Vector k = endTransform.getLocalFrameK_p ();
			Vector j = up_w;
			Vector i = j.cross (k);
			i.normalize();
			k = i.cross (j);
			k.normalize();
			endTransform.setLocalFrameIJK_p (i, j, k);

			//-- Align to terrain when applicable.
			bool shouldAlignToTerrain = false;

			if (!m_isSwimming && !isBurning)
			{
				// Mounted non-vehicular creatures always align to terrain.
				if (!isHoveringVehicle && creatureObject->getState(States::MountedCreature))
				{
					shouldAlignToTerrain = true;
				}
				else if (terrainCalculated) //-- we align to terrain OR floor
				{
					// If we're on the terrain, use posture to determine whether we align to terrain.
					shouldAlignToTerrain = creatureObject->getAlignToTerrain();
				}
			}

			if (shouldAlignToTerrain)
			{
				Vector k2 = startTransform.getLocalFrameK_p ();
				Vector i2 = terrainNormal.cross (k2);
				i2.normalize();
				k2 = i2.cross (terrainNormal);
				k2.normalize();
				endTransform.setLocalFrameIJK_p (i2, terrainNormal, k2);
			}

			if (!startTransform.approximates(endTransform, 0.0005f))
			{
				if (startTransform.approximates(endTransform, 0.005f))
					creatureObject->setTransform_o2w(endTransform);
				else
				{
					const Quaternion startOrientation (startTransform);
					const Quaternion endOrientation (endTransform);
					const Quaternion orientation (startOrientation.slerp (endOrientation, 0.25f));

					Transform transform;
					orientation.getTransform (&transform);
					transform.reorthonormalize ();
					transform.setPosition_p (startTransform.getPosition_p ());

					creatureObject->setTransform_o2w (transform);
				}
			}
		}

		//-- Capture yaw changes and report to appearance.
		reportYawStateToAppearance (deltaTime);

		// clientside burning effect
		if (isBurning && (creatureObject->isPlayer() || creatureObject->isBeast()))
		{
			const unsigned long currentTimeMs = Clock::timeMs();
			const unsigned long lastWaterDamageTimeMs = creatureObject->getLastWaterDamageTimeMs();
			const unsigned long damageIntervalMs = WaterTypeManager::getDamageInterval(TGWT_lava) * 1000;
			if(currentTimeMs - lastWaterDamageTimeMs > damageIntervalMs)
			{
				static const ConstCharCrcLowerString c("clienteffect/lava_player_burning.cef");
				Object* obj = safe_cast<Object*> (getOwner());
				if(obj)
				{
					creatureObject->setLastWaterDamageTimeMs(currentTimeMs);
					ClientEffectManager::playClientEffect(c, obj, CrcLowerString::empty);
				}
			}
		}

	}

	if (isRidingMount)
	{
		//-- Mounts: update the collision property extent for the rider.
		//   Since the rider is removed from CollisionWorld, the extents
		//   on the property don't get updated.  The validity of these
		//   extents are important for line of sight check, though.
		CollisionProperty *const collisionProperty = creatureObject->getCollisionProperty();
		if (collisionProperty)
			collisionProperty->updateExtents();

		//-- Sanity check: ensure rider's objectToParent is identity.
		Transform const &riderTransform_o2p = creatureObject->getTransform_o2p();
		if (riderTransform_o2p != Transform::identity)
		{
			// Report the incident.
			Vector const riderPosition_p = riderTransform_o2p.getPosition_p();
			DEBUG_WARNING(true, ("rider id=[%s] had o2p set to non-identity with position set to (%g,%g,%g), something is breaking the rider's transform.",
				creatureObject->getNetworkId().getValueString().c_str(),
				riderPosition_p.x, riderPosition_p.y, riderPosition_p.z));

			// Fix up the busted transform.
			// @todo -TRF- add a notification to catch exactly what is modifying this.
			if (creatureObject->getMountedCreature() != NULL)
				creatureObject->setTransform_o2p(Transform::identity);
		}
	}

	//-- Keep track of the fact that we've already run through the alter at least once.
	m_isFirstAlter = false;

	return AlterResult::cms_alterNextFrame;
}

void CreatureController::getFirstCombatTargetObject (const CreatureObject& creature, Object*& targetObject, bool &hasCombatTarget)
{
	if (creature.getCombatTarget() != CachedNetworkId::cms_invalid)
	{
		targetObject = creature.getCombatTarget().getObject ();
		hasCombatTarget = true;
	}
	else
	{
		targetObject = 0;
		hasCombatTarget = false;
	}
}

// ----------------------------------------------------------------------

void CreatureController::initializeAnimationController ()
{
	//-- determine which type of animation controller we should use
	Appearance *const baseAppearance = getOwner ()->getAppearance ();
	if (!baseAppearance)
		return;

	SkeletalAppearance2 *const skeletalAppearance = baseAppearance->asSkeletalAppearance2();
	if (!skeletalAppearance)
		return;

	//-- we now know appearance is skeletal.  Keep track of this to avoid per-frame dynamic casts in release.
	m_appearanceIsSkeletal = true;
}

// ----------------------------------------------------------------------

void CreatureController::determineAnimationStatePath (const CreatureObject &creature, AnimationStatePath &statePath, bool inCombat, int serverPostureOverride) const
{
	//-- Build current path based on the creature's environment.
	// Clear the path.
	statePath.clearPath ();

	// All paths start at the root.
	statePath.appendState (AnimationStateNameIdManager::getRootId ());

	// Check for held item class.
	if (m_useHeldItemState)
		statePath.appendState (*m_heldItemStateId);

	// Check for combat.
	if (inCombat)
		statePath.appendState (ms_combatStateId);

	// Check for posture.
	const int serverPostureToUse = (serverPostureOverride >= 0) ? serverPostureOverride : static_cast<int>(creature.getVisualPosture ());

	bool isRiding = false;
	if ((serverPostureToUse == Postures::Upright) && (creature.isRidingMount () || 
		(creature.getShipStation() != ShipStation::ShipStation_None && (creature.getShipStation() < ShipStation::ShipStation_Gunner_First || creature.getShipStation() > ShipStation::ShipStation_Gunner_Last))
		))
	{
		// Player is riding a mount.
		isRiding = true;
		statePath.appendState (s_ridingStateId);
	}
	else if ((serverPostureToUse == Postures::Sitting) && creature.isSittingOnObject ())
	{
		//-- Handle special-case sitting in chair posture.  I have to handle Postures::Sitting mapping to animation "sitting_ground" or "sitting_chair".
		//   This is a pile of crap because I can no longer 100% data drive animation posture selection in any meaningful way without going
		//   into overkill design for this mechanism.
		statePath.appendState (ms_sittingOnChairStateId);
	}
	else if ((m_isSwimming) && !creature.isRidingMount() && creature.isInWorld())
	{
		// Handle swimming posture.
		const AnimationStateNameId *swimStateId;

		switch (serverPostureToUse)
		{
			case Postures::Incapacitated:
			case Postures::KnockedDown:
			case Postures::Dead:
				swimStateId = &s_swimIncapacitatedStateId;
				break;

			default:
				swimStateId = &s_swimStateId;
				break;

		}
		statePath.appendState (*swimStateId);
	}
	else
	{
		// Map the creature's posture to an animation state path component.
		AnimationStateNameId  postureStateId;

		if (AnimationPostureMapper::mapPosture (serverPostureToUse, postureStateId))
		{
			// The animation posture mapper has a state for the current posture.  Some postures
			// map to no addition to the state path.
			statePath.appendState (postureStateId);
		}
	}

	// Append aimed state if applicable.
	// -TRF- if game states are used to control more than just aiming, create a new AnimationGameStateMap class to handle this.
	if (creature.isAiming() && !isRiding)
		statePath.appendState (ms_aimedStateId);	
}

// ----------------------------------------------------------------------

void CreatureController::setControllerToCurrentState (bool inCombat, bool forceState)
{
	if (!m_appearanceIsSkeletal)
		return;

	//-- Get the animation state path for the current environment of this object.
	CreatureObject& creature = *getCreatureObject (getOwner ());
	determineAnimationStatePath (creature, *m_currentAnimationStatePath, inCombat);

	//-- Only set new state if it changed.
	if (*m_currentAnimationStatePath != *m_previousAnimationStatePath || forceState)
	{
		DEBUG_REPORT_LOG(s_logAnimationStatePathActivity && isOwnerAnimationDebuggerTarget(), ("CC: changing animation state path to [%s].\n", m_currentAnimationStatePath->getPathString().c_str()));

		//-- Set the current state as the destination state.
		Appearance *const          baseAppearance = creature.getAppearance();
		SkeletalAppearance2 *const appearance     = (baseAppearance != NULL) ? baseAppearance->asSkeletalAppearance2() : NULL;
		NOT_NULL(appearance);

		appearance->getAnimationResolver ().setDestinationState (*m_currentAnimationStatePath);

		//-- Save new state.
		m_currentAnimationStatePath->copyTo(*m_previousAnimationStatePath);

		//-- Stop suppressing stand requests any time anything in the state path changes.
		//   NOTE: this could be moved to CreatureObject and cleared any time we change the posture.
		m_suppressStandRequest = false;
	}
	else
	{
		DEBUG_REPORT_LOG(s_logAnimationStatePathActivity && isOwnerAnimationDebuggerTarget(), ("CC: animation state path already set for [%s].\n", m_currentAnimationStatePath->getPathString().c_str()));
	}
}

// ----------------------------------------------------------------------

void CreatureController::setAppearanceTarget (const Object *target)
{
	if (!m_appearanceIsSkeletal)
		return;

	CreatureObject* creature = getCreatureObject (getOwner ());

	//-- Set appearance's target object.
	SkeletalAppearance2 *const appearance = safe_cast<SkeletalAppearance2*> (creature->getAppearance ());
	NOT_NULL(appearance);

	appearance->setTargetObject (target);
}


// ----------------------------------------------------------------------
/**
 * Modify the owner Object's localToParent transform due to animation-driven
 * rotation and translation.
 *
 * Animations may drive a creature object's rotation and translation.  This
 * is the function that applies the rotation and translation.
 */

void CreatureController::applyAnimationDrivenLocomotion (float elapsedTime)
{
	if (!m_appearanceIsSkeletal)
		return;

	//-- get the animation controller
	Object *const object = getOwner();
	if (!object)
		return;

	Appearance          *const baseAppearance = object->getAppearance ();
	SkeletalAppearance2 *const appearance     = (baseAppearance != NULL) ? baseAppearance->asSkeletalAppearance2() : NULL;
	if (!appearance)
		return;

	//-- get locomotion
	Quaternion  locomotionRotation(Quaternion::identity);
	Vector      locomotionTranslation(Vector::zero);

	appearance->getObjectLocomotion(locomotionRotation, locomotionTranslation, elapsedTime);
	/*if(object == Game::getPlayerCreature())
	{
		Vector originalPos = object->getPosition_w();
		Vector alteredPos = originalPos + locomotionTranslation;

		Vector finalVec = alteredPos - originalPos;

		float movementSpeed = finalVec.magnitude();
		float validMovementSpeed = ms_playerServerMovementSpeed * (elapsedTime * 1.15f); // Give 15% leeway since these values can be verryyyyy close as the animation speed is based on the full player's speed.
		if(movementSpeed > validMovementSpeed)
			return;
	}*/

	if (locomotionTranslation.magnitude() > std::numeric_limits<float>::epsilon())
	{
		//-- apply rotation
		// convert to Transform
		Transform animationRotationTransform(Transform::IF_none);
		locomotionRotation.getTransform(&animationRotationTransform);

		// create resultant Transform
		Transform objectToParent = object->getTransform_o2p();
		Transform rotatedObjectToParent(Transform::IF_none);
		rotatedObjectToParent.multiply(objectToParent, animationRotationTransform);

		// apply rotation transform to Object
		object->setTransform_o2p(rotatedObjectToParent);

		//-- apply translation (translation is relative to the Object's frame of reference, not the parent's)
		object->move_o(locomotionTranslation);
	}

}

//----------------------------------------------------------------------

void CreatureController::handleMessage (const int message, const float value, const MessageQueue::Data* const data, const uint32 flags)
{
	switch (message)
	{

		case CM_jump:
			{
				// Ignore jump animation message if this is the current client's player.
				// Player's own jump animation has already played immediately on client-side when jump was executed.
				if (getOwner() != Game::getPlayer())
				{
					MessageQueueString * const msg = new MessageQueueString ("jump");

					CreatureObject * const creatureOwner = dynamic_cast<CreatureObject*>(getOwner());
					if(creatureOwner && creatureOwner->isRidingMount())
					{
						CreatureObject * const mountCreature = creatureOwner->getMountedCreature();
						CreatureController * const mountController = mountCreature ? dynamic_cast<CreatureController *>(mountCreature->getController()) : NULL;

						if(mountController)
						{
							mountController->appendMessage(CM_animationAction, 0, msg,
								GameControllerMessageFlags::SEND |
								GameControllerMessageFlags::RELIABLE |
								GameControllerMessageFlags::DEST_ALL_CLIENT);

							return;
						}

					}

					appendMessage(CM_animationAction, 0, msg,
						  GameControllerMessageFlags::SEND |
						  GameControllerMessageFlags::RELIABLE |
						  GameControllerMessageFlags::DEST_ALL_CLIENT);
				}
			}
			break;

		case CM_setPosture:
			{
				const MessageQueuePosture * const msg = NON_NULL (safe_cast<const MessageQueuePosture *>(data));
				if (msg != NULL)
				{
					CreatureObject * ownerObject = safe_cast<CreatureObject *>(getOwner ());
					if (ownerObject != NULL)
					{
						//-- Change the visual posture.
						const Postures::Enumerator posture = static_cast<Postures::Enumerator>(msg->getPosture());

						if (ownerObject->getVisualPosture() != posture)
						{
							//-- First stop playing any client playback scripts --- one of them might be planning on changing the posture
							//   to what the server told us the posture was back then.
							ClientCombatPlaybackManager::stopAllPlaybackScriptsForObject(*ownerObject, posture);

							ownerObject->setVisualPosture(posture);

#ifdef _DEBUG
							CreatureInfo::PostureState postureState = CreatureInfo::PS_incapacitatedBySetPostureMessage;
							bool setPostureState = false;

							switch (posture)
							{
								case Postures::Incapacitated:
									postureState    = CreatureInfo::PS_incapacitatedBySetPostureMessage;
									setPostureState = true;
									break;

								case Postures::Dead:
									postureState    = CreatureInfo::PS_deadBySetPostureMessage;
									setPostureState = true;
									break;

								default:
									// do nothing.
									break;
							}

							if (setPostureState)
								IGNORE_RETURN(CreatureInfo::setCreaturePostureState(ownerObject->getNetworkId(), postureState));
#endif
						}
					}
				}
			}
			break;

		case CM_sitOnObject:
			{
				const MessageQueueSitOnObject *const msg = safe_cast<const MessageQueueSitOnObject*> (data);
				if (msg != NULL)
				{
					//-- Get this CreatureObject.
					CreatureObject *const creatureObject = safe_cast<CreatureObject*>(getOwner());
					NOT_NULL(creatureObject);

					//-- Get the chair object specified in the message.
					TangibleObject *const chairObject = TangibleObject::findClosestInRangeChair(msg->getChairCellId(), msg->getChairPosition_p());
					if (!chairObject)
					{
						//-- Sit the creature on the ground.
						creatureObject->setVisualPosture(Postures::Sitting);
						DEBUG_WARNING(true, ("CM_sitOnObject: request for id=[%s],template=[%s] denied because no appropriate chair could be found with cell id=[%s].", creatureObject->getNetworkId().getValueString().c_str(), creatureObject->getObjectTemplateName(), msg->getChairCellId().getValueString().c_str()));
						return;
					}

					//-- Tell the creature object we're sitting on a chair.
					creatureObject->setSittingOnObject(true);
					GamePlaybackScript::sitCreatureOnChair(*creatureObject, *chairObject, 0);
				}
			}
			break;

		case CM_musicFlourish:
			{
				const MessageQueueGenericValueType<int> * const msg = safe_cast<const MessageQueueGenericValueType<int> *>(data);
				if (msg)
				{
					CreatureObject *creatureObject = dynamic_cast<CreatureObject *>(getOwner());

					if (creatureObject != NULL)
					{
						PlayerMusicManager::queueFlourish(creatureObject, msg->getValue());
					}
				}
			}
			break;

		case CM_modData:
			{
				const MessageQueueGenericValueType<std::pair<uint32, float> > * const msg = safe_cast<const MessageQueueGenericValueType<std::pair<uint32, float> > *>(data);
				if (msg)
				{
					CuiModifierManager::addModifier(msg->getValue().first, msg->getValue().second);
				}
			}
			break;

		case CM_openCustomizationWindow:
			{
				const MessageQueueGenericValueType<std::pair<NetworkId, std::string> > * const msg = safe_cast<const MessageQueueGenericValueType<std::pair<NetworkId, std::string> > *>(data);
				if (msg)
				{
					std::string value;
					value.append(msg->getValue().first.getValueString());
					value.append(" ");
					value.append(msg->getValue().second);
					IGNORE_RETURN(CuiActionManager::performAction(CuiActions::openCustomizationWindow, Unicode::narrowToWide(value)));
				}
			}
			break;

		case CM_cancelMod:
			{
				const MessageQueueGenericValueType<uint32> * const msg = safe_cast<const MessageQueueGenericValueType<uint32> *>(data);
				if (msg)
				{
					CuiModifierManager::removeModifier(msg->getValue());
				}
			}
			break;

		case CM_slowDownEffect:
			{
				const MessageQueueSlowDownEffect * const msg = safe_cast<const MessageQueueSlowDownEffect * const>(data);
				if (msg)
				{
					CreatureObject * owner = dynamic_cast<CreatureObject *>(getOwner());
					if (owner != NULL)
					{
						Object * object = NetworkIdManager::getObjectById(msg->getTarget());
						if (object != NULL && object->asClientObject()->asTangibleObject() != NULL)
						{
							TangibleObject * target = object->asClientObject()->asTangibleObject();
							owner->addSlowDownEffect(*target, msg->getConeLength(), msg->getConeAngle(), msg->getSlopeAngle(), msg->getExpireTime());
						}
					}
				}
			}
			break;

		case CM_removeSlowDownEffectProxy:
			{
				CreatureObject * owner = dynamic_cast<CreatureObject *>(getOwner());
				if (owner != NULL)
					owner->removeSlowDownEffect();
			}
			break;

		case CM_openRatingWindow:
			{
				const MessageQueueGenericValueType< std::pair<std::string, std::string> > * const msg = safe_cast<const MessageQueueGenericValueType< std::pair<std::string, std::string> > *>(data);
				if(msg)
				{
					std::string value = msg->getValue().first;
					value.append("|", 1);
					value += msg->getValue().second;
					IGNORE_RETURN(CuiActionManager::performAction(CuiActions::rating, Unicode::narrowToWide(value)));
				}
			}
			break;

		case CM_openRecipe:
			{
				const MessageQueueGenericValueType< NetworkId > * const msg = safe_cast<const MessageQueueGenericValueType< NetworkId > *>(data);
				if(msg)
				{
					CuiRecipeManager::setRecipeObject(msg->getValue());
					IGNORE_RETURN(CuiActionManager::performAction(CuiActions::questBuilder, Unicode::emptyString));
				}
			}
			break;

		default:
			ClientController::handleMessage (message, value, data, flags);
			break;
	}
}

// ----------------------------------------------------------------------

void CreatureController::requestStand ()
{
	CreatureObject * ownerObject = safe_cast<CreatureObject *>(getOwner ());

	//-- Check if we're already standing.
	if (ownerObject->getVisualPosture () == Postures::Upright)
	{
		// nothing to do.
		m_suppressStandRequest = false;
		return;
	}

	//-- Check if we've already requested standing.
	if (m_suppressStandRequest)
	{
		// ignore request.
		return;
	}

	//-- Request standing.
	ownerObject->requestServerPostureChange (Postures::Upright);
	m_suppressStandRequest = true;
}

// ----------------------------------------------------------------------

void CreatureController::requestServerPostureChange (Postures::Enumerator posture)
{
	const std::string * command = NULL;
	switch (posture)
	{
		case Postures::Upright:
			command = &CommandNames::stand;
			break;
		case Postures::Crouched:
			command = &CommandNames::kneel;
			break;
		case Postures::Prone:
			command = &CommandNames::prone;
			break;
		case Postures::Sitting:
			command = &CommandNames::sit;
			break;
		default:
			break;
	}

	if (command != NULL)
		IGNORE_RETURN(ClientCommandQueue::enqueueCommand(*command, NetworkId::cms_invalid, Unicode::String()));
} //lint !e1762 // function could be made const // This is a logically non-const operation.

//----------------------------------------------------------------------

void CreatureController::requestServerAttitudeChange    (int8 attitude)
{
	if (attitude != States::CombatAttitudeEvasive &&
		attitude != States::CombatAttitudeNormal &&
		attitude != States::CombatAttitudeAggressive)
	{
		WARNING (true, ("setAttitude bad attitude %d", attitude));
		return;
	}

	const std::string * command = 0;

	switch (attitude)
	{
	case States::CombatAttitudeEvasive:
		command = &CommandNames::combatAttitudeEvasive;
		break;
	case States::CombatAttitudeNormal:
		command = &CommandNames::combatAttitudeNeutral;
		break;
	case States::CombatAttitudeAggressive:
		command = &CommandNames::combatAttitudeAggressive;
		break;
	default:
		{
			DEBUG_WARNING(true, ("unexpected attitude specified [%d].", static_cast<int>(attitude)));
			return;
		}
	}

	IGNORE_RETURN(ClientCommandQueue::enqueueCommand(*command, NetworkId::cms_invalid, Unicode::String ()));
} //lint !e1762 // member function could be made const // this is a logically non-const operation.

//----------------------------------------------------------------------

void CreatureController::setLookAtTarget (const NetworkId & id)
{
	MessageQueueNetworkId * const msg = new MessageQueueNetworkId (id);
	appendMessage (static_cast<int>(CM_clientLookAtTarget), 0.0f, msg,
		GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_SERVER);
} //lint !e429 // msg has not been freed or returned // it's okay, message queue owns.

//----------------------------------------------------------------------

void CreatureController::setIntendedTarget (const NetworkId & id)
{
	MessageQueueNetworkId * const msg = new MessageQueueNetworkId (id);
	appendMessage (static_cast<int>(CM_clientIntendedTarget), 0.0f, msg,
		GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_SERVER);
} //lint !e429 // msg has not been freed or returned // it's okay, message queue owns.

// ----------------------------------------------------------------------

void CreatureController::getAnimationTarget (Object*& targetObject, bool& isCombatTarget) const
{
	//-- Call the real worker function.
	getAnimationTargetInternal (targetObject, isCombatTarget);

	//-- Only both checking the rest if our target is combat related and if we're going to disable combat
	//   animation paths on the rider of vehicles.
	if (isCombatTarget && ConfigClientGame::getDisableVehicleRiderCombatAnimationState())
	{
		//-- Set isCombatTarget to false if we're a rider of a vehicle.  We don't support vehicle riders in combat.

		// Get the potential rider.
		Object const *const ownerObject = getOwner();
		ClientObject const *const ownerAsClient = ownerObject ? ownerObject->asClientObject() : NULL;
		CreatureObject const *const ownerAsCreature = ownerAsClient ? ownerAsClient->asCreatureObject() : NULL;

		// Check if we're riding.
		if (ownerAsCreature && ownerAsCreature->getState(States::RidingMount))
		{
			// Get the mount/vehicle.
			CreatureObject const *const vehicle = ownerAsCreature->getMountedCreature();

			// If it's a vehicle, indicate that we don't have a combat target.  This will prevent us from
			// later moving into a combat animation state path.  Riders of non-mount vehicles are not
			// allowed to participate in combat, so getting them to raise weapons in the combat state is
			// a glimmer of false hope we need to stomp out.
			if (vehicle && GameObjectTypes::isTypeOf(vehicle->getGameObjectType(), static_cast<int>(SharedObjectTemplate::GOT_vehicle)))
				isCombatTarget = false;
		}
	}
}

// ----------------------------------------------------------------------
/**
 * Retrieve the animation target and whether the target is a combat target.
 *
 * Knowing whether the target is from a combat target source allows the
 * controller to determine if it should be in the combat portion of the
 * animation state hierarchy.
 *
 * @param targetObject    the proper animation target is returned in this variable,
 *                        and will be NULL if no target exists.
 * @param isCombatTarget  returns true if this controller's owner should be in combat mode.
 */

void CreatureController::getAnimationTargetInternal (Object*& targetObject, bool& isCombatTarget) const
{
	targetObject   = 0;
	isCombatTarget = false;

	//-- Don't bother if appearance is not skeletal.
	if (!m_appearanceIsSkeletal)
		return;

	const CreatureObject *creature = getCreatureObject (getOwner ());

	//-- First, handle using the override animation target.
	if (m_overrideAnimationTargetWatcher)
	{
		Object *const overrideTarget = m_overrideAnimationTargetWatcher->getPointer ();
		if (overrideTarget)
		{
			DEBUG_REPORT_PRINT(s_printTargetSource && (creature == static_cast<const Object*>(Game::getPlayer ())), ("player target source: override.\n"));

			targetObject   = overrideTarget;
			isCombatTarget = m_isOverrideTargetForCombat;
			return;
		}
	}

	//-- Next, check if this creature object is the primary (attacker) object of a PlaybackScript.
	// @todo consider maintaining a list of active playback scripts per TangibleObject so we don't have to do this potentially expensive search.
	//       (This isn't expensive if there typically aren't many combat actions going off simultaneously on a given client).
	s_playbackScripts.clear ();
	PlaybackScriptManager::getPlaybackScriptsForActor (creature, s_playbackScripts);

	if (!s_playbackScripts.empty())
	{
		// There's at least one playback script in which this creature is participating.

		//-- Check if there's a script where this creature is the attacker.
		const PlaybackScript *script = 0;

		const ConstPlaybackScriptVector::iterator endIt = s_playbackScripts.end ();
		for (ConstPlaybackScriptVector::iterator it = s_playbackScripts.begin (); it != endIt; ++it)
		{
			NOT_NULL (*it);

			if ((*it)->getActor (0) == creature)
			{
				// Found a script where the attacker is this creature.  Use this script for determining attacker's target.
				script = *it;
				break;
			}
		}

		//-- Handle target selection and animation combat mode specification for the case of an attacker.
		if (script)
		{
			// Find the target object for the script.
			DEBUG_REPORT_PRINT(s_printTargetSource && targetObject && (creature == static_cast<const Object*>(Game::getPlayer ())), ("player target source: PlaybackScript.\n"));

			targetObject = script->getObjectVariable (TAG_ATRG);

			if (targetObject)
			{
				// If the combat playback has a target, we definitely use the combat portion of the animation hierarchy.
				isCombatTarget = true;
			}
			else
			{
				// If the attacker has a combat target, we want to appear in combat mode, even if the target object is
				// not on the client due to update range issues.
				getFirstCombatTargetObject (*creature, targetObject, isCombatTarget);
			}

			// @todo we will lose the combat target temporarily for a combat action that doesn't have a defender.
			return;
		}
	}

	//-- Next, check for an intended target... (in combat this overrides lookat if autoaim is on)
	if (creature && creature->getState(States::Combat) && CuiPreferences::getAutoAimToggle() && creature == Game::getPlayer() && creature->getIntendedTarget().isValid())
	{
		targetObject   = NetworkIdManager::getObjectById (creature->getIntendedTarget());
		if (targetObject)
		{
			isCombatTarget = true;
			return;
		}
	}

	//-- Next, check if there is a current combat target.  If so, this is the animation target.
	getFirstCombatTargetObject (*creature, targetObject, isCombatTarget);
	if (isCombatTarget)
	{
		// We can have a combat target that is out of client range, so it is possible targetObject is NULL.
		// That's okay at this point.  We definitely want to show as if we have a combat target in this case,
		// even though the animation target will be NULL, causing us to point as if the attacker is straight ahead.
		DEBUG_REPORT_PRINT(s_printTargetSource && (creature == static_cast<const Object*>(Game::getPlayer ())) && targetObject, ("player target source: combat target list.\n"));
		return;
	}

	//-- Next, handle the case where this creature isn't an attacker but is in combat.  The net result is we want
	//   the defender to be in the animation combat hierarchy, but we don't want the defender to start targeting
	//   a look-at target.
	if (!s_playbackScripts.empty ())
	{
		// Look for this creature in anything other than slot 0.  The creature must be listed as an active
		// participant in the script before we consider them a defender.
		const ConstPlaybackScriptVector::iterator endIt = s_playbackScripts.end ();
		for (ConstPlaybackScriptVector::iterator it = s_playbackScripts.begin (); it != endIt; ++it)
		{
			PlaybackScript const *const script = *it;
			NOT_NULL(script);

			int const scriptActorCount = script->getSupportedActorCount() - 1;
			for (int actorIndex = 1; actorIndex < scriptActorCount; ++actorIndex)
			{
				if ((script->getActor(actorIndex) == creature) && script->isActorActive(actorIndex))
				{
					// The creature is participating in a combat playback script as a defender but doesn't have a combat target.
					// It might have a look at target.  We want to put the defender in the combat hierarchy, but don't want the
					// possibly existant look-at target to be used.  The target will be NULL, and the creature will be placed in
					// combat mode.
					targetObject   = 0;
					isCombatTarget = true;
					return;
				}
			}
		}
	}

	//-- Next, if there is no combat target and the creature is not an attacker/defender, use the "lookAt" target,
	//   which should be the currently selected target.
	const NetworkId& targetNetworkId = creature->getLookAtTarget ();
	if (targetNetworkId != NetworkId::cms_invalid)
	{
		DEBUG_REPORT_PRINT(s_printTargetSource && (creature == static_cast<const Object*>(Game::getPlayer ())) && targetObject, ("player target source: look at target.\n"));

		targetObject   = NetworkIdManager::getObjectById (targetNetworkId);
		isCombatTarget = false;

#ifdef _DEBUG
		// In single player, turn the lookAt target into a combat-style target.  This is not how it works in multiplayer,
		// but this makes testing combat actions much simpler.
#if 0
		if (Game::getSinglePlayer())
			isCombatTarget = true;
#endif
#endif

		return;
	}

	//-- Next, check for an intended target... (out of combat lookat overrides this, even if autoaim is on)
	if (creature && !creature->getState(States::Combat) && CuiPreferences::getAutoAimToggle() && creature == Game::getPlayer() && creature->getIntendedTarget().isValid())
	{
		targetObject = NetworkIdManager::getObjectById (creature->getIntendedTarget());
		if (targetObject)
		{
			isCombatTarget = false;
			return;
		}
	}


	DEBUG_REPORT_PRINT(s_printTargetSource && (creature == static_cast<const Object*>(Game::getPlayer ())) && !targetObject, ("player target source: no target.\n"));
}

// ----------------------------------------------------------------------
/**
 * Override the animation target until the next controller alter call.
 *
 * This function allows the caller to override the Object used for
 * the animation target.  It stays in effect until the very next alter()
 * call on this controller, after which the override is cleared and normal
 * target determination is used.
 *
 * @param targetObject    the Object at which this controller's animation should
 *                        target.
 * @parma isCombatTarget  specifies if the target is for combat.  If so, the
 *                        an attacker with a combat target will go in the combat
 *                        branch of the animation state hierarchy.
 */

void CreatureController::overrideAnimationTarget (Object *targetObject, bool isCombatTarget, CrcLowerString const &hardpoint)
{
	if (!m_overrideAnimationTargetWatcher)
		m_overrideAnimationTargetWatcher = new Watcher<Object> (targetObject);
	else
		*m_overrideAnimationTargetWatcher = targetObject;

	m_isOverrideTargetForCombat = isCombatTarget;

	//-- Set the animation target now.
	Appearance          *const baseAppearance = getOwner() ? getOwner ()->getAppearance () : NULL;
	SkeletalAppearance2 *const appearance     = baseAppearance ? baseAppearance->asSkeletalAppearance2 () : NULL;
	if (appearance)
		appearance->setTargetObjectAndHardpoint(targetObject, hardpoint);
}

// ----------------------------------------------------------------------

void CreatureController::changeAnimationStatePath(int serverPosture, bool skipTraversal, bool skipWithDelay)
{
	//-- Do the dynamic casts to get the CreatureObject and the SkeletalAppearance2.
	//   We do not check the m_appearanceIsSkeletal flag because this function must be
	//   called to initialize posture for the client prior to endOfBaselines and prior to
	//   m_appearanceIsSkeletal getting called.
	//-- Get the animation state path for the current environment of this object.

	CreatureObject& creature = *getCreatureObject (getOwner ());

	// Get the animation target to find out if we should be in combat.
	Object* targetObject   = 0;
	bool    isCombatTarget = false;

	getAnimationTarget (targetObject, isCombatTarget);

	// Determine the proper animation state path.
	AnimationStatePath  statePath;
	determineAnimationStatePath (creature, statePath, isCombatTarget, serverPosture);


	// @todo -TRF- revisit this.

	//-- Tell all animation controllers to go to the new state.  Fake the controller out
	//   into thinking it already knows the proper state to be in by not updating the
	//   m_previousAnimationStatePath.  If I did not do this, combat-induced posture changes
	//   would need a way to override the client's idea of the current posture each frame
	//   because the very next alter() on this will cause the controller to revert to the
	//   previous posture change which, no doubt, is not fully in sync with combat.
	//   I would have no idea how long the posture override should take affect.  At some
	//   point the combat-induced posture should no longer come from combat but should come
	//   from the server.  It's almost like I need a way to know "we're no longer in combat."
	//   so I could stop looking at the combat system's idea of the current combat-synched
	//   posture.
	DEBUG_REPORT_LOG(s_logAnimationStatePathActivity && isOwnerAnimationDebuggerTarget(), ("CC: changing animation state path to [%s].\n", m_currentAnimationStatePath->getPathString().c_str()));

	//-- Set the current state as the destination state.
	SkeletalAppearance2 *const appearance = creature.getAppearance () ? creature.getAppearance ()->asSkeletalAppearance2() : 0;
	if (appearance)
		appearance->getAnimationResolver ().setDestinationState (statePath, skipTraversal, skipWithDelay);
}

// ----------------------------------------------------------------------
/**
 * Set the AnimationStateNameId for the currently held item.
 *
 * Set heldStateId to NULL if the creature isn't holding anything.
 *
 * This modifies the state hierarchy's animation state path.
 *
 * @param heldStateId  the name of the animation state that should be used
 *                     for whatever item is being held.  If null, nothing
 *                     is being held by the creature.
 */

void CreatureController::setHeldItemAnimationStateId (const AnimationStateNameId *heldStateId)
{
	if (heldStateId)
	{
		m_useHeldItemState = true;
		*m_heldItemStateId = *heldStateId;

		DEBUG_REPORT_LOG(s_logAnimationStatePathActivity && isOwnerAnimationDebuggerTarget(), ("CC: changing held state to [%s].\n", AnimationStateNameIdManager::getNameString(*heldStateId).getString()));
	}
	else
	{
		m_useHeldItemState = false;

		DEBUG_REPORT_LOG(s_logAnimationStatePathActivity && isOwnerAnimationDebuggerTarget(), ("CC: clearing held state.\n"));
	}
}

//----------------------------------------------------------------------

void CreatureController::setMood (const uint32 mood)
{
	char buf [64];

	IGNORE_RETURN(_itoa (static_cast<int>(mood), buf, 10));
	IGNORE_RETURN(ClientCommandQueue::enqueueCommand (CommandNames::setMoodInternal, NetworkId::cms_invalid, Unicode::narrowToWide (buf)));
} //lint !e1762 // function could be const // Logically this is non-const.

//----------------------------------------------------------------------

void CreatureController::pauseDeadReckoning (float /*pauseTime*/)
{
}

//----------------------------------------------------------------------

bool CreatureController::shouldApplyAnimationDrivenLocomotion () const
{
	return false;
}

// ----------------------------------------------------------------------

float CreatureController::getCurrentSpeed() const
{
	return 0.0f;
}

// ----------------------------------------------------------------------

void CreatureController::setCurrentSpeed (float const /*currentSpeed*/)
{
}

//----------------------------------------------------------------------
/**
 * Find out if the body's animation controller is playing an animation
 * on the play-once track, and if so, does the play-once track have
 * locomotion priority.
 *
 * @param mustBeAtLeastLocomotionPriority  true if the action track must have at
 *                                         least a locomotion priority of 'locomotion' or higher
 *                                         to return a value of true; false otherwise.  Default is false.
 *
 * @return  true when body is playing a play-once animation that has locomotion
 *          priority; false otherwise.
 */

bool CreatureController::doesBodyPlayOnceHaveLocomotionPriority (bool mustBeAtLeastLocomotionPriority) const
{
	if (!m_appearanceIsSkeletal)
		return false;

	//-- get the owner object.
	const Object *const object = getOwner ();
	if (!object)
		return false;

	//-- retrieve the appearance.
	const SkeletalAppearance2 *const appearance = safe_cast<const SkeletalAppearance2*> (object->getAppearance ());
	if (!appearance)
		return false;

	//-- find out if the primary animation controller is playing something on its play-once track.
	const TransformAnimationResolver& resolver = appearance->getAnimationResolver ();

	if (!resolver.isPrimaryPlayOnceTrackActive ())
	{
		// Nothing is playing on the action track; play once cannot have priority.
		return false;
	}

	int const playOnceTrackPriority = resolver.getPrimaryPlayOnceTrackLocomotionPriority ();
	int const loopTrackPriority     = resolver.getPrimaryLoopTrackLocomotionPriority ();

	if (loopTrackPriority > playOnceTrackPriority)
	{
		// Play once track does not have priority.
		return false;
	}

	// PlayOnce track has priority.  Now see if it's high enough.
	if (!mustBeAtLeastLocomotionPriority)
		return true;
	else
	{
		// Return true if play once priority meets or exceeds locomotion priority.
		return playOnceTrackPriority >= s_locomotionPriority;
	}
}

//----------------------------------------------------------------------

void CreatureController::face (const Vector& position_w)
{
	m_face = true;
	m_facePosition_w = position_w;
}

// ----------------------------------------------------------------------

void CreatureController::playAnimationWithFaceTracking (const CrcLowerString &animationActionName, Object& targetObject, int &animationId, bool &animationIsAdd, AnimationNotification *notification)
{
	if (!m_appearanceIsSkeletal)
	{
		DEBUG_WARNING(true, ("playAnimationWithFaceTracking () called on object without a skeletal appearance, ignoring unsupported call."));
		return;
	}

	//-- Keep track of the face-tracking target.
	*m_faceTrackingTarget = &targetObject;

	//-- Play the animation action, keeping track of the action id.
	SkeletalAppearance2* const appearance = safe_cast<SkeletalAppearance2 *> (getOwner ()->getAppearance ());
	NOT_NULL(appearance);

	appearance->getAnimationResolver ().playAction (animationActionName, m_faceTrackingAnimationId, m_faceTrackingAnimationIsAdd, notification);

	//-- Return action's animation id and isAdd info.
	animationId    = m_faceTrackingAnimationId;
	animationIsAdd = m_faceTrackingAnimationIsAdd;

	//-- Indicate we're in face tracking mode.
	m_faceTracking = true;
}

// ----------------------------------------------------------------------

void CreatureController::stopFaceTracking ()
{
	*m_faceTrackingTarget = 0;
	m_faceTracking = false;
}

// ----------------------------------------------------------------------

bool CreatureController::isSwimming() const
{
	return m_isSwimming;
}

// ----------------------------------------------------------------------

bool CreatureController::areVisualsInCombat() const
{
	return m_inCombatState;
}

// ----------------------------------------------------------------------

AnimationStatePath const *CreatureController::getCurrentAnimationStatePath() const
{
	return m_currentAnimationStatePath;
}

// ----------------------------------------------------------------------

void CreatureController::reportYawStateToAppearance (float deltaTime)
{
	// @todo consider having the Player and Remote controller push down this
	//       rotate state if we can guarantee that we can always set it properly.
	//       This function uses theta() which I think is an expensive function to use.
	//       Also, note this is only needed by the animation system if the
	//       character is idling.  Perhaps this code would be best placed
	//       in the YawSkeletalAnimation object and executed only when
	//       necessary.

	if (!m_appearanceIsSkeletal)
	{
		// There's no point in proceeding if the appearance isn't skeletal.
		return;
	}

	//-- Ignore this call if deltaTime <= 0.0.
	if (deltaTime <= 0.0f)
		return;

	//-- Detect current theta (yaw angle) in world space.
	Object *const object = getOwner ();
	if (!object)
		return;

	const Vector forwardVector_w = object->getObjectFrameK_w ();
	const float  newTheta_w      = forwardVector_w.theta ();

	//-- Report the theta change on all but this controller's first frame.
	if (!m_isFirstAlter)
	{
		int yawDirection;

		if (ConfigClientGame::getDisableCreatureTurningAnimation ())
			yawDirection = 0;
		else
		{
			//-- Determine the yaw direction.
			const float deltaTheta_w = newTheta_w - m_previousTheta_w;
			const float thetaRate    = deltaTheta_w / deltaTime;

			if (thetaRate > cs_minAnimationYawRate)
			{
				// This is a rotate right if the delta is less than 180 degrees.
				if (deltaTheta_w <= PI)
					yawDirection = +1;
				else
					yawDirection = -1;

				m_continueTurningTimer = cs_continueTurningTime;
			}
			else if (thetaRate < -cs_minAnimationYawRate)
			{
				// This is a rotate left if the delta is less than 180 degrees.
				if (deltaTheta_w >= - PI)
					yawDirection = -1;
				else
					yawDirection = +1;

				m_continueTurningTimer = cs_continueTurningTime;
			}
			else
			{
				// There is no yaw.

				// Check for continuation of previous turn direction.
				m_continueTurningTimer = std::max(m_continueTurningTimer - deltaTime, 0.0f);
				if (m_continueTurningTimer > 0.0f)
				{
					// DEBUG_REPORT_PRINT(true ,("CC:yaw continue in effect [%.2f]\n", m_continueTurningTimer));
					yawDirection = m_previousYawDirection;
				}
				else
					yawDirection = 0;
			}
		}

		// Tell the appearance about the yaw state.
		SkeletalAppearance2 *const appearance = safe_cast<SkeletalAppearance2*> (object->getAppearance ());
		NOT_NULL(appearance);

		appearance->setYawDirection (yawDirection);
		m_previousYawDirection = yawDirection;
	}

	//-- Keep track of theta value.
	m_previousTheta_w = newTheta_w;
}

// ----------------------------------------------------------------------

void CreatureController::handleFaceTracking ()
{
	if (!m_appearanceIsSkeletal)
		return;

	//-- Check if face tracking animation has ended.
	Object* const owner = getOwner ();
	NOT_NULL (owner);

	SkeletalAppearance2* const appearance = safe_cast<SkeletalAppearance2*> (owner->getAppearance ());
	NOT_NULL(appearance);

	const bool trackAnimationCompleted = appearance->getAnimationResolver ().hasActionCompleted (m_faceTrackingAnimationId, m_faceTrackingAnimationIsAdd);
	if (trackAnimationCompleted)
	{
		stopFaceTracking ();
		return;
	}

	//-- Don't allow face tracking when the action being played doesn't have locomotion priority.
	if (!doesBodyPlayOnceHaveLocomotionPriority ())
	{
		stopFaceTracking ();
		return;
	}

	//-- Get target object position.
	const Object* const targetObject = *m_faceTrackingTarget;
	if (!targetObject)
	{
		// Target object is not longer around.
		stopFaceTracking ();
		return;
	}

	//-- Tell controller to face the target object's position.
	face (targetObject->getPosition_w ());
}

// ----------------------------------------------------------------------
/**
 * Called whenever the owner object moved into or out of combat mode as
 * far as the animation system is concerned.
 *
 * @param movingIntoCombat  specifies the new state of being in combat.  If true,
 *                          the player is moving into combat; false, the player is
 *                          moving out of combat.
 */

void CreatureController::processCombatStateChange(bool movingIntoCombat)
{
	//-- Get the skeletal appearance.
	Object *const owner = getOwner();
	if (!owner)
		return;

	Appearance          *const ownerBaseAppearance = owner->getAppearance();
	SkeletalAppearance2 *const ownerAppearance     = ownerBaseAppearance ? ownerBaseAppearance->asSkeletalAppearance2() : 0;

	if (!ownerAppearance)
		return;

	LabelHash::Id const eventToSend = movingIntoCombat ? ClientGameAppearanceEvents::getOnEquippedEnteredCombatEventId() : ClientGameAppearanceEvents::getOnEquippedExitedCombatEventId();

	//-- Send event to attachments.
	{
		int const attachmentCount = ownerAppearance->getAttachedAppearanceCount();
		for (int i = 0; i < attachmentCount; ++i)
		{
			Appearance *const attachedAppearance = ownerAppearance->getAttachedAppearance(i);
			if (attachedAppearance)
				attachedAppearance->onEvent(eventToSend);
		}
	}

	//-- Send event to wearables.
	{
		int const wearableCount = ownerAppearance->getWearableCount();
		for (int i = 0; i < wearableCount; ++i)
		{
			SkeletalAppearance2 *const wearableAppearance = ownerAppearance->getWearableAppearance(i);
			if (wearableAppearance)
				wearableAppearance->onEvent(eventToSend);
		}
	}
}

// ----------------------------------------------------------------------

bool CreatureController::calculateTerrainAndSwimmingInfo(bool &onTerrain, Vector &terrainNormal, bool &isSwimmingNow, float & terrainHeight, float & waterHeight, bool & isBurning) const
{
	onTerrain     = false;
	terrainNormal = Vector::unitY;
	isSwimmingNow    = false;
	isBurning = false;

	//-- Mounts: if this is a rider, the rider is technically swimming if the mount is swimming.
	CreatureObject const *const ownerCreature = safe_cast<CreatureObject const*>(getOwner());
	if (!ownerCreature)
		return false;

	if (ownerCreature->getState(States::RidingMount))
	{
		CreatureObject const *const mountCreature = ownerCreature->getMountedCreature();
		if (mountCreature)
		{
			CreatureController const *const mountController = safe_cast<CreatureController const*>(mountCreature->getController());
			if (mountController)
			{
				return mountController->calculateTerrainAndSwimmingInfo(onTerrain, terrainNormal, isSwimmingNow, terrainHeight, waterHeight, isBurning);
			}
		}
	}

	// Determine if the creature is on a solid floor.
	CollisionProperty const *const collisionProperty = ownerCreature->getCollisionProperty();
	Footprint const         *const footprint         = collisionProperty ? collisionProperty->getFootprint() : NULL;
	bool const                     isOnSolidFloor    = footprint ? footprint->isOnSolidFloor() : false;

	if (!isOnSolidFloor)
	{
		//-- Creature is not on terrain or swimming if not in the world cell.
		if (!ownerCreature->isInWorldCell())
			return false;

		//-- Determine if the creature should be swimming based on terrain, swim height and water height.		
		// The creature can't be swimming if there's no terrain to provide a water height.
		TerrainObject const *const terrainObject = TerrainObject::getConstInstance();
		if (!terrainObject)
			return false;

		Vector const position = ownerCreature->getPosition_w();
		if (terrainObject->getHeight(position, terrainHeight, terrainNormal))
		{
			onTerrain = true;
			TerrainGeneratorWaterType waterType;
			if (terrainObject->getWaterHeight(position, waterHeight, waterType))
			{
				const float swimHeight = ownerCreature->getSwimHeight();
				isSwimmingNow             = (waterHeight - swimHeight) > terrainHeight;

				if(waterHeight > terrainHeight && waterType == TGWT_lava)
				{
					isBurning = true;
				}
			}
		}
	}
	else if (footprint)
	{
		if (footprint->getGroundHeight(terrainHeight))
			waterHeight = -10000.0f;
		else
			return false;

		terrainNormal = footprint->getGroundNormal_w ();
	}
	else
		return false;

	return true;
}

//----------------------------------------------------------------------

float CreatureController::getDesiredSpeed () const
{
	return 0.0f;
}

// ======================================================================
