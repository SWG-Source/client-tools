// ======================================================================
//
// PlaybackScript.cpp
// Copyright 2001 - 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientAnimation/FirstClientAnimation.h"
#include "clientAnimation/PlaybackScript.h"

#include "clientAnimation/PlaybackAction.h"
#include "clientAnimation/PlaybackScriptTemplate.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/Tag.h"
#include "sharedObject/Object.h"

#include <algorithm>
#include <map>
#include <string>

// ======================================================================

namespace PlaybackScriptNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	const Tag TAG_AEPS = TAG(A,E,P,S);
	const Tag TAG_DEPS = TAG(D,E,P,S);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool  s_installed;

	bool  s_logPlaybackScriptTimeouts;
	bool  s_logUndefinedVariableAccess;
	bool  s_verbosePlaybackScriptWarnings = true;
}

using namespace PlaybackScriptNamespace;

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL(PlaybackScript, true, 0, 0, 0);

// ======================================================================
// class PlaybackScript: PUBLIC STATIC
// ======================================================================

void PlaybackScript::install()
{
	InstallTimer const installTimer("PlaybackScript::install");

	DEBUG_FATAL(s_installed, ("PlaybackScript already installed!"));

	installMemoryBlockManager();

	//-- Register debug flags.
	DebugFlags::registerFlag(s_logPlaybackScriptTimeouts, "ClientAnimation", "logPlaybackScriptTimeouts");
	DebugFlags::registerFlag(s_logUndefinedVariableAccess, "ClientAnimation", "logUndefinedVariableAccess");
	DebugFlags::registerFlag(s_verbosePlaybackScriptWarnings, "ClientAnimation", "verbosePlaybackScriptWarnings");

	s_installed = true;
	ExitChain::add(remove, "PlaybackScript");
}

// ======================================================================
// public member functions
// ======================================================================

PlaybackScript::~PlaybackScript()
{
	callCleanupOnAllActions();

	//-- remove each actor present so actions and triggers have a chance to cleanup.
	{
		const int actorCount = static_cast<int>(m_actors.size());
		for (int actorIndex = 0; actorIndex < actorCount; ++actorIndex)
		{
			// check if this actor is present
			Object *const object = m_actors[static_cast<size_t>(actorIndex)].getPointer();
			if (!object)
				continue;

			// remove the actor
			removeActor(actorIndex);
		}
	}

	const ThreadVector::iterator threadEndIt = m_threads.end();
	for (ThreadVector::iterator threadIt = m_threads.begin(); threadIt != threadEndIt; ++threadIt)
	{
		// notify all actions in this thread
		PlaybackActionVector *const actions = *threadIt;
		NOT_NULL(actions);

		std::for_each(actions->begin(), actions->end(), PointerDeleter());
		delete actions;
	}

	//-- release template reference
	m_playbackScriptTemplate->release();
	m_playbackScriptTemplate = 0;
}

// ----------------------------------------------------------------------
/**
 * Perform all activity for this frame.
 *
 * All activity handled by the PlaybackScript occurs during the update
 * cycle.
 *
 * The PlaybackScript should be deleted once this routine returns false.
 * It is an error to call update() after the routine returns false.
 *
 * @param   deltaTime  the amount of time that occured since the last
 *                     call to update.
 *
 * @return  true if the PlaybackScript is not complete; false when it is
 *          complete.
 */
