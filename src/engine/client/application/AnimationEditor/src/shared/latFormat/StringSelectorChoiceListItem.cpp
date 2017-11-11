// ======================================================================
//
// StringSelectorChoiceListItem.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"
#include "AnimationEditor/StringSelectorChoiceListItem.h"
#include "StringSelectorChoiceListItem.moc"

#include "AnimationEditor/AnimationEditorGameWorld.h"
#include "AnimationEditor/EmbeddedImageLoader.h"
#include "AnimationEditor/FileModel.h"
#include "AnimationEditor/FolderListItem.h"
#include "AnimationEditor/QtSkeletalAnimationTemplateUiFactory.h"
#include "AnimationEditor/SkeletalAnimationTemplateFactory.h"
#include "clientSkeletalAnimation/AnimationEnvironment.h"
#include "clientSkeletalAnimation/SkeletalAnimationTemplate.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/StringSelectorSkeletalAnimationTemplate.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedObject/Object.h"

#include <algorithm>
#include <qpixmap.h>
#include <qpopupmenu.h>

// ======================================================================

const QString StringSelectorChoiceListItem::cms_animationSortKey("1:");
const QString StringSelectorChoiceListItem::cms_valueFolderSortKey("2:");

int           StringSelectorChoiceListItem::ms_newValueCounter;

// ======================================================================
// class StringSelectorChoiceListItem: public member functions
// ======================================================================

StringSelectorChoiceListItem::StringSelectorChoiceListItem(QListViewItem *newParent, const SkeletalAnimationTemplate *choiceTemplate, StringSelectorAnimationListItem &selectorListItem, StringSelectorSkeletalAnimationTemplate &selectorTemplate, FileModel &fileModel) :
	ListItem(newParent),
	m_choiceTemplate(choiceTemplate),
	m_selectorListItem(selectorListItem),
	m_selectorTemplate(selectorTemplate),
	m_fileModel(fileModel),
	m_valueFolder(0)
{
	m_type = LITYPE_StringSelectorChoiceListItem;

	//-- Set text.
	StringSelectorChoiceListItem::setText(0, "Selection");

	//-- Set pixmap.
	StringSelectorChoiceListItem::setPixmap(0, getPixmap());

	//-- Create animation item (the list item that holds the animation).
	IGNORE_RETURN(new AnimationListItem(*this, &cms_animationSortKey));

	//-- Create Value folder (contains the values that map to this choice animation template).
	m_valueFolder = new FolderListItem(this, "Values", FolderListItem::FC_grey, &cms_valueFolderSortKey);

	//-- Build values.
	if (m_choiceTemplate)
	{
		const int valueCount = m_selectorTemplate.getSelectionValueCount(m_choiceTemplate);
		for (int i = 0; i < valueCount; ++i)
		{
			const CrcLowerString &value = m_selectorTemplate.getSelectionValue(m_choiceTemplate, i);
			ListItem *const newListItem = new ValueListItem(m_valueFolder, *this, value.getString(), m_fileModel, false);
			UNREF(newListItem);

			// newListItem->expandItemAndParents();
		} //lint !e429 // newListItem not freed or returned // Qt owns it.
	}
}

// ----------------------------------------------------------------------

StringSelectorChoiceListItem::~StringSelectorChoiceListItem()
{
	m_choiceTemplate = 0;
	m_valueFolder    = 0; //lint !e423  // creation of memory leak // no, Qt owns and deletes.
}

// ----------------------------------------------------------------------

bool StringSelectorChoiceListItem::supportsPopupMenu() const
{
	return true;
}

// ----------------------------------------------------------------------

QPopupMenu *StringSelectorChoiceListItem::createPopupMenu() const
{
	//-- Create the menu.
	QPopupMenu *menu = new QPopupMenu();

	//-- Add menu items.
	IGNORE_RETURN(menu->insertItem("Create New Value", this, SLOT(createNewValue()), static_cast<int>(QListViewItem::CTRL) + static_cast<int>(QListViewItem::Key_N))); //lint !e56 // bad type // ?

	//-- Return menu to caller.
	return menu;
}

// ======================================================================
// class StringSelectorChoiceListItem: public slot member functions
// ======================================================================

void StringSelectorChoiceListItem::createNewValue()
{
	//-- Only allow creation of values when there is an animation specified.
	if (!m_choiceTemplate)
		return;

	//-- Find an unmapped name.
	std::string  newValueText;
	do
	{
		newValueText = getNewValueText();
	} while (m_selectorTemplate.hasValue(CrcLowerString(newValueText.c_str())));

	//-- Create the value item, expand it.
	ListItem *newListItem = new ValueListItem(m_valueFolder, *this, newValueText, m_fileModel, true);
	newListItem->expandItemAndParents();
} //lint !e429 // newListItem not freed or returned // Qt owns it.

// ======================================================================
// class StringSelectorChoiceListItem: private static member functions
// ======================================================================

