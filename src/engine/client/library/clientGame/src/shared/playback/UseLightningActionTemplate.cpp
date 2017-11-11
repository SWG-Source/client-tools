// ======================================================================
//
// UseLightningActionTemplate.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/UseLightningActionTemplate.h"

#include "clientAnimation/PlaybackScriptTemplate.h"
#include "clientGame/UseLightningAction.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"

// ======================================================================

namespace UseLightningActionTemplateNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	Tag const TAG_NONE = TAG(N,O,N,E);
	Tag const TAG_ULAT = TAG(U,L,A,T);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool  s_installed;
}

using namespace UseLightningActionTemplateNamespace;

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL(UseLightningActionTemplate, true, 0, 0, 0);

// ======================================================================
// class UseLightningActionTemplate: PUBLIC STATIC
// ======================================================================

void UseLightningActionTemplate::install()
{
	InstallTimer const installTimer("UseLightningActionTemplate::install");

	DEBUG_FATAL(s_installed, ("UseLightningActionTemplate already installed."));
	
	installMemoryBlockManager();
	PlaybackScriptTemplate::registerActionTemplate(TAG_ULAT, create);

	s_installed = true;
	ExitChain::add(remove, "UseLightningActionTemplate");
}

// ======================================================================
// class UseLightningActionTemplate: PUBLIC
// ======================================================================

PlaybackAction *UseLightningActionTemplate::createPlaybackAction() const
{
	return new UseLightningAction(*this);
}

// ----------------------------------------------------------------------

int UseLightningActionTemplate::getAttackerActorIndex() const
{
	return m_attackerActorIndex;
}

// ----------------------------------------------------------------------

int UseLightningActionTemplate::getDefenderActorIndex() const
{
	return m_defenderActorIndex;
}

// ----------------------------------------------------------------------

Tag UseLightningActionTemplate::getHardpointVariableTag(int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, static_cast<int>(cms_maxHardpointCount));
	return m_hardpointVariableTag[index];
}

// ----------------------------------------------------------------------

Tag UseLightningActionTemplate::getLevelVariableTag() const
{
	return m_levelVariableTag;
}

// ----------------------------------------------------------------------

Tag UseLightningActionTemplate::getTotalTimeVariableTag() const
{
	return m_totalTimeVariableTag;
}

// ----------------------------------------------------------------------

Tag UseLightningActionTemplate::getGrowTimeFractionVariableTag() const
{
	return m_growTimeFractionVariableTag;
}

// ----------------------------------------------------------------------

Tag UseLightningActionTemplate::getDeathTimeFractionVariableTag() const
{
	return m_deathTimeFractionVariableTag;
}

// ----------------------------------------------------------------------

Tag UseLightningActionTemplate::getBeginSoundVariableTag() const
{
	return m_beginSoundVariableTag;
}

// ----------------------------------------------------------------------

Tag UseLightningActionTemplate::getLoopSoundVariableTag() const
{
	return m_loopSoundVariableTag;
}

// ----------------------------------------------------------------------

Tag UseLightningActionTemplate::getEndSoundVariableTag() const
{
	return m_endSoundVariableTag;
}

// ----------------------------------------------------------------------

Tag UseLightningActionTemplate::getAppearanceTemplateVariableTag() const
{
	return m_appearanceTemplateVariableTag;
}

// ----------------------------------------------------------------------

Tag UseLightningActionTemplate::getBeamRotationRadiusVariableTag() const
{
	return m_beamRotationRadiusVariableTag;
}

// ----------------------------------------------------------------------

Tag UseLightningActionTemplate::getBeamDpsRotationRateVariableTag() const
{
	return m_beamRotationRateVariableTag;
}

// ======================================================================
// class UseLightningActionTemplate: PRIVATE STATIC
// ======================================================================

void UseLightningActionTemplate::remove()
{
	DEBUG_FATAL(!s_installed, ("UseLightningActionTemplate not installed."));
	s_installed = false;

	PlaybackScriptTemplate::deregisterActionTemplate(TAG_ULAT);

	removeMemoryBlockManager();
}

// ----------------------------------------------------------------------

PlaybackActionTemplate *UseLightningActionTemplate::create(Iff &iff)
{
	return new UseLightningActionTemplate(iff);
}

// ======================================================================
// class UseLightningActionTemplate: PRIVATE
// ======================================================================

