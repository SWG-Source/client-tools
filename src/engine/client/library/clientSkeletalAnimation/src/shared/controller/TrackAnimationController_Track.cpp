// ======================================================================
//
// TrackAnimationController_Track.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/TrackAnimationController_Track.h"

#include "clientGraphics/GraphicsDebugFlags.h"
#include "clientSkeletalAnimation/ConfigClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/SkeletalAnimation.h"
#include "clientSkeletalAnimation/TimedBlendSkeletalAnimation.h"
#include "clientSkeletalAnimation/TrackAnimationController_PhysicalTrackTemplate.h"
#include "clientSkeletalAnimation/TrackAnimationController_PriorityBlendAnimation.h"
#include "clientSkeletalAnimation/TransformNameMap.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedMath/Vector.h"
#include "sharedMath/Quaternion.h"

#include <algorithm>
#include <vector>
#include <limits>

// ======================================================================

namespace TrackAnimationControllerTrackNamespace
{
	// Min time to apply to animations, used during looped animation processing.
	float const cs_minTimeToApply = 0.005f;

	// We should not apply more than 3 animations per frame.  If so, it is a bug
	// and could cause an infinite loop.
	int const cs_maxAnimationUpdatesPerFrame = 4;
}

using namespace TrackAnimationControllerTrackNamespace;

// ======================================================================
// class TrackAnimationController::Track::QueueEntry
// ======================================================================

TrackAnimationController::Track::QueueEntry::QueueEntry(SkeletalAnimation *skeletalAnimation, bool loop, BlendMode blendMode, float blendInTime, int animationId) :
	m_skeletalAnimation(skeletalAnimation),
	m_loop(loop),
	m_blendMode(blendMode),
	m_blendInTime(blendInTime),
	m_animationId(animationId)
{
	NOT_NULL(m_skeletalAnimation);
	m_skeletalAnimation->fetch();
}

// ----------------------------------------------------------------------

inline TrackAnimationController::Track::QueueEntry::~QueueEntry()
{
	m_skeletalAnimation->release();
	m_skeletalAnimation = 0;
}

// ======================================================================
// class TrackAnimationController::Track
// ======================================================================

TrackAnimationController::Track::Track(TrackAnimationController &controller, const PhysicalTrackTemplate &physicalTrackTemplate) :
	m_controller(controller),
	m_physicalTrackTemplate(physicalTrackTemplate),
	m_currentAnimation(0),
	m_loop(false),
	m_queue(),
	m_isIdle(true),
	m_locomotionTranslation(),
	m_locomotionRotation(),
	m_mostRecentRotations(new QuaternionVector(static_cast<QuaternionVector::size_type>(controller.getTransformNameMap().getTransformCount()))),
	m_mostRecentTranslations(new VectorVector(static_cast<VectorVector::size_type>(controller.getTransformNameMap().getTransformCount()))),
	m_currentAnimationId(-1),
	m_nextAnimationId(0)
{
	createInitialAnimation();
}

// ----------------------------------------------------------------------

TrackAnimationController::Track::~Track()
{
	delete m_mostRecentTranslations;
	delete m_mostRecentRotations;

	//-- stop the animation
	if (m_currentAnimation)
	{
		m_currentAnimation->setStateStoppedExplicitlyStopped(m_controller.getChannel());
		m_currentAnimation->release();
		m_currentAnimation = 0;
	}

	//-- empty the animation queue
	clearQueue();
}

// ----------------------------------------------------------------------
/**
 * Do the minimal amount of bookkeeping work needed per frame for this
 * animation track.
 *
 * Most animations that can be played on a Track instance will do the minimal 
 * amount of work required in alter (alterSingleCycle).  The more expensive
 * work is done in evaluateTransformComponents(), which may not be called
 * for off-screen skeletons.
 *
 * The caller may want to suppress processing of animation messages.  One
 * such case is when a priority blend is occurring.  The track with the higher
 * locomotion priority will be able to play its messages while the track with the
 * lower locomotion priority will have to wait.  This handles footsteps, but may
 * prove disastrous for other messages that kick off from the upper body.
 *
 * @param deltaTime                 the time elapsed since the last active game frame.
 * @param processAnimationMessages  true if alter should process animation messages; false
 *                                  otherwise.  This defaults to true.
 */

