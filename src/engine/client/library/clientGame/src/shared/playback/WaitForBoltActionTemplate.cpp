// ======================================================================
//
// WaitForBoltActionTemplate.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/WaitForBoltAction.h"
#include "clientGame/WaitForBoltActionTemplate.h"

#include "clientAnimation/PassthroughPlaybackAction.h"
#include "clientAnimation/PlaybackScript.h"
#include "clientAnimation/PlaybackScriptTemplate.h"
#include "clientAnimation/WatcherAction.h"
#include "clientGame/ClientCombatActionInfo.h"
#include "clientGame/FireProjectileActionTemplate.h"
#include "clientGame/PlaySkeletalAnimationAction.h"
#include "clientGame/PlaySkeletalAnimationActionTemplate.h"
#include "clientSkeletalAnimation/AnimationMessageAction.h"
#include "clientSkeletalAnimation/CallbackAnimationNotification.h"
#include "clientSkeletalAnimation/SkeletalAnimation.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/StateHierarchyAnimationController.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedObject/Object.h"
#include "sharedRandom/Random.h"

#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <vector>

// ======================================================================

bool WaitForBoltActionTemplate::ms_installed;

// ======================================================================

const Tag         TAG_WFBO                    = TAG(W,F,B,O);

namespace
{
	bool  ms_logInfo = true;
}

// ======================================================================
// class WaitForBoltActionTemplate: public static member functions
// ======================================================================

void WaitForBoltActionTemplate::install()
{
	InstallTimer const installTimer("WaitForBoltActionTemplate::install");

	DEBUG_FATAL(ms_installed, ("WaitForBoltActionTemplate already installed"));

	PlaybackScriptTemplate::registerActionTemplate(TAG_WFBO, create);

	ms_installed = true;
	ExitChain::add(remove, "WaitForBoltActionTemplate");
}

// ======================================================================
// class WaitForBoltActionTemplate: public member functions
// ======================================================================

PlaybackAction *WaitForBoltActionTemplate::createPlaybackAction() const
{
	return new WaitForBoltAction(*this);
}

// ----------------------------------------------------------------------

bool WaitForBoltActionTemplate::update(float deltaTime, PlaybackScript &script) const
{
	UNREF(deltaTime);
	UNREF(script);
	
	//-- Indicate to caller that this script action is complete.
	return false;
}

// ======================================================================
// class WaitForBoltActionTemplate: private static member functions
// ======================================================================

void WaitForBoltActionTemplate::remove()
{
	DEBUG_FATAL(!ms_installed, ("WaitForBoltActionTemplate not installed."));

	ms_installed = false;
	PlaybackScriptTemplate::deregisterActionTemplate(TAG_WFBO);
}

// ----------------------------------------------------------------------

PlaybackActionTemplate *WaitForBoltActionTemplate::create(Iff &iff)
{
	return new WaitForBoltActionTemplate(iff);
}

WaitForBoltActionTemplate::WaitForBoltActionTemplate(int attackerActorIndex, int defenderActorIndex, bool subtractInitialWait) :
	PlaybackActionTemplate(),
	m_attackerActorIndex(attackerActorIndex),
	m_defenderActorIndex(defenderActorIndex),
	m_subtractInitialWait(subtractInitialWait)
{
	DEBUG_FATAL(!ms_installed, ("WaitForBoltActionTemplate not installed."));
}


// ======================================================================
// class WaitForBoltActionTemplate: private member functions
// ======================================================================

WaitForBoltActionTemplate::WaitForBoltActionTemplate(Iff &iff) :
	PlaybackActionTemplate(),
	m_attackerActorIndex(-1),
	m_defenderActorIndex(-1),
	m_subtractInitialWait(true)
{
	DEBUG_FATAL(!ms_installed, ("WaitForBoltActionTemplate not installed."));

	iff.enterForm(TAG_WFBO);

		const Tag version = iff.getCurrentName();
		switch (version)
		{
			case TAG_0002:
				load_0002(iff);
				break;

			default:
				{
					char buffer[5];
					ConvertTagToString(version, buffer);
					FATAL(true, ("WaitForBoltActionTemplate: unsupported version [%s].", buffer));
				}
		}

	iff.exitForm(TAG_WFBO);
}

// ----------------------------------------------------------------------

void WaitForBoltActionTemplate::load_0002(Iff &iff)
{
	iff.enterForm(TAG_0002);

		iff.enterChunk(TAG_INFO);

			m_attackerActorIndex   = static_cast<int>(iff.read_int16());
			m_defenderActorIndex   = static_cast<int>(iff.read_int16());

		iff.exitChunk(TAG_INFO);

	iff.exitForm(TAG_0002);
}

// ----------------------------------------------------------------------

int WaitForBoltActionTemplate::getAttackerActorIndex() const
{
	return m_attackerActorIndex;
}

// ----------------------------------------------------------------------

int WaitForBoltActionTemplate::getDefenderActorIndex() const
{
	return m_defenderActorIndex;
}

// ----------------------------------------------------------------------

bool WaitForBoltActionTemplate::getSubtractInitialWait() const
{
	return m_subtractInitialWait;
}


// ======================================================================
