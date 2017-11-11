// ======================================================================
//
// PriorityGroupListItem.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"
#include "AnimationEditor/PriorityGroupListItem.h"
#include "PriorityGroupListItem.moc"

#include "AnimationEditor/AnimationPriorityListItem.h"
#include "clientSkeletalAnimation/AnimationPriorityMap.h"
#include "sharedFoundation/CrcLowerString.h"

#include <qpopupmenu.h>
#include <string>
#include <map>

// ======================================================================
// class PriorityGroupListItem: public member functions
// ======================================================================

PriorityGroupListItem::PriorityGroupListItem(QListViewItem *newParent, AnimationPriorityListItem &animationPriorityListItem, const std::string &itemPrefixString, const CrcLowerString &priorityGroupName, const QString *preTextSortKey) :
	ListItem(newParent, preTextSortKey),
	m_animationPriorityListItem(animationPriorityListItem),
	m_itemPrefixString(new std::string(itemPrefixString)),
	m_priorityGroupName(new CrcLowerString(priorityGroupName))
{
	m_type = LITYPE_PriorityGroupListItem;
	updateText();
}

// ----------------------------------------------------------------------

PriorityGroupListItem::~PriorityGroupListItem()
{
	delete m_itemPrefixString;
	delete m_priorityGroupName;
}

// ----------------------------------------------------------------------

const CrcLowerString &PriorityGroupListItem::getPriorityGroupName() const
{
	return *m_priorityGroupName;
}

// ----------------------------------------------------------------------

bool PriorityGroupListItem::supportsPopupMenu() const
{
	return true;
}

// ----------------------------------------------------------------------

QPopupMenu *PriorityGroupListItem::createPopupMenu() const
{
	//-- Create the menu.
	QPopupMenu *menu = new QPopupMenu();

	//-- Sort priority groups by priority, highest to lowest.
	typedef std::multimap<int, std::pair<CrcLowerString, int> >  Map;
	Map  map;

	// Collect map entries.
	const int priorityGroupCount = AnimationPriorityMap::getPriorityGroupCount();
	for (int i = 0; i < priorityGroupCount; ++i)
	{
		// Get the priority group name.
		const CrcLowerString &priorityGroupName  = AnimationPriorityMap::getPriorityGroupName(i);
		const int             priorityGroupValue = AnimationPriorityMap::getPriorityGroupPriority(i);
		
		IGNORE_RETURN(map.insert(Map::value_type(priorityGroupValue, std::make_pair(priorityGroupName, i))));
	}

	//-- Add one menu item per supported animation priority group.
	char buffer[1024];

	const Map::reverse_iterator endIt = map.rend();
	for (Map::reverse_iterator it = map.rbegin(); it != endIt; ++it) //lint !e55 !e81 // bad type, struct/union not supported in comparison // ?
	{
		// Get the priority group name.
		const CrcLowerString &priorityGroupName = (it->second).first;
		const int             index             = (it->second).second;
		
		// Add priority to buffer.
		sprintf(buffer, "%s (%d)", priorityGroupName.getString(), it->first);

		// Add a menu entry for the priority group.
		const int entryId = menu->insertItem(buffer, this, SLOT(setPriorityGroup(int)));

		// Tell call to slot to use the animation priority group index as the parameter.
		IGNORE_RETURN(menu->setItemParameter(entryId, index));
	}

	//-- Return menu to caller.
	return menu;
}

// ======================================================================
// class PriorityGroupListItem: public slot member functions
// ======================================================================

void PriorityGroupListItem::setPriorityGroup(int index)
{
	//-- Validate the index.
	const int groupCount = AnimationPriorityMap::getPriorityGroupCount();
	if ((index < 0) || (index >= groupCount))
	{
		WARNING(true, ("attempted to set priority group to index %d, but max index is %d.", index, groupCount - 1));
		return;
	}

	//-- Get the group name.
	const CrcLowerString &priorityGroupName = AnimationPriorityMap::getPriorityGroupName(index);

	//-- Set this node's group name.
	*m_priorityGroupName = priorityGroupName;

	//-- Update list item text.
	updateText();

	//-- Tell associated AnimationPriorityListItem that there was a change.
	m_animationPriorityListItem.handleUpdate();
}

// ======================================================================
// class PriorityGroupListItem: private member functions
// ======================================================================

void PriorityGroupListItem::updateText()
{
	//-- Construct the item text.
	std::string  builtText(*m_itemPrefixString);

	builtText += m_priorityGroupName->getString();
	PriorityGroupListItem::setText(0, builtText.c_str());
}

// ======================================================================
