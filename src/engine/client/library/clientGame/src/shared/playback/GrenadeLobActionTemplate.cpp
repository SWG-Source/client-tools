// ======================================================================
//
// GrenadeLobActionTemplate.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/GrenadeLobActionTemplate.h"

#include "clientAnimation/PlaybackScript.h"
#include "clientAnimation/PlaybackScriptTemplate.h"
#include "clientGame/GrenadeLobAction.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedObject/Object.h"
#include "sharedObject/ObjectTemplate.h"

// ======================================================================

const Tag TAG_AMSG = TAG(A,M,S,G);
const Tag TAG_GRLB = TAG(G,R,L,B);
const Tag TAG_NONE = TAG(N,O,N,E);

// ======================================================================

bool GrenadeLobActionTemplate::ms_installed;

// ======================================================================
// class GrenadeLobActionTemplate: public static member functions
// ======================================================================

void GrenadeLobActionTemplate::install()
{
	InstallTimer const installTimer("GrenadeLobActionTemplate::install");

	DEBUG_FATAL(ms_installed, ("GrenadeLobActionTemplate already installed."));

	PlaybackScriptTemplate::registerActionTemplate(TAG_GRLB, create);

	ms_installed = true;
	ExitChain::add(remove, "GrenadeLobActionTemplate");
}

// ======================================================================
// class GrenadeLobActionTemplate: public member functions
// ======================================================================

PlaybackAction *GrenadeLobActionTemplate::createPlaybackAction() const
{
	return new GrenadeLobAction(*this);
}

// ----------------------------------------------------------------------

bool GrenadeLobActionTemplate::getHardpointName(const PlaybackScript &script, CrcLowerString &name) const
{
	//-- Retrieve the hardpoint name.
	std::string  hardpointName;

	const bool gotName = script.getStringVariable(getHardpointNameVariable(), hardpointName);
	if (!gotName)
		return false;

	//-- Set the string.
	name.setString(hardpointName.c_str());
	return true;
}

// ======================================================================
// class GrenadeLobActionTemplate: private static member functions
// ======================================================================

void GrenadeLobActionTemplate::remove()
{
	DEBUG_FATAL(!ms_installed, ("GrenadeLobActionTemplate not installed."));
	ms_installed = false;

	PlaybackScriptTemplate::deregisterActionTemplate(TAG_GRLB);
}

// ----------------------------------------------------------------------

PlaybackActionTemplate *GrenadeLobActionTemplate::create(Iff &iff)
{
	return new GrenadeLobActionTemplate(iff);
}

// ======================================================================
// class GrenadeLobActionTemplate: private member functions
// ======================================================================

GrenadeLobActionTemplate::GrenadeLobActionTemplate(Iff &iff) :
	PlaybackActionTemplate(),
	m_attackerActorIndex(-1),
	m_defenderActorIndex(-1),
	m_hardpointNameVariable(TAG_NONE),
	m_gravitationalForceVariable(TAG_NONE),
	m_airTimeVariable(TAG_NONE),
	m_sitTimeVariable(TAG_NONE),
	m_isHitVariable(TAG_NONE),
	m_throwAtCenterVariable(TAG_NONE),
	m_trackTargetVariable(TAG_NONE),
	m_throwAnimationThreadIndex(-1),
	m_throwAnimationActionIndex(-1),
	m_throwAnimationMessageNameVariable(TAG_AMSG)
{
	DEBUG_FATAL(!ms_installed, ("GrenadeLobActionTemplate not installed."));

	iff.enterForm(TAG_GRLB);

		const Tag version = iff.getCurrentName();
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

			case TAG_0003:
				load_0003(iff);
				break;

			default:
				{
					char buffer[5];

					ConvertTagToString(version, buffer);
					DEBUG_FATAL(true, ("unsupported GrenadeLobActionTemplate version [%s].", buffer));
				}
		}

	iff.exitForm(TAG_GRLB);
}

// ----------------------------------------------------------------------

