// ======================================================================
//
// LatFormatUiBuilder.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"
#include "AnimationEditor/LatFormatUiBuilder.h"

#include "AnimationEditor/LogicalAnimationListItem.h"
#include "AnimationEditor/LogicalAnimationTableWidget.h"
#include "AnimationEditor/QtSkeletalAnimationTemplateUiFactory.h"
#include "clientSkeletalAnimation/LogicalAnimationTableTemplate.h"
#include "clientSkeletalAnimation/SkeletalAnimationTemplate.h"

// ======================================================================

void LatFormatUiBuilder::buildUi(LogicalAnimationTableTemplate &latTemplate, LogicalAnimationTableWidget &latUi)
{
	//-- Clear all logical animation names.
	latUi.clearLogicalAnimationUi(); //lint !e1060 // can't call private member // Lint is confused, this is a public member.

	//-- Add each logical animation name.
	const int latAnimationCount = latTemplate.getAnimationCount();
	for (int i = 0; i < latAnimationCount; ++i)
	{
		//-- Create the list item for the logical animation name.
		LogicalAnimationListItem  *laListItem = latUi.addLogicalAnimationNameUi(latTemplate.getLogicalAnimationName(i)); //lint !e1060 // can't call private member // Lint is confused, this is a public member.

		//-- Get the SkeletalAnimationTemplate corresonding to the logical animation name.
		SkeletalAnimationTemplate *animationTemplate = latTemplate.fetchAnimationTemplateByIndex(i);
		if (!animationTemplate)
			continue;

		//-- Retrieve the UiFactory for the SkeletalAnimationTemplate.
		const QtSkeletalAnimationTemplateUiFactory *const uiFactory = dynamic_cast<const QtSkeletalAnimationTemplateUiFactory*>(animationTemplate->getConstUiFactory());
		if (uiFactory)
		{
			//-- Create the list item for the SkeletalAnimationTemplate.
			ListItem *const listItem = dynamic_cast<ListItem*>(uiFactory->createListItem(laListItem, animationTemplate, latUi));
			UNREF(listItem);

			//-- Expand the new list item.
			// if (listItem)
			//	listItem->expandItemAndParents();
		}
		else
			WARNING_STRICT_FATAL(true, ("An animation template did not have a ui factory associated with it."));
	}
}

// ======================================================================