const QPixmap &StringSelectorChoiceListItem::getPixmap()
{
	static const QPixmap cs_pixmap = EmbeddedImageLoader::getPixmap("bullet_grey.png");
	return cs_pixmap;
}

// ----------------------------------------------------------------------

std::string StringSelectorChoiceListItem::getNewValueText()
{
	++ms_newValueCounter;
	
	char buffer[64];
	sprintf(buffer, "unnamed%d", ms_newValueCounter);

	return buffer;
}

// ======================================================================
// class StringSelectorChoiceListItem::AnimationListItem: public member functions
// ======================================================================

StringSelectorChoiceListItem::AnimationListItem::AnimationListItem(StringSelectorChoiceListItem &choiceParent, const QString *preTextSortKey) :
	ListItem(&choiceParent, preTextSortKey),
	m_choiceParent(choiceParent)
{
	m_type = LITYPE_StringSelectorChoiceListItem_AnimationListItem;

	//-- Set text.
	AnimationListItem::setText(0, "Animation");

	//-- Set pixmap.
	AnimationListItem::setPixmap(0, getPixmap());

	const SkeletalAnimationTemplate *const choiceTemplate = m_choiceParent.getChoiceTemplate();
	if (choiceTemplate)
	{
		//-- Create Ui under this item.
		const QtSkeletalAnimationTemplateUiFactory *uiFactory = dynamic_cast<const QtSkeletalAnimationTemplateUiFactory*>(choiceTemplate->getConstUiFactory());
		if (!uiFactory)
		{
			WARNING_STRICT_FATAL(true, ("new SkeletalAnimationTemplate-derived class did not have a UI factory registered.  Ui out of sync."));
			return;
		}

		ListItem *const listItem = dynamic_cast<ListItem*>(uiFactory->createListItem(this, const_cast<SkeletalAnimationTemplate*>(choiceTemplate), m_choiceParent.getFileModel()));
		UNREF(listItem);
		// if (listItem)
		//	listItem->expandItemAndParents();

		//-- Set FileModel as modified.
		m_choiceParent.getFileModel().setModifiedState(true);
	}
}

// ----------------------------------------------------------------------

bool StringSelectorChoiceListItem::AnimationListItem::supportsPopupMenu() const
{
	return true;
}

// ----------------------------------------------------------------------

QPopupMenu *StringSelectorChoiceListItem::AnimationListItem::createPopupMenu() const
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

bool StringSelectorChoiceListItem::AnimationListItem::handleKeyPress(QKeyEvent *keyEvent)
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

void StringSelectorChoiceListItem::AnimationListItem::createSkeletalAnimationTemplate(int templateIndex)
{
	//-- Delete Ui children.
	deleteListItemChildren();

	//-- Create the new SkeletalAnimationTemplate instance.  This does not fetch a reference, so ref count is zero upon return.
	SkeletalAnimationTemplate *const newAnimationTemplate = SkeletalAnimationTemplateFactory::createSkeletalAnimationTemplate(templateIndex);
	NOT_NULL(newAnimationTemplate);

	//-- Set the new animation.
	const SkeletalAnimationTemplate *const oldAnimationTemplate = m_choiceParent.getChoiceTemplate();
	if (oldAnimationTemplate)
	{
		// An animation was set for this node already, replace it.
		oldAnimationTemplate->fetch();
		m_choiceParent.getSelectorTemplate().replaceSelectionAnimationTemplate(oldAnimationTemplate, newAnimationTemplate);
	}
	else
	{
		// No animation exists for this selection, create one.
		m_choiceParent.getSelectorTemplate().addSelectionAnimationTemplate(newAnimationTemplate);
	}

	//-- Save and release local reference --- this instance does not keep references to animation templates.
	m_choiceParent.setChoiceTemplate(newAnimationTemplate);

	//-- Create Ui under this item.
	const QtSkeletalAnimationTemplateUiFactory *uiFactory = dynamic_cast<const QtSkeletalAnimationTemplateUiFactory*>(newAnimationTemplate->getConstUiFactory());
	if (!uiFactory)
	{
		WARNING_STRICT_FATAL(true, ("new SkeletalAnimationTemplate-derived class did not have a UI factory registered.  Ui out of sync."));
		return;
	}

	ListItem *const listItem = dynamic_cast<ListItem*>(uiFactory->createListItem(this, newAnimationTemplate, m_choiceParent.getFileModel()));
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
	m_choiceParent.getFileModel().setModifiedState(true);
}

// ----------------------------------------------------------------------

void StringSelectorChoiceListItem::AnimationListItem::clearLogicalAnimation()
{
	//-- Delete Ui children.
	deleteListItemChildren();

	//-- If we have a template, remove it.
	const SkeletalAnimationTemplate *const oldAnimationTemplate = m_choiceParent.getChoiceTemplate();
	if (oldAnimationTemplate)
	{
		// An animation was set for this node already, replace it.
		m_choiceParent.getSelectorTemplate().deleteSelectionAnimationTemplate(oldAnimationTemplate);
		m_choiceParent.setChoiceTemplate(0);
	}

	//-- Set FileModel as modified.
	m_choiceParent.getFileModel().setModifiedState(true);
}

