// ======================================================================
//
// YawAnimationListItem.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"
#include "AnimationEditor/YawAnimationListItem.h"
#include "YawAnimationListItem.moc"

#include "AnimationEditor/DirectionAnimationChoiceListItem.h"
#include "AnimationEditor/EmbeddedImageLoader.h"
#include "AnimationEditor/FileModel.h"
#include "AnimationEditor/QtSkeletalAnimationTemplateUiFactory.h"
#include "AnimationEditor/SkeletalAnimationTemplateFactory.h"
#include "clientSkeletalAnimation/YawSkeletalAnimationTemplate.h"

#include <qpixmap.h>
#include <qpopupmenu.h>

// ======================================================================

const QString YawAnimationListItem::cms_yawItemText("Yaw Chooser Animation");

// ======================================================================

const QString YawAnimationListItem::ChoiceListItem::cms_yawChoiceItemText[3] =
{
	QString("Yaw Left"),
	QString("Yaw Right"),
	QString("No Yaw")
};

// ======================================================================
// class YawAnimationListItem::ChoiceListItem
// ======================================================================

YawAnimationListItem::ChoiceListItem::ChoiceListItem(YawAnimationListItem &newParent, YawChoice yawChoice) :
	ListItem(&newParent),
	m_parent(newParent),
	m_yawChoice(yawChoice)
{
	m_type = LITYPE_YawAnimationListItem_ChoiceListItem;

	//-- Set item text.
	ChoiceListItem::setText(0, cms_yawChoiceItemText[static_cast<size_t>(yawChoice)]);

	//-- Set Pixmap.
	ChoiceListItem::setPixmap(0, getPixmap());

	//-- Build UI.
	buildUi();
}

// ----------------------------------------------------------------------

bool YawAnimationListItem::ChoiceListItem::supportsPopupMenu() const
{
	return true;
}

// ----------------------------------------------------------------------

QPopupMenu *YawAnimationListItem::ChoiceListItem::createPopupMenu() const
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

	//-- Return menu to caller.
	return menu;
}

// ----------------------------------------------------------------------

bool YawAnimationListItem::ChoiceListItem::handleKeyPress(QKeyEvent *keyEvent)
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

// ----------------------------------------------------------------------

YawAnimationListItem::YawChoice YawAnimationListItem::ChoiceListItem::getYawChoice() const
{
	return m_yawChoice;
}

// ----------------------------------------------------------------------