bool PlaybackScript::update(float deltaTime)
{
	//-- check for an aborted script
	if (m_abortRequested)
		return false;

	int activeThreadCount = 0;

	//-- run all threads
	//   Note: per-loop size() call is necessary.  Thread count can change during execution;
	//   iterators are dangerous in this context.
	for (ThreadVector::size_type threadIndex = 0; threadIndex < m_threads.size(); ++threadIndex)
	{
		PlaybackActionVector *const actions = m_threads[threadIndex];
		NOT_NULL(actions);

		//-- Update (run) all actions.  start with current action, run up through the first action that returns true.
		//   Note:   the explicit size call per loop iteration is necessary --- an action can actually add other actions (e.g. FireSetupActionTemplate), which changes the action count during a run.
		//
		//   Note 2: m_threadActionIndices[threadIndex] cannot be stored in a reference because it can change since actions can dynamically create
		//           new threads and alter m_threadActionIndices during execution.  (I found this out the hard way!)
		for (; m_threadActionIndices[threadIndex] < static_cast<int>(actions->size()); ++(m_threadActionIndices[threadIndex]))
		{
			// get the action
			PlaybackAction *const action = (*actions)[static_cast<size_t>(m_threadActionIndices[threadIndex])];
			NOT_NULL(action);

			// update (run) the action
			const bool actionStalled = action->update(deltaTime, *this);

			//-- Immediately handle an abort occurring during action processing.
			if (m_abortRequested)
				return false;

			// stall the thread if action stalled
			if (!actionStalled)
			{
				// This action has completed, move on to other actions in the thread.

				//-- Clear the accumulated time time (used by lock detection) for the new action.
				m_threadUpdateTimes[threadIndex] = 0.0f;
			}
			else
			{
				// This thread is still active because the current action in the thread requires more time to complete.
				// Do not play any other actions in the thread until the current action completes on another update run.

				//-- Check if the current action has locked up.
				float &actionUpdateTime = m_threadUpdateTimes[threadIndex];

				actionUpdateTime += deltaTime;
				const bool actionIsHanging = (actionUpdateTime > action->getMaxReasonableUpdateTime());

				if (!actionIsHanging)
				{
					//-- Stop processing this thread: we've got a thread that is still active and it's not locked up.
					++activeThreadCount;
					break;
				}
				else
				{
					// Clear the accumulated time for the new forced-play action.
					m_threadUpdateTimes[threadIndex] = 0.0f;
					DEBUG_REPORT_LOG(s_logPlaybackScriptTimeouts, ("PS::update(): dropping apparently hanging thread [%u], action [%u] from [%s].\n", threadIndex, m_threadActionIndices[threadIndex], getPlaybackScriptTemplateName()));
				}
			}
		}
	}

	//-- keep running if there are any active threads remaining
	return (activeThreadCount > 0);
}

// ----------------------------------------------------------------------
/**
 * Return the number of actors supported by this script.
 *
 * The script may have been started with "missing" actors, or may have
 * had actors removed during playback.  This function returns the total
 * number of actors that could be in the script.  Some of these actors
 * currently may be NULL.
 *
 * @return  the total number of actors that could be in this script.
 */

int PlaybackScript::getSupportedActorCount() const
{
	if(m_playbackScriptTemplate)
		return m_playbackScriptTemplate->getSupportedActorCount();
	return 0;
}

// ----------------------------------------------------------------------
/**
 * Retrieve the specified actor object.
 *
 * An actor is an object playing a particular role in a script.
 *
 * It is possible for a given actor to be "absent" such that the
 * script continues playing without that actor.  In this case, the
 * actor returned from this fuction will be NULL.
 *
 * @param actorIndex  the 0-based index of the actor to retrieve from the
 *                    script.  Valid range is 0 .. getSupportedActorCount() - 1.
 */

const Object *PlaybackScript::getActor(int actorIndex) const
{
	if ((actorIndex < 0) || (actorIndex >= static_cast<int>(m_actors.size())))
	{
		DEBUG_WARNING(s_verbosePlaybackScriptWarnings, ("PlaybackScript::getActor(%d) called for [%s], actor index is out of range [0..%d)", 
			actorIndex, getPlaybackScriptTemplateName(), static_cast<int>(m_actors.size())));
		return 0;
	}

	// retrieve the actor
	return m_actors[static_cast<size_t>(actorIndex)];
}

// ----------------------------------------------------------------------
/**
 * Retrieve the specified actor object.
 *
 * An actor is an object playing a particular role in a script.
 *
 * It is possible for a given actor to be "absent" such that the
 * script continues playing without that actor.  In this case, the
 * actor returned from this fuction will be NULL.
 *
 * @param actorIndex  the 0-based index of the actor to retrieve from the
 *                    script.  Valid range is 0 .. getSupportedActorCount() - 1.
 */

Object *PlaybackScript::getActor(int actorIndex)
{
	if ((actorIndex < 0) || (actorIndex >= static_cast<int>(m_actors.size())))
	{
		DEBUG_WARNING(s_verbosePlaybackScriptWarnings, ("getActor(%d) called for [%s], actor index is out of range [0..%d)", 
			actorIndex, getPlaybackScriptTemplateName(), static_cast<int>(m_actors.size())));
		return 0;
	}

	// retrieve the actor
	return m_actors[static_cast<size_t>(actorIndex)];
}

