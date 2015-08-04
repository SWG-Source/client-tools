// ======================================================================
//
// FireProjectileActionTemplate.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_FireProjectileActionTemplate_H
#define INCLUDED_FireProjectileActionTemplate_H

// ======================================================================

#include "clientAnimation/PlaybackActionTemplate.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"

#include "sharedMath/Vector.h"

class CallbackAnimationNotification;
class CellObject;
class CellProperty;
class CrcLowerString;
class ConstCharCrcLowerString;
class DataTable;
class Iff;
class Object;
class Projectile;
class Skeleton;
class Vector;

// ======================================================================
/**
 * Support a PlaybackScript action for firing a projectile and
 * playing its associated particle effects.
 */

class FireProjectileActionTemplate: public PlaybackActionTemplate
{
public:

	enum TargetMode
	{
		/// In this mode, a hit is a hit, a miss is a miss, and we're always using the target actor for our target.  We use a tracking projectile for hits, a non-tracking for misses.  Used for normal combat.
		TM_normal = 0,

		/// In this mode, the target is the animation target, not the defender actor.  The projectile is always a non-tracking projectile.  Used for combat with special targets.
		TM_animationTargetNoTrack = 1,

		/// In this mode, the target is the animation target ,not the defender actor.  The projecitle is always tracking and a ricochet effect is played when the projectile hits the target.
		TM_animationTargetTrackRicochet = 2,

		/// In this mode, the target is the animation target ,not the defender actor.  The projecitle is always tracking and a reflect effect is played when the projectile hits the target.
		TM_animationTargetTrackReflect = 3,

		/// The difference between this and the above one is that the reflected particle will hit the jedi's combat target, not the attacker
		TM_animationTargetTrackReflectTarget = 4,

		// In this target mode, target actor index is not valid.  We're showing at m_locationTarget, a world location.
		TM_location = 5


	};

public:

	static void install();

public:

	FireProjectileActionTemplate(
		bool hitSuccessful,
		int  emittingActorIndex,
		int  muzzleIndex,
		Tag  projectileFilenameVariable,
		Tag  projectileVelocityVariable,
		int  targetActorIndex,
		int  watcherActionIndex,
		int  watcherActionThread,
		TargetMode  targetMode,
		bool useEmitterTargetAsTargetActor = false,
		int  targetHitLocation = 0);

	virtual PlaybackAction *createPlaybackAction() const;
	virtual PlaybackAction *createPlaybackAction(CallbackAnimationNotification *notification, char const *animationMessageName) const;
	virtual bool            update(float deltaTime, PlaybackScript &script) const;

private:

	static void                    remove();
	static PlaybackActionTemplate *create(Iff &iff);
	static PlaybackActionTemplate *createLocation(Iff &iff);

private:

	explicit FireProjectileActionTemplate(Iff &iff, bool isLocationBased = false);
	
	void                           load_0000(Iff &iff);
	void                           load_location_0000(Iff &iff);

	bool                           getProjectileStartPosition(PlaybackScript &script, const CellProperty *&startCellProperty, Vector &startPosition_w, CrcLowerString &emitterHardpoint) const;
	bool                           getProjectileEndPosition(PlaybackScript &script, bool pickExactHitLocation, const Vector &startPosition_w, Vector &endPosition_w) const;
	void                           setProjectileTransform(Projectile &projectile, const CellProperty *startCellProperty, const Vector &startPosition_w, const Vector &endPosition_w) const;
	void                           setProjectileVelocityAndLifetime(const PlaybackScript &script, Projectile &projectile, const Vector &startPosition_w, const Vector &endPosition_w) const;
	Projectile                     *createProjectile(PlaybackScript & script, CrcLowerString const &emitterHardpoint) const;

	bool                           getHardpointPositionByName(const Object &object, const CrcLowerString &hardpointName, Vector &position_w) const;
	bool                           getSkeleton(const Object &object, const Skeleton *&skeleton) const;
	bool                           getSkeletonTransformPositionByIndex(const Object &object, const Skeleton &skeleton, int transformIndex, Vector &position_w) const;
	bool                           getEndPosition(const Object &object, bool pickExactHitLocation, const Vector &startPosition_w, Vector &endPosition_w) const;

	void                           setWatcherAction(PlaybackScript &script, Projectile &projectile) const;

	void                           createFireClientEvent(PlaybackScript &script) const;

	Object                        *getEmitterObject(PlaybackScript &script) const;
	Object                        *getTargetObject(PlaybackScript &script) const;
	Object                        *getAttackerWeaponObject(const PlaybackScript &script) const;

	Vector                         getTargetLocation_w(const PlaybackScript &script) const;
	const CellObject              *getTargetCell(const PlaybackScript &script) const;

	// disabled
	FireProjectileActionTemplate();
	FireProjectileActionTemplate(const FireProjectileActionTemplate&);
	FireProjectileActionTemplate &operator =(const FireProjectileActionTemplate&);

private:

	static const float                    cms_maximumProjectileDistance;
	static const char *const              cms_weaponHoldingSlotName;
	static const ConstCharCrcLowerString  cms_fireClientEventName;
	static const ConstCharCrcLowerString  cms_emissionHardpointNameArray[];
	static const int                      cms_emissionHardpointNameArrayLength;

private:

	int   m_emittingActorIndex;
	Tag   m_hardpointNameVariable;

	Tag   m_projectileFilenameVariable;
	Tag   m_projectileVelocityVariable;  

	int   m_targetActorIndex;

	bool  m_hitSuccessful;

	int   m_watcherActionIndex;
	int   m_watcherActionThread;

	int   m_muzzleIndex;

	TargetMode  m_targetMode;

	bool  m_useEmitterTargetAsTargetActor;

	int   m_targetHitLocation;
};

// ======================================================================

#endif