void TrackAnimationController::Track::alter(float const originalTimeDelta, bool processAnimationMessages)
{
	NP_PROFILER_AUTO_BLOCK_DEFINE("TrackAnimationController::Track::alter");

	//-- Clear out locomotion.
	m_locomotionRotation    = Quaternion::identity;
	m_locomotionTranslation = Vector::zero;

	//-- Expire an animation if it's non looping or if other items are waiting in the queue.
	bool terminateAtEndOfLoop = !m_loop || !m_queue.empty();

	//-- Apply deltaTime to animations.
	SkeletalAnimation *replacementAnimation = 0;

	bool animationChangedOrLooped = false;
	float deltaTime = originalTimeDelta;

	// Prevent an infinite loop iff bad data is triggered multiple times per frame.
	int loopCount = cs_maxAnimationUpdatesPerFrame;

	do
	{
		// Animation hasn't changed.
		animationChangedOrLooped = false;

		//-- Alter up through the end of the current animation cycle.
		replacementAnimation = 0;

		//-- Apply remaining deltaTime to the animation.
		if (m_currentAnimation)
		{
			bool const keepAnimation = m_currentAnimation->alterSingleCycle(deltaTime, replacementAnimation, deltaTime);

			//-- Collect object locomotion.
			// @todo per controller should enable/disable this: species-specific face skeletons never need to evaluate this.
			if (keepAnimation) 
			{
				Quaternion         cycleRotation;
				Vector             cycleTranslation;

				m_currentAnimation->getScaledLocomotion(cycleRotation, cycleTranslation);
				m_locomotionTranslation += cycleTranslation;
				m_locomotionRotation = cycleRotation * m_locomotionRotation;
			}
			
			//-- Collect messages.
			if (processAnimationMessages)
				m_controller.emitAnimationMessages(*m_currentAnimation);

			//-- Determine disposition of current animation.
			if (!keepAnimation)
			{
				if (replacementAnimation != m_currentAnimation)
				{
					animationChangedOrLooped = true;
					deltaTime = originalTimeDelta;
				}

				// Fetch reference to replacement animation if specified.
				if (replacementAnimation)
					replacementAnimation->fetch();

				// Animation requested itself to be killed.  Kill it and replace with specified replacement.
				m_currentAnimation->setStateStoppedNormal(m_controller.getChannel());
				m_currentAnimation->release();

				m_currentAnimation = replacementAnimation; // This can set the animation to null.

			}
			else if (deltaTime >= cs_minTimeToApply)
			{
				// Animation finished the current cycle.  Either loop or feed next entry in the queue.
				if (terminateAtEndOfLoop)
				{
					animationChangedOrLooped = true;
					
					// Transition to next queue entry.
					if (m_currentAnimation)  //lint !e774 // always true // hmm, experience shows otherwise.
						m_currentAnimation->setStateStoppedNormal(m_controller.getChannel());

					transitionToNextQueueEntry();
				}
				else
				{
					// Loop current animation.
					if (m_currentAnimation) //lint !e774 // boolean always evaluates to true // wrong, transitionToNextQueueEntry() can cause it to be set to NULL (unusual but possible).
						m_currentAnimation->startNewCycle();
				}
			}
		}
		else
		{
			if (m_queue.empty())
			{
				//-- No queue entries, no current animation, setup the control loop exit condition.
				deltaTime = 0.0f;

				//-- Notify the track is entering an idle state (i.e. not playing anything).
				// @todo fix this: it appears that it gets called every alter frame instead of just the first one.
				postEnterIdleHook();
			}
		}
	}
	while(animationChangedOrLooped && loopCount--);
}

// ----------------------------------------------------------------------

void TrackAnimationController::Track::evaluateTransformComponents(int transformIndex, Quaternion &rotation, Vector &translation)
{
	NP_PROFILER_AUTO_BLOCK_DEFINE("TrackAnimationController::Track::evaluateTransformComponents");

	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, transformIndex, static_cast<int>(m_mostRecentRotations->size()));
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, transformIndex, static_cast<int>(m_mostRecentTranslations->size()));

	if (m_currentAnimation)
	{
		m_currentAnimation->evaluateTransformComponents(transformIndex, rotation, translation);

		//-- Keep track of the most recently evaluated data.  When the track has no animation playing on it, I can still access this.
		//   Currently used for blending out of action track when action animation completes.
		//   Note there is a flaw here.  If we didn't need the translation/rotation before an animation completed, but we need it after
		//   the animation completed, the data won't be here because this function hadn't been called to cache the value.
		(*m_mostRecentRotations)[static_cast<QuaternionVector::size_type>(transformIndex)] = rotation;
		(*m_mostRecentTranslations)[static_cast<VectorVector::size_type>(transformIndex)]  = translation;
	}
	else
	{
		rotation    = (*m_mostRecentRotations)[static_cast<QuaternionVector::size_type>(transformIndex)];
		translation = (*m_mostRecentTranslations)[static_cast<VectorVector::size_type>(transformIndex)];
	}

}

