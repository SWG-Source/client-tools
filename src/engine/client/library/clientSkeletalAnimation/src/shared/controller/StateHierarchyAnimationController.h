// ======================================================================
//
// StateHierarchyAnimationController.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_StateHierarchyAnimationController_H
#define INCLUDED_StateHierarchyAnimationController_H

// ======================================================================

#include "clientSkeletalAnimation/TransformAnimationController.h"

class AnimationEnvironment;
class AnimationNotification;
class AnimationStatePath;
class AnimationStateHierarchyTemplate;
class AnimationTrackId;
class ConstCharCrcLowerString;
class CrcLowerString;
class CrcString;
class LogicalAnimationTableTemplate;
class SkeletalAnimation;
class TrackAnimationController;

// ======================================================================

class StateHierarchyAnimationController: public TransformAnimationController
{
public:

	static void install();

public:

	StateHierarchyAnimationController(AnimationEnvironment &animationEnvironment, const TransformNameMap &transformNameMap, const AnimationStateHierarchyTemplate *hierarchyTemplate, const LogicalAnimationTableTemplate *latTemplate, int channel, AnimationStatePath const &initialPath);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// Inherited interface.
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	virtual ~StateHierarchyAnimationController();

	virtual void                           alter(real time);

	virtual void                           evaluateTransformComponents(int localTransformIndex, Quaternion &rotation, Vector &translation);
	virtual void                           getObjectLocomotion(Quaternion &rotation, Vector &translation) const;

	virtual int                            addAnimationMessageListener(AnimationMessageCallback callback, void *context);
	virtual void                           removeAnimationMessageListener(int id);

	virtual StateHierarchyAnimationController       *asStateHierarchyAnimationController();
	virtual StateHierarchyAnimationController const *asStateHierarchyAnimationController() const;
	virtual TrackAnimationController                *asTrackAnimationController();

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// Animation controller modification.
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	void                      setDestinationState(const AnimationStatePath &destinationState, bool skipTraversal = false, bool skipWithDelay = true);
	const AnimationStatePath &getDestinationPath() const;

	void                      playAction(const CrcLowerString &actionName, const AnimationEnvironment &animationEnvironment, int &animationId, bool &animationIsAdd, AnimationNotification *notification);
	void                      stopAction(int animationId, bool animationIsAdd);
	bool                      hasActionCompleted(int animationId, bool animationIsAdd) const;

	bool                      isPlayOnceTrackActive() const;
	bool                      doesPlayOnceTrackHaveLocomotionPriority() const;

	int                       getLoopTrackLocomotionPriority() const;
	int                       getPlayOnceTrackLocomotionPriority() const;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// Tool/special interface (i.e. expensive or unusual operations not for use in general game code).
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	TrackAnimationController              &getTrackAnimationController();

	SkeletalAnimation                     *fetchAnimationForAction(const CrcLowerString &actionName, const AnimationEnvironment &animationEnvironment);
	
	const AnimationStateHierarchyTemplate *fetchHierarchyTemplate() const;
	const LogicalAnimationTableTemplate   *fetchLogicalAnimationTableTemplate() const;

private:

	static void remove();

private:

	void               playAnimation(const AnimationTrackId &trackId, CrcString const &logicalAnimationName, bool playImmediately, bool doBlend, bool playLooping, bool fallbackToDefault, int &animationId, AnimationNotification *notification);
	bool               lookupAnimationNameForAction(const CrcLowerString &actionName, const AnimationEnvironment &animationEnvironment, const CrcLowerString *&logicalAnimationName, bool &playOnAddTrack) const;
	SkeletalAnimation *fetchAnimation(CrcString const &logicalAnimationName);

	void               requestDeferredLoopAnimation(CrcString const &logicalAnimationName, float waitTime);

	// disabled
	StateHierarchyAnimationController(const StateHierarchyAnimationController&);
	StateHierarchyAnimationController &operator =(const StateHierarchyAnimationController&);

private:

	static const ConstCharCrcLowerString  cms_defaultLogicalAnimationName;
	static const ConstCharCrcLowerString  cms_actionTrackName;
	static const ConstCharCrcLowerString  cms_addTrackName;
	static const ConstCharCrcLowerString  cms_locomotionTrackName;

	static bool                  ms_installed;

	static AnimationTrackId      ms_locomotionTrackId;
	static AnimationTrackId      ms_actionTrackId;
	static AnimationTrackId      ms_addTrackId;

private:

	const AnimationStateHierarchyTemplate       *m_hierarchyTemplate;
	const LogicalAnimationTableTemplate         *m_latTemplate;
	AnimationStatePath                    *const m_currentState;

	TrackAnimationController              *const m_trackAnimationController;

	CrcString const                             *m_deferredLoopAnimationName;
	float                                        m_deferredPlayTimeRemaining;
};

// ======================================================================

inline TrackAnimationController &StateHierarchyAnimationController::getTrackAnimationController()
{
	NOT_NULL(m_trackAnimationController);
	return *m_trackAnimationController;
}

// ======================================================================

#endif
