// ======================================================================
//
// PlaybackScriptTemplate.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientAnimation/FirstClientAnimation.h"
#include "clientAnimation/PlaybackScriptTemplate.h"

#include "clientAnimation/PlaybackActionTemplate.h"
#include "clientAnimation/PlaybackScript.h"
#include "clientAnimation/PlaybackScriptTemplateList.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/VoidMemberFunction.h"
#include "sharedObject/Object.h"

#include <algorithm>
#include <functional>
#include <map>
#include <string>

// ======================================================================

namespace PlaybackScriptTemplateNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	const Tag TAG_FLOT = TAG(F,L,O,T);
	const Tag TAG_INT  = TAG3(I,N,T);
	const Tag TAG_PBSC = TAG(P,B,S,C);
	const Tag TAG_STRN = TAG(S,T,R,N);
	const Tag TAG_THRD = TAG(T,H,R,D);
	const Tag TAG_THDS = TAG(T,H,D,S);
	const Tag TAG_VAR  = TAG3(V,A,R);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	const PlaybackScriptTemplate::TagFloatMap   s_emptyTagFloatMap;
	const PlaybackScriptTemplate::TagIntMap     s_emptyTagIntMap;
	const PlaybackScriptTemplate::TagStringMap  s_emptyTagStringMap;
}

using namespace PlaybackScriptTemplateNamespace;

// ======================================================================
// private static member variables
// ======================================================================

bool                                                PlaybackScriptTemplate::ms_installed;
PlaybackScriptTemplate::ActionTemplateCreationMap   PlaybackScriptTemplate::ms_actionTemplateCreationMap;

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL(PlaybackScriptTemplate, true, 0, 0, 0);

// ======================================================================
// public static member functions
// ======================================================================

void PlaybackScriptTemplate::install()
{
	InstallTimer const installTimer("PlaybackScriptTemplate::install");

	DEBUG_FATAL(ms_installed, ("PlaybackScriptTemplate already installed"));

	ms_actionTemplateCreationMap.clear();
	installMemoryBlockManager();

	ms_installed = true;
	ExitChain::add(remove, "PlaybackScriptTemplate");
}

// ----------------------------------------------------------------------

void PlaybackScriptTemplate::registerActionTemplate(const Tag &tag, ActionTemplateCreateFunction createFunction)
{
	DEBUG_FATAL(!ms_installed, ("PlaybackScriptTemplate not installed"));

	//-- find insertion point
	ActionTemplateCreationMap::iterator lowerBoundResult = ms_actionTemplateCreationMap.lower_bound(tag);
	if ((lowerBoundResult != ms_actionTemplateCreationMap.end()) && !ms_actionTemplateCreationMap.key_comp()(tag, lowerBoundResult->first))
	{
		// tag already bound
		DEBUG_FATAL(true, ("tried to register PlaybackActionTemplate for existing tag\n"));
		return; //lint !e527 // unreachable // only in debug build
	}

	//-- insert creation function into map
	IGNORE_RETURN(ms_actionTemplateCreationMap.insert(lowerBoundResult, ActionTemplateCreationMap::value_type(tag, createFunction)));
}

// ----------------------------------------------------------------------

void PlaybackScriptTemplate::deregisterActionTemplate(const Tag &tag)
{
	DEBUG_FATAL(!ms_installed, ("PlaybackScriptTemplate not installed"));

	ActionTemplateCreationMap::iterator findIt = ms_actionTemplateCreationMap.find(tag);
	DEBUG_WARNING(findIt == ms_actionTemplateCreationMap.end(), ("no action templates registered for given tag\n"));

	if (findIt != ms_actionTemplateCreationMap.end())
		ms_actionTemplateCreationMap.erase(findIt);
}

// ======================================================================
// public member functions
// ======================================================================

const CrcLowerString &PlaybackScriptTemplate::getName() const
{
	return m_name;
}

// ----------------------------------------------------------------------

PlaybackScript *PlaybackScriptTemplate::createPlaybackScript(int priority, const ObjectVector &actors) const
{
	return new PlaybackScript(this, priority, actors);
}

// ----------------------------------------------------------------------

int PlaybackScriptTemplate::getSupportedActorCount() const
{
	return m_supportedActorCount;
}

