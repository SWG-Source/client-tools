// ======================================================================
//
// BasePriorityBlendAnimation.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/BasePriorityBlendAnimation.h"

#include "clientSkeletalAnimation/ConfigClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/TransformNameMap.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedMath/Quaternion.h"
#include "sharedMath/Vector.h"

#include <algorithm>
#include <limits>
#include <vector>

// ======================================================================

// If non-zero, DEBUG_REPORT_LOG will dump copious amounts of info related to blending.
#define DUMP_BLEND_ACTIVITY 0

// ======================================================================

namespace BasePriorityBlendAnimationNamespace
{
	float cs_blendTime            = 0.25f;
	float cs_blendFractionPerTime = 1.0f / cs_blendTime;

	char const * const cs_blendStateName[] =
	{
		"BS_playA",
		"BS_blendToB",
		"BS_playB",
		"BS_blendToA"
	};
}

using namespace BasePriorityBlendAnimationNamespace;

// ======================================================================

struct BasePriorityBlendAnimation::BlendData
{
public:

	BlendData();

	void update(float deltaTime, int priorityA, int priorityB);

public:

	BlendState  m_currentState;
	float       m_blendFraction;

};

// ======================================================================
// class BasePriorityBlendAnimation::BlendData
// ======================================================================

BasePriorityBlendAnimation::BlendData::BlendData() :
	m_currentState(BS_playB),
	m_blendFraction(1.0f)
{
}

// ----------------------------------------------------------------------

void BasePriorityBlendAnimation::BlendData::update(float deltaTime, int priorityA, int priorityB)
{
	switch (m_currentState)
	{
		case BS_playA:
		{
			//-- Handle pre-update state exit conditions.
			if (priorityB > priorityA)
			{
				// Switch to blending to B.
				m_currentState = BS_blendToB;
				break;
			}

			//-- Update for state.
			// Nothing to do, no change to blend fraction.
			// m_blendFraction = 0.0f;
		}
		break;

		case BS_blendToB:
		{
			//-- Handle pre-update state exit conditions.
			if (priorityA >= priorityB)
			{
				// Switch back to blending to A.
				m_currentState = BS_blendToA;
				break;
			}

			//-- Update for state.
			m_blendFraction += cs_blendFractionPerTime * deltaTime;

			//-- Handle pre-update state exit conditions.
			if (m_blendFraction >= 1.0f)
			{
				m_currentState  = BS_playB;
				m_blendFraction = 1.0f;
			}
		}
		break;

		case BS_playB:
		{
			//-- Handle pre-update state exit conditions.
			if (priorityA >= priorityB)
			{
				// Switch to blending to A
				m_currentState = BS_blendToA;
				break;
			}

			//-- Update for state.
			// Nothing to do, no change to blend fraction.
			// m_blendFraction = 1.0f;
		}
		break;

		case BS_blendToA:
		{
			//-- Handle pre-update state exit conditions.
			if (priorityB > priorityA)
			{
				// Switch back to blending to B.
				m_currentState = BS_blendToB;
				break;
			}

			//-- Update for state.
			m_blendFraction -= cs_blendFractionPerTime * deltaTime;

			//-- Handle pre-update state exit conditions.
			if (m_blendFraction <= 0.0f)
			{
				m_currentState  = BS_playA;
				m_blendFraction = 0.0f;
			}
		}
		break;
	}
}

// ======================================================================
// class BasePriorityBlendAnimation: PUBLIC STATIC
// ======================================================================

void BasePriorityBlendAnimation::install()
{
	cs_blendTime = ConfigClientSkeletalAnimation::getBlendTime();
	cs_blendFractionPerTime = 1.0f / cs_blendTime;
}

float BasePriorityBlendAnimation::getBlendTime()
{
	return cs_blendTime;
}

// ======================================================================
// class BasePriorityBlendAnimation: PUBLIC
// ======================================================================

void BasePriorityBlendAnimation::startNewCycle()
{
	// This default may not be acceptable for all uses.  See TrackAnimationController's track priority blender,
	// which doesn't do anything here.

	int const animationCount = getAnimationCount();
	for (int animationIndex = 0; animationIndex < animationCount; ++animationIndex)
	{
		SkeletalAnimation *const animation = getAnimation(animationIndex);
		if (animation)
			animation->startNewCycle();
	}
}

// ----------------------------------------------------------------------

int BasePriorityBlendAnimation::getTransformCount() const
{
	return m_transformCount;
}

// ----------------------------------------------------------------------

