// ======================================================================
//
// PriorityPlaybackScriptManager.cpp
// Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "clientAnimation/FirstClientAnimation.h"
#include "clientAnimation/PriorityPlaybackScriptManager.h"

#include "clientAnimation/ConfigClientAnimation.h"
#include "clientAnimation/PlaybackScript.h"
#include "clientAnimation/PlaybackScriptManager.h"
#include "sharedRandom/Random.h"

#include <algorithm>
#include <vector>

// ======================================================================

namespace PriorityPlaybackScriptManagerNamespace
{
	bool s_ignorePriority = false;
}

using namespace PriorityPlaybackScriptManagerNamespace;

PriorityPlaybackScriptManager::ObjectVector          PriorityPlaybackScriptManager::ms_actorObjects;
PriorityPlaybackScriptManager::PlaybackScriptVector  PriorityPlaybackScriptManager::ms_playbackScripts;

// ======================================================================
// class PriorityPlaybackScriptManager: public static member functions
// ======================================================================

bool PriorityPlaybackScriptManager::submitPlaybackScript(PlaybackScript *playbackScript, bool alwaysPlayScript)
{
	//-- Handle the case where the game wants us to ignore priority altogether and just play the damn thing.
	//   Note this will allow any of the participants of this script to be active in other scripts that are already playing.
	//   Those other scripts will continue to play.
	if (alwaysPlayScript)
	{
		PlaybackScriptManager::addPlaybackScript(playbackScript);
		return true;
	}

	//-- Validate parameters.
	if (!playbackScript)
	{
		WARNING_STRICT_FATAL(true, ("playbackScript is NULL, ignoring."));
		return false;
	}

	//-- Support disabled combat trumping.
	if (s_ignorePriority)
	{
		// Combat trumping is disabled.  This means we will let multiple combat
		// playback scripts play for a given actor at the same time.  This may
		// cause unpredictable results.
		PlaybackScriptManager::addPlaybackScript(playbackScript);
		return true;
	}


	bool  acceptNewScript = false;

	//-- Get participating actor Objects from the submitted PlaybackScript instance.
	// NOTE: if we already determined that some actors are not active at this point,
	//       we should only collect active actors here.
	ms_actorObjects.clear();
	getPlaybackScriptActors(*playbackScript, ms_actorObjects);

	//-- Get the set of currently-running PlaybackScript instances in which any
	//   member of ms_actorObjects is active.
	ms_playbackScripts.clear();
	getPlaybackScriptsWithActiveActors(ms_actorObjects, ms_playbackScripts);

	if (ms_playbackScripts.empty())
	{
		acceptNewScript = true;
		DEBUG_REPORT_LOG(ConfigClientAnimation::getLogPriorityHandling(), ("PPSM: playback script [%s] allowed, no conflicting active scripts.\n", playbackScript->getPlaybackScriptTemplateName()));
	}
	else
	{
		// TRUE: At least one PlaybackScript instance is currently playing that contains
		//       an active actor that is also active in the submitting script.  This is a
		//       PlaybackScript collision.  Only one of the PlaybackScript instances should
		//       play.  Now I figure out whether to continue playing the existing
		//       script(s) and ignore the submitted script, or kill the playing script(s)
		//       and start the new script.
		
		const int submittedPriority = playbackScript->getPriority();

		//-- Find the highest priority of the specified PlaybackScript instances.
		const int maxCurrentPriority = getMaxPlaybackScriptPriority(ms_playbackScripts);
		if (maxCurrentPriority > submittedPriority)
		{
			// TRUE: At least one of the scripts currently being played on at least one of the
			//       actors in the submitted script has a priority higher than the submitted script.

			//-- Ignore the submitted script.
			acceptNewScript = false;
			DEBUG_REPORT_LOG(ConfigClientAnimation::getLogPriorityHandling(), ("PPSM: playback script [%s] disallowed, one of [%u] conflicting active scripts trumped.\n", playbackScript->getPlaybackScriptTemplateName(), ms_playbackScripts.size()));
		}
		else
		{
			// TRUE: All of the scripts currently being played containing members
			//       from the submitted script are at the same or lower priority
			//       than the submitted script.

			//-- If the submitted script is competing with another script currently playing at the identical
			//   priority level, flip a coin to see who wins.
			if ((maxCurrentPriority == submittedPriority) && (Random::random(100) > 50))
			{
				// TRUE: The submitted script is at the same priority level as another script, and this time
				//       it has been randomly chosen that the submitted script will not trump the currently
				//       playing animation.
				acceptNewScript = false;
				DEBUG_REPORT_LOG(ConfigClientAnimation::getLogPriorityHandling(), ("PPSM: playback script [%s] disallowed, one of the [%u] conflicting active scripts was at the same priority level and randomly won.\n", playbackScript->getPlaybackScriptTemplateName(), ms_playbackScripts.size()));
			}
			else
			{
				// TRUE: If there was another script playing at the same priority level, it has been randomly trumped.
				//       Otherwise any currently playing scripts are at a lower priority than this script.

				//-- Play the submitted script, stop the rest.
				acceptNewScript = true;
				DEBUG_REPORT_LOG(ConfigClientAnimation::getLogPriorityHandling(), ("PPSM: playback script [%s] allowed, all [%u] conflicting active scripts will terminate.\n", playbackScript->getPlaybackScriptTemplateName(), ms_playbackScripts.size()));
			}
		}
	}

	//-- Handle the determined fate of the submitted script.
	if (acceptNewScript)
	{
		//-- Kill the old PlaybackScript instances.
		std::for_each(ms_playbackScripts.begin(), ms_playbackScripts.end(), PlaybackScriptManager::killPlaybackScript);

		//-- Play the submitted PlaybackScript instance.
		PlaybackScriptManager::addPlaybackScript(playbackScript);

		return true;
	}
	else
	{
		//-- Kill the submitted PlaybackScript; continue playing other playback scripts.
		delete playbackScript;

		return false;
	}
}

