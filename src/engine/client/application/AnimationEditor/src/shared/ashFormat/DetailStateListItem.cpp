// ======================================================================
//
// DetailStateListItem.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"
#include "AnimationEditor/DetailStateListItem.h"
#include "DetailStateListItem.moc"

#include "AnimationEditor/ActionListItem.h"
#include "AnimationEditor/AnimationEditorGameWorld.h"
#include "AnimationEditor/AnimationStateHierarchyWidget.h"
#include "AnimationEditor/CreateStateLinkAction.h"
#include "AnimationEditor/MovementActionListItem.h"
#include "clientSkeletalAnimation/EditableAnimationState.h"
#include "clientSkeletalAnimation/EditableAnimationStateLink.h"
#include "clientSkeletalAnimation/EditableBasicAnimationAction.h"
#include "clientSkeletalAnimation/EditableMovementAnimationAction.h"
#include "AnimationEditor/EmbeddedImageLoader.h"
#include "AnimationEditor/FolderListItem.h"
#include "AnimationEditor/IdleTextListItem.h"
#include "AnimationEditor/StateLinkListItem.h"
#include "clientSkeletalAnimation/AnimationStateNameId.h"
#include "clientSkeletalAnimation/AnimationStateNameIdManager.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/TransformAnimationResolver.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedObject/Object.h"

#include <qpopupmenu.h>
#include <qstring.h>
#include <string>

// ======================================================================

const std::string DetailStateListItem::ms_newStateNameBase("new_state");
const std::string DetailStateListItem::ms_childStateGroupingText("Child States");
const std::string DetailStateListItem::ms_actionGroupingText("Actions");
const std::string DetailStateListItem::ms_actionGroupGroupingText("Action Groups");
const std::string DetailStateListItem::ms_linkGroupingText("Links");

const QString DetailStateListItem::ms_idleAnimationPreTextSortKey("1:");
const QString DetailStateListItem::ms_linkGroupingPreTextSortKey("3:");
const QString DetailStateListItem::ms_actionGroupingPreTextSortKey("4:");
const QString DetailStateListItem::ms_actionGroupGroupingPreTextSortKey("5:");
const QString DetailStateListItem::ms_childStatePreTextSortKey("6:");

// ======================================================================

int DetailStateListItem::ms_newStateCounter;

// ======================================================================
// class DetailStateListItem: public member functions
// ======================================================================

DetailStateListItem::DetailStateListItem(AnimationStateHierarchyWidget &ashWidget, EditableAnimationState *state, QListView *newParent) :
	ListItem(newParent),
	m_ashWidget(ashWidget),
	m_state(state),
	m_actionGrouping(0),
	m_actionGroupGrouping(0),
	m_linkGrouping(0),
	m_childStateGrouping(0),
	m_currentVisualState(-1)
{
	m_type = LITYPE_DetailStateListItem;
	NOT_NULL(m_state);
	doCommonConstructionTasks();
}

// ----------------------------------------------------------------------

DetailStateListItem::DetailStateListItem(AnimationStateHierarchyWidget &ashWidget, EditableAnimationState *state, ListItem *newParent) :
	ListItem(newParent),
	m_ashWidget(ashWidget),
	m_state(state),
	m_actionGrouping(0),
	m_actionGroupGrouping(0),
	m_linkGrouping(0),
	m_childStateGrouping(0),
	m_currentVisualState(-1)
{
	m_type = LITYPE_DetailStateListItem;
	NOT_NULL(m_state);
	doCommonConstructionTasks();
}

// ----------------------------------------------------------------------

DetailStateListItem::~DetailStateListItem()
{
	// error 1540: pointer variable not deleted or zero'ed // Qt deletes this, can't be zero'ed because const.
	//lint -esym(1540, DetailStateListItem::m_actionGrouping) 
	//lint -esym(1540, DetailStateListItem::m_actionGroupGrouping) 
	//lint -esym(1540, DetailStateListItem::m_linkGrouping) 
	//lint -esym(1540, DetailStateListItem::m_childStateGrouping) 

	//-- The UI doesn't own this state.  It should only delete
	//   the state when the user requests it.
	m_state = 0;
}

