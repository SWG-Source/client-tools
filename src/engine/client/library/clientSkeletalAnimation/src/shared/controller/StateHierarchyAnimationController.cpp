// ======================================================================
//
// StateHierarchyAnimationController.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/StateHierarchyAnimationController.h"

#include "clientSkeletalAnimation/AnimationAction.h"
#include "clientSkeletalAnimation/AnimationEnvironment.h"
#include "clientSkeletalAnimation/AnimationStateHierarchyTemplate.h"
#include "clientSkeletalAnimation/AnimationState.h"
#include "clientSkeletalAnimation/AnimationStateLink.h"
#include "clientSkeletalAnimation/AnimationStateNameIdManager.h"
#include "clientSkeletalAnimation/AnimationStatePath.h"
#include "clientSkeletalAnimation/AnimationTrackId.h"
#include "clientSkeletalAnimation/BasePriorityBlendAnimation.h"
#include "clientSkeletalAnimation/ConfigClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/LogicalAnimationTableTemplate.h"
#include "clientSkeletalAnimation/SkeletalAnimation.h"
#include "clientSkeletalAnimation/SkeletalAnimationTemplate.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/TrackAnimationController.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedObject/Object.h"

#include <string>

// ======================================================================

#define LOG_LOOP_TRACK_DEFERRAL  0

// ======================================================================

const ConstCharCrcLowerString StateHierarchyAnimationController::cms_defaultLogicalAnimationName("default");
const ConstCharCrcLowerString StateHierarchyAnimationController::cms_actionTrackName("action");
const ConstCharCrcLowerString StateHierarchyAnimationController::cms_addTrackName("add");
const ConstCharCrcLowerString StateHierarchyAnimationController::cms_locomotionTrackName("loop");

bool                 StateHierarchyAnimationController::ms_installed;

AnimationTrackId     StateHierarchyAnimationController::ms_locomotionTrackId;
AnimationTrackId     StateHierarchyAnimationController::ms_actionTrackId;
AnimationTrackId     StateHierarchyAnimationController::ms_addTrackId;

// ======================================================================
// class StateHierarchyAnimationController: public static member functions
// ======================================================================

void StateHierarchyAnimationController::install()
{
	DEBUG_FATAL(ms_installed, ("StateHierarchyAnimationController already installed."));

	bool lookupResult = TrackAnimationController::getTrackIdByName(cms_locomotionTrackName, ms_locomotionTrackId);
	WARNING(!lookupResult, ("failed to lookup track id for [%s].", cms_locomotionTrackName.getString()));

	lookupResult = TrackAnimationController::getTrackIdByName(cms_actionTrackName, ms_actionTrackId);
	WARNING(!lookupResult, ("failed to lookup track id for [%s].", cms_actionTrackName.getString()));

	lookupResult = TrackAnimationController::getTrackIdByName(cms_addTrackName, ms_addTrackId);
	WARNING(!lookupResult, ("failed to lookup track id for [%s].", cms_addTrackName.getString()));

	ms_installed = true;
	ExitChain::add(remove, "StateHierarchyAnimationController");
}

// ======================================================================
// class StateHierarchyAnimationController: public member functions
// ======================================================================

