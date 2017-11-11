// ======================================================================
//
// ActionGroupRootListItem.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"
#include "AnimationEditor/ActionGroupRootListItem.h"
#include "ActionGroupRootListItem.moc"

#include "AnimationEditor/ActionGroupListItem.h"
#include "AnimationEditor/FileModel.h"
#include "clientSkeletalAnimation/EditableAnimationActionGroup.h"
#include "clientSkeletalAnimation/EditableAnimationStateHierarchyTemplate.h"

#include <qpopupmenu.h>
#include <string>

// ======================================================================

const std::string ActionGroupRootListItem::cms_actionGroupRootItemText("Action Groups");

// ======================================================================
// class ActionGroupRootListItem: public member functions
// ======================================================================

ActionGroupRootListItem::ActionGroupRootListItem(QListView *newParent, EditableAnimationStateHierarchyTemplate &hierarchyTemplate, FileModel &fileModel) :
	ListItem(newParent),
	m_hierarchyTemplate(hierarchyTemplate),
	m_fileModel(fileModel)
{
	m_type = LITYPE_ActionGroupRootListItem;
	//-- Set item text.
	ActionGroupRootListItem::setText(0, cms_actionGroupRootItemText.c_str());
}

// ----------------------------------------------------------------------

bool ActionGroupRootListItem::supportsPopupMenu() const
{
	return true;
}

// ----------------------------------------------------------------------

QPopupMenu *ActionGroupRootListItem::createPopupMenu() const
{
	//-- Create the menu.
	QPopupMenu *menu = new QPopupMenu();

	//-- Add menu items.
	IGNORE_RETURN(menu->insertItem("New Action Group", this, SLOT(createNewActionGroup()), QListViewItem::CTRL + QListViewItem::Key_N)); //lint !e56 // bad type // ?

	//-- Return menu to caller.
	return menu;
}

// ----------------------------------------------------------------------

void ActionGroupRootListItem::createNewActionGroup()
{
	//-- Create the new action group.
	//lint -esym(429, newActionGroup) // not freed or returned // owned by hierarchy template.
	EditableAnimationActionGroup *newActionGroup = new EditableAnimationActionGroup(m_hierarchyTemplate);

	//-- Add action group to the hierarchy template.
	m_hierarchyTemplate.addAnimationActionGroup(newActionGroup);

	//-- Create the new ActionGroup UI.
	IGNORE_RETURN(new ActionGroupListItem(this, *newActionGroup, m_fileModel));

	//-- Modify the file model.
	m_fileModel.setModifiedState(true);
}

// ======================================================================
