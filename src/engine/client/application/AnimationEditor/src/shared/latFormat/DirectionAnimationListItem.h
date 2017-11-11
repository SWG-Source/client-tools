// ======================================================================
//
// DirectionAnimationListItem.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_DirectionAnimationListItem_H
#define INCLUDED_DirectionAnimationListItem_H

// ======================================================================

#include "AnimationEditor/ListItem.h"

class DirectionSkeletalAnimationTemplate;
class FileModel;
class QPixmap;
class QString;

// ======================================================================

class DirectionAnimationListItem: public ListItem
{
public:

	DirectionAnimationListItem(QListViewItem *newParent, DirectionSkeletalAnimationTemplate &directionAnimationTemplate, FileModel &fileModel);

	virtual bool        supportsAttachingDownStreamAnimationTemplate() const;
	virtual void        attachDownStreamAnimationTemplate(SkeletalAnimationTemplate const *animationTemplate);

private:

	static const QPixmap &getPixmap();

private:

	// disabled
	DirectionAnimationListItem();
	DirectionAnimationListItem(const DirectionAnimationListItem&);
	DirectionAnimationListItem &operator =(const DirectionAnimationListItem&);

private:

	static const QString                cms_directionItemText;

private:

	FileModel                          &m_fileModel;
	DirectionSkeletalAnimationTemplate &m_directionAnimationTemplate;

};

// ======================================================================

#endif
