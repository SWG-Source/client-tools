// ======================================================================
//
// GamePlaybackScript.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/GamePlaybackScript.h"

#include "clientAnimation/PlaybackScript.h"
#include "clientAnimation/PlaybackScriptTemplate.h"
#include "clientAnimation/PlaybackScriptTemplateList.h"
#include "clientAnimation/PriorityPlaybackScriptManager.h"
#include "clientGame/CreatureObject.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/Tag.h"
#include "sharedObject/Appearance.h"
#include "swgSharedUtility/Postures.h"

#include <deque>
#include <string>
#include <vector>

// ======================================================================

namespace GamePlaybackScriptNamespace
{
	Tag const TAG_EPS  = TAG3(E,P,S);
	Tag const TAG_HPNT = TAG(H,P,N,T);
	Tag const TAG_TIME = TAG(T,I,M,E);

	const int                             cs_sitScriptPriority = 100;

	bool                                  s_installed;

	const PlaybackScriptTemplate         *s_sitInChairScriptTemplate;
	const PlaybackScriptTemplate         *s_standFromChairScriptTemplate;

	PlaybackScriptTemplate::ObjectVector  s_actors;
}

using namespace GamePlaybackScriptNamespace;

// ======================================================================

void GamePlaybackScript::install()
{
	InstallTimer const installTimer("GamePlaybackScript::install");

	DEBUG_FATAL(s_installed, ("already installed"));

	s_sitInChairScriptTemplate = PlaybackScriptTemplateList::fetch(ConstCharCrcLowerString("playback/change_posture_align.pst"));
	NOT_NULL(s_sitInChairScriptTemplate);

	s_standFromChairScriptTemplate = PlaybackScriptTemplateList::fetch(ConstCharCrcLowerString("playback/restore_scale.pst"));
	NOT_NULL(s_standFromChairScriptTemplate);

	s_installed = true;
	ExitChain::add(remove, "GamePlaybackScript");
}

// ----------------------------------------------------------------------

void GamePlaybackScript::sitCreatureOnChair(Object &creatureObject, Object &chairObject, int positionIndex)
{
	// We bailed on multi-seater chairs.
	UNREF(positionIndex);

	DEBUG_FATAL(!s_installed, ("not installed."));

	//-- Setup script actors.
	s_actors.resize(2);
	s_actors[0] = &creatureObject;
	s_actors[1] = &chairObject;

	//-- Create a playback script using the creature as the move object and the target as the alignment object.
	//   Default values for script variables are fine.
	PlaybackScript *const playbackScript = s_sitInChairScriptTemplate->createPlaybackScript(cs_sitScriptPriority, s_actors);
	NOT_NULL(playbackScript);
		
	//-- Submit the script for playback.
	IGNORE_RETURN(PriorityPlaybackScriptManager::submitPlaybackScript(playbackScript));
}

// ----------------------------------------------------------------------

void GamePlaybackScript::standFromChair(Object &creatureObject)
{
	DEBUG_FATAL(!s_installed, ("not installed."));

	s_actors.resize(1);
	s_actors[0] = &creatureObject;

	PlaybackScript *const playbackScript = s_standFromChairScriptTemplate->createPlaybackScript(cs_sitScriptPriority, s_actors);
	NOT_NULL(playbackScript);
	
	//-- Submit the script for playback.
	IGNORE_RETURN(PriorityPlaybackScriptManager::submitPlaybackScript(playbackScript));
}

// ----------------------------------------------------------------------

void GamePlaybackScript::sitOnSaddle(Object &riderObject, Object &mountObject, CrcString const &riderHardpointName)
{
	DEBUG_FATAL(!s_installed, ("not installed."));

	//-- For now we just snap the player appearance's scale to 1.0f.
	Appearance *const appearance = riderObject.getAppearance();
	if (appearance)
		appearance->setScale(Vector::xyz111);

	UNREF(mountObject);
	UNREF(riderHardpointName);
}

// ----------------------------------------------------------------------

void GamePlaybackScript::standFromSaddle(Object &riderObject)
{
	DEBUG_FATAL(!s_installed, ("not installed."));

	//-- For now we just snap the player appearance's scale back to whatever scale is specified for the CreatureObject.
	ClientObject *const riderAsClientObject = riderObject.asClientObject();
	if (riderAsClientObject)
	{
		CreatureObject *const riderAsCreatureObject = riderAsClientObject->asCreatureObject();
		if (riderAsCreatureObject)
		{
			Appearance *const appearance = riderAsCreatureObject->getAppearance();
			if (appearance)
			{
				float const riderDismountedScale = riderAsCreatureObject->getScaleFactor();
				appearance->setScale(Vector(riderDismountedScale, riderDismountedScale, riderDismountedScale));
			}
		}
	}
}

// ======================================================================

void GamePlaybackScript::remove()
{
	DEBUG_FATAL(!s_installed, ("not installed."));

	s_standFromChairScriptTemplate->release();
	s_standFromChairScriptTemplate = 0;

	s_sitInChairScriptTemplate->release();
	s_sitInChairScriptTemplate = 0;

	s_installed = false;
}

// ======================================================================
