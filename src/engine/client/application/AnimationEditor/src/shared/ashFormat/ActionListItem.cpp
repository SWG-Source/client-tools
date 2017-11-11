// ======================================================================
//
// ActionListItem.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"
#include "AnimationEditor/ActionListItem.h"
#include "ActionListItem.moc"

#include "AnimationEditor/ActionAnimationListItem.h"
#include "AnimationEditor/ActionGroupListItem.h"
#include "AnimationEditor/AnimationEditorGameWorld.h"
#include "AnimationEditor/DetailStateListItem.h"
#include "AnimationEditor/EmbeddedImageLoader.h"
#include "AnimationEditor/FileModel.h"
#include "clientSkeletalAnimation/EditableBasicAnimationAction.h"
#include "clientSkeletalAnimation/EditableAnimationActionGroup.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/TransformAnimationResolver.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedObject/Object.h"

#include <qpixmap.h>
#include <qpopupmenu.h>

#include <algorithm>
#include <string>

// ======================================================================

const QString ActionListItem::cms_animationSortKey("1:");
const QString ActionListItem::cms_trackSelectorSortKey("2:");

// ======================================================================
// class ActionListItem::ApplyAddListItem
// ======================================================================

ActionListItem::ApplyAddListItem::ApplyAddListItem(ActionListItem &actionParent, FileModel *fileModel, const QString *preTextSortKey) :
	ListItem(&actionParent, preTextSortKey),
	m_actionParent(actionParent),
	m_fileModel(fileModel)
{
	m_type = LITYPE_ActionListItem;
	//-- Set text.
	updateUiText();

	//-- Set image.
	ApplyAddListItem::setPixmap(0, getPixmap());
}

// ----------------------------------------------------------------------

void ActionListItem::ApplyAddListItem::doDoubleClick()
{
	m_actionParent.getAction().setShouldApplyAnimationAsAdd(!m_actionParent.getAction().shouldApplyAnimationAsAdd(AnimationEditorGameWorld::getFocusObjectAnimationEnvironment()));
	if (m_fileModel)
		m_fileModel->setModifiedState(true);

	//-- Set text.
	updateUiText();
}

// ======================================================================

const QPixmap &ActionListItem::ApplyAddListItem::getPixmap()
{
	static const QPixmap cs_pixmap(EmbeddedImageLoader::getPixmap("track_choice.png"));
	return cs_pixmap;
}

// ======================================================================

void ActionListItem::ApplyAddListItem::updateUiText()
{
	const char *const newText = (m_actionParent.getAction().shouldApplyAnimationAsAdd(AnimationEditorGameWorld::getFocusObjectAnimationEnvironment()) ? "add track" : "play-once track");
	setText(0, newText);
}


// ======================================================================
// class ActionListItem: public member functions
// ======================================================================

ActionListItem::ActionListItem(QListViewItem *newParent, EditableBasicAnimationAction &action, FileModel *fileModel) :
	TextListItem(newParent, action.getName().getString(), getPixmap(), fileModel),
	m_action(action)
{
	//-- Create logical animation name UI.
	IGNORE_RETURN(new ActionAnimationListItem(this, action, fileModel, &cms_animationSortKey)); //lint !e1524 // new in class with no destructor // it's okay, the alloc is managed by Qt.

	//-- Add child items.
	IGNORE_RETURN(new ApplyAddListItem(*this, fileModel, &cms_trackSelectorSortKey)); //lint !e1524 // new in constructor, no destructor // qt owns.
}

// ----------------------------------------------------------------------

bool ActionListItem::supportsPopupMenu() const
{
	return true;
}

// ----------------------------------------------------------------------

QPopupMenu *ActionListItem::createPopupMenu() const
{
	//-- Create the menu.
	QPopupMenu *menu = new QPopupMenu();

	//-- Add menu items.
	IGNORE_RETURN(menu->insertItem("Delete Action", this, SLOT(deleteAction()))); //lint !e56 // bad type // ?
	IGNORE_RETURN(menu->insertItem("Upgrade to Movement Action", this, SLOT(upgradeToMovementAction()))); //lint !e56 // bad type // ?

	//-- Return menu to caller.
	return menu;
}

// ----------------------------------------------------------------------

