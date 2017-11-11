// ======================================================================
//
// PriorityBlendAnimation.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/PriorityBlendAnimation.h"

#include "clientSkeletalAnimation/PriorityBlendAnimationTemplate.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(PriorityBlendAnimation, true, 0, 0, 0);

// ======================================================================
// class PriorityBlendAnimation: INLINES (PRIVATE)
// ======================================================================

inline SkeletalAnimation *PriorityBlendAnimation::getAnimationArrayEntry(int index)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, static_cast<int>(MAX_ANIMATION_COUNT));
	return m_animationArray[index];
} //lint !e1762 // could be made const --- returns a non-const pointer, I think it is logically non-const.

// ----------------------------------------------------------------------

inline SkeletalAnimation const *PriorityBlendAnimation::getAnimationArrayEntry(int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, static_cast<int>(MAX_ANIMATION_COUNT));
	return m_animationArray[index];
}

// ======================================================================
// class PriorityBlendAnimation: PUBLIC
// ======================================================================

int PriorityBlendAnimation::getAnimationCount() const
{
	return MAX_ANIMATION_COUNT;
}

// ----------------------------------------------------------------------

SkeletalAnimation *PriorityBlendAnimation::getAnimation(int index)
{
	return getAnimationArrayEntry(index);
}

// ----------------------------------------------------------------------

SkeletalAnimation const *PriorityBlendAnimation::getAnimation(int index) const
{
	return getAnimationArrayEntry(index);
}

// ----------------------------------------------------------------------

SkeletalAnimation *PriorityBlendAnimation::getPrimaryAnimation()
{
	return getAnimationArrayEntry(m_primaryAnimationIndex);
}

// ----------------------------------------------------------------------

SkeletalAnimation const *PriorityBlendAnimation::getPrimaryAnimation() const
{
	return getAnimationArrayEntry(m_primaryAnimationIndex);
}

// ----------------------------------------------------------------------

bool PriorityBlendAnimation::doAlterSingleCycle(float deltaTime, SkeletalAnimation *&replacementAnimation, float &deltaTimeRemaining)
{
	//-- Handle the primary animation like a normal animation.

	//... get primary animation.
	SkeletalAnimation *const primaryAnimation = getAnimationArrayEntry(m_primaryAnimationIndex);
	if (!primaryAnimation)
		return false;

	//... alter primary animation.
	deltaTimeRemaining = 0.0f;
	bool  const primaryAlterReturnValue = primaryAnimation->alterSingleCycle(deltaTime, replacementAnimation, deltaTimeRemaining);
	float const timeAppliedToPrimaryAnimation = deltaTime - deltaTimeRemaining;

	//-- Apply the amount of time sucked up by the primary animation to all the other animations.
	for (int animationIndex = 0; animationIndex < MAX_ANIMATION_COUNT; ++animationIndex)
	{
		// Skip primary animation, we already altered it.
		if (animationIndex == m_primaryAnimationIndex)
			continue;

		SkeletalAnimation *componentAnimation;

		float componentDeltaTime         = timeAppliedToPrimaryAnimation;
		float previousComponentDeltaTime = componentDeltaTime + 1.0f;

		while ((componentDeltaTime > 0.0f) && (componentDeltaTime < previousComponentDeltaTime) && ((componentAnimation = getAnimationArrayEntry(animationIndex)) != NULL))
		{
			previousComponentDeltaTime = componentDeltaTime;

			SkeletalAnimation *componentReplacementAnimation = NULL;
			bool const componentResult = componentAnimation->alterSingleCycle(componentDeltaTime, componentReplacementAnimation, componentDeltaTime);
			if (!componentResult)
			{
				//-- We need to shut down this animation.
				if (componentReplacementAnimation != NULL)
					componentReplacementAnimation->fetch();

				componentAnimation->release();
				m_animationArray[animationIndex] = componentReplacementAnimation;
			}
		}
	}

	return primaryAlterReturnValue;
}

// ======================================================================
// class PriorityBlendAnimation: PRIVATE
// ======================================================================

PriorityBlendAnimation::PriorityBlendAnimation(PriorityBlendAnimationTemplate const &animationTemplate, TransformNameMap const &transformNameMap, SkeletalAnimation *animation0, SkeletalAnimation *animation1, int primaryAnimationIndex) :
	BasePriorityBlendAnimation(&animationTemplate, transformNameMap),
	m_primaryAnimationIndex(primaryAnimationIndex)	
{
	if (animation0)
		animation0->fetch();
	m_animationArray[0] = animation0;

	if (animation1)
		animation1->fetch();
	m_animationArray[1] = animation1;

	for (int i = 2; i < MAX_ANIMATION_COUNT; ++i) //lint !e681 // loop is not enetered // true only when MAX_ANIMATION_COUNT <= 2
		m_animationArray[i] = NULL;
}

// ----------------------------------------------------------------------

PriorityBlendAnimation::~PriorityBlendAnimation()
{
	for (int i = 0; i < MAX_ANIMATION_COUNT; ++i)
	{
		if (m_animationArray[i])
		{
			m_animationArray[i]->release();
			m_animationArray[i] = NULL;
		}
	}
}

// ======================================================================
