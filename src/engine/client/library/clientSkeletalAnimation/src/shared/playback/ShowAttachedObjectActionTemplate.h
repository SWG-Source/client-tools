// PRIVATE

// ======================================================================
//
// ShowAttachedObjectActionTemplate.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ShowAttachedObjectActionTemplate_H
#define INCLUDED_ShowAttachedObjectActionTemplate_H

// ======================================================================

#include "clientAnimation/PlaybackActionTemplate.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"

class Iff;
class MemoryBlockManager;

// ======================================================================
/**
 * A PlaybackActionTemplate that supports hiding and showing objects
 * attached to specific hardpoints/joints on a skeletal appearance.
 */

class ShowAttachedObjectActionTemplate: public PlaybackActionTemplate
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

public:

	static void install();

public:

	virtual PlaybackAction *createPlaybackAction() const;

	bool                    isShowCommand() const;
	Tag                     getHardpointNameVariable() const;

	// Commands that make sense for hide mode only.
	int                     getActorIndex() const;

	// Commands that make sense for show mode only.
	int                     getHideCommandThreadIndex() const;
	int                     getHideCommandActionIndex() const;

private:

	static void                    remove();
	static PlaybackActionTemplate *create(Iff &iff);

private:

	explicit ShowAttachedObjectActionTemplate(Iff &iff);

	void     load_0000(Iff &iff);

	// Disabled.
	ShowAttachedObjectActionTemplate();
	ShowAttachedObjectActionTemplate(const ShowAttachedObjectActionTemplate&);
	ShowAttachedObjectActionTemplate &operator =(const ShowAttachedObjectActionTemplate&);

private:

	bool  m_isShowCommand;
	Tag   m_hardpointNameVariable;
	int   m_actorIndex;
	int   m_hideCommandThreadIndex;
	int   m_hideCommandActionIndex;

};

// ======================================================================

#endif
