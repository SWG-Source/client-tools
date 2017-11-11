// ======================================================================
//
// FireProjectileActionTemplate.cpp
// Copyright 2001 - 2003 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/FireProjectileActionTemplate.h"

// ======================================================================
// lint supression
// ======================================================================
//lint -e39 // conflicting redefinition of std::vector<bool,...> // -TRF- if another compiler has problems with this, convert BoolVector to BoolDeque or another container

#include "clientAnimation/PassthroughPlaybackAction.h"
#include "clientAnimation/PlaybackScript.h"
#include "clientAnimation/PlaybackScriptTemplate.h"
#include "clientAnimation/WatcherAction.h"
#include "clientGame/CellObject.h"
#include "clientGame/ClientEffect.h"
#include "clientGame/ClientEffectTemplate.h"
#include "clientGame/ClientObject.h"
#include "clientGame/ClientWeaponObjectTemplate.h"
#include "clientGame/CombatEffectsManager.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureController.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/FireProjectileAction.h"
#include "clientGame/Game.h"
#include "clientGame/NonTrackingProjectile.h"
#include "clientGame/SequenceObject.h"
#include "clientGame/TrackingProjectile.h"
#include "clientGame/WeaponObject.h"
#include "clientGraphics/Camera.h"
#include "clientGraphics/RenderWorld.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/Skeleton.h"
#include "clientUserInterface/CuiPreferences.h"
#include "sharedCollision/CollideParameters.h"
#include "sharedCollision/CollisionInfo.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedMath/VectorArgb.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/CellProperty.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedRandom/Random.h"
#include "swgSharedUtility/Postures.def"
#include "UIManager.h"

// ======================================================================

const char *const    FireProjectileActionTemplate::cms_weaponHoldingSlotName = "hold_r";

const ConstCharCrcLowerString FireProjectileActionTemplate::cms_fireClientEventName("fire");
const ConstCharCrcLowerString FireProjectileActionTemplate::cms_emissionHardpointNameArray[] =
	{
		ConstCharCrcLowerString("muzzle"),
		ConstCharCrcLowerString("muzzle2"),
		ConstCharCrcLowerString("muzzle3"),
		ConstCharCrcLowerString("muzzle4"),
		ConstCharCrcLowerString("whp1")
	};

const int            FireProjectileActionTemplate::cms_emissionHardpointNameArrayLength = static_cast<int>(sizeof(cms_emissionHardpointNameArray) / sizeof(cms_emissionHardpointNameArray[0]));

// ======================================================================

namespace FireProjectileActionTemplateNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	Tag const TAG_AWPN = TAG(A,W,P,N);
	Tag const TAG_FIPR = TAG(F,I,P,R);
	Tag const TAG_FIPL = TAG(F,I,P,L);
	Tag const TAG_NONE = TAG(N,O,N,E);
	Tag const TAG_PTRL = TAG(P,T,R,L);
	Tag const TAG_AAID = TAG(A,A,I,D);
	Tag const TAG_AANC = TAG(A,A,N,C);
	Tag const TAG_TLCX = TAG(T,L,C,X);
	Tag const TAG_TLCY = TAG(T,L,C,Y);
	Tag const TAG_TLCZ = TAG(T,L,C,Z);
	Tag const TAG_TLCC = TAG(T,L,C,C);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool  s_logProjectileCreationFailure;

	const float s_pistolVelocityScale = 0.5f;
	const float s_carbineVelocityScale = 0.75f;

	const float s_offAxisFiringMaxRadians = PI_OVER_180 * 5.f;

	const float s_aimDistance = 10.f;
}

using namespace FireProjectileActionTemplateNamespace;

// ======================================================================
// public static member functions
// ======================================================================

void FireProjectileActionTemplate::install()
{
	InstallTimer const installTimer("FireProjectileActionTemplate::install");

	PlaybackScriptTemplate::registerActionTemplate(TAG_FIPR, create);
	PlaybackScriptTemplate::registerActionTemplate(TAG_FIPL, createLocation);

	DebugFlags::registerFlag(s_logProjectileCreationFailure, "ClientGame/Combat", "logProjectileCreationFailure");

	ExitChain::add(remove, "FireProjectileActionTemplate");
}

// ======================================================================
// public member functions
// ======================================================================