void BasePriorityBlendAnimation::evaluateTransformComponents(int index, Quaternion &rotation, Vector &translation)
{
	DEBUG_FATAL(getAnimationCount() < 2, ("BasePriorityBlendAnimation: expecting at least two component animations but found [%d].", getAnimationCount()));

	const BlendData &blendData = (*m_blendDataVector)[static_cast<BlendDataVector::size_type>(index)];

	switch (blendData.m_currentState)
	{
		case BS_blendToA:
		case BS_blendToB:
			{
				//-- Retrieve first animation's values.
				Quaternion  rotation1(Quaternion::identity);
				Vector      translation1(Vector::zero);
				doEvaluateTransformComponents(0, index, rotation1, translation1);
				
				//-- Retrieve second animation's values.
				Quaternion  rotation2(rotation1);
				Vector      translation2(translation1);
				doEvaluateTransformComponents(1, index, rotation2, translation2);
				
				//-- Do the blend.
				translation = ((1.0f - blendData.m_blendFraction) * translation1) + (blendData.m_blendFraction * translation2);
				rotation    = rotation1.slerp(rotation2, blendData.m_blendFraction);
			}
			break;
		
		case BS_playA:
			//-- Retrieve first animation's values.
			doEvaluateTransformComponents(0, index, rotation, translation);
			break;

		case BS_playB:
			//-- Retrieve first animation's values.
			doEvaluateTransformComponents(1, index, rotation, translation);
			break;
	}
}

// ----------------------------------------------------------------------

int BasePriorityBlendAnimation::getTransformPriority(int index) const
{
	DEBUG_FATAL(getAnimationCount() < 2, ("BasePriorityBlendAnimation: expecting at least two component animations but found [%d].", getAnimationCount()));
	const SkeletalAnimation *const animation1 = getAnimation(0);
	const SkeletalAnimation *const animation2 = getAnimation(1);

	const int animation1Priority = (animation1 ? animation1->getTransformPriority(index) : std::numeric_limits<int>::min());
	const int animation2Priority = (animation2 ? animation2->getTransformPriority(index) : std::numeric_limits<int>::min());

	return std::max(animation1Priority, animation2Priority);
}

// ----------------------------------------------------------------------

int BasePriorityBlendAnimation::getLocomotionPriority() const
{
	SkeletalAnimation const *const primaryAnimation = getPrimaryAnimation();
	return (primaryAnimation != NULL) ? primaryAnimation->getLocomotionPriority() : std::numeric_limits<int>::min();
}

// ----------------------------------------------------------------------

void BasePriorityBlendAnimation::getScaledLocomotion(Quaternion &rotation, Vector &translation) const
{
	DEBUG_FATAL(getAnimationCount() < 2, ("BasePriorityBlendAnimation: expecting at least two component animations but found [%d].", getAnimationCount()));

	const BlendData &blendData = m_blendDataVector->back();

	switch (blendData.m_currentState)
	{
		case BS_blendToA:
		case BS_blendToB:
			{
				SkeletalAnimation const * const animation1 = getAnimation(0);
				SkeletalAnimation const * const animation2 = getAnimation(1);

				if (animation1 != animation2) 
				{
					//-- Retrieve first animation's values.
					Quaternion  rotation1(Quaternion::identity);
					Vector      translation1(Vector::zero);
					
					if (animation1)
						animation1->getScaledLocomotion(rotation1, translation1);
					
					//-- Retrieve second animation's values.
					Quaternion  rotation2(rotation1);
					Vector      translation2(translation1);
					
					if (animation2)
						animation2->getScaledLocomotion(rotation2, translation2);
					
					//-- Do the blend.
					translation = ((1.0f - blendData.m_blendFraction) * translation1) + (blendData.m_blendFraction * translation2);
					rotation    = rotation1.slerp(rotation2, blendData.m_blendFraction);
				}
				else if (animation1)
				{
					animation1->getScaledLocomotion(rotation, translation);
				}
				else
				{
					rotation    = Quaternion::identity;
					translation = Vector::zero;
				}
			}
			break;
		
		case BS_playA:
			{
				//-- Retrieve first animation's values.
				const SkeletalAnimation *const animation = getAnimation(0);
				if (animation)
					animation->getScaledLocomotion(rotation, translation);
				else
				{
					rotation    = Quaternion::identity;
					translation = Vector::zero;
				}
			}
			break;

		case BS_playB:
			{
				//-- Retrieve first animation's values.
				const SkeletalAnimation *const animation = getAnimation(1);
				if (animation)
					animation->getScaledLocomotion(rotation, translation);
				else
				{
					rotation    = Quaternion::identity;
					translation = Vector::zero;
				}
			}
			break;
	}
}

// ----------------------------------------------------------------------

