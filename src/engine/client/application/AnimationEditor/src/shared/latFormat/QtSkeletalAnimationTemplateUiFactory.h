// ======================================================================
//
// QtSkeletalAnimationTemplateUiFactory.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_QtSkeletalAnimationTemplateUiFactory_H
#define INCLUDED_QtSkeletalAnimationTemplateUiFactory_H

// ======================================================================

#include "clientSkeletalAnimation/SkeletalAnimationTemplateUiFactory.h"

class FileModel;
class QListViewItem;
class SkeletalAnimationTemplate;

// ======================================================================

class QtSkeletalAnimationTemplateUiFactory: public SkeletalAnimationTemplateUiFactory
{
public:

	virtual QListViewItem *createListItem(QListViewItem *parentListItem, SkeletalAnimationTemplate *animationTemplate, FileModel &fileModel) const = 0;

protected:

	QtSkeletalAnimationTemplateUiFactory();

private:

	QtSkeletalAnimationTemplateUiFactory(const QtSkeletalAnimationTemplateUiFactory&);
	QtSkeletalAnimationTemplateUiFactory &operator =(const QtSkeletalAnimationTemplateUiFactory&);

};

// ======================================================================

inline QtSkeletalAnimationTemplateUiFactory::QtSkeletalAnimationTemplateUiFactory()
{
}

// ======================================================================

#endif