FireProjectileActionTemplate::FireProjectileActionTemplate(
		bool hitSuccessful,
		int  emittingActorIndex,
		int  muzzleIndex,
		Tag  projectileFilenameVariable,
		Tag  projectileVelocityVariable,
		int  targetActorIndex,
		int  watcherActionIndex,
		int  watcherActionThread,
		TargetMode  targetMode,
		bool useEmitterTargetAsTargetActor,
		int targetHitLocation
		) :
	PlaybackActionTemplate(),
	m_emittingActorIndex(emittingActorIndex),
	m_hardpointNameVariable(TAG_NONE),
	m_projectileFilenameVariable(projectileFilenameVariable),
	m_projectileVelocityVariable(projectileVelocityVariable),
	m_targetActorIndex(targetActorIndex),
	m_hitSuccessful(hitSuccessful),
	m_watcherActionIndex(watcherActionIndex),
	m_watcherActionThread(watcherActionThread),
	m_muzzleIndex(muzzleIndex),
	m_targetMode(targetMode),
	m_useEmitterTargetAsTargetActor(useEmitterTargetAsTargetActor),
	m_targetHitLocation(targetHitLocation)
{
}

// ----------------------------------------------------------------------

PlaybackAction *FireProjectileActionTemplate::createPlaybackAction() const
{
	return new PassthroughPlaybackAction(*this);
}

// ----------------------------------------------------------------------

PlaybackAction *FireProjectileActionTemplate::createPlaybackAction(CallbackAnimationNotification *notification, char const *animationMessageName) const
{
	return new FireProjectileAction(*this, notification, animationMessageName);
}

// ----------------------------------------------------------------------

