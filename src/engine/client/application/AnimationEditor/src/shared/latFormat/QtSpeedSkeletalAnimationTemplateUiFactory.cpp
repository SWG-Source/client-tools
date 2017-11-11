// ======================================================================
//
// QtSpeedSkeletalAnimationTemplateUiFactory.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"
#include "AnimationEditor/QtSpeedSkeletalAnimationTemplateUiFactory.h"

#include "AnimationEditor/SpeedAnimationListItem.h"
#include "clientSkeletalAnimation/SpeedSkeletalAnimationTemplate.h"

// ======================================================================
// class QtSpeedSkeletalAnimationTemplateUiFactory: public member functions
// ======================================================================

QtSpeedSkeletalAnimationTemplateUiFactory::QtSpeedSkeletalAnimationTemplateUiFactory() :
	QtSkeletalAnimationTemplateUiFactory()
{
}

// ----------------------------------------------------------------------

QtSpeedSkeletalAnimationTemplateUiFactory::~QtSpeedSkeletalAnimationTemplateUiFactory()
{
}

// ----------------------------------------------------------------------

QListViewItem *QtSpeedSkeletalAnimationTemplateUiFactory::createListItem(QListViewItem *parentListItem, SkeletalAnimationTemplate *animationTemplate, FileModel &fileModel) const
{
	//-- Get SpeedSkeletalAnimationTemplate.
	SpeedSkeletalAnimationTemplate *const speedTemplate = dynamic_cast<SpeedSkeletalAnimationTemplate*>(animationTemplate);
	if (!speedTemplate)
	{
		WARNING_STRICT_FATAL(true, ("animationTemplate arg is not a SpeedSkeletalAnimationTemplate."));
		return 0;
	}

	//-- Create and return proxy ui element.
	return new SpeedAnimationListItem(parentListItem, *speedTemplate, fileModel);
}

// ======================================================================
