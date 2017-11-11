// ======================================================================
//
// ArcTargetAction.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientAnimation/PlaybackAction.h"

class ArcTargetPlaybackAction;
class CreatureController;
class Object;

template <class T>
class Watcher;

// ======================================================================

class ArcTargetAction: public PlaybackAction
{
friend class ArcTargetActionTemplate;

public:

	virtual bool update(float deltaTime, PlaybackScript &script);

private:

	explicit ArcTargetAction(const ArcTargetActionTemplate &actionTemplate);
	virtual ~ArcTargetAction();

	const ArcTargetActionTemplate *getOurTemplate() const;
	Object                        *getAttacker(PlaybackScript &script) const;
	Object                        *getDefender(PlaybackScript &script) const;
	CreatureController            *getAttackerController(PlaybackScript &script) const;
	float                          getArcTargetTimeToLive(const PlaybackScript &script) const;
	float                          getArcDistanceMultiplier(const PlaybackScript &script) const;
	float                          getArcHeightMultiplier(const PlaybackScript &script) const;

	Object                        *createArcTarget(PlaybackScript &script) const;

private:

	ArcTargetAction();
	ArcTargetAction(const ArcTargetAction&);
	ArcTargetAction &operator =(const ArcTargetAction&);

private:

	bool             m_arcTargetCreated;
	Watcher<Object> *m_arcTargetWatcher;

	bool             m_hasCreatureController;
};

// ======================================================================
