// ======================================================================
//
// PushCreatureAction.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_PushCreatureAction_H
#define INCLUDED_PushCreatureAction_H

// ======================================================================

#include "clientAnimation/PlaybackAction.h"
#include "sharedMath/Vector.h"

class PushCreatureActionTemplate;

// ======================================================================

class PushCreatureAction: public PlaybackAction
{
public:

	PushCreatureAction(const PushCreatureActionTemplate &actionTemplate);

	virtual bool update(float deltaTime, PlaybackScript &script);

private:

	const PushCreatureActionTemplate &getOurTemplate() const;

	// Disabled.
	PushCreatureAction();
	PushCreatureAction(const PushCreatureAction&);
	PushCreatureAction &operator =(const PushCreatureAction&);

private:

	bool   m_initializedPlaybackData;

	Vector m_offset;
	float  m_totalTime;
	float  m_time;

};

// ======================================================================

#endif