// ----------------------------------------------------------------------

bool DetailStateListItem::supportsPopupMenu() const
{
	return true;
}

// ----------------------------------------------------------------------

QPopupMenu *DetailStateListItem::createPopupMenu() const
{
	//-- Create the menu.
	QPopupMenu *menu = new QPopupMenu();

	//-- Add menu items.
	IGNORE_RETURN(menu->insertItem("New Child State",     this, SLOT(createNewChildState())));
	IGNORE_RETURN(menu->insertItem("New State Link",      this, SLOT(createNewStateLink())));
	IGNORE_RETURN(menu->insertItem("New Basic Action",    this, SLOT(createNewBasicAction())));
	IGNORE_RETURN(menu->insertItem("New Movement Action", this, SLOT(createNewMovementAction())));

	// Don't let user delete the root node.
	const int deleteMenuId = menu->insertItem("Delete State", this, SLOT(deleteState()));
	menu->setItemEnabled(deleteMenuId, m_state->getStateNameId() != AnimationStateNameIdManager::getRootId());

	//-- Return menu to caller.
	return menu;
}

// ----------------------------------------------------------------------

void DetailStateListItem::updateVisuals()
{
	//-- Declare pixmaps here.  These should go in an install.
	static const QPixmap cs_pixmap(EmbeddedImageLoader::getPixmap("letter_s_dark_red.png"));
	static const QPixmap *const cs_pixmapArray[] = { &cs_pixmap };

	//-- Determine what current state should be.
	const int newState = 0;

	//-- Change pixmap if current has changed.
	if (newState != m_currentVisualState)
	{
		m_currentVisualState = newState;
		setPixmap(0, *cs_pixmapArray[newState]);
	}
}

// ----------------------------------------------------------------------

void DetailStateListItem::doDoubleClick()
{
	// GOAL: tell the focus Object in the game to go to this state.

	//-- Get the focus Object.
	Object *const object = AnimationEditorGameWorld::getFocusObject();
	if (!object)
	{
		REPORT_LOG(true, ("No focus object present to do state change.\n"));
		return;
	}

	//-- Get the SkeletalAppearance2 Appearance for the Object.
	SkeletalAppearance2 *const appearance = dynamic_cast<SkeletalAppearance2*>(object->getAppearance());
	if (!appearance)
	{
		REPORT_LOG(true, ("Focus object is not a skeletal appearance, animation state change not possible.\n"));
		return;
	}

	//-- Specify this state as the new state for all animation controllers.
	appearance->getAnimationResolver().setDestinationState(m_state->getStatePath());
}

// ----------------------------------------------------------------------

std::string DetailStateListItem::getItemName() const
{
	if (m_state)
		return m_state->getStateName().getString();
	else
		return "";
}

// ----------------------------------------------------------------------

void DetailStateListItem::createLinkToState(const DetailStateListItem &targetStateListItem)
{
	//-- Check for NULL state.
	if (!m_state)
	{
		WARNING(true, ("createLinkToState(): m_state is NULL."));
		return;
	}

	//-- Create the state link.
	//lint -esym(429, newStateLink) // custodial pointer not freed or returned // m_state owns it.
	EditableAnimationStateLink *const newStateLink = new EditableAnimationStateLink(m_state->getHierarchyTemplate());
	newStateLink->setDestinationPath(targetStateListItem.m_state->getStatePath());

	//-- Add the state link to this state.
	m_state->addLink(newStateLink);

	//-- Add state link UI element.
	IGNORE_RETURN(addStateLinkUi(*newStateLink));

	//-- Tell FileModel it's been modified.
	m_ashWidget.setModifiedState(true);

	//-- Ensure links item is expanded.
	getLinkGrouping().expandItemAndParents();
}

// ----------------------------------------------------------------------

