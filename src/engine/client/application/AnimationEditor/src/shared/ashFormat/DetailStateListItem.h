// ======================================================================
//
// DetailStateListItem.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_DetailStateListItem_H
#define INCLUDED_DetailStateListItem_H

// ======================================================================

#include "AnimationEditor/ListItem.h"

class ActionListItem;
class AnimationStateHierarchyWidget;
class FolderListItem;
class EditableAnimationAction;
class EditableAnimationState;
class EditableAnimationStateLink;
class StateLinkListItem;

// ======================================================================

class DetailStateListItem: public ListItem
{
	Q_OBJECT

public:

	DetailStateListItem(AnimationStateHierarchyWidget &ashWidget, EditableAnimationState *state, QListView *newParent);
	DetailStateListItem(AnimationStateHierarchyWidget &ashWidget, EditableAnimationState *state, ListItem *newParent);
	virtual ~DetailStateListItem();

	virtual bool                   supportsPopupMenu() const;
	virtual QPopupMenu            *createPopupMenu() const;
	virtual void                   updateVisuals();
	virtual void                   doDoubleClick();

	std::string                    getItemName() const;

	void                           createLinkToState(const DetailStateListItem &targetStateListItem);

	AnimationStateHierarchyWidget &getAshWidget();
	EditableAnimationState        &getAnimationState();
	
	DetailStateListItem           *addChildStateUi(EditableAnimationState &childState);
	StateLinkListItem             *addStateLinkUi(EditableAnimationStateLink &stateLink);
	ListItem                      *addActionUi(EditableAnimationAction &action);

	void                           deleteAction(EditableAnimationAction *action);
	void                           upgradeBasicActionToMovementAction(EditableAnimationAction *action);

	void                           deleteStateLink(EditableAnimationStateLink *link);
	void                           deleteChildState(EditableAnimationState *childState);

public slots:

	void createNewStateLink();
	void createNewChildState();
	void createNewBasicAction();
	void createNewMovementAction();
	void deleteState();

private:

	static std::string  getNewStateName();

private:

	void            doCommonConstructionTasks();
	virtual void    okRename(int col);

	FolderListItem &getActionGrouping();
	FolderListItem &getActionGroupGrouping();
	FolderListItem &getChildStateGrouping();
	FolderListItem &getLinkGrouping();

	// disabled
	DetailStateListItem();
	DetailStateListItem(const DetailStateListItem&);
	DetailStateListItem &operator =(const DetailStateListItem&);

private:

	static const std::string  ms_newStateNameBase;
	static const std::string  ms_childStateGroupingText;
	static const std::string  ms_actionGroupingText;
	static const std::string  ms_actionGroupGroupingText;
	static const std::string  ms_linkGroupingText;

	static const QString      ms_idleAnimationPreTextSortKey;
	static const QString      ms_childStatePreTextSortKey;
	static const QString      ms_actionGroupingPreTextSortKey;
	static const QString      ms_actionGroupGroupingPreTextSortKey;
	static const QString      ms_linkGroupingPreTextSortKey;

	static int                ms_newStateCounter;

private:

	AnimationStateHierarchyWidget &m_ashWidget;
	EditableAnimationState        *m_state;

	FolderListItem                *m_actionGrouping;
	FolderListItem                *m_actionGroupGrouping;
	FolderListItem                *m_linkGrouping;
	FolderListItem                *m_childStateGrouping;

	int                            m_currentVisualState;
};

// ======================================================================

inline AnimationStateHierarchyWidget &DetailStateListItem::getAshWidget()
{
	return m_ashWidget;
}

// ----------------------------------------------------------------------

inline EditableAnimationState &DetailStateListItem::getAnimationState()
{
	return *m_state;
}

// ======================================================================

#endif
