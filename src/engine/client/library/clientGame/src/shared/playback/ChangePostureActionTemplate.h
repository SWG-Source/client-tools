// ======================================================================
//
// ChangePostureActionTemplate.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ChangePostureActionTemplate_H
#define INCLUDED_ChangePostureActionTemplate_H

// ======================================================================

#include "clientAnimation/PlaybackActionTemplate.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"

class Iff;

// ======================================================================

class ChangePostureActionTemplate: public PlaybackActionTemplate
{
public:

	static void install();

public:

	virtual PlaybackAction *createPlaybackAction() const;
	virtual bool            update(float deltaTime, PlaybackScript &script) const;

private:

	static void                    remove();
	static PlaybackActionTemplate *create(Iff &iff);

private:

	explicit ChangePostureActionTemplate(Iff &iff);

	void     load_0000(Iff &iff);

	int      getActorIndex() const;
	int      getDestinationPosture(const PlaybackScript &script) const;

	// Disabled.
	ChangePostureActionTemplate();
	ChangePostureActionTemplate(const ChangePostureActionTemplate&);
	ChangePostureActionTemplate &operator =(const ChangePostureActionTemplate&);

private:

	static bool  ms_installed;

private:

	int  m_actorIndex;
	Tag  m_destinationPostureVariable;

};

// ======================================================================

#endif
