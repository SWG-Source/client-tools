// ======================================================================
//
// ThrowSwordAction.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ThrowSwordAction.h"

#include "clientAnimation/PlaybackScript.h"
#include "clientGame/ClientEventManager.h"
#include "clientGame/ClientGameAppearanceEvents.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/CombatEffectsManager.h"
#include "clientGame/PlaySkeletalAnimationActionTemplate.h"
#include "clientGame/ThrowSwordActionTemplate.h"
#include "clientGraphics/RenderWorld.h"
#include "clientObject/HardpointObject.h"
#include "clientParticle/SwooshAppearance.h"
#include "clientParticle/SwooshAppearanceTemplate.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/TransformAnimationResolver.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/CustomizationDataProperty.h"
#include "sharedObject/MemoryBlockManagedObject.h"
#include "sharedObject/TrackingDynamics.h"

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(ThrowSwordAction, true, 0, 0, 0);

// ======================================================================

namespace ThrowSwordActionNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	Object *getAttackerObject(ThrowSwordAction const &action, PlaybackScript &script);
	Object *getDefenderObject(ThrowSwordAction const &action, PlaybackScript &script);

	void    getHardpointName(ThrowSwordAction const &action, PlaybackScript const &script, CrcString &hardpointName);
	float   getYawRateInRadians(ThrowSwordAction const &action, PlaybackScript const &script);
	float   getThrowSpeed(ThrowSwordAction const &action, PlaybackScript const &script);

	bool    isHit(ThrowSwordAction const &action, PlaybackScript const &script);
	float   getMissExtentMultiplier(ThrowSwordAction const &action, PlaybackScript const &script);
	float   getHitExtentMultiplier(ThrowSwordAction const &action, PlaybackScript const &script);
	void    getHitEventName(ThrowSwordAction const &action, PlaybackScript const &script, CrcString &eventName);
	void    getHitDefenderActionName(ThrowSwordAction const &action, PlaybackScript const &script, CrcString &actionName);

	float   getCatchTimeDuration(ThrowSwordAction const &action, PlaybackScript const &script);
	void    getCatchSwordActionName(ThrowSwordAction const &action, PlaybackScript const &script, CrcString &actionName);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	Tag const TAG_DEPS = TAG(D,E,P,S);
	Tag const TAG_DHAC = TAG(D,H,A,C);
	Tag const TAG_NONE = TAG(N,O,N,E);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	std::string  temporaryString;

	void addSwoosh(Object *parentObject, CrcString const &startHardPoint, CrcString const &endHardPoint, Object *realSwordObject);
}

using namespace ThrowSwordActionNamespace;

// ======================================================================
// Namespace: ThrowSwordActionNamespace
// ======================================================================

Object *ThrowSwordActionNamespace::getAttackerObject(ThrowSwordAction const &action, PlaybackScript &script)
{
	ThrowSwordActionTemplate const *const actionTemplate = safe_cast<ThrowSwordActionTemplate const*>(action.getPlaybackActionTemplate());
	NOT_NULL(actionTemplate);

	return script.getActor(actionTemplate->getAttackerActorIndex());
}

// ----------------------------------------------------------------------

Object *ThrowSwordActionNamespace::getDefenderObject(ThrowSwordAction const &action, PlaybackScript &script)
{
	ThrowSwordActionTemplate const *const actionTemplate = safe_cast<ThrowSwordActionTemplate const*>(action.getPlaybackActionTemplate());
	NOT_NULL(actionTemplate);

	return script.getActor(actionTemplate->getDefenderActorIndex());
}

// ----------------------------------------------------------------------

void ThrowSwordActionNamespace::getHardpointName(ThrowSwordAction const &action, PlaybackScript const &script, CrcString &hardpointName)
{
	ThrowSwordActionTemplate const *const actionTemplate = safe_cast<ThrowSwordActionTemplate const*>(action.getPlaybackActionTemplate());
	NOT_NULL(actionTemplate);

	bool const gotVariable = script.getStringVariable(actionTemplate->getHardpointNameVariable(), temporaryString);
	if (gotVariable)
		hardpointName.set(temporaryString.c_str(), false);
	else
		hardpointName.set("", false);
}

