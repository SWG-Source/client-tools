// ======================================================================
//
// PriorityGroupListItem.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_PriorityGroupListItem_H
#define INCLUDED_PriorityGroupListItem_H

// ======================================================================

#include "AnimationEditor/ListItem.h"

class AnimationPriorityListItem;
class CrcLowerString;

// ======================================================================

class PriorityGroupListItem: public ListItem
{
	Q_OBJECT

public:

	explicit PriorityGroupListItem(QListViewItem *newParent, AnimationPriorityListItem &animationPriorityListItem, const std::string &itemPrefixString, const CrcLowerString &priorityGroupName, const QString *preTextSortKey = 0);
	virtual ~PriorityGroupListItem();
	
	const CrcLowerString &getPriorityGroupName() const;

	virtual bool        supportsPopupMenu() const;
	virtual QPopupMenu *createPopupMenu() const;

public slots:

	void                setPriorityGroup(int index);

private:

	void                updateText();

	// Disabled.
	PriorityGroupListItem();
	PriorityGroupListItem(const PriorityGroupListItem&);
	PriorityGroupListItem &operator =(const PriorityGroupListItem&);
	
private:

	AnimationPriorityListItem &      m_animationPriorityListItem;
	std::string               *const m_itemPrefixString;
	CrcLowerString            *const m_priorityGroupName;
	
};

// ======================================================================

#endif