bool FireProjectileActionTemplate::update(float deltaTime, PlaybackScript &script) const
{
	DEBUG_REPORT_LOG(ConfigClientGame::getLogFireProjectileAction(), ("FPAT: start.\n"));

	Object  * targetObject = getTargetObject(script);

	if(m_targetMode != TM_location)
	{	
		if (!targetObject)
		{
			DEBUG_WARNING(PlaybackScriptNamespace::s_verbosePlaybackScriptWarnings, ("Could not fire projectile because target object (defender) is not present in script."));
			return false;
		}

		if (!ConfigClientGame::getAllowShootingDeadTarget())
		{
			//-- Abort firing if target object is a creature that is in the dead or incapacitated posture.
			if (targetObject)
			{
				ClientObject const *const clientTargetObject = targetObject->asClientObject();
				if (clientTargetObject)
				{
					CreatureObject const *const creatureObject = clientTargetObject->asCreatureObject();
					if (creatureObject)
					{
						Postures::Enumerator const targetPosture = creatureObject->getVisualPosture();
						if ((targetPosture == Postures::Dead) || (targetPosture == Postures::Incapacitated))
						{
							//-- Indicate the action is complete --- we'll abort this fire.
							DEBUG_REPORT_LOG(ConfigClientGame::getLogFireProjectileAction(), ("FPAT: aborting attack on target with id [%s], template [%s] because it is dead/incapacitated.\n", creatureObject->getNetworkId().getValueString().c_str(), creatureObject->getObjectTemplateName()));
							return false;
						}
					}
				}
			}
		}
	}

	//-- get projectile starting point in world space
	CellProperty const * startCellProperty = NULL;
	Vector startPosition_w;
	CrcLowerString emitterHardpoint;
	if (!getProjectileStartPosition(script, startCellProperty, startPosition_w, emitterHardpoint))
	{
		// end the action
		DEBUG_WARNING(true, ("failed to get projectile's starting position"));
		return false;
	}

	//-- create the projectile
	Projectile * const projectile = createProjectile(script, emitterHardpoint);
	if (!projectile)
	{
		// end the action
		DEBUG_WARNING(true, ("failed to create the projectile"));
		return false;
	}

	//-- get projectile end point in world space
	Vector endPosition_w;

	// we now shoot where the user is pointing...
	if(m_targetMode == TM_location)
	{
		endPosition_w = getTargetLocation_w(script);
	}
	// The auto aim toggle on which means we ALWAYS shoot at our intended target if we have one...
	else if (Game::getPlayer() == getEmitterObject(script) && CuiPreferences::getAutoAimToggle() && targetObject != 0)
	{
		endPosition_w = targetObject->getCollisionSphereExtent_w().getCenter();
	}
	else if (Game::getPlayer() == getEmitterObject(script))
	{
		float targetDistance = 1000.0f;
		Camera const * const camera = Game::getConstCamera();

		if (targetObject != 0)
		{
			Vector direction = targetObject->getPosition_w() - startPosition_w;
			targetDistance = direction.approximateMagnitude();
		}
		if (camera != 0)
		{
			UIPoint mouse(UIManager::gUIManager().GetLastMouseCoord());

			Vector mouseDirection_w = camera->rotate_o2w(camera->reverseProjectInScreenSpace(mouse.x, mouse.y));
			mouseDirection_w.normalize();

			endPosition_w = startPosition_w + (mouseDirection_w * targetDistance);

			if (targetObject != 0)
			{
				Appearance const * const appearance = targetObject->getAppearance();

				if (appearance != 0)
				{
					Vector const cameraStart_w(camera->getPosition_w());
					Vector const cameraEnd_w(cameraStart_w + (mouseDirection_w * 1000.0f));
					Vector const start_o(targetObject->rotateTranslate_w2o(cameraStart_w));
					Vector const end_o(targetObject->rotateTranslate_w2o(cameraEnd_w));

					CollideParameters collideParameters;
					CollisionInfo result;

					if (appearance->collide(start_o, end_o, collideParameters, result))
					{
						endPosition_w = targetObject->rotateTranslate_o2w(result.getPoint());
					}
				}
			}
		}
	}
	else if (targetObject != 0)
	{
		endPosition_w = ((targetObject != 0) && (targetObject->getAppearance() != 0))
			? targetObject->getAppearanceSphereCenter_w()
			: targetObject->getPosition_w();

		Vector direction = endPosition_w - startPosition_w;
		const float targetDistance = direction.approximateMagnitude();

		if (targetDistance > s_aimDistance)
		{
			CrcLowerString hardpointName = CombatEffectsManager::getHitLocationHardPointName(*targetObject, m_targetHitLocation);
			if (!hardpointName.isEmpty())
			{
				getHardpointPositionByName(*targetObject, hardpointName, endPosition_w);
			}
		}

		bool pickExactHitPosition = (m_targetMode == TM_normal) ? m_hitSuccessful : (m_targetMode != TM_animationTargetNoTrack);

		if (!getProjectileEndPosition(script, pickExactHitPosition, startPosition_w, endPosition_w))
		{
			// end the action
			DEBUG_WARNING(true, ("failed to get projectile's ending position"));
			return false;
		}
	}
	else
	{
		DEBUG_FATAL(true, ("Should always have a target object unless this is is the player's client and they are playing an attack animation only\n"));
		return false;
	}

	// set the look at position to be the 
	//-- calculate projectile position and heading given start and end point
	setProjectileTransform(*projectile, startCellProperty, startPosition_w, endPosition_w);
	projectile->setStartPosition_w(startPosition_w);

	//-- setup the projectile's velocity and time to live
	setProjectileVelocityAndLifetime(script, *projectile, startPosition_w, endPosition_w);

	//-- setup watcher trigger to signal when projectile dies right when hitting target.
	if (m_watcherActionIndex >= 0)
		setWatcherAction(script, *projectile);

	//-- add to world
	ClientWorld::queueObject(projectile); //lint !e1060 // (Error -- protected member World::queueObject(Object *) is not accessible to non-member non-friend functions) // ??

	//-- Create the client event at the weapon muzzle.
	createFireClientEvent(script);

	//-- Move bolt forward in time equal to the amount of elapsed time since the bolt went off (passed in via delta time via FireProjectileAction).
	// @todo if this is not getting called from FireProjectileAction, which will happen if this action is used outside of FIRE_SETUP_ACTION in PSTs,
	//       this code is wrong.  I'm not aware of this getting used right now, though.
	if (deltaTime > 0.0f)
	{
		float const alterResult = projectile->alter(deltaTime);
		if (alterResult == AlterResult::cms_kill) //lint !e777 // testing floats for equality // it's okay, using consts on both ends.
			projectile->kill();
	}

	// Indicate that the action is finished.
	return false;
}

// ======================================================================
// private static member functions
// ======================================================================

void FireProjectileActionTemplate::remove()
{
	PlaybackScriptTemplate::deregisterActionTemplate(TAG_FIPR);
}

// ----------------------------------------------------------------------

PlaybackActionTemplate *FireProjectileActionTemplate::create(Iff &iff)
{
	return new FireProjectileActionTemplate(iff);
}

// ----------------------------------------------------------------------

PlaybackActionTemplate *FireProjectileActionTemplate::createLocation(Iff &iff)
{
	return new FireProjectileActionTemplate(iff, true);
}

