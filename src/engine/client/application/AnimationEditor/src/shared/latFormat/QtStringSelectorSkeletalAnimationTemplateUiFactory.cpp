// ======================================================================
//
// QtStringSelectorSkeletalAnimationTemplateUiFactory.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"
#include "AnimationEditor/QtStringSelectorSkeletalAnimationTemplateUiFactory.h"

#include "AnimationEditor/StringSelectorAnimationListItem.h"
#include "clientSkeletalAnimation/StringSelectorSkeletalAnimationTemplate.h"

// ======================================================================
// class QtStringSelectorSkeletalAnimationTemplateUiFactory: public member functions
// ======================================================================

QtStringSelectorSkeletalAnimationTemplateUiFactory::QtStringSelectorSkeletalAnimationTemplateUiFactory() :
	QtSkeletalAnimationTemplateUiFactory()
{
}

// ----------------------------------------------------------------------

QtStringSelectorSkeletalAnimationTemplateUiFactory::~QtStringSelectorSkeletalAnimationTemplateUiFactory()
{
}

// ----------------------------------------------------------------------

QListViewItem *QtStringSelectorSkeletalAnimationTemplateUiFactory::createListItem(QListViewItem *parentListItem, SkeletalAnimationTemplate *animationTemplate, FileModel &fileModel) const
{
	//-- Get StringSelectorSkeletalAnimationTemplate.
	StringSelectorSkeletalAnimationTemplate *const stringSelectorTemplate = dynamic_cast<StringSelectorSkeletalAnimationTemplate*>(animationTemplate);
	if (!stringSelectorTemplate)
	{
		WARNING_STRICT_FATAL(true, ("animationTemplate arg is not a StringSelectorSkeletalAnimationTemplate."));
		return 0;
	}

	//-- Create and return proxy ui element.
	return new StringSelectorAnimationListItem(parentListItem, *stringSelectorTemplate, fileModel);
}

// ======================================================================