// ----------------------------------------------------------------------

void TrackAnimationController::Track::getMostRecentAnimationTransformComponents(int transformIndex, Quaternion &rotation, Vector &translation)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, transformIndex, static_cast<int>(m_mostRecentRotations->size()));
	rotation = (*m_mostRecentRotations)[static_cast<QuaternionVector::size_type>(transformIndex)];

	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, transformIndex, static_cast<int>(m_mostRecentTranslations->size()));
	translation = (*m_mostRecentTranslations)[static_cast<VectorVector::size_type>(transformIndex)];
}

// ----------------------------------------------------------------------

int TrackAnimationController::Track::playAnimation(SkeletalAnimation *skeletalAnimation, PlayMode playMode, bool loop, BlendMode transitionBlendMode, float blendInTime, AnimationNotification *notification)
{
	NOT_NULL(skeletalAnimation);

	int animationId = -1;

	//-- Skip the animation if already playing.  Changing loop flag is considered a new animation.
	if (skeletalAnimation == m_currentAnimation && loop == m_loop)
	{
		DEBUG_WARNING(true, ("attempting to play same animation twice [0x%08x].", skeletalAnimation));
		return m_currentAnimationId;
	}

	//-- Associate the notification with the animation.
	skeletalAnimation->setNotification(notification, m_controller.getChannel());

	//-- Signal if we're exiting an idle state (i.e. track previously was inactive, now is active).
	if (!m_currentAnimation)
		preExitIdleHook();

	//-- Handle request based on play mode.
	switch (playMode)
	{
		case PM_queue:
			{
				if (!m_currentAnimation && m_queue.empty())
				{
					// Nothing in queue, go ahead and play it now.
					animationId = m_nextAnimationId++;
					playNow(skeletalAnimation, loop, transitionBlendMode, blendInTime, animationId);
				}
				else
				{
					// queue it
					animationId = m_nextAnimationId++;
					skeletalAnimation->setStateInQueue(m_controller.getChannel());
					m_queue.push_back(new QueueEntry(skeletalAnimation, loop, transitionBlendMode, blendInTime, animationId));
				}
			}
			break;

		case PM_immediate:
			{
				// Do not allow an animation of lower priority to replace a higher priority currently-playing animation.
				if (m_physicalTrackTemplate.getAllowSameTrackTrumping() && ConfigClientSkeletalAnimation::getAllowSameTrackTrumping() && m_currentAnimation)
				{
					int const compareResult = SkeletalAnimation::compareAnimationPriority(*m_currentAnimation, *skeletalAnimation);
					if (compareResult < 0)
					{
						//-- The current animation completely dominated the submitted animation.  Drop
						//   the new animation from consideration.
						skeletalAnimation->setStateSameTrackTrumped(m_controller.getChannel());

						DEBUG_REPORT_LOG(ConfigClientSkeletalAnimation::getLogSameTrackTrumping(), ("Anim Track [%s]: dropping the newly requested animation [%s], existing animation [%s] dominates priority.\n", 
							getTrackName().getString(),
							skeletalAnimation->getLeafAnimationTemplateName().getString(), 
							m_currentAnimation->getLeafAnimationTemplateName().getString()));

						return -1;
					}
				}

				// Clear the queue.
				clearQueue();

				// Indicate the current animation is being interrupted.
				if (m_currentAnimation)
					m_currentAnimation->setStateStoppedInterruped(m_controller.getChannel());

				// Remove the current animation.
				// If blending, keep around existing operations for blend-out.
				if (transitionBlendMode == BM_noBlend)
				{
					if (m_currentAnimation)
					{
						m_currentAnimation->release();
						m_currentAnimation = 0;
					}
				}

				animationId = m_nextAnimationId++;
				playNow(skeletalAnimation, loop, transitionBlendMode, blendInTime, animationId);
			}
			break;
		
		default:
			DEBUG_FATAL(true, ("unknown play mode %d", static_cast<int>(playMode)));
	}

	return animationId;
}

// ----------------------------------------------------------------------

