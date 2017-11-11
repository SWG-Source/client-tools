// ======================================================================
//
// MovementActionListItem.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"
#include "AnimationEditor/MovementActionListItem.h"
#include "MovementActionListItem.moc"

#include "AnimationEditor/ActionAnimationListItem.h"
#include "AnimationEditor/ActionGroupListItem.h"
#include "AnimationEditor/AnimationEditorGameWorld.h"
#include "AnimationEditor/DetailStateListItem.h"
#include "AnimationEditor/EmbeddedImageLoader.h"
#include "AnimationEditor/FileModel.h"
#include "clientSkeletalAnimation/EditableAnimationActionGroup.h"
#include "clientSkeletalAnimation/EditableBasicAnimationAction.h"
#include "clientSkeletalAnimation/EditableMovementAnimationAction.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/TransformAnimationResolver.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedObject/Object.h"

#include <qpixmap.h>
#include <qpopupmenu.h>

#include <algorithm>
#include <string>

// ======================================================================

const QString MovementActionListItem::cms_animationSortKey("1:");
const QString MovementActionListItem::cms_speedChildName("Moving Action");
const QString MovementActionListItem::cms_speedChildSortKey("2:");
const QString MovementActionListItem::cms_trackSelectorSortKey("2:");
const QString MovementActionListItem::cms_zeroSpeedChildName("Idle Action");
const QString MovementActionListItem::cms_zeroSpeedChildSortKey("1:");

// ======================================================================
// class MovementActionListItem::ApplyAddListItem
// ======================================================================

MovementActionListItem::ApplyAddListItem::ApplyAddListItem(QListViewItem &newParent, EditableBasicAnimationAction &action, FileModel *fileModel, const QString *preTextSortKey) :
	ListItem(&newParent, preTextSortKey),
	m_action(action),
	m_fileModel(fileModel)
{
	m_type = LITYPE_MovementActionListItem;
	//-- Set text.
	updateUiText();

	//-- Set image.
	ApplyAddListItem::setPixmap(0, getPixmap());
}

// ----------------------------------------------------------------------

void MovementActionListItem::ApplyAddListItem::doDoubleClick()
{
	m_action.setShouldApplyAnimationAsAdd(!m_action.shouldApplyAnimationAsAdd(AnimationEditorGameWorld::getFocusObjectAnimationEnvironment()));
	if (m_fileModel)
		m_fileModel->setModifiedState(true);

	//-- Set text.
	updateUiText();
}

// ======================================================================

const QPixmap &MovementActionListItem::ApplyAddListItem::getPixmap()
{
	static const QPixmap cs_pixmap(EmbeddedImageLoader::getPixmap("track_choice.png"));
	return cs_pixmap;
}

// ======================================================================

void MovementActionListItem::ApplyAddListItem::updateUiText()
{
	const char *const newText = (m_action.shouldApplyAnimationAsAdd(AnimationEditorGameWorld::getFocusObjectAnimationEnvironment()) ? "add track" : "play-once track");
	setText(0, newText);
}


// ======================================================================
// class MovementActionListItem: public member functions
// ======================================================================

MovementActionListItem::MovementActionListItem(QListViewItem *newParent, EditableMovementAnimationAction &action, FileModel *fileModel) :
	TextListItem(newParent, action.getName().getString(), getPixmap(), fileModel),
	m_action(action)
{
	createChildAction(cms_zeroSpeedChildName, cms_zeroSpeedChildSortKey, fileModel, m_action.getZeroSpeedAction());
	createChildAction(cms_speedChildName, cms_speedChildSortKey, fileModel, m_action.getSpeedAction());
}

// ----------------------------------------------------------------------

bool MovementActionListItem::supportsPopupMenu() const
{
	return true;
}

// ----------------------------------------------------------------------

QPopupMenu *MovementActionListItem::createPopupMenu() const
{
	//-- Create the menu.
	QPopupMenu *menu = new QPopupMenu();

	//-- Add menu items.
	IGNORE_RETURN(menu->insertItem("Delete Action", this, SLOT(deleteAction()), QListViewItem::CTRL + QListViewItem::Key_D)); //lint !e56 // bad type // ?

	//-- Return menu to caller.
	return menu;
}

// ----------------------------------------------------------------------

void MovementActionListItem::doDoubleClick()
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

void MovementActionListItem::deleteAction()
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

// ======================================================================
// class MovementActionListItem: private static member functions
// ======================================================================

const QPixmap &MovementActionListItem::getPixmap()
{
	// @todo change to letter a.
	static const QPixmap cs_pixmap(EmbeddedImageLoader::getPixmap("animation_action.png"));
	return cs_pixmap;
}

// ======================================================================
// class MovementActionListItem: private member functions
// ======================================================================

void MovementActionListItem::createChildAction(const QString &childName, const QString &childSortKey, FileModel *fileModel, EditableBasicAnimationAction &childAction)
{
	//-- Create the list item for this child.
	ListItem *childListItem = new ListItem(this, &childSortKey);

	//-- Set list item name.
	childListItem->setText(0, childName);

	//-- Create logical animation name UI.
	IGNORE_RETURN(new ActionAnimationListItem(childListItem, childAction, fileModel, &cms_animationSortKey)); //lint !e1524 // new in class with no destructor // it's okay, the alloc is managed by Qt.

	//-- Add child items.
	IGNORE_RETURN(new ApplyAddListItem(*childListItem, childAction, fileModel, &cms_trackSelectorSortKey)); //lint !e1524 // new in constructor, no destructor // qt owns.
}

// ----------------------------------------------------------------------

bool MovementActionListItem::modifyCandidateText(std::string &candidateText) const
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

void MovementActionListItem::setSourceText(const std::string &newText) const
{
	m_action.getZeroSpeedAction().setName(CrcLowerString(newText.c_str()));
}

// ----------------------------------------------------------------------

std::string MovementActionListItem::getSourceText() const
{
	return m_action.getZeroSpeedAction().getName().getString();
}

// ======================================================================