// ======================================================================
// class StringSelectorChoiceListItem::AnimationListItem: private static member functions
// ======================================================================

const QPixmap &StringSelectorChoiceListItem::AnimationListItem::getPixmap()
{
	static const QPixmap cs_pixmap = EmbeddedImageLoader::getPixmap("logical_animation_name.png");
	return cs_pixmap;
}

// ======================================================================
// class StringSelectorChoiceListItem::ValueListItem: public TextListItem
// ======================================================================

StringSelectorChoiceListItem::ValueListItem::ValueListItem(QListViewItem *newParent, StringSelectorChoiceListItem &choiceParent, const std::string &value, FileModel &fileModel, bool createMapping) :
	TextListItem(newParent, value, getPixmap(), &fileModel),
	m_choiceParent(choiceParent),
	m_value(value)
{
	m_type = LITYPE_StringSelectorChoiceListItem_ValueListItem;

	// Only create the mapping when requested.  When loading from disk, this is unnecessary
	// since the link already exists in the loaded template..
	if (createMapping)
		m_choiceParent.getSelectorTemplate().addSelectionAnimationTemplateMapping(m_choiceParent.getChoiceTemplate(), CrcLowerString(m_value.c_str()));
}

// ----------------------------------------------------------------------

bool StringSelectorChoiceListItem::ValueListItem::modifyCandidateText(std::string &candidateText) const
{
	//-- Throw away empty text.
	if (candidateText.empty())
		return false;

	//-- Convert to lower case.
	std::transform(candidateText.begin(), candidateText.end(), candidateText.begin(), tolower);

	//-- Accept as long as this value isn't already mapped somewhere by the selector.
	return !m_choiceParent.getSelectorTemplate().hasValue(CrcLowerString(candidateText.c_str()));
}

// ----------------------------------------------------------------------

void StringSelectorChoiceListItem::ValueListItem::setSourceText(const std::string &newText) const
{
	//-- Tell selector template that the old value has been renamed to a new value.
	m_choiceParent.getSelectorTemplate().replaceSelectionAnimationTemplateMapping(m_choiceParent.getChoiceTemplate(), CrcLowerString(m_value.c_str()), CrcLowerString(newText.c_str()));

	//-- Keep track of new value.
	m_value = newText;
}

// ----------------------------------------------------------------------

std::string StringSelectorChoiceListItem::ValueListItem::getSourceText() const
{
	return m_value;
}

// ----------------------------------------------------------------------

bool StringSelectorChoiceListItem::ValueListItem::supportsPopupMenu() const
{
	return true;
}

// ----------------------------------------------------------------------

QPopupMenu *StringSelectorChoiceListItem::ValueListItem::createPopupMenu() const
{
	//-- Create the menu.
	QPopupMenu *menu = new QPopupMenu();

	//-- Add menu items.
	IGNORE_RETURN(menu->insertItem("Delete Value", this, SLOT(deleteValue()), static_cast<int>(QListViewItem::CTRL) + static_cast<int>(QListViewItem::Key_D))); //lint !e56 // bad type // ?

	//-- Return menu to caller.
	return menu;
}

// ----------------------------------------------------------------------

void StringSelectorChoiceListItem::ValueListItem::doDoubleClick()
{
	//-- Get the focus Object.
	Object *const object = AnimationEditorGameWorld::getFocusObject();
	if (!object)
	{
		REPORT_LOG(true, ("No focus object present to do state change.\n"));
		return;
	}

	//-- Get the SkeletalAppearance2 Appearance for the Object.
	SkeletalAppearance2 *const appearance = dynamic_cast<SkeletalAppearance2*>(object->getAppearance());
	if (!appearance)
	{
		REPORT_LOG(true, ("Focus object is not a skeletal appearance, can't set value.\n"));
		return;
	}

	//-- Set animation environment variable to the specified value.
	AnimationEnvironment &env = appearance->getAnimationEnvironment();
	env.getString(m_choiceParent.getSelectorTemplate().getEnvironmentVariableName()) = CrcLowerString(m_value.c_str());
}

// ======================================================================

void StringSelectorChoiceListItem::ValueListItem::deleteValue()
{
	m_choiceParent.getSelectorTemplate().deleteSelectionAnimationTemplateMapping(m_choiceParent.getChoiceTemplate(), CrcLowerString(m_value.c_str()));
	delete this;
}

// ======================================================================

const QPixmap &StringSelectorChoiceListItem::ValueListItem::getPixmap()
{
	static const QPixmap cs_pixmap = EmbeddedImageLoader::getPixmap("selection_value.png");
	return cs_pixmap;
}

// ======================================================================