void TrackAnimationController::Track::stopAnimation()
{
	//-- Stop currently playing animation.
	if (m_currentAnimation)
	{
		m_currentAnimation->setStateStoppedExplicitlyStopped(m_controller.getChannel());
		m_currentAnimation->release();
		m_currentAnimation = 0;
	}

	//-- Clear out the animation queue.
	clearQueue();

	//-- Notify that the track has entered the idle state.
	postEnterIdleHook();
}

// ----------------------------------------------------------------------
/**
 * Retrieve the animation id of the most recently completed animation.
 *
 * This function does not guarantee that the id returned ever played, it
 * just ensures that this instance is in the process of playing an animation
 * assigned a sequential ID that comes after the value returned.
 *
 * If more than 2^31-1 animations play in a given client session, animation
 * ids will wrap around.  Callers should beware of this condition and can
 * probably time-out or specifically handle the wrap-around condition.
 *
 * @return  the animation id of the most recently completed animation.
 */

int TrackAnimationController::Track::getMostRecentlyCompletedAnimationId() const
{
	// when we are playing an animation, the previous id is the most recently completed one.
	// When we are not playing one, the current id is the most recently completed one.
	return (m_currentAnimation == 0) ? m_currentAnimationId : m_currentAnimationId - 1;
}

// ----------------------------------------------------------------------
/**
 * Retrieve the id of the currently playing animation.
 *
 * @param animationId  if an animation is currently playing (see return value),
 *                     this value returns the id of the currently playing animation.
 *
 * @return  true if an animation is currently playing; false otherwise.
 */

bool TrackAnimationController::Track::getCurrentlyPlayingAnimationId(int &animationId) const
{
	animationId = m_currentAnimationId;
	return (m_currentAnimation != NULL);
}

// ----------------------------------------------------------------------

void TrackAnimationController::Track::getObjectLocomotion(Quaternion &rotation, Vector &translation) const
{
	rotation    = m_locomotionRotation;
	translation = m_locomotionTranslation;
}

// ----------------------------------------------------------------------

const TrackAnimationController::Track &TrackAnimationController::Track::getLocomotionPriorityTrack() const
{
	const PriorityBlendAnimation *const animation = dynamic_cast<const PriorityBlendAnimation*>(m_currentAnimation);
	if (!animation)
	{
		// It must be this track.
		return *this;
	}
	else
	{
		// The priority blend animation track actually blends between two other tracks, ask it which one has priority.
		return animation->getLocomotionPriorityTrack();
	}
}

// ----------------------------------------------------------------------

const CrcString   &TrackAnimationController::Track::getTrackName() const
{
	return m_physicalTrackTemplate.getTrackName();
}

// ======================================================================

void TrackAnimationController::Track::clearQueue()
{
	//-- delete all queue entries
	QueueEntryVector::iterator const endIt = m_queue.end();
	for (QueueEntryVector::iterator it = m_queue.begin(); it != endIt; ++it)
	{
		QueueEntry *const entry = *it;
		NOT_NULL(entry);
		NOT_NULL(entry->m_skeletalAnimation);

		entry->m_skeletalAnimation->setStateStoppedEjectedFromQueue(m_controller.getChannel());
		delete entry;
	}

	//-- reset queue size
	m_queue.clear();
}

// ----------------------------------------------------------------------

void TrackAnimationController::Track::transitionToNextQueueEntry()
{
	if (m_queue.empty())
	{
		// Nothing to transition to.  Clear current entry.
		if (m_currentAnimation)
		{
			m_currentAnimation->setStateStoppedNormal(m_controller.getChannel());
			m_currentAnimation->release();
			m_currentAnimation = 0;
		}

		postEnterIdleHook();
		return;
	}

	//-- Pop entry at start of queue.
	QueueEntry *const entry = m_queue.front();
	NOT_NULL(entry);

	IGNORE_RETURN(m_queue.erase(m_queue.begin()));

	//-- Play queue entry now.
	playNow(entry->m_skeletalAnimation, entry->m_loop, entry->m_blendMode, entry->m_blendInTime, entry->m_animationId);

	//-- Delete queue entry.
	delete entry;
}

// ----------------------------------------------------------------------

