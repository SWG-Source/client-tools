// ======================================================================
//
// FaceTargetAction.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_FaceTargetAction_H
#define INCLUDED_FaceTargetAction_H

// ======================================================================

#include "clientAnimation/PlaybackAction.h"

class FaceTargetActionTemplate;

// ======================================================================

class FaceTargetAction: public PlaybackAction
{
public:

	FaceTargetAction(const FaceTargetActionTemplate &actionTemplate);

	virtual bool update(float deltaTime, PlaybackScript &script);

private:

	const FaceTargetActionTemplate &getOurTemplate() const;

	// Disabled.
	FaceTargetAction();
	FaceTargetAction(const FaceTargetAction&);
	FaceTargetAction &operator =(const FaceTargetAction&);

private:

	bool   m_initializedPlaybackData;

	float  m_maxRotationalSpeed;
	float  m_onTargetThresholdAngle;
	float  m_timeToLive;

};

// ======================================================================

#endif
