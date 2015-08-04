// ======================================================================
//
// FireSetupActionTemplate.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/FireSetupActionTemplate.h"

#include "clientAnimation/PassthroughPlaybackAction.h"
#include "clientAnimation/PlaybackScript.h"
#include "clientAnimation/PlaybackScriptTemplate.h"
#include "clientAnimation/WatcherAction.h"
#include "clientGame/ClientCombatActionInfo.h"
#include "clientGame/FireProjectileActionTemplate.h"
#include "clientGame/PlaySkeletalAnimationAction.h"
#include "clientGame/PlaySkeletalAnimationActionTemplate.h"
#include "clientGame/WaitForBoltActionTemplate.h"
#include "clientSkeletalAnimation/AnimationMessageAction.h"
#include "clientSkeletalAnimation/CallbackAnimationNotification.h"
#include "clientSkeletalAnimation/SkeletalAnimation.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/StateHierarchyAnimationController.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedObject/Object.h"
#include "sharedRandom/Random.h"

#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <vector>

// ======================================================================

bool FireSetupActionTemplate::ms_installed;

// ======================================================================

const char *const cms_fireMessagePrefix       = "fire";
const int         cms_fireMessagePrefixLength = 4;

//This magic number corresponds to an entry in the muzzle table for "whp1" which is the jedi lightsaber hardpoint
#define LIGHTSABER_MUZZLE_INDEX 4

const Tag         TAG_DAMG                    = TAG(D,A,M,G);
const Tag         TAG_DAMT                    = TAG(D,A,M,T);
const Tag         TAG_DDFS                    = TAG(D,D,F,S);
const Tag         TAG_DEPS                    = TAG(D,E,P,S); // Defender End PoSture
const Tag         TAG_DHAC                    = TAG(D,H,A,C); // Defender hit action
const Tag         TAG_DMAC                    = TAG(D,M,A,C); // Defender miss action
const Tag         TAG_DMAP                    = TAG(D,M,A,P); // Defender play miss action probability
const Tag         TAG_FISU                    = TAG(F,I,S,U);
const Tag         TAG_NONE                    = TAG(N,O,N,E);
const Tag         TAG_PNAM                    = TAG(P,N,A,M);
const Tag         TAG_PVEL                    = TAG(P,V,E,L);
const Tag         TAG_DHLC                    = TAG(D,H,L,C); // Defender Hit LoCation

namespace
{
	bool  ms_logInfo = true;

}

// ======================================================================
// class FireSetupActionTemplate: public static member functions
// ======================================================================

void FireSetupActionTemplate::install()
{
	InstallTimer const installTimer("FireSetupActionTemplate::install");

	DEBUG_FATAL(ms_installed, ("FireSetupActionTemplate already installed"));

	PlaybackScriptTemplate::registerActionTemplate(TAG_FISU, create);

	ms_installed = true;
	ExitChain::add(remove, "FireSetupActionTemplate");
}

// ======================================================================
// class FireSetupActionTemplate: public member functions
// ======================================================================

PlaybackAction *FireSetupActionTemplate::createPlaybackAction() const
{
	return new PassthroughPlaybackAction(*this);
}

// ----------------------------------------------------------------------

