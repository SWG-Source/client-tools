// ======================================================================
//
// PriorityBlendAnimationListItem.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"
#include "AnimationEditor/PriorityBlendAnimationListItem.h"
#include "PriorityBlendAnimationListItem.moc"

#include "AnimationEditor/DirectionAnimationChoiceListItem.h"
#include "AnimationEditor/EmbeddedImageLoader.h"
#include "AnimationEditor/FileModel.h"
#include "AnimationEditor/QtSkeletalAnimationTemplateUiFactory.h"
#include "AnimationEditor/SkeletalAnimationTemplateFactory.h"
#include "clientSkeletalAnimation/PriorityBlendAnimationTemplate.h"

#include <qpixmap.h>
#include <qpopupmenu.h>

// ======================================================================

QString const PriorityBlendAnimationListItem::cms_itemText("Priority Blend Animation");

// ======================================================================

QString const PriorityBlendAnimationListItem::ChoiceListItem::cms_choiceItemText[2] =
{
	QString("Secondary Animation"),
	QString("Primary Animation")
};

// ======================================================================
// class PriorityBlendAnimationListItem::ChoiceListItem
// ======================================================================

PriorityBlendAnimationListItem::ChoiceListItem::ChoiceListItem(PriorityBlendAnimationListItem &newParent, int componentAnimationIndex) :
	ListItem(&newParent),
	m_parent(newParent),
	m_componentAnimationIndex(componentAnimationIndex)
{
	m_type = LITYPE_PriorityBlendAnimationListItem_ChoiceListItem;
	//-- Set item text.
	ChoiceListItem::setText(0, getChoiceItemText());

	//-- Set Pixmap.
	ChoiceListItem::setPixmap(0, getPixmap());

	//-- Build UI.
	buildUi();
}

// ----------------------------------------------------------------------

bool PriorityBlendAnimationListItem::ChoiceListItem::supportsPopupMenu() const
{
	return true;
}

// ----------------------------------------------------------------------

QPopupMenu *PriorityBlendAnimationListItem::ChoiceListItem::createPopupMenu() const
{
	//-- Create the menu.
	QPopupMenu *menu = new QPopupMenu();

	//-- Add menu items.
	const int count = SkeletalAnimationTemplateFactory::getTemplateCount();
	for (int i = 0; i < count; ++i)
	{
		//-- Create text for menu item.
		const QString menuItemName = QString("New ") + SkeletalAnimationTemplateFactory::getFriendlyClassName(i).c_str();

		//-- Add menu item.
		const int menuItemId = menu->insertItem(menuItemName, this, SLOT(createSkeletalAnimationTemplate(int)));

		//-- Record index of factory template for use when item is selected.
		const bool success = menu->setItemParameter(menuItemId, i);
		WARNING_STRICT_FATAL(!success, ("setting item parameter for menu id [%d] to [%d] failed.", menuItemId, i));
	}

	IGNORE_RETURN(menu->insertItem("Clear Logical Animation", this, SLOT(clearLogicalAnimation()), static_cast<int>(QListViewItem::CTRL) + static_cast<int>(QListViewItem::Key_C))); //lint !e56 // bad type // ?
	IGNORE_RETURN(menu->insertItem("Swap Primary Animation", this, SLOT(swapPrimaryAnimation())));

	//-- Return menu to caller.
	return menu;
}

// ----------------------------------------------------------------------

bool PriorityBlendAnimationListItem::ChoiceListItem::handleKeyPress(QKeyEvent *keyEvent)
{
	if (!keyEvent)
		return false;

	//-- If Ctrl + Shift + hotkey maps to a "insert animation" command, do it.
	if (static_cast<int>(keyEvent->state()) == (static_cast<int>(QObject::ShiftButton) | static_cast<int>(QObject::ControlButton)))
	{
		int const animationTemplateIndex = SkeletalAnimationTemplateFactory::lookupIndexFromHotkey(static_cast<char>(keyEvent->key()));
		if (animationTemplateIndex >= 0)
		{
			// Use the mapping.
			createSkeletalAnimationTemplate(animationTemplateIndex);
			return true;
		}
	}
	else if ((static_cast<int>(keyEvent->state()) == 0) && (keyEvent->key() == static_cast<int>(QObject::Key_Delete)))
	{
		clearLogicalAnimation();
		return true;
	}

	//-- Didn't have a mapping or wasn't a keypress we understood.
	return false;
}

// ----------------------------------------------------------------------

int PriorityBlendAnimationListItem::ChoiceListItem::getComponentAnimationIndex() const
{
	return m_componentAnimationIndex;
}

// ----------------------------------------------------------------------