// ----------------------------------------------------------------------

float ThrowSwordActionNamespace::getYawRateInRadians(ThrowSwordAction const &action, PlaybackScript const &script)
{
	ThrowSwordActionTemplate const *const actionTemplate = safe_cast<ThrowSwordActionTemplate const*>(action.getPlaybackActionTemplate());
	NOT_NULL(actionTemplate);

	float yawRateInDegrees = 270.0f;
	IGNORE_RETURN(script.getFloatVariable(actionTemplate->getYawRateVariable(), yawRateInDegrees));

	return yawRateInDegrees * PI_OVER_180;
}

// ----------------------------------------------------------------------

float ThrowSwordActionNamespace::getThrowSpeed(ThrowSwordAction const &action, PlaybackScript const &script)
{
	ThrowSwordActionTemplate const *const actionTemplate = safe_cast<ThrowSwordActionTemplate const*>(action.getPlaybackActionTemplate());
	NOT_NULL(actionTemplate);

	float throwSpeed = 10.0f;
	IGNORE_RETURN(script.getFloatVariable(actionTemplate->getThrowSpeedVariable(), throwSpeed));

	return throwSpeed;
}

// ----------------------------------------------------------------------

bool ThrowSwordActionNamespace::isHit(ThrowSwordAction const &action, PlaybackScript const &script)
{
	ThrowSwordActionTemplate const *const actionTemplate = safe_cast<ThrowSwordActionTemplate const*>(action.getPlaybackActionTemplate());
	NOT_NULL(actionTemplate);

	int damageAmount = 0;
	IGNORE_RETURN(script.getIntVariable(actionTemplate->getDamageVariable(), damageAmount));

	// Determine if the defender is hit by checking if the defender received any damage.
	return (damageAmount > 0);
}

// ----------------------------------------------------------------------

float ThrowSwordActionNamespace::getMissExtentMultiplier(ThrowSwordAction const &action, PlaybackScript const &script)
{
	ThrowSwordActionTemplate const *const actionTemplate = safe_cast<ThrowSwordActionTemplate const*>(action.getPlaybackActionTemplate());
	NOT_NULL(actionTemplate);

	float floatValue = 2.0f;
	IGNORE_RETURN(script.getFloatVariable(actionTemplate->getMissExtentMultiplierVariable(), floatValue));

	return floatValue;
}

// ----------------------------------------------------------------------

float ThrowSwordActionNamespace::getHitExtentMultiplier(ThrowSwordAction const &action, PlaybackScript const &script)
{
	ThrowSwordActionTemplate const *const actionTemplate = safe_cast<ThrowSwordActionTemplate const*>(action.getPlaybackActionTemplate());
	NOT_NULL(actionTemplate);

	float floatValue = 0.75f;
	IGNORE_RETURN(script.getFloatVariable(actionTemplate->getHitExtentMultiplierVariable(), floatValue));

	return floatValue;
}

// ----------------------------------------------------------------------

void ThrowSwordActionNamespace::getHitEventName(ThrowSwordAction const &action, PlaybackScript const &script, CrcString &eventName)
{
	ThrowSwordActionTemplate const *const actionTemplate = safe_cast<ThrowSwordActionTemplate const*>(action.getPlaybackActionTemplate());
	NOT_NULL(actionTemplate);

	bool const gotVariable = script.getStringVariable(actionTemplate->getHitEventNameVariable(), temporaryString);
	if (gotVariable)
		eventName.set(temporaryString.c_str(), false);
	else
		eventName.set("", false);
}

// ----------------------------------------------------------------------

