// ======================================================================
//
// LogicalAnimationListItem.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"
#include "AnimationEditor/LogicalAnimationListItem.h"
#include "LogicalAnimationListItem.moc"

#include "AnimationEditor/EmbeddedImageLoader.h"
#include "AnimationEditor/FileModel.h"
#include "AnimationEditor/QtSkeletalAnimationTemplateUiFactory.h"
#include "AnimationEditor/SkeletalAnimationTemplateFactory.h"
#include "clientSkeletalAnimation/AnimationStateHierarchyTemplate.h"
#include "clientSkeletalAnimation/LogicalAnimationTableTemplate.h"
#include "clientSkeletalAnimation/SkeletalAnimationTemplate.h"
#include "sharedFoundation/CrcLowerString.h"

#include <qpopupmenu.h>
#include <string>

// ======================================================================

LogicalAnimationListItem::LogicalAnimationListItem(QListViewItem *newParent, LogicalAnimationTableTemplate &latTemplate, const CrcLowerString &logicalAnimationName, FileModel &fileModel) :
	ListItem(newParent),
	m_latTemplate(latTemplate),
	m_logicalAnimationName(new CrcLowerString(logicalAnimationName)),
	m_fileModel(fileModel)
{

	m_type = LITYPE_LogicalAnimationListItem;

	//-- Set item text.
	LogicalAnimationListItem::setText(0, logicalAnimationName.getString());

	//-- Set pixmap.
	LogicalAnimationListItem::setPixmap(0, getPixmap());
}

// ----------------------------------------------------------------------

LogicalAnimationListItem::~LogicalAnimationListItem()
{
	delete m_logicalAnimationName;
}

// ----------------------------------------------------------------------

bool LogicalAnimationListItem::supportsPopupMenu() const
{
	return true;
}

// ----------------------------------------------------------------------

QPopupMenu *LogicalAnimationListItem::createPopupMenu() const
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

bool LogicalAnimationListItem::handleKeyPress(QKeyEvent *keyEvent)
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

void LogicalAnimationListItem::createSkeletalAnimationTemplate(int templateIndex)
{
	//-- Delete Ui children.
	deleteListItemChildren();

	//-- Create the new SkeletalAnimationTemplate instance.
	SkeletalAnimationTemplate *const newAnimationTemplate = SkeletalAnimationTemplateFactory::createSkeletalAnimationTemplate(templateIndex);
	NOT_NULL(newAnimationTemplate);

	//-- Replace the Lat animation.
	SkeletalAnimationTemplate const *const oldAnimationTemplate = m_latTemplate.fetchConstAnimationTemplate(*m_logicalAnimationName);
	m_latTemplate.setAnimationTemplate(*m_logicalAnimationName, newAnimationTemplate);

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
		//-- Show the item.
		listItem->expandItemAndParents();

		//-- Insert processing: handle reattaching old animation.
		if (oldAnimationTemplate && listItem->supportsAttachingDownStreamAnimationTemplate())
			listItem->attachDownStreamAnimationTemplate(oldAnimationTemplate);
	}

	//-- Release local references.
	if (oldAnimationTemplate)
		oldAnimationTemplate->release();

	//-- Set FileModel as modified.
	m_fileModel.setModifiedState(true);
}

// ----------------------------------------------------------------------

void LogicalAnimationListItem::clearLogicalAnimation()
{
	//-- Delete Ui children.
	deleteListItemChildren();

	//-- Reset the Lat animation.
	m_latTemplate.setAnimationTemplate(*m_logicalAnimationName, 0);

	//-- Set FileModel as modified.
	m_fileModel.setModifiedState(true);
}

// ======================================================================

const QPixmap &LogicalAnimationListItem::getPixmap()
{
	static const QPixmap cs_pixmap = EmbeddedImageLoader::getPixmap("logical_animation_name.png");
	return cs_pixmap;
}

// ======================================================================
