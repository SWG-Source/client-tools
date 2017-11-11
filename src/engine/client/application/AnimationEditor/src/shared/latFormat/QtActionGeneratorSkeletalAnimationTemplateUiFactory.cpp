// ======================================================================
//
// QtActionGeneratorSkeletalAnimationTemplateUiFactory.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"
#include "AnimationEditor/QtActionGeneratorSkeletalAnimationTemplateUiFactory.h"

#include "AnimationEditor/ActionGeneratorListItem.h"
#include "clientSkeletalAnimation/ActionGeneratorSkeletalAnimationTemplate.h"

// ======================================================================
// class QtActionGeneratorSkeletalAnimationTemplateUiFactory: public member functions
// ======================================================================

QtActionGeneratorSkeletalAnimationTemplateUiFactory::QtActionGeneratorSkeletalAnimationTemplateUiFactory() :
	QtSkeletalAnimationTemplateUiFactory()
{
}

// ----------------------------------------------------------------------

QtActionGeneratorSkeletalAnimationTemplateUiFactory::~QtActionGeneratorSkeletalAnimationTemplateUiFactory()
{
}

// ----------------------------------------------------------------------

QListViewItem *QtActionGeneratorSkeletalAnimationTemplateUiFactory::createListItem(QListViewItem *parentListItem, SkeletalAnimationTemplate *animationTemplate, FileModel &fileModel) const
{
	//-- Get ProxySkeletalAnimationTemplate.
	ActionGeneratorSkeletalAnimationTemplate *const directionTemplate = dynamic_cast<ActionGeneratorSkeletalAnimationTemplate*>(animationTemplate);
	if (!directionTemplate)
	{
		WARNING_STRICT_FATAL(true, ("animationTemplate arg is not a ActionGeneratorSkeletalAnimationTemplate."));
		return 0;
	}

	//-- Create and return proxy ui element.
	return new ActionGeneratorListItem(parentListItem, *directionTemplate, fileModel);
}

// ======================================================================