DetailStateListItem *DetailStateListItem::addChildStateUi(EditableAnimationState &childState)
{
	// Create child state ui.
	return new DetailStateListItem(m_ashWidget, &childState, &getChildStateGrouping());
}

// ----------------------------------------------------------------------

StateLinkListItem *DetailStateListItem::addStateLinkUi(EditableAnimationStateLink &stateLink)
{
	// Create state link ui.
	NOT_NULL(m_state);
	return new StateLinkListItem(&getLinkGrouping(), *m_state, stateLink, &m_ashWidget);
}

// ----------------------------------------------------------------------

ListItem *DetailStateListItem::addActionUi(EditableAnimationAction &action)
{
	// @todo engineer the dynamic casts out of existence.

	//-- Handle a basic action.
	EditableBasicAnimationAction *const basicAction = dynamic_cast<EditableBasicAnimationAction*>(&action);
	if (basicAction)
		return new ActionListItem(&getActionGrouping(), *basicAction, &m_ashWidget);

	//-- Handle a movement action.
	EditableMovementAnimationAction *const movementAction = dynamic_cast<EditableMovementAnimationAction*>(&action);
	if (movementAction)
		return new MovementActionListItem(&getActionGrouping(), *movementAction, &m_ashWidget);
	else
	{
		DEBUG_WARNING(true, ("-TRF- unsupported movement class type."));
		return 0;
	}
}

// ----------------------------------------------------------------------

void DetailStateListItem::deleteAction(EditableAnimationAction *action)
{
	if (!m_state)
	{
		WARNING(true, ("null m_state, logic error."));
		return;
	}

	//-- Find the child with the given action, delete the ui.
	if (!m_actionGrouping)
	{
		// there's no way we have this action as a ui.
		WARNING(true, ("trying to delete action from animation state but no action group ui, action [%s].", action->getName().getString()));
		return;
	}

	// Walk action group children.
	// @todo engineer away this type-switching.

	QListViewItem *childListItem = m_actionGrouping->firstChild();
	while (childListItem)
	{
		ActionListItem *const actionListItem = dynamic_cast<ActionListItem*>(childListItem);
		if (actionListItem)
		{
			if (&(actionListItem->getAction()) == action)
			{
				// Found it.
				break;
			}
		}
		else
		{
			MovementActionListItem *const movementActionListItem = dynamic_cast<MovementActionListItem*>(childListItem);
			if (movementActionListItem)
			{
				if (action->getName() == movementActionListItem->getAction().getName())
				{
					// Found it, it's a movement action list item.
					break;
				}
			}
		}

		childListItem = childListItem->nextSibling();
	}

	if (!childListItem)
	{
		// there's no way we have this action as a ui.
		WARNING(true, ("trying to delete action from animation state but animation state doesn't have ui item for action [%s].", action->getName().getString()));
		return;
	}

	// Remove action ui.
	delete childListItem;

	//-- Remove the action from the state, delete it.
	m_state->removeAction(action);
	delete action;

	//-- If there's no more actions, delete the action ui folder.
	if (!m_actionGrouping->firstChild())
	{
		delete m_actionGrouping;
		m_actionGrouping = 0;
	}

	//-- Mark as modified.
	m_ashWidget.setModifiedState(true);
}

// ----------------------------------------------------------------------

