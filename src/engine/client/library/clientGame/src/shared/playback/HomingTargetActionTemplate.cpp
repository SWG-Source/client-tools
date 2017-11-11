// ======================================================================
//
// HomingTargetActionTemplate.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/HomingTargetActionTemplate.h"

#include "clientAnimation/PlaybackScriptTemplate.h"
#include "clientGame/HomingTargetAction.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"

// ======================================================================

bool HomingTargetActionTemplate::ms_installed;

// ======================================================================

const Tag TAG_HTAT = TAG(H,T,A,T);
const Tag TAG_NONE = TAG(N,O,N,E);

// ======================================================================
// class HomingTargetActionTemplate: public static member functions
// ======================================================================

void HomingTargetActionTemplate::install()
{
	InstallTimer const installTimer("HomingTargetActionTemplate::install");

	DEBUG_FATAL(ms_installed, ("HomingTargetActionTemplate already installed."));

	PlaybackScriptTemplate::registerActionTemplate(TAG_HTAT, create);

	ms_installed = true;
	ExitChain::add(remove, "HomingTargetActionTemplate");
}

// ======================================================================
// class HomingTargetActionTemplate: public member functions
// ======================================================================

PlaybackAction *HomingTargetActionTemplate::createPlaybackAction() const
{
	return new HomingTargetAction(*this);
}

// ======================================================================
// class HomingTargetActionTemplate: private static member functions
// ======================================================================

void HomingTargetActionTemplate::remove()
{
	DEBUG_FATAL(!ms_installed, ("HomingTargetActionTemplate not installed."));
	ms_installed = false;

	PlaybackScriptTemplate::deregisterActionTemplate(TAG_HTAT);
}

// ----------------------------------------------------------------------

PlaybackActionTemplate *HomingTargetActionTemplate::create(Iff &iff)
{
	return new HomingTargetActionTemplate(iff);
}

// ======================================================================
// class HomingTargetActionTemplate: private member functions
// ======================================================================

HomingTargetActionTemplate::HomingTargetActionTemplate(Iff &iff) :
	PlaybackActionTemplate(),
	m_travelTimeVariable(TAG_NONE),
	m_startDistanceFractionVariable(TAG_NONE),
	m_endDistanceFractionVariable(TAG_NONE),
	m_attackerActorIndex(0),
	m_defenderActorIndex(1)
{
	DEBUG_FATAL(!ms_installed, ("HomingTargetActionTemplate not installed."));

	iff.enterForm(TAG_HTAT);

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
					WARNING_STRICT_FATAL(true, ("unsupported HomingTargetActionTemplate version [%s].", buffer));

					iff.exitForm(true);
					return;
				}
		}

	iff.exitForm(TAG_HTAT);
}

// ----------------------------------------------------------------------

void HomingTargetActionTemplate::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);

		iff.enterChunk(TAG_INFO);

			m_travelTimeVariable            = static_cast<Tag>(iff.read_uint32());
			m_startDistanceFractionVariable = static_cast<Tag>(iff.read_uint32());
			m_endDistanceFractionVariable   = static_cast<Tag>(iff.read_uint32());
			m_attackerActorIndex            = static_cast<int>(iff.read_int16());
			m_defenderActorIndex            = static_cast<int>(iff.read_int16());

		iff.exitChunk(TAG_INFO);

	iff.exitForm(TAG_0000);
}

// ======================================================================
