// ======================================================================
//
// SkeletalAnimation.cpp
// Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/SkeletalAnimation.h"

#include "clientSkeletalAnimation/AnimationNotification.h"
#include "clientSkeletalAnimation/ConfigClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/SkeletalAnimationTemplate.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"

#include <algorithm>
#include <limits>

#define  DO_VALIDATE_REFERENCE_COUNT  1

#if DO_VALIDATE_REFERENCE_COUNT != 0
#  define  VALIDATE_REFERENCE_COUNT() FATAL((m_referenceCount <= 0) || (m_referenceCount > s_maxValidReferenceCount), ("SkeletalAnimation has invalid reference count [%d], valid range is [0 .. %d].", m_referenceCount, s_maxValidReferenceCount))
#  define  ENSURE_ZERO_REFERENCES()   FATAL((m_referenceCount != 0), ("Expecting zero outstanding references in SkeletalAnimation::~SkeletalAnimation."))
#else
#  define  VALIDATE_REFERENCE_COUNT() NOP
#  define  ENSURE_ZERO_REFERENCES()   NOP
#endif

// ======================================================================

namespace SkeletalAnimationNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void remove();

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool  s_installed;

	int   s_maxValidReferenceCount;

}

using namespace SkeletalAnimationNamespace;

// ======================================================================

void SkeletalAnimationNamespace::remove()
{
	DEBUG_FATAL(!s_installed, ("SkeletalAnimation not installed."));
	s_installed = false;
}

// ======================================================================

void SkeletalAnimation::install()
{
	DEBUG_FATAL(s_installed, ("SkeletalAnimation already installed."));

	s_maxValidReferenceCount = ConfigClientSkeletalAnimation::getAnimationMaxValidReferenceCount();
	FATAL(s_maxValidReferenceCount < 1, ("s_maxValidReferenceCount should be >= 1, specified value is [%d].", s_maxValidReferenceCount));

	s_installed = true;
	ExitChain::add(SkeletalAnimationNamespace::remove, "SkeletalAnimation");
}

// ----------------------------------------------------------------------

/**
 * Compare the priority of two animations and return the results of the
 * comparison.
 *
 * Comparison is done in two phases: lhs and rhs compare transform priorities
 * first, then compare locomotion priorities.  If the transform and locomotion
 * priority comparison are different, then 0 is returned; otherwise, the comparison
 * value specified is returned.
 *
 * @param lhs  the first animation to compare.
 * @param rhs  the second animation to compare.
 *
 * @return  -1 if lhs's lowest priority is higher than rhs's highest priority;
 *           0 if there is an overlap in priorities between lhs and rhs;
 *           1 if rhs's lowest priority is higher than lhs's highest priority
 */

int SkeletalAnimation::compareAnimationPriority(SkeletalAnimation const &lhs, SkeletalAnimation const &rhs)
{
	int const transformCompareResult = compareAnimationTransformPriority(lhs, rhs);
	int const locomotionCompareResult = compareAnimationLocomotionPriority(lhs, rhs);

	if (transformCompareResult == locomotionCompareResult)
	{
		// The lhs/rhs dominance is identical for both locomotion and transforms.  Return the dominance.
		return transformCompareResult;
	}
	else
	{
		// The lhs/rhs dominance is different for transforms and locomotion priority.  We return a
		// "no clear dominance" result in this case.
		return 0;
	}
}

// ----------------------------------------------------------------------

int SkeletalAnimation::compareAnimationTransformPriority(SkeletalAnimation const &lhs, SkeletalAnimation const &rhs)
{
	//-- Get lhs priority.
	int lhsMinPriority;
	int lhsMaxPriority;

	lhs.getMinMaxTransformPriority(lhsMinPriority, lhsMaxPriority);

	//-- Get rhs priority.
	int rhsMinPriority;
	int rhsMaxPriority;

	rhs.getMinMaxTransformPriority(rhsMinPriority, rhsMaxPriority);

	if (lhsMinPriority > rhsMaxPriority)
	{
		// The lowest priority on lhs is greater than the highest priority of rhs.
		// The lhs animation completely dominates the rhs animation in priority.
		return -1;
	}
	else if (rhsMinPriority > lhsMaxPriority)
	{
		// The lowest priority on rhs is greater than the highest priority of lhs.
		// The rhs animation completely dominates the lhs animation in priority.
		return 1;
	}
	else
	{
		// There is some kind of overlap on priority.  Neither animation is completely dominant.
		// NOTE: one animation may be dominant over the other on a particular transform.  This
		//       is comparing the min/max of all transforms present in each animation.
		return 0;
	}
}