StateHierarchyAnimationController::StateHierarchyAnimationController(AnimationEnvironment &animationEnvironment, const TransformNameMap &transformNameMap, const AnimationStateHierarchyTemplate *hierarchyTemplate, const LogicalAnimationTableTemplate *latTemplate, int channel, AnimationStatePath const &initialPath) :
	TransformAnimationController(animationEnvironment, &transformNameMap),
	m_hierarchyTemplate(hierarchyTemplate),
	m_latTemplate(latTemplate),
	m_currentState(new AnimationStatePath()),
	m_trackAnimationController(new TrackAnimationController(animationEnvironment, &transformNameMap, channel)),
	m_deferredLoopAnimationName(0),
	m_deferredPlayTimeRemaining(0.0f)
{
	//-- Fetch reference to hierarchy template.
	if (m_hierarchyTemplate)
		m_hierarchyTemplate->fetch();

	if (m_latTemplate)
		m_latTemplate->fetch();

	//-- Setup initial animation state path.
	initialPath.copyTo(*m_currentState);

	//-- Play root state idle animation.
	if (m_hierarchyTemplate)
	{
		int animationId;

		const AnimationState *const animationState = m_hierarchyTemplate->getConstAnimationState(*m_currentState);
		if (animationState != NULL)
			playAnimation(ms_locomotionTrackId, animationState->getLogicalAnimationName(), true, false, true, true, animationId, NULL);
		else
		{
#ifdef _DEBUG
			Object const *object = animationEnvironment.getSkeletalAppearance() ? animationEnvironment.getSkeletalAppearance()->getOwner() : NULL;
			DEBUG_WARNING(true, ("StateHierarchyAnimationController(): object id=[%s],template=[%s],LAT controller=[%s]: initial animation path [%s] doesn't exist, no animation set.",
				object ? object->getNetworkId().getValueString().c_str() : "<null object>", object ? object->getObjectTemplateName() : "<null object>", 
				m_latTemplate ? m_latTemplate->getName().getString() : "<null LAT>", m_currentState->getPathString().c_str()));
#endif
		}
	}
}

// ----------------------------------------------------------------------

StateHierarchyAnimationController::~StateHierarchyAnimationController()
{
	delete m_trackAnimationController;
	delete m_currentState;

	if (m_latTemplate)
	{
		m_latTemplate->release();
		m_latTemplate = 0;
	}

	if (m_hierarchyTemplate)
	{
		m_hierarchyTemplate->release();
		m_hierarchyTemplate = 0;
	}
}

// ----------------------------------------------------------------------

void StateHierarchyAnimationController::alter(real deltaTime)
{
	//-- Handle deferred loop track play for state changes.  This prevents action blend from clobbering.
	if (m_deferredLoopAnimationName)
	{
		// Adjust deltaTime remaining until deferred play call is made.
		m_deferredPlayTimeRemaining -= deltaTime;

		// Play animation when deferral deltaTime expires.
		if (m_deferredPlayTimeRemaining <= 0.0f)
		{
			int animationId;

			playAnimation(ms_locomotionTrackId, *m_deferredLoopAnimationName, true, false, true, true, animationId, NULL);

#if LOG_LOOP_TRACK_DEFERRAL
			DEBUG_REPORT_LOG(true, ("SHAC: delayed loop track: PLAY NOW (%d).\n", ms_locomotionTrackId));
#endif

			// Reset deferral mechanism.
			m_deferredLoopAnimationName = 0;
		}
	}

	m_trackAnimationController->alter(deltaTime);
}

// ----------------------------------------------------------------------

void StateHierarchyAnimationController::evaluateTransformComponents(int localTransformIndex, Quaternion &rotation, Vector &translation)
{
	m_trackAnimationController->evaluateTransformComponents(localTransformIndex, rotation, translation);
}

// ----------------------------------------------------------------------

void StateHierarchyAnimationController::getObjectLocomotion(Quaternion &rotation, Vector &translation) const
{
	m_trackAnimationController->getObjectLocomotion(rotation, translation);
}

// ----------------------------------------------------------------------

int StateHierarchyAnimationController::addAnimationMessageListener(AnimationMessageCallback callback, void *context)
{
	// forward/redirect this to the embedded animation controller
	return m_trackAnimationController->addAnimationMessageListener(callback, context);
}

// ----------------------------------------------------------------------

void StateHierarchyAnimationController::removeAnimationMessageListener(int id)
{
	// forward/redirect this to the embedded animation controller
	m_trackAnimationController->removeAnimationMessageListener(id);
}

// ----------------------------------------------------------------------

StateHierarchyAnimationController *StateHierarchyAnimationController::asStateHierarchyAnimationController()
{
	return this;
}

// ----------------------------------------------------------------------

StateHierarchyAnimationController const *StateHierarchyAnimationController::asStateHierarchyAnimationController() const
{
	return this;
}

