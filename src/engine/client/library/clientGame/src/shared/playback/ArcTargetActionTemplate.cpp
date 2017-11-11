// ======================================================================
//
// ArcTargetActionTemplate.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ArcTargetActionTemplate.h"

#include "clientAnimation/PlaybackScriptTemplate.h"
#include "clientGame/ArcTargetAction.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"

// ======================================================================

bool ArcTargetActionTemplate::ms_installed;

// ======================================================================

const Tag TAG_ATAT = TAG(A,T,A,T);
const Tag TAG_ADML = TAG(A,D,M,L);
const Tag TAG_AHML = TAG(A,H,M,L);
const Tag TAG_ATTL = TAG(A,T,T,L);

// ======================================================================
// class ArcTargetActionTemplate: public static member functions
// ======================================================================

void ArcTargetActionTemplate::install()
{
	InstallTimer const installTimer("ArcTargetActionTemplate::install");

	DEBUG_FATAL(ms_installed, ("ArcTargetActionTemplate already installed."));

	PlaybackScriptTemplate::registerActionTemplate(TAG_ATAT, create);

	ms_installed = true;
	ExitChain::add(remove, "ArcTargetActionTemplate");
}

// ======================================================================
// class ArcTargetActionTemplate: public member functions
// ======================================================================

PlaybackAction *ArcTargetActionTemplate::createPlaybackAction() const
{
	return new ArcTargetAction(*this);
}


// ======================================================================
// class ArcTargetActionTemplate: private static member functions
// ======================================================================

void ArcTargetActionTemplate::remove()
{
	DEBUG_FATAL(!ms_installed, ("ArcTargetActionTemplate not installed."));
	ms_installed = false;

	PlaybackScriptTemplate::deregisterActionTemplate(TAG_ATAT);
}

// ----------------------------------------------------------------------

PlaybackActionTemplate *ArcTargetActionTemplate::create(Iff &iff)
{
	return new ArcTargetActionTemplate(iff);
}

// ======================================================================
// class ArcTargetActionTemplate: private member functions
// ======================================================================

ArcTargetActionTemplate::ArcTargetActionTemplate(Iff &iff) :
	PlaybackActionTemplate(),
	m_arcTimeVariable(TAG_ATTL),
	m_distanceMultipleVariable(TAG_ADML),
	m_heightMultipleVariable(TAG_AHML),
	m_attackerActorIndex(-1),
	m_defenderActorIndex(-1)
{
	DEBUG_FATAL(!ms_installed, ("ArcTargetActionTemplate not installed."));

	iff.enterForm(TAG_ATAT);

		const Tag version = iff.getCurrentName();
		switch (version)
		{
			case TAG_0000:
				load_0000(iff);
				break;

			case TAG_0001:
				load_0001(iff);
				break;

			default:
				{
					char buffer[5];

					ConvertTagToString(version, buffer);
					DEBUG_FATAL(true, ("unsupported ArcTargetActionTemplate version [%s].", buffer));
				}
		}

	iff.exitForm(TAG_ATAT);
}

// ----------------------------------------------------------------------

void ArcTargetActionTemplate::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);

		iff.enterChunk(TAG_INFO);

			m_arcTimeVariable          = static_cast<Tag>(iff.read_uint32());
			m_distanceMultipleVariable = static_cast<Tag>(iff.read_uint32());
			m_attackerActorIndex       = static_cast<int>(iff.read_int16());
			m_defenderActorIndex       = static_cast<int>(iff.read_int16());

		iff.exitChunk(TAG_INFO);

	iff.exitForm(TAG_0000);
}

// ----------------------------------------------------------------------

void ArcTargetActionTemplate::load_0001(Iff &iff)
{
	iff.enterForm(TAG_0001);

		iff.enterChunk(TAG_INFO);

			m_arcTimeVariable          = static_cast<Tag>(iff.read_uint32());
			m_distanceMultipleVariable = static_cast<Tag>(iff.read_uint32());
			m_heightMultipleVariable   = static_cast<Tag>(iff.read_uint32());
			m_attackerActorIndex       = static_cast<int>(iff.read_int16());
			m_defenderActorIndex       = static_cast<int>(iff.read_int16());

		iff.exitChunk(TAG_INFO);

	iff.exitForm(TAG_0001);
}

// ======================================================================