// ----------------------------------------------------------------------

PlaybackScript::ActorActivity PlaybackScript::getActorActivity(int actorIndex) const
{
	return static_cast<PlaybackScript::ActorActivity>(m_playbackScriptTemplate->getActorActivity(actorIndex));
}

// ----------------------------------------------------------------------

bool PlaybackScript::isActorActive(int actorIndex) const
{
	ActorActivity const actorActivity = getActorActivity(actorIndex);

	bool isActive;

	switch (actorActivity)
	{
		case AA_activeAlways:
			//-- Actor is active as long as the actor is present on the client.
			isActive = (getActor(actorIndex) != NULL);
			break;

		case AA_activeIfPostureChange:
			{
				//-- Actor is active if present on the client and if there is a posture change scheduled for the actor.
				if (getActor(actorIndex) == NULL)
					isActive = false;
				else
				{
					Tag const targetTag = (actorIndex == 0) ? TAG_AEPS : TAG_DEPS;

					int postureChangeValue = -1;
					bool const hasVariable = getIntVariable(targetTag, postureChangeValue);

					//-- The actor is active if the variable exists and is >= 0.
					isActive = (hasVariable && (postureChangeValue >= 0));
				}
			}
			break;

		case AA_notActive:
		default:
			isActive = false;
	}

	return isActive;
}

// ----------------------------------------------------------------------
/**
 * Set a string value for a variable supported by the PlaybackScript.
 *
 * @param tag    the string variable name.
 * @param value  the new value for the variable.
 *
 * @return  true if the variable is supported by the PlaybackScript and set,
 *          false otherwise.
 */

bool PlaybackScript::setStringVariable(const Tag &tag, const std::string &value)
{
	//-- ensure the tag is already present.  we only accept setting a string
	//   variable already known to the PlaybackScriptTemplate.
	TagStringMap::iterator lowerBoundResult = m_localStringVariables.lower_bound(tag);
	if ((lowerBoundResult == m_localStringVariables.end()) || m_localStringVariables.key_comp()(tag, lowerBoundResult->first))
	{
		// variable not found, warn and exit
		char buffer[5];

		ConvertTagToString(tag, buffer);
		WARNING(s_logUndefinedVariableAccess, ("caller tried to set unknown string variable [%s] on PlaybackScript from [%s]\n", buffer, getPlaybackScriptTemplateName()));

		return false;
	}

	//-- replace the value of variable
	lowerBoundResult->second = value;

	// success
	return true;
}

// ----------------------------------------------------------------------
/**
 * Retrieve the value of a string variable set for this PlaybackScript.
 *
 * @param tag    the name of the string variable to retrieve.
 * @param value  the value of the variable is returned in this parameter.
 *
 * @return true if the variable was found and returned, false otherwise.
 */

bool PlaybackScript::getStringVariable(const Tag &tag, std::string &value) const
{
	//-- find the key
	TagStringMap::const_iterator lowerBoundResult = m_localStringVariables.lower_bound(tag);
	if ((lowerBoundResult == m_localStringVariables.end()) || m_localStringVariables.key_comp()(tag, lowerBoundResult->first))
	{
		// variable not found, warn and exit
		char buffer[5];

		ConvertTagToString(tag, buffer);
		WARNING(s_logUndefinedVariableAccess, ("caller tried to get unknown string variable [%s] on PlaybackScript from [%s]\n", buffer, getPlaybackScriptTemplateName()));

		return false;
	}

	//-- return the value
	value = lowerBoundResult->second;

	// success
	return true;
}

// ----------------------------------------------------------------------
/**
 * Set a float value for a variable supported by the PlaybackScript.
 *
 * @param tag    the float variable name.
 * @param value  the new value for the variable.
 *
 * @return  true if the variable is supported by the PlaybackScript and set,
 *          false otherwise.
 */

