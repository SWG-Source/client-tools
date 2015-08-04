// ======================================================================
//
// ActionGroupListItem.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"
#include "AnimationEditor/ActionGroupListItem.h"
#include "ActionGroupListItem.moc"

#include "AnimationEditor/ActionListItem.h"
#include "AnimationEditor/FileModel.h"
#include "clientSkeletalAnimation/EditableBasicAnimationAction.h"
#include "clientSkeletalAnimation/EditableAnimationActionGroup.h"
#include "sharedFoundation/CrcLowerString.h"

#include <qpixmap.h>
#include <qpopupmenu.h>

#include <algorithm>
#include <string>

// ======================================================================

ActionGroupListItem::ActionGroupListItem(QListViewItem *newParent, EditableAnimationActionGroup &actionGroup, FileModel &fileModel) :
	TextListItem(newParent, actionGroup.getName().getString(), getPixmap(), &fileModel),
	m_actionGroup(actionGroup),
	m_fileModel(fileModel)
{
	m_type = LITYPE_ActionGroupListItem;
}

// ----------------------------------------------------------------------

bool ActionGroupListItem::supportsPopupMenu() const
{
	return true;
}

// ----------------------------------------------------------------------

QPopupMenu *ActionGroupListItem::createPopupMenu() const
{
	//-- Create the menu.
	QPopupMenu *menu = new QPopupMenu();

	//-- Add menu items.
	IGNORE_RETURN(menu->insertItem("New Action", this, SLOT(createNewAction()), QListViewItem::CTRL + QListViewItem::Key_N)); //lint !e56 // bad type // ?

	//-- Return menu to caller.
	return menu;
}

// ----------------------------------------------------------------------

void ActionGroupListItem::createNewAction()
{
	//-- Create the new action.
	//lint -esym(429, newAction) // not freed or returned // okay, owned by action group.
	EditableBasicAnimationAction *const newAction = new EditableBasicAnimationAction(CrcLowerString::empty, m_actionGroup.getHierarchyTemplate());

	//-- Add action to group.
	m_actionGroup.addAction(newAction);

	//-- Create the new UI element.
	IGNORE_RETURN(new ActionListItem(this, *newAction, &m_fileModel));

	//-- Modify the file model.
	m_fileModel.setModifiedState(true);
}

// ======================================================================

const QPixmap &ActionGroupListItem::getPixmap()
{
	// @todo create pixmap for action group.
	static const QPixmap  cs_pixmap;

	return cs_pixmap;
}

// ======================================================================

bool ActionGroupListItem::modifyCandidateText(std::string &candidateText) const
{
	//-- Don't accept empty name.
	if (candidateText.empty())
		return false;

	//-- Insist on lower case.
	std::transform(candidateText.begin(), candidateText.end(), candidateText.begin(), tolower);

	//-- Accept text.
	return true;
}

// ----------------------------------------------------------------------

void ActionGroupListItem::setSourceText(const std::string &newText) const
{
	m_actionGroup.setName(CrcLowerString(newText.c_str()));
}

// ----------------------------------------------------------------------

std::string ActionGroupListItem::getSourceText() const
{
	return m_actionGroup.getName().getString();
}

// ======================================================================
