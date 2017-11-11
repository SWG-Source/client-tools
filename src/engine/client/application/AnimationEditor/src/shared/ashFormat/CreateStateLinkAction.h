// ======================================================================
//
// CreateStateLinkAction.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_CreateStateLinkAction_H
#define INCLUDED_CreateStateLinkAction_H

// ======================================================================

#include "AnimationEditor/ListItemAction.h"

class DetailStateListItem;

// ======================================================================

class CreateStateLinkAction: public ListItemAction
{
public:

	explicit CreateStateLinkAction(DetailStateListItem &sourceStateListItem);

	virtual bool isTargetAcceptable(const QListViewItem &target) const;
	virtual void doAction(QListViewItem &target) const;

private:

	// disabled
	CreateStateLinkAction();
	CreateStateLinkAction(const CreateStateLinkAction&);
	CreateStateLinkAction &operator =(const CreateStateLinkAction&);

private:

	DetailStateListItem &m_sourceStateListItem;

};

// ======================================================================

#endif