void DetailStateListItem::upgradeBasicActionToMovementAction(EditableAnimationAction *action)
{
	if (!m_state)
	{
		WARNING(true, ("null m_state, logic error."));
		return;
	}

	//-- Find the child with the given action, delete the ui.
	if (!m_actionGrouping)
	{
		// there's no way we have this action as a ui.
		WARNING(true, ("trying to upgrade basic action from animation state but no action group ui, action [%s].", action->getName().getString()));
		return;
	}

	// Walk action group children.
	// @todo engineer away this type-switching.

	QListViewItem *childListItem = m_actionGrouping->firstChild();
	while (childListItem)
	{
		ActionListItem *const actionListItem = dynamic_cast<ActionListItem*>(childListItem);
		if (actionListItem)
		{
			if (&(actionListItem->getAction()) == action)
			{
				// Found it.
				break;
			}
		}
		else
		{
			MovementActionListItem *const movementActionListItem = dynamic_cast<MovementActionListItem*>(childListItem);
			if (movementActionListItem)
			{
				if (action->getName() == movementActionListItem->getAction().getName())
				{
					// Found it, it's a movement action list item.  No upgrade needed.
					DEBUG_WARNING(true, ("tried to upgrade action [%s] to movement action but it is already a movement action, ignoring.", action->getName().getString()));
					return;
				}
			}
		}

		childListItem = childListItem->nextSibling();
	}

	if (!childListItem)
	{
		// there's no way we have this action as a ui.
		WARNING(true, ("trying to upgrade action from animation state but animation state doesn't have ui item for action [%s].", action->getName().getString()));
		return;
	}

	//-- Make sure this is an editable basic animation action.
	EditableBasicAnimationAction *const oldBasicAction = dynamic_cast<EditableBasicAnimationAction*>(action);
	if (!oldBasicAction)
	{
		DEBUG_WARNING(true, ("tried to upgrade action [%s] to movement action but it is not an instance of EditableBasicAnimationAction.", action->getName().getString()));
		return;
	}

	//-- Remove action ui.
	delete childListItem;

	//-- Remove the old basic action from the state.
	m_state->removeAction(action);

	//-- Create the new movement action.
	EditableMovementAnimationAction *const newMovementAction = new EditableMovementAnimationAction(oldBasicAction->getName(), m_state->getHierarchyTemplate());

	//-- Copy parameters from old action to zero-speed action.
	EditableBasicAnimationAction &newZeroSpeedAction = newMovementAction->getZeroSpeedAction();
	
	newZeroSpeedAction.setLogicalAnimationName(oldBasicAction->getLogicalAnimationName(AnimationEditorGameWorld::getFocusObjectAnimationEnvironment()));
	newZeroSpeedAction.setShouldHideHeldItem(oldBasicAction->shouldHideHeldItem(AnimationEditorGameWorld::getFocusObjectAnimationEnvironment()));
	newZeroSpeedAction.setShouldApplyAnimationAsAdd(false);

	//-- Delete the old basic action we just upgraded.
	delete action;
	action = 0;

	//-- Set speed action's style to add since they will almost certainly do this.
	EditableBasicAnimationAction &newSpeedAction = newMovementAction->getSpeedAction();
	newSpeedAction.setShouldApplyAnimationAsAdd(true);

	//-- Add new movement action to state.
	m_state->addAction(newMovementAction);

	//-- Create the UI for the new action.
	ListItem *const newActionListItem = addActionUi(*newMovementAction);
	if (newActionListItem)
	{
		// Select the list item.  If I don't do this, the user can get disoriented since the tree view list items shift around.
		newActionListItem->makeOnlySelection();
		newActionListItem->expandItemAndChildren();
		newActionListItem->expandItemAndParents();
	}

	//-- Mark as modified.
	m_ashWidget.setModifiedState(true);
}

// ----------------------------------------------------------------------

void DetailStateListItem::deleteStateLink(EditableAnimationStateLink *link)
{
	if (!m_state)
	{
		WARNING(true, ("null m_state, logic error."));
		return;
	}

	//-- Find the child with the given link, delete the ui.
	if (!m_linkGrouping)
	{
		// there's no way we have this link as a ui.
		WARNING(true, ("trying to delete link from animation state but no link group ui."));
		return;
	}

	// Walk link group children.
	QListViewItem *childListItem = m_linkGrouping->firstChild();
	while (childListItem)
	{
		StateLinkListItem *const linkListItem = dynamic_cast<StateLinkListItem*>(childListItem);
		if (linkListItem)
		{
			if (&(linkListItem->getStateLink()) == link)
			{
				// Found it.
				break;
			}
		}

		childListItem = childListItem->nextSibling();
	}

	if (!childListItem)
	{
		// there's no way we have this link as a ui.
		WARNING(true, ("trying to delete link from animation state but animation state doesn't have ui item for it."));
		return;
	}

	// Remove link ui.
	delete childListItem;

	//-- Remove the link from the state, delete it.
	m_state->removeLink(link);
	delete link;

	//-- If there's no more links, delete the link ui folder.
	if (!m_linkGrouping->firstChild())
	{
		delete m_linkGrouping;
		m_linkGrouping = 0;
	}

	//-- Mark as modified.
	m_ashWidget.setModifiedState(true);
}

