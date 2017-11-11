// ======================================================================
//
// HomingTargetAction.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientAnimation/PlaybackAction.h"

class HomingTargetPlaybackAction;
class CreatureController;
class Object;

template <class T>
class Watcher;

// ======================================================================

class HomingTargetAction: public PlaybackAction
{
friend class HomingTargetActionTemplate;

public:

	virtual bool update(float deltaTime, PlaybackScript &script);

private:

	explicit HomingTargetAction(const HomingTargetActionTemplate &actionTemplate);
	virtual ~HomingTargetAction();

	const HomingTargetActionTemplate *getOurTemplate() const;
	Object                           *getAttacker(PlaybackScript &script) const;
	Object                           *getDefender(PlaybackScript &script) const;
	CreatureController               *getAttackerController(PlaybackScript &script) const;
	float                             getTravelTime(const PlaybackScript &script) const;
	float                             getStartDistanceFraction(const PlaybackScript &script) const;
	float                             getEndDistanceFraction(const PlaybackScript &script) const;

	float                             calculateDeltaTargetFraction(const PlaybackScript &script) const;

	Object                           *createTarget(const PlaybackScript &script) const;

	void                              updateTargetPosition(PlaybackScript &script) const;

private:

	HomingTargetAction();
	HomingTargetAction(const HomingTargetAction&);
	HomingTargetAction &operator =(const HomingTargetAction&);

private:

	bool             m_targetCreated;
	Watcher<Object> *m_targetWatcher;
	bool             m_hasCreatureController;
	float            m_targetFraction;
	float            m_deltaTargetFractionPerTime;
};

// ======================================================================
