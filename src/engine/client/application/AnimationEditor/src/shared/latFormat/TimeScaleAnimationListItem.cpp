// ======================================================================
//
// TimeScaleAnimationListItem.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"
#include "AnimationEditor/TimeScaleAnimationListItem.h"
#include "TimeScaleAnimationListItem.moc"

#include "AnimationEditor/DirectionAnimationChoiceListItem.h"
#include "AnimationEditor/EmbeddedImageLoader.h"
#include "AnimationEditor/FileModel.h"
#include "AnimationEditor/QtSkeletalAnimationTemplateUiFactory.h"
#include "AnimationEditor/SkeletalAnimationTemplateFactory.h"
#include "clientSkeletalAnimation/TimeScaleSkeletalAnimationTemplate.h"

#include <qinputdialog.h>
#include <qpixmap.h>
#include <qpopupmenu.h>

// ======================================================================

namespace TimeScaleAnimationListItemNamespace
{
	QString const  cs_scaledAnimationItemText("Scaled Animation");
}

using namespace TimeScaleAnimationListItemNamespace;

// ======================================================================
// class TimeScaleAnimationListItem::ScaledAnimationListItem
// ======================================================================

TimeScaleAnimationListItem::ScaledAnimationListItem::ScaledAnimationListItem(TimeScaleAnimationListItem &newParent) :
	ListItem(&newParent),
	m_parent(newParent)
{
	m_type = LITYPE_TimeScaleAnimationListItem_ScaledAnimationListItem;

	//-- Set item text.
	ScaledAnimationListItem::setText(0, cs_scaledAnimationItemText);

	//-- Set Pixmap.
	ScaledAnimationListItem::setPixmap(0, getPixmap());

	//-- Create child animation UI.
	// Get the base SkeletalAnimationTemplate for the time scaler.
	TimeScaleSkeletalAnimationTemplate     &timeScaleAnimationTemplate = m_parent.getTimeScaleAnimationTemplate();
	SkeletalAnimationTemplate const *const  childAnimationTemplate     = timeScaleAnimationTemplate.fetchBaseAnimationTemplate();

	if (childAnimationTemplate)
	{
		//-- Retrieve the UiFactory for the SkeletalAnimationTemplate.
		const QtSkeletalAnimationTemplateUiFactory *const uiFactory = dynamic_cast<const QtSkeletalAnimationTemplateUiFactory*>(childAnimationTemplate->getConstUiFactory());

		if (!uiFactory)
			WARNING_STRICT_FATAL(true, ("A referenced animation template did not have a ui factory associated with it."));
		else
		{
			//-- Create the list item for the SkeletalAnimationTemplate.
			ListItem *const listItem = dynamic_cast<ListItem*>(uiFactory->createListItem(this, const_cast<SkeletalAnimationTemplate*>(childAnimationTemplate), m_parent.getFileModel()));
			UNREF(listItem);

			//-- Release local reference.
			childAnimationTemplate->release();
		}
	}

}

// ----------------------------------------------------------------------

bool TimeScaleAnimationListItem::ScaledAnimationListItem::supportsPopupMenu() const
{
	return true;
}

// ----------------------------------------------------------------------

QPopupMenu *TimeScaleAnimationListItem::ScaledAnimationListItem::createPopupMenu() const
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

bool TimeScaleAnimationListItem::ScaledAnimationListItem::handleKeyPress(QKeyEvent *keyEvent)
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

