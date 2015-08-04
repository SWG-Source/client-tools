// ======================================================================
//
// QtYawSkeletalAnimationTemplateUiFactory.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_QtYawSkeletalAnimationTemplateUiFactory_H
#define INCLUDED_QtYawSkeletalAnimationTemplateUiFactory_H

// ======================================================================

#include "AnimationEditor/QtSkeletalAnimationTemplateUiFactory.h"

// ======================================================================

class QtYawSkeletalAnimationTemplateUiFactory: public QtSkeletalAnimationTemplateUiFactory
{
public:

	QtYawSkeletalAnimationTemplateUiFactory();
	virtual ~QtYawSkeletalAnimationTemplateUiFactory();

	virtual QListViewItem *createListItem(QListViewItem *parentListItem, SkeletalAnimationTemplate *animationTemplate, FileModel &fileModel) const;

private:

	// disabled
	QtYawSkeletalAnimationTemplateUiFactory(const QtYawSkeletalAnimationTemplateUiFactory&);
	QtYawSkeletalAnimationTemplateUiFactory &operator =(const QtYawSkeletalAnimationTemplateUiFactory&);

};

// ======================================================================

#endif
