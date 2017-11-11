// ======================================================================
//
// QtProxySkeletalAnimationTemplateUiFactory.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_QtProxySkeletalAnimationTemplateUiFactory_H
#define INCLUDED_QtProxySkeletalAnimationTemplateUiFactory_H

// ======================================================================

#include "AnimationEditor/QtSkeletalAnimationTemplateUiFactory.h"

// ======================================================================

class QtProxySkeletalAnimationTemplateUiFactory: public QtSkeletalAnimationTemplateUiFactory
{
public:

	QtProxySkeletalAnimationTemplateUiFactory();
	virtual ~QtProxySkeletalAnimationTemplateUiFactory();

	virtual QListViewItem *createListItem(QListViewItem *parentListItem, SkeletalAnimationTemplate *animationTemplate, FileModel &fileModel) const;

private:

	// disabled
	QtProxySkeletalAnimationTemplateUiFactory(const QtProxySkeletalAnimationTemplateUiFactory&);
	QtProxySkeletalAnimationTemplateUiFactory &operator =(const QtProxySkeletalAnimationTemplateUiFactory&);

};

// ======================================================================

#endif
