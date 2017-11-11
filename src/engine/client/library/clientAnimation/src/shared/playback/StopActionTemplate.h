// PRIVATE

// ======================================================================
//
// StopActionTemplate.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_StopActionTemplate_H
#define INCLUDED_StopActionTemplate_H

// ======================================================================

#include "clientAnimation/PlaybackActionTemplate.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"

class Iff;

// ======================================================================
/**
 * Provide a PlaybackScript Action/ActionTemplate combo that is capable
 * of calling PlaybackAction::stop() a specified action in the current
 * playback script.
 */

class StopActionTemplate: public PlaybackActionTemplate
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

public:

	static void install();

public:

	virtual PlaybackAction *createPlaybackAction() const;
	virtual bool            update(float deltaTime, PlaybackScript &script) const;

private:

	static void                    remove();
	static PlaybackActionTemplate *create(Iff &iff);

private:

	StopActionTemplate(Iff &iff);

	void load_0000(Iff &iff);

	// Disabled.
	StopActionTemplate();
	StopActionTemplate(const StopActionTemplate&);
	StopActionTemplate &operator =(const StopActionTemplate&);

private:

	int  m_threadIndex;
	int  m_actionIndex;

};

// ======================================================================

#endif