// ======================================================================
// private member functions
// ======================================================================

FireProjectileActionTemplate::FireProjectileActionTemplate(Iff &iff, bool isLocationBased) :
	PlaybackActionTemplate(),
	m_emittingActorIndex(-1),
	m_hardpointNameVariable(TAG_NONE),
	m_projectileFilenameVariable(TAG_NONE),
	m_projectileVelocityVariable(TAG_NONE),
	m_targetActorIndex(-1),
	m_hitSuccessful(false),
	m_watcherActionIndex(-1),
	m_watcherActionThread(0),
	m_muzzleIndex(0),
	m_targetMode(TM_normal),
	m_useEmitterTargetAsTargetActor(false)
{
	if(!isLocationBased)
	{	
		iff.enterForm(TAG_FIPR);

			const Tag version = iff.getCurrentName();

			if (version == TAG_0000)
				load_0000(iff);
			else
			{
				char name[5];

				ConvertTagToString(version, name);
				DEBUG_FATAL(true, ("unsupported FireProjectileActionTemplate version [%s]", name));
			}

		iff.exitForm(TAG_FIPR);
	}
	else
	{
		iff.enterForm(TAG_FIPL);
			const Tag version = iff.getCurrentName();

			if (version == TAG_0000)
				load_location_0000(iff);
			else
			{
				char name[5];

				ConvertTagToString(version, name);
				DEBUG_FATAL(true, ("unsupported FireProjectileActionTemplate version [%s]", name));
			}
		iff.exitForm(TAG_FIPL);
	}
}

// ----------------------------------------------------------------------

void FireProjectileActionTemplate::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);
		iff.enterChunk(TAG_INFO);

			m_emittingActorIndex = static_cast<int>(iff.read_int16());
			DEBUG_FATAL(m_emittingActorIndex < 0, ("invalid m_emittingActorIndex %d", m_emittingActorIndex));

			m_hardpointNameVariable = Tag(iff.read_uint32());

			m_projectileFilenameVariable = Tag(iff.read_uint32());
			m_projectileVelocityVariable = Tag(iff.read_uint32());

			m_targetActorIndex = static_cast<int>(iff.read_int16());

			m_hitSuccessful = (iff.read_uint8() != 0);

			m_watcherActionIndex  = static_cast<int>(iff.read_int16());
			m_watcherActionThread = static_cast<int>(iff.read_int16());

		iff.exitChunk(TAG_INFO);
	iff.exitForm(TAG_0000);

	//-- Setup target mode based on older, more confusing mechanism.
	if (m_hitSuccessful && (m_targetActorIndex < 0))
		m_targetMode = TM_animationTargetNoTrack;
	else
		m_targetMode = TM_normal;
	
}

// ----------------------------------------------------------------------

void FireProjectileActionTemplate::load_location_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);
		iff.enterChunk(TAG_INFO);

			m_emittingActorIndex = static_cast<int>(iff.read_int16());
			DEBUG_FATAL(m_emittingActorIndex < 0, ("invalid m_emittingActorIndex %d", m_emittingActorIndex));

			m_hardpointNameVariable = Tag(iff.read_uint32());

			m_projectileFilenameVariable = Tag(iff.read_uint32());
			m_projectileVelocityVariable = Tag(iff.read_uint32());

			float dummy;
			dummy = iff.read_float();
			dummy = iff.read_float();
			dummy = iff.read_float();
			
			m_targetActorIndex = -2;  // Magical tag indicating we're using location field instead

			m_hitSuccessful = false;

			m_watcherActionIndex  = static_cast<int>(iff.read_int16());
			m_watcherActionThread = static_cast<int>(iff.read_int16());

		iff.exitChunk(TAG_INFO);
	iff.exitForm(TAG_0000);

	m_targetMode = TM_location;
}

// ----------------------------------------------------------------------
/**
 * Get the projectile starting point in world space.
 *
 * @return  true if operation was successful; false if operation failed.
 */

