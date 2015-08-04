// ======================================================================
//
// WaitForBoltAction.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_WaitForBoltAction_H
#define INCLUDED_WaitForBoltAction_H

// ======================================================================

#include "clientAnimation/PlaybackAction.h"

class WaitForBoltActionTemplate;

// ======================================================================

class WaitForBoltAction: public PlaybackAction
{
public:

	WaitForBoltAction(const WaitForBoltActionTemplate &actionTemplate);

	virtual bool update(float deltaTime, PlaybackScript &script);

private:

	const WaitForBoltActionTemplate &getOurTemplate() const;

	// Disabled.
	WaitForBoltAction();
	WaitForBoltAction(const WaitForBoltAction&);
	WaitForBoltAction &operator =(const WaitForBoltAction&);

private:

	bool   m_initializedPlaybackData;
	float  m_timeToLive;

};

// ======================================================================

#endif