bool PlaybackScript::setFloatVariable(const Tag &tag, float value)
{
	//-- ensure the tag is already present.  we only accept setting a float
	//   variable already known to the PlaybackScriptTemplate.
	TagFloatMap::iterator lowerBoundResult = m_localFloatVariables.lower_bound(tag);
	if ((lowerBoundResult == m_localFloatVariables.end()) || m_localFloatVariables.key_comp()(tag, lowerBoundResult->first))
	{
		// variable not found, warn and exit
		char buffer[5];

		ConvertTagToString(tag, buffer);
		WARNING(s_logUndefinedVariableAccess, ("caller tried to set unknown float variable [%s] on PlaybackScript from [%s]\n", buffer, getPlaybackScriptTemplateName()));

		return false;
	}

	//-- replace the value of variable
	lowerBoundResult->second = value;

	// success
	return true;
}

// ----------------------------------------------------------------------
/**
 * Retrieve the value of a float variable set for this PlaybackScript.
 *
 * @param tag    the name of the float variable to retrieve.
 * @param value  the value of the variable is returned in this parameter.
 *
 * @return true if the variable was found and returned, false otherwise.
 */

bool PlaybackScript::getFloatVariable(const Tag &tag, float &value) const
{
	//-- find the key
	TagFloatMap::const_iterator lowerBoundResult = m_localFloatVariables.lower_bound(tag);
	if ((lowerBoundResult == m_localFloatVariables.end()) || m_localFloatVariables.key_comp()(tag, lowerBoundResult->first))
	{
		// variable not found, warn and exit
		char buffer[5];

		ConvertTagToString(tag, buffer);
		WARNING(s_logUndefinedVariableAccess, ("caller tried to get unknown float variable [%s] on PlaybackScript from [%s]\n", buffer, getPlaybackScriptTemplateName()));

		return false;
	}

	//-- return the value
	value = lowerBoundResult->second;

	// success
	return true;
}

// ----------------------------------------------------------------------
/**
 * Retrieve the value of a float variable set for this PlaybackScript.
 *
 * @param tag    the name of the float variable to retrieve.
 * @param value  the value of the variable is returned in this parameter.
 *
 * @return true if the variable was found and returned, false otherwise.
 */

bool PlaybackScript::getIntVariable(const Tag &tag, int &value) const
{
	//-- find the key
	TagIntMap::const_iterator lowerBoundResult = m_localIntVariables.lower_bound(tag);
	if ((lowerBoundResult == m_localIntVariables.end()) || m_localIntVariables.key_comp()(tag, lowerBoundResult->first))
	{
		// variable not found, warn and exit
		char buffer[5];

		ConvertTagToString(tag, buffer);
		WARNING(s_logUndefinedVariableAccess, ("caller tried to get unknown int variable [%s] on PlaybackScript from [%s]\n", buffer, getPlaybackScriptTemplateName()));

		return false;
	}

	//-- return the value
	value = lowerBoundResult->second;

	// success
	return true;
}

// ----------------------------------------------------------------------
/**
 * Set an int value for a variable supported by the PlaybackScript.
 *
 * @param tag    the int variable name.
 * @param value  the new value for the variable.
 *
 * @return  true if the variable is supported by the PlaybackScript and set,
 *          false otherwise.
 */

bool PlaybackScript::setIntVariable(const Tag &tag, int value, bool allowCreate)
{
	//-- ensure the tag is already present.  we only accept setting a int
	//   variable already known to the PlaybackScriptTemplate.
	TagIntMap::iterator lowerBoundResult = m_localIntVariables.lower_bound(tag);
	if ((lowerBoundResult == m_localIntVariables.end()) || m_localIntVariables.key_comp()(tag, lowerBoundResult->first))
	{
		if (allowCreate)
		{
			IGNORE_RETURN(m_localIntVariables.insert(TagIntMap::value_type(tag, value)));
			return true;
		}
		else
		{
			// variable not found, warn and exit
			char buffer[5];

			ConvertTagToString(tag, buffer);
			WARNING(s_logUndefinedVariableAccess, ("caller tried to set unknown int variable [%s] on PlaybackScript from [%s]\n", buffer, getPlaybackScriptTemplateName()));

			return false;
		}
	}

	//-- replace the value of variable
	lowerBoundResult->second = value;

	// success
	return true;
}

