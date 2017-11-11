// ======================================================================
//
// QtStringSelectorSkeletalAnimationTemplateUiFactory.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_QtStringSelectorSkeletalAnimationTemplateUiFactory_H
#define INCLUDED_QtStringSelectorSkeletalAnimationTemplateUiFactory_H

// ======================================================================

#include "AnimationEditor/QtSkeletalAnimationTemplateUiFactory.h"

// ======================================================================

class QtStringSelectorSkeletalAnimationTemplateUiFactory: public QtSkeletalAnimationTemplateUiFactory
{
public:

	QtStringSelectorSkeletalAnimationTemplateUiFactory();
	virtual ~QtStringSelectorSkeletalAnimationTemplateUiFactory();

	virtual QListViewItem *createListItem(QListViewItem *parentListItem, SkeletalAnimationTemplate *animationTemplate, FileModel &fileModel) const;

private:

	// disabled
	QtStringSelectorSkeletalAnimationTemplateUiFactory(const QtStringSelectorSkeletalAnimationTemplateUiFactory&);
	QtStringSelectorSkeletalAnimationTemplateUiFactory &operator =(const QtStringSelectorSkeletalAnimationTemplateUiFactory&);

};

// ======================================================================

#endif