// ----------------------------------------------------------------------

TrackAnimationController *StateHierarchyAnimationController::asTrackAnimationController()
{
	return m_trackAnimationController;
}

// ----------------------------------------------------------------------
/**
 * @param skipTraversal  if true, don't play any transition animations, just get
 *                       into the new state and start playing its loop.
 * @param skipWithDelay  only applicable if skipTraversal is true.  If true,
 *                       the skip to the new looping state takes place in a deferred
 *                       manner to allow a specifically called out transition animation
 *                       to do its thing.  If false, the transition on the looping
 *                       track takes place immediately, good for initializing the
 *                       character to a non-standing posture.
 */

void StateHierarchyAnimationController::setDestinationState(const AnimationStatePath &destinationState, bool skipTraversal, bool skipWithDelay)
{
	//-- Validate preconditions
	if (!m_hierarchyTemplate || !m_latTemplate)
		return;

	//-- Check if the destination state is the same as the current state.
	//   If so, nothing more to do.
	if (destinationState == *m_currentState)
		return;

	//-- Ensure destination state path exists in ash file.
	const AnimationState *const destAnimationState = m_hierarchyTemplate->getConstAnimationState(destinationState);
	if (!destAnimationState)
	{
		DEBUG_WARNING(true, ("SHAC: destination animation state [%s] not present in ash file [%s], ignoring state path change.", destinationState.getPathString().c_str(), m_hierarchyTemplate->getName().getString()));
		return;
	}

	if (skipTraversal)
	{
		//-- Change the current state path to the destination state now.
		destinationState.copyTo(*m_currentState);

		if (skipWithDelay)
			requestDeferredLoopAnimation(destAnimationState->getLogicalAnimationName(), BasePriorityBlendAnimation::getBlendTime());
		else
		{
			//-- Play the idle for the destination state now.
			int animationId;
			playAnimation(ms_locomotionTrackId, destAnimationState->getLogicalAnimationName(), true, false, true, true, animationId, NULL);
		}

		return;
	}
	else
	{
		DEBUG_REPORT_LOG(ConfigClientSkeletalAnimation::getLogStateTraversal() && isObjectLevelDebuggingEnabled(), ("SHAC: starting state traversal check.\n"));

		int animationPlayCount = 0;


		//-- Get the index of the last state in the common root state between the current 
		//   and destination state.
		const int commonPathLength = AnimationStatePath::getCommonPathLength(*m_currentState, destinationState);
		DEBUG_FATAL(commonPathLength < 1, ("unexpected, no common path (not even mandatory root) [%d].", commonPathLength));

		//-- Get the length of the current and destination state.
		const int currentStatePathLength     = m_currentState->getPathLength();
		const int destinationStatePathLength = destinationState.getPathLength();

		const AnimationState *focusState = m_hierarchyTemplate->getConstAnimationState(*m_currentState);
		if (!focusState)
		{
			DEBUG_WARNING(true, ("StateHierarchyAnimationController: failed to set focusState [%s].", m_currentState->getPathString().c_str()));
			return;
		}

		//-- Traverse from current state to common root.
		int startingUpwardPathIndex = commonPathLength;

		{
			for (int i = currentStatePathLength; i > commonPathLength; --i)
			{
				//-- Goal: find the longest jump we can take from the focus state
				//         along the path to the destination state.

				// Current state must travel down to common root state.
				bool breakDownwardTraversal = false;

				//-- Search for links to all states from the destination state to the 
				//   common root state. (Do link check from furthest-away to nearest).
				for (int j = destinationStatePathLength; j > commonPathLength; --j)
				{
					const AnimationStateLink *const link = focusState->getConstLink(destinationState, j);
					if (link)
					{
						//-- Found a link, take this.  Break us out of the "travel down to root" branch.
						DEBUG_REPORT_LOG(ConfigClientSkeletalAnimation::getLogStateTraversal() && isObjectLevelDebuggingEnabled(), ("SHAC: taking link from [%s] to [%s].\n", m_currentState->getPathString(i).c_str(), destinationState.getPathString(j).c_str()));

						breakDownwardTraversal  = true;
						focusState              = m_hierarchyTemplate->getConstAnimationState(destinationState, j);
						startingUpwardPathIndex = j;

						//-- Perform animation activity necessary to take this link.
						// Determine logical animation to play.
						const CrcLowerString *logicalAnimationName = 0;

						if (link->hasTransitionLogicalAnimation())
							logicalAnimationName = &(link->getTransitionLogicalAnimationName());

						// Play the animation.
						int animationId;

						if (logicalAnimationName && *(logicalAnimationName->getString()) != 0)
						{
							playAnimation(ms_actionTrackId, *logicalAnimationName, ((animationPlayCount > 0) ? false : true), true, false, false, animationId, NULL);
							++animationPlayCount;
						}

						break;
					}
				}

				if (breakDownwardTraversal)
					break;

				for (int k = commonPathLength; k < i; ++k)
				{
					const AnimationStateLink *const link = focusState->getConstLink(*m_currentState, k);
					if (link)
					{
						//-- Found a link, take this.
						DEBUG_REPORT_LOG(ConfigClientSkeletalAnimation::getLogStateTraversal() && isObjectLevelDebuggingEnabled(), ("SHAC: taking link from [%s] to [%s].\n", m_currentState->getPathString(i).c_str(), m_currentState->getPathString(k).c_str()));

						// Adjust the focus state to the state we just hopped to.
						for (int lcv = i; lcv > k; --lcv)
						{
							focusState = focusState->getConstParentState();
							if (!focusState)
							{
								DEBUG_WARNING(true, ("StateHierarchyAnimationController: null focusState on traversal, non-existent path [dest = %s]?", destinationState.getPathString().c_str()));
								return;
							}
						}
						i = k + 1;

						//-- Perform animation activity necessary to take this link.
						// Determine logical animation to play.
						const CrcLowerString *logicalAnimationName = 0;

						if (link->hasTransitionLogicalAnimation())
							logicalAnimationName = &(link->getTransitionLogicalAnimationName());

						// Play the animation.
						int animationId;

						if (logicalAnimationName && *(logicalAnimationName->getString()) != 0)
						{
							playAnimation(ms_actionTrackId, *logicalAnimationName, ((animationPlayCount > 0) ? false : true), true, false, false, animationId, NULL);
							++animationPlayCount;
						}

						break;
					}
				}
			}
		}

		//-- Continue building traversal activity from the common root up to the destination
		//   path.
		{
			for (int i = startingUpwardPathIndex; i < destinationStatePathLength; ++i)
			{
				for (int j = destinationStatePathLength; j > i; --j)
				{
					const AnimationStateLink *const link = focusState->getConstLink(destinationState, j);
					if (link)
					{
						//-- Perform activity necessary to take this link.
						DEBUG_REPORT_LOG(ConfigClientSkeletalAnimation::getLogStateTraversal() && isObjectLevelDebuggingEnabled(), ("SHAC: taking link from [%s] to [%s].\n", destinationState.getPathString(i).c_str(), destinationState.getPathString(j).c_str()));

						// Set the new focus state.
						focusState      = m_hierarchyTemplate->getConstAnimationState(destinationState, j);

						// Set focus state index.
						i = j - 1;

						//-- Perform animation activity necessary to take this link.
						// Determine logical animation to play.
						const CrcLowerString *logicalAnimationName = 0;

						if (link->hasTransitionLogicalAnimation())
							logicalAnimationName = &(link->getTransitionLogicalAnimationName());

						// Play the animation.
						int animationId;

						if (logicalAnimationName && *(logicalAnimationName->getString()) != 0)
						{
							playAnimation(ms_actionTrackId, *logicalAnimationName, ((animationPlayCount > 0) ? false : true), true, false, false, animationId, NULL);
							++animationPlayCount;
						}

						break;
					}
				}
			}
		}

		// -TRF-
		// Phew, we're done.  I'd like to clean up the traversal logic so long as
		// it is kept efficient.  The above code isn't pretty but also prevents
		// memory allocations during the traversal check, which is O_max(N!) complexity
		// where N = # hops between source and destination state.  N should be
		// around 5 in typical cases, and generally shouldn't hit the worst
		// case loop count (due to links that short-circuit the evaluation).
		DEBUG_REPORT_LOG(ConfigClientSkeletalAnimation::getLogStateTraversal() && isObjectLevelDebuggingEnabled(), ("SHAC: ending state traversal check.\n"));
	}

	//-- Change the current state path to the destination state.
	//   This means we will treat actions as if they are played from
	//   the destination state, even though it may take us a little while
	//   to get there.
	destinationState.copyTo(*m_currentState);

	//-- Request to play the idle animation for the new state after the blend to the action is complete.
	requestDeferredLoopAnimation(destAnimationState->getLogicalAnimationName(), BasePriorityBlendAnimation::getBlendTime());
}

