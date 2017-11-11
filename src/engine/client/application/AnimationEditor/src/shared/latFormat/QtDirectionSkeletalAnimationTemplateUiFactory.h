// ======================================================================
//
// QtDirectionSkeletalAnimationTemplateUiFactory.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_QtDirectionSkeletalAnimationTemplateUiFactory_H
#define INCLUDED_QtDirectionSkeletalAnimationTemplateUiFactory_H

// ======================================================================

#include "AnimationEditor/QtSkeletalAnimationTemplateUiFactory.h"

// ======================================================================

class QtDirectionSkeletalAnimationTemplateUiFactory: public QtSkeletalAnimationTemplateUiFactory
{
public:

	QtDirectionSkeletalAnimationTemplateUiFactory();
	virtual ~QtDirectionSkeletalAnimationTemplateUiFactory();

	virtual QListViewItem *createListItem(QListViewItem *parentListItem, SkeletalAnimationTemplate *animationTemplate, FileModel &fileModel) const;

private:

	// disabled
	QtDirectionSkeletalAnimationTemplateUiFactory(const QtDirectionSkeletalAnimationTemplateUiFactory&);
	QtDirectionSkeletalAnimationTemplateUiFactory &operator =(const QtDirectionSkeletalAnimationTemplateUiFactory&);

};

// ======================================================================

#endif