// ----------------------------------------------------------------------
/**
 * Retrieve whether a supported actor is potentially active.
 *
 * Active means that the actor is controlled in some way such that it is
 * wrong for another script to try to control the actor at the same time.
 * For example, one script trying to rotate an actor one way while another
 * script tries to rotate the actor another way would be a bad thing.
 * Possibly rotating an actor in a script would definitely mean that actor
 * is potentially active.  On the other hand, if a script only uses an actor
 * for the actor's position information, such an actor is really passive,
 * and will always be passive in the script.  It is totally fine if another
 * script is actively controlling that actor at the same time as the first
 * inactive script is playing.
 *
 * This information is useful because an actor that is not possibly active
 * in a particular script should never be processed as part of the
 * script priority mechanism.  In other words, if a particular actor is
 * playing in a particular script but is never active in that script,
 * then the inactive script should never stop another script from being played.
 *
 * @param actorIndex  must fall within range 0 .. (getSupportedActorCount() - 1).
 *
 * @return  true if the actor object is potentially active; false if there is
 *          no way for the actor to be active in this script.
 */

PlaybackScriptTemplate::ActorActivity PlaybackScriptTemplate::getActorActivity(int actorIndex) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, actorIndex, m_supportedActorCount);
	return m_actorActivityVector[static_cast<ActorActivityVector::size_type>(actorIndex)];
}

// ----------------------------------------------------------------------

int PlaybackScriptTemplate::getThreadCount() const
{
	return static_cast<int>(m_threads->size());
}

// ----------------------------------------------------------------------

PlaybackScriptTemplate::PlaybackActionVector *PlaybackScriptTemplate::createThreadActions(int threadIndex) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, threadIndex, getThreadCount());

	//-- get the action templates for this thread
	const PlaybackActionTemplateVector *const actionTemplates = (*m_threads)[static_cast<size_t>(threadIndex)];
	NOT_NULL(actionTemplates);

	//-- create new action container
	PlaybackActionVector *const actions = new PlaybackActionVector(actionTemplates->size());

	//-- populate container with new actions created from the action templates
	std::transform(actionTemplates->begin(), actionTemplates->end(), actions->begin(), std::mem_fun(&PlaybackActionTemplate::createPlaybackAction));

	//-- return it
	return actions;
}

// ----------------------------------------------------------------------
/**
 * Return the supported string variables and the script defaults for
 * the template.
 *
 * @return  tag-string map containing the supported string variables
 *          as the map keys and the default values as the map values.
 *          This may return NULL if no variables of this type are
 *          supported.
 */

const PlaybackScriptTemplate::TagStringMap &PlaybackScriptTemplate::getInitialStringVariables() const
{
	if (m_stringVariables)
		return *m_stringVariables;
	else
		return s_emptyTagStringMap;
}

// ----------------------------------------------------------------------
/**
 * Return the supported float variables and the script defaults for
 * the template.
 *
 * @return  tag-float map containing the supported float variables
 *          as the map keys and the default values as the map values.
 *          This may return NULL if no variables of this type are
 *          supported.
 */

const PlaybackScriptTemplate::TagFloatMap &PlaybackScriptTemplate::getInitialFloatVariables() const
{
	if (m_floatVariables)
		return *m_floatVariables;
	else
		return s_emptyTagFloatMap;
}

// ----------------------------------------------------------------------
/**
 * Return the supported int variables and the script defaults for
 * the template.
 *
 * @return  tag-int map containing the supported int variables
 *          as the map keys and the default values as the map values.
 *          This may return NULL if no variables of this type are
 *          supported.
 */

const PlaybackScriptTemplate::TagIntMap &PlaybackScriptTemplate::getInitialIntVariables() const
{
	if (m_intVariables)
		return *m_intVariables;
	else
		return s_emptyTagIntMap;
}

// ----------------------------------------------------------------------

void PlaybackScriptTemplate::fetch() const
{
	++m_referenceCount;
}

// ----------------------------------------------------------------------

void PlaybackScriptTemplate::release() const
{
	--m_referenceCount;
	if (m_referenceCount < 1)
	{
		// time to delete

		//-- ensure list is no longer tracking us
		PlaybackScriptTemplateList::stopTracking(this);

		//-- delete this instance
		delete const_cast<PlaybackScriptTemplate*>(this);
	}
}

// ======================================================================
// private static member functions
// ======================================================================

void PlaybackScriptTemplate::remove()
{
	DEBUG_FATAL(!ms_installed, ("PlaybackScriptTemplate not installed"));

	removeMemoryBlockManager();
	ms_actionTemplateCreationMap.clear();

	ms_installed = false;
}

// ======================================================================
// private static member functions
// ======================================================================

PlaybackScriptTemplate::PlaybackScriptTemplate(Iff &iff, const CrcLowerString &name)
:	m_name(name),
	m_referenceCount(0),
	m_supportedActorCount(0),
	m_actorActivityVector(),
	m_threads(new ThreadVector()),
	m_stringVariables(0),
	m_floatVariables(0),
	m_intVariables(0)
{
	iff.enterForm(TAG_PBSC);

		const Tag version = iff.getCurrentName();
		switch (version)
		{
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
					DEBUG_FATAL(true, ("unsupported PlaybackScriptTemplate version [%s]", buffer));
				}
		}

	iff.exitForm(TAG_PBSC);
}