bool FireSetupActionTemplate::update(float deltaTime, PlaybackScript &script) const
{
	UNREF(deltaTime);

	//-- Count the number of fire* messages in the attacker animation.
	CallbackAnimationNotification *notification = 0;
	IntVector                      muzzleIndices(8);
	bool                           messageNamesIncludeMuzzle;
	int                            fireCount = 0;

	bool const gfiSuccess = getFireInfo(script, fireCount, muzzleIndices, messageNamesIncludeMuzzle, &notification);
	if (!gfiSuccess || !notification)
	{
		DEBUG_WARNING(PlaybackScriptNamespace::s_verbosePlaybackScriptWarnings, ("FireSetupActionTemplate: failed to retrieve fire-related info from animation for script %s, aborting.", script.getPlaybackScriptTemplateName()));
		if (notification)
		{
			notification->release();
			notification = 0;
		}

		script.abortPlayback();
		return false;
	}

	if (fireCount == 0)
	{
		// Abort if no fire animations.
		DEBUG_REPORT_LOG(ms_logInfo, ("FireSetupActionTemplate: attacker fire animation doesn't signal any fire messages in script %s, playing defender action.\n", script.getPlaybackScriptTemplateName()));
		addNoFireActions(script);

		notification->release();
		notification = 0;

		return false;
	}

	int defenderDisposition;
	if(script.getIntVariable(TAG_DDFS, defenderDisposition))
	{
		if(defenderDisposition == static_cast<int>(ClientCombatActionInfo::DD_lightsaberBlock) ||
		   defenderDisposition == static_cast<int>(ClientCombatActionInfo::DD_lightsaberCounter) ||
		   defenderDisposition == static_cast<int>(ClientCombatActionInfo::DD_lightsaberCounterTarget)
		   )
			fireCount = 1;
	}

	//-- Determine if attack succeeded at all.
	const int  totalDamageAmount  = getDefenderDamageAmount(script);
	const bool isAttackSuccessful = (totalDamageAmount > 0);

	//-- Distribute the damage done by each fire.
	IntVector  perFireDamageAmount(fireCount, 0);
	calculateDamagePerFire(fireCount, totalDamageAmount, perFireDamageAmount);

	//-- Get probability of defender playing miss action from script vars.
	float defenderMissActionProbability;

	if (!script.getFloatVariable(TAG_DMAP, defenderMissActionProbability))
		defenderMissActionProbability = 0.25f;

	//-- For each fire, add actions to thread to handle hit/miss synchronization.
	for (int i = 0; i < fireCount; ++i)
	{
		const int damageForFire = perFireDamageAmount[static_cast<IntVector::size_type>(i)];
		const int muzzleIndex   = muzzleIndices[static_cast<IntVector::size_type>(i)];

		if (damageForFire > 0)
			addSuccessfulFire(script, messageNamesIncludeMuzzle, i + 1, muzzleIndex, notification);
		else
		{
			//-- Figure out if I play the defender miss animation.  This happens when
			//   the attack is not successful and this is the first projectile bolt.
			bool playMissAnimation;

			switch (static_cast<FireProjectileActionTemplate::TargetMode>(m_targetMode))
			{
				case FireProjectileActionTemplate::TM_animationTargetTrackRicochet:
					// Never play the miss animation for these type of targets --- the animation has already been
					// played by the PST.
					playMissAnimation = false;
					break;

				case FireProjectileActionTemplate::TM_normal:
				case FireProjectileActionTemplate::TM_animationTargetNoTrack:
				default:
					playMissAnimation = (!isAttackSuccessful && (i == 0));
					break;
			}

			addMissedFire(script, messageNamesIncludeMuzzle, i + 1, muzzleIndex, playMissAnimation, notification);
		}
	}

	//-- Release local references.
	if (notification)
	{
		notification->release();
		notification = 0;
	}

	//-- Indicate to caller that this script action is complete.
	return false;
}

// ======================================================================
// class FireSetupActionTemplate: private static member functions
// ======================================================================

void FireSetupActionTemplate::remove()
{
	DEBUG_FATAL(!ms_installed, ("FireSetupActionTemplate not installed."));

	ms_installed = false;
	PlaybackScriptTemplate::deregisterActionTemplate(TAG_FISU);
}

// ----------------------------------------------------------------------

PlaybackActionTemplate *FireSetupActionTemplate::create(Iff &iff)
{
	return new FireSetupActionTemplate(iff);
}

// ======================================================================
// class FireSetupActionTemplate: private member functions
// ======================================================================

FireSetupActionTemplate::FireSetupActionTemplate(Iff &iff) :
	PlaybackActionTemplate(),
	m_attackerActorIndex(-1),
	m_defenderActorIndex(-1),
	m_targetMode(static_cast<int>(FireProjectileActionTemplate::TM_normal)),
	m_animationThreadIndex(-1),
	m_animationActionIndex(-1)

{
	DEBUG_FATAL(!ms_installed, ("FireSetupActionTemplate not installed."));

	iff.enterForm(TAG_FISU);

		const Tag version = iff.getCurrentName();
		switch (version)
		{
			case TAG_0002:
				load_0002(iff);
				break;

			default:
				{
					char buffer[5];
					ConvertTagToString(version, buffer);
					FATAL(true, ("FireSetupActionTemplate: unsupported version [%s].", buffer));
				}
		}

	iff.exitForm(TAG_FISU);
}