// ----------------------------------------------------------------------

const AnimationStatePath &StateHierarchyAnimationController::getDestinationPath() const
{
	return *m_currentState;
}

// ----------------------------------------------------------------------

void StateHierarchyAnimationController::playAction(const CrcLowerString &actionName, const AnimationEnvironment &animationEnvironment, int &animationId, bool &animationIsAdd, AnimationNotification *notification)
{
	//-- Lookup ash logical animation name index.
	const CrcLowerString *logicalAnimationName = 0;

	if (!lookupAnimationNameForAction(actionName, animationEnvironment, logicalAnimationName, animationIsAdd))
	{
		// Failed to lookup action index.
		DEBUG_REPORT_LOG(ConfigClientSkeletalAnimation::getLogActionSelection() && isObjectLevelDebuggingEnabled(), ("SHAC: action: failed to find a state defining the action [%s] within the current hierarchy starting from state [%s] [heir=%s] [LAT=%s].", actionName.getString(), m_currentState->getPathString(m_currentState->getPathLength()).c_str(),
			m_hierarchyTemplate->getName().getString(), m_latTemplate->getName().getString()));
		return;
	}

	//-- Play the action.
	NOT_NULL(logicalAnimationName);

	DEBUG_REPORT_LOG(ConfigClientSkeletalAnimation::getLogActionSelection() && isObjectLevelDebuggingEnabled(), ("SHAC: action: playing action [%s] [track=%s] [LAN=%s] [state=%s] [heir=%s] [LAT=%s].\n", actionName.getString(), animationIsAdd ? "add" : "playOnce", logicalAnimationName->getString(), m_currentState->getPathString(m_currentState->getPathLength()).c_str(),
		m_hierarchyTemplate->getName().getString(), m_latTemplate->getName().getString()));
	playAnimation(animationIsAdd ? ms_addTrackId : ms_actionTrackId, *logicalAnimationName, true, true, false, false, animationId, notification);
}

