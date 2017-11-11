// ======================================================================
//
// MaskedPrioritySkeletalAnimation.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/MaskedPrioritySkeletalAnimation.h"

#include "clientSkeletalAnimation/TransformMask.h"
#include "clientSkeletalAnimation/TransformNameMap.h"

#include <vector>

// ======================================================================

MaskedPrioritySkeletalAnimation::MaskedPrioritySkeletalAnimation(SkeletalAnimation &animation, const TransformNameMap &transformNameMap, const TransformMask &transformMask, int inGroupTransformPriority, int outGroupTransformPriority, int locomotionPriority) :
	SkeletalAnimation(0),
	m_animation(animation),
	m_transformPriorities(new IntVector(static_cast<IntVector::size_type>(transformNameMap.getTransformCount()))),
	m_locomotionPriority(locomotionPriority)
{
	//-- Fetch local references.
	m_animation.fetch();

	//-- Gather up transform priorities from the transform mask.
	transformMask.fillTransformPriorityVector(transformNameMap, inGroupTransformPriority, outGroupTransformPriority, *m_transformPriorities);
}

// ----------------------------------------------------------------------

int MaskedPrioritySkeletalAnimation::getTransformPriority(int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, static_cast<int>(m_transformPriorities->size()));

	//-- Return the transform priority for this transform.
	return (*m_transformPriorities)[static_cast<IntVector::size_type>(index)];
}

// ----------------------------------------------------------------------

int MaskedPrioritySkeletalAnimation::getLocomotionPriority() const
{
	return m_locomotionPriority;
}

// ----------------------------------------------------------------------

bool MaskedPrioritySkeletalAnimation::alterSingleCycle(float deltaTime, SkeletalAnimation *&replacementAnimation, float &deltaTimeRemaining)
{
	return m_animation.alterSingleCycle(deltaTime, replacementAnimation, deltaTimeRemaining);
}

// ----------------------------------------------------------------------

void MaskedPrioritySkeletalAnimation::startNewCycle()
{
	m_animation.startNewCycle();
}

// ----------------------------------------------------------------------

int MaskedPrioritySkeletalAnimation::getTransformCount() const
{
	return m_animation.getTransformCount();
}

// ----------------------------------------------------------------------

void MaskedPrioritySkeletalAnimation::evaluateTransformComponents(int index, Quaternion &rotation, Vector &translation)
{
	m_animation.evaluateTransformComponents(index, rotation, translation);
}

// ----------------------------------------------------------------------

void MaskedPrioritySkeletalAnimation::getScaledLocomotion(Quaternion &rotation, Vector &translation) const
{
	m_animation.getScaledLocomotion(rotation, translation);
}

// ----------------------------------------------------------------------

float MaskedPrioritySkeletalAnimation::getCycleScaledLocomotionDistance() const
{
	return m_animation.getCycleScaledLocomotionDistance();
}

// ----------------------------------------------------------------------

int MaskedPrioritySkeletalAnimation::getFrameCount() const
{
	return m_animation.getFrameCount();
}

// ----------------------------------------------------------------------

float MaskedPrioritySkeletalAnimation::getRecordedFramesPerSecond() const
{
	return m_animation.getRecordedFramesPerSecond();
}

// ----------------------------------------------------------------------

void MaskedPrioritySkeletalAnimation::setPlaybackFramesPerSecond(float playbackFramesPerSecond)
{
	m_animation.setPlaybackFramesPerSecond(playbackFramesPerSecond);
}

// ----------------------------------------------------------------------

float MaskedPrioritySkeletalAnimation::getPlaybackFramesPerSecond() const
{
	return m_animation.getPlaybackFramesPerSecond();
}

// ----------------------------------------------------------------------

int MaskedPrioritySkeletalAnimation::getMessageCount() const
{
	return m_animation.getMessageCount();
}

// ----------------------------------------------------------------------

const CrcLowerString &MaskedPrioritySkeletalAnimation::getMessageName(int index) const
{
	return m_animation.getMessageName(index);
}

// ----------------------------------------------------------------------

void MaskedPrioritySkeletalAnimation::getSignaledMessages(stdvector<int>::fwd &signaledMessageIndices, stdvector<float>::fwd &elapsedTimeSinceSignal) const
{
	m_animation.getSignaledMessages(signaledMessageIndices, elapsedTimeSinceSignal);
}

// ----------------------------------------------------------------------

SkeletalAnimation *MaskedPrioritySkeletalAnimation::resolveSkeletalAnimation()
{
	return &m_animation; //lint !e1536 // (Warning -- Exposing low access member 'MaskedPrioritySkeletalAnimation::m_animation') -- Effective C++ #30 // It's okay.
}

// ======================================================================

MaskedPrioritySkeletalAnimation::~MaskedPrioritySkeletalAnimation()
{
	//-- Delete resources.
	delete m_transformPriorities;

	//-- Release local references.
	m_animation.release();
}

// ======================================================================
