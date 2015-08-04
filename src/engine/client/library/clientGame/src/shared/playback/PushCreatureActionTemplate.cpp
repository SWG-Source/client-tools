// ======================================================================
//
// PushCreatureActionTemplate.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/PushCreatureActionTemplate.h"

#include "clientAnimation/PlaybackScriptTemplate.h"
#include "clientGame/PushCreatureAction.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"

// ======================================================================

const Tag TAG_PUSH = TAG(P,U,S,H);
const Tag TAG_IDRD = TAG(I,D,R,D);
const Tag TAG_NONE = TAG(N,O,N,E);

// ======================================================================

bool PushCreatureActionTemplate::ms_installed;

// ======================================================================
// class PushCreatureActionTemplate: public static member functions
// ======================================================================

void PushCreatureActionTemplate::install()
{
	DEBUG_FATAL(ms_installed, ("PushCreatureActionTemplate already installed."));

	PlaybackScriptTemplate::registerActionTemplate(TAG_PUSH, create);

	ms_installed = true;
	ExitChain::add(remove, "PushCreatureActionTemplate");
}

// ======================================================================
// class PushCreatureActionTemplate: public member functions
// ======================================================================

PlaybackAction *PushCreatureActionTemplate::createPlaybackAction() const
{
	return new PushCreatureAction(*this);
}

// ======================================================================
// class PushCreatureActionTemplate: private static member functions
// ======================================================================

void PushCreatureActionTemplate::remove()
{
	DEBUG_FATAL(!ms_installed, ("PushCreatureActionTemplate not installed."));
	ms_installed = false;

	PlaybackScriptTemplate::deregisterActionTemplate(TAG_PUSH);
}

// ----------------------------------------------------------------------

PlaybackActionTemplate *PushCreatureActionTemplate::create(Iff &iff)
{
	return new PushCreatureActionTemplate(iff);
}

// ======================================================================
// class PushCreatureActionTemplate: private member functions
// ======================================================================

PushCreatureActionTemplate::PushCreatureActionTemplate(Iff &iff) :
	PlaybackActionTemplate(),
	m_defenderActorIndex(-1),
	m_attackerActorIndex(-1),
	m_distanceVariable(TAG_NONE),
	m_timeVariable(TAG_NONE)
{
	DEBUG_FATAL(!ms_installed, ("PushCreatureActionTemplate not installed."));

	iff.enterForm(TAG_PUSH);

		const Tag version = iff.getCurrentName();
		switch (version)
		{
			case TAG_0000:
				load_0000(iff);
				break;

			default:
				{
					char buffer[5];

					ConvertTagToString(version, buffer);
					DEBUG_FATAL(true, ("unsupported PushCreatureActionTemplate version [%s].", buffer));
				}
		}

	iff.exitForm(TAG_PUSH);
}

// ----------------------------------------------------------------------

void PushCreatureActionTemplate::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);

		iff.enterChunk(TAG_INFO);

			m_defenderActorIndex = static_cast<int>(iff.read_int16());
			m_attackerActorIndex   = static_cast<int>(iff.read_int16());

			m_distanceVariable     = static_cast<Tag>(iff.read_uint32());
			m_timeVariable = static_cast<Tag>(iff.read_uint32());

		iff.exitChunk(TAG_INFO);

	iff.exitForm(TAG_0000);
}

// ======================================================================