void ActionListItem::doDoubleClick()
{
	// GOAL: tell the focus Object in the game to do the current action.

	//-- Get the focus Object.
	Object *const object = AnimationEditorGameWorld::getFocusObject();
	if (!object)
	{
		REPORT_LOG(true, ("No focus object present to do action.\n"));
		return;
	}

	//-- Get the SkeletalAppearance2 Appearance for the Object.
	SkeletalAppearance2 *const appearance = dynamic_cast<SkeletalAppearance2*>(object->getAppearance());
	if (!appearance)
	{
		REPORT_LOG(true, ("Focus object is not a skeletal appearance, action playback not possible.\n"));
		return;
	}

	//-- Play the action on all animation controllers associated with the appearance.
	int  animationId;
	bool animationIsAdd;

	appearance->getAnimationResolver().playAction(m_action.getName(), animationId, animationIsAdd, NULL);
}

// ----------------------------------------------------------------------

void ActionListItem::deleteAction()
{
	//-- Get the parent for this item.
	QListViewItem *parentListItem = QListViewItem::parent();
	if (!parentListItem)
	{
		WARNING(true, ("deleteAction(): can't delete, action doesn't have a UI parent."));
		return;
	}

	// Handle attachment to action group.  (Actions are directly attached to their parent action group).
	ActionGroupListItem *const actionGroupItem = dynamic_cast<ActionGroupListItem*>(parentListItem);
	if (actionGroupItem)
	{
		//-- Parent is a state.  Delete the action from the state.
		EditableAnimationAction *const deadAction = &m_action;
		actionGroupItem->getActionGroup().removeAction(deadAction);
		delete deadAction; //lint !e424 // inappropriate deallocation for 'member' data. // it's okay, reference was initialized this way.
		delete this;
		return;
	}

	// Handle attachment to animation state (Actions are attached to an intermediate folder node when attached to animation states).
	parentListItem = parentListItem->parent();
	if (!parentListItem)
	{
		WARNING(true, ("deleteAction(): can't delete, action's parent doesn't have a UI parent."));
		return;
	}

	// Handle animation state.
	DetailStateListItem *const stateListItem = dynamic_cast<DetailStateListItem*>(parentListItem);
	if (stateListItem)
	{
		//-- Parent is a state.  Delete the action from the state.
		stateListItem->deleteAction(&m_action);
		return;
	}

	WARNING(true, ("Failed to delete animation action: couldn't figure out it's relationship to the rest of the world."));
}

// ----------------------------------------------------------------------

void ActionListItem::upgradeToMovementAction()
{
	//-- Get the parent for this item.
	QListViewItem *parentListItem = QListViewItem::parent();
	if (!parentListItem)
	{
		WARNING(true, ("upgradeToMovementAction(): can't do it, action doesn't have a UI parent."));
		return;
	}

	// Handle attachment to action group.  (Actions are directly attached to their parent action group).
	ActionGroupListItem *const actionGroupItem = dynamic_cast<ActionGroupListItem*>(parentListItem);
	if (actionGroupItem)
	{
		//-- Parent is an action group.  Can't handle this yet.
		DEBUG_WARNING(true, ("We don't support upgrading actions attached to action groups yet!"));
		return;
	}

	// Handle attachment to animation state (Actions are attached to an intermediate folder node when attached to animation states).
	parentListItem = parentListItem->parent();
	if (!parentListItem)
	{
		WARNING(true, ("upgradeToMovementAction(): can't delete, action's parent doesn't have a UI parent."));
		return;
	}

	// Handle animation state.
	DetailStateListItem *const stateListItem = dynamic_cast<DetailStateListItem*>(parentListItem);
	if (stateListItem)
	{
		//-- Parent is a state.  Delete the action from the state.
		stateListItem->upgradeBasicActionToMovementAction(&m_action);
		return;
	}

	WARNING(true, ("Failed to upgrade action: couldn't figure out it's relationship to the rest of the world."));
}

// ======================================================================
// class ActionListItem: private static member functions
// ======================================================================

const QPixmap &ActionListItem::getPixmap()
{
	// @todo change to letter a.
	static const QPixmap cs_pixmap(EmbeddedImageLoader::getPixmap("animation_action.png"));
	return cs_pixmap;
}

// ======================================================================
// class ActionListItem: private member functions
// ======================================================================

bool ActionListItem::modifyCandidateText(std::string &candidateText) const
{
	//-- Don't accept blank action names.
	if (candidateText.empty())
		return false;

	//-- Convert action name to lower-case.
	std::transform(candidateText.begin(), candidateText.end(), candidateText.begin(), tolower);

	//-- Accept the modified candidate text.
	return true;
}

// ----------------------------------------------------------------------

void ActionListItem::setSourceText(const std::string &newText) const
{
	m_action.setName(CrcLowerString(newText.c_str()));
}

// ----------------------------------------------------------------------

std::string ActionListItem::getSourceText() const
{
	return m_action.getName().getString();
}

// ======================================================================
