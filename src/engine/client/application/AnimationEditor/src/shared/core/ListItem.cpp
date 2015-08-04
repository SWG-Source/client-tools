// ======================================================================
//
// ListItem.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"
#include "AnimationEditor/ListItem.h"

#include <qmessagebox.h>
#include <qstring.h>

// ======================================================================

ListItem::ListItem(QListView *newParent, const QString *preTextSortKey) :
	QListViewItem(newParent),
	QObject(),
	m_preTextSortKey(preTextSortKey ? new QString(*preTextSortKey) : 0)
{
	m_type = LITYPE_ListItem;
}

// ----------------------------------------------------------------------

ListItem::ListItem(QListViewItem *newParent, const QString *preTextSortKey) :
	QListViewItem(newParent),
	QObject(),
	m_preTextSortKey(preTextSortKey ? new QString(*preTextSortKey) : 0)
{
	m_type = LITYPE_ListItem;
}

// ----------------------------------------------------------------------

ListItem::~ListItem()
{
	delete m_preTextSortKey;
}

// ----------------------------------------------------------------------

void ListItem::expandItemAndChildren()
{
	//-- Expand this.
	setOpen(true);

	//-- Instruct children to expand themselves.
	QListViewItem *targetItem = firstChild();
	while (targetItem)
	{
		// If item is a ListItem, ask it to expand itself and children.
		ListItem *const listItem = dynamic_cast<ListItem*>(targetItem);
		if (listItem)
			listItem->expandItemAndChildren();

		// Move on to next child item.
		targetItem = targetItem->nextSibling();
	}
} //lint !e1762 // member function could be made const // Logically this is a non-const operation.

// ----------------------------------------------------------------------

void ListItem::collapseItemAndChildren()
{
	//-- Expand this.
	setOpen(false);

	//-- Instruct children to expand themselves.
	QListViewItem *targetItem = firstChild();
	while (targetItem)
	{
		// If item is a ListItem, ask it to expand itself and children.
		ListItem *const listItem = dynamic_cast<ListItem*>(targetItem);
		if (listItem)
			listItem->collapseItemAndChildren();

		// Move on to next child item.
		targetItem = targetItem->nextSibling();
	}
} //lint !e1762 // member function could be made const // Logically this is a non-const operation.


ListItem* ListItem::findItemMatch(const char* subString, ListItem* selectedItem, bool &ignore, bool onlyTextReplaceableItems)
{
	QString qString(subString);
	
	if(text(0).contains(subString,0) && !ignore)
	{
		if(!onlyTextReplaceableItems || canReplaceText())
		{
			return this;
		}
	}

	// this is the selected item - stop ignoring
	if(selectedItem == this)
		ignore = false;

	ListItem * listItem = dynamic_cast<ListItem*>(firstChild());
	while(listItem != NULL)
	{
		ListItem* foundMatchingListItem = dynamic_cast<ListItem*>(listItem->findItemMatch(subString, selectedItem, ignore, onlyTextReplaceableItems));
		if(foundMatchingListItem)
		{
			return foundMatchingListItem;
		}
		listItem = dynamic_cast<ListItem*>(listItem->nextSibling());
	}
	
	return NULL;
}
// ----------------------------------------------------------------------

ListItem::Type ListItem::getType()
{
	return m_type;
}

// ----------------------------------------------------------------------

bool ListItem::canReplaceText()
{
	switch(m_type)
	{
	case LITYPE_ProxyAnimationListItem:
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------

void ListItem::replaceText(const char *subStringToReplace, const char* replacementText)
{
	UNREF(subStringToReplace);
	UNREF(replacementText);

	QMessageBox::warning(NULL, "Replace Text", "replaceTextCalled on invalid type", QMessageBox::Ok, QMessageBox::NoButton);
}

// ----------------------------------------------------------------------

void ListItem::expandItemAndParents()
{
	QListViewItem *item = this;

	while (item)
	{
		//-- Open the item.
		item->setOpen(true);

		//-- Get the item's parent.
		item = item->parent();
	}
}

// ----------------------------------------------------------------------
/**
 * Delete all the child Ui elements of this Ui element.
 */

void ListItem::deleteListItemChildren()
{
	QListViewItem *targetItem = firstChild();
	while (targetItem)
	{
		QListViewItem *nextItem = targetItem->nextSibling();
		delete targetItem;

		targetItem = nextItem;
	}
} //lint !e1762 // member function could be made const // Logically this is a non-const operation.

// ----------------------------------------------------------------------

void ListItem::makeOnlySelection()
{
	QListView *const ownerListView = listView();
	if (ownerListView)
		ownerListView->setSelected(this, true);
}

// ----------------------------------------------------------------------

bool ListItem::supportsPopupMenu() const
{
	// No popup menu support by default.
	return false;
}

// ----------------------------------------------------------------------

QPopupMenu *ListItem::createPopupMenu() const
{
	// No popup menu support by default.  If we get here, the implementer
	// overrode supportsPopupMenu() but did not overload createPopupMenu().
	WARNING(true, ("createPopupMenu() called on unsupported ListItem, likely an implementation error."));
	return 0;
}

// ----------------------------------------------------------------------

void ListItem::updateVisuals()
{
	// Default: do nothing.  Derived classes can override.
}

// ----------------------------------------------------------------------

void ListItem::doDoubleClick()
{
	// Default: do nothing.  Derived classes can override.
}

// ----------------------------------------------------------------------

QString ListItem::key(int column, bool /* ascending */) const
{
	if (m_preTextSortKey)
		return *m_preTextSortKey + text(column);
	else
		return text(column);
}

// ----------------------------------------------------------------------
/**
 * Indicates whether the ListItem supports attaching an animation template
 * on its underlying data.
 *
 * This function is used by the insert-animation architecture.  By default,
 * a ListItem does not support this.  The root-most ListItem for a
 * SkeletalAnimationTemplate-derived class that can reasonably support
 * attaching an animation should do so.  If it does, it will support
 * being inserted in the spot of an existing animation.
 *
 * @return  true if the ListItem handles attaching a SkeletalAnimationTemplate
 *          to its underlying data source; false otherwise.
 */

bool ListItem::supportsAttachingDownStreamAnimationTemplate() const
{
	return false;
}

// ----------------------------------------------------------------------

void ListItem::attachDownStreamAnimationTemplate(SkeletalAnimationTemplate const * /* animationTemplate */)
{
	FATAL(true, ("ListItem does not support attachDownStreamAnimationTemplate()"));
}

// ----------------------------------------------------------------------
/**
 * Process the specified key press.
 *
 * This interface is used to support list-item-specific hotkey support.
 *
 * @return  true if the list item consumes/handles the input; false otherwise.
 *          The list item should consume the input if it did something
 *          with it.
 */

bool ListItem::handleKeyPress(QKeyEvent *keyEvent)
{
	NOT_NULL(keyEvent);

	DEBUG_REPORT_LOG(true, ("ListItem: default handleKeyPress() called for char [%c], value [%d].\n", keyEvent->ascii(), static_cast<int>(keyEvent->ascii())));
	UNREF(keyEvent);

	//-- Do not consume the input.
	return false;
}

// ======================================================================