// ----------------------------------------------------------------------

void FireSetupActionTemplate::load_0002(Iff &iff)
{
	iff.enterForm(TAG_0002);

		iff.enterChunk(TAG_INFO);

			m_attackerActorIndex   = static_cast<int>(iff.read_int16());
			m_defenderActorIndex   = static_cast<int>(iff.read_int16());
			m_targetMode           = static_cast<int>(iff.read_int16());
			m_animationThreadIndex = static_cast<int>(iff.read_int16());
			m_animationActionIndex = static_cast<int>(iff.read_int16());

		iff.exitChunk(TAG_INFO);

	iff.exitForm(TAG_0002);
}

// ----------------------------------------------------------------------

void FireSetupActionTemplate::calculateDamagePerFire(int fireCount, int totalDamageAmount, IntVector &perFireDamageAmount) const
{
	if (perFireDamageAmount.empty())
	{
		WARNING_STRICT_FATAL(true, ("calculateDamagePerFire() called with empty perFireDamageAmount."));
		return;
	}

	//-- Check for no-damage (i.e. a miss).
	if (totalDamageAmount <= 0)
	{
		// Assume perFireDamageAmount is initialized with zero.
		return;
	}

	int index = 0;

	switch (static_cast<FireProjectileActionTemplate::TargetMode>(m_targetMode))
	{
		case FireProjectileActionTemplate::TM_normal:
			//-- Apply all damage to one of the random entries.  I do this because combat can cause posture-changing
			//   animations to play on the defender.  If the defender got hit multiple times and played the specified animation
			//   for each, it would look wrong because it is posture changing.  Also, I don't have enough information at
			//   this stage to know whether the defender animation causes a posture change; therefore, I couldn't even play an alternate
			//   non-posture-changing animation and then have the last hit animation play the state changing animation.

			// Choose random fire.
			index = Random::random(static_cast<int>(perFireDamageAmount.size() - 1));
			break;

		case FireProjectileActionTemplate::TM_animationTargetNoTrack:
			//-- When using the animation target (e.g. arc target, homing target shot), only the last
			//   shot should successfully hit the target.  Otherwise, the attacker will follow some kind
			//   of progressive firing pattern, "hop" to the defender as target on the successful hit that's
			//   not the last shot, then "hop back" to the progressive firing pattern.
			index = static_cast<int>(perFireDamageAmount.size()) - 1;
			break;

		case FireProjectileActionTemplate::TM_animationTargetTrackRicochet:
		case FireProjectileActionTemplate::TM_animationTargetTrackReflect:
		case FireProjectileActionTemplate::TM_animationTargetTrackReflectTarget:
		default:
			//-- Ricochet never assigns damage to anything.
			return;
	}

	// Apply all damage to the chosen projectile index.
	VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(0, index, static_cast<int>(perFireDamageAmount.size() - 1));
	perFireDamageAmount[static_cast<IntVector::size_type>(index)] = totalDamageAmount;

	UNREF(fireCount);
}

// ----------------------------------------------------------------------

int FireSetupActionTemplate::getAttackerActorIndex() const
{
	return m_attackerActorIndex;
}

// ----------------------------------------------------------------------

int FireSetupActionTemplate::getDefenderActorIndex() const
{
	return m_defenderActorIndex;
}

// ----------------------------------------------------------------------
/**
 * Count the number of times the attacker will fire in an animation & the muzzle index for each fired projectile.
 *
 * This function finds the message of the form "fireX" where X is an
 * integer.  Firing is assumed to start with message "fire1" and end
 * with "fireY".  Y is the number of times an animation is fired.
 *
 * @param script          The playback script on which this action is playing.
 * @param fireCount       The number of times a fire event occurs in the animation.
 * @param muzzleIndices   For each fire that occurs, the muzzleIndex for the fire event
 *                        is stored here.
 */