void YawAnimationListItem::ChoiceListItem::buildUi()
{
	//-- Clear the UI.
	deleteListItemChildren();

	//-- Create child animation UI.
	SkeletalAnimationTemplate const *childAnimationTemplate = fetchYawChoiceAnimationTemplate();

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

void YawAnimationListItem::ChoiceListItem::createSkeletalAnimationTemplate(int templateIndex)
{
	//-- Delete Ui children.
	deleteListItemChildren();

	//-- Create the new SkeletalAnimationTemplate instance.
	SkeletalAnimationTemplate const *const oldAnimationTemplate = fetchYawChoiceAnimationTemplate();

	SkeletalAnimationTemplate *const newAnimationTemplate = SkeletalAnimationTemplateFactory::createSkeletalAnimationTemplate(templateIndex);
	NOT_NULL(newAnimationTemplate);

	//-- Replace the Lat animation.
	YawSkeletalAnimationTemplate &yawAnimationTemplate = m_parent.getYawAnimationTemplate();

	switch (m_yawChoice)
	{
		case YC_yawLeft:
			yawAnimationTemplate.setYawLeftAnimationTemplate(newAnimationTemplate);
			break;

		case YC_yawRight:
			yawAnimationTemplate.setYawRightAnimationTemplate(newAnimationTemplate);
			break;

		case YC_noYaw:
			yawAnimationTemplate.setNoYawAnimationTemplate(newAnimationTemplate);
			break;
	}

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

void YawAnimationListItem::ChoiceListItem::clearLogicalAnimation()
{
	//-- Delete Ui children.
	deleteListItemChildren();

	//-- Replace the Lat animation.
	YawSkeletalAnimationTemplate &yawAnimationTemplate = m_parent.getYawAnimationTemplate();

	switch (m_yawChoice)
	{
		case YC_yawLeft:
			yawAnimationTemplate.setYawLeftAnimationTemplate(0);
			break;

		case YC_yawRight:
			yawAnimationTemplate.setYawRightAnimationTemplate(0);
			break;

		case YC_noYaw:
			yawAnimationTemplate.setNoYawAnimationTemplate(0);
			break;
	}


	//-- Set FileModel as modified.
	m_parent.getFileModel().setModifiedState(true);
}

// ======================================================================

const QPixmap &YawAnimationListItem::ChoiceListItem::getPixmap()
{
	static const QPixmap cs_pixmap = EmbeddedImageLoader::getPixmap("yaw_animation_choice.png");
	return cs_pixmap;
}

// ======================================================================

SkeletalAnimationTemplate const *YawAnimationListItem::ChoiceListItem::fetchYawChoiceAnimationTemplate() const
{
	YawSkeletalAnimationTemplate &yawAnimationTemplate = m_parent.getYawAnimationTemplate();
	SkeletalAnimationTemplate const *childAnimationTemplate = 0;

	switch (m_yawChoice)
	{
		case YC_yawLeft:
			childAnimationTemplate = yawAnimationTemplate.fetchYawLeftAnimationTemplate();
			break;

		case YC_yawRight:
			childAnimationTemplate = yawAnimationTemplate.fetchYawRightAnimationTemplate();
			break;

		case YC_noYaw:
			childAnimationTemplate = yawAnimationTemplate.fetchNoYawAnimationTemplate();
			break;
	}

	return childAnimationTemplate;
}

// ======================================================================
// class YawAnimationListItem: public member functions
// ======================================================================

YawAnimationListItem::YawAnimationListItem(QListViewItem *newParent, YawSkeletalAnimationTemplate &yawAnimationTemplate, FileModel &fileModel) :
	ListItem(newParent),
	m_fileModel(fileModel),
	m_yawAnimationTemplate(yawAnimationTemplate)
{
	m_type = LITYPE_YawAnimationListItem;

	//-- Set item text.
	YawAnimationListItem::setText(0, cms_yawItemText);

	//-- Set Pixmap.
	YawAnimationListItem::setPixmap(0, getPixmap());

	//-- Create child ui.
	IGNORE_RETURN(new ChoiceListItem(*this, YC_yawLeft));  //lint !e1524 // new constructor for class with no destructor // it's okay, Qt owns it.
	IGNORE_RETURN(new ChoiceListItem(*this, YC_yawRight)); //lint !e1524 // new constructor for class with no destructor // it's okay, Qt owns it.
	IGNORE_RETURN(new ChoiceListItem(*this, YC_noYaw));    //lint !e1524 // new constructor for class with no destructor // it's okay, Qt owns it.
}

// ----------------------------------------------------------------------

bool YawAnimationListItem::supportsAttachingDownStreamAnimationTemplate() const
{
	return true;
}

// ----------------------------------------------------------------------

void YawAnimationListItem::attachDownStreamAnimationTemplate(SkeletalAnimationTemplate const *animationTemplate)
{
	if (!animationTemplate)
		return;

	//-- Set no-yaw child to this animation; mark file as dirty.
	m_yawAnimationTemplate.setNoYawAnimationTemplate(animationTemplate);
	m_fileModel.setModifiedState(true);

	//-- Find the forward DirectionAnimationChoiceListItem.
	for (QListViewItem *item = firstChild(); item != NULL; item = item->nextSibling())
	{
		ChoiceListItem *const listItem = dynamic_cast<ChoiceListItem*>(item);
		if (listItem && (listItem->getYawChoice() == YC_noYaw))
		{
			// Found the no yaw list item, fix the UI for it.
			listItem->buildUi();
			break;
		}
	}

}

// ======================================================================
// class YawAnimationListItem: private static member functions
// ======================================================================

const QPixmap &YawAnimationListItem::getPixmap()
{
	static const QPixmap cs_pixmap = EmbeddedImageLoader::getPixmap("yaw_animation.png");
	return cs_pixmap;
}

// ======================================================================