void ThrowSwordActionNamespace::getHitDefenderActionName(ThrowSwordAction const &action, PlaybackScript const &script, CrcString &actionName)
{
	ThrowSwordActionTemplate const *const actionTemplate = safe_cast<ThrowSwordActionTemplate const*>(action.getPlaybackActionTemplate());
	NOT_NULL(actionTemplate);

	bool const gotVariable = script.getStringVariable(actionTemplate->getHitDefenderActionNameVariable(), temporaryString);
	if (gotVariable)
		actionName.set(temporaryString.c_str(), false);
	else
		actionName.set("", false);
}

// ----------------------------------------------------------------------

float ThrowSwordActionNamespace::getCatchTimeDuration(ThrowSwordAction const &action, PlaybackScript const &script)
{
	ThrowSwordActionTemplate const *const actionTemplate = safe_cast<ThrowSwordActionTemplate const*>(action.getPlaybackActionTemplate());
	NOT_NULL(actionTemplate);

	float floatValue = 1.0f;
	IGNORE_RETURN(script.getFloatVariable(actionTemplate->getCatchTimeDurationVariable(), floatValue));

	return floatValue;
}

// ----------------------------------------------------------------------

void ThrowSwordActionNamespace::getCatchSwordActionName(ThrowSwordAction const &action, PlaybackScript const &script, CrcString &actionName)
{
	ThrowSwordActionTemplate const *const actionTemplate = safe_cast<ThrowSwordActionTemplate const*>(action.getPlaybackActionTemplate());
	NOT_NULL(actionTemplate);

	bool const gotVariable = script.getStringVariable(actionTemplate->getCatchSwordActionNameVariable(), temporaryString);
	if (gotVariable)
		actionName.set(temporaryString.c_str(), false);
	else
		actionName.set("", false);
}

// ----------------------------------------------------------------------

void ThrowSwordActionNamespace::addSwoosh(Object *parentObject, CrcString const &startHardPoint, CrcString const &endHardPoint, Object *realSwordObject)
{
	Appearance *parentAppearance = parentObject->getAppearance();

	WeaponObject *weaponObject = dynamic_cast<WeaponObject *>(realSwordObject);

	if ((parentAppearance != NULL) && (weaponObject != NULL))
	{
		Transform m_junkTransform(Transform::IF_none);
		bool const hasStartHardpoint = parentAppearance->findHardpoint(startHardPoint, m_junkTransform);

		if (hasStartHardpoint)
		{
			bool const hasEndHardpoint = parentAppearance->findHardpoint(endHardPoint, m_junkTransform);

			if (hasEndHardpoint)
			{
				//-- Retrieve info on which swoosh to use and which color to apply.
				std::string  swooshFilename;
				VectorArgb   color(VectorArgb::solidWhite);

				// Lookup using the weapon
				CombatEffectsManager::getSwooshEffect(*weaponObject, swooshFilename, color);

				//-- Create the swoosh appearance.
				SwooshAppearance *swooshAppearance = NULL;
				if (swooshFilename.empty())
				{
					const SwooshAppearanceTemplate *swooshAppearanceTemplate = safe_cast<SwooshAppearanceTemplate const *>(AppearanceTemplateList::fetch(SwooshAppearanceTemplate::getDefaultTemplate()));
					swooshAppearance = new SwooshAppearance(swooshAppearanceTemplate, parentAppearance, startHardPoint, endHardPoint);
				}
				else
				{
					const SwooshAppearanceTemplate *swooshAppearanceTemplate = safe_cast<SwooshAppearanceTemplate const *>(AppearanceTemplateList::fetch(swooshFilename.c_str()));
					swooshAppearance = new SwooshAppearance(swooshAppearanceTemplate, parentAppearance, startHardPoint, endHardPoint);
					AppearanceTemplateList::release(swooshAppearanceTemplate);
				}

				NOT_NULL(swooshAppearance);
				swooshAppearance->setColorModifier(color);

				//-- Create the swoosh object.
				Object *swooshObject = new MemoryBlockManagedObject();
				swooshObject->setAppearance(swooshAppearance);
				RenderWorld::addObjectNotifications(*swooshObject);
				parentObject->addChildObject_o(swooshObject);
			} //lint !e429 // custodial pointer 'swooshAppearance' has not been freed or returned // fine: swooshObject owns it.
		}
	}
}

