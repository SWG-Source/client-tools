// ======================================================================
//
// TrackAnimationController_PhysicalTrackTemplate.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_TrackAnimationController_PhysicalTrackTemplate_H
#define INCLUDED_TrackAnimationController_PhysicalTrackTemplate_H

// ======================================================================

#include "clientSkeletalAnimation/TrackAnimationController_TrackTemplate.h"
#include <vector>

// ======================================================================

class TrackAnimationController::PhysicalTrackTemplate: public TrackAnimationController::TrackTemplate
{
public:

	typedef stdvector<StateChangeAction*>::fwd  StateChangeActionVector;

public:

	PhysicalTrackTemplate(const CrcLowerString &trackName, const AnimationTrackId &trackId, int physicalTrackIndex, AnimationFactory *animationFactory, bool allowSameTrackTrumping);
	virtual ~PhysicalTrackTemplate();

	void                           addPostEnterIdleAction(StateChangeAction *stateChangeAction);
	void                           addPreExitIdleAction(StateChangeAction *stateChangeAction);

	const StateChangeActionVector &getPostEnterIdleActions() const;
	const StateChangeActionVector &getPreExitIdleActions() const;
	bool                           getAllowSameTrackTrumping() const;

	SkeletalAnimation             *fetchInitialAnimation(const TransformNameMap &transformNameMap, TrackAnimationController &animationController) const;

	virtual bool                   hasTrackIndexResolver() const;
	virtual int                    getTrackIndexResolverIndex() const;
	virtual int                    getPhysicalTrackIndex() const;

private:

	// disabled
	PhysicalTrackTemplate();

	//lint -esym(754, PhysicalTrackTemplate::PhysicalTrackTemplate)  // unreferenced // defensive hiding
	//lint -esym(1714, PhysicalTrackTemplate::PhysicalTrackTemplate) // unreferenced
	PhysicalTrackTemplate(const PhysicalTrackTemplate&);             

	//lint -esym(754, PhysicalTrackTemplate::operator=)  // unreferenced // defensive hiding
	//lint -esym(1714, PhysicalTrackTemplate::operator=) // unreferenced
	PhysicalTrackTemplate &operator =(const PhysicalTrackTemplate&); 

private:

	int                              m_physicalTrackIndex;
	StateChangeActionVector          m_postEnterIdleActions;
	StateChangeActionVector          m_preExitIdleActions;

	AnimationFactory                *m_animationFactory;
	bool                             m_allowSameTrackTrumping;

};

// ======================================================================

inline void TrackAnimationController::PhysicalTrackTemplate::addPostEnterIdleAction(StateChangeAction *stateChangeAction)
{
	m_postEnterIdleActions.push_back(NON_NULL(stateChangeAction));
}

// ----------------------------------------------------------------------

inline void TrackAnimationController::PhysicalTrackTemplate::addPreExitIdleAction(StateChangeAction *stateChangeAction)
{
	m_preExitIdleActions.push_back(NON_NULL(stateChangeAction));
}

// ----------------------------------------------------------------------

inline const TrackAnimationController::PhysicalTrackTemplate::StateChangeActionVector &TrackAnimationController::PhysicalTrackTemplate::getPostEnterIdleActions() const
{
	return m_postEnterIdleActions;
}

// ----------------------------------------------------------------------

inline const TrackAnimationController::PhysicalTrackTemplate::StateChangeActionVector &TrackAnimationController::PhysicalTrackTemplate::getPreExitIdleActions() const
{
	return m_preExitIdleActions;
}

// ----------------------------------------------------------------------

inline bool TrackAnimationController::PhysicalTrackTemplate::getAllowSameTrackTrumping() const
{
	return m_allowSameTrackTrumping;
}

// ======================================================================

#endif
