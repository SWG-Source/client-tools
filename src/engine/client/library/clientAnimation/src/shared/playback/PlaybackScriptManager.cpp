// ======================================================================
//
// PlaybackScriptManager.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientAnimation/FirstClientAnimation.h"
#include "clientAnimation/PlaybackScriptManager.h"

#include "clientAnimation/PlaybackScript.h"
#include "sharedDebug/Profiler.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/PointerDeleter.h"

#include <algorithm>
#include <vector>

namespace PlaybackScriptManagerNamespace
{
	bool s_lockedForUpdateDontDelete = false;
}

using namespace PlaybackScriptManagerNamespace;

// ======================================================================

void PlaybackScriptManager::install()
{
	InstallTimer const installTimer("PlaybackScriptManager::install");

	DEBUG_FATAL(ms_installed, ("PlaybackScriptManager already installed"));

#ifdef _DEBUG
	ms_installed = true;
#endif
	ExitChain::add(remove, "PlaybackScriptManager");
}

// ----------------------------------------------------------------------

void PlaybackScriptManager::cleanup()
{
	DEBUG_FATAL(!ms_installed, ("PlaybackScriptManager not installed"));

	//-- Clean up playback scripts still in the pipe.  Should be called by a game uninstall routine prior to running the ExitChain for PlaybackScript-related libraries.
	DEBUG_REPORT_LOG(!ms_playbackScripts.empty(), ("PlaybackScriptManager: [%u] scripts exist at shutdown, cleaning up.\n", ms_playbackScripts.size()));
	std::for_each(ms_playbackScripts.begin(), ms_playbackScripts.end(), PointerDeleter());
	ms_playbackScripts.clear();
}

// ----------------------------------------------------------------------
/**
 * Update each managed PlaybackScript instance.
 */

void PlaybackScriptManager::update(float deltaTime)
{
	DEBUG_FATAL(!ms_installed, ("PlaybackScriptManager not installed"));
	s_lockedForUpdateDontDelete = true;

	NP_PROFILER_AUTO_BLOCK_DEFINE("PlaybackScriptManager::update");

	// update can trigger other playback scripts being added to
	// the manager. this can invalidate our iterators so we make
	// local copies of the vector to work with
	PlaybackScriptVector const localPlaybackScripts = ms_playbackScripts;
	PlaybackScriptVector playbackScriptsToDelete;

	{
		PlaybackScriptVector::const_iterator ii = localPlaybackScripts.begin();
		PlaybackScriptVector::const_iterator iiEnd = localPlaybackScripts.end();

		for (; ii != iiEnd; ++ii)
		{
			// get the script
			PlaybackScript *const playbackScript = *ii;
			NOT_NULL(playbackScript);

			if (playbackScript != 0)
			{
				// update the script
				const bool keepMe = playbackScript->update(deltaTime);

				// queue for deletion if script has ended
				if (!keepMe)
				{
					playbackScriptsToDelete.push_back(playbackScript);
				}
			}
		}
	}

	{
		// delete what we need to
		PlaybackScriptVector::iterator ii = playbackScriptsToDelete.begin();
		PlaybackScriptVector::iterator iiEnd = playbackScriptsToDelete.end();

		for (; ii != iiEnd; ++ii)
		{
			PlaybackScript * const playbackScript = *ii;
			NOT_NULL(playbackScript);

			if (playbackScript != 0)
			{
				// find and remove the iterator first just in case deleting the
				// playback script puts anything back on list
				PlaybackScriptVector::iterator jj = std::find(ms_playbackScripts.begin(), ms_playbackScripts.end(), playbackScript);
				if (jj != ms_playbackScripts.end())
				{
					ms_playbackScripts.erase(jj);
				}
				else
				{
					DEBUG_FATAL(true, ("PlaybackScriptManager::update: could not delete iterator from ms_playbackScripts"));
				}

				delete playbackScript;
			}
		}
	}

	s_lockedForUpdateDontDelete = false;
}

// ----------------------------------------------------------------------

void PlaybackScriptManager::addPlaybackScript(PlaybackScript *playbackScript)
{
	DEBUG_FATAL(!ms_installed, ("PlaybackScriptManager not installed"));
	NOT_NULL(playbackScript);

	ms_playbackScripts.push_back(playbackScript);
}

// ----------------------------------------------------------------------
/**
 * Remove the PlaybackScript instance from the manager, and delete the
 * PlaybackScript instance.
 *
 * @param playbackScript  the script to stop managing and delete.
 */