// ----------------------------------------------------------------------

int SkeletalAnimation::compareAnimationLocomotionPriority(SkeletalAnimation const &lhs, SkeletalAnimation const &rhs)
{
	int const lhsPriority = lhs.getLocomotionPriority();
	int const rhsPriority = rhs.getLocomotionPriority();

	if (lhsPriority > rhsPriority)
		return -1;
	else if (rhsPriority > lhsPriority)
		return 1;
	else
		return 0;
}

// ======================================================================

void SkeletalAnimation::release() const
{
	VALIDATE_REFERENCE_COUNT();

	--m_referenceCount;
	if (m_referenceCount == 0)
	{
		// we're no longer needed
		delete const_cast<SkeletalAnimation*>(this);
	}
}

// ----------------------------------------------------------------------
/**
 * Return the minimum and maximum priority values for transform priorities
 * on this animation.
 *
 * @param min  will contain the minimum transform priority value on exit.
 * @param max  will contain the maximum transform priority value on exit.
 */

void SkeletalAnimation::getMinMaxTransformPriority(int &min, int &max) const
{
	VALIDATE_REFERENCE_COUNT();

	int const transformCount = getTransformCount();
	
	if (transformCount > 0)
	{
		min = std::numeric_limits<int>::max();
		max = std::numeric_limits<int>::min();

		for (int i = 0; i < transformCount; ++i)
		{
			int const priority = getTransformPriority(i);
			min = std::min(min, priority);
			max = std::max(max, priority);
		}
	}
	else
	{
		min = -1;
		max = -1;
	}
}

// ----------------------------------------------------------------------
/**
 * Retrieve the name of the animation template for this animation or
 * an animation that is more toward the leaf of the animation tree.
 *
 * The result returned from this function is suitable for debug logging
 * messages.  The result is obtained by using the resolveAnimation() function,
 * running until we have an animation template with a non-zero-length
 * name, resolveAnimation() returns NULL or resolveAnimation() returns
 * the same animation as we're currently examining.
 *
 * @return  the first non-zero-length animation template (an .ans file if
 *          one exists).  It can return a zero-length name if no .ans
 *          files are in the resolveAnimation() active animation path.
 */

const CrcString &SkeletalAnimation::getLeafAnimationTemplateName() const
{
	VALIDATE_REFERENCE_COUNT();

	SkeletalAnimation const *focusAnimation = this;

	while (focusAnimation)
	{
		//-- Check if the focus animation's template has a non-zero-length name.
		SkeletalAnimationTemplate const *const saTemplate = focusAnimation->getSkeletalAnimationTemplate();
		if (saTemplate)
		{
			CrcString const &name = saTemplate->getName();
			if (strlen(name.getString()) > 0)
			{
				// Found one.
				return name;
			}
		}

		//-- Move on to the next animation in the resolve chain.
		//   NOTE: resolveSkeletalAnimation() should be a const function.
		SkeletalAnimation const *const oldAnimation = focusAnimation;
		focusAnimation = const_cast<SkeletalAnimation*>(oldAnimation)->resolveSkeletalAnimation();

		if (focusAnimation == oldAnimation)
		{
			// Resolve could not move any closer to the root.  Nothing more to search.
			break;
		}
	}

	//-- Didn't find anything.
	// @todo add an empty to CrcPersistentString and CrcTemporaryString.
	return CrcLowerString::empty;
}

// ----------------------------------------------------------------------

