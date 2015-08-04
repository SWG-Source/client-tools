// ======================================================================
//
// FolderListItem.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"
#include "AnimationEditor/FolderListItem.h"

#include "AnimationEditor/EmbeddedImageLoader.h"

#include <qpixmap.h>
#include <qstring.h>

// ======================================================================

FolderListItem::FolderListItem(QListView *newParent, const std::string &newText, FolderColor folderColor, const QString *preTextSortKey) :
	ListItem(newParent, preTextSortKey),
	m_currentVisualState(-1),
	m_folderColor(folderColor)
{
	m_type = LITYPE_FolderListItem;
	NOT_NULL(newParent);

	//-- Set text.
	FolderListItem::setText(0, newText.c_str());

	//-- Set image.
	FolderListItem::updateVisuals();
}

// ----------------------------------------------------------------------

FolderListItem::FolderListItem(QListViewItem *newParent, const std::string &newText, FolderColor folderColor, const QString *preTextSortKey) :
	ListItem(newParent, preTextSortKey),
	m_currentVisualState(-1),
	m_folderColor(folderColor)
{
	m_type = LITYPE_FolderListItem;

	NOT_NULL(newParent);

	//-- Set text.
	FolderListItem::setText(0, newText.c_str());

	//-- Set image.
	FolderListItem::updateVisuals();
}

// ----------------------------------------------------------------------

FolderListItem::~FolderListItem()
{
}

// ----------------------------------------------------------------------

void FolderListItem::updateVisuals()
{
	//-- Determine what current state should be.
	int newState = 0;

	if (isOpen())
		newState = 1;

	//-- Change pixmap if current has changed.
	if (newState != m_currentVisualState)
	{
		//-- Declare pixmaps here.  These should go in an install.
		static const QPixmap cs_pixmapYellowClosed(EmbeddedImageLoader::getPixmap("folder_yellow.png"));
		static const QPixmap cs_pixmapYellowOpened(EmbeddedImageLoader::getPixmap("folder_yellow_open.png"));

		static const QPixmap cs_pixmapOrangeClosed(EmbeddedImageLoader::getPixmap("folder_orange.png"));
		static const QPixmap cs_pixmapOrangeOpened(EmbeddedImageLoader::getPixmap("folder_orange_open.png"));

		static const QPixmap cs_pixmapRedClosed(EmbeddedImageLoader::getPixmap("folder_red.png"));
		static const QPixmap cs_pixmapRedOpened(EmbeddedImageLoader::getPixmap("folder_red_open.png"));

		static const QPixmap cs_pixmapGreyClosed(EmbeddedImageLoader::getPixmap("folder_grey.png"));
		static const QPixmap cs_pixmapGreyOpened(EmbeddedImageLoader::getPixmap("folder_grey_open.png"));

		static const QPixmap cs_pixmapGreenClosed(EmbeddedImageLoader::getPixmap("folder_green.png"));
		static const QPixmap cs_pixmapGreenOpened(EmbeddedImageLoader::getPixmap("folder_green_open.png"));

		static const QPixmap cs_pixmapBlueClosed(EmbeddedImageLoader::getPixmap("folder_blue.png"));
		static const QPixmap cs_pixmapBlueOpened(EmbeddedImageLoader::getPixmap("folder_blue_open.png"));


		static const QPixmap *const cs_pixmapArray[] = 
			{ 
				&cs_pixmapYellowClosed, &cs_pixmapYellowOpened,
				&cs_pixmapOrangeClosed, &cs_pixmapOrangeOpened,
				&cs_pixmapRedClosed,    &cs_pixmapRedOpened,
				&cs_pixmapGreyClosed,   &cs_pixmapGreyOpened,
				&cs_pixmapGreenClosed,  &cs_pixmapGreenOpened,
				&cs_pixmapBlueClosed,   &cs_pixmapBlueOpened
			};

		static const int cs_pixmapEntryCount = static_cast<int>(sizeof(cs_pixmapArray) / sizeof(cs_pixmapArray[0]));

		//-- Keep track of new state.
		m_currentVisualState = newState;

		//-- Calculate pixmap index based on folder color and open state.
		const int pixmapIndex = (2 * static_cast<int>(m_folderColor)) + newState;

		//-- Set the new pixmap.
		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, pixmapIndex, cs_pixmapEntryCount);
		setPixmap(0, *cs_pixmapArray[pixmapIndex]);
	}
}

// ======================================================================
