// PRIVATE

// ======================================================================
//
// TemporaryAttachedObjectAction.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_TemporaryAttachedObjectAction_H
#define INCLUDED_TemporaryAttachedObjectAction_H

// ======================================================================

#include "clientAnimation/PlaybackAction.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"
#include "sharedObject/Object.h"

class MemoryBlockManager;
class TemporaryAttachedObjectActionTemplate;

// ======================================================================

class TemporaryAttachedObjectAction: public PlaybackAction
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	explicit TemporaryAttachedObjectAction(const TemporaryAttachedObjectActionTemplate &actionTemplate);

	virtual bool  update(float deltaTime, PlaybackScript &script);
	virtual void  cleanup(PlaybackScript &playbackScript);

	void          destroyObject(PlaybackScript &script);

private:

	const TemporaryAttachedObjectActionTemplate &getOurTemplate() const;

	// Disabled.
	TemporaryAttachedObjectAction();
	TemporaryAttachedObjectAction(const TemporaryAttachedObjectAction&);
	TemporaryAttachedObjectAction &operator =(const TemporaryAttachedObjectAction&);

private:

	Watcher<Object>  m_temporaryObject;

};

// ======================================================================

#endif
