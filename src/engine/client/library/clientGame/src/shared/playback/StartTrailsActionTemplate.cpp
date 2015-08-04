// ======================================================================
//
// StartTrailsActionTemplate.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/StartTrailsActionTemplate.h"

#include "clientAnimation/PlaybackScriptTemplate.h"
#include "clientGame/StartTrailsAction.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"

// ======================================================================

namespace StartTrailsActionTemplateNamespace
{
	const Tag TAG_SWSH = TAG(S,W,S,H);
	const Tag TAG_TRBF = TAG(T,R,B,F);
	const Tag TAG_TSTR = TAG(T,S,T,R);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool  s_installed;
}

using namespace StartTrailsActionTemplateNamespace;

// ======================================================================
// StartTrailsActionTemplate: public static members
// ======================================================================

void StartTrailsActionTemplate::install()
{
	InstallTimer const installTimer("StartTrailsActionTemplate::install");

	DEBUG_FATAL(s_installed, ("StartTrailsActionTemplate already installed."));
	
	//-- Register the action template with the playback script system.
	PlaybackScriptTemplate::registerActionTemplate(TAG_TSTR, create);

	s_installed = true;
	ExitChain::add(remove, "StartTrailsActionTemplate");
}

// ======================================================================
// StartTrailsActionTemplate: public members
// ======================================================================

PlaybackAction *StartTrailsActionTemplate::createPlaybackAction() const
{
	return new StartTrailsAction(*this);
}

// ======================================================================
// StartTrailsActionTemplate: private static members
// ======================================================================

void StartTrailsActionTemplate::remove()
{
	DEBUG_FATAL(!s_installed, ("StartTrailsActionTemplate not installed."));

	//-- Deregister the action template with the playback script system.
	PlaybackScriptTemplate::deregisterActionTemplate(TAG_TSTR);

	s_installed = false;
}

// ----------------------------------------------------------------------

PlaybackActionTemplate *StartTrailsActionTemplate::create(Iff &iff)
{
	return new StartTrailsActionTemplate(iff);
}

// ======================================================================
// StartTrailsActionTemplate: private members
// ======================================================================

StartTrailsActionTemplate::StartTrailsActionTemplate(Iff &iff) :
	PlaybackActionTemplate(),
	m_actorIndex(0),
	m_trailBitfieldVariable(TAG_TRBF),
	m_swooshFilenameVariable(TAG_SWSH)
{
	iff.enterForm(TAG_TSTR);

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
					DEBUG_FATAL(true, ("unsupported StartTrailsActionTemplate [%s] specified.", buffer));

					iff.exitForm(TAG_TSTR, true);
					return;
				}
		}

	iff.exitForm(TAG_TSTR);
}

// ----------------------------------------------------------------------

void StartTrailsActionTemplate::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);
		iff.enterChunk(TAG_INFO);

			m_actorIndex            = static_cast<int>(iff.read_int16());
			m_trailBitfieldVariable = static_cast<Tag>(iff.read_uint32());

		iff.exitChunk(TAG_INFO);
	iff.exitForm(TAG_0000);
}

// ----------------------------------------------------------------------

void StartTrailsActionTemplate::load_0001(Iff &iff)
{
	iff.enterForm(TAG_0001);
		iff.enterChunk(TAG_INFO);

			m_actorIndex             = static_cast<int>(iff.read_int16());
			m_trailBitfieldVariable  = static_cast<Tag>(iff.read_uint32());
			m_swooshFilenameVariable = static_cast<Tag>(iff.read_uint32());

		iff.exitChunk(TAG_INFO);
	iff.exitForm(TAG_0001);
}

// ======================================================================
