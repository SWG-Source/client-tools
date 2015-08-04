// ======================================================================
//
// ShowAttachedObjectActionTemplate.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/ShowAttachedObjectActionTemplate.h"

#include "clientAnimation/PlaybackScriptTemplate.h"
#include "clientSkeletalAnimation/ShowAttachedObjectAction.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL(ShowAttachedObjectActionTemplate, true, 0, 0, 0);

// ======================================================================

namespace ShowAttachedObjectActionTemplateNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	const Tag TAG_NONE = TAG(N,O,N,E);
	const Tag TAG_SOAT = TAG(S,O,A,T);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool  s_installed;

}

using namespace ShowAttachedObjectActionTemplateNamespace;

// ======================================================================
// class ShowAttachedObjectActionTemplate: public static members
// ======================================================================

void ShowAttachedObjectActionTemplate::install()
{
	DEBUG_FATAL(s_installed, ("ShowAttachedObjectActionTemplate already installed."));

	installMemoryBlockManager();

	//-- Hookup this action template into the system.
	PlaybackScriptTemplate::registerActionTemplate(TAG_SOAT, create);

	s_installed = true;
	ExitChain::add(remove, "ShowAttachedObjectActionTemplate");
}

// ======================================================================
// class ShowAttachedObjectActionTemplate: public members
// ======================================================================

PlaybackAction *ShowAttachedObjectActionTemplate::createPlaybackAction() const
{
	return new ShowAttachedObjectAction(*this);
}

// ----------------------------------------------------------------------

bool ShowAttachedObjectActionTemplate::isShowCommand() const
{
	return m_isShowCommand;
}

// ----------------------------------------------------------------------

Tag ShowAttachedObjectActionTemplate::getHardpointNameVariable() const
{
	return m_hardpointNameVariable;
}

// ----------------------------------------------------------------------

int ShowAttachedObjectActionTemplate::getActorIndex() const
{
	return m_actorIndex;
}

// ----------------------------------------------------------------------

int ShowAttachedObjectActionTemplate::getHideCommandThreadIndex() const
{
	return m_hideCommandThreadIndex;
}

// ----------------------------------------------------------------------

int ShowAttachedObjectActionTemplate::getHideCommandActionIndex() const
{
	return m_hideCommandActionIndex;
}

// ======================================================================
// class ShowAttachedObjectActionTemplate: private static members
// ======================================================================

void ShowAttachedObjectActionTemplate::remove()
{
	DEBUG_FATAL(!s_installed, ("ShowAttachedObjectActionTemplateNamespace not installed."));
	s_installed = false;

	//-- Unhook this action template from the system.
	PlaybackScriptTemplate::deregisterActionTemplate(TAG_SOAT);

	removeMemoryBlockManager();
}

// ----------------------------------------------------------------------

PlaybackActionTemplate *ShowAttachedObjectActionTemplate::create(Iff &iff)
{
	return new ShowAttachedObjectActionTemplate(iff);
}

// ======================================================================
// class ShowAttachedObjectActionTemplate: private members
// ======================================================================

ShowAttachedObjectActionTemplate::ShowAttachedObjectActionTemplate(Iff &iff) :
	PlaybackActionTemplate(),
	m_isShowCommand(false),
	m_hardpointNameVariable(TAG_NONE),
	m_actorIndex(-1),
	m_hideCommandThreadIndex(-1),
	m_hideCommandActionIndex(-1)
{
	iff.enterForm(TAG_SOAT);

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
				DEBUG_FATAL(true, ("unsupported ShowAttachedObjectActionTemplate version [%s].", buffer));
			}
	}

	iff.exitForm(TAG_SOAT);
}

// ----------------------------------------------------------------------

void ShowAttachedObjectActionTemplate::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);
		iff.enterChunk(TAG_INFO);

			m_isShowCommand          = (iff.read_uint8() != 0);
			m_hardpointNameVariable  = static_cast<Tag>(iff.read_uint32());
			m_actorIndex             = static_cast<int>(iff.read_uint8());
			m_hideCommandThreadIndex = static_cast<int>(iff.read_uint8());
			m_hideCommandActionIndex = static_cast<int>(iff.read_uint8());

		iff.exitChunk(TAG_INFO);
	iff.exitForm(TAG_0000);
}

// ======================================================================