// ----------------------------------------------------------------------

void StateHierarchyAnimationController::stopAction(int animationId, bool animationIsAdd)
{
	//-- Select the track based on add flag.
	const AnimationTrackId &trackId = (animationIsAdd ? ms_addTrackId : ms_actionTrackId);

	//-- Check which action is curently playing.
	int currentlyPlayingAnimationId = -1;

	const bool actionIsPlaying = m_trackAnimationController->getCurrentlyPlayingAnimationId(trackId, currentlyPlayingAnimationId);
	if (actionIsPlaying && (animationId == currentlyPlayingAnimationId))
	{
		// The action playing is the one we want to stop.  Let's stop it.
		DEBUG_REPORT_LOG(ConfigClientSkeletalAnimation::getLogActionSelection() && isObjectLevelDebuggingEnabled(), ("SHAC: action: stopping action [id=%d].\n", animationId));
		m_trackAnimationController->stopAnimation(trackId);
	}
	else
	{
		DEBUG_REPORT_LOG(ConfigClientSkeletalAnimation::getLogActionSelection() && isObjectLevelDebuggingEnabled(), ("SHAC: action: stop action request ignored, animation [id=%d] not currently playing.\n", animationId));
	}
}

// ----------------------------------------------------------------------

bool StateHierarchyAnimationController::hasActionCompleted(int animationId, bool animationIsAdd) const
{
	//-- Retrieve the most recently completed animation id for the proper track.
	const int mostRecentlyCompletedAnimationId = m_trackAnimationController->getMostRecentlyCompletedAnimationId(animationIsAdd ? ms_addTrackId : ms_actionTrackId);

	//-- The action has completed if its animation id has already completed --- animation ids are issued sequentially.
	return (mostRecentlyCompletedAnimationId >= animationId);
}