// ======================================================================
// class ThrowSwordAction: PUBLIC MEMBER FUNCTIONS
// ======================================================================

bool ThrowSwordAction::update(float deltaTime, PlaybackScript &script)
{
	UNREF(deltaTime);

	//-- Perform one-time initialization that requires playback script data.
	if (!m_initializedScriptData)
	{
		bool const result = initializeScriptData(script);
		if (!result)
		{
			DEBUG_WARNING(true, ("ThrowSwordAction::update() failed due to initialization error, skipping ThrowSwordAction."));
			return false;
		}

		m_initializedScriptData = true;
	}

	//-- Manage throw-related tasks.
	if (m_headingOutward)
	{
		if (swordWithinDestinationRange())
		{
			// Do activities for hitting destination.
			onDestinationReached(script);

			// Turn around, time to head home.
			m_headingOutward = false;
		}
	}
	else
	{
		// Do activities that occur when the sword is flying inward back toward the thrower.

		// Play the catch animation when necessary.
		if (!m_playedCatchAnimation && swordWithinCatchRange())
		{
			playCatchAnimation(script);
			m_playedCatchAnimation = true;
		}

		// Attach the sword when appropriate.  This action terminates the sequence.
		if (swordWithinAttachRange())
		{
			// This will get called again when the playback script terminates, but this function will do no harm being called multiple times.
			cleanup(script);
			return false;
		}
	}

	// Still going.
	return true;
}

// ----------------------------------------------------------------------

void ThrowSwordAction::cleanup(PlaybackScript &script)
{
	//-- Destroy sword copy.
	Object *const swordCopyObject = m_swordWatcher.getPointer();
	if (swordCopyObject)
	{
		m_swordWatcher = 0;
		swordCopyObject->kill();
	}

	//-- Show the real sword.
	// Get the attacker.
	Object *const attackerObject = getAttackerObject(*this, script);
	if (!attackerObject)
		DEBUG_WARNING(true, ("cleanup(): partially failing because attackerObject is NULL in playback script."));
	else
	{
		// Get the skeletal appearance for the attacker.
		SkeletalAppearance2 *const appearance = safe_cast<SkeletalAppearance2*>(attackerObject->getAppearance());
		NOT_NULL(appearance);

		// Lookup the hardpoint index for the weapon.
		int const attachmentIndex = appearance->findAttachmentIndexByHardpoint(m_attackerHardpointName);
		if (attachmentIndex < 0)
			DEBUG_WARNING(true, ("cleanup(): partially failing because no object is in hardpoint named [%s] on attackerObject [id=%s].", m_attackerHardpointName.getString(), attackerObject->getNetworkId().getValueString().c_str()));
		else
			appearance->hideAttachment(attachmentIndex, false);
	}

	//-- Destroy the attacker child object.
	Object *const attackerChildObject = m_attackerChildWatcher.getPointer();
	if (attackerChildObject)
	{
		m_attackerChildWatcher = 0;
		attackerChildObject->kill();
	}

	//-- Destroy the defender child object.
	Object *const defenderChildObject = m_defenderChildWatcher.getPointer();
	if (defenderChildObject)
	{
		m_defenderChildWatcher = 0;
		defenderChildObject->kill();
	}
}

// ----------------------------------------------------------------------

float ThrowSwordAction::getMaxReasonableUpdateTime() const
{
	//-- Allow this to go for up to this many seconds.  This action has
	//   the potential to take a while depending on how far the blade is traveling.
	return 20.0f;
}

// ======================================================================
// class ThrowSwordAction: PRIVATE MEMBER FUNCTIONS
// ======================================================================