// ----------------------------------------------------------------------

PlaybackScriptTemplate::~PlaybackScriptTemplate()
{
	DEBUG_WARNING(m_referenceCount, ("non-zero reference count, bad reference handling (%d)\n", m_referenceCount));

	// Release all action templates in every thread.
	const ThreadVector::iterator endIt = m_threads->end();
	for (ThreadVector::iterator it = m_threads->begin(); it != endIt; ++it)
	{
		std::for_each((*it)->begin(), (*it)->end(), VoidMemberFunction(&PlaybackActionTemplate::release));
	}

	// delete the threads
	std::for_each(m_threads->begin(), m_threads->end(), PointerDeleter());
	delete m_threads;

	delete m_intVariables;
	delete m_floatVariables;
	delete m_stringVariables;
}

// ----------------------------------------------------------------------

void PlaybackScriptTemplate::load_0001(Iff &iff)
{
	iff.enterForm(TAG_0001);

		// load general script info
		iff.enterChunk(TAG_INFO);
		{
			m_supportedActorCount = static_cast<int>(iff.read_int16());
			DEBUG_FATAL(m_supportedActorCount < 1, ("invalid m_supportedActorCount %d", m_supportedActorCount));

			//-- Default all to true.
			m_actorActivityVector.resize(static_cast<ActorActivityVector::size_type>(m_supportedActorCount));
			for (int i = 0; i < m_supportedActorCount; ++i)
				m_actorActivityVector[static_cast<ActorActivityVector::size_type>(i)] = AA_activeAlways;
		}
		iff.exitChunk(TAG_INFO);

		// load variables available for script, along with default values
		iff.enterForm(TAG_VAR);
		{
			// handle string variables
			if (iff.enterChunk(TAG_STRN, true))
			{
				IS_NULL(m_stringVariables);
				m_stringVariables = new TagStringMap;

				char  defaultValue[1024];

				while (iff.getChunkLengthLeft())
				{
					// load variable tag
					const Tag variableName = Tag(iff.read_uint32());

					// load default value
					iff.read_string(defaultValue, sizeof(defaultValue) - 1);

					// add to variable map
					IGNORE_RETURN(m_stringVariables->insert(TagStringMap::value_type(variableName, std::string(defaultValue))));
				}

				iff.exitChunk(TAG_STRN);
			}

			// handle float variables
			if (iff.enterChunk(TAG_FLOT, true))
			{
				IS_NULL(m_floatVariables);
				m_floatVariables = new TagFloatMap;

				while (iff.getChunkLengthLeft())
				{
					// load variable tag
					const Tag variableName = Tag(iff.read_uint32());

					// load default value
					const float defaultValue = iff.read_float();

					// add to variable map
					IGNORE_RETURN(m_floatVariables->insert(TagFloatMap::value_type(variableName, defaultValue)));
				}

				iff.exitChunk(TAG_FLOT);
			}

			// handle int variables
			if (iff.enterChunk(TAG_INT, true))
			{
				IS_NULL(m_intVariables);
				m_intVariables = new TagIntMap;

				while (iff.getChunkLengthLeft())
				{
					// load variable tag
					const Tag variableName = Tag(iff.read_uint32());

					// load default value
					const int defaultValue = static_cast<int>(iff.read_int32());

					// add to variable map
					IGNORE_RETURN(m_intVariables->insert(TagIntMap::value_type(variableName, defaultValue)));
				}

				iff.exitChunk(TAG_INT);
			}
		}
		iff.exitForm(TAG_VAR);

		// load all threads (sequences of action templates)
		iff.enterForm(TAG_THDS);
		{
			PlaybackActionTemplateVector  tempVector;

			while (!iff.atEndOfForm())
			{
				//-- load the thread
				iff.enterForm(TAG_THRD);
				{
					//-- load the thread's action templates into the temp vector
					tempVector.clear();

					while (!iff.atEndOfForm())
					{
						//-- load the action template
						// get the tag name
						const Tag typeTag(iff.getCurrentName());

						// look up action template's creation function
						const ActionTemplateCreationMap::iterator creationIt = ms_actionTemplateCreationMap.find(typeTag);
						if (creationIt == ms_actionTemplateCreationMap.end())
						{
							char buffer[5];

							ConvertTagToString(typeTag, buffer);
							FATAL(true, ("unknown action template type [%s]", buffer));
						}

						// create the action template
						const ActionTemplateCreateFunction createFunction = creationIt->second;
						NOT_NULL(createFunction);

						PlaybackActionTemplate *const actionTemplate = (*createFunction)(iff);

						// Fetch reference to action template.
						actionTemplate->fetch();

						tempVector.push_back(actionTemplate);
					}

					//-- add the thread's action templates to the thread list
					m_threads->push_back(new PlaybackActionTemplateVector(tempVector));
				}
				iff.exitForm(TAG_THRD);
			}
		}
		iff.exitForm(TAG_THDS);

	iff.exitForm(TAG_0001);
}