// ----------------------------------------------------------------------

bool StateHierarchyAnimationController::isPlayOnceTrackActive() const
{
	return !m_trackAnimationController->isTrackIdle(ms_actionTrackId);
}

// ----------------------------------------------------------------------

bool StateHierarchyAnimationController::doesPlayOnceTrackHaveLocomotionPriority() const
{
	return m_trackAnimationController->doesTrackHaveLocomotionPriority(ms_actionTrackId);
}

// ----------------------------------------------------------------------

int StateHierarchyAnimationController::getLoopTrackLocomotionPriority() const
{
	return m_trackAnimationController->getTrackLocomotionPriority(ms_locomotionTrackId);
}

// ----------------------------------------------------------------------

int StateHierarchyAnimationController::getPlayOnceTrackLocomotionPriority() const
{
	return m_trackAnimationController->getTrackLocomotionPriority(ms_actionTrackId);
}

// ----------------------------------------------------------------------

SkeletalAnimation *StateHierarchyAnimationController::fetchAnimationForAction(const CrcLowerString &actionName, const AnimationEnvironment &animationEnvironment)
{
	//-- Lookup ash logical animation name index.
	const CrcLowerString *logicalAnimationName = 0;
	bool  playOnAddTrack                       = false;

	if (!lookupAnimationNameForAction(actionName, animationEnvironment, logicalAnimationName, playOnAddTrack))
	{
		// Failed to lookup action index.
		return 0;
	}

	//-- Get animation from LAT.
	NOT_NULL(logicalAnimationName);
	SkeletalAnimation *const skeletalAnimation = fetchAnimation(*logicalAnimationName);

	return skeletalAnimation;
}

// ----------------------------------------------------------------------

const AnimationStateHierarchyTemplate *StateHierarchyAnimationController::fetchHierarchyTemplate() const
{
	if (m_hierarchyTemplate)
		m_hierarchyTemplate->fetch();

	return m_hierarchyTemplate;
}

// ----------------------------------------------------------------------

const LogicalAnimationTableTemplate *StateHierarchyAnimationController::fetchLogicalAnimationTableTemplate() const
{
	if (m_latTemplate)
		m_latTemplate->fetch();

	return m_latTemplate;
}

// ======================================================================
// class StateHierarchyAnimationController: private static member functions
// ======================================================================

void StateHierarchyAnimationController::remove()
{
	DEBUG_FATAL(!ms_installed, ("StateHierarchyAnimationController not installed."));
	ms_installed = false;
}

// ======================================================================
// class StateHierarchyAnimationController: private member functions
// ======================================================================

void StateHierarchyAnimationController::playAnimation(const AnimationTrackId &trackId, CrcString const &logicalAnimationName, bool playImmediately, bool doBlend, bool playLooping, bool fallbackToDefault, int &animationId, AnimationNotification *notification)
{
	//-- Fetch the skeletal animation associated with the specified animation index.  Use default animation if specified animation doesn't exist and
	//   default is requested.
	SkeletalAnimation *skeletalAnimation = fetchAnimation(logicalAnimationName);

	if (!skeletalAnimation && fallbackToDefault)
	{
		//-- Try to use default animation.
		skeletalAnimation = fetchAnimation(cms_defaultLogicalAnimationName);
		//DEBUG_WARNING(!skeletalAnimation && m_hierarchyTemplate, ("specified logical animation [%s] doesn't exist, neither does default animation.", logicalAnimationName.getString()));
	}

	if (!skeletalAnimation)
		return;

	TrackAnimationController::BlendMode blendMode;
	float                               blendInTime;
	
	if (doBlend)
	{
		blendMode   = TrackAnimationController::BM_linearBlend;
		blendInTime = 0.15f;
	}
	else
	{
		blendMode   = TrackAnimationController::BM_noBlend;
		blendInTime = 0.0f;
	}

	//-- play the animation
	animationId = m_trackAnimationController->playAnimation(trackId, skeletalAnimation, static_cast<TrackAnimationController::PlayMode>(playImmediately ? 1 : 0), playLooping, blendMode, blendInTime, notification);

	//-- release local references
	skeletalAnimation->release();
}

