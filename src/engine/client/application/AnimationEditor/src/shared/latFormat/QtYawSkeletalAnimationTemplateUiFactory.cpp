// ======================================================================
//
// QtYawSkeletalAnimationTemplateUiFactory.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"
#include "AnimationEditor/QtYawSkeletalAnimationTemplateUiFactory.h"

#include "AnimationEditor/YawAnimationListItem.h"
#include "clientSkeletalAnimation/YawSkeletalAnimationTemplate.h"

// ======================================================================
// class QtYawSkeletalAnimationTemplateUiFactory: public member functions
// ======================================================================

QtYawSkeletalAnimationTemplateUiFactory::QtYawSkeletalAnimationTemplateUiFactory() :
	QtSkeletalAnimationTemplateUiFactory()
{
}

// ----------------------------------------------------------------------

QtYawSkeletalAnimationTemplateUiFactory::~QtYawSkeletalAnimationTemplateUiFactory()
{
}

// ----------------------------------------------------------------------

QListViewItem *QtYawSkeletalAnimationTemplateUiFactory::createListItem(QListViewItem *parentListItem, SkeletalAnimationTemplate *animationTemplate, FileModel &fileModel) const
{
	//-- Get AnimationTemplate.
	YawSkeletalAnimationTemplate *const yawAnimationTemplate = dynamic_cast<YawSkeletalAnimationTemplate*>(animationTemplate);
	if (!yawAnimationTemplate)
	{
		WARNING_STRICT_FATAL(true, ("animationTemplate arg is not a YawSkeletalAnimationTemplate."));
		return 0;
	}

	//-- Create and return ui element for this type of animation template.
	return new YawAnimationListItem(parentListItem, *yawAnimationTemplate, fileModel);
}

// ======================================================================
