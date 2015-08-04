// ======================================================================
//
// StringSelectorAnimationListItem.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_StringSelectorAnimationListItem_H
#define INCLUDED_StringSelectorAnimationListItem_H

// ======================================================================

#include "AnimationEditor/ListItem.h"

class FileModel;
class FolderListItem;
class StringSelectorSkeletalAnimationTemplate;

// ======================================================================

class StringSelectorAnimationListItem: public ListItem
{
	Q_OBJECT

public:

	StringSelectorAnimationListItem(QListViewItem *parentListItem, StringSelectorSkeletalAnimationTemplate &stringSelectorTemplate, FileModel &fileModel);
	virtual ~StringSelectorAnimationListItem();

	virtual bool        supportsPopupMenu() const;
	virtual QPopupMenu *createPopupMenu() const;

	virtual bool        supportsAttachingDownStreamAnimationTemplate() const;
	virtual void        attachDownStreamAnimationTemplate(SkeletalAnimationTemplate const *animationTemplate);

	void                populateSelectionFolder();

public slots:

	void                addChoiceAnimation();

private:

	static const QPixmap &getPixmap();

private:

	// Disabled.
	StringSelectorAnimationListItem();
	StringSelectorAnimationListItem(const StringSelectorAnimationListItem&);
	StringSelectorAnimationListItem &operator =(const StringSelectorAnimationListItem&);

private:

	static const QString  cms_varNameSortKey;
	static const QString  cms_selectionFolderSortKey;

private:

	void  buildUi();

private:

	StringSelectorSkeletalAnimationTemplate &m_selectorTemplate;
	FileModel                               &m_fileModel;
	FolderListItem                          *m_selectionFolderListItem;

};

// ======================================================================

#endif