float BasePriorityBlendAnimation::getCycleScaledLocomotionDistance() const
{
	SkeletalAnimation const *const primaryAnimation = getPrimaryAnimation();
	return (primaryAnimation != NULL) ? primaryAnimation->getCycleScaledLocomotionDistance() : 0.0f;
}

// ----------------------------------------------------------------------

int BasePriorityBlendAnimation::getFrameCount() const
{
	SkeletalAnimation const *const primaryAnimation = getPrimaryAnimation();
	return (primaryAnimation != NULL) ? primaryAnimation->getFrameCount() : 0;
}

// ----------------------------------------------------------------------

float BasePriorityBlendAnimation::getRecordedFramesPerSecond() const
{
	SkeletalAnimation const *const primaryAnimation = getPrimaryAnimation();
	return (primaryAnimation != NULL) ? primaryAnimation->getRecordedFramesPerSecond() : 30.0f;
}

// ----------------------------------------------------------------------

void BasePriorityBlendAnimation::setPlaybackFramesPerSecond(float playbackFramesPerSecond)
{
	SkeletalAnimation *const primaryAnimation = getPrimaryAnimation();
	if (primaryAnimation)
		primaryAnimation->setPlaybackFramesPerSecond(playbackFramesPerSecond);
}

// ----------------------------------------------------------------------

float BasePriorityBlendAnimation::getPlaybackFramesPerSecond() const
{
	SkeletalAnimation const *const primaryAnimation = getPrimaryAnimation();
	return (primaryAnimation != NULL) ? primaryAnimation->getPlaybackFramesPerSecond() : 30.0f;
}

// ----------------------------------------------------------------------

int BasePriorityBlendAnimation::getMessageCount() const
{
	SkeletalAnimation const *const primaryAnimation = getPrimaryAnimation();
	return (primaryAnimation != NULL) ? primaryAnimation->getMessageCount() : 0;
}

// ----------------------------------------------------------------------

const CrcLowerString &BasePriorityBlendAnimation::getMessageName(int index) const
{
	SkeletalAnimation const *const primaryAnimation = getPrimaryAnimation();
	return (primaryAnimation != NULL) ? primaryAnimation->getMessageName(index) : CrcLowerString::empty;
}

// ----------------------------------------------------------------------

void BasePriorityBlendAnimation::getSignaledMessages(stdvector<int>::fwd &signaledMessageIndices, stdvector<float>::fwd &elapsedTimeSinceSignal) const
{
	SkeletalAnimation const *const primaryAnimation = getPrimaryAnimation();
	if (primaryAnimation)
		primaryAnimation->getSignaledMessages(signaledMessageIndices, elapsedTimeSinceSignal);
}

// ----------------------------------------------------------------------

SkeletalAnimation *BasePriorityBlendAnimation::resolveSkeletalAnimation()
{
	return getPrimaryAnimation();
}

// ======================================================================
// class BasePriorityBlendAnimation: PROTECTED
// ======================================================================

BasePriorityBlendAnimation::BasePriorityBlendAnimation(SkeletalAnimationTemplate const *animationTemplate, TransformNameMap const &transformNameMap) :
	SkeletalAnimation(animationTemplate),
	m_transformCount(transformNameMap.getTransformCount()),
	m_blendDataVector(new BlendDataVector(static_cast<BlendDataVector::size_type>(transformNameMap.getTransformCount() + 1)))
{
}

// ----------------------------------------------------------------------

BasePriorityBlendAnimation::~BasePriorityBlendAnimation()
{
	delete m_blendDataVector;
}

// ----------------------------------------------------------------------
/**
 * Returns the animation that should be used as the source animation in
 * many of the default implementations in this class.
 */

SkeletalAnimation *BasePriorityBlendAnimation::getPrimaryAnimation()
{
	// By default, return the animation with locomotion priority.
	DEBUG_FATAL(getAnimationCount() < 2, ("BasePriorityBlendAnimation: expecting at least two component animations but found [%d].", getAnimationCount()));
	SkeletalAnimation *const animation1 = getAnimation(0);
	SkeletalAnimation *const animation2 = getAnimation(1);

	int const animation1Priority = (animation1 ? animation1->getLocomotionPriority() : std::numeric_limits<int>::min());
	int const animation2Priority = (animation2 ? animation2->getLocomotionPriority() : std::numeric_limits<int>::min());

	return (animation1Priority >= animation2Priority) ? animation1 : animation2;
}

// ----------------------------------------------------------------------

