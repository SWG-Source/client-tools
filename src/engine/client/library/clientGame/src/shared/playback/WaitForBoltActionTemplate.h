// ======================================================================
//
// WaitForBoltActionTemplate.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_WaitForBoltActionTemplate_H
#define INCLUDED_WaitForBoltActionTemplate_H

// ======================================================================

#include "clientAnimation/PlaybackActionTemplate.h"

class CallbackAnimationNotification;
class Iff;

// ======================================================================

class WaitForBoltActionTemplate: public PlaybackActionTemplate
{
public:

	static void install();

public:

	virtual PlaybackAction *createPlaybackAction() const;
	virtual bool            update(float deltaTime, PlaybackScript &script) const;

	int      getAttackerActorIndex() const;
	int      getDefenderActorIndex() const;
	bool     getSubtractInitialWait() const;

	WaitForBoltActionTemplate(int attackerActorIndex, int defenderActorIndex, bool subtractInitialWait = false);

private:

	typedef stdvector<int>::fwd  IntVector;

private:

	static void                    remove();
	static PlaybackActionTemplate *create(Iff &iff);


private:

	explicit WaitForBoltActionTemplate(Iff &iff);

	void     load_0002(Iff &iff);

	
	// Disabled.
	WaitForBoltActionTemplate();
	WaitForBoltActionTemplate(const WaitForBoltActionTemplate&);
	WaitForBoltActionTemplate &operator =(const WaitForBoltActionTemplate&);

private:

	static bool  ms_installed;

private:

	int    m_attackerActorIndex;
	int    m_defenderActorIndex;
	bool   m_subtractInitialWait;

};

// ======================================================================

#endif
