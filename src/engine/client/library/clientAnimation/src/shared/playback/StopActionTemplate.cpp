// ======================================================================
//
// StopActionTemplate.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientAnimation/FirstClientAnimation.h"
#include "clientAnimation/StopActionTemplate.h"

#include "clientAnimation/PassthroughPlaybackAction.h"
#include "clientAnimation/PlaybackScript.h"
#include "clientAnimation/PlaybackScriptTemplate.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL(StopActionTemplate, true, 0, 0, 0);

// ======================================================================

namespace StopActionTemplateNamespace
{
	Tag const TAG_STOP = TAG(S,T,O,P);

	bool  s_installed;
}

using namespace StopActionTemplateNamespace;

// ======================================================================

void StopActionTemplate::install()
{
	InstallTimer const installTimer("StopActionTemplate::install");

	DEBUG_FATAL(s_installed, ("StopActionTemplate already installed."));

	installMemoryBlockManager();
	PlaybackScriptTemplate::registerActionTemplate(TAG_STOP, create);

	s_installed = true;
	ExitChain::add(remove, "StopActionTemplate");
}

// ======================================================================

PlaybackAction *StopActionTemplate::createPlaybackAction() const
{
	return new PassthroughPlaybackAction(*this);
}

// ----------------------------------------------------------------------

bool StopActionTemplate::update(float deltaTime, PlaybackScript &script) const
{
	UNREF(deltaTime);

	//-- Stop the specified action so that it no longer gets an update.
	//   Also, move the thread's instruction pointer past this action.
	script.stopAction(m_threadIndex, m_actionIndex);

	//-- This action always completes immediately after one call.
	return false;
}

// ======================================================================

void StopActionTemplate::remove()
{
	DEBUG_FATAL(!s_installed, ("StopActionTemplate not installed."));
	s_installed = false;

	PlaybackScriptTemplate::deregisterActionTemplate(TAG_STOP);
	removeMemoryBlockManager();
}

// ----------------------------------------------------------------------

PlaybackActionTemplate *StopActionTemplate::create(Iff &iff)
{
	return new StopActionTemplate(iff);
}

// ======================================================================

StopActionTemplate::StopActionTemplate(Iff &iff) :
	PlaybackActionTemplate(),
	m_threadIndex(-1),
	m_actionIndex(-1)
{
	iff.enterForm(TAG_STOP);

		Tag const version = iff.getCurrentName();
		switch (version)
		{
			case TAG_0000:
				load_0000(iff);
				break;

			default:
				{
					char name[5];

					ConvertTagToString(version, name);
					FATAL(true, ("Unsupported StopActionTemplate version [%s].", name));
				}
		}

	iff.exitForm(TAG_STOP);
}

// ----------------------------------------------------------------------

void StopActionTemplate::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);
		iff.enterChunk(TAG_INFO);

			m_threadIndex = static_cast<int>(iff.read_int16());
			m_actionIndex = static_cast<int>(iff.read_int16());

		iff.exitChunk(TAG_INFO);
	iff.exitForm(TAG_0000);
}

// ======================================================================