// ======================================================================
// class PriorityPlaybackScriptManager: private static member functions
// ======================================================================

void PriorityPlaybackScriptManager::getPlaybackScriptActors(const PlaybackScript &playbackScript, ObjectVector &actorObjects)
{
	//-- Add all non-null potentially active actor Object instances to actorObjects.
	const int supportedActorCount = playbackScript.getSupportedActorCount();
	for (int i = 0; i < supportedActorCount; ++i)
	{
		if (playbackScript.isActorActive(i))
		{
			//-- If we get here, the object should not be able to be NULL.
			const Object *const object = playbackScript.getActor(i);
			NOT_NULL(object);

			actorObjects.push_back(object);
		}
	}
}

// ----------------------------------------------------------------------
/**
 * Retrieve the list of currently-playing PlaybackScript instances that actively contain 
 * any of the specified actors.
 *
 * @param actorObjects     the list of actors for which to search.
 * @param playbackScripts  the PlaybackScript instances matching the search
 *                         criteria are appended to this list.
 */

void PriorityPlaybackScriptManager::getPlaybackScriptsWithActiveActors(const ObjectVector &actorObjects, PlaybackScriptVector &playbackScripts)
{
	//-- Search each active PlaybackScript instance.
	const int playbackScriptCount = PlaybackScriptManager::getPlaybackScriptCount();
	for (int i = 0; i < playbackScriptCount; ++i)
	{
		//-- Get the playback script.
		const PlaybackScript *const script = PlaybackScriptManager::getPlaybackScript(i);
		if (!script)
		{
			WARNING_STRICT_FATAL(true, ("NULL PlaybackScript instance index [%d] of count [%d].", i, playbackScriptCount));
			continue;
		}

		//-- Check each script actor for a match against the target actor Object instances.
		const int supportedActorCount = script->getSupportedActorCount();
		for (int actorIndex = 0; actorIndex < supportedActorCount; ++actorIndex)
		{
			const Object *const object = script->getActor(actorIndex);
			if (object && script->isActorActive(actorIndex))
			{
				// Check if this script active actor is a target actor.
				const bool scriptContainsTargetActor = (std::find(actorObjects.begin(), actorObjects.end(), object) != actorObjects.end());
				if (scriptContainsTargetActor)
				{
					// This script contains a targe actor.  Add this script to the list of
					// scripts that matched the criteria.  Stop checking actors in this script 
					// since the script has been accepted.
					playbackScripts.push_back(script);
					break;
				}
			}
		}
	}
}

// ----------------------------------------------------------------------
/**
 * Return the maximum priority value from the list of PlaybackScript instances.
 *
 * @param playbackScripts  the vector of PlaybackScript instances to scan for
 *                         a maximum priority value.
 *
 * @return  the maximum priority value from the list of PlaybackScript instances
 *          if playbackScripts is not empty; otherwise, return -1.
 */

int PriorityPlaybackScriptManager::getMaxPlaybackScriptPriority(const PlaybackScriptVector &playbackScripts)
{
	//-- Find the container element containing the max value for priority.
	const PlaybackScriptVector::const_iterator findIt = std::max_element(playbackScripts.begin(), playbackScripts.end(), PlaybackScript::LessPriorityComparator());

	//-- Return the priority.
	if (findIt != playbackScripts.end())
		return (*findIt)->getPriority();
	else
		return -1;
}

// ----------------------------------------------------------------------

void PriorityPlaybackScriptManager::setPlaybackPriorityIgnore(bool ignorePriority)
{
	s_ignorePriority = ignorePriority;
}

// ======================================================================
