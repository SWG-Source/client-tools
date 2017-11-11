// ======================================================================
//
// QtSpeedSkeletalAnimationTemplateUiFactory.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_QtSpeedSkeletalAnimationTemplateUiFactory_H
#define INCLUDED_QtSpeedSkeletalAnimationTemplateUiFactory_H

// ======================================================================

#include "AnimationEditor/QtSkeletalAnimationTemplateUiFactory.h"

// ======================================================================

class QtSpeedSkeletalAnimationTemplateUiFactory: public QtSkeletalAnimationTemplateUiFactory
{
public:

	QtSpeedSkeletalAnimationTemplateUiFactory();
	virtual ~QtSpeedSkeletalAnimationTemplateUiFactory();

	virtual QListViewItem *createListItem(QListViewItem *parentListItem, SkeletalAnimationTemplate *animationTemplate, FileModel &fileModel) const;

private:

	// disabled
	QtSpeedSkeletalAnimationTemplateUiFactory(const QtSpeedSkeletalAnimationTemplateUiFactory&);
	QtSpeedSkeletalAnimationTemplateUiFactory &operator =(const QtSpeedSkeletalAnimationTemplateUiFactory&);

};

// ======================================================================

#endif