void SkeletalAnimation::setNotification(AnimationNotification *notification, int channel)
{
	VALIDATE_REFERENCE_COUNT();

	if (notification == m_notification)
		return;

	DEBUG_FATAL(m_notification && notification, ("SkeletalAnimation: attempting to set notification to a new unique notification, unexpected."));

	//-- Note: we don't hold a reference to the notification because the notification holds a reference to the animation.
	//   We would have a reference count loop that would never get freed if I held a reference here.  The choice for holding
	//   the reference on the notification is because an animation play caller will already have a reference to the notification whereas
	//   nothing necessarily keeps a reference to the animation if it is stopped on return.
	if (notification)
		notification->attachAnimation(channel, this);

	m_notification = notification;
}

// ----------------------------------------------------------------------

bool SkeletalAnimation::hasNotification() const
{
	VALIDATE_REFERENCE_COUNT();

	return (m_notification != NULL);
}

// ----------------------------------------------------------------------

void SkeletalAnimation::setStateInQueue(int channel)
{
	VALIDATE_REFERENCE_COUNT();

	if (m_notification)
		m_notification->setState(channel, AnimationNotification::NS_inQueue);
}

// ----------------------------------------------------------------------

void SkeletalAnimation::setStatePlayInProgress(int channel)
{
	VALIDATE_REFERENCE_COUNT();

	if (m_notification)
		m_notification->setState(channel, AnimationNotification::NS_playInProgress);
}

// ----------------------------------------------------------------------

void SkeletalAnimation::setStateStoppedNormal(int channel)
{
	VALIDATE_REFERENCE_COUNT();

	if (m_notification)
		m_notification->setState(channel, AnimationNotification::NS_stoppedNormal);
}

// ----------------------------------------------------------------------

void SkeletalAnimation::setStateStoppedExplicitlyStopped(int channel)
{
	VALIDATE_REFERENCE_COUNT();

	if (m_notification)
		m_notification->setState(channel, AnimationNotification::NS_stoppedExplicitlyStopped);
}

// ----------------------------------------------------------------------

void SkeletalAnimation::setStateStoppedInterruped(int channel)
{
	VALIDATE_REFERENCE_COUNT();

	if (m_notification)
		m_notification->setState(channel, AnimationNotification::NS_stoppedInterrupted);
}

// ----------------------------------------------------------------------

void SkeletalAnimation::setStateStoppedEjectedFromQueue(int channel)
{
	VALIDATE_REFERENCE_COUNT();

	if (m_notification)
		m_notification->setState(channel, AnimationNotification::NS_stoppedEjectedFromQueue);
}

// ----------------------------------------------------------------------

void SkeletalAnimation::setStateSameTrackTrumped(int channel)
{
	VALIDATE_REFERENCE_COUNT();

	if (m_notification)
		m_notification->setState(channel, AnimationNotification::NS_stoppedSameTrackTrumped);
}

// ----------------------------------------------------------------------

void SkeletalAnimation::setStateStoppedMisbehaving(int channel)
{
	VALIDATE_REFERENCE_COUNT();

	if (m_notification)
		m_notification->setState(channel, AnimationNotification::NS_stoppedMisbehaving);
}

// ----------------------------------------------------------------------

void SkeletalAnimation::handleAnimationMessage(int channel, CrcString const &messageName, float elapsedTimeSinceMessageTrigger)
{
	VALIDATE_REFERENCE_COUNT();

	if (m_notification)
		m_notification->handleAnimationMessage(channel, messageName, elapsedTimeSinceMessageTrigger);
}

// ======================================================================

SkeletalAnimation::SkeletalAnimation(const SkeletalAnimationTemplate *skeletalAnimationTemplate):
	m_referenceCount(0),
	m_skeletalAnimationTemplate(skeletalAnimationTemplate),
	m_notification(0)
{
	// keep our own reference to this
	if (m_skeletalAnimationTemplate)
		m_skeletalAnimationTemplate->fetch();
}

// ----------------------------------------------------------------------

SkeletalAnimation::~SkeletalAnimation()
{
	ENSURE_ZERO_REFERENCES();
	m_notification = 0;

	if (m_skeletalAnimationTemplate)
	{
		m_skeletalAnimationTemplate->release();
		m_skeletalAnimationTemplate = 0;
	}
}

// ======================================================================