ThrowSwordAction::ThrowSwordAction(ThrowSwordActionTemplate const &actionTemplate) :
	PlaybackAction(&actionTemplate),
	m_initializedScriptData(false),
	m_defenderIsHit(false),
	m_attackerHardpointName(),
	m_headingOutward(true),
	m_playedCatchAnimation(false),
	m_attackerChildWatcher(0),
	m_defenderChildWatcher(0),
	m_swordWatcher(0),
	m_destinationReachedDistanceSquared(0.0f),
	m_playCatchAnimationDistanceSquared(0.0f),
	m_destroySwordDistanceSquared(0.0f)
{
}

// ----------------------------------------------------------------------
/**
 * Initialize action data that requires the playback script.
 *
 * This will:
 *   - create the attacker child hardpoint object (at the thrown weapon hardpoint, probably hold_r).
 *   - create the defender child hardpoint object (at the root, usually a good place to aim for).
 *   - create the duplicate sword object and appearance (we don't use the real one because that is a heavy object that communicates with the server).
 *   - hide the real sword object in the attacker's hand.
 *   - intialize distances from target at which certain events should occur.
 */

bool ThrowSwordAction::initializeScriptData(PlaybackScript &script)
{
	//--
	//-- Create the attacker child target.
	//--

	// Get the attacker object.
	Object *const attackerObject = getAttackerObject(*this, script);
	if (!attackerObject)
	{
		DEBUG_WARNING(true, ("initializeScriptData(): failing because attackerObject is NULL in playback script."));
		return false;
	}

	// Ensure attacker has a skeletal appearance.  If not, there's no way the throw should work.
	SkeletalAppearance2 *const attackerAppearance = dynamic_cast<SkeletalAppearance2*>(attackerObject->getAppearance());
	if (!attackerAppearance)
	{
		DEBUG_WARNING(true, ("initializeScriptData(): failing because attackerObject [id=%s] does not have a skeletal appearance.", attackerObject->getNetworkId().getValueString().c_str()));
		return false;
	}

	// Lookup the sword hardpoint name.
	getHardpointName(*this, script, m_attackerHardpointName);
	if (strlen(m_attackerHardpointName.getString()) < 1)
	{
		DEBUG_WARNING(true, ("initializeScriptData(): failing because sword hardpoint name is zero-length (appears to be unspecified by client combat manager)."));
		return false;
	}

	// Make sure the attacker actually has something in this hardpoint.  This has the side effect of validating that
	// the attacker has the hardpoint in the first place.
	int const attachmentIndex = attackerAppearance->findAttachmentIndexByHardpoint(m_attackerHardpointName);
	if (attachmentIndex < 0)
	{
		DEBUG_WARNING(true, ("initializeScriptData(): failing because no object is in hardpoint named [%s] on attackerObject [id=%s].", m_attackerHardpointName.getString(), attackerObject->getNetworkId().getValueString().c_str()));
		return false;
	}

	// Create the target object on the attacker at the specified hardpoint.
	m_attackerChildWatcher = new HardpointObject(m_attackerHardpointName);
	NOT_NULL(m_attackerChildWatcher.getPointer());

	attackerObject->addChildObject_o(m_attackerChildWatcher.getPointer());
	

	//--
	//-- Create the defender child target.
	//--

	// Get the defender object.
	Object *const defenderObject = getDefenderObject(*this, script);
	if (!defenderObject)
	{
		DEBUG_WARNING(true, ("initializeScriptData(): defender object is NULL, skipping sword throw."));
		return false;
	}

	// Create a target hardpoint object that aims at "root" --- all skeletal-based appearances should have this joint.
	// If it so happens that the target is not a skeletal appearance, the hardpoint object should act just like
	// a pass through to the base of the defender.  This should be acceptable fallback behavior.
	m_defenderChildWatcher = new HardpointObject(ConstCharCrcString("root"));
	NOT_NULL(m_defenderChildWatcher.getPointer());

	defenderObject->addChildObject_o(m_defenderChildWatcher.getPointer());


	//--
	//-- Create the sword copy object that will fly through the air.
	//--

	// Get the real sword object from the attacker.
	Object *const realSwordObject = attackerAppearance->getAttachedObject(attachmentIndex);
	if (!realSwordObject)
	{
		DEBUG_WARNING(true, ("initializeScriptData(): failing because object in hardpoint named [%s] on attackerObject [id=%s] is NULL.", m_attackerHardpointName.getString(), attackerObject->getNetworkId().getValueString().c_str()));
		return false;
	}

	// Hide the real sword.
	attackerAppearance->hideAttachment(attachmentIndex, true);

	// Create the sword copy appearance.
	Appearance const *const realSwordAppearance = realSwordObject->getAppearance();
	AppearanceTemplate const *const swordAppearanceTemplate = realSwordAppearance ? realSwordAppearance->getAppearanceTemplate() : 0;

	if (!swordAppearanceTemplate)
	{
		DEBUG_WARNING(true, ("initializeScriptData(): failing because the sword doesn't have an appearance template."));
		return false;
	}

	Appearance *const swordCopyAppearance = swordAppearanceTemplate->createAppearance();
	NOT_NULL(swordCopyAppearance);

	// Create and keep track of the sword copy object.
	m_swordWatcher = new MemoryBlockManagedObject();

	Object *const swordCopyObject = m_swordWatcher.getPointer();

	// Attach sword to another object that is a child of the sword copy object --- we need to orient the sword 90 degrees off axis.
	Object *const swordCopyAppearanceObject = new MemoryBlockManagedObject();
	swordCopyAppearanceObject->pitch_o(PI_OVER_2);
	swordCopyAppearanceObject->setAppearance(swordCopyAppearance);

	swordCopyObject->addChildObject_o(swordCopyAppearanceObject);

	// Setup the swoosh attachment

	addSwoosh(swordCopyAppearanceObject, TemporaryCrcString("tr1s", false), TemporaryCrcString("tr1e", false), realSwordObject);
	addSwoosh(swordCopyAppearanceObject, TemporaryCrcString("tr2s", false), TemporaryCrcString("tr2e", false), realSwordObject);

	// Get real sword's customization data.
	CustomizationData *swordCustomizationData = NULL;
	CustomizationDataProperty *const property = safe_cast<CustomizationDataProperty*>(realSwordObject->getProperty(CustomizationDataProperty::getClassPropertyId()));
	if (property)
		swordCustomizationData = property->fetchCustomizationData();

	// Set customization data for sword copy appearance object.
	if (swordCustomizationData)
	{
		swordCopyAppearance->setCustomizationData(swordCustomizationData);
		swordCustomizationData->release();
	}

	// Send an event to the sword appearance copy to let it know it should already be initialized in the combat state without a transition.
	swordCopyAppearance->onEvent(ClientGameAppearanceEvents::getOnInitializeCopyEquippedInCombatStateEventId());

	// Get the sword's cell and position prior to hide.
	Transform sword_l2o(Transform::IF_none);
	bool const result = attackerAppearance->findHardpoint(m_attackerHardpointName, sword_l2o);
	if (!result)
	{
		DEBUG_WARNING(true, ("Appearance::findHardpoint() failed to find the sword hardpoint [%s] but we found it before.", m_attackerHardpointName.getString()));
		return false;
	}

	Vector const swordPosition_o = sword_l2o.getPosition_p();
	Vector const swordPosition_w = attackerObject->getTransform_o2w().rotateTranslate_l2p(swordPosition_o);

	CellProperty *const swordCell = attackerObject->getParentCell();

	// Set the sword copy's cell and position.
	swordCopyObject->setParentCell(swordCell);
	swordCopyObject->setPosition_w(swordPosition_w);

	// Allow object to render and to alter.
	swordCopyObject->addNotification(ClientWorld::getIntangibleNotification());

	// Must add parent (non-renderable) object to render world; otherwise, the child object is never rendered.
	// In this case, only the child object has info to render.
	RenderWorld::addObjectNotifications(*swordCopyObject);
	RenderWorld::addObjectNotifications(*swordCopyAppearanceObject);

	// Add the sword to the world.
	swordCopyObject->addToWorld();

	// Get sword dynamics info.
	float const throwSpeed          = getThrowSpeed(*this, script);
	Vector const rotationRateVector = Vector(0.0f, getYawRateInRadians(*this, script), 0.0f);

	// Set the sword copy's dynamics.
	swordCopyObject->setDynamics(new TrackingDynamics(swordCopyObject, m_defenderChildWatcher.getPointer(), throwSpeed, rotationRateVector));

	//--
	//-- Initialize trigger distances.
	//--

	// Get appearance radii.
	float const defenderRadius = defenderObject->getAppearanceSphereRadius();
	float const swordRadius    = swordCopyObject->getAppearanceSphereRadius();

	// Find out if defender is hit.
	m_defenderIsHit = isHit(*this, script);

	// Setup distance between sword and defender when sword has reached its final destination.
	if (m_defenderIsHit)
		m_destinationReachedDistanceSquared = swordRadius + defenderRadius * getHitExtentMultiplier(*this, script);
	else
		m_destinationReachedDistanceSquared = swordRadius + defenderRadius * getMissExtentMultiplier(*this, script);
	m_destinationReachedDistanceSquared *= m_destinationReachedDistanceSquared;

	// Determine distance between sword and the thrower before the catch animation should be played.
	m_playCatchAnimationDistanceSquared = throwSpeed * getCatchTimeDuration(*this, script);
	m_playCatchAnimationDistanceSquared *= m_playCatchAnimationDistanceSquared;

	// Determine distance between the sword and the thrower before destroying the sword.
	m_destroySwordDistanceSquared = 0.5f;
	m_destroySwordDistanceSquared *= m_destroySwordDistanceSquared;

	// Successful initialization.
	return true;
}