void PriorityBlendAnimationListItem::ChoiceListItem::buildUi()
{
	//-- Clear the UI.
	deleteListItemChildren();

	//-- Create child animation UI.
	SkeletalAnimationTemplate const *childAnimationTemplate = fetchComponentAnimationTemplate();

	if (childAnimationTemplate)
	{
		//-- Retrieve the UiFactory for the SkeletalAnimationTemplate.
		const QtSkeletalAnimationTemplateUiFactory *const uiFactory = dynamic_cast<const QtSkeletalAnimationTemplateUiFactory*>(childAnimationTemplate->getConstUiFactory());

		if (!uiFactory)
			WARNING_STRICT_FATAL(true, ("An animation template did not have a ui factory associated with it."));
		else
		{
			//-- Create the list item for the SkeletalAnimationTemplate.
			ListItem *const listItem = dynamic_cast<ListItem*>(uiFactory->createListItem(this, const_cast<SkeletalAnimationTemplate*>(childAnimationTemplate), m_parent.getFileModel()));
			UNREF(listItem);

			//-- Expand the new list item.
			// if (listItem)
			//	listItem->expandItemAndParents();

			//-- Release local reference.
			childAnimationTemplate->release();
		}
	}
}

// ----------------------------------------------------------------------

void PriorityBlendAnimationListItem::ChoiceListItem::updateTitle()
{
	//-- Set item text.
	ChoiceListItem::setText(0, getChoiceItemText());
}

// ----------------------------------------------------------------------

void PriorityBlendAnimationListItem::ChoiceListItem::createSkeletalAnimationTemplate(int templateIndex)
{
	//-- Delete Ui children.
	deleteListItemChildren();

	//-- Create the new SkeletalAnimationTemplate instance.
	SkeletalAnimationTemplate const *const oldAnimationTemplate = fetchComponentAnimationTemplate();

	SkeletalAnimationTemplate *const newAnimationTemplate = SkeletalAnimationTemplateFactory::createSkeletalAnimationTemplate(templateIndex);
	NOT_NULL(newAnimationTemplate);

	//-- Replace the Lat animation.
	PriorityBlendAnimationTemplate &priorityBlendAnimationTemplate = m_parent.getPriorityBlendAnimationTemplate();
	priorityBlendAnimationTemplate.setComponentAnimationTemplate(m_componentAnimationIndex, newAnimationTemplate);
	
	//-- Create Ui under this item.
	const QtSkeletalAnimationTemplateUiFactory *uiFactory = dynamic_cast<const QtSkeletalAnimationTemplateUiFactory*>(newAnimationTemplate->getConstUiFactory());
	if (!uiFactory)
	{
		WARNING_STRICT_FATAL(true, ("new SkeletalAnimationTemplate-derived class did not have a UI factory registered.  Ui out of sync."));
		return;
	}

	FileModel &fileModel = m_parent.getFileModel();

	ListItem *const listItem = dynamic_cast<ListItem*>(uiFactory->createListItem(this, newAnimationTemplate, fileModel));
	if (listItem)
	{
		//-- Show the newly created animation.
		listItem->expandItemAndParents();

		//-- Insert processing: handle attaching the replaced animation (the downstream animation) to the newly inserted animation.
		if (oldAnimationTemplate && listItem->supportsAttachingDownStreamAnimationTemplate())
			listItem->attachDownStreamAnimationTemplate(oldAnimationTemplate);
	}
	else
	{
		DEBUG_WARNING(true, ("failed to insert animation because newly created UI item was not a list item."));
	}

	//-- Release local references.
	if (oldAnimationTemplate)
		oldAnimationTemplate->release();

	//-- Set FileModel as modified.
	fileModel.setModifiedState(true);
}

// ----------------------------------------------------------------------

void PriorityBlendAnimationListItem::ChoiceListItem::clearLogicalAnimation()
{
	//-- Delete Ui children.
	deleteListItemChildren();

	//-- Replace the Lat animation.
	PriorityBlendAnimationTemplate &priorityBlendAnimationTemplate = m_parent.getPriorityBlendAnimationTemplate();
	priorityBlendAnimationTemplate.setComponentAnimationTemplate(m_componentAnimationIndex, NULL);

	//-- Set FileModel as modified.
	m_parent.getFileModel().setModifiedState(true);
}

// ----------------------------------------------------------------------

void PriorityBlendAnimationListItem::ChoiceListItem::swapPrimaryAnimation()
{
	//-- Change the primary animation index.
	PriorityBlendAnimationTemplate &priorityBlendAnimationTemplate = m_parent.getPriorityBlendAnimationTemplate();

	int const newPrimaryAnimationIndex = (priorityBlendAnimationTemplate.getPrimaryComponentAnimationIndex() + 1) % 2;
	priorityBlendAnimationTemplate.setPrimaryComponentAnimationIndex(newPrimaryAnimationIndex);
	m_parent.getFileModel().setModifiedState(true);

	//-- Have parent cause all UI children to recreate their UI.
	m_parent.updateChoiceTitles();
}

