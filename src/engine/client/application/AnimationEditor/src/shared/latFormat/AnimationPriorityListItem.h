// ======================================================================
//
// AnimationPriorityListItem.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_AnimationPriorityListItem_H
#define INCLUDED_AnimationPriorityListItem_H

// ======================================================================

#include "AnimationEditor/ListItem.h"

class FileModel;
class Iff;
class ProxySkeletalAnimationTemplate;

// ======================================================================

class AnimationPriorityListItem: public ListItem
{
public:

	static AnimationPriorityListItem *createFromData(QListViewItem *newParent, FileModel &fileModel, ProxySkeletalAnimationTemplate &animationTemplate, const QString *preTextSortKey = 0);

public:

	virtual ~AnimationPriorityListItem();
	virtual void handleUpdate() = 0;

protected:

	AnimationPriorityListItem(QListViewItem *newParent, ProxySkeletalAnimationTemplate &animationTemplate, const QString *preTextSortKey = 0);

	ProxySkeletalAnimationTemplate &getAnimationTemplate();

protected:
	class PathListItem;

	class MaskedListItem;
	class UniformListItem;

private:

	// Default.
	AnimationPriorityListItem();
	AnimationPriorityListItem(const AnimationPriorityListItem&);
	AnimationPriorityListItem &operator =(const AnimationPriorityListItem&);

private:

	ProxySkeletalAnimationTemplate &m_animationTemplate;

};

// ======================================================================

inline ProxySkeletalAnimationTemplate &AnimationPriorityListItem::getAnimationTemplate()
{
	return m_animationTemplate;
}

// ======================================================================

#endif