// ----------------------------------------------------------------------
/**
 * Store a watcher to the specified Object associated with the given
 * tag.
 *
 * Note the *ObjectVariable functions operate differently than the other
 * *Variable functions.  The other style of variable only supports
 * setting and getting of a specific .pst-defined set of variables.
 * The *ObjectVariable functions allow the caller to store any
 * tagged variable the caller chooses.  Also, there are no defaults
 * for the object variables, while there are default values defined
 * in the .pst files for the other type of variables.
 *
 * If object is null, the function call is ignored.
 *
 * The variable stored is a Watcher to the object.  If the given Object
 * is destroyed sometime before a getObjectVariable() call is made,
 * the latter will return a NULL Object instance.
 *
 * @param tag     the name of the variable.
 * @param object  the Object instance to associate with the given variable.
 */

void PlaybackScript::setObjectVariable(const Tag &tag, Object *object)
{
	//-- Ignore if object is NULL.
	if (!object)
		return;

	//-- Modify or create variable entry.
	ObjectWatcherMap::iterator lowerBoundResult = m_objectVariables.lower_bound(tag);
	if ((lowerBoundResult == m_objectVariables.end()) || m_objectVariables.key_comp()(tag, lowerBoundResult->first))
	{
		//-- Add new entry.
		IGNORE_RETURN(m_objectVariables.insert(lowerBoundResult, ObjectWatcherMap::value_type(tag, Watcher<Object>(object))));
	}
	else
	{
		//-- Change existing entry.
		lowerBoundResult->second = object;
	}
}

// ----------------------------------------------------------------------

Object *PlaybackScript::getObjectVariable(const Tag &tag) const
{
	//-- Find the entry.
	ObjectWatcherMap::const_iterator findIt = m_objectVariables.find(tag);

	if (findIt != m_objectVariables.end())
		return findIt->second;
	else
		return NULL;
}

// ----------------------------------------------------------------------

int PlaybackScript::getThreadCount() const
{
	return static_cast<int>(m_threads.size());
}

// ----------------------------------------------------------------------
/**
 * Create a new thread of execution for the PlaybackScript and return
 * the new thread's index.
 *
 * @return  the index for the new thread.  This index is used as the
 *          threadIndex parameter for the getAction()/setAction() calls.
 */

int PlaybackScript::addThread()
{
	//-- Create a new thread.
	m_threads.push_back(new PlaybackActionVector());

	//-- Create a new thread action index (i.e. thread context, index of the head action to execute).
	m_threadActionIndices.push_back(0);
	m_threadUpdateTimes.push_back(0.0f);

	return static_cast<int>(m_threads.size()) - 1;
}

// ----------------------------------------------------------------------

int PlaybackScript::getActionCount(int threadIndex) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, threadIndex, getThreadCount());

	PlaybackActionVector *const actions = m_threads[static_cast<size_t>(threadIndex)];
	return static_cast<int>(actions->size());
}

// ----------------------------------------------------------------------

PlaybackAction *PlaybackScript::getAction(int threadIndex, int actionIndex)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, threadIndex, getThreadCount());

	PlaybackActionVector *const actions = m_threads[static_cast<size_t>(threadIndex)];
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, actionIndex, static_cast<int>(actions->size()));

	return (*actions)[static_cast<size_t>(actionIndex)];
}

// ----------------------------------------------------------------------

void PlaybackScript::addAction(int threadIndex, PlaybackAction *action)
{
	//-- Skip null actions.
	NOT_NULL(action);

	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, threadIndex, getThreadCount());

	PlaybackActionVector *const actions = m_threads[static_cast<size_t>(threadIndex)];
	actions->push_back(action);

	//-- Call initial notifications for the script.
	sendInitialNotificationsToAction(*action);
}

// ----------------------------------------------------------------------
/**
 * Call the virtual PlaybackAction::stop() function on the specified action
 * and prevent the playback action from getting any more update() calls.
 *
 * This function will move the thread action index past this action if it
 * currently is at or before this action index.  This means that a call to
 * this function on a thread that is currently playing before this action
 * will skip actions when the active action index is set past the specified
 * actionIndex.
 *
 * This function will ignore the call if the action already has completed
 * via returning false in PlaybackAction::update().
 *
 * @param threadIndex  index of the thread containing the action to stop.
 * @param actionIndex  index of the action to stop.
 */

void PlaybackScript::stopAction(int threadIndex, int actionIndex)
{
	//-- Get the action.
	PlaybackAction *const action = getAction(threadIndex, actionIndex);

	if (m_threadActionIndices[static_cast<IntVector::size_type>(threadIndex)] <= actionIndex)
	{
		//-- Tell action to stop.
		action->stop(*this);

		//-- Ensure the thread's current action is past the specified action.
		m_threadActionIndices[static_cast<IntVector::size_type>(threadIndex)] = actionIndex + 1;
	}
}

