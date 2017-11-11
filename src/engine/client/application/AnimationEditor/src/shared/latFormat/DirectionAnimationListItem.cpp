// ======================================================================
//
// DirectionAnimationListItem.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"
#include "AnimationEditor/DirectionAnimationListItem.h"

#include "AnimationEditor/DirectionAnimationChoiceListItem.h"
#include "AnimationEditor/EmbeddedImageLoader.h"
#include "AnimationEditor/FileModel.h"
#include "clientSkeletalAnimation/DirectionSkeletalAnimationTemplate.h"

#include <qpixmap.h>

// ======================================================================

const QString DirectionAnimationListItem::cms_directionItemText("Directional Animation");

// ======================================================================

DirectionAnimationListItem::DirectionAnimationListItem(QListViewItem *newParent, DirectionSkeletalAnimationTemplate &directionAnimationTemplate, FileModel &fileModel) :
	ListItem(newParent),
	m_fileModel(fileModel),
	m_directionAnimationTemplate(directionAnimationTemplate)
{
	m_type = LITYPE_DirectionAnimationListItem;
	//-- Set item text.
	DirectionAnimationListItem::setText(0, cms_directionItemText);

	//-- Set Pixmap.
	DirectionAnimationListItem::setPixmap(0, getPixmap());

	//-- Create child ui.

	// forward
	IGNORE_RETURN(new DirectionAnimationChoiceListItem(this, m_directionAnimationTemplate, DirectionSkeletalAnimationTemplate::M_zero,     DirectionSkeletalAnimationTemplate::M_positive, m_fileModel)); //lint !e1524 // new constructor for class with no destructor // it's okay, Qt owns it.
	IGNORE_RETURN(new DirectionAnimationChoiceListItem(this, m_directionAnimationTemplate, DirectionSkeletalAnimationTemplate::M_negative, DirectionSkeletalAnimationTemplate::M_positive, m_fileModel)); //lint !e1524 // new constructor for class with no destructor // it's okay, Qt owns it.
	IGNORE_RETURN(new DirectionAnimationChoiceListItem(this, m_directionAnimationTemplate, DirectionSkeletalAnimationTemplate::M_positive, DirectionSkeletalAnimationTemplate::M_positive, m_fileModel)); //lint !e1524 // new constructor for class with no destructor // it's okay, Qt owns it.

	// walk left, right
	IGNORE_RETURN(new DirectionAnimationChoiceListItem(this, m_directionAnimationTemplate, DirectionSkeletalAnimationTemplate::M_negative, DirectionSkeletalAnimationTemplate::M_zero, m_fileModel)); //lint !e1524 // new constructor for class with no destructor // it's okay, Qt owns it.
	IGNORE_RETURN(new DirectionAnimationChoiceListItem(this, m_directionAnimationTemplate, DirectionSkeletalAnimationTemplate::M_positive, DirectionSkeletalAnimationTemplate::M_zero, m_fileModel)); //lint !e1524 // new constructor for class with no destructor // it's okay, Qt owns it.

	// backward
	IGNORE_RETURN(new DirectionAnimationChoiceListItem(this, m_directionAnimationTemplate, DirectionSkeletalAnimationTemplate::M_zero,     DirectionSkeletalAnimationTemplate::M_negative, m_fileModel)); //lint !e1524 // new constructor for class with no destructor // it's okay, Qt owns it.
	IGNORE_RETURN(new DirectionAnimationChoiceListItem(this, m_directionAnimationTemplate, DirectionSkeletalAnimationTemplate::M_negative, DirectionSkeletalAnimationTemplate::M_negative, m_fileModel)); //lint !e1524 // new constructor for class with no destructor // it's okay, Qt owns it.
	IGNORE_RETURN(new DirectionAnimationChoiceListItem(this, m_directionAnimationTemplate, DirectionSkeletalAnimationTemplate::M_positive, DirectionSkeletalAnimationTemplate::M_negative, m_fileModel)); //lint !e1524 // new constructor for class with no destructor // it's okay, Qt owns it.

	// idle
	IGNORE_RETURN(new DirectionAnimationChoiceListItem(this, m_directionAnimationTemplate, DirectionSkeletalAnimationTemplate::M_zero, DirectionSkeletalAnimationTemplate::M_zero, m_fileModel)); //lint !e1524 // new constructor for class with no destructor // it's okay, Qt owns it.
}

// ----------------------------------------------------------------------

bool DirectionAnimationListItem::supportsAttachingDownStreamAnimationTemplate() const
{
	return true;
}

// ----------------------------------------------------------------------

void DirectionAnimationListItem::attachDownStreamAnimationTemplate(SkeletalAnimationTemplate const *animationTemplate)
{
	//-- Set the forward direction animation; mark file as modified.
	m_directionAnimationTemplate.setDirectionalAnimationTemplate(DirectionSkeletalAnimationTemplate::M_zero, DirectionSkeletalAnimationTemplate::M_positive, animationTemplate);
	m_fileModel.setModifiedState(true);

	//-- Find the forward DirectionAnimationChoiceListItem.
	for (QListViewItem *item = firstChild(); item != NULL; item = item->nextSibling())
	{
		DirectionAnimationChoiceListItem *const listItem = dynamic_cast<DirectionAnimationChoiceListItem*>(item);
		if (listItem && (listItem->getXMagnitude() == DirectionSkeletalAnimationTemplate::M_zero) && (listItem->getZMagnitude() == DirectionSkeletalAnimationTemplate::M_positive))
		{
			// Found the forward direction list item, fix the UI for it.
			listItem->buildUi();
			break;
		}
	}
}

// ======================================================================

const QPixmap &DirectionAnimationListItem::getPixmap()
{
	static const QPixmap cs_pixmap = EmbeddedImageLoader::getPixmap("directional_animation.png");
	return cs_pixmap;
}

// ======================================================================
