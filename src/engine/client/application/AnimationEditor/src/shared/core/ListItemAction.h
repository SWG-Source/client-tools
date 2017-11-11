// ======================================================================
//
// ListItemAction.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ListItemAction_H
#define INCLUDED_ListItemAction_H

// ======================================================================

class QListViewItem;

// ======================================================================

class ListItemAction
{
public:

	virtual ~ListItemAction();

	virtual bool isTargetAcceptable(const QListViewItem &target) const = 0;
	virtual void doAction(QListViewItem &target) const = 0;

};

// ======================================================================

#endif
