// ======================================================================
//
// StopTrailsActionTemplate.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/StopTrailsActionTemplate.h"

#include "clientAnimation/PassthroughPlaybackAction.h"
#include "clientAnimation/PlaybackScript.h"
#include "clientAnimation/PlaybackScriptTemplate.h"
#include "clientGame/StartTrailsAction.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"

// ======================================================================

namespace StopTrailsActionTemplateNamespace
{
	const Tag TAG_TRBF = TAG(T,R,B,F);
	const Tag TAG_TSTP = TAG(T,S,T,P);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool  s_installed;
}

using namespace StopTrailsActionTemplateNamespace;

// ======================================================================
// StopTrailsActionTemplate: public static members
// ======================================================================

void StopTrailsActionTemplate::install()
{
	InstallTimer const installTimer("StopTrailsActionTemplate::install");

	DEBUG_FATAL(s_installed, ("StopTrailsActionTemplate already installed."));
	
	//-- Register the action template with the playback script system.
	PlaybackScriptTemplate::registerActionTemplate(TAG_TSTP, create);

	s_installed = true;
	ExitChain::add(remove, "StopTrailsActionTemplate");
}

// ======================================================================
// StopTrailsActionTemplate: public members
// ======================================================================

PlaybackAction *StopTrailsActionTemplate::createPlaybackAction() const
{
	return new PassthroughPlaybackAction(*this);
}

// ----------------------------------------------------------------------

bool StopTrailsActionTemplate::update(float deltaTime, PlaybackScript &script) const
{
	UNREF(deltaTime);

	//-- Retrieve the action for the given thread and index.
	PlaybackAction *const action = script.getAction(m_threadIndex, m_actionIndex);
	if (!action)
	{
		DEBUG_REPORT_LOG(true, ("StopTrailsActionTemplate: thread [%d] action [%d] is NULL, can't stop trails on cue.\n", m_threadIndex, m_actionIndex));
		return false;
	}

	// Ensure it is a StartTrailsAction-derived instance.
	StartTrailsAction *const startTrailsAction = safe_cast<StartTrailsAction*>(action);
	if (!startTrailsAction)
	{
		DEBUG_REPORT_LOG(true, ("StopTrailsActionTemplate: thread [%d] action [%d] is non-NULL but isn't a StartTrailsAction, can't stop trails on cue.\n", m_threadIndex, m_actionIndex));
		return false;
	}

	//-- Stop the trails now.
	startTrailsAction->killTrailAppearances();

	//-- Signal that the action has completed.
	return false;
}

// ======================================================================
// StopTrailsActionTemplate: private static members
// ======================================================================

void StopTrailsActionTemplate::remove()
{
	DEBUG_FATAL(!s_installed, ("StopTrailsActionTemplate not installed."));

	//-- Deregister the action template with the playback script system.
	PlaybackScriptTemplate::deregisterActionTemplate(TAG_TSTP);

	s_installed = false;
}

// ----------------------------------------------------------------------

PlaybackActionTemplate *StopTrailsActionTemplate::create(Iff &iff)
{
	return new StopTrailsActionTemplate(iff);
}

// ======================================================================
// StopTrailsActionTemplate: private members
// ======================================================================

StopTrailsActionTemplate::StopTrailsActionTemplate(Iff &iff) :
	PlaybackActionTemplate(),
	m_threadIndex(-1),
	m_actionIndex(-1)
{
	iff.enterForm(TAG_TSTP);

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
					DEBUG_FATAL(true, ("unsupported StopTrailsActionTemplate [%s] specified.", buffer));

					iff.exitForm(TAG_TSTP, true);
					return;
				}
		}

	iff.exitForm(TAG_TSTP);
}

// ----------------------------------------------------------------------

void StopTrailsActionTemplate::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);
		iff.enterChunk(TAG_INFO);

			m_threadIndex = static_cast<int>(iff.read_int16());
			m_actionIndex = static_cast<Tag>(iff.read_int16());

		iff.exitChunk(TAG_INFO);
	iff.exitForm(TAG_0000);
}

// ======================================================================
