// ======================================================================
//
// LatEntryFolderListItem.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_LatEntryFolderListItem_H
#define INCLUDED_LatEntryFolderListItem_H

// ======================================================================

#include "AnimationEditor/FolderListItem.h"

class LogicalAnimationTableWidget;

// ======================================================================

class LatEntryFolderListItem: public FolderListItem
{
public:

	LatEntryFolderListItem(QListView *newParent, LogicalAnimationTableWidget &latWidget, const QString *preTextSortKey = 0);

	virtual bool        supportsPopupMenu() const;
	virtual QPopupMenu *createPopupMenu() const;

private:

	// disabled
	LatEntryFolderListItem();
	LatEntryFolderListItem(const LatEntryFolderListItem&);
	LatEntryFolderListItem &operator =(const LatEntryFolderListItem&);

private:

	static const std::string  cms_logicalAnimationFolderText;

private:

	LogicalAnimationTableWidget &m_latWidget;

};

// ======================================================================

#endif