bool FireProjectileActionTemplate::getProjectileStartPosition(PlaybackScript &script, const CellProperty *&startCellProperty, Vector &startPosition_w, CrcLowerString &emitterHardpoint) const
{
	//-- make sure the emitting actor (attacker) is present.  If not, it's
	//   not feasible for us to shoot a bolt since we don't know where to
	//   shoot from.  -TRF- rather than having actors be present and non-present,
	//   we may want to go for "modifiable" and "non-modifiable".  The attacker
	//   could be in another combat action and be "non-modifiable", but still present
	//   so we can shoot the bolt.
	const Object *const emittingObject = getEmitterObject(script);
	if (!emittingObject)
	{
		DEBUG_WARNING(true, ("Could not fire projectile because emitting object (attacker) not present in script."));

		// we failed
		return false;
	}

	DEBUG_REPORT_LOG(ConfigClientGame::getLogFireProjectileAction(), ("FPAT: attacker object [id=%s].\n", emittingObject->getNetworkId().getValueString().c_str()));

	//-- Get projectile start point in world space.
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, m_muzzleIndex, cms_emissionHardpointNameArrayLength);
	emitterHardpoint = cms_emissionHardpointNameArray[m_muzzleIndex];

	//-- get the starting position for the projectile
	startCellProperty = emittingObject->getParentCell();

	return getHardpointPositionByName(*emittingObject, emitterHardpoint, startPosition_w);
}

// ----------------------------------------------------------------------

bool FireProjectileActionTemplate::getProjectileEndPosition(PlaybackScript &script, bool pickExactHitLocation, const Vector &startPosition_w, Vector &endPosition_w) const
{
	if(m_targetMode == TM_location)
	{
		endPosition_w = getTargetLocation_w(script);
		return true;
	}
	//-- get the target object
	Object *const targetObject = getTargetObject(script);

	// target is hit
	if (targetObject)
	{
		if (getEndPosition(*targetObject, pickExactHitLocation, startPosition_w, endPosition_w))
		{
			DEBUG_REPORT_LOG(ConfigClientGame::getLogFireProjectileAction(), ("FPAT: projectile end world position (%.2f,%.2f,%.2f).\n", endPosition_w.x, endPosition_w.y, endPosition_w.z));

			// success
			return true;
		}
		else
		{
			// fail the operation
			DEBUG_WARNING(true, ("failed to get defender hit position"));
			return false;
		}
	}

	// -TRF- to do: figure out how to successfully hit a non-supplied object.  I think this goes back
	//              to needing the ability to have modifiable/non-modifiable actors rather
	//              than present/non-present actors.
	DEBUG_WARNING(true, ("Could not fire projectile because target object (defender) is not present in script."));

	// fail the operation
	return false;
}

// ----------------------------------------------------------------------
/**
 * Retrieve the world position of the specified hardpoint on the given
 * object.
 *
 * @return  true if operation is successful; false otherwise.
 */

bool FireProjectileActionTemplate::getHardpointPositionByName(const Object &object, const CrcLowerString &hardpointName, Vector &position_w) const
{
	//-- Get the appearance.
	const Appearance *const appearance = object.getAppearance();
	if (!appearance)
	{
		DEBUG_WARNING(true, ("object has no skeleton"));
		return false;
	}

	//-- Get the transform for the hardpoint.
	Transform  hardpointTransform(Transform::IF_none);

	const bool hardpointExists = appearance->findHardpoint(hardpointName, hardpointTransform);
	if (!hardpointExists)
	{
		DEBUG_WARNING(true, ("FireProjectile's emitter hardpoint [%s] doesn't exist on emitting object.", hardpointName.getString()));
		return false;
	}

	//-- Convert hardpointToObject into hardpointToWorld.
	position_w = object.rotateTranslate_o2w(hardpointTransform.getPosition_p());

	//-- Success.
	return true;
}

// ----------------------------------------------------------------------
/**
 * Get the world-space position of a skeletal transform given the
 * skeleton and transform index.
 */

bool FireProjectileActionTemplate::getSkeletonTransformPositionByIndex(const Object &object, const Skeleton &skeleton, int transformIndex, Vector &position_w) const
{
	//-- calculate the hardpoint's position
	// get hardpoint's transform to object root
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, transformIndex, skeleton.getTransformCount());
	const Transform &hardpointToObject = skeleton.getJointToRootTransformArray()[transformIndex];

	// hardopint to world = object to world * hardpoint to object
	Transform hardpointToWorld(Transform::IF_none);
	hardpointToWorld.multiply(object.getTransform_o2w(), hardpointToObject);

	// extract position and return
	position_w = hardpointToWorld.getPosition_p();
	return true;
}

// ----------------------------------------------------------------------

