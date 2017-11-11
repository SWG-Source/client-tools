// ======================================================================
//
// QtDirectionSkeletalAnimationTemplateUiFactory.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"
#include "AnimationEditor/QtDirectionSkeletalAnimationTemplateUiFactory.h"

#include "AnimationEditor/DirectionAnimationListItem.h"
#include "clientSkeletalAnimation/DirectionSkeletalAnimationTemplate.h"

// ======================================================================
// class QtDirectionSkeletalAnimationTemplateUiFactory: public member functions
// ======================================================================

QtDirectionSkeletalAnimationTemplateUiFactory::QtDirectionSkeletalAnimationTemplateUiFactory() :
	QtSkeletalAnimationTemplateUiFactory()
{
}

// ----------------------------------------------------------------------

QtDirectionSkeletalAnimationTemplateUiFactory::~QtDirectionSkeletalAnimationTemplateUiFactory()
{
}

// ----------------------------------------------------------------------

QListViewItem *QtDirectionSkeletalAnimationTemplateUiFactory::createListItem(QListViewItem *parentListItem, SkeletalAnimationTemplate *animationTemplate, FileModel &fileModel) const
{
	//-- Get ProxySkeletalAnimationTemplate.
	DirectionSkeletalAnimationTemplate *const directionTemplate = dynamic_cast<DirectionSkeletalAnimationTemplate*>(animationTemplate);
	if (!directionTemplate)
	{
		WARNING_STRICT_FATAL(true, ("animationTemplate arg is not a DirectionSkeletalAnimationTemplate."));
		return 0;
	}

	//-- Create and return proxy ui element.
	return new DirectionAnimationListItem(parentListItem, *directionTemplate, fileModel);
}

// ======================================================================