// ----------------------------------------------------------------------

bool ThrowSwordAction::swordWithinDestinationRange() const
{
	//-- Get the sword.
	Object const *const swordObject = m_swordWatcher.getPointer();
	if (!swordObject)
		return true;

	//-- Check distance to target.
	TrackingDynamics const *dynamics = safe_cast<TrackingDynamics const*>(swordObject->getDynamics());
	NOT_NULL(dynamics);

	float const distance = dynamics->getDistanceToTargetSquared();

	//-- Check if distance is close enough.
	//   NOTE: if frame rate is choppy, we could get "too close" since we're only
	//         checking if we got close enough.
	return distance <= m_destinationReachedDistanceSquared;
}

// ----------------------------------------------------------------------

void ThrowSwordAction::onDestinationReached(PlaybackScript &script) const
{
	Object *const swordObject = m_swordWatcher.getPointer();

	if (m_defenderIsHit)
	{
		//-- Play client effect on sword for hitting a target.  NOTE: we may want to change this so it plays on the defender, but then the question is where.
		if (swordObject)
		{
			// Get the client event name to trigger.
			TemporaryCrcString  eventName;
			getHitEventName(*this, script, eventName);

			if (strlen(eventName.getString()) < 1)
				DEBUG_WARNING(true, ("onDestinationReached(): failed to trigger sword throw hit event because no hit event name specified in playback script."));
			else
			{
				// @todo redo client event/effect manager to use CrcString instead of CrcLowerString.
				IGNORE_RETURN(ClientEventManager::playEvent(CrcLowerString(eventName.getString()), swordObject, CrcLowerString::empty));
			}
		}

		//-- Play defender hit animation.
		// Create a new thread for the defender got hit animation.
		int const newThreadIndex = script.addThread();

		// Get the defender actor index.
		ThrowSwordActionTemplate const *const actionTemplate = safe_cast<ThrowSwordActionTemplate const*>(getPlaybackActionTemplate());
		NOT_NULL(actionTemplate);

		int const defenderActorIndex = actionTemplate->getDefenderActorIndex();

		// Create PlaySkeletalAnimation action for defender's reaction (plays an animation controller action).
		//lint -esym(429, playDefenderActionTemplate) // Custodial pointer 'actionTemplate' not freed or returned // It's okay, it is ref counted.
		PlaybackActionTemplate *const playDefenderActionTemplate = new PlaySkeletalAnimationActionTemplate(defenderActorIndex, TAG_DHAC, true, TAG_DEPS, TAG_NONE);
		playDefenderActionTemplate->fetch();

		script.addAction(newThreadIndex, playDefenderActionTemplate->createPlaybackAction());

		// Release local references.
		playDefenderActionTemplate->release();
	}

	//-- Change the target of the thrown sword to the player.
	// Get the sword dynamics.
	if (swordObject)
	{
		TrackingDynamics *const dynamics = safe_cast<TrackingDynamics*>(swordObject->getDynamics());
		NOT_NULL(dynamics);

		// Change sword target to attacker.
		dynamics->setTarget(m_attackerChildWatcher.getPointer());
	}
}