bool FireSetupActionTemplate::getFireInfo(PlaybackScript &script, int &fireCount, IntVector &muzzleIndices, bool &messageNamesIncludeMuzzle, CallbackAnimationNotification **notification) const
{
	NOT_NULL(notification);

	//-- Nothing to fire if attacker is not active.
	if (!script.isActorActive(m_attackerActorIndex))
	{
		DEBUG_REPORT_LOG(ms_logInfo, ("FireSetupActionTemplate: Attacker object not active, no projectiles fired for script %s.\n", script.getPlaybackScriptTemplateName()));
		fireCount = 0;
		return false;
	}

	//-- Get animation notification from PlaySkeletalAnimationAction command.
	PlaybackAction * const a = script.getAction(m_animationThreadIndex, m_animationActionIndex);
	PlaySkeletalAnimationAction *const action = dynamic_cast<PlaySkeletalAnimationAction*>(a);
	if (!action)
	{
		DEBUG_WARNING(PlaybackScriptNamespace::s_verbosePlaybackScriptWarnings, ("FireSetupActionTemplate: PlaySkeletlAnimationAction not at thread=[%d], action=[%d] in playback script %s, canceling playback.", m_animationThreadIndex, m_animationActionIndex, script.getPlaybackScriptTemplateName()));
		return false;
	}

	*notification = action->fetchNotification();
	if (!*notification)
	{
		DEBUG_WARNING(PlaybackScriptNamespace::s_verbosePlaybackScriptWarnings, ("FireSetupActionTemplate: PlaySkeletlAnimationAction returned a NULL animation notification, canceling playback for script %s.", script.getPlaybackScriptTemplateName()));
		return false;
	}

	if ((*notification)->getChannelCount() < 1)
	{
		DEBUG_WARNING(PlaybackScriptNamespace::s_verbosePlaybackScriptWarnings, ("FireSetupActionTemplate: PlaySkeletlAnimationAction returned an animation notification with no channels available, canceling playback for script %s.", script.getPlaybackScriptTemplateName()));
		return false;
	}

	SkeletalAnimation *const animation = (*notification)->fetchAnimation(0);
	if (!animation)
	{
		if(PlaybackScriptNamespace::s_verbosePlaybackScriptWarnings)
		{		
			DEBUG_WARNING(true, ("Got null animation for attacker's action from script [%s], no projectiles fired.", script.getPlaybackScriptTemplateName()));
			(*notification)->debugDump();
			action->debugDump();
		}

		return false;
	}

	// Resolve animation to leaf-most node.
	SkeletalAnimation *nextAnimation     = animation;
	SkeletalAnimation *resolvedAnimation = animation;

	while (nextAnimation)
	{
		resolvedAnimation = nextAnimation;
		nextAnimation     = nextAnimation->resolveSkeletalAnimation();
	}

	NOT_NULL(resolvedAnimation);

	//-- Scan skeletal animation for messages following the pattern "fireX".  Keep track of the largest X, where X is a base-10 integer.
	char buffer[64];
	int  count = 0;

	const int messageCount = resolvedAnimation->getMessageCount();
	for (int i = 0; i < messageCount; ++i)
	{
		// Get the message name.
		const CrcLowerString &messageName  = resolvedAnimation->getMessageName(i);
		const char *const     cMessageName = messageName.getString();

		// Check if name starts with "fire".
		if ((strlen(cMessageName) >= cms_fireMessagePrefixLength) && (strncmp(cms_fireMessagePrefix, messageName.getString(), cms_fireMessagePrefixLength) == 0))
		{
			// Fire event names follow one of these formats:
			//     fire<X>_<Y>
			//       where X = monotonically increasing projectile number, starting with 1 (e.g. fire1_1, fire2_2, fire3_1)
			//             Y = number of the muzzle at which the projectile should be fired.  A projectile firing from:
			//                   muzzle1 (fire<X>_1) fires from the joint/hardpoint named "muzzle"   <== note this is "muzzle", not "muzzle1".
			//                   muzzle2 (fire<X>_2) fires from the joint/hardpoint named "muzzle2"
			//                   muzzle2 (fire<X>_3) fires from the joint/hardpoint named "muzzle3"
			//                   ...
			//
			//     fire<X>
			//       Just like above, but all projectiles come from a hardpoint named "muzzle".
			//
			//     All fire messages in an animation must follow the same format (i.e. must all include muzzle information or all not include it).

			//-- Extract projectile number.
			const char *const startProjectileNumber = messageName.getString() + cms_fireMessagePrefixLength;
			const char *const endProjectileNumber   = strrchr(startProjectileNumber, '_');
			const char *      scanString;

			if (endProjectileNumber)
			{
				//-- Handle embedded
				const size_t copyCount = static_cast<size_t>(endProjectileNumber - startProjectileNumber);
				strncpy(buffer, startProjectileNumber, copyCount);
				buffer[copyCount] = 0;

				scanString                = buffer;
				messageNamesIncludeMuzzle = true;
			}
			else
			{
				scanString = startProjectileNumber;
				messageNamesIncludeMuzzle = false;
			}

			//-- Convert projectile number to int.
			const int fireNumber = atoi(scanString);
			count                = std::max(count, fireNumber);

			//-- Lookup muzzle index.  (Muzzle index is zero-based, muzzle number embedded in fire message is 1-based.)
			const int muzzleIndex = (endProjectileNumber ? atoi(endProjectileNumber + 1) : 1) - 1;

			//-- Store the muzzle number in the return array for the given projectile number.
			if (static_cast<int>(muzzleIndices.size()) < fireNumber)
				muzzleIndices.resize(static_cast<IntVector::size_type>(2 * fireNumber));

			muzzleIndices[static_cast<IntVector::size_type>(fireNumber - 1)] = muzzleIndex;
		}
	}

	//-- Release local reference.
	animation->release();

	//-- Return count.
	fireCount = count;
	return true;
}

