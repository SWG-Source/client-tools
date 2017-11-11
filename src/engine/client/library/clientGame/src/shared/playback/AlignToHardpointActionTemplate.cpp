// ======================================================================
//
// AlignToHardpointActionTemplate.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/AlignToHardpointActionTemplate.h"

#include "clientAnimation/PlaybackScriptTemplate.h"
#include "clientGame/AlignToHardpointAction.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"

// ======================================================================

const Tag TAG_ALHP = TAG(A,L,H,P);
const Tag TAG_NONE = TAG(N,O,N,E);

// ======================================================================

bool AlignToHardpointActionTemplate::ms_installed;

// ======================================================================
// class AlignToHardpointActionTemplate: public static member functions
// ======================================================================

void AlignToHardpointActionTemplate::install()
{
	InstallTimer const installTimer("AlignToHardpointActionTemplate::install");

	DEBUG_FATAL(ms_installed, ("AlignToHardpointActionTemplate already installed."));

	PlaybackScriptTemplate::registerActionTemplate(TAG_ALHP, create);

	ms_installed = true;
	ExitChain::add(remove, "AlignToHardpointActionTemplate");
}

// ======================================================================
// class AlignToHardpointActionTemplate: public member functions
// ======================================================================

PlaybackAction *AlignToHardpointActionTemplate::createPlaybackAction() const
{
	return new AlignToHardpointAction(*this);
}

// ======================================================================
// class AlignToHardpointActionTemplate: private static member functions
// ======================================================================

void AlignToHardpointActionTemplate::remove()
{
	DEBUG_FATAL(!ms_installed, ("AlignToHardpointActionTemplate not installed."));
	ms_installed = false;

	PlaybackScriptTemplate::deregisterActionTemplate(TAG_ALHP);
}

// ----------------------------------------------------------------------

PlaybackActionTemplate *AlignToHardpointActionTemplate::create(Iff &iff)
{
	return new AlignToHardpointActionTemplate(iff);
}

// ======================================================================
// class AlignToHardpointActionTemplate: private member functions
// ======================================================================

AlignToHardpointActionTemplate::AlignToHardpointActionTemplate(Iff &iff) :
	PlaybackActionTemplate(),
	m_moveActorIndex(-1),
	m_alignmentActorIndex(-1),
	m_alignmentHardpointNameVariable(TAG_NONE)
{
	DEBUG_FATAL(!ms_installed, ("AlignToHardpointActionTemplate not installed."));

	iff.enterForm(TAG_ALHP);

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
					DEBUG_FATAL(true, ("unsupported AlignToHardpointActionTemplate version [%s].", buffer));
				}
		}

	iff.exitForm(TAG_ALHP);
}

// ----------------------------------------------------------------------

void AlignToHardpointActionTemplate::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);

		iff.enterChunk(TAG_INFO);

			m_moveActorIndex                 = static_cast<int>(iff.read_int16());
			m_alignmentActorIndex            = static_cast<int>(iff.read_int16());
			m_alignmentHardpointNameVariable = static_cast<Tag>(iff.read_uint32());

		iff.exitChunk(TAG_INFO);

	iff.exitForm(TAG_0000);
}

// ======================================================================
