// ======================================================================
//
// PlaybackScriptTemplate.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_PlaybackScriptTemplate_H
#define INCLUDED_PlaybackScriptTemplate_H

// ======================================================================

// NOTE: I'm including more stuff in here so I don't have to do as many
//       memory allocations at construction time.  I think this is acceptable
//       because only a small amount of code includes this class.

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/CrcLowerString.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"

#include <vector>

class Iff;
class MemoryBlockManager;
class Object;
class PlaybackAction;
class PlaybackActionTemplate;
class PlaybackScript;

// ======================================================================

class PlaybackScriptTemplate
{
friend class PlaybackScriptTemplateList;

	MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

public:

	typedef stdvector<PlaybackAction*>::fwd  PlaybackActionVector;

	typedef stdvector<int>::fwd              IntVector;

	typedef stdmap<Tag, std::string>::fwd    TagStringMap;
	typedef stdmap<Tag, float>::fwd          TagFloatMap;
	typedef stdmap<Tag, int>::fwd            TagIntMap;

	enum ActorActivity
	{
		AA_notActive             = 0,
		AA_activeAlways          = 1,
		AA_activeIfPostureChange = 2
	};

public:

	typedef PlaybackActionTemplate   *(*ActionTemplateCreateFunction)(Iff &iff);

	typedef stdvector<Object*>::fwd  ObjectVector;

public:

	static void                      install();

	static void                      registerActionTemplate(const Tag &tag, ActionTemplateCreateFunction createFunction);
	static void                      deregisterActionTemplate(const Tag &tag);

public:

	const CrcLowerString            &getName() const;
	
	PlaybackScript                  *createPlaybackScript(int priority, const ObjectVector &actors) const;

	int                              getSupportedActorCount() const;
	ActorActivity                    getActorActivity(int actorIndex) const;

	int                              getThreadCount() const;
	PlaybackActionVector            *createThreadActions(int threadIndex) const;

	const TagStringMap              &getInitialStringVariables() const;
	const TagFloatMap               &getInitialFloatVariables() const;
	const TagIntMap                 &getInitialIntVariables() const;

	void                             fetch() const;
	void                             release() const;

private:

	typedef stdmap<Tag, ActionTemplateCreateFunction>::fwd   ActionTemplateCreationMap;
	typedef stdvector<ActorActivity>::fwd                    ActorActivityVector;
	typedef stdvector<PlaybackActionTemplate*>::fwd          PlaybackActionTemplateVector;
	typedef stdvector<PlaybackActionTemplateVector*>::fwd    ThreadVector;

private:

	static void                      remove();

private:

	PlaybackScriptTemplate(Iff &iff, const CrcLowerString &name);
	~PlaybackScriptTemplate();

	void                             load_0001(Iff &iff);
	void                             load_0002(Iff &iff);

	// disabled
	PlaybackScriptTemplate();
	PlaybackScriptTemplate(const PlaybackScriptTemplate&);
	PlaybackScriptTemplate &operator =(const PlaybackScriptTemplate&);

private:

	static bool                        ms_installed;
	static ActionTemplateCreationMap   ms_actionTemplateCreationMap;

private:

	CrcLowerString                     m_name;
	mutable int                        m_referenceCount;

	int                                m_supportedActorCount;
	ActorActivityVector                m_actorActivityVector;
	ThreadVector                      *m_threads;
	TagStringMap                      *m_stringVariables;
	TagFloatMap                       *m_floatVariables;
	TagIntMap                         *m_intVariables;
};

// ======================================================================

#endif