// ----------------------------------------------------------------------

void DetailStateListItem::deleteChildState(EditableAnimationState *childState)
{
	if (!m_state)
	{
		WARNING(true, ("null m_state, logic error."));
		return;
	}

	//-- Find the child with the given childState, delete the ui.
	if (!m_childStateGrouping)
	{
		// there's no way we have this childState as a ui.
		WARNING(true, ("trying to delete child state from parent animation state but no child state group ui."));
		return;
	}

	// Walk childState group children.
	QListViewItem *childListItem = m_childStateGrouping->firstChild();
	while (childListItem)
	{
		DetailStateListItem *const childStateListItem = dynamic_cast<DetailStateListItem*>(childListItem);
		if (childStateListItem)
		{
			if (&(childStateListItem->getAnimationState()) == childState)
			{
				// Found it.
				break;
			}
		}

		childListItem = childListItem->nextSibling();
	}

	if (!childListItem)
	{
		// there's no way we have this childState as a ui.
		WARNING(true, ("trying to delete child state from parent animation state but animation state doesn't have ui item for it."));
		return;
	}

	// Remove childState ui.
	delete childListItem;

	//-- Remove the childState from the state, delete it.
	m_state->removeChildState(childState);
	delete childState;

	//-- If there's no more links, delete the child state ui folder.
	if (!m_childStateGrouping->firstChild())
	{
		delete m_childStateGrouping;
		m_childStateGrouping = 0;
	}

	//-- Mark as modified.
	m_ashWidget.setModifiedState(true);
}

// ======================================================================
// class DetailStateListItem: public slots
// ======================================================================

void DetailStateListItem::createNewStateLink()
{
	//-- Tell the document to perform the "CreateStateLink" action next time
	//   a state is clicked.  The state link is not created until that
	//   time.
	m_ashWidget.setListItemAction(new CreateStateLinkAction(*this));
}

// ----------------------------------------------------------------------

void DetailStateListItem::createNewChildState()
{
	//-- Validate preconditions.
	if (!m_state)
	{
		DEBUG_WARNING(true, ("m_state is null."));
		return;
	}

	//-- construct temporary name for new state
	AnimationStateNameId newNameId = AnimationStateNameIdManager::createId(CrcLowerString(getNewStateName().c_str()));
	
	//-- create new state, attach to parent state
	//lint -esym(429, newState) // custodial pointer has not been freed or returned // owned by hierarchy.
	EditableAnimationState *const newState = new EditableAnimationState(newNameId, m_state->getHierarchyTemplate());
	m_state->addChildState(newState);

	//-- add child state UI
	IGNORE_RETURN(addChildStateUi(*newState));

	//-- Tell FileModel it's been modified.
	m_ashWidget.setModifiedState(true);

	//-- Ensure child state grouping item is expanded.
	getChildStateGrouping().expandItemAndParents();
}

// ----------------------------------------------------------------------

void DetailStateListItem::createNewBasicAction()
{
	//-- Validate preconditions.
	if (!m_state)
	{
		DEBUG_WARNING(true, ("m_state is null."));
		return;
	}

	//-- Create new action, attach to parent state.
	//lint -esym(429, newAction) // custodial pointer has not been freed or returned // owned by hierarchy.
	EditableAnimationAction *const newAction = new EditableBasicAnimationAction(CrcLowerString::empty, m_state->getHierarchyTemplate());
	m_state->addAction(newAction);

	//-- create new Action UI
	IGNORE_RETURN(addActionUi(*newAction));

	//-- Tell FileModel it's been modified.
	m_ashWidget.setModifiedState(true);

	//-- Ensure child state grouping item is expanded.
	getActionGrouping().expandItemAndParents();
}

