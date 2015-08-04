// ======================================================================
//
// SpeedAnimationChoiceListItem.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"
#include "AnimationEditor/SpeedAnimationChoiceListItem.h"
#include "SpeedAnimationChoiceListItem.moc"

#include "AnimationEditor/EmbeddedImageLoader.h"
#include "AnimationEditor/FileModel.h"
#include "AnimationEditor/QtSkeletalAnimationTemplateUiFactory.h"
#include "AnimationEditor/SkeletalAnimationTemplateFactory.h"

#include <qpixmap.h>
#include <qpopupmenu.h>

// ======================================================================

SpeedAnimationChoiceListItem::SpeedAnimationChoiceListItem(QListViewItem *newParent, SpeedSkeletalAnimationTemplate &animationTemplate, SkeletalAnimationTemplate *choiceAnimationTemplate, FileModel &fileModel) :
	ListItem(newParent),
	m_speedAnimationTemplate(animationTemplate),
	m_fileModel(fileModel),
	m_choiceAnimationTemplate(choiceAnimationTemplate)
{
	m_type = LITYPE_SpeedAnimationChoiceListItem;

	//-- Set item text.
	SpeedAnimationChoiceListItem::setText(0, "Speed choice");

	//-- Set Pixmap
	SpeedAnimationChoiceListItem::setPixmap(0, getPixmap());

	//-- Create child animation.
	if (m_choiceAnimationTemplate)
	{
		//-- Retrieve the UiFactory for the SkeletalAnimationTemplate.
		const QtSkeletalAnimationTemplateUiFactory *const uiFactory = dynamic_cast<const QtSkeletalAnimationTemplateUiFactory*>(m_choiceAnimationTemplate->getConstUiFactory());

		if (!uiFactory)
			WARNING_STRICT_FATAL(true, ("An animation template did not have a ui factory associated with it."));
		else
		{
			//-- Create the list item for the SkeletalAnimationTemplate.
			ListItem *const listItem = dynamic_cast<ListItem*>(uiFactory->createListItem(this, m_choiceAnimationTemplate, m_fileModel));
			UNREF(listItem);

			//-- Expand the new list item.
			// if (listItem)
			//	listItem->expandItemAndParents();
		}
	}
}

// ----------------------------------------------------------------------
	
bool SpeedAnimationChoiceListItem::supportsPopupMenu() const
{
	return true;
}

// ----------------------------------------------------------------------

QPopupMenu *SpeedAnimationChoiceListItem::createPopupMenu() const
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

	IGNORE_RETURN(menu->insertItem("Clear Choice Animation Assignment", this, SLOT(clearLogicalAnimation()), static_cast<int>(QListViewItem::CTRL) + static_cast<int>(QListViewItem::Key_C))); //lint !e56 // bad type // ?
	IGNORE_RETURN(menu->insertItem("Delete Choice Animation", this, SLOT(deleteChoiceAnimation()), static_cast<int>(QListViewItem::CTRL) + static_cast<int>(QListViewItem::Key_D))); //lint !e56 // bad type // ?

	//-- Return menu to caller.
	return menu;
}

// ----------------------------------------------------------------------

bool SpeedAnimationChoiceListItem::handleKeyPress(QKeyEvent *keyEvent)
{
	if (!keyEvent)
		return false;

	//-- If Ctrl + Shift + hotkey maps to a "insert animation" command, do it.
	if (keyEvent->state() == (QObject::ShiftButton | QObject::ControlButton))
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

// ======================================================================

void SpeedAnimationChoiceListItem::createSkeletalAnimationTemplate(int templateIndex)
{
	//-- Clear the UI.
	deleteListItemChildren();

	//-- Remove existing entry.
	SkeletalAnimationTemplate const *const oldAnimationTemplate = m_choiceAnimationTemplate;
	if (m_choiceAnimationTemplate)
	{
		//-- We need to fetch a local reference for ourselves; otherwise, this animation template dies directly below.
		m_choiceAnimationTemplate->fetch();

		m_speedAnimationTemplate.deleteSpeedChoiceAnimationTemplate(m_choiceAnimationTemplate);
		m_choiceAnimationTemplate = 0;
	}

	//-- Create the new SkeletalAnimationTemplate instance.
	SkeletalAnimationTemplate *const newAnimationTemplate = SkeletalAnimationTemplateFactory::createSkeletalAnimationTemplate(templateIndex);
	NOT_NULL(newAnimationTemplate);

	m_choiceAnimationTemplate = newAnimationTemplate;

	//-- Replace the Lat animation.
	m_speedAnimationTemplate.addSpeedChoiceAnimationTemplate(newAnimationTemplate);

	//-- Create Ui under this item.
	const QtSkeletalAnimationTemplateUiFactory *uiFactory = dynamic_cast<const QtSkeletalAnimationTemplateUiFactory*>(newAnimationTemplate->getConstUiFactory());
	if (!uiFactory)
	{
		WARNING_STRICT_FATAL(true, ("new SkeletalAnimationTemplate-derived class did not have a UI factory registered.  Ui out of sync."));
		return;
	}

	ListItem *const listItem = dynamic_cast<ListItem*>(uiFactory->createListItem(this, newAnimationTemplate, m_fileModel));
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
	m_fileModel.setModifiedState(true);
}

// ----------------------------------------------------------------------

void SpeedAnimationChoiceListItem::clearLogicalAnimation()
{
	//-- Delete Ui children.
	deleteListItemChildren();

	//-- Delete the choice skeletal animation template.
	if (m_choiceAnimationTemplate)
	{
		m_speedAnimationTemplate.deleteSpeedChoiceAnimationTemplate(m_choiceAnimationTemplate);
		m_choiceAnimationTemplate = 0;
	}

	//-- Set FileModel as modified.
	m_fileModel.setModifiedState(true);
}

// ----------------------------------------------------------------------

void SpeedAnimationChoiceListItem::deleteChoiceAnimation()
{
	clearLogicalAnimation();
	delete this;
}

// ======================================================================

const QPixmap &SpeedAnimationChoiceListItem::getPixmap()
{
	static const QPixmap cs_pixmap = EmbeddedImageLoader::getPixmap("speed_choice_animation.png");
	return cs_pixmap;
}

// ======================================================================