bool FireProjectileActionTemplate::getEndPosition(const Object & /*object*/, bool pickExactHitLocation, const Vector& startPosition_w, Vector &endPosition_w) const
{
	Vector direction = endPosition_w - startPosition_w;

	if (!pickExactHitLocation)
	{
		const float targetDistance = direction.approximateMagnitude();
		const float t = targetDistance > s_aimDistance ? 1.f : targetDistance * 0.1f;
		const float deviation = 0.4f * t;
		endPosition_w.x += Random::randomReal(-deviation, deviation);
		endPosition_w.y += Random::randomReal(-deviation, deviation);
		endPosition_w.z += Random::randomReal(-deviation, deviation);
		direction = endPosition_w - startPosition_w;
	}

	direction.approximateNormalize();
	direction *= 128.0f;
	endPosition_w = startPosition_w + direction;

	return true;

}

// ----------------------------------------------------------------------
/**
 * Retrieve the skeleton for an object.
 *
 * @return  true if the operation succeeded; false otherwise.
 */

bool FireProjectileActionTemplate::getSkeleton(const Object &object, const Skeleton *&skeleton) const
{
	//-- get appearance
	const Appearance *const baseAppearance = object.getAppearance();
	if (!baseAppearance)
	{
		// DEBUG_WARNING(true, ("object has no appearance"));
		return false;
	}

	//-- handle hardpoint retrieval if skeletal appearance
	const SkeletalAppearance2 *const skeletalAppearance = dynamic_cast<const SkeletalAppearance2*>(baseAppearance);
	if (!skeletalAppearance)
	{
		// DEBUG_WARNING(true, ("object does not have a skeletal-based appearance"));
		return false;
	}

	//-- get the skeleton
	skeleton = skeletalAppearance->getDisplayLodSkeleton();
	if (skeleton)
		return true;
	else
		return false;
}

// ----------------------------------------------------------------------
/**
 * Create the projectile for the action.
 *
 * @return  true if the operation succeeded; false otherwise.
 */

Projectile * FireProjectileActionTemplate::createProjectile(PlaybackScript &script, CrcLowerString const &emitterHardpoint) const
{
	//-- Get the attacker's weapon template.
	WeaponObject * const weaponObject = dynamic_cast<WeaponObject *>(getAttackerWeaponObject(script));

	if (!weaponObject)
	{
		DEBUG_WARNING(!weaponObject, ("ERROR: Unable to create the projectile. The attackers WeaponObject is NULL.\n"));
		return NULL;
	}

	//-- Create the projectile based on the weapon template.
	bool  tracking = false;
	bool  doRicochet            = false;

	int actionNameCrc = 0;
	IGNORE_RETURN(script.getIntVariable(TAG_AANC, actionNameCrc));
	
	switch (m_targetMode)
	{
		case TM_normal:
			tracking = m_hitSuccessful;
			doRicochet = false;
			break;
		
		case TM_animationTargetNoTrack:
			tracking = false;
			doRicochet = false;
			break;
	
		case TM_animationTargetTrackRicochet:
			tracking = true;
			doRicochet = true;
			break;
	
		case TM_animationTargetTrackReflect:
			tracking = true;
			doRicochet = false;
			break;
			
		case TM_animationTargetTrackReflectTarget:
			tracking = true;
			doRicochet = false;
			break;
	}

	bool attackIsSpecial = CombatEffectsManager::actionHasSpecialAttackEffect(actionNameCrc);

	//-- Get the target.  Target must account for using animation target in place of defender when doing arc firing.
	Object * target = getTargetObject(script);

	Projectile * projectile = NULL;

	if (tracking && (Game::getPlayer() != getEmitterObject(script) || attackIsSpecial))
	{
		TrackingProjectile* trackingProjectile = new TrackingProjectile(NULL, *weaponObject, emitterHardpoint, doRicochet);
		trackingProjectile->setActionNameCrc(actionNameCrc);

		CrcLowerString hardpointName = CombatEffectsManager::getHitLocationHardPointName(*target, m_targetHitLocation);
		trackingProjectile->setTargetHardpoint(hardpointName);
		trackingProjectile->setAttacker(getEmitterObject(script));

		projectile = trackingProjectile;
	}
	else
	{
		NonTrackingProjectile* nonTrackingProjectile = new NonTrackingProjectile(NULL, *weaponObject, emitterHardpoint);
		nonTrackingProjectile->setAttacker(getEmitterObject(script));

		projectile = nonTrackingProjectile;
	}

	// Put the appearance on the projectile

	CombatEffectsManager::attachProjectileAppearance(*weaponObject, *projectile);

	projectile->setTarget(target);

	// Find out if projectiles should show trails.
	int showTrail = 0;
	IGNORE_RETURN(script.getIntVariable(TAG_PTRL, showTrail));

	if (showTrail)
	{
		CombatEffectsManager::addProjectileSpecialAttackEffect(*projectile, actionNameCrc, *weaponObject);
	}

	// success

	return projectile;
}

