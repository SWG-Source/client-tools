// ======================================================================
//
// StartTrailsActionTemplate.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDE_StartTrailsActionTemplate_H
#define INCLUDE_StartTrailsActionTemplate_H

// ======================================================================

#include "clientAnimation/PlaybackActionTemplate.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"

class Iff;

// ======================================================================

class StartTrailsActionTemplate: public PlaybackActionTemplate
{
public:

	static void install();

public:

	virtual PlaybackAction *createPlaybackAction() const;

	int                     getActorIndex() const;
	Tag                     getTrailBitfieldVariable() const;
	Tag                     getSwooshFilenameVariable() const;

private:

	static void                    remove();
	static PlaybackActionTemplate *create(Iff &iff);

private:

	StartTrailsActionTemplate(Iff &iff);

	void  load_0000(Iff &iff);
	void  load_0001(Iff &iff);

	// Disabled.
	StartTrailsActionTemplate();
	StartTrailsActionTemplate(const StartTrailsActionTemplate&);
	StartTrailsActionTemplate &operator =(const StartTrailsActionTemplate&);

private:

	int  m_actorIndex;
	Tag  m_trailBitfieldVariable;
	Tag  m_swooshFilenameVariable;

};

// ======================================================================
// StartTrailsActionTemplate: inlines
// ======================================================================

inline int StartTrailsActionTemplate::getActorIndex() const
{
	return m_actorIndex;
}

// ----------------------------------------------------------------------

inline Tag StartTrailsActionTemplate::getTrailBitfieldVariable() const
{
	return m_trailBitfieldVariable;
}

// ----------------------------------------------------------------------

inline Tag StartTrailsActionTemplate::getSwooshFilenameVariable() const
{
	return m_swooshFilenameVariable;
}

// ======================================================================

#endif