// ----------------------------------------------------------------------

void DetailStateListItem::createNewMovementAction()
{
	//-- Validate preconditions.
	if (!m_state)
	{
		DEBUG_WARNING(true, ("m_state is null."));
		return;
	}

	//-- Create new action, attach to parent state.
	//lint -esym(429, newAction) // custodial pointer has not been freed or returned // owned by hierarchy.
	EditableAnimationAction *const newAction = new EditableMovementAnimationAction(CrcLowerString::empty, m_state->getHierarchyTemplate());
	m_state->addAction(newAction);

	//-- create new Action UI
	IGNORE_RETURN(addActionUi(*newAction));

	//-- Tell FileModel it's been modified.
	m_ashWidget.setModifiedState(true);

	//-- Ensure child state grouping item is expanded.
	getActionGrouping().expandItemAndParents();
}

// ----------------------------------------------------------------------
/**
 * A slot function that can be called to delete the state represented
 * by this UI state element.
 *
 * This function is directly hooked up to the right-click "delete" menu item
 * for this state.
 */

void DetailStateListItem::deleteState()
{
	//-- Check for null m_state.
	if (!m_state)
	{
		WARNING(true, ("deleteState(): m_state is null."));
		return;
	}

	//-- Don't let user delete the root state.
	if (m_state->getStateNameId() == AnimationStateNameIdManager::getRootId()) //lint !e1702 // operator == both global and member // ?
	{
		WARNING(true, ("deleteState(): cannot delete the root state."));
		return;
	}

	//-- Pass the action along to the parent state UI for real processing.
	//   The parent state will take care of deleting the grouping node when
	//   no more child states exist.

	//-- Get the parent for this item.
	QListViewItem *parentListItem = QListViewItem::parent();
	if (!parentListItem)
	{
		WARNING(true, ("deleteState(): can't delete, state link doesn't have a UI parent."));
		return;
	}

	// Handle attachment to animation state (Child state ui elements are attached to an intermediate folder node when attached to parent animation states).
	parentListItem = parentListItem->parent();
	if (!parentListItem)
	{
		WARNING(true, ("deleteState(): can't delete, state's parent doesn't have a UI parent."));
		return;
	}

	// Handle animation state.
	DetailStateListItem *const stateListItem = dynamic_cast<DetailStateListItem*>(parentListItem);
	if (stateListItem)
	{
		//-- Parent is a state.  Delete the action from the state.
		stateListItem->deleteChildState(m_state);
		return;
	}

	WARNING(true, ("Failed to delete animation state: couldn't figure out it's relationship to the rest of the world."));
}

// ======================================================================
// class DetailStateListItem: private static member functions
// ======================================================================

std::string DetailStateListItem::getNewStateName()
{
	//-- Adjust new state counter so we always get a new state name.
	++ms_newStateCounter;

	//-- Convert number to string.
	char numberBuffer[32];
	sprintf(numberBuffer, "%d", ms_newStateCounter);

	//-- Combine state base name with number.
	return ms_newStateNameBase + numberBuffer;
}

// ======================================================================
// class DetailStateListItem: private member functions
// ======================================================================

void DetailStateListItem::doCommonConstructionTasks()
{
	if (!m_state)
	{
		DEBUG_WARNING(true, ("doCommonConstructionTasks(): m_state is null, logic error."));
		return;
	}

	//-- Set the state name.
	DetailStateListItem::setText(0, getItemName().c_str());

	//-- Enable direct renaming of the state if this item is not the root.
	if (m_state->getStateNameId() != AnimationStateNameIdManager::getRootId()) //lint !e1702 // operator != both global and member // ?
		setRenameEnabled(0, true);

	//-- Update visuals.
	DetailStateListItem::updateVisuals();

	//-- Add child list items not directly tracked.
	// Add Idle LogicalAnimation name specification.
	IGNORE_RETURN(new IdleTextListItem(this, *m_state, &m_ashWidget, &ms_idleAnimationPreTextSortKey));
}