// ----------------------------------------------------------------------

void FireProjectileActionTemplate::setProjectileTransform(Projectile &projectile, const CellProperty *startCellProperty, const Vector &startPosition_w, const Vector &endPosition_w) const
{
	// build the projectile-to-world transform.
	const Vector startPosition_o = startCellProperty != CellProperty::getWorldCellProperty() ? startCellProperty->getOwner().rotateTranslate_w2o(startPosition_w) : startPosition_w;
	const Vector endPosition_o   = startCellProperty != CellProperty::getWorldCellProperty() ? startCellProperty->getOwner().rotateTranslate_w2o(endPosition_w) : endPosition_w;

	//-- create transform K vector (z,forward) along line of start/end
	Vector kVector = endPosition_o - startPosition_o;
	IGNORE_RETURN(kVector.normalize());

	//-- create transform I vector (x,right) as perpendicular to the plane of the K vector and up vector.
	//   this is not guaranteed to be normalized since the K vector may not be perpendicular to the up vector.
	Vector iVector = Vector::unitY.cross(kVector);
	IGNORE_RETURN(iVector.normalize());

	//-- J vector (y,up) simply is the cross of the valid K and I.  This will be normalized since I and K are.
	const Vector jVector = kVector.cross(iVector);

	projectile.setParentCell(const_cast<CellProperty *>(startCellProperty));

	CellProperty::setPortalTransitionsEnabled(false);
		projectile.setTransformIJK_o2p(iVector, jVector, kVector);
		projectile.setPosition_p(startPosition_o);
	CellProperty::setPortalTransitionsEnabled(true);

#ifdef _DEBUG
	if (ConfigClientGame::getLogFireProjectileAction())
	{
		const Vector pos_w = projectile.getPosition_w();
		DEBUG_REPORT_LOG(true, ("FPAT: projectile start world pos (%.2f,%.2f,%.2f), is in world cell [%s].\n", pos_w.x, pos_w.y, pos_w.z, projectile.isInWorldCell() ? "yes" : "no"));
	}
#endif
}

// ----------------------------------------------------------------------

void FireProjectileActionTemplate::setProjectileVelocityAndLifetime(const PlaybackScript &script, Projectile &projectile, const Vector &startPosition, const Vector &endPosition) const
{
	//-- set projectile velocity
	// get velocity from script variable
	float velocity = 1.0f;

	if (!script.getFloatVariable(m_projectileVelocityVariable, velocity))
	{
		char name[5];

		ConvertTagToString(m_projectileVelocityVariable, name);
		DEBUG_FATAL(true, ("projectile's velocity variable [%s] is not a PlaybackScript variable\n", name));
	}

	//-- scale the speed based on the weapon type
	WeaponObject * const weaponObject = dynamic_cast<WeaponObject *>(getAttackerWeaponObject(script));

	if (weaponObject)
	{
		switch (weaponObject->getWeaponType())
		{
			case WeaponObject::WT_pistol:
				velocity *= s_pistolVelocityScale;
				break;
			case WeaponObject::WT_lightRifle:
				velocity *= s_carbineVelocityScale;
				break;
			default:
				// leave the value unchanged
				break;
		}
	}
	else
	{
		DEBUG_WARNING(!weaponObject, ("ERROR: Unable to create the projectile. The attackers WeaponObject is NULL.\n"));
	}


	projectile.setSpeed(velocity);

	//-- calculate time of travel given distance and velocity
	const float shootDistance    = endPosition.magnitudeBetween(startPosition);
	const float expirationTime   = (shootDistance > velocity) ? shootDistance / velocity : 1.0f;

	projectile.setExpirationTime(expirationTime);
}

// ----------------------------------------------------------------------

void FireProjectileActionTemplate::setWatcherAction(PlaybackScript &script, Projectile &projectile) const
{
	//-- ensure trigger index is valid
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, m_watcherActionIndex, script.getActionCount(m_watcherActionThread));

	//-- get the trigger
	WatcherAction *const action = safe_cast<WatcherAction*>(script.getAction(m_watcherActionThread, m_watcherActionIndex));

	//-- set the trigger object to the projectile
	action->setWatcherObject(&projectile);
}