void GrenadeLobActionTemplate::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);

		iff.enterChunk(TAG_INFO);

			m_attackerActorIndex         = static_cast<int>(iff.read_int16());
			m_defenderActorIndex         = static_cast<int>(iff.read_int16());

			m_hardpointNameVariable      = static_cast<Tag>(iff.read_uint32());
			m_gravitationalForceVariable = static_cast<Tag>(iff.read_uint32());
			m_airTimeVariable            = static_cast<Tag>(iff.read_uint32());
			m_sitTimeVariable            = static_cast<Tag>(iff.read_uint32());
			m_isHitVariable              = static_cast<Tag>(iff.read_uint32());

			// These are the one and only values that are used during version 0000.
			m_throwAnimationThreadIndex  = 0;
			m_throwAnimationActionIndex  = 2;

			m_throwAnimationMessageNameVariable = TAG_AMSG;

		iff.exitChunk(TAG_INFO);

	iff.exitForm(TAG_0000);
}

// ----------------------------------------------------------------------

void GrenadeLobActionTemplate::load_0001(Iff &iff)
{
	iff.enterForm(TAG_0001);

		iff.enterChunk(TAG_INFO);

			m_attackerActorIndex         = static_cast<int>(iff.read_int16());
			m_defenderActorIndex         = static_cast<int>(iff.read_int16());

			m_hardpointNameVariable      = static_cast<Tag>(iff.read_uint32());
			m_gravitationalForceVariable = static_cast<Tag>(iff.read_uint32());
			m_airTimeVariable            = static_cast<Tag>(iff.read_uint32());
			m_sitTimeVariable            = static_cast<Tag>(iff.read_uint32());
			m_isHitVariable              = static_cast<Tag>(iff.read_uint32());

			m_throwAnimationThreadIndex  = static_cast<int>(iff.read_int16());
			m_throwAnimationActionIndex  = static_cast<int>(iff.read_int16());
			m_throwAnimationMessageNameVariable = static_cast<Tag>(iff.read_uint32());

		iff.exitChunk(TAG_INFO);

	iff.exitForm(TAG_0001);
}

// ----------------------------------------------------------------------

void GrenadeLobActionTemplate::load_0002(Iff &iff)
{
	iff.enterForm(TAG_0002);

	iff.enterChunk(TAG_INFO);

	m_attackerActorIndex         = static_cast<int>(iff.read_int16());
	m_defenderActorIndex         = static_cast<int>(iff.read_int16());

	m_hardpointNameVariable      = static_cast<Tag>(iff.read_uint32());
	m_gravitationalForceVariable = static_cast<Tag>(iff.read_uint32());
	m_airTimeVariable            = static_cast<Tag>(iff.read_uint32());
	m_sitTimeVariable            = static_cast<Tag>(iff.read_uint32());
	m_isHitVariable              = static_cast<Tag>(iff.read_uint32());
	m_throwAtCenterVariable      = static_cast<Tag>(iff.read_uint32());

	m_throwAnimationThreadIndex  = static_cast<int>(iff.read_int16());
	m_throwAnimationActionIndex  = static_cast<int>(iff.read_int16());
	m_throwAnimationMessageNameVariable = static_cast<Tag>(iff.read_uint32());

	iff.exitChunk(TAG_INFO);

	iff.exitForm(TAG_0002);
}

// ----------------------------------------------------------------------

void GrenadeLobActionTemplate::load_0003(Iff &iff)
{
	iff.enterForm(TAG_0003);

	iff.enterChunk(TAG_INFO);

	m_attackerActorIndex         = static_cast<int>(iff.read_int16());
	m_defenderActorIndex         = static_cast<int>(iff.read_int16());

	m_hardpointNameVariable      = static_cast<Tag>(iff.read_uint32());
	m_gravitationalForceVariable = static_cast<Tag>(iff.read_uint32());
	m_airTimeVariable            = static_cast<Tag>(iff.read_uint32());
	m_sitTimeVariable            = static_cast<Tag>(iff.read_uint32());
	m_isHitVariable              = static_cast<Tag>(iff.read_uint32());
	m_throwAtCenterVariable      = static_cast<Tag>(iff.read_uint32());
	m_trackTargetVariable        = static_cast<Tag>(iff.read_uint32());

	m_throwAnimationThreadIndex  = static_cast<int>(iff.read_int16());
	m_throwAnimationActionIndex  = static_cast<int>(iff.read_int16());
	m_throwAnimationMessageNameVariable = static_cast<Tag>(iff.read_uint32());

	iff.exitChunk(TAG_INFO);

	iff.exitForm(TAG_0003);
}

// ======================================================================
