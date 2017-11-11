// ======================================================================
//
// PlayParticleSystemActionTemplate.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_PlayParticleSystemActionTemplate_H
#define INCLUDED_PlayParticleSystemActionTemplate_H

// ======================================================================

#include "clientAnimation/PlaybackActionTemplate.h"
#include "sharedFile/Iff.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"

// ======================================================================

class PlayParticleSystemActionTemplate: public PlaybackActionTemplate
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

public:

	static void install();

public:

	virtual PlaybackAction *createPlaybackAction() const;

	int                     getEmittingActorIndex() const;
	Tag                     getHardpointNameTag() const;
	Tag                     getAppearanceNameTag() const;
	Tag                     getTimeDurationTag() const;

	Tag                     getLoopSoundNameTag() const;
	Tag                     getStartSoundNameTag() const;
	Tag                     getStopSoundNameTag() const;

	bool                    getUsesLocation() const;

private:

	static void                    remove();
	static PlaybackActionTemplate *create(Iff &iff);
	static PlaybackActionTemplate *createLocation(Iff &iff);

private:

	explicit PlayParticleSystemActionTemplate(Iff &iff, bool useLocation = false);
	void     load_0000(Iff &iff);

	// Disabled.
	PlayParticleSystemActionTemplate();
	PlayParticleSystemActionTemplate(PlayParticleSystemActionTemplate const&);
	PlayParticleSystemActionTemplate &operator =(PlayParticleSystemActionTemplate const&);

private:

	int  m_emittingActorIndex;
	Tag  m_hardpointNameTag;
	Tag  m_appearanceNameTag;
	Tag  m_timeDurationTag;
	Tag  m_loopSoundNameTag;
	Tag  m_startSoundNameTag;
	Tag  m_stopSoundNameTag;

	bool m_useLocation;

};

// ======================================================================

#endif