// ----------------------------------------------------------------------
/**
 * Signal the client event for weapon firing on the weapon's muzzle.
 *
 * The muzzle hardpoint name is "muzzleX", where X = <nothing> when the
 * muzzle number is 1, or the muzzle number when it is anything other than
 * one (e.g. muzzle number 2 will shoot from hardpoint "muzzle2").
 *
 * @param script  The playback script playing the fire action.
 */

void FireProjectileActionTemplate::createFireClientEvent(PlaybackScript &script) const
{
	Object * const attacker = getEmitterObject(script);

	if (!attacker)
		return;

	//-- Get the weapon

	WeaponObject * weaponObject = dynamic_cast<WeaponObject *> (getAttackerWeaponObject(script));

	//-- Signal the client event.

	if (weaponObject != NULL)
	{
		//-- Get the muzzle hardpoint name based on the muzzle index.

		CrcLowerString const & hardPoint = cms_emissionHardpointNameArray[m_muzzleIndex];

		CombatEffectsManager::createMuzzleFlash(*weaponObject, hardPoint);
	}
	else
	{
		DEBUG_REPORT_LOG(true, ("ERROR: "));
	}
}

// ----------------------------------------------------------------------

Object *FireProjectileActionTemplate::getEmitterObject(PlaybackScript &script) const
{
	return script.getActor(m_emittingActorIndex);
}

// ----------------------------------------------------------------------

Object *FireProjectileActionTemplate::getTargetObject(PlaybackScript &script) const
{
	//-- get the target object
	Object *targetObject = 0;

	switch (m_targetMode)
	{
		case TM_location:
			return NULL;
		case TM_normal:
			if(m_useEmitterTargetAsTargetActor)
			{
				//-- Use the emitter actor's combat target.
				CreatureObject *const attackerObject = dynamic_cast<CreatureObject*>(getEmitterObject(script));
				if (attackerObject)
				{
					CachedNetworkId const & lookAtTarget = attackerObject->getLookAtTarget();

					if (lookAtTarget == CachedNetworkId::cms_invalid)
					{
						//Default back to script target
						targetObject    = script.getActor(m_targetActorIndex);			
					}
					else
					{
						targetObject = lookAtTarget.getObject ();
					}
				}
				else
				{					
					targetObject    = script.getActor(m_targetActorIndex);
				}
			}
			else
			{
				targetObject = script.getActor(m_targetActorIndex);
			}
			break;


		case TM_animationTargetNoTrack:
		case TM_animationTargetTrackRicochet:
		case TM_animationTargetTrackReflect:
		case TM_animationTargetTrackReflectTarget:
			{
				//-- Use the actor's animation target.
				Object *const attackerObject = getEmitterObject(script);
				if (attackerObject)
				{
					//-- Get CreatureController.
					CreatureController *const attackerController = dynamic_cast<CreatureController*>(attackerObject->getController());
					if (attackerController)
					{
						bool  isCombatTarget = false;
						attackerController->getAnimationTarget(targetObject, isCombatTarget);
					}
				}
			}
			break;
	}

	return targetObject;
}

// ----------------------------------------------------------------------
/**
 * Retrieve the weapon object used by the attacker.
 *
 * @param script  the playback script.
 *
 * @return  the weapon object used by the player, may be NULL if no weapon info is available.
 */

Object *FireProjectileActionTemplate::getAttackerWeaponObject(const PlaybackScript &script) const
{
	return script.getObjectVariable(TAG_AWPN);
}

// ----------------------------------------------------------------------

Vector FireProjectileActionTemplate::getTargetLocation_w(const PlaybackScript &script) const
{
	Vector result;
	script.getFloatVariable(TAG_TLCX, result.x);
	script.getFloatVariable(TAG_TLCY, result.y);
	script.getFloatVariable(TAG_TLCZ, result.z);
	const CellObject *targetCell = getTargetCell(script);
	if(targetCell)
	{
		result = targetCell->getTransform_o2w().rotateTranslate_l2p(result);
	}
	return result;
}

// ----------------------------------------------------------------------

const CellObject *FireProjectileActionTemplate::getTargetCell(const PlaybackScript &script) const
{
	return static_cast<CellObject *>(script.getObjectVariable(TAG_TLCC));
}
// ======================================================================
