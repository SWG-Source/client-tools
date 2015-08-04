// ======================================================================
//
// QtPriorityBlendAnimationTemplateUiFactory.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_QtPriorityBlendAnimationTemplateUiFactory_H
#define INCLUDED_QtPriorityBlendAnimationTemplateUiFactory_H

// ======================================================================

#include "AnimationEditor/QtSkeletalAnimationTemplateUiFactory.h"

// ======================================================================

class QtPriorityBlendAnimationTemplateUiFactory: public QtSkeletalAnimationTemplateUiFactory
{
public:

	QtPriorityBlendAnimationTemplateUiFactory();
	virtual ~QtPriorityBlendAnimationTemplateUiFactory();

	virtual QListViewItem *createListItem(QListViewItem *parentListItem, SkeletalAnimationTemplate *animationTemplate, FileModel &fileModel) const;

private:

	// disabled
	QtPriorityBlendAnimationTemplateUiFactory(const QtPriorityBlendAnimationTemplateUiFactory&);
	QtPriorityBlendAnimationTemplateUiFactory &operator =(const QtPriorityBlendAnimationTemplateUiFactory&);

};

// ======================================================================

#endif
