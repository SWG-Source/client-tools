// PRIVATE

// ======================================================================
//
// TemporaryAttachedObjectActionTemplate.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_TemporaryAttachedObjectActionTemplate_H
#define INCLUDED_TemporaryAttachedObjectActionTemplate_H

// ======================================================================

#include "clientAnimation/PlaybackActionTemplate.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"

class Iff;
class MemoryBlockManager;

// ======================================================================
/**
 * A PlaybackActionTemplate that supports creating, attaching, removing 
 * and destroying a temporary, client-side-only object to a skeletal
 * appearance.
 */

class TemporaryAttachedObjectActionTemplate: public PlaybackActionTemplate
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

public:

	static void install();

public:

	virtual PlaybackAction *createPlaybackAction() const;

	bool                    isCreateCommand() const;
	Tag                     getObjectTemplateNameVariable() const;
	Tag                     getHardpointNameVariable() const;
	int                     getActorIndex() const;
	int                     getCreateCommandThreadIndex() const;
	int                     getCreateCommandActionIndex() const;

private:

	static void                    remove();
	static PlaybackActionTemplate *create(Iff &iff);

private:

	explicit TemporaryAttachedObjectActionTemplate(Iff &iff);

	void     load_0000(Iff &iff);

	// Disabled.
	TemporaryAttachedObjectActionTemplate();
	TemporaryAttachedObjectActionTemplate(const TemporaryAttachedObjectActionTemplate&);
	TemporaryAttachedObjectActionTemplate &operator =(const TemporaryAttachedObjectActionTemplate&);

private:

	bool  m_isCreateCommand;
	Tag   m_objectTemplateNameVariable;
	Tag   m_hardpointNameVariable;
	int   m_actorIndex;
	int   m_createCommandThreadIndex;
	int   m_createCommandActionIndex;

};

// ======================================================================

#endif
