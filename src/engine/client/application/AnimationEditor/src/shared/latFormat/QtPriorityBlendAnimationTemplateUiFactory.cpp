// ======================================================================
//
// QtPriorityBlendAnimationTemplateUiFactory.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"
#include "AnimationEditor/QtPriorityBlendAnimationTemplateUiFactory.h"

#include "AnimationEditor/PriorityBlendAnimationListItem.h"
#include "clientSkeletalAnimation/PriorityBlendAnimationTemplate.h"

// ======================================================================
// class QtPriorityBlendAnimationTemplateUiFactory: public member functions
// ======================================================================

QtPriorityBlendAnimationTemplateUiFactory::QtPriorityBlendAnimationTemplateUiFactory() :
	QtSkeletalAnimationTemplateUiFactory()
{
}

// ----------------------------------------------------------------------

QtPriorityBlendAnimationTemplateUiFactory::~QtPriorityBlendAnimationTemplateUiFactory()
{
}

// ----------------------------------------------------------------------

QListViewItem *QtPriorityBlendAnimationTemplateUiFactory::createListItem(QListViewItem *parentListItem, SkeletalAnimationTemplate *animationTemplate, FileModel &fileModel) const
{
	//-- Get AnimationTemplate.
	PriorityBlendAnimationTemplate *const priorityBlendAnimationTemplate = dynamic_cast<PriorityBlendAnimationTemplate*>(animationTemplate);
	if (!priorityBlendAnimationTemplate)
	{
		WARNING_STRICT_FATAL(true, ("animationTemplate arg is not a PriorityBlendAnimationTemplate."));
		return 0;
	}

	//-- Create and return ui element for this type of animation template.
	return new PriorityBlendAnimationListItem(parentListItem, *priorityBlendAnimationTemplate, fileModel);
}

// ======================================================================
