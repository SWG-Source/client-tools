// ======================================================================
//
// SpeedAnimationListItem.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_SpeedAnimationListItem_H
#define INCLUDED_SpeedAnimationListItem_H

// ======================================================================

#include "AnimationEditor/ListItem.h"

class SpeedSkeletalAnimationTemplate;
class FileModel;
class QPixmap;
class QString;

// ======================================================================

class SpeedAnimationListItem: public ListItem
{
	Q_OBJECT

public:

	SpeedAnimationListItem(QListViewItem *newParent, SpeedSkeletalAnimationTemplate &directionAnimationTemplate, FileModel &fileModel);

	virtual bool        supportsPopupMenu() const;
	virtual QPopupMenu *createPopupMenu() const;

	virtual bool        supportsAttachingDownStreamAnimationTemplate() const;
	virtual void        attachDownStreamAnimationTemplate(SkeletalAnimationTemplate const *animationTemplate);

public slots:

	void  createNewChoiceTemplate();

private:

	static const QPixmap &getPixmap();

private:

	// disabled
	SpeedAnimationListItem();
	SpeedAnimationListItem(const SpeedAnimationListItem&);
	SpeedAnimationListItem &operator =(const SpeedAnimationListItem&);

private:

	static const QString            cms_speedItemText;

private:

	FileModel                      &m_fileModel;
	SpeedSkeletalAnimationTemplate &m_speedAnimationTemplate;

};

// ======================================================================

#endif