// ----------------------------------------------------------------------

#ifdef _DEBUG

void PlaybackScript::debugDumpVariables() const
{
	char variableName[5];

	//-- print out strings
	{
		typedef TagStringMap  MapType;

		const MapType &variables = m_localStringVariables;
		REPORT_LOG(true, ("PlaybackScript string variables [%u]:\n", variables.size()));
		const MapType::const_iterator endIt = variables.end();
		for (MapType::const_iterator it = variables.begin(); it != endIt; ++it)
		{
			ConvertTagToString(it->first, variableName);
			REPORT_LOG(true, ("  [%s] = [%s]\n", variableName, it->second.c_str()));
		}
	}

	//-- print out floats
	{
		typedef TagFloatMap  MapType;

		const MapType &variables = m_localFloatVariables;
		REPORT_LOG(true, ("PlaybackScript float variables [%u]:\n", variables.size()));
		const MapType::const_iterator endIt = variables.end();
		for (MapType::const_iterator it = variables.begin(); it != endIt; ++it)
		{
			ConvertTagToString(it->first, variableName);
			REPORT_LOG(true, ("  [%s] = %g\n", variableName, it->second));
		}
	}

	//-- print out ints
	{
		typedef TagIntMap  MapType;

		const MapType &variables = m_localIntVariables;
		REPORT_LOG(true, ("PlaybackScript int variables [%u]:\n", variables.size()));
		const MapType::const_iterator endIt = variables.end();
		for (MapType::const_iterator it = variables.begin(); it != endIt; ++it)
		{
			ConvertTagToString(it->first, variableName);
			REPORT_LOG(true, ("  [%s] = %d\n", variableName, it->second));
		}
	}
}

#endif

// ----------------------------------------------------------------------

const char *PlaybackScript::getPlaybackScriptTemplateName() const
{
	if (!m_playbackScriptTemplate)
		return "<no playback script template>";
	else
		return m_playbackScriptTemplate->getName().getString();
}

// ======================================================================
// class PlaybackScript: PRIVATE STATIC
// ======================================================================

void PlaybackScript::remove()
{
	DEBUG_FATAL(!s_installed, ("PlaybackScript not installed!"));
	s_installed = false;

	DebugFlags::unregisterFlag(s_verbosePlaybackScriptWarnings);

	removeMemoryBlockManager();
}

// ======================================================================
// class PlaybackScript: PRIVATE
// ======================================================================

PlaybackScript::PlaybackScript(const PlaybackScriptTemplate *playbackScriptTemplate, int priority, const ObjectVector &actors)
:	m_playbackScriptTemplate(NON_NULL(playbackScriptTemplate)),
	m_actors(actors.size()),
	m_threads(static_cast<ThreadVector::size_type>(m_playbackScriptTemplate->getThreadCount())),
	m_threadActionIndices(static_cast<IntVector::size_type>(playbackScriptTemplate->getThreadCount())),
	m_threadUpdateTimes(static_cast<FloatVector::size_type>(playbackScriptTemplate->getThreadCount())),
	m_localStringVariables(playbackScriptTemplate->getInitialStringVariables()),
	m_localFloatVariables(playbackScriptTemplate->getInitialFloatVariables()),
	m_localIntVariables(playbackScriptTemplate->getInitialIntVariables()),
	m_objectVariables(),
	m_abortRequested(false),
	m_priority(priority)
{
	DEBUG_FATAL(!s_installed, ("PlaybackScript not installed!"));

	// NOTE: we don't use a reference to m_playbackScriptTemplate here
	//       because we will release() the reference
	//       in the destructor.  If we used a reference, the reference would
	//       be an alias for a NULL object (bad).

	//-- keep a reference to the template
	m_playbackScriptTemplate->fetch();

	//-- create a watcher for each non-null actor
	{
		const size_t actorCount = actors.size();
		for (size_t i = 0; i < actorCount; ++i)
			m_actors[i] = actors[i];
	}

	//-- create threads
	{
		const int threadCount = m_playbackScriptTemplate->getThreadCount();

		for (int i = 0; i < threadCount; ++i)
			m_threads[static_cast<ThreadVector::size_type>(i)] = m_playbackScriptTemplate->createThreadActions(i);
	}

	//-- notify all actions about the actors added to this script
	sendInitialNotifications();
}

