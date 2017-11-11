// ======================================================================
//
// FolderListItem.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_FolderListItem_H
#define INCLUDED_FolderListItem_H

// ======================================================================

#include "AnimationEditor/ListItem.h"

class QListView;
class QListViewItem;
class QString;

// ======================================================================

class FolderListItem: public ListItem
{
public:

	enum FolderColor
	{
		FC_yellow,
		FC_orange,
		FC_red,
		FC_grey,
		FC_green,
		FC_blue
	};

public:

	explicit FolderListItem(QListView *newParent, const std::string &newText, FolderColor folderColor, const QString *preTextSortKey = 0);
	explicit FolderListItem(QListViewItem *newParent, const std::string &newText, FolderColor folderColor, const QString *preTextSortKey = 0);
	virtual ~FolderListItem();

	virtual void  updateVisuals();

private:

	// disabled
	FolderListItem();
	FolderListItem(const FolderListItem&);
	FolderListItem &operator =(const FolderListItem&);

private:

	int                m_currentVisualState;
	const FolderColor  m_folderColor;

};

// ======================================================================

#endif
