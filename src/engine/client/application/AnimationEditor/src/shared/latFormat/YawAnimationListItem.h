// ======================================================================
//
// YawAnimationListItem.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_YawAnimationListItem_H
#define INCLUDED_YawAnimationListItem_H

// ======================================================================

#include "AnimationEditor/ListItem.h"

class FileModel;
class QPixmap;
class QString;
class SkeletalAnimationTemplate;
class YawSkeletalAnimationTemplate;

// ======================================================================

class YawAnimationListItem: public ListItem
{
public:

	enum YawChoice
	{
		YC_yawLeft,
		YC_yawRight,
		YC_noYaw
	};

	class ChoiceListItem;

public:

	YawAnimationListItem(QListViewItem *newParent, YawSkeletalAnimationTemplate &yawAnimationTemplate, FileModel &fileModel);

	virtual bool                  supportsAttachingDownStreamAnimationTemplate() const;
	virtual void                  attachDownStreamAnimationTemplate(SkeletalAnimationTemplate const *animationTemplate);


	FileModel                    &getFileModel();
	YawSkeletalAnimationTemplate &getYawAnimationTemplate();

private:

	static const QPixmap &getPixmap();

private:

	// disabled
	YawAnimationListItem();
	YawAnimationListItem(const YawAnimationListItem&);
	YawAnimationListItem &operator =(const YawAnimationListItem&);

private:

	static const QString          cms_yawItemText;

private:

	FileModel                    &m_fileModel;
	YawSkeletalAnimationTemplate &m_yawAnimationTemplate;

};

// ======================================================================

class YawAnimationListItem::ChoiceListItem: public ListItem
{
	Q_OBJECT

public:

	ChoiceListItem(YawAnimationListItem &newParent, YawChoice yawChoice);

	virtual bool        supportsPopupMenu() const;
	virtual QPopupMenu *createPopupMenu() const;

	virtual bool        handleKeyPress(QKeyEvent *keyEvent);

	YawChoice           getYawChoice() const;
	void                buildUi();

public slots:

	void                createSkeletalAnimationTemplate(int templateIndex);
	void                clearLogicalAnimation();

private:

	static const QPixmap &getPixmap();

private:

	SkeletalAnimationTemplate const *fetchYawChoiceAnimationTemplate() const;

	// Disabled.
	ChoiceListItem();
	ChoiceListItem(const ChoiceListItem&);
	ChoiceListItem &operator =(const ChoiceListItem&);

private:

	static const QString  cms_yawChoiceItemText[3];

private:

	YawAnimationListItem &m_parent;
	const YawChoice       m_yawChoice;

};

// ======================================================================

inline FileModel &YawAnimationListItem::getFileModel()
{
	return m_fileModel;
}

// ----------------------------------------------------------------------

inline YawSkeletalAnimationTemplate &YawAnimationListItem::getYawAnimationTemplate()
{
	return m_yawAnimationTemplate;
}

// ======================================================================

#endif