// ----------------------------------------------------------------------

void PlaybackScript::sendInitialNotifications()
{
	//-- for each actor present, notify each trigger and action about the actor's presence
	const int actorCount = static_cast<int>(m_actors.size());
	for (int actorIndex = 0; actorIndex < actorCount; ++actorIndex)
	{
		// check if this actor is present
		Object *object = m_actors[static_cast<size_t>(actorIndex)];
		if (!object)
			continue;

		// notify addition of actor to all threads
		const ThreadVector::iterator threadEndIt = m_threads.end();
		for (ThreadVector::iterator threadIt = m_threads.begin(); threadIt != threadEndIt; ++threadIt)
		{
			// notify all actions in this thread
			PlaybackActionVector *const actions = *threadIt;
			NOT_NULL(actions);

			const PlaybackActionVector::iterator endIt = actions->end();
			for (PlaybackActionVector::iterator it = actions->begin(); it != endIt; ++it)
			{
				NOT_NULL(*it);
				(*it)->notifyActorAdded(*this, actorIndex);
			}
		}
	}
}

// ----------------------------------------------------------------------

void PlaybackScript::sendInitialNotificationsToAction(PlaybackAction &action)
{
	//-- for each actor present, notify each trigger and action about the actor's presence
	const int actorCount = static_cast<int>(m_actors.size());
	for (int actorIndex = 0; actorIndex < actorCount; ++actorIndex)
	{
		// check if this actor is present
		Object *object = m_actors[static_cast<size_t>(actorIndex)];
		if (!object)
			continue;

		//-- Notify addition of actor.
		action.notifyActorAdded(*this, actorIndex);
	}
}

// ----------------------------------------------------------------------
/**
 * Remove an actor from the script.
 *
 * Use getSupportedActorCount() to find the total number of actors
 * available in the script.
 *
 * Internally, once an actor is removed from a script all triggers
 * that depend on the removed actor's existence will act as if signaled,
 * and all actions solely intended for the removed actor will be ignored.
 *
 * @param actorIndex  the 0-based index of the actor to remove from the
 *                    script.  Valid range is 0 .. getSupportedActorCount() - 1.
 */

void PlaybackScript::removeActor(int actorIndex)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, actorIndex, static_cast<int>(m_actors.size()));

	//-- get the actor
	Object *object = m_actors[static_cast<size_t>(actorIndex)];
	if (!object)
	{
		DEBUG_WARNING(s_verbosePlaybackScriptWarnings, ("tried to remove an actor that was already removed (index=%d)\n", actorIndex));
		return;
	}

	//-- tell all actions the actor is about to be removed
	const ThreadVector::iterator threadEndIt = m_threads.end();
	for (ThreadVector::iterator threadIt = m_threads.begin(); threadIt != threadEndIt; ++threadIt)
	{
		// notify all actions in this thread
		PlaybackActionVector *const actions = *threadIt;
		NOT_NULL(actions);

		const PlaybackActionVector::iterator endIt = actions->end();
		for (PlaybackActionVector::iterator it = actions->begin(); it != endIt; ++it)
		{
			NOT_NULL(*it);
			(*it)->notifyRemovingActor(*this, actorIndex);
		}
	}

	//-- remove the actor from the script by setting it to NULL.
	m_actors[static_cast<ObjectWatcherVector::size_type>(actorIndex)] = static_cast<Object*>(0);
}

// ----------------------------------------------------------------------

void PlaybackScript::callCleanupOnAllActions()
{
	//-- Tell all actions that the action is about to be deleted.
	const ThreadVector::iterator threadEndIt = m_threads.end();
	for (ThreadVector::iterator threadIt = m_threads.begin(); threadIt != threadEndIt; ++threadIt)
	{
		// notify all actions in this thread
		PlaybackActionVector *const actions = *threadIt;
		NOT_NULL(actions);

		const PlaybackActionVector::iterator endIt = actions->end();
		for (PlaybackActionVector::iterator it = actions->begin(); it != endIt; ++it)
		{
			NOT_NULL(*it);
			(*it)->cleanup(*this);
		}
	}
}

// ======================================================================
