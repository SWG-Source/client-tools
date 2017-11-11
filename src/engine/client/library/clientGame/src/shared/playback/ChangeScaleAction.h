// ======================================================================
//
// ChangeScaleAction.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ChangeScaleAction_H
#define INCLUDED_ChangeScaleAction_H

// ======================================================================

#include "clientAnimation/PlaybackAction.h"

class ChangeScaleActionTemplate;

// ======================================================================

class ChangeScaleAction: public PlaybackAction
{
public:

	ChangeScaleAction(const ChangeScaleActionTemplate &actionTemplate);

	virtual bool update(float deltaTime, PlaybackScript &script);

private:

	const ChangeScaleActionTemplate &getOurTemplate() const;

	bool                             initialize(PlaybackScript &script);

	// Disabled.
	ChangeScaleAction();
	ChangeScaleAction(const ChangeScaleAction&);
	ChangeScaleAction &operator =(const ChangeScaleAction&);

private:

	bool   m_initialized;
	float  m_deltaScaleDeltaTime;
	float  m_accumulatedTime;
	float  m_totalTimeDuration;
	float  m_startScale;

};

// ======================================================================

#endif
