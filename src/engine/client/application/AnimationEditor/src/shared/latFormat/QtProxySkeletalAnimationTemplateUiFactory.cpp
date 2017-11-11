// ======================================================================
//
// QtProxySkeletalAnimationTemplateUiFactory.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"
#include "AnimationEditor/QtProxySkeletalAnimationTemplateUiFactory.h"

#include "AnimationEditor/ProxyAnimationListItem.h"
#include "clientSkeletalAnimation/ProxySkeletalAnimationTemplate.h"

// ======================================================================
// class QtProxySkeletalAnimationTemplateUiFactory: public member functions
// ======================================================================

QtProxySkeletalAnimationTemplateUiFactory::QtProxySkeletalAnimationTemplateUiFactory() :
	QtSkeletalAnimationTemplateUiFactory()
{
}

// ----------------------------------------------------------------------

QtProxySkeletalAnimationTemplateUiFactory::~QtProxySkeletalAnimationTemplateUiFactory()
{
}

// ----------------------------------------------------------------------

QListViewItem *QtProxySkeletalAnimationTemplateUiFactory::createListItem(QListViewItem *parentListItem, SkeletalAnimationTemplate *animationTemplate, FileModel &fileModel) const
{
	//-- Get ProxySkeletalAnimationTemplate.
	ProxySkeletalAnimationTemplate *const proxyTemplate = dynamic_cast<ProxySkeletalAnimationTemplate*>(animationTemplate);
	if (!proxyTemplate)
	{
		WARNING_STRICT_FATAL(true, ("animationTemplate arg is not a ProxySkeletalAnimationTemplate."));
		return 0;
	}

	//-- Create and return proxy ui element.
	return new ProxyAnimationListItem(parentListItem, fileModel, *proxyTemplate);
}

// ======================================================================
