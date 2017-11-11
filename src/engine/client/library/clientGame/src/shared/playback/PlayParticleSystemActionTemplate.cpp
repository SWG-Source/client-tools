// ======================================================================
//
// PlayParticleSystemActionTemplate.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/PlayParticleSystemActionTemplate.h"

#include "clientAnimation/PlaybackScriptTemplate.h"
#include "clientGame/PlayParticleSystemAction.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"

// ======================================================================

namespace PlayParticleSystemActionTemplateNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	Tag const TAG_NONE = TAG(N,O,N,E);
	Tag const TAG_PPRT = TAG(P,P,R,T);
	Tag const TAG_PPRL = TAG(P,P,R,L);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool  s_installed;
}

using namespace PlayParticleSystemActionTemplateNamespace;

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL(PlayParticleSystemActionTemplate, true, 0, 0, 0);

// ======================================================================
// class PlayParticleSystemActionTemplate: PUBLIC STATIC
// ======================================================================

void PlayParticleSystemActionTemplate::install()
{
	InstallTimer const installTimer("PlayParticleSystemActionTemplate::install");

	DEBUG_FATAL(s_installed, ("PlayParticleSystemActionTemplate already installed."));

	installMemoryBlockManager();

	PlaybackScriptTemplate::registerActionTemplate(TAG_PPRT, create);
	PlaybackScriptTemplate::registerActionTemplate(TAG_PPRL, createLocation);

	s_installed = true;
	ExitChain::add(remove, "PlayParticleSystemActionTemplate");
}

// ======================================================================
// class PlayParticleSystemActionTemplate: PUBLIC
// ======================================================================

PlaybackAction *PlayParticleSystemActionTemplate::createPlaybackAction() const
{
	return new PlayParticleSystemAction(*this);
}

// ----------------------------------------------------------------------

int PlayParticleSystemActionTemplate::getEmittingActorIndex() const
{
	return m_emittingActorIndex;
}

// ----------------------------------------------------------------------

Tag PlayParticleSystemActionTemplate::getHardpointNameTag() const
{
	return m_hardpointNameTag;
}

// ----------------------------------------------------------------------

Tag PlayParticleSystemActionTemplate::getAppearanceNameTag() const
{
	return m_appearanceNameTag;
}

// ----------------------------------------------------------------------

Tag PlayParticleSystemActionTemplate::getTimeDurationTag() const
{
	return m_timeDurationTag;
}

// ----------------------------------------------------------------------

Tag PlayParticleSystemActionTemplate::getLoopSoundNameTag() const
{
	return m_loopSoundNameTag;
}

// ----------------------------------------------------------------------

Tag PlayParticleSystemActionTemplate::getStartSoundNameTag() const
{
	return m_startSoundNameTag;
}

// ----------------------------------------------------------------------

Tag PlayParticleSystemActionTemplate::getStopSoundNameTag() const
{
	return m_stopSoundNameTag;
}

// ======================================================================
// class PlayParticleSystemActionTemplate: PRIVATE STATIC
// ======================================================================

void PlayParticleSystemActionTemplate::remove()
{
	DEBUG_FATAL(!s_installed, ("PlayParticleSystemActionTemplate not installed."));
	s_installed = false;

	PlaybackScriptTemplate::deregisterActionTemplate(TAG_PPRT);

	removeMemoryBlockManager();
}

// ----------------------------------------------------------------------

PlaybackActionTemplate *PlayParticleSystemActionTemplate::create(Iff &iff)
{
	return new PlayParticleSystemActionTemplate(iff, false);
}

// ----------------------------------------------------------------------

PlaybackActionTemplate *PlayParticleSystemActionTemplate::createLocation(Iff &iff)
{
	return new PlayParticleSystemActionTemplate(iff, true);
}

// ======================================================================
// class PlayParticleSystemActionTemplate: PRIVATE
// ======================================================================

PlayParticleSystemActionTemplate::PlayParticleSystemActionTemplate(Iff &iff, bool useLocation):
	PlaybackActionTemplate(),
	m_emittingActorIndex(-1),
	m_hardpointNameTag(TAG_NONE),
	m_appearanceNameTag(TAG_NONE),
	m_timeDurationTag(TAG_NONE),
	m_loopSoundNameTag(TAG_NONE),
	m_startSoundNameTag(TAG_NONE),
	m_stopSoundNameTag(TAG_NONE),
	m_useLocation(useLocation)
{
		iff.enterForm(useLocation ? TAG_PPRL : TAG_PPRT);

		Tag const version = iff.getCurrentName();
		switch (version)
		{
			case TAG_0000:
				load_0000(iff);
				break;

			default:
				{
					char buffer[5];
					ConvertTagToString(version, buffer);
					FATAL(true, ("PlayParticleSystemActionTemplate: unsupported version format [%s].", buffer));
				}
		}

	iff.exitForm(useLocation ? TAG_PPRL : TAG_PPRT);
}

// ----------------------------------------------------------------------

void PlayParticleSystemActionTemplate::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);
		iff.enterChunk(TAG_INFO);

			m_emittingActorIndex = static_cast<int>(iff.read_int16());

			if(!m_useLocation)
				m_hardpointNameTag   = static_cast<Tag>(iff.read_uint32());
			m_appearanceNameTag  = static_cast<Tag>(iff.read_uint32());
			m_timeDurationTag    = static_cast<Tag>(iff.read_uint32());

			m_loopSoundNameTag   = static_cast<Tag>(iff.read_uint32());
			m_startSoundNameTag  = static_cast<Tag>(iff.read_uint32());
			m_stopSoundNameTag   = static_cast<Tag>(iff.read_uint32());

		iff.exitChunk(TAG_INFO);
	iff.exitForm(TAG_0000);
}

// ----------------------------------------------------------------------

bool PlayParticleSystemActionTemplate::getUsesLocation() const
{
	return m_useLocation;
}

// ======================================================================
