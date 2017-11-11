// ======================================================================
//
// StopTrailsActionTemplate.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDE_StopTrailsActionTemplate_H
#define INCLUDE_StopTrailsActionTemplate_H

// ======================================================================

#include "clientAnimation/PlaybackActionTemplate.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"

class Iff;

// ======================================================================

class StopTrailsActionTemplate: public PlaybackActionTemplate
{
public:

	static void install();

public:

	virtual PlaybackAction *createPlaybackAction() const;
	virtual bool            update(float deltaTime, PlaybackScript &script) const;

	int                     getStartTrailsThreadIndex() const;
	int                     getStartTrailsActionIndex() const;

private:

	static void                    remove();
	static PlaybackActionTemplate *create(Iff &iff);

private:

	StopTrailsActionTemplate(Iff &iff);

	void  load_0000(Iff &iff);

	// Disabled.
	StopTrailsActionTemplate();
	StopTrailsActionTemplate(const StopTrailsActionTemplate&);
	StopTrailsActionTemplate &operator =(const StopTrailsActionTemplate&);

private:

	int m_threadIndex;
	int m_actionIndex;

};

// ======================================================================
// StopTrailsActionTemplate: inlines
// ======================================================================

inline int StopTrailsActionTemplate::getStartTrailsThreadIndex() const
{
	return m_threadIndex;
}

// ----------------------------------------------------------------------

inline int StopTrailsActionTemplate::getStartTrailsActionIndex() const
{
	return m_actionIndex;
}

// ======================================================================

#endif