// ----------------------------------------------------------------------

int FireSetupActionTemplate::getDefenderDamageAmount(const PlaybackScript &script) const
{
	int totalDamageAmount = 0;

	IGNORE_RETURN(script.getIntVariable(TAG_DAMG, totalDamageAmount));
	return totalDamageAmount;
}

// ----------------------------------------------------------------------

int FireSetupActionTemplate::getDefenderDamageType(const PlaybackScript &script) const
{
	int damageType = 0;

	IGNORE_RETURN(script.getIntVariable(TAG_DAMT, damageType));
	return damageType;
}

// ----------------------------------------------------------------------

int FireSetupActionTemplate::getDefenderHitLocation(const PlaybackScript &script) const
{
	int hitLocation = 0;

	IGNORE_RETURN(script.getIntVariable(TAG_DHLC, hitLocation));
	return hitLocation;
}

// ----------------------------------------------------------------------

void FireSetupActionTemplate::addSuccessfulFire(PlaybackScript &script, bool messageNamesIncludeMuzzle, int fireNumber, int muzzleIndex, CallbackAnimationNotification *notification) const
{
	//-- Each fire and wait action goes on its own thread so that one set of fire actions can't
	//   block another set of fire actions from occuring.  If all actions were on the same thread,
	//   one set of firing could fail to finish before the next set should go off.  This would be
	//   catastrophic failure as the second set of fire actions never would get triggered.
	//   The culprit in the successful attack scenario is the WatcherAction() that waits for the
	//   attack to hit.  There is no culprit in the miss action, but since this might be added
	//   later, I am setting it up to use a separate thread as well.
	const int newThreadIndex = script.addThread();

	//-- Get the attacker and defender actor index.
	const int attackerActorIndex = getAttackerActorIndex();
	const int defenderActorIndex = getDefenderActorIndex();

	const int defenderHitLocation = getDefenderHitLocation(script);

	//-- Create AnimationMessageWaitAction to wait for the fireX message, add to script actions.
	char messageName[32];

	if (messageNamesIncludeMuzzle)
		sprintf(messageName, "fire%d_%d", fireNumber, muzzleIndex + 1);
	else
		sprintf(messageName, "fire%d", fireNumber);

	//-- Create fire projectile action, failure mode, add to script actions.
	// Get index for thread action that follows the FireProjectileActionTemplate action.  I need this to wait for playing the defender "got hit" animation.
	const int watcherActionIndex = script.getActionCount(newThreadIndex) + 1;

	//lint -esym(429, fireActionTemplate) // Custodial pointer 'actionTemplate' not freed or returned // It's okay, it is ref counted.
	FireProjectileActionTemplate *const fireActionTemplate = new FireProjectileActionTemplate(true, attackerActorIndex, muzzleIndex, TAG_PNAM, TAG_PVEL, defenderActorIndex, watcherActionIndex, newThreadIndex, static_cast<FireProjectileActionTemplate::TargetMode>(m_targetMode), false, defenderHitLocation);
	fireActionTemplate->fetch();

	script.addAction(newThreadIndex, fireActionTemplate->createPlaybackAction(notification, messageName));

	// Release local references.
	fireActionTemplate->release();

	//-- Create watcher action to wait for projectile death.  FireProjectileActionTemplate will set the projectile Object instance as the watcher target.
	script.addAction(newThreadIndex, new WatcherAction());

	//-- Create PlaySkeletalAnimation action for defender's reaction (plays an animation controller action).
	//lint -esym(429, playDefenderActionTemplate) // Custodial pointer 'actionTemplate' not freed or returned // It's okay, it is ref counted.
	PlaybackActionTemplate *const playDefenderActionTemplate = new PlaySkeletalAnimationActionTemplate(defenderActorIndex, TAG_DHAC, true, TAG_DEPS, TAG_NONE);
	playDefenderActionTemplate->fetch();

	script.addAction(newThreadIndex, playDefenderActionTemplate->createPlaybackAction());

	// Release local references.
	playDefenderActionTemplate->release();
}