void PlaybackScriptManager::killPlaybackScript(const PlaybackScript *playbackScript)
{
	DEBUG_FATAL(!ms_installed, ("PlaybackScriptManager not installed"));
	DEBUG_FATAL(s_lockedForUpdateDontDelete, ("PlaybackScriptManager::killPlaybackScript: not allowed to delete while updating"));

	//-- Erase the script entry from the management table.
	const PlaybackScriptVector::iterator findIt = std::find(ms_playbackScripts.begin(), ms_playbackScripts.end(), playbackScript);
	if (findIt != ms_playbackScripts.end())
	{
		// Delete the instance.
		delete (*findIt);

		// Remove from the managed list.
		IGNORE_RETURN(ms_playbackScripts.erase(findIt));
	}
	else
		WARNING_STRICT_FATAL(true, ("playbackScript at [0x%08x] is not managed, ignoring request.", playbackScript));
}

// ----------------------------------------------------------------------

int PlaybackScriptManager::getPlaybackScriptCount()
{
	DEBUG_FATAL(!ms_installed, ("PlaybackScriptManager not installed"));
	return static_cast<int>(ms_playbackScripts.size());
}

// ----------------------------------------------------------------------

const PlaybackScript *PlaybackScriptManager::getPlaybackScript(int index)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getPlaybackScriptCount());
	return ms_playbackScripts[static_cast<PlaybackScriptVector::size_type>(index)];
}

// ----------------------------------------------------------------------
/**
 * Append all playback scripts that contain the specified actor Object
 * instance to the specified container.
 *
 * @param object           the Object instance used for the search criteria.
 * @param playbackScripts  the container where PlaybackScript instances
 *                         will be returned if they match the search
 *                         criteria.
 */

void PlaybackScriptManager::getPlaybackScriptsForActor(const Object *object, ConstPlaybackScriptVector &playbackScripts)
{
	//-- Check args.
	if (!object)
	{
		WARNING_STRICT_FATAL(true, ("object arg is NULL."));
		return;
	}

	//-- Find the script.
	const int count = getPlaybackScriptCount();
	for (int i = 0; i < count; ++i)
	{
		// Get the script.
		const PlaybackScript *script = getPlaybackScript(i);
		if (!script)
			continue;

		// Check if the search object is an actor in the script.
		const int actorCount = script->getSupportedActorCount();
		for (int actorIndex = 0; actorIndex < actorCount; ++actorIndex)
		{
			if (script->getActor(actorIndex) == object)
			{
				// Found a script that matches the search criteria, add it to the container.
				playbackScripts.push_back(script);

				// No point in continuing to check this script since it has been accepted.
				break;
			}
		}
	}
}

// ----------------------------------------------------------------------
/**
 * Find an active PlaybackScript that contains the specified actor object
 * in the specified actor index.
 *
 * @param actorIndex  the position of the actor within the script.  Attackers
 *                    are always in actorIndex 0 for combat-related scripts.
 * @param object      the Object instance to match in the specified actor index slot.
 *
 * @return  the first found PlaybackScript instance that matches the search criteria.
 */

const PlaybackScript *PlaybackScriptManager::getPlaybackScriptForActorAtIndex(int actorIndex, const Object *object)
{
	//-- Check args.
	if (actorIndex < 0)
	{
		WARNING_STRICT_FATAL(true, ("actorIndex arg [%d] out of valid range.", actorIndex));
		return 0;
	}

	if (!object)
	{
		WARNING_STRICT_FATAL(true, ("object arg is NULL."));
		return 0;
	}

	//-- Find the script.
	const int count = getPlaybackScriptCount();
	for (int i = 0; i < count; ++i)
	{
		// Get the script.
		const PlaybackScript *script = getPlaybackScript(i);
		if (!script)
			continue;

		// Check if the specified actor slot holds the specified Object.
		if (actorIndex < script->getSupportedActorCount())
		{
			if (script->getActor(actorIndex) == object)
			{
				// Found a script that matches the search criteria, return it.
				return script;
			}
		}
	}

	//-- No active scripts found that match the criteria.
	return 0;
}

// ======================================================================

void PlaybackScriptManager::remove()
{
	DEBUG_FATAL(!ms_installed, ("PlaybackScriptManager not installed"));

	//-- Warn if we are going to leak memory.  I don't delete these here because deleting a playback script
	//   may cause action templates to be deleted that have been removed already, generating a FATAL.
	DEBUG_WARNING(!ms_playbackScripts.empty(), ("PlaybackScriptManager: [%u] scripts exist at shutdown, leaking memory since these cannot be deleted without risking a FATAL.", ms_playbackScripts.size()));
	// std::for_each(ms_playbackScripts.begin(), ms_playbackScripts.end(), PointerDeleter());

	//-- release unneeded memory
	{
		PlaybackScriptVector tempVector;
		tempVector.swap(ms_playbackScripts);
	}

#ifdef _DEBUG
	ms_installed = false;
#endif
}

// ======================================================================

PlaybackScriptManager::PlaybackScriptVector  PlaybackScriptManager::ms_playbackScripts;

#ifdef _DEBUG
bool                                         PlaybackScriptManager::ms_installed;
#endif

// ======================================================================
