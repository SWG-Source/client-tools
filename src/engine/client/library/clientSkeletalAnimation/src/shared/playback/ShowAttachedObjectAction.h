// PRIVATE

// ======================================================================
//
// ShowAttachedObjectAction.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ShowAttachedObjectAction_H
#define INCLUDED_ShowAttachedObjectAction_H

// ======================================================================

#include "clientAnimation/PlaybackAction.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"
#include "sharedObject/Object.h"

class MemoryBlockManager;
class ShowAttachedObjectActionTemplate;

// ======================================================================

class ShowAttachedObjectAction: public PlaybackAction
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	explicit ShowAttachedObjectAction(const ShowAttachedObjectActionTemplate &actionTemplate);

	virtual bool  update(float deltaTime, PlaybackScript &script);
	virtual void  cleanup(PlaybackScript &playbackScript);

	void          showObject(PlaybackScript &script);

private:

	const ShowAttachedObjectActionTemplate &getOurTemplate() const;

	// Disabled.
	ShowAttachedObjectAction();
	ShowAttachedObjectAction(const ShowAttachedObjectAction&);
	ShowAttachedObjectAction &operator =(const ShowAttachedObjectAction&);

private:

	bool  m_objectIsHidden;

};

// ======================================================================

#endif