// ----------------------------------------------------------------------

bool ThrowSwordAction::swordWithinCatchRange() const
{
	//-- Get the sword.
	Object const *const swordObject = m_swordWatcher.getPointer();
	if (!swordObject)
		return true;

	//-- Check distance to target.
	TrackingDynamics const *dynamics = safe_cast<TrackingDynamics const*>(swordObject->getDynamics());
	NOT_NULL(dynamics);

	float const distance = dynamics->getDistanceToTargetSquared();

	//-- Check if distance is close enough.
	//   NOTE: if frame rate is choppy, we could get "too close" since we're only
	//         checking if we got close enough.
	return distance <= m_playCatchAnimationDistanceSquared;
}

// ----------------------------------------------------------------------

void ThrowSwordAction::playCatchAnimation(PlaybackScript &script) const
{
	//-- Retrieve the name of the catch animation action.
	TemporaryCrcString  actionName;
	getCatchSwordActionName(*this, script, actionName);

	if (strlen(actionName.getString()) < 1)
		return;

	//-- Play the action on the attacker.
	// Get the attacker.
	Object *const attackerObject = getAttackerObject(*this, script);
	if (!attackerObject)
	{
		DEBUG_WARNING(true, ("playCatchAnimation(): failing because attackerObject is NULL in playback script."));
		return;
	}

	// Get the skeletal appearance for the attacker.
	SkeletalAppearance2 *const appearance = safe_cast<SkeletalAppearance2*>(attackerObject->getAppearance());
	if (!appearance)
	{
		DEBUG_WARNING(true, ("playCatchAnimation(): failing because attackerObject [id=%s] does not have a skeletal appearance.", attackerObject->getNetworkId().getValueString().c_str()));
		return;
	}

	// Play the animation action.
	int   animationId;
	bool  animationIsAdd;

	// @todo convert interface to CrcString, get rid of temporary CrcLowerString.
	appearance->getAnimationResolver().playAction(CrcLowerString(actionName.getString()), animationId, animationIsAdd, NULL);
}

// ----------------------------------------------------------------------

bool ThrowSwordAction::swordWithinAttachRange() const
{
	//-- Get the sword.
	Object const *const swordObject = m_swordWatcher.getPointer();
	if (!swordObject)
		return true;

	//-- Check distance to target.
	TrackingDynamics const *dynamics = safe_cast<TrackingDynamics const*>(swordObject->getDynamics());
	NOT_NULL(dynamics);

	float const distance = dynamics->getDistanceToTargetSquared();

	//-- Check if distance is close enough.
	//   NOTE: if frame rate is choppy, we could get "too close" since we're only
	//         checking if we got close enough.
	return distance <= m_destroySwordDistanceSquared;
}

// ======================================================================