// ----------------------------------------------------------------------

void DetailStateListItem::okRename(int col)
{
	//-- Call handling performed by Qt.
	ListItem::okRename(col);

	//-- Check for null m_state.
	if (!m_state)
	{
		WARNING(true, ("okRename(): m_state is null."));
		return;
	}

	//-- I only care about renaming of the name column.
	if (col != 0)
	{
		WARNING(true, ("okRename(): received rename completion for unsupported column [%d].", col));
		return;
	}

	//-- I should never receive a rename operation on the root.
	if (m_state->getStateNameId() == AnimationStateNameIdManager::getRootId()) //lint !e1702 // operator == both global and member // ?
	{
		WARNING(true, ("okRename(): tried to rename root operation, it should have been impossible to get here."));

		//-- Reset text to root name.
		setText(0, AnimationStateNameIdManager::getNameString(AnimationStateNameIdManager::getRootId()).getString());

		return;
	}

	//-- Get name, convert to lowercase.
	QString  newStateName = text(0).lower();
	DEBUG_REPORT_LOG(true, ("okRename(): text is [%s].\n", static_cast<const char*>(newStateName)));

	//-- Never accept a name change that evaluates to the root id.
	AnimationStateNameId newStateNameId = AnimationStateNameIdManager::createId(CrcLowerString(newStateName));
	if (newStateNameId == AnimationStateNameIdManager::getRootId()) //lint !e1702 // operator == both global and member // ?
	{
		WARNING(true, ("okRename(): tried to rename node to root, forbidden."));

		//-- Reset text to previous name.
		setText(0, AnimationStateNameIdManager::getNameString(m_state->getStateNameId()).getString());

		return;
	}

	//-- Reset state's name.
	m_state->setStateNameId(newStateNameId);
}

// ----------------------------------------------------------------------

FolderListItem &DetailStateListItem::getActionGrouping()
{
	//-- Create grouping as necessary.  This is part of the UI declutter
	//   strategy --- only create groupings that have child data.
	if (!m_actionGrouping)
		m_actionGrouping = new FolderListItem(this, ms_actionGroupingText, FolderListItem::FC_blue, &ms_actionGroupingPreTextSortKey);

	return *m_actionGrouping;
}

// ----------------------------------------------------------------------

FolderListItem &DetailStateListItem::getActionGroupGrouping()
{
	//-- Create grouping as necessary.  This is part of the UI declutter
	//   strategy --- only create groupings that have child data.
	if (!m_actionGroupGrouping)
		m_actionGroupGrouping = new FolderListItem(this, ms_actionGroupGroupingText, FolderListItem::FC_yellow, &ms_actionGroupGroupingPreTextSortKey);

	return *m_actionGroupGrouping;
}

// ----------------------------------------------------------------------

FolderListItem &DetailStateListItem::getChildStateGrouping()
{
	//-- Create grouping as necessary.  This is part of the UI declutter
	//   strategy --- only create groupings that have child data.
	if (!m_childStateGrouping)
		m_childStateGrouping = new FolderListItem(this, ms_childStateGroupingText, FolderListItem::FC_red, &ms_childStatePreTextSortKey);

	return *m_childStateGrouping;
}

// ----------------------------------------------------------------------

FolderListItem &DetailStateListItem::getLinkGrouping()
{
	//-- Create grouping as necessary.  This is part of the UI declutter
	//   strategy --- only create groupings that have child data.
	if (!m_linkGrouping)
		m_linkGrouping = new FolderListItem(this, ms_linkGroupingText, FolderListItem::FC_green, &ms_linkGroupingPreTextSortKey);

	return *m_linkGrouping;
}

// ======================================================================
