// ======================================================================
//
// PassthroughPlaybackAction.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "clientAnimation/FirstClientAnimation.h"
#include "clientAnimation/PassthroughPlaybackAction.h"

#include "clientAnimation/PlaybackActionTemplate.h"

// ======================================================================

PassthroughPlaybackAction::PassthroughPlaybackAction(const PlaybackActionTemplate &playbackActionTemplate)
:	PlaybackAction(&playbackActionTemplate)
{
}

// ----------------------------------------------------------------------

PassthroughPlaybackAction::~PassthroughPlaybackAction()
{
}

// ----------------------------------------------------------------------

bool PassthroughPlaybackAction::update(float deltaTime, PlaybackScript &script)
{
	//-- Get the action template.
	const PlaybackActionTemplate *actionTemplate = getPlaybackActionTemplate();

	//-- Pass through the update to the template.
	if (actionTemplate)
		return actionTemplate->update(deltaTime, script);
	else
		return false;
}
	
// ======================================================================