// ----------------------------------------------------------------------

void FireSetupActionTemplate::addMissedFire(PlaybackScript &script, bool messageNamesIncludeMuzzle, int fireNumber, int muzzleIndex, bool playDefenderMissAction, CallbackAnimationNotification *notification) const
{
	//-- Each fire and wait action goes on its own thread so that one set of fire actions can't
	//   block another set of fire actions from occuring.  If all actions were on the same thread,
	//   one set of firing could fail to finish before the next set should go off.  This would be
	//   catastrophic failure as the second set of fire actions never would get triggered.
	//   The culprit in the successful attack scenario is the WatcherAction() that waits for the
	//   attack to hit.  There is no culprit in the miss action, but since this might be added
	//   later, I am setting it up to use a separate thread as well.
	int newThreadIndex = script.addThread();

	//-- Get the attacker actor index.
	const int attackerActorIndex = getAttackerActorIndex();
	const int defenderActorIndex = getDefenderActorIndex();

	//-- Create AnimationMessageWaitAction to wait for the fireX message, add to script actions.
	char messageName[32];

	if (messageNamesIncludeMuzzle)
		sprintf(messageName, "fire%d_%d", fireNumber, muzzleIndex + 1);
	else
		sprintf(messageName, "fire%d", fireNumber);

	//-- Create fire projectile action, failure mode, add to script actions.
	//lint -esym(429, actionTemplate) // Custodial pointer 'actionTemplate' not freed or returned // It's okay, it is ref counted.
	FireProjectileActionTemplate *actionTemplate = 0;

	const int defenderHitLocation = getDefenderHitLocation(script);

	switch (static_cast<FireProjectileActionTemplate::TargetMode>(m_targetMode))
	{
		case FireProjectileActionTemplate::TM_normal:
			{
				//-- Normal miss.
				actionTemplate = new FireProjectileActionTemplate(false, attackerActorIndex, muzzleIndex, TAG_PNAM, TAG_PVEL, defenderActorIndex, -1, newThreadIndex, static_cast<FireProjectileActionTemplate::TargetMode>(m_targetMode), false, defenderHitLocation);

				//-- Have target play miss animation immediately if specified.
				if (playDefenderMissAction)
				{
					//lint -esym(429, playDefenderActionTemplate) // Custodial pointer 'actionTemplate' not freed or returned // It's okay, it is ref counted.
					PlaybackActionTemplate *const playDefenderActionTemplate = new PlaySkeletalAnimationActionTemplate(defenderActorIndex, TAG_DMAC, false, TAG_DEPS, TAG_NONE);
					playDefenderActionTemplate->fetch();

					script.addAction(newThreadIndex, playDefenderActionTemplate->createPlaybackAction());

					// Release local references.
					playDefenderActionTemplate->release();
				}
			}
			break;
		
		case FireProjectileActionTemplate::TM_animationTargetTrackReflect:
		case FireProjectileActionTemplate::TM_animationTargetTrackReflectTarget:
			{
				bool isReflectTarget = (static_cast<FireProjectileActionTemplate::TargetMode>(m_targetMode)) == FireProjectileActionTemplate::TM_animationTargetTrackReflectTarget;
				//-- The attacker missed, but the attacker actually is going to "hit" the CreatureController's animation target.
				//   Signal this by passing in a -1 defender with a successful hit.
				//   @todo this is feeling very hacky.
				const int fakeDefenderIndex = -1;
				actionTemplate              = new FireProjectileActionTemplate(true, attackerActorIndex, muzzleIndex, TAG_PNAM, TAG_PVEL, fakeDefenderIndex , -1, newThreadIndex, static_cast<FireProjectileActionTemplate::TargetMode>(m_targetMode));
				
				actionTemplate->fetch();
				script.addAction(newThreadIndex, actionTemplate->createPlaybackAction(notification, messageName));
				
				FireProjectileActionTemplate *reflectActionTemplate = 0;
				WaitForBoltActionTemplate *waitActionTemplate = 0;

				//newThreadIndex = script.addThread();
				waitActionTemplate          = new WaitForBoltActionTemplate(defenderActorIndex, attackerActorIndex, false);

				waitActionTemplate->fetch();
				script.addAction(newThreadIndex, waitActionTemplate->createPlaybackAction());
				
				//newThreadIndex = script.addThread();
				reflectActionTemplate       = new FireProjectileActionTemplate(true, defenderActorIndex, LIGHTSABER_MUZZLE_INDEX, TAG_PNAM, TAG_PVEL, attackerActorIndex, -1, newThreadIndex, FireProjectileActionTemplate::TM_normal, isReflectTarget);				
				
				reflectActionTemplate->fetch();				
				script.addAction(newThreadIndex, reflectActionTemplate->createPlaybackAction(notification, messageName));
				
				//-- Release local references.
				actionTemplate->release();
				reflectActionTemplate->release();
				waitActionTemplate->release();

				return;

			}
			break;
		
		case FireProjectileActionTemplate::TM_animationTargetNoTrack:
		case FireProjectileActionTemplate::TM_animationTargetTrackRicochet:
		default:
			{
				//-- The attacker missed, but the attacker actually is going to "hit" the CreatureController's animation target.
				//   Signal this by passing in a -1 defender with a successful hit.
				//   @todo this is feeling very hacky.
				const int fakeDefenderIndex = -1;
				actionTemplate              = new FireProjectileActionTemplate(true, attackerActorIndex, muzzleIndex, TAG_PNAM, TAG_PVEL, fakeDefenderIndex , -1, newThreadIndex, static_cast<FireProjectileActionTemplate::TargetMode>(m_targetMode), false, defenderHitLocation);
			}
			break;
			
		
	}

	actionTemplate->fetch();

	script.addAction(newThreadIndex, actionTemplate->createPlaybackAction(notification, messageName));

	//-- Release local references.
	actionTemplate->release();
}

