// ======================================================================
//
// DirectionAnimationChoiceListItem.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"
#include "AnimationEditor/DirectionAnimationChoiceListItem.h"
#include "DirectionAnimationChoiceListItem.moc"

#include "AnimationEditor/EmbeddedImageLoader.h"
#include "AnimationEditor/FileModel.h"
#include "AnimationEditor/QtSkeletalAnimationTemplateUiFactory.h"
#include "AnimationEditor/SkeletalAnimationTemplateFactory.h"

#include <qpixmap.h>
#include <qpopupmenu.h>

// ======================================================================

const char *const DirectionAnimationChoiceListItem::cms_xMagnitudeString[] =
	{
		"",
		"right",
		"left"
	};

const char *const DirectionAnimationChoiceListItem::cms_zMagnitudeString[] =
	{
		"",
		"forward",
		"backward"
	};

// ======================================================================

DirectionAnimationChoiceListItem::DirectionAnimationChoiceListItem(QListViewItem *newParent, DirectionSkeletalAnimationTemplate &animationTemplate, DirectionSkeletalAnimationTemplate::Magnitude xMagnitude, DirectionSkeletalAnimationTemplate::Magnitude zMagnitude, FileModel &fileModel) :
	ListItem(newParent),
	m_directionAnimationTemplate(animationTemplate),
	m_fileModel(fileModel),
	m_xMagnitude(xMagnitude),
	m_zMagnitude(zMagnitude)
{
	m_type = LITYPE_DirectionAnimationChoiceListItem;
	//-- Build item name based on direction.
	std::string  newItemText(cms_zMagnitudeString[static_cast<int>(zMagnitude)]);

	if (!newItemText.empty() && *cms_xMagnitudeString[static_cast<int>(xMagnitude)])
		newItemText += ", ";

	newItemText += cms_xMagnitudeString[static_cast<int>(xMagnitude)];

	// handle case of idle
	if (newItemText.empty())
		newItemText = "idle";

	DirectionAnimationChoiceListItem::setText(0, newItemText.c_str());

	//-- Set Pixmap
	DirectionAnimationChoiceListItem::setPixmap(0, getPixmap());

	//-- Setup the UI.
	buildUi();
}

// ----------------------------------------------------------------------
	
bool DirectionAnimationChoiceListItem::supportsPopupMenu() const
{
	return true;
}

// ----------------------------------------------------------------------

QPopupMenu *DirectionAnimationChoiceListItem::createPopupMenu() const
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

bool DirectionAnimationChoiceListItem::handleKeyPress(QKeyEvent *keyEvent)
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

void DirectionAnimationChoiceListItem::createSkeletalAnimationTemplate(int templateIndex)
{
	//-- Delete Ui children.
	deleteListItemChildren();

	//-- Create the new SkeletalAnimationTemplate instance.
	SkeletalAnimationTemplate *const newAnimationTemplate = SkeletalAnimationTemplateFactory::createSkeletalAnimationTemplate(templateIndex);
	NOT_NULL(newAnimationTemplate);

	//-- Replace the Lat animation.
	int const encodedDirection = DirectionSkeletalAnimationTemplate::encodeDirectionInInt(m_xMagnitude, m_zMagnitude);
	SkeletalAnimationTemplate const *const oldAnimationTemplate = m_directionAnimationTemplate.fetchDirectionalAnimationTemplate(encodedDirection);
	m_directionAnimationTemplate.setDirectionalAnimationTemplate(m_xMagnitude, m_zMagnitude, newAnimationTemplate);

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

void DirectionAnimationChoiceListItem::clearLogicalAnimation()
{
	//-- Delete Ui children.
	deleteListItemChildren();

	//-- Reset the Lat animation.
	m_directionAnimationTemplate.setDirectionalAnimationTemplate(m_xMagnitude, m_zMagnitude, 0);

	//-- Set FileModel as modified.
	m_fileModel.setModifiedState(true);
}

// ----------------------------------------------------------------------

DirectionSkeletalAnimationTemplate::Magnitude DirectionAnimationChoiceListItem::getXMagnitude() const
{
	return m_xMagnitude;
}

// ----------------------------------------------------------------------

DirectionSkeletalAnimationTemplate::Magnitude DirectionAnimationChoiceListItem::getZMagnitude() const
{
	return m_zMagnitude;
}

// ----------------------------------------------------------------------

void DirectionAnimationChoiceListItem::buildUi()
{
	//-- Clear all child UI components.
	deleteListItemChildren();

	// Get the SkeletalAnimationTemplate corresonding to this direction.
	const int encodedDirection = DirectionSkeletalAnimationTemplate::encodeDirectionInInt(m_xMagnitude, m_zMagnitude);
	SkeletalAnimationTemplate *directionChoiceAnimationTemplate = const_cast<SkeletalAnimationTemplate*>(m_directionAnimationTemplate.fetchDirectionalAnimationTemplate(encodedDirection));
	if (directionChoiceAnimationTemplate)
	{
		//-- Retrieve the UiFactory for the SkeletalAnimationTemplate.
		const QtSkeletalAnimationTemplateUiFactory *const uiFactory = dynamic_cast<const QtSkeletalAnimationTemplateUiFactory*>(directionChoiceAnimationTemplate->getConstUiFactory());

		if (!uiFactory)
			WARNING_STRICT_FATAL(true, ("An animation template did not have a ui factory associated with it."));
		else
		{
			//-- Create the list item for the SkeletalAnimationTemplate.
			ListItem *const listItem = dynamic_cast<ListItem*>(uiFactory->createListItem(this, directionChoiceAnimationTemplate, m_fileModel));
			UNREF(listItem);

			//-- Release local reference.
			directionChoiceAnimationTemplate->release();
		}
	}
}

// ======================================================================

const QPixmap &DirectionAnimationChoiceListItem::getPixmap()
{
	static const QPixmap cs_pixmap = EmbeddedImageLoader::getPixmap("directional_choice_animation.png");
	return cs_pixmap;
}

// ======================================================================
