// ======================================================================
//
// SpeedAnimationListItem.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"
#include "AnimationEditor/SpeedAnimationListItem.h"
#include "SpeedAnimationListItem.moc"

#include "AnimationEditor/SpeedAnimationChoiceListItem.h"
#include "AnimationEditor/EmbeddedImageLoader.h"
#include "AnimationEditor/FileModel.h"
#include "clientSkeletalAnimation/SpeedSkeletalAnimationTemplate.h"

#include <qpixmap.h>
#include <qpopupmenu.h>

// ======================================================================

const QString SpeedAnimationListItem::cms_speedItemText("Speed Animation");

// ======================================================================

SpeedAnimationListItem::SpeedAnimationListItem(QListViewItem *newParent, SpeedSkeletalAnimationTemplate &speedAnimationTemplate, FileModel &fileModel) :
	ListItem(newParent),
	m_fileModel(fileModel),
	m_speedAnimationTemplate(speedAnimationTemplate)
{
	m_type = LITYPE_SpeedAnimationListItem;

	//-- Set item text.
	SpeedAnimationListItem::setText(0, cms_speedItemText);

	//-- Set Pixmap.
	SpeedAnimationListItem::setPixmap(0, getPixmap());

	//-- Create child ui.
	const int choiceTemplateCount = speedAnimationTemplate.getSpeedChoiceCount();
	for (int i = 0; i < choiceTemplateCount; ++i)
	{
		SkeletalAnimationTemplate *const choiceAnimationTemplate = const_cast<SkeletalAnimationTemplate*>(speedAnimationTemplate.fetchSpeedChoiceAnimationTemplate(i));
		IGNORE_RETURN(new SpeedAnimationChoiceListItem(this, m_speedAnimationTemplate, choiceAnimationTemplate, m_fileModel)); //lint !e1524 // new constructor for class with no destructor // it's okay, Qt owns it.

		// Release local reference --- I really don't want the UI element to hold a reference to the template.
		// The UI element will die prior to this node dying, which means this speed template will have a reference to the animation
		// template already.
		if (choiceAnimationTemplate)
			choiceAnimationTemplate->release();
	}
}

// ----------------------------------------------------------------------

bool SpeedAnimationListItem::supportsPopupMenu() const
{
	return true;
}

// ----------------------------------------------------------------------

QPopupMenu *SpeedAnimationListItem::createPopupMenu() const
{
	//-- Create the menu.
	QPopupMenu *menu = new QPopupMenu();

	//-- Add menu items.
	IGNORE_RETURN(menu->insertItem("New Speed Choice Animation", this, SLOT(createNewChoiceTemplate()), static_cast<int>(QListViewItem::CTRL) + static_cast<int>(QListViewItem::Key_N))); //lint !e56 // bad type // ?

	//-- Return menu to caller.
	return menu;
}

// ----------------------------------------------------------------------

bool SpeedAnimationListItem::supportsAttachingDownStreamAnimationTemplate() const
{
	return true;
}

// ----------------------------------------------------------------------

void SpeedAnimationListItem::attachDownStreamAnimationTemplate(SkeletalAnimationTemplate const *animationTemplate)
{
	if (!animationTemplate)
		return;

	//-- Add the animation as a speed choice; set file as modified.
	m_speedAnimationTemplate.addSpeedChoiceAnimationTemplate(animationTemplate);
	m_fileModel.setModifiedState(true);

	IGNORE_RETURN(new SpeedAnimationChoiceListItem(this, m_speedAnimationTemplate, const_cast<SkeletalAnimationTemplate*>(animationTemplate), m_fileModel));
}

// ======================================================================

void SpeedAnimationListItem::createNewChoiceTemplate()
{
	IGNORE_RETURN(new SpeedAnimationChoiceListItem(this, m_speedAnimationTemplate, 0, m_fileModel));
}

// ======================================================================

const QPixmap &SpeedAnimationListItem::getPixmap()
{
	static const QPixmap cs_pixmap = EmbeddedImageLoader::getPixmap("speed_animation.png");
	return cs_pixmap;
}

// ======================================================================