// ----------------------------------------------------------------------
/**
 * Add the actions to the script that should be carried out if the
 * fire animation doesn't actually generate fire messages.
 *
 * This is the scenario hit when the attacker's animation doesn't exist
 * or is malformed.  One common reason for this happening is if the attacker
 * has somehow traversed to a posture where the firing animation simply
 * doesn't exist.  We should make sure that the defender's hit or miss
 * animation is played so that required posture changes take place regardless
 * of the attacker's posture.
 *
 * @param script  the playback script under consideration.
 */

void FireSetupActionTemplate::addNoFireActions(PlaybackScript &script) const
{
	//-- Have the defender animation play immediately.
	const int newThreadIndex = script.addThread();

	//-- Get the defender actor index.
	const int defenderActorIndex = getDefenderActorIndex();

	//-- Determine whether defender is hit.
	const bool defenderIsHit          = (getDefenderDamageAmount(script) > 0);
	const Tag  defenderActionVariable = defenderIsHit ? TAG_DHAC : TAG_DMAC;

	//-- Create PlaySkeletalAnimation action for defender's reaction (plays an animation controller action).
	//lint -esym(429, playDefenderActionTemplate) // Custodial pointer 'actionTemplate' not freed or returned // It's okay, it is ref counted.
	PlaybackActionTemplate *const playDefenderActionTemplate = new PlaySkeletalAnimationActionTemplate(defenderActorIndex, defenderActionVariable, true, TAG_DEPS, TAG_NONE);
	playDefenderActionTemplate->fetch();

	script.addAction(newThreadIndex, playDefenderActionTemplate->createPlaybackAction());

	// Release local references.
	playDefenderActionTemplate->release();
}

// ======================================================================
