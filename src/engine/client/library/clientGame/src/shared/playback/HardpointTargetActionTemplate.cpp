// ======================================================================
//
// HardpointTargetActionTemplate.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/HardpointTargetActionTemplate.h"

#include "clientAnimation/PlaybackScriptTemplate.h"
#include "clientGame/HardpointTargetAction.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedObject/Object.h"

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL(HardpointTargetActionTemplate, true, 0, 0, 0);

// ======================================================================

namespace HardpointTargetActionTemplateNamespace
{
	bool  ms_installed;
}

using namespace HardpointTargetActionTemplateNamespace;

// ======================================================================

const Tag TAG_HPTA = TAG(H,P,T,A);
const Tag TAG_NONE = TAG(N,O,N,E);

// ======================================================================
// class HardpointTargetActionTemplate: public static member functions
// ======================================================================

void HardpointTargetActionTemplate::install()
{
	InstallTimer const installTimer("HardpointTargetActionTemplate::install");

	DEBUG_FATAL(ms_installed, ("HardpointTargetActionTemplate already installed."));

	installMemoryBlockManager();
	PlaybackScriptTemplate::registerActionTemplate(TAG_HPTA, create);

	ms_installed = true;
	ExitChain::add(remove, "HardpointTargetActionTemplate");
}

// ======================================================================
// class HardpointTargetActionTemplate: public member functions
// ======================================================================

PlaybackAction *HardpointTargetActionTemplate::createPlaybackAction() const
{
	return new HardpointTargetAction(*this);
}


// ======================================================================
// class HardpointTargetActionTemplate: private static member functions
// ======================================================================

void HardpointTargetActionTemplate::remove()
{
	DEBUG_FATAL(!ms_installed, ("HardpointTargetActionTemplate not installed."));
	ms_installed = false;

	PlaybackScriptTemplate::deregisterActionTemplate(TAG_HPTA);
	removeMemoryBlockManager();
}

// ----------------------------------------------------------------------

PlaybackActionTemplate *HardpointTargetActionTemplate::create(Iff &iff)
{
	return new HardpointTargetActionTemplate(iff);
}

// ======================================================================
// class HardpointTargetActionTemplate: private member functions
// ======================================================================

HardpointTargetActionTemplate::HardpointTargetActionTemplate(Iff &iff) :
	PlaybackActionTemplate(),
	m_attackerActorIndex(-1),
	m_defenderActorIndex(-1),
	m_hardpointNameVariable(TAG_NONE)
{
	DEBUG_FATAL(!ms_installed, ("HardpointTargetActionTemplate not installed."));

	iff.enterForm(TAG_HPTA);

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
					DEBUG_FATAL(true, ("unsupported HardpointTargetActionTemplate version [%s].", buffer));
				}
		}

	iff.exitForm(TAG_HPTA);
}

// ----------------------------------------------------------------------

void HardpointTargetActionTemplate::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);

		iff.enterChunk(TAG_INFO);

			m_attackerActorIndex       = static_cast<int>(iff.read_int16());
			m_defenderActorIndex       = static_cast<int>(iff.read_int16());
			m_hardpointNameVariable    = static_cast<Tag>(iff.read_uint32());

		iff.exitChunk(TAG_INFO);

	iff.exitForm(TAG_0000);
}

// ======================================================================
