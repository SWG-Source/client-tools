// ======================================================================
//
// FireSetupActionTemplate.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_FireSetupActionTemplate_H
#define INCLUDED_FireSetupActionTemplate_H

// ======================================================================

#include "clientAnimation/PlaybackActionTemplate.h"

class CallbackAnimationNotification;
class Iff;

// ======================================================================

class FireSetupActionTemplate: public PlaybackActionTemplate
{
public:

	static void install();

public:

	virtual PlaybackAction *createPlaybackAction() const;
	virtual bool            update(float deltaTime, PlaybackScript &script) const;

private:

	typedef stdvector<int>::fwd  IntVector;

private:

	static void                    remove();
	static PlaybackActionTemplate *create(Iff &iff);


private:

	explicit FireSetupActionTemplate(Iff &iff);

	void     load_0002(Iff &iff);

	void     calculateDamagePerFire(int fireCount, int totalDamageAmount, IntVector &perFireDamageAmount) const;

	int      getAttackerActorIndex() const;
	int      getDefenderActorIndex() const;
	bool     getFireInfo(PlaybackScript &script, int &fireCount, IntVector &muzzleIndices, bool &messageNamesIncludeMuzzle, CallbackAnimationNotification **notification) const;
	int      getDefenderDamageAmount(const PlaybackScript &script) const;
	int      getDefenderDamageType(const PlaybackScript &script) const;
	int      getDefenderHitLocation(const PlaybackScript &script) const;

	void     addSuccessfulFire(PlaybackScript &script, bool messageNamesIncludeMuzzle, int fireNumber, int muzzleIndex, CallbackAnimationNotification *notification) const;
	void     addMissedFire(PlaybackScript &script, bool messageNamesIncludeMuzzle, int fireNumber, int muzzleIndex, bool playDefenderMissAction, CallbackAnimationNotification *notification) const;
	void     addNoFireActions(PlaybackScript &script) const;

	// Disabled.
	FireSetupActionTemplate();
	FireSetupActionTemplate(const FireSetupActionTemplate&);
	FireSetupActionTemplate &operator =(const FireSetupActionTemplate&);

private:

	static bool  ms_installed;

private:

	int    m_attackerActorIndex;
	int    m_defenderActorIndex;
	int    m_targetMode;
	int    m_animationThreadIndex;
	int    m_animationActionIndex;

};

// ======================================================================

#endif
