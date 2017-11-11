// ======================================================================
//
// TemporaryAttachedObjectActionTemplate.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/TemporaryAttachedObjectActionTemplate.h"

#include "clientAnimation/PlaybackScriptTemplate.h"
#include "clientGame/TemporaryAttachedObjectAction.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL(TemporaryAttachedObjectActionTemplate, true, 0, 0, 0);

// ======================================================================

namespace TemporaryAttachedObjectActionTemplateNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	const Tag TAG_NONE = TAG(N,O,N,E);
	const Tag TAG_TOAT = TAG(T,O,A,T);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool  s_installed;

}

using namespace TemporaryAttachedObjectActionTemplateNamespace;

// ======================================================================
// class TemporaryAttachedObjectActionTemplate: public static members
// ======================================================================

void TemporaryAttachedObjectActionTemplate::install()
{
	InstallTimer const installTimer("TemporaryAttachedObjectActionTemplate::install");

	DEBUG_FATAL(s_installed, ("TemporaryAttachedObjectActionTemplate already installed."));

	installMemoryBlockManager();

	//-- Hookup this action template into the system.
	PlaybackScriptTemplate::registerActionTemplate(TAG_TOAT, create);

	s_installed = true;
	ExitChain::add(remove, "TemporaryAttachedObjectActionTemplate");
}

// ======================================================================
// class TemporaryAttachedObjectActionTemplate: public members
// ======================================================================

PlaybackAction *TemporaryAttachedObjectActionTemplate::createPlaybackAction() const
{
	return new TemporaryAttachedObjectAction(*this);
}

// ----------------------------------------------------------------------

bool TemporaryAttachedObjectActionTemplate::isCreateCommand() const
{
	return m_isCreateCommand;
}

// ----------------------------------------------------------------------

Tag TemporaryAttachedObjectActionTemplate::getObjectTemplateNameVariable() const
{
	return m_objectTemplateNameVariable;
}

// ----------------------------------------------------------------------

Tag TemporaryAttachedObjectActionTemplate::getHardpointNameVariable() const
{
	return m_hardpointNameVariable;
}

// ----------------------------------------------------------------------

int TemporaryAttachedObjectActionTemplate::getActorIndex() const
{
	return m_actorIndex;
}

// ======================================================================
// class TemporaryAttachedObjectActionTemplate: private static members
// ======================================================================

void TemporaryAttachedObjectActionTemplate::remove()
{
	DEBUG_FATAL(!s_installed, ("TemporaryAttachedObjectActionTemplateNamespace not installed."));
	s_installed = false;

	//-- Unhook this action template from the system.
	PlaybackScriptTemplate::deregisterActionTemplate(TAG_TOAT);

	removeMemoryBlockManager();
}

// ----------------------------------------------------------------------

PlaybackActionTemplate *TemporaryAttachedObjectActionTemplate::create(Iff &iff)
{
	return new TemporaryAttachedObjectActionTemplate(iff);
}

// ======================================================================
// class TemporaryAttachedObjectActionTemplate: private members
// ======================================================================

TemporaryAttachedObjectActionTemplate::TemporaryAttachedObjectActionTemplate(Iff &iff) :
	PlaybackActionTemplate(),
	m_isCreateCommand(false),
	m_objectTemplateNameVariable(TAG_NONE),
	m_hardpointNameVariable(TAG_NONE),
	m_actorIndex(-1),
	m_createCommandThreadIndex(255),
	m_createCommandActionIndex(255)
{
	iff.enterForm(TAG_TOAT);

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
				DEBUG_FATAL(true, ("unsupported TemporaryAttachedObjectActionTemplate version [%s].", buffer));
			}
	}

	iff.exitForm(TAG_TOAT);
}

// ----------------------------------------------------------------------

void TemporaryAttachedObjectActionTemplate::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);
		iff.enterChunk(TAG_INFO);

			m_isCreateCommand            = (iff.read_uint8() != 0);
			m_objectTemplateNameVariable = static_cast<Tag>(iff.read_uint32());
			m_hardpointNameVariable      = static_cast<Tag>(iff.read_uint32());
			m_actorIndex                 = static_cast<int>(iff.read_uint8());
			m_createCommandThreadIndex   = static_cast<int>(iff.read_uint8());
			m_createCommandActionIndex   = static_cast<int>(iff.read_uint8());

		iff.exitChunk(TAG_INFO);
	iff.exitForm(TAG_0000);
}

// ----------------------------------------------------------------------

int TemporaryAttachedObjectActionTemplate::getCreateCommandThreadIndex() const
{
	return m_createCommandThreadIndex;
}

// ----------------------------------------------------------------------

int TemporaryAttachedObjectActionTemplate::getCreateCommandActionIndex() const
{
	return m_createCommandActionIndex;
}

// ======================================================================
