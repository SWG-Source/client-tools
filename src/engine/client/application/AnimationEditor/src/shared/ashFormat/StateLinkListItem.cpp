// ======================================================================
//
// StateLinkListItem.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"
#include "AnimationEditor/StateLinkListItem.h"
#include "StateLinkListItem.moc"

#include "AnimationEditor/DetailStateListItem.h"
#include "AnimationEditor/EmbeddedImageLoader.h"
#include "AnimationEditor/LinkAnimationTextListItem.h"
#include "clientSkeletalAnimation/AnimationStatePath.h"
#include "clientSkeletalAnimation/AnimationStateNameIdManager.h"
#include "clientSkeletalAnimation/EditableAnimationStateLink.h"
#include "sharedFoundation/CrcLowerString.h"

#include <qpixmap.h>
#include <qpopupmenu.h>

// ======================================================================

StateLinkListItem::StateLinkListItem(ListItem *newParent, EditableAnimationState &sourceState, EditableAnimationStateLink &link, FileModel *fileModel) :
	ListItem(newParent),
	m_sourceState(sourceState),
	m_link(link)
{
	m_type = LITYPE_StateLinkListItem;

	//-- Construct text from path.
	std::string  newText;

	const AnimationStatePath &path = m_link.getDestinationPath();
	const int pathLength           = path.getPathLength();
	for (int i = 0; i < pathLength; ++i)
	{
		if (i != 0)
			newText += '.';

		newText += AnimationStateNameIdManager::getNameString(path.getState(i)).getString();
	}

	//-- Set text.
	StateLinkListItem::setText(0, newText.c_str());

	//-- Set pixmap.
	StateLinkListItem::setPixmap(0, getPixmap());

	//-- Create Ui for link's optional LogicalAnimation name specification.
	IGNORE_RETURN(new LinkAnimationTextListItem(this, link, fileModel)); //lint !e1524 // new in constructor for class with no explicit destructor // it's okay, Qt deletes this one.

	//-- Expand this node.
	// expandItemAndParents();
}

// ----------------------------------------------------------------------

bool StateLinkListItem::supportsPopupMenu() const
{
	return true;
}

// ----------------------------------------------------------------------

QPopupMenu *StateLinkListItem::createPopupMenu() const
{
	//-- Create the menu.
	QPopupMenu *menu = new QPopupMenu();

	//-- Add menu items.
	IGNORE_RETURN(menu->insertItem("Delete Link", this, SLOT(deleteStateLink()), QListViewItem::CTRL + QListViewItem::Key_D)); //lint !e56 // bad type // ?

	//-- Return menu to caller.
	return menu;
}

// ----------------------------------------------------------------------

void StateLinkListItem::deleteStateLink()
{
	//-- Get the parent for this item.
	QListViewItem *parentListItem = QListViewItem::parent();
	if (!parentListItem)
	{
		WARNING(true, ("deleteAction(): can't delete, state link doesn't have a UI parent."));
		return;
	}

	// Handle attachment to animation state (State Link ui elements are attached to an intermediate folder node when attached to animation states).
	parentListItem = parentListItem->parent();
	if (!parentListItem)
	{
		WARNING(true, ("deleteAction(): can't delete, state link's parent doesn't have a UI parent."));
		return;
	}

	// Handle animation state.
	DetailStateListItem *const stateListItem = dynamic_cast<DetailStateListItem*>(parentListItem);
	if (stateListItem)
	{
		//-- Parent is a state.  Delete the action from the state.
		stateListItem->deleteStateLink(&m_link);
		return;
	}

	WARNING(true, ("Failed to delete animation state link: couldn't figure out it's relationship to the rest of the world."));
}

// ======================================================================

const QPixmap &StateLinkListItem::getPixmap()
{
	static const QPixmap cs_pixmap = EmbeddedImageLoader::getPixmap("state_link.png");
	return cs_pixmap;
}

// ======================================================================