// ----------------------------------------------------------------------
/**
 * Return the ash logical animation name index for the specified action.
 *
 * Action LAN index lookup is context based, where the context comes
 * from the animation state path at which the actor currently resides.
 *
 * @todo update docs
 *
 * @param actionName  the name of the action to lookup.
 *
 * @return  -1 if the action name could not be found; otherwise, returns
 *          the ash logical animation name index associated with the action name
 *          based on the current animation state path.
 */

bool StateHierarchyAnimationController::lookupAnimationNameForAction(const CrcLowerString &actionName, const AnimationEnvironment &animationEnvironment, const CrcLowerString *&logicalAnimationName, bool &playOnAddTrack) const
{
	//-- Validate preconditions
	if (!m_hierarchyTemplate)
		return false;

	//-- Find the action.
	const AnimationState *const currentAnimationState = m_hierarchyTemplate->getConstAnimationState(*m_currentState);
	const AnimationState *      animationState        = currentAnimationState;

	logicalAnimationName = 0;
	playOnAddTrack       = false;

	while (!logicalAnimationName && animationState)
	{
		//-- Search the current state for an action defined for the given action name.
		const AnimationAction *const action = animationState->getConstActionByName(actionName);
		if (action)
		{
			logicalAnimationName = &(action->getLogicalAnimationName(animationEnvironment));
			playOnAddTrack       = action->shouldApplyAnimationAsAdd(animationEnvironment);
		}
		else
		{
			//-- Search parent state.
			animationState = animationState->getConstParentState();
		}
	}

	//-- Indicate the animation name was found if there is a valid animation index for the action name.
	return (logicalAnimationName != 0);
}

// ----------------------------------------------------------------------

SkeletalAnimation *StateHierarchyAnimationController::fetchAnimation(CrcString const &logicalAnimationName)
{
	if (!m_latTemplate)
	{
		DEBUG_WARNING(true, ("StateHierarchyAnimationController: cannot lookup animation because no .lat."));
		return 0;
	}

	const SkeletalAnimationTemplate *animationTemplate = m_latTemplate->fetchConstAnimationTemplate(logicalAnimationName);
	if (!animationTemplate)
	{
		DEBUG_REPORT_LOG(ConfigClientSkeletalAnimation::getLogActionSelection() && isObjectLevelDebuggingEnabled(), ("SHAC: action: LAT [%s] has no mapping for action name [%s].\n", m_latTemplate->getName().getString(), logicalAnimationName.getString()));
		return 0;
	}

	//-- create the animation
	SkeletalAnimation *const skeletalAnimation = animationTemplate->fetchSkeletalAnimation(getAnimationEnvironment(), getTransformNameMap());

	//-- Release local references.
	animationTemplate->release();

	return skeletalAnimation;
}

// ----------------------------------------------------------------------

void StateHierarchyAnimationController::requestDeferredLoopAnimation(CrcString const &logicalAnimationName, float waitTime)
{
	m_deferredLoopAnimationName = &logicalAnimationName;
	m_deferredPlayTimeRemaining = waitTime;

#if LOG_LOOP_TRACK_DEFERRAL
	DEBUG_REPORT_LOG(true, ("SHAC: delayed loop track: request (%d, %.2f).\n", logicalAnimationNameIndex, waitTime));
#endif
}

// ======================================================================
