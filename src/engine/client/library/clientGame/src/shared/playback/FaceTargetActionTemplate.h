// ======================================================================
//
// FaceTargetActionTemplate.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_FaceTargetActionTemplate_H
#define INCLUDED_FaceTargetActionTemplate_H

// ======================================================================

#include "clientAnimation/PlaybackActionTemplate.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"

class Iff;

// ======================================================================

class FaceTargetActionTemplate: public PlaybackActionTemplate
{
public:

	static void install();

public:

	virtual PlaybackAction *createPlaybackAction() const;

	int  getRotationActorIndex() const;
	int  getTargetActorIndex() const;
	Tag  getMaxRotationalSpeedVariable() const;
	Tag  getOnTargetThresholdAngleVariable() const;
	Tag  getMaxTimeToLiveVariable() const;
	Tag  getIgnoreDeadReckoningDurationVariable() const;

	bool getLimitRotationToSkeletalAppearance() const;

private:

	static void                    remove();
	static PlaybackActionTemplate *create(Iff &iff);

private:

	FaceTargetActionTemplate(Iff &iff);

	void load_0000(Iff &iff);
	void load_0001(Iff &iff);

	// Disabled.
	FaceTargetActionTemplate();
	FaceTargetActionTemplate(const FaceTargetActionTemplate&);
	FaceTargetActionTemplate &operator =(const FaceTargetActionTemplate&);

private:

	static bool  ms_installed;

private:

	/// Actor index for the object to be yawed.
	int  m_rotationActorIndex;

	/// Actor index for the target.
	int  m_targetActorIndex;

	/// Indicates the maximum rotational speed, radians per second. [float variable]
	Tag  m_maxRotationalSpeedVariable;

	/// If the difference in facing is this angle (in radians) or less, the actor is considered on target and the action stops. [float variable]
	Tag  m_onTargetThresholdAngleVariable;

	/// Even if the on-target threshold has not been achieved, this action will stop after this amount of time (in seconds). [float variable]
	Tag  m_maxTimeToLiveVariable;

	/// The value of this variable this value represents the amount of time during which dead reckoning will be disabled on the rotating object. [float variable]
	Tag  m_ignoreDeadReckoningDurationVariable;
};

// ======================================================================

inline int FaceTargetActionTemplate::getRotationActorIndex() const
{
	return m_rotationActorIndex;
}

// ----------------------------------------------------------------------

inline int FaceTargetActionTemplate::getTargetActorIndex() const
{
	return m_targetActorIndex;
}

// ----------------------------------------------------------------------

inline Tag FaceTargetActionTemplate::getMaxRotationalSpeedVariable() const
{
	return m_maxRotationalSpeedVariable;
}

// ----------------------------------------------------------------------

inline Tag FaceTargetActionTemplate::getOnTargetThresholdAngleVariable() const
{
	return m_onTargetThresholdAngleVariable;
}

// ----------------------------------------------------------------------

inline Tag FaceTargetActionTemplate::getMaxTimeToLiveVariable() const
{
	return m_maxTimeToLiveVariable;
}

// ----------------------------------------------------------------------

inline Tag FaceTargetActionTemplate::getIgnoreDeadReckoningDurationVariable() const
{
	return m_ignoreDeadReckoningDurationVariable;
}

// ----------------------------------------------------------------------

inline bool FaceTargetActionTemplate::getLimitRotationToSkeletalAppearance() const
{
	return true;
}

// ======================================================================

#endif
