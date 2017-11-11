// ======================================================================
//
// AlignToHardpointAction.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_AlignToHardpointAction_H
#define INCLUDED_AlignToHardpointAction_H

// ======================================================================

#include "clientAnimation/PlaybackAction.h"

class AlignToHardpointActionTemplate;

// ======================================================================

class AlignToHardpointAction: public PlaybackAction
{
public:

	AlignToHardpointAction(const AlignToHardpointActionTemplate &actionTemplate);

	virtual bool update(float deltaTime, PlaybackScript &script);

private:

	const AlignToHardpointActionTemplate &getOurTemplate() const;

	// Disabled.
	AlignToHardpointAction();
	AlignToHardpointAction(const AlignToHardpointAction&);
	AlignToHardpointAction &operator =(const AlignToHardpointAction&);

};

// ======================================================================

#endif
