// ======================================================================
//
// ChangeScaleActionTemplate.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ChangeScaleActionTemplate.h"

#include "clientAnimation/PlaybackScriptTemplate.h"
#include "clientGame/ChangeScaleAction.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"

// ======================================================================

const Tag TAG_NONE = TAG(N,O,N,E);
const Tag TAG_SCAL = TAG(S,C,A,L);

// ======================================================================

bool ChangeScaleActionTemplate::ms_installed;

// ======================================================================
// class ChangeScaleActionTemplate: public static member functions
// ======================================================================

void ChangeScaleActionTemplate::install()
{
	InstallTimer const installTimer("ChangeScaleActionTemplate::install");

	DEBUG_FATAL(ms_installed, ("ChangeScaleActionTemplate already installed."));

	PlaybackScriptTemplate::registerActionTemplate(TAG_SCAL, create);

	ms_installed = true;
	ExitChain::add(remove, "ChangeScaleActionTemplate");
}

// ======================================================================
// class ChangeScaleActionTemplate: public member functions
// ======================================================================

PlaybackAction *ChangeScaleActionTemplate::createPlaybackAction() const
{
	return new ChangeScaleAction(*this);
}

// ======================================================================
// class ChangeScaleActionTemplate: private static member functions
// ======================================================================

void ChangeScaleActionTemplate::remove()
{
	DEBUG_FATAL(!ms_installed, ("ChangeScaleActionTemplate not installed."));
	ms_installed = false;

	PlaybackScriptTemplate::deregisterActionTemplate(TAG_SCAL);
}

// ----------------------------------------------------------------------

PlaybackActionTemplate *ChangeScaleActionTemplate::create(Iff &iff)
{
	return new ChangeScaleActionTemplate(iff);
}

// ======================================================================
// class ChangeScaleActionTemplate: private member functions
// ======================================================================

ChangeScaleActionTemplate::ChangeScaleActionTemplate(Iff &iff) :
	PlaybackActionTemplate(),
	m_actorIndex(-1),
	m_blendMode(BM_targetToObject),
	m_targetScaleVariable(TAG_NONE),
	m_timeDurationVariable(TAG_NONE)
{
	DEBUG_FATAL(!ms_installed, ("ChangeScaleActionTemplate not installed."));

	iff.enterForm(TAG_SCAL);

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
					DEBUG_FATAL(true, ("unsupported ChangeScaleActionTemplate version [%s].", buffer));
				}
		}

	iff.exitForm(TAG_SCAL);
}

// ----------------------------------------------------------------------

void ChangeScaleActionTemplate::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);

		iff.enterChunk(TAG_INFO);

			m_actorIndex           = static_cast<int>(iff.read_int16());
			m_blendMode            = static_cast<BlendMode>(iff.read_int8());
			m_targetScaleVariable  = static_cast<Tag>(iff.read_uint32());
			m_timeDurationVariable = static_cast<Tag>(iff.read_uint32());

		iff.exitChunk(TAG_INFO);

	iff.exitForm(TAG_0000);
}

// ======================================================================