// ----------------------------------------------------------------------

void PlaybackScriptTemplate::load_0002(Iff &iff)
{
	iff.enterForm(TAG_0002);

		// load general script info
		iff.enterChunk(TAG_INFO);
		{
			m_supportedActorCount = static_cast<int>(iff.read_int16());
			DEBUG_FATAL(m_supportedActorCount < 1, ("invalid m_supportedActorCount %d", m_supportedActorCount));

			m_actorActivityVector.resize(static_cast<ActorActivityVector::size_type>(m_supportedActorCount));

			for (int i = 0; i < m_supportedActorCount; ++i)
				m_actorActivityVector[static_cast<ActorActivityVector::size_type>(i)] = static_cast<ActorActivity>(iff.read_uint8());
		}
		iff.exitChunk(TAG_INFO);

		// load variables available for script, along with default values
		iff.enterForm(TAG_VAR);
		{
			// handle string variables
			if (iff.enterChunk(TAG_STRN, true))
			{
				IS_NULL(m_stringVariables);
				m_stringVariables = new TagStringMap;

				char  defaultValue[1024];

				while (iff.getChunkLengthLeft())
				{
					// load variable tag
					const Tag variableName = Tag(iff.read_uint32());

					// load default value
					iff.read_string(defaultValue, sizeof(defaultValue) - 1);

					// add to variable map
					IGNORE_RETURN(m_stringVariables->insert(TagStringMap::value_type(variableName, std::string(defaultValue))));
				}

				iff.exitChunk(TAG_STRN);
			}

			// handle float variables
			if (iff.enterChunk(TAG_FLOT, true))
			{
				IS_NULL(m_floatVariables);
				m_floatVariables = new TagFloatMap;

				while (iff.getChunkLengthLeft())
				{
					// load variable tag
					const Tag variableName = Tag(iff.read_uint32());

					// load default value
					const float defaultValue = iff.read_float();

					// add to variable map
					IGNORE_RETURN(m_floatVariables->insert(TagFloatMap::value_type(variableName, defaultValue)));
				}

				iff.exitChunk(TAG_FLOT);
			}

			// handle int variables
			if (iff.enterChunk(TAG_INT, true))
			{
				IS_NULL(m_intVariables);
				m_intVariables = new TagIntMap;

				while (iff.getChunkLengthLeft())
				{
					// load variable tag
					const Tag variableName = Tag(iff.read_uint32());

					// load default value
					const int defaultValue = static_cast<int>(iff.read_int32());

					// add to variable map
					IGNORE_RETURN(m_intVariables->insert(TagIntMap::value_type(variableName, defaultValue)));
				}

				iff.exitChunk(TAG_INT);
			}
		}
		iff.exitForm(TAG_VAR);

		// load all threads (sequences of action templates)
		iff.enterForm(TAG_THDS);
		{
			PlaybackActionTemplateVector  tempVector;

			while (!iff.atEndOfForm())
			{
				//-- load the thread
				iff.enterForm(TAG_THRD);
				{
					//-- load the thread's action templates into the temp vector
					tempVector.clear();

					while (!iff.atEndOfForm())
					{
						//-- load the action template
						// get the tag name
						const Tag typeTag(iff.getCurrentName());

						// look up action template's creation function
						const ActionTemplateCreationMap::iterator creationIt = ms_actionTemplateCreationMap.find(typeTag);
						if (creationIt == ms_actionTemplateCreationMap.end())
						{
							char buffer[5];

							ConvertTagToString(typeTag, buffer);
							FATAL(true, ("unknown action template type [%s]", buffer));
						}

						// create the action template
						const ActionTemplateCreateFunction createFunction = creationIt->second;
						NOT_NULL(createFunction);

						PlaybackActionTemplate *const actionTemplate = (*createFunction)(iff);

						// Fetch reference to action template.
						actionTemplate->fetch();

						tempVector.push_back(actionTemplate);
					}

					//-- add the thread's action templates to the thread list
					m_threads->push_back(new PlaybackActionTemplateVector(tempVector));
				}
				iff.exitForm(TAG_THRD);
			}
		}
		iff.exitForm(TAG_THDS);

	iff.exitForm(TAG_0002);
}

// ======================================================================
