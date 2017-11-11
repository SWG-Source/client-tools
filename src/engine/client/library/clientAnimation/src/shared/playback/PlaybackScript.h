// ======================================================================
//
// PlaybackScript.h
// Copyright 2001 - 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDE_PlaybackScript_H
#define INCLUDE_PlaybackScript_H

// ======================================================================

// NOTE: I am including more stuff in the header because this class is
//       not included in much.  I think it is not worth making the extra
//       allocations to hide STL containers in this case.

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"
#include "sharedObject/Object.h"       // "`What!?!' you say?  You'll see nothing below that would lead you to expect this, but Object is required (via ~Watcher<Object> --- why is that needed for this declaration?).

#include <map>
#include <vector>

class MemoryBlockManager;
class Object;
class PlaybackAction;
class PlaybackScriptTemplate;

// ======================================================================

namespace PlaybackScriptNamespace
{
	extern bool s_verbosePlaybackScriptWarnings;
}

class PlaybackScript
{
friend class PlaybackScriptTemplate;

	MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

public:

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	/// STL-style comparator function to compare PlaybackScript instance pointers by priority value.
	struct LessPriorityComparator
	{
		bool operator()(const PlaybackScript *lhs, const PlaybackScript *rhs) const;
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	enum ActorActivity
	{
		AA_notActive             = 0,
		AA_activeAlways          = 1,
		AA_activeIfPostureChange = 2
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

public:

	static void     install();

public:

	~PlaybackScript();

	bool            update(float deltaTime);

	int             getSupportedActorCount() const;
	const Object   *getActor(int actorIndex) const;
	Object         *getActor(int actorIndex);

	ActorActivity   getActorActivity(int actorIndex) const;
	bool            isActorActive(int actorIndex) const;

	bool            setStringVariable(const Tag &tag, const std::string &value);
	bool            getStringVariable(const Tag &tag, std::string &value) const;

	bool            setFloatVariable(const Tag &tag, float value);
	bool            getFloatVariable(const Tag &tag, float &value) const;

	bool            setIntVariable(const Tag &tag, int value, bool allowCreate = false);
	bool            getIntVariable(const Tag &tag, int &value) const;

	void            setObjectVariable(const Tag &tag, Object *object);
	Object         *getObjectVariable(const Tag &tag) const;

	int             getThreadCount() const;
	int             addThread();

	int             getActionCount(int threadIndex) const;
	PlaybackAction *getAction(int threadIndex, int actionIndex);
	void            addAction(int threadIndex, PlaybackAction *action);
	void            stopAction(int threadIndex, int actionIndex);

	void            abortPlayback();
	bool            hasBeenAborted() const;

	int             getPriority() const;

#ifdef _DEBUG
	void            debugDumpVariables() const;
#endif

	const PlaybackScriptTemplate *getPlaybackScriptTemplate() const;
	const char                   *getPlaybackScriptTemplateName() const;

private:

	typedef stdvector<Object*>::fwd                ObjectVector;
	typedef stdvector<Watcher<Object> >::fwd       ObjectWatcherVector;

	typedef stdvector<int>::fwd                    IntVector;
	typedef stdvector<float>::fwd                  FloatVector;

	typedef stdvector<PlaybackAction*>::fwd        PlaybackActionVector;
	typedef stdvector<PlaybackActionVector*>::fwd  ThreadVector;

	typedef stdmap<Tag, std::string>::fwd          TagStringMap;
	typedef stdmap<Tag, float>::fwd                TagFloatMap;
	typedef stdmap<Tag, int>::fwd                  TagIntMap;
	typedef stdmap<Tag, Watcher<Object> >::fwd     ObjectWatcherMap;

private:

	static void     remove();

private:

	PlaybackScript(const PlaybackScriptTemplate *playbackScriptTemplate, int priority, const ObjectVector &actors);

	void  sendInitialNotifications();
	void  sendInitialNotificationsToAction(PlaybackAction &action);
	void  removeActor(int actorIndex);
	void 	callCleanupOnAllActions();

	// disabled
	PlaybackScript();
	PlaybackScript(const PlaybackScript&);
	PlaybackScript &operator =(const PlaybackScript&);

private:

	const PlaybackScriptTemplate *m_playbackScriptTemplate;

	ObjectWatcherVector           m_actors;

	ThreadVector                  m_threads;
	IntVector                     m_threadActionIndices;
	FloatVector                   m_threadUpdateTimes;
	
	TagStringMap                  m_localStringVariables;
	TagFloatMap                   m_localFloatVariables;
	TagIntMap                     m_localIntVariables;

	ObjectWatcherMap              m_objectVariables;

	bool                          m_abortRequested;

	int                           m_priority;
};

// ======================================================================

inline bool PlaybackScript::LessPriorityComparator::operator()(const PlaybackScript *lhs, const PlaybackScript *rhs) const
{
	NOT_NULL(lhs);
	NOT_NULL(rhs);
	return lhs->getPriority() < rhs->getPriority();
}

// ======================================================================

inline void PlaybackScript::abortPlayback()
{
	m_abortRequested = true;
}

// ----------------------------------------------------------------------

inline bool PlaybackScript::hasBeenAborted() const
{
	return m_abortRequested;
}

// ----------------------------------------------------------------------

inline int PlaybackScript::getPriority() const
{
	return m_priority;
}

// ----------------------------------------------------------------------

inline const PlaybackScriptTemplate *PlaybackScript::getPlaybackScriptTemplate() const
{
	return m_playbackScriptTemplate;
}

// ======================================================================

#endif
