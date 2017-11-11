// ======================================================================
//
// QtTimeScaleSkeletalAnimationTemplateUiFactory.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"
#include "AnimationEditor/QtTimeScaleSkeletalAnimationTemplateUiFactory.h"

#include "AnimationEditor/TimeScaleAnimationListItem.h"
#include "clientSkeletalAnimation/TimeScaleSkeletalAnimationTemplate.h"

// ======================================================================
// class QtTimeScaleSkeletalAnimationTemplateUiFactory: PUBLIC
// ======================================================================

QtTimeScaleSkeletalAnimationTemplateUiFactory::QtTimeScaleSkeletalAnimationTemplateUiFactory() :
	QtSkeletalAnimationTemplateUiFactory()
{
}

// ----------------------------------------------------------------------

QtTimeScaleSkeletalAnimationTemplateUiFactory::~QtTimeScaleSkeletalAnimationTemplateUiFactory()
{
}

// ----------------------------------------------------------------------

QListViewItem *QtTimeScaleSkeletalAnimationTemplateUiFactory::createListItem(QListViewItem *parentListItem, SkeletalAnimationTemplate *animationTemplate, FileModel &fileModel) const
{
	//-- Get AnimationTemplate.
	TimeScaleSkeletalAnimationTemplate *const timeScaleAnimationTemplate = dynamic_cast<TimeScaleSkeletalAnimationTemplate*>(animationTemplate);
	if (!timeScaleAnimationTemplate)
	{
		WARNING_STRICT_FATAL(true, ("animationTemplate arg is not a TimeScaleSkeletalAnimationTemplate."));
		return 0;
	}

	//-- Create and return ui element for this type of animation template.
	return new TimeScaleAnimationListItem(parentListItem, *timeScaleAnimationTemplate, fileModel);
}

// ======================================================================