SkeletalAnimation const *BasePriorityBlendAnimation::getPrimaryAnimation() const
{
	// By default, return the animation with locomotion priority.
	DEBUG_FATAL(getAnimationCount() < 2, ("BasePriorityBlendAnimation: expecting at least two component animations but found [%d].", getAnimationCount()));
	SkeletalAnimation const *const animation1 = getAnimation(0);
	SkeletalAnimation const *const animation2 = getAnimation(1);

	int const animation1Priority = (animation1 ? animation1->getLocomotionPriority() : std::numeric_limits<int>::min());
	int const animation2Priority = (animation2 ? animation2->getLocomotionPriority() : std::numeric_limits<int>::min());

	return (animation1Priority >= animation2Priority) ? animation1 : animation2;
}

// ----------------------------------------------------------------------

void BasePriorityBlendAnimation::doEvaluateTransformComponents(int animationIndex, int transformIndex, Quaternion &rotation, Vector &translation)
{
	// Default implementation is to get the specified component animation, then ask it to evaluate its components.
	// This is overridden by the track blender version where a track maintains most recent values for animations after they complete.
	SkeletalAnimation *const animation = getAnimation(animationIndex);
	if (animation)
		animation->evaluateTransformComponents(transformIndex, rotation, translation);
	else
	{
		rotation    = Quaternion::identity;
		translation = Vector::zero;
	}
}

// ----------------------------------------------------------------------

bool BasePriorityBlendAnimation::allBlendStatesMatch(BlendState blendState) const
{
	const BlendDataVector::const_iterator endIt = m_blendDataVector->end();
	for (BlendDataVector::const_iterator it = m_blendDataVector->begin(); it != endIt; ++it)
	{
		if (it->m_currentState != blendState)
			return false;
	}

	//-- All match.
	return true;
}

// ======================================================================
// class BasePriorityBlendAnimation: PRIVATE
// ======================================================================

bool BasePriorityBlendAnimation::alterSingleCycle(float deltaTime, SkeletalAnimation *&replacementAnimation, float &deltaTimeRemaining)
{
	//-- Alter animations.
	bool const keepAlive = doAlterSingleCycle(deltaTime, replacementAnimation, deltaTimeRemaining);

	//-- Update blender data.
	updateBlenderData(deltaTime - deltaTimeRemaining);

	return keepAlive;
}

// ----------------------------------------------------------------------

void BasePriorityBlendAnimation::updateBlenderData(float elapsedTime)
{
	DEBUG_FATAL(getAnimationCount() < 2, ("BasePriorityBlendAnimation: expecting at least two component animations but found [%d].", getAnimationCount()));
	const SkeletalAnimation *const animation1 = getAnimation(0);
	const SkeletalAnimation *const animation2 = getAnimation(1);

	//-- Update blender states.
	if (!m_blendDataVector->empty())
	{
		// Update per-transform blends.
		const BlendDataVector::size_type  transformCount = m_blendDataVector->size() - 1;

		for (BlendDataVector::size_type i = 0; i < transformCount; ++i)
		{
			const int animation1Priority = (animation1 ? animation1->getTransformPriority(static_cast<int>(i)) : std::numeric_limits<int>::min());
			const int animation2Priority = (animation2 ? animation2->getTransformPriority(static_cast<int>(i)) : std::numeric_limits<int>::min());
			
			(*m_blendDataVector)[i].update(elapsedTime, animation1Priority, animation2Priority);
		}

		// Update locomotion blend.
		if (transformCount > 0)
		{
			const int animation1Priority = (animation1 ? animation1->getLocomotionPriority() : std::numeric_limits<int>::min());
			const int animation2Priority = (animation2 ? animation2->getLocomotionPriority() : std::numeric_limits<int>::min());

			BlendData &locomotionBlendData = m_blendDataVector->back();

#if DUMP_BLEND_ACTIVITY
			static int listNumber = 0;

			++listNumber;
			DEBUG_REPORT_LOG(true, ("PBASC: %d: pre (p1=%d,p2=%d,state=%s,blend=%.2f), ", 
				listNumber, 
				animation1Priority, 
				animation2Priority, 
				cms_blendStateName[static_cast<size_t>(locomotionBlendData.m_currentState)],
				locomotionBlendData.m_blendFraction
				));
#endif

			// Update the blend variable.
			locomotionBlendData.update(elapsedTime, animation1Priority, animation2Priority);

#if DUMP_BLEND_ACTIVITY
			DEBUG_REPORT_LOG(true, ("post(state=%s,blend=%.2f).\n",
				cms_blendStateName[static_cast<size_t>(locomotionBlendData.m_currentState)],
				locomotionBlendData.m_blendFraction
				));
#endif

		}
	}
}

// ======================================================================
