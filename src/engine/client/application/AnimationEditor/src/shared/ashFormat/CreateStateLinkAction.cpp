// ======================================================================
//
// CreateStateLinkAction.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"
#include "AnimationEditor/CreateStateLinkAction.h"

#include "AnimationEditor/DetailStateListItem.h"

// ======================================================================

CreateStateLinkAction::CreateStateLinkAction(DetailStateListItem &sourceStateListItem) :
	m_sourceStateListItem(sourceStateListItem)
{
}

// ----------------------------------------------------------------------

bool CreateStateLinkAction::isTargetAcceptable(const QListViewItem &target) const
{
	//-- Target is acceptable if it is a DetailStateListItem.  The user can only make
	//   state links to states.
	const DetailStateListItem *const targetStateListItem = dynamic_cast<const DetailStateListItem*>(&target);

	if (!targetStateListItem)
		return false;

	//-- Do not permit link to self.
	return (targetStateListItem != &m_sourceStateListItem);
}

// ----------------------------------------------------------------------

void CreateStateLinkAction::doAction(QListViewItem &target) const
{
	//-- Get target as DetailStateListItem.
	const DetailStateListItem *const targetStateListItem = dynamic_cast<const DetailStateListItem*>(&target);
	if (!targetStateListItem)
	{
		WARNING(true, ("doAction(): tried to create link to non state, isTargetAcceptable() logic should have prevented this."));
		return;
	}

	//-- Add the link.
	m_sourceStateListItem.createLinkToState(*targetStateListItem);
} //lint !e1764 // target could be const ref // inherited API must allow for non-const.

// ======================================================================
