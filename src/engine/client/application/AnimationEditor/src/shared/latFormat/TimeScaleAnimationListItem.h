// ======================================================================
//
// TimeScaleAnimationListItem.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_TimeScaleAnimationListItem_H
#define INCLUDED_TimeScaleAnimationListItem_H

// ======================================================================

#include "AnimationEditor/ListItem.h"

class FileModel;
class QPixmap;
class QString;
class TimeScaleSkeletalAnimationTemplate;

// ======================================================================

class TimeScaleAnimationListItem: public ListItem
{
public:

	class ScaledAnimationListItem;

public:

	TimeScaleAnimationListItem(QListViewItem *newParent, TimeScaleSkeletalAnimationTemplate &timeScaleAnimationTemplate, FileModel &fileModel);

	virtual void                              doDoubleClick();

	virtual bool                              supportsAttachingDownStreamAnimationTemplate() const;
	virtual void                              attachDownStreamAnimationTemplate(SkeletalAnimationTemplate const *animationTemplate);

	FileModel                                &getFileModel();
	TimeScaleSkeletalAnimationTemplate       &getTimeScaleAnimationTemplate();
	TimeScaleSkeletalAnimationTemplate const &getTimeScaleAnimationTemplate() const;

private:

	static const QPixmap                     &getPixmap();

private:

	QString                                   getItemText() const;

private:

	// Disabled.
	TimeScaleAnimationListItem();
	TimeScaleAnimationListItem(const TimeScaleAnimationListItem&);
	TimeScaleAnimationListItem &operator =(const TimeScaleAnimationListItem&);

private:

	FileModel                          &m_fileModel;
	TimeScaleSkeletalAnimationTemplate &m_timeScaleAnimationTemplate;

};

// ======================================================================

class TimeScaleAnimationListItem::ScaledAnimationListItem: public ListItem
{
	Q_OBJECT

public:

	ScaledAnimationListItem(TimeScaleAnimationListItem &newParent);

	virtual bool          supportsPopupMenu() const;
	virtual QPopupMenu   *createPopupMenu() const;

	virtual bool          handleKeyPress(QKeyEvent *keyEvent);

public slots:

	void                  createSkeletalAnimationTemplate(int templateIndex);
	void                  clearLogicalAnimation();

private:

	static const QPixmap &getPixmap();

private:

	// Disabled.
	ScaledAnimationListItem();
	ScaledAnimationListItem(const ScaledAnimationListItem&);
	ScaledAnimationListItem &operator =(const ScaledAnimationListItem&);

private:

	TimeScaleAnimationListItem &m_parent;

};

// ======================================================================

#endif