void TrackAnimationController::Track::playNow(SkeletalAnimation *skeletalAnimation, bool loop, BlendMode transitionBlendMode, float blendInTime, int animationId)
{
	// Do not trigger identical animations.
	if (m_currentAnimationId != animationId || m_loop != loop)
	{
		//-- Verify pre-conditions.
		NOT_NULL(skeletalAnimation);
	
		//-- Any way it works out, the specified animation is starting up now.
		skeletalAnimation->setStatePlayInProgress(m_controller.getChannel());
		
		//-- Determine next SkeletalAnimation instance to play.
		SkeletalAnimation * newSkeletalAnimation = 0;
		
		switch (transitionBlendMode)
		{
		case BM_noBlend:
			// New skeletal animation will be an immediate cut to the specified skeletal animation.
			newSkeletalAnimation = skeletalAnimation;
			break;
			
		case BM_linearBlend:
			// New timed blend animation from old animation to new animation.
			// Ignore blend if no animation is currently playing.
			if (m_currentAnimation)
			{
				// @todo may need to stop old animation from progressing.
				newSkeletalAnimation = new TimedBlendSkeletalAnimation(m_currentAnimation, skeletalAnimation, blendInTime);
			}
			else
			{
				// No point in blending from the bind pose to this animation.  Play it immediately.
				// This allows code for actions to always use blend, but only apply blend when another
				// animation is playing.  If I really always use blend, then the creature blends from
				// the bind pose to the action animation.  If I really never use blend, then playing
				// one action before another finishes will not do any blend.  This appears to be a
				// reasonable compromise.
				newSkeletalAnimation = skeletalAnimation;
			}
			break;
			
		default:
			DEBUG_FATAL(true, ("invalid transitionBlendMode %d", static_cast<int>(transitionBlendMode)));
		}
		
		//-- Fetch reference to new skeletal animation.
		if (newSkeletalAnimation) 
		{
			newSkeletalAnimation->fetch();
		}
		
		//-- Release reference to previously-playing skeletal animation.
		if (m_currentAnimation)
			m_currentAnimation->release();
		
		m_currentAnimation = newSkeletalAnimation;
		
		//-- Set animation parameters
		m_loop = loop;
		
		//-- Setup currently playing animation id.
		m_currentAnimationId = animationId;
	}
}

// ----------------------------------------------------------------------

inline const TrackAnimationController::PhysicalTrackTemplate &TrackAnimationController::Track::getPhysicalTrackTemplate() const
{
	return m_physicalTrackTemplate;
}

// ----------------------------------------------------------------------

void TrackAnimationController::Track::preExitIdleHook()
{
	DEBUG_REPORT_LOG(GraphicsDebugFlags::logCharacterSystem, ("preExitIdleHook() called\n"));

	if (m_isIdle) 
	{
		m_isIdle = false;

		typedef PhysicalTrackTemplate::StateChangeActionVector  ActionVector;
		
		const ActionVector                 &actions = getPhysicalTrackTemplate().getPreExitIdleActions();
		const ActionVector::const_iterator  endIt   = actions.end();
		for ( ActionVector::const_iterator  it = actions.begin(); it != endIt; ++it)
		{
			NOT_NULL(*it);
			(*it)->doAction(m_controller, *this);
		}
	}
}

// ----------------------------------------------------------------------

void TrackAnimationController::Track::postEnterIdleHook()
{
	DEBUG_REPORT_LOG(GraphicsDebugFlags::logCharacterSystem, ("postEnterIdleHook() called\n"));

	if (!m_isIdle) 
	{
		m_isIdle = true;

		typedef PhysicalTrackTemplate::StateChangeActionVector  ActionVector;
		
		const ActionVector                 &actions = getPhysicalTrackTemplate().getPostEnterIdleActions();
		const ActionVector::const_iterator  endIt   = actions.end();
		for ( ActionVector::const_iterator  it = actions.begin(); it != endIt; ++it)
		{
			NOT_NULL(*it);
			(*it)->doAction(m_controller, *this);
		}
	}
}

// ----------------------------------------------------------------------
/**
 * Create initial animation used by track.
 */

void TrackAnimationController::Track::createInitialAnimation()
{
	//-- Release existing animation.
	if (m_currentAnimation)
	{
		m_currentAnimation->setStateStoppedExplicitlyStopped(m_controller.getChannel());
		m_currentAnimation->release();
		m_currentAnimation = 0;
	}

	//-- Assign initial animation.
	m_currentAnimation = getPhysicalTrackTemplate().fetchInitialAnimation(m_controller.getTransformNameMap(), m_controller);
	if (m_currentAnimation)
		m_currentAnimation->setStatePlayInProgress(m_controller.getChannel());
}

// ----------------------------------------------------------------------

void TrackAnimationController::Track::clearLocomotionTranslation()
{
	m_locomotionTranslation = Vector::zero;
}

// ----------------------------------------------------------------------
void TrackAnimationController::Track::clearLocomotionRotation()
{
	m_locomotionRotation = Quaternion::identity;
}

// ======================================================================
