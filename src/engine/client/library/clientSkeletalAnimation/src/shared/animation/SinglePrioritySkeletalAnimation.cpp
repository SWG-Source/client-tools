// ======================================================================
//
// SinglePrioritySkeletalAnimation.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/SinglePrioritySkeletalAnimation.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(SinglePrioritySkeletalAnimation, true, 0, 0, 0);

// ======================================================================

SinglePrioritySkeletalAnimation::SinglePrioritySkeletalAnimation(SkeletalAnimation &animation, int uniformTransformPriority, int locomotionPriority) :
	SkeletalAnimation(0),
	m_animation(animation),
	m_uniformTransformPriority(uniformTransformPriority),
	m_locomotionPriority(locomotionPriority)
{
	//-- Fetch local references.
	m_animation.fetch();
}

// ----------------------------------------------------------------------

int SinglePrioritySkeletalAnimation::getTransformPriority(int index) const
{
	UNREF(index);

	//-- All transforms in this animation have the same priority, return it.
	return m_uniformTransformPriority;
}

// ----------------------------------------------------------------------

int SinglePrioritySkeletalAnimation::getLocomotionPriority() const
{
	return m_locomotionPriority;
}

// ----------------------------------------------------------------------

bool SinglePrioritySkeletalAnimation::alterSingleCycle(float deltaTime, SkeletalAnimation *&replacementAnimation, float &deltaTimeRemaining)
{
	return m_animation.alterSingleCycle(deltaTime, replacementAnimation, deltaTimeRemaining);
}

// ----------------------------------------------------------------------

void SinglePrioritySkeletalAnimation::startNewCycle()
{
	m_animation.startNewCycle();
}

// ----------------------------------------------------------------------

int SinglePrioritySkeletalAnimation::getTransformCount() const
{
	return m_animation.getTransformCount();
}

// ----------------------------------------------------------------------

void SinglePrioritySkeletalAnimation::evaluateTransformComponents(int index, Quaternion &rotation, Vector &translation)
{
	m_animation.evaluateTransformComponents(index, rotation, translation);
}

// ----------------------------------------------------------------------

void SinglePrioritySkeletalAnimation::getScaledLocomotion(Quaternion &rotation, Vector &translation) const
{
	m_animation.getScaledLocomotion(rotation, translation);
}

// ----------------------------------------------------------------------

float SinglePrioritySkeletalAnimation::getCycleScaledLocomotionDistance() const
{
	return m_animation.getCycleScaledLocomotionDistance();
}

// ----------------------------------------------------------------------

int SinglePrioritySkeletalAnimation::getFrameCount() const
{
	return m_animation.getFrameCount();
}

// ----------------------------------------------------------------------

float SinglePrioritySkeletalAnimation::getRecordedFramesPerSecond() const
{
	return m_animation.getRecordedFramesPerSecond();
}

// ----------------------------------------------------------------------

void SinglePrioritySkeletalAnimation::setPlaybackFramesPerSecond(float playbackFramesPerSecond)
{
	m_animation.setPlaybackFramesPerSecond(playbackFramesPerSecond);
}

// ----------------------------------------------------------------------

float SinglePrioritySkeletalAnimation::getPlaybackFramesPerSecond() const
{
	return m_animation.getPlaybackFramesPerSecond();
}

// ----------------------------------------------------------------------

int SinglePrioritySkeletalAnimation::getMessageCount() const
{
	return m_animation.getMessageCount();
}

// ----------------------------------------------------------------------

const CrcLowerString &SinglePrioritySkeletalAnimation::getMessageName(int index) const
{
	return m_animation.getMessageName(index);
}

// ----------------------------------------------------------------------

void SinglePrioritySkeletalAnimation::getSignaledMessages(stdvector<int>::fwd &signaledMessageIndices, stdvector<float>::fwd &elapsedTimeSinceSignal) const
{
	m_animation.getSignaledMessages(signaledMessageIndices, elapsedTimeSinceSignal);
}

// ----------------------------------------------------------------------

SkeletalAnimation *SinglePrioritySkeletalAnimation::resolveSkeletalAnimation()
{
	return &m_animation; //lint !e1536 // (Warning -- Exposing low access member 'SinglePrioritySkeletalAnimation::m_animation') -- Effective C++ #30 // It's okay.
}

// ======================================================================

SinglePrioritySkeletalAnimation::~SinglePrioritySkeletalAnimation()
{
	//-- Release local references.
	m_animation.release();
}

// ======================================================================
