// ======================================================================
//
// CreatureController.h
// Portions Copyright 1999, Bootprint Entertainment, Inc.
// Portions Copyright 2001, 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_CreatureController_H
#define INCLUDED_CreatureController_H

// ======================================================================

#include "clientGame/ClientController.h"
#include "sharedMath/Vector.h"

class AnimationStateNameId;
class AnimationStatePath;
class AnimationNotification;
class CrcLowerString;
class CreatureObject;
class NetworkId;

template <class T>
class Watcher;

// ======================================================================

class CreatureController : public ClientController
{
public:

	static void install();

public:

	explicit      CreatureController             (CreatureObject* newOwner);
	virtual      ~CreatureController             ();

	void          requestServerPostureChange     (int8 posture);
	void          requestServerAttitudeChange    (int8 attitude);
	void          setHeldItemAnimationStateId    (const AnimationStateNameId *heldStateId);

	void          setLookAtTarget                (const NetworkId & id);
	void          setIntendedTarget              (const NetworkId & id);

	void          getAnimationTarget             (Object*& targetObject, bool& isCombatTarget) const;

	void          overrideAnimationTarget        (Object* targetObject, bool isCombatTarget, CrcLowerString const &hardpoint);
	void          changeAnimationStatePath       (int serverPosture, bool skipTraversal = true, bool skipWithDelay = true);

	void          setMood                        (uint32 mood);

	virtual void  pauseDeadReckoning (float pauseTime);
	virtual bool  shouldApplyAnimationDrivenLocomotion () const;
	virtual float getCurrentSpeed () const;
	virtual void  setCurrentSpeed (float currentSpeed);

	bool          doesBodyPlayOnceHaveLocomotionPriority (bool mustBeAtLeastLocomotionPriority = false) const;

	void          face (const Vector& position_w);
	void          playAnimationWithFaceTracking (const CrcLowerString &animationActionName, Object& targetObject, int &animationId, bool &animationIsAdd, AnimationNotification *notification);
	bool          isFaceTracking () const;
	void          stopFaceTracking ();

	bool          isSwimming() const;
	bool          areVisualsInCombat() const;
	void          forceVisualsIntoCombat();

	AnimationStatePath const *getCurrentAnimationStatePath() const;

	virtual float getDesiredSpeed () const;

	void          setControllerToCurrentState    (bool inCombat, bool forceState = false);

protected:

	virtual float realAlter                      (float time);
	virtual void  handleMessage                  (int message, float value, const MessageQueue::Data* data, uint32 flags);
	bool          canCreatureMove                () const;
	bool          appearanceIsSkeletal           () const;

	void          requestStand                   ();

private:

	static 	void  getFirstCombatTargetObject     (const CreatureObject& creature, Object*& targetObject, bool &hasCombatTarget);

private:

	void          initializeAnimationController  ();
	void          determineAnimationStatePath    (const CreatureObject &creature, AnimationStatePath &statePath, bool inCombat, int serverPostureOverride = -1) const;
	void          setAppearanceTarget            (const Object *target);
	void          applyAnimationDrivenLocomotion (float elapsedTime);
	void          reportYawStateToAppearance     (float deltaTime);
	void          handleFaceTracking             ();
	void          processCombatStateChange       (bool newInCombat);

	// disabled
	              CreatureController             ();
	              CreatureController             (const CreatureController&);
	CreatureController& operator=                (const CreatureController&);

	bool          calculateTerrainAndSwimmingInfo(bool &onTerrain, Vector &terrainNormal, bool &isSwimmingNow, float & terrainHeight, float & waterHeight, bool & isBurning) const;
	void          getAnimationTargetInternal     (Object*& targetObject, bool& isCombatTarget) const;

private:

	static AnimationStateNameId ms_combatStateId;
	static AnimationStateNameId ms_aimedStateId;
	static AnimationStateNameId ms_sittingOnChairStateId;

private:

	bool                        m_appearanceIsSkeletal;
	bool                        m_animationControllerInitialized;

	AnimationStatePath         *m_currentAnimationStatePath;
	AnimationStatePath         *m_previousAnimationStatePath;

	bool                        m_useHeldItemState;
	AnimationStateNameId       *m_heldItemStateId;

	Watcher<Object>            *m_overrideAnimationTargetWatcher;
	bool                        m_isOverrideTargetForCombat;

	bool                        m_suppressStandRequest;

	bool                        m_face;
	Vector                      m_facePosition_w;

	float                       m_previousTheta_w;
	int                         m_previousYawDirection;
	float                       m_continueTurningTimer;
	bool                        m_isFirstAlter;

	bool                        m_faceTracking;
	Watcher<Object>            *m_faceTrackingTarget;
	int                         m_faceTrackingAnimationId;
	bool                        m_faceTrackingAnimationIsAdd;

	bool                        m_isSwimming;
	bool                        m_inCombatState;
};

// ======================================================================

inline bool CreatureController::appearanceIsSkeletal () const
{
	return m_appearanceIsSkeletal;
}

// ----------------------------------------------------------------------

inline bool CreatureController::isFaceTracking () const
{
	return m_faceTracking;
}

// ======================================================================

#endif
