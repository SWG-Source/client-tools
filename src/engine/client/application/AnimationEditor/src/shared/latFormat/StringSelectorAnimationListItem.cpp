// ======================================================================
//
// StringSelectorAnimationListItem.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"
#include "AnimationEditor/StringSelectorAnimationListItem.h"
#include "StringSelectorAnimationListItem.moc"

#include "AnimationEditor/EmbeddedImageLoader.h"
#include "AnimationEditor/FileModel.h"
#include "AnimationEditor/FolderListItem.h"
#include "AnimationEditor/StringSelectorChoiceListItem.h"
#include "AnimationEditor/StringSelectorVarNameListItem.h"
#include "clientSkeletalAnimation/StringSelectorSkeletalAnimationTemplate.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"

#include <qpixmap.h>
#include <qpopupmenu.h>

// ======================================================================

const QString StringSelectorAnimationListItem::cms_varNameSortKey("1:");
const QString StringSelectorAnimationListItem::cms_selectionFolderSortKey("2:");

// ======================================================================

StringSelectorAnimationListItem::StringSelectorAnimationListItem(QListViewItem *parentListItem, StringSelectorSkeletalAnimationTemplate &stringSelectorTemplate, FileModel &fileModel) :
	ListItem(parentListItem),
	m_selectorTemplate(stringSelectorTemplate),
	m_fileModel(fileModel),
	m_selectionFolderListItem(0)
{
	m_type = LITYPE_StringSelectorAnimationListItem;

	//-- Set text.
	StringSelectorAnimationListItem::setText(0, "String Selector");

	//-- Set pixmap.
	StringSelectorAnimationListItem::setPixmap(0, getPixmap());

	//-- Create children UI elements.
	buildUi();
}

// ----------------------------------------------------------------------

StringSelectorAnimationListItem::~StringSelectorAnimationListItem()
{
	//lint -esym(423, StringSelectorAnimationListItem::m_selectionFolderListItem) // Qt owns and deletes this.
	m_selectionFolderListItem = 0;
}

// ----------------------------------------------------------------------

bool StringSelectorAnimationListItem::supportsPopupMenu() const
{
	return true;
}

// ----------------------------------------------------------------------

QPopupMenu *StringSelectorAnimationListItem::createPopupMenu() const
{
	//-- Create the menu.
	QPopupMenu *menu = new QPopupMenu();

	//-- Add menu items.
	IGNORE_RETURN(menu->insertItem("New Choice Animation", this, SLOT(addChoiceAnimation()), static_cast<int>(QListViewItem::CTRL) + static_cast<int>(QListViewItem::Key_N))); //lint !e56 // bad type // ?

	//-- Return menu to caller.
	return menu;
}

// ----------------------------------------------------------------------

bool StringSelectorAnimationListItem::supportsAttachingDownStreamAnimationTemplate() const
{
	return true;
}

// ----------------------------------------------------------------------

void StringSelectorAnimationListItem::attachDownStreamAnimationTemplate(SkeletalAnimationTemplate const *animationTemplate)
{
	if (!animationTemplate)
		return;

	//-- Set specified animation as default; mark file as dirty.
	m_selectorTemplate.setDefaultSelectionAnimationTemplate(animationTemplate);
	m_selectorTemplate.addSelectionAnimationTemplateMapping(animationTemplate, ConstCharCrcLowerString("default"));
	m_fileModel.setModifiedState(true);

	//-- Rebuild the UI.
	buildUi();
}

// ----------------------------------------------------------------------

void StringSelectorAnimationListItem::populateSelectionFolder()
{
	//-- Create child nodes.
	m_selectionFolderListItem = new FolderListItem(this, "Selections", FolderListItem::FC_grey, &cms_selectionFolderSortKey);

	const int choiceTemplateCount = m_selectorTemplate.getSelectionAnimationTemplateCount();
	for (int i = 0; i < choiceTemplateCount; ++i)
	{
		//-- Get the choice animation template.
		const SkeletalAnimationTemplate *const choiceTemplate = m_selectorTemplate.fetchSelectionAnimationTemplate(i);
		if (choiceTemplate)
		{
			//-- Add the choice list item.
			ListItem *const newListItem = new StringSelectorChoiceListItem(m_selectionFolderListItem, choiceTemplate, *this, m_selectorTemplate, m_fileModel);
			UNREF(newListItem);

			//-- Release local reference --- selector has reference.  Ui never hangs on to reference.
			choiceTemplate->release();
		} //lint !e429 // newListItem not freed or returned // owned by Qt.
	}
}

// ======================================================================
// class StringSelectorAnimationListItem: public slots
// ======================================================================

void StringSelectorAnimationListItem::addChoiceAnimation()
{
	ListItem *const newListItem = new StringSelectorChoiceListItem(m_selectionFolderListItem, 0, *this, m_selectorTemplate, m_fileModel); 			//lint !esym(429, newListItem) // not freed or returned // owned by Qt.
	newListItem->expandItemAndParents();
} //lint !e429 // newListItem not freed or returned // owned by Qt.

// ======================================================================
// class StringSelectorAnimationListItem: private static member functions
// ======================================================================

const QPixmap &StringSelectorAnimationListItem::getPixmap()
{
	static const QPixmap s_pixmap = EmbeddedImageLoader::getPixmap("string_selector_animation.png");
	return s_pixmap;
}

// ======================================================================

void StringSelectorAnimationListItem::buildUi()
{
	deleteListItemChildren();
	m_selectionFolderListItem = 0;

	IGNORE_RETURN(new StringSelectorVarNameListItem(this, m_selectorTemplate, m_fileModel, &cms_varNameSortKey));
	populateSelectionFolder();
}

// ======================================================================