// ======================================================================

const QPixmap &PriorityBlendAnimationListItem::ChoiceListItem::getPixmap()
{
	static const QPixmap cs_pixmap = EmbeddedImageLoader::getPixmap("priority_blend_animation_choice.png");
	return cs_pixmap;
}

// ======================================================================

SkeletalAnimationTemplate const *PriorityBlendAnimationListItem::ChoiceListItem::fetchComponentAnimationTemplate() const
{
	PriorityBlendAnimationTemplate &priorityBlendAnimationTemplate = m_parent.getPriorityBlendAnimationTemplate();
	return priorityBlendAnimationTemplate.fetchComponentAnimationTemplate(m_componentAnimationIndex);
}

// ----------------------------------------------------------------------

QString const &PriorityBlendAnimationListItem::ChoiceListItem::getChoiceItemText() const
{
	PriorityBlendAnimationTemplate &priorityBlendAnimationTemplate = m_parent.getPriorityBlendAnimationTemplate();
	bool const choiceIsPrimaryAnimation = (m_componentAnimationIndex == priorityBlendAnimationTemplate.getPrimaryComponentAnimationIndex());

	int const textIndex = (choiceIsPrimaryAnimation ? 1 : 0);
	return cms_choiceItemText[textIndex];
}

// ======================================================================
// class PriorityBlendAnimationListItem: public member functions
// ======================================================================

PriorityBlendAnimationListItem::PriorityBlendAnimationListItem(QListViewItem *newParent, PriorityBlendAnimationTemplate &priorityBlendAnimationTemplate, FileModel &fileModel) :
	ListItem(newParent),
	m_fileModel(fileModel),
	m_priorityBlendAnimationTemplate(priorityBlendAnimationTemplate)
{

	m_type = LITYPE_PriorityBlendAnimationListItem;

	//-- Set item text.
	PriorityBlendAnimationListItem::setText(0, cms_itemText);

	//-- Set Pixmap.
	PriorityBlendAnimationListItem::setPixmap(0, getPixmap());

	//-- Create child ui.
	for (int i = 0; i < PriorityBlendAnimationTemplate::MAX_ANIMATION_COUNT; ++i)
		IGNORE_RETURN(new ChoiceListItem(*this, i));  //lint !e1524 // new constructor for class with no destructor // it's okay, Qt owns it.
}

// ----------------------------------------------------------------------

bool PriorityBlendAnimationListItem::supportsAttachingDownStreamAnimationTemplate() const
{
	return true;
}

// ----------------------------------------------------------------------

void PriorityBlendAnimationListItem::attachDownStreamAnimationTemplate(SkeletalAnimationTemplate const *animationTemplate)
{
	if (!animationTemplate)
		return;

	//-- Set primary component animation to the animation template.
	int const primaryAnimationIndex = m_priorityBlendAnimationTemplate.getPrimaryComponentAnimationIndex();
	m_priorityBlendAnimationTemplate.setComponentAnimationTemplate(primaryAnimationIndex, animationTemplate);
	m_fileModel.setModifiedState(true);

	//-- Find the forward DirectionAnimationChoiceListItem.
	for (QListViewItem *item = firstChild(); item != NULL; item = item->nextSibling())
	{
		ChoiceListItem *const listItem = dynamic_cast<ChoiceListItem*>(item);
		if (listItem && (listItem->getComponentAnimationIndex() == primaryAnimationIndex))
		{
			// Found the list item that just changed, fix the UI for it.
			listItem->buildUi();
			break;
		}
	}
}

// ----------------------------------------------------------------------

void PriorityBlendAnimationListItem::updateChoiceTitles()
{
	//-- Find the forward DirectionAnimationChoiceListItem.
	for (QListViewItem *item = firstChild(); item != NULL; item = item->nextSibling())
	{
		ChoiceListItem *const listItem = dynamic_cast<ChoiceListItem*>(item);
		if (listItem)
			listItem->updateTitle();
	}

	//-- Sort child list items.
	sort();
}

// ======================================================================
// class PriorityBlendAnimationListItem: private static member functions
// ======================================================================

const QPixmap &PriorityBlendAnimationListItem::getPixmap()
{
	static const QPixmap cs_pixmap = EmbeddedImageLoader::getPixmap("priority_blend_animation.png");
	return cs_pixmap;
}

// ======================================================================
