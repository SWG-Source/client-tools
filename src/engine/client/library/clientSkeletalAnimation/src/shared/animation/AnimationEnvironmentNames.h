// ======================================================================
//
// AnimationEnvironmentNames.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_AnimationEnvironmentNames_H
#define INCLUDED_AnimationEnvironmentNames_H

// ======================================================================

class ConstCharCrcLowerString;

// ======================================================================
/**
 * Defines common names for AnimationEnivornment instances.
 */

class AnimationEnvironmentNames
{
public:

	/// Appearance locomotion velocity in Object space.
	static const ConstCharCrcLowerString  cms_locomotionVelocity;

	/// Direction to this Appearance's target in Object space.
	static const ConstCharCrcLowerString  cms_directionToTarget;

	/// Uniform scale of the appearance. (float)
	static const ConstCharCrcLowerString  cms_appearanceScale;

	/// Appearance yaw (around +y axis) direction: -1 = counter clockwise, 0 = no yaw, +1 = clockwise.
	static const ConstCharCrcLowerString  cms_yawDirection;

	/// Animating skill name.
	static const ConstCharCrcLowerString  cms_animatingSkill;

	/// Gender of character (m=male, f=female, o=other).
	static const ConstCharCrcLowerString  cms_gender;

	/// Rider pose: used on the rider appearance for mount posture.
	static const ConstCharCrcLowerString  cms_riderPose;

	/// Mounted creature: used on the mount appearance, set to 0 when not mounted, set to 1 when mounted.
	static const ConstCharCrcLowerString  cms_mountedCreature;
};

// ======================================================================

#endif
