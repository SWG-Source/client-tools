// ======================================================================
//
// LatEntryFolderListItem.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"
#include "AnimationEditor/LatEntryFolderListItem.h"

#include "AnimationEditor/LogicalAnimationTableWidget.h"

#include <qpopupmenu.h>
#include <string>

// ======================================================================

const std::string LatEntryFolderListItem::cms_logicalAnimationFolderText("Logical Animations");

// ======================================================================
// class LatEntryFolderListItem: public member functions
// ======================================================================

LatEntryFolderListItem::LatEntryFolderListItem(QListView *newParent, LogicalAnimationTableWidget &latWidget, const QString *preTextSortKey) :
	FolderListItem(newParent, cms_logicalAnimationFolderText, FolderListItem::FC_green, preTextSortKey),
	m_latWidget(latWidget)
{
}

// ----------------------------------------------------------------------

bool LatEntryFolderListItem::supportsPopupMenu() const
{
	return true;
}

// ----------------------------------------------------------------------

QPopupMenu *LatEntryFolderListItem::createPopupMenu() const
{
	//-- Create the menu.
	QPopupMenu *menu = new QPopupMenu();

	//-- Add menu items.
	IGNORE_RETURN(menu->insertItem("Add New Entries", &m_latWidget, SLOT(addNewLatEntries()), QListViewItem::CTRL + QListViewItem::Key_A)); //lint !e56 // bad type // ?
	IGNORE_RETURN(menu->insertItem("Delete Stale Entries", &m_latWidget, SLOT(deleteStaleLatEntries()), QListViewItem::CTRL + QListViewItem::Key_D)); //lint !e56 // bad type // ?

	//-- Return menu to caller.
	return menu;
}

// ======================================================================