UseLightningActionTemplate::UseLightningActionTemplate(Iff &iff):
	PlaybackActionTemplate(),
	m_attackerActorIndex(-1),
	m_defenderActorIndex(-1),
	m_appearanceTemplateVariableTag(TAG_NONE),
	m_levelVariableTag(TAG_NONE),
	m_totalTimeVariableTag(TAG_NONE),
	m_growTimeFractionVariableTag(TAG_NONE),
	m_deathTimeFractionVariableTag(TAG_NONE),
	m_beginSoundVariableTag(TAG_NONE),
	m_loopSoundVariableTag(TAG_NONE),
	m_endSoundVariableTag(TAG_NONE),
	m_beamRotationRadiusVariableTag(TAG_NONE),
	m_beamRotationRateVariableTag(TAG_NONE)
{
	iff.enterForm(TAG_ULAT);

		for (size_t i = 0; i < cms_maxHardpointCount; ++i)
			m_hardpointVariableTag[i] = TAG_NONE;

		Tag const version = iff.getCurrentName();
		switch (version)
		{
			case TAG_0000:
				load_0000(iff);
				break;

			case TAG_0001:
				load_0001(iff);
				break;

			case TAG_0002:
				load_0002(iff);
				break;

			default:
				{
					char buffer[5];
					ConvertTagToString(version, buffer);
					FATAL(true, ("Unsupported UseLightningActionTemplate version [%s].", buffer));
				}
		}

	iff.exitForm(TAG_ULAT);
}

// ----------------------------------------------------------------------

void UseLightningActionTemplate::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);
		iff.enterChunk(TAG_INFO);

			m_attackerActorIndex           = static_cast<int>(iff.read_int16());
			m_defenderActorIndex           = static_cast<int>(iff.read_int16());

			m_hardpointVariableTag[0]      = static_cast<Tag>(iff.read_uint32());
			m_hardpointVariableTag[1]      = static_cast<Tag>(iff.read_uint32());

			m_levelVariableTag             = static_cast<Tag>(iff.read_uint32());
			m_totalTimeVariableTag         = static_cast<Tag>(iff.read_uint32());
			m_growTimeFractionVariableTag  = static_cast<Tag>(iff.read_uint32());
			m_deathTimeFractionVariableTag = static_cast<Tag>(iff.read_uint32());

			m_beginSoundVariableTag        = static_cast<Tag>(iff.read_uint32());
			m_loopSoundVariableTag         = static_cast<Tag>(iff.read_uint32());
			m_endSoundVariableTag          = static_cast<Tag>(iff.read_uint32());

		iff.exitChunk(TAG_INFO);
	iff.exitForm(TAG_0000);
}

// ----------------------------------------------------------------------

void UseLightningActionTemplate::load_0001(Iff &iff)
{
	iff.enterForm(TAG_0001);
		iff.enterChunk(TAG_INFO);

			m_attackerActorIndex           = static_cast<int>(iff.read_int16());
			m_defenderActorIndex           = static_cast<int>(iff.read_int16());

			m_appearanceTemplateVariableTag= static_cast<Tag>(iff.read_uint32());

			m_hardpointVariableTag[0]      = static_cast<Tag>(iff.read_uint32());
			m_hardpointVariableTag[1]      = static_cast<Tag>(iff.read_uint32());

			m_levelVariableTag             = static_cast<Tag>(iff.read_uint32());
			m_totalTimeVariableTag         = static_cast<Tag>(iff.read_uint32());
			m_growTimeFractionVariableTag  = static_cast<Tag>(iff.read_uint32());
			m_deathTimeFractionVariableTag = static_cast<Tag>(iff.read_uint32());

			m_beginSoundVariableTag        = static_cast<Tag>(iff.read_uint32());
			m_loopSoundVariableTag         = static_cast<Tag>(iff.read_uint32());
			m_endSoundVariableTag          = static_cast<Tag>(iff.read_uint32());

		iff.exitChunk(TAG_INFO);
	iff.exitForm(TAG_0001);
}

// ----------------------------------------------------------------------

void UseLightningActionTemplate::load_0002(Iff &iff)
{
	iff.enterForm(TAG_0002);
		iff.enterChunk(TAG_INFO);

			m_attackerActorIndex           = static_cast<int>(iff.read_int16());
			m_defenderActorIndex           = static_cast<int>(iff.read_int16());

			m_appearanceTemplateVariableTag= static_cast<Tag>(iff.read_uint32());

			m_hardpointVariableTag[0]      = static_cast<Tag>(iff.read_uint32());
			m_hardpointVariableTag[1]      = static_cast<Tag>(iff.read_uint32());

			m_levelVariableTag             = static_cast<Tag>(iff.read_uint32());
			m_totalTimeVariableTag         = static_cast<Tag>(iff.read_uint32());
			m_growTimeFractionVariableTag  = static_cast<Tag>(iff.read_uint32());
			m_deathTimeFractionVariableTag = static_cast<Tag>(iff.read_uint32());

			m_beginSoundVariableTag        = static_cast<Tag>(iff.read_uint32());
			m_loopSoundVariableTag         = static_cast<Tag>(iff.read_uint32());
			m_endSoundVariableTag          = static_cast<Tag>(iff.read_uint32());

			m_beamRotationRadiusVariableTag= static_cast<Tag>(iff.read_uint32());
			m_beamRotationRateVariableTag  = static_cast<Tag>(iff.read_uint32());

		iff.exitChunk(TAG_INFO);
	iff.exitForm(TAG_0002);
}

// ======================================================================
