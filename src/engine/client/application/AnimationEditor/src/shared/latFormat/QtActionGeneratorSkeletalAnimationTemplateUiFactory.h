// ======================================================================
//
// QtActionGeneratorSkeletalAnimationTemplateUiFactory.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_QtActionGeneratorSkeletalAnimationTemplateUiFactory_H
#define INCLUDED_QtActionGeneratorSkeletalAnimationTemplateUiFactory_H

// ======================================================================

#include "AnimationEditor/QtSkeletalAnimationTemplateUiFactory.h"

// ======================================================================

class QtActionGeneratorSkeletalAnimationTemplateUiFactory: public QtSkeletalAnimationTemplateUiFactory
{
public:

	QtActionGeneratorSkeletalAnimationTemplateUiFactory();
	virtual ~QtActionGeneratorSkeletalAnimationTemplateUiFactory();

	virtual QListViewItem *createListItem(QListViewItem *parentListItem, SkeletalAnimationTemplate *animationTemplate, FileModel &fileModel) const;

private:

	// disabled
	QtActionGeneratorSkeletalAnimationTemplateUiFactory(const QtActionGeneratorSkeletalAnimationTemplateUiFactory&);
	QtActionGeneratorSkeletalAnimationTemplateUiFactory &operator =(const QtActionGeneratorSkeletalAnimationTemplateUiFactory&);

};

// ======================================================================

#endif
