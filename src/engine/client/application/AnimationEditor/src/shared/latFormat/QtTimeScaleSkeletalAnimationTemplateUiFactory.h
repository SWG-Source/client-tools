// ======================================================================
//
// QtTimeScaleSkeletalAnimationTemplateUiFactory.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_QtTimeScaleSkeletalAnimationTemplateUiFactory_H
#define INCLUDED_QtTimeScaleSkeletalAnimationTemplateUiFactory_H

// ======================================================================

#include "AnimationEditor/QtSkeletalAnimationTemplateUiFactory.h"

// ======================================================================

class QtTimeScaleSkeletalAnimationTemplateUiFactory: public QtSkeletalAnimationTemplateUiFactory
{
public:

	QtTimeScaleSkeletalAnimationTemplateUiFactory();
	virtual ~QtTimeScaleSkeletalAnimationTemplateUiFactory();

	virtual QListViewItem *createListItem(QListViewItem *parentListItem, SkeletalAnimationTemplate *animationTemplate, FileModel &fileModel) const;

private:

	// Disabled.
	QtTimeScaleSkeletalAnimationTemplateUiFactory(const QtTimeScaleSkeletalAnimationTemplateUiFactory&);
	QtTimeScaleSkeletalAnimationTemplateUiFactory &operator =(const QtTimeScaleSkeletalAnimationTemplateUiFactory&);

};

// ======================================================================

#endif