void TimeScaleAnimationListItem::ScaledAnimationListItem::createSkeletalAnimationTemplate(int templateIndex)
{
	//-- Delete Ui children.
	deleteListItemChildren();

	//-- Create the new SkeletalAnimationTemplate instance.
	SkeletalAnimationTemplate *const newAnimationTemplate = SkeletalAnimationTemplateFactory::createSkeletalAnimationTemplate(templateIndex);
	NOT_NULL(newAnimationTemplate);

	TimeScaleSkeletalAnimationTemplate &timeScaleAnimationTemplate = m_parent.getTimeScaleAnimationTemplate();
	SkeletalAnimationTemplate const *oldAnimationTemplate  = timeScaleAnimationTemplate.fetchBaseAnimationTemplate();

	//-- Replace the Lat animation.
	timeScaleAnimationTemplate.setBaseAnimationTemplate(newAnimationTemplate);

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

void TimeScaleAnimationListItem::ScaledAnimationListItem::clearLogicalAnimation()
{
	//-- Delete Ui children.
	deleteListItemChildren();

	//-- Replace the Lat animation.
	m_parent.getTimeScaleAnimationTemplate().setBaseAnimationTemplate(0);

	//-- Set FileModel as modified.
	m_parent.getFileModel().setModifiedState(true);
}

// ======================================================================

const QPixmap &TimeScaleAnimationListItem::ScaledAnimationListItem::getPixmap()
{
	static const QPixmap cs_pixmap = EmbeddedImageLoader::getPixmap("time_scale_scaled_animation.png");
	return cs_pixmap;
}

// ======================================================================
// class TimeScaleAnimationListItem: PUBLIC
// ======================================================================

TimeScaleAnimationListItem::TimeScaleAnimationListItem(QListViewItem *newParent, TimeScaleSkeletalAnimationTemplate &timeScaleAnimationTemplate, FileModel &fileModel) :
	ListItem(newParent),
	m_fileModel(fileModel),
	m_timeScaleAnimationTemplate(timeScaleAnimationTemplate)
{
	m_type = LITYPE_TimeScaleAnimationListItem;

	//-- Set item text.
	TimeScaleAnimationListItem::setText(0, getItemText());

	//-- Set Pixmap.
	TimeScaleAnimationListItem::setPixmap(0, getPixmap());

	//-- Create child ui.
	IGNORE_RETURN(new ScaledAnimationListItem(*this));  //lint !e1524 // new constructor for class with no destructor // it's okay, Qt owns it.
}

// ----------------------------------------------------------------------

void TimeScaleAnimationListItem::doDoubleClick()
{
  bool ok = FALSE;
  double const result = QInputDialog::getDouble(tr("Time Scale Animation"), tr("Time Scale Value"), static_cast<double>(m_timeScaleAnimationTemplate.getTimeScale()), 0.001, 1000.0, 3, &ok);
  if (ok)
	{
		//-- Set the new time scale.
		m_timeScaleAnimationTemplate.setTimeScale(static_cast<float>(result));
		m_fileModel.setModifiedState(true);

		//-- Update item text.
		TimeScaleAnimationListItem::setText(0, getItemText());
	}
}

// ----------------------------------------------------------------------

bool TimeScaleAnimationListItem::supportsAttachingDownStreamAnimationTemplate() const
{
	return true;
}

// ----------------------------------------------------------------------

void TimeScaleAnimationListItem::attachDownStreamAnimationTemplate(SkeletalAnimationTemplate const *animationTemplate)
{
	//-- Set base animation.
	m_timeScaleAnimationTemplate.setBaseAnimationTemplate(animationTemplate);

	//-- Recreate UI.
	deleteListItemChildren();
	IGNORE_RETURN(new ScaledAnimationListItem(*this));  //lint !e1524 // new constructor for class with no destructor // it's okay, Qt owns it.
	
	//-- Mark file as modified.
	m_fileModel.setModifiedState(true);
}

// ----------------------------------------------------------------------

FileModel &TimeScaleAnimationListItem::getFileModel()
{
	return m_fileModel; //lint !e1536 // Exposing low access member.  That's the intent.
}

// ----------------------------------------------------------------------

TimeScaleSkeletalAnimationTemplate &TimeScaleAnimationListItem::getTimeScaleAnimationTemplate()
{
	return m_timeScaleAnimationTemplate; //lint !e1536 // Exposing low access member.  That's the intent.
}

// ----------------------------------------------------------------------

TimeScaleSkeletalAnimationTemplate const &TimeScaleAnimationListItem::getTimeScaleAnimationTemplate() const
{
	return m_timeScaleAnimationTemplate;
}

// ======================================================================
// class TimeScaleAnimationListItem: PRIVATE STATIC
// ======================================================================

const QPixmap &TimeScaleAnimationListItem::getPixmap()
{
	static const QPixmap cs_pixmap = EmbeddedImageLoader::getPixmap("time_scale_animation.png");
	return cs_pixmap;
}

// ======================================================================
// class TimeScaleAnimationListItem: PRIVATE
// ======================================================================

QString TimeScaleAnimationListItem::getItemText() const
{
	char buffer[128];

	sprintf(buffer, "Time Scaler (%gx)", getTimeScaleAnimationTemplate().getTimeScale());
	return buffer;
}

// ======================================================================
