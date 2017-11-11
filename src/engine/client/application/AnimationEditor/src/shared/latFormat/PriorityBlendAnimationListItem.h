// ======================================================================
//
// PriorityBlendAnimationListItem.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_PriorityBlendAnimationListItem_H
#define INCLUDED_PriorityBlendAnimationListItem_H

// ======================================================================

#include "AnimationEditor/ListItem.h"

class FileModel;
class QPixmap;
class QString;
class SkeletalAnimationTemplate;
class PriorityBlendAnimationTemplate;

// ======================================================================

class PriorityBlendAnimationListItem: public ListItem
{
public:

	class ChoiceListItem;

public:

	PriorityBlendAnimationListItem(QListViewItem *newParent, PriorityBlendAnimationTemplate &priorityBlendAnimationTemplate, FileModel &fileModel);

	virtual bool                  supportsAttachingDownStreamAnimationTemplate() const;
	virtual void                  attachDownStreamAnimationTemplate(SkeletalAnimationTemplate const *animationTemplate);


	FileModel                      &getFileModel();
	PriorityBlendAnimationTemplate &getPriorityBlendAnimationTemplate();

	void                            updateChoiceTitles();

private:

	static const QPixmap &getPixmap();

private:

	// Disabled.
	PriorityBlendAnimationListItem();
	PriorityBlendAnimationListItem(const PriorityBlendAnimationListItem&);
	PriorityBlendAnimationListItem &operator =(const PriorityBlendAnimationListItem&);

private:

	static QString const         cms_itemText;

private:

	FileModel                      &m_fileModel;
	PriorityBlendAnimationTemplate &m_priorityBlendAnimationTemplate;

};

// ======================================================================

class PriorityBlendAnimationListItem::ChoiceListItem: public ListItem
{
	Q_OBJECT

public:

	ChoiceListItem(PriorityBlendAnimationListItem &newParent, int componentAnimationIndex);

	virtual bool        supportsPopupMenu() const;
	virtual QPopupMenu *createPopupMenu() const;

	virtual bool        handleKeyPress(QKeyEvent *keyEvent);

	int                 getComponentAnimationIndex() const;
	void                buildUi();
	void                updateTitle();

public slots:

	void                createSkeletalAnimationTemplate(int templateIndex);
	void                clearLogicalAnimation();
	void                swapPrimaryAnimation();

private:

	static const QPixmap &getPixmap();

private:

	SkeletalAnimationTemplate const *fetchComponentAnimationTemplate() const;
	QString const                   &getChoiceItemText() const;

	// Disabled.
	ChoiceListItem();
	ChoiceListItem(const ChoiceListItem&);
	ChoiceListItem &operator =(const ChoiceListItem&);

private:

	static QString const cms_choiceItemText[2];

private:

	PriorityBlendAnimationListItem &m_parent;
	int const                       m_componentAnimationIndex;

};

// ======================================================================

inline FileModel &PriorityBlendAnimationListItem::getFileModel()
{
	return m_fileModel;
}

// ----------------------------------------------------------------------

inline PriorityBlendAnimationTemplate &PriorityBlendAnimationListItem::getPriorityBlendAnimationTemplate()
{
	return m_priorityBlendAnimationTemplate;
}

// ======================================================================

#endif
