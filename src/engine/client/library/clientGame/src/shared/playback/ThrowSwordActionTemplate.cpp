// ======================================================================
//
// ThrowSwordActionTemplate.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ThrowSwordActionTemplate.h"

#include "clientAnimation/PlaybackScriptTemplate.h"
#include "clientGame/ThrowSwordAction.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedObject/Object.h"

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL(ThrowSwordActionTemplate, true, 0, 0, 0);

// ======================================================================

namespace ThrowSwordActionTemplateNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	Tag const TAG_NONE = TAG(N,O,N,E);
	Tag const TAG_THSW = TAG(T,H,S,W);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool  s_installed;
}

using namespace ThrowSwordActionTemplateNamespace;

// ======================================================================
// class ThrowSwordActionTemplate: PUBLIC STATIC FUNCTIONS
// ======================================================================

void ThrowSwordActionTemplate::install()
{
	InstallTimer const installTimer("ThrowSwordActionTemplate::install");

	DEBUG_FATAL(s_installed, ("ThrowSwordActionTemplate already installed."));

	installMemoryBlockManager();

	//-- Tell playback script system about this action type.
	PlaybackScriptTemplate::registerActionTemplate(TAG_THSW, create);

	s_installed = true;
	ExitChain::add(remove, "ThrowSwordActionTemplate");
}

// ======================================================================
// class ThrowSwordActionTemplate: PUBLIC FUNCTIONS
// ======================================================================

PlaybackAction *ThrowSwordActionTemplate::createPlaybackAction() const
{
	return new ThrowSwordAction(*this);
}

// ----------------------------------------------------------------------

int ThrowSwordActionTemplate::getAttackerActorIndex() const
{
	return m_attackerActorIndex;
}

// ----------------------------------------------------------------------

int ThrowSwordActionTemplate::getDefenderActorIndex() const
{
	return m_defenderActorIndex;
}

// ----------------------------------------------------------------------

Tag ThrowSwordActionTemplate::getHardpointNameVariable() const
{
	return m_hardpointNameVariable;
}

// ----------------------------------------------------------------------

Tag ThrowSwordActionTemplate::getYawRateVariable() const
{
	return m_yawRateVariable;
}

// ----------------------------------------------------------------------

Tag ThrowSwordActionTemplate::getThrowSpeedVariable() const
{
	return m_throwSpeedVariable;
}

// ----------------------------------------------------------------------

Tag ThrowSwordActionTemplate::getDamageVariable() const
{
	return m_damageVariable;
}

// ----------------------------------------------------------------------

Tag ThrowSwordActionTemplate::getMissExtentMultiplierVariable() const
{
	return m_missExtentMultiplierVariable;
}

// ----------------------------------------------------------------------

Tag ThrowSwordActionTemplate::getHitExtentMultiplierVariable() const
{
	return m_hitExtentMultiplierVariable;
}

// ----------------------------------------------------------------------

Tag ThrowSwordActionTemplate::getHitEventNameVariable() const
{
	return m_hitEventNameVariable;
}

// ----------------------------------------------------------------------

Tag ThrowSwordActionTemplate::getHitDefenderActionNameVariable() const
{
	return m_hitDefenderActionNameVariable;
}

// ----------------------------------------------------------------------

Tag ThrowSwordActionTemplate::getCatchTimeDurationVariable() const
{
	return m_catchTimeDurationVariable;
}

// ----------------------------------------------------------------------

Tag ThrowSwordActionTemplate::getCatchSwordActionNameVariable() const
{
	return m_catchSwordActionNameVariable;
}

// ======================================================================
// class ThrowSwordActionTemplate: PRIVATE STATIC FUNCTIONS
// ======================================================================

void ThrowSwordActionTemplate::remove()
{
	DEBUG_FATAL(!s_installed, ("ThrowSwordActionTemplate not installed."));
	s_installed = false;

	PlaybackScriptTemplate::deregisterActionTemplate(TAG_THSW);
	removeMemoryBlockManager();
}

// ----------------------------------------------------------------------

PlaybackActionTemplate *ThrowSwordActionTemplate::create(Iff &iff)
{
	return new ThrowSwordActionTemplate(iff);
}

// ======================================================================
// class ThrowSwordActionTemplate: PRIVATE FUNCTIONS
// ======================================================================

ThrowSwordActionTemplate::ThrowSwordActionTemplate(Iff &iff) :
	PlaybackActionTemplate(),
	m_attackerActorIndex(-1),
	m_defenderActorIndex(-1),
	m_hardpointNameVariable(TAG_NONE),
	m_yawRateVariable(TAG_NONE),
	m_throwSpeedVariable(TAG_NONE),
	m_damageVariable(TAG_NONE),
	m_missExtentMultiplierVariable(TAG_NONE),
	m_hitExtentMultiplierVariable(TAG_NONE),
	m_hitEventNameVariable(TAG_NONE),
	m_hitDefenderActionNameVariable(TAG_NONE),
	m_catchTimeDurationVariable(TAG_NONE),
	m_catchSwordActionNameVariable(TAG_NONE)
{
	iff.enterForm(TAG_THSW);

		Tag const version = iff.getCurrentName();
		switch (version)
		{
			case TAG_0000:
				load_0000(iff);
				break;

			default:
				{
					char buffer[5];
					ConvertTagToString(version, buffer);
					DEBUG_FATAL(true, ("Unsupported ThrowSwordActionTemplate version [%s].", buffer));
				}
		}

	iff.exitForm(TAG_THSW);
}

// ----------------------------------------------------------------------

void ThrowSwordActionTemplate::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);
		iff.enterChunk(TAG_INFO);

			m_attackerActorIndex = static_cast<int>(iff.read_int16());
			m_defenderActorIndex = static_cast<int>(iff.read_int16());

			m_damageVariable                = static_cast<Tag>(iff.read_uint32());
			m_hardpointNameVariable         = static_cast<Tag>(iff.read_uint32());
			m_yawRateVariable               = static_cast<Tag>(iff.read_uint32());
			m_throwSpeedVariable            = static_cast<Tag>(iff.read_uint32());
			m_missExtentMultiplierVariable  = static_cast<Tag>(iff.read_uint32());
			m_hitExtentMultiplierVariable   = static_cast<Tag>(iff.read_uint32());
			m_hitEventNameVariable          = static_cast<Tag>(iff.read_uint32());
			m_hitDefenderActionNameVariable = static_cast<Tag>(iff.read_uint32());
			m_catchTimeDurationVariable     = static_cast<Tag>(iff.read_uint32());
			m_catchSwordActionNameVariable  = static_cast<Tag>(iff.read_uint32());

		iff.exitChunk(TAG_INFO);
	iff.exitForm(TAG_0000);
}

// ======================================================================
