// ======================================================================
//
// AshFormatUiBuilder.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"
#include "AnimationEditor/AshFormatUiBuilder.h"

#include "AnimationEditor/DetailStateListItem.h"
#include "clientSkeletalAnimation/EditableAnimationState.h"

// ======================================================================

void AshFormatUiBuilder::buildStateUi(EditableAnimationState &state, DetailStateListItem &stateUi)
{
	//-- Build state link ui.
	{
		const int linkCount = state.getLinkCount();
		for (int i = 0; i < linkCount; ++i)
		{
			// Get state link.
			EditableAnimationStateLink &stateLink = state.getLink(i);

			// Create and add state link ui.
			IGNORE_RETURN(stateUi.addStateLinkUi(stateLink));
		}
	}

	//-- Build action ui.
	{
		const int actionCount = state.getActionCount();
		for (int i = 0; i < actionCount; ++i)
		{
			// Get action.
			EditableAnimationAction &action = state.getAction(i);

			// Create and add action ui.
			IGNORE_RETURN(stateUi.addActionUi(action));
		}
	}

	//-- Build granted action ui.
	// @todo Build granted action ui.

	//-- Build denied action ui.
	// @todo Build denied action ui.

	//-- Build child state ui.
	{
		const int childCount = state.getChildStateCount();
		for (int i = 0; i < childCount; ++i)
		{
			// Get child state.
			EditableAnimationState &childState = state.getChildState(i);

			// Create and add child state ui to parent ui.
			DetailStateListItem *const newItem = stateUi.addChildStateUi(childState);

			// Populate child state ui.
			buildStateUi(childState, *newItem);
		}
	}
}

// ======================================================================
