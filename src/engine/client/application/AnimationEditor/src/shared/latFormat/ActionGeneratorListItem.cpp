// ======================================================================
//
// ActionGeneratorListItem.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"
#include "AnimationEditor/ActionGeneratorListItem.h"
#include "ActionGeneratorListItem.moc"

#include "AnimationEditor/EmbeddedImageLoader.h"
#include "AnimationEditor/FileModel.h"
#include "AnimationEditor/FolderListItem.h"
#include "AnimationEditor/QtSkeletalAnimationTemplateUiFactory.h"
#include "AnimationEditor/SkeletalAnimationTemplateFactory.h"
#include "clientSkeletalAnimation/ActionGeneratorSkeletalAnimationTemplate.h"
#include "sharedFoundation/CrcLowerString.h"

#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qstring.h>
#include <stdio.h>

#include <algorithm>
#include <string>

// ======================================================================

const QString ActionGeneratorListItem::cms_minTimeSortKey("1:");
const QString ActionGeneratorListItem::cms_maxTimeSortKey("2:");
const QString ActionGeneratorListItem::cms_actionFolderSortKey("3:");
const QString ActionGeneratorListItem::cms_loopingAnimationSortKey("4:");

int           ActionGeneratorListItem::ms_newActionEntryCount;

// ======================================================================

class ActionGeneratorListItem::MinTimeListItem: public TextListItem
{
public:

	MinTimeListItem(ActionGeneratorListItem &generatorParent, FileModel &fileModel, const QString *preTextSortKey = 0);

protected:

	virtual bool         modifyCandidateText(std::string &candidateText) const;
	virtual void         setSourceText(const std::string &newText) const;
	virtual std::string  getSourceText() const;

private:

	static const QPixmap &getPixmap();

private:

	// Disabled.
	MinTimeListItem();
	MinTimeListItem(const MinTimeListItem &);            //lint -esym(754, MinTimeListItem::MinTimeListItem) // must disable.
	MinTimeListItem &operator =(const MinTimeListItem&); //lint -esym(754, MinTimeListItem::operator=)       // must disable.

private:

	ActionGeneratorListItem &m_generatorParent;
};

// ======================================================================

class ActionGeneratorListItem::MaxTimeListItem: public TextListItem
{
public:

	MaxTimeListItem(ActionGeneratorListItem &generatorParent, FileModel &fileModel, const QString *preTextSortKey = 0);

protected:

	virtual bool         modifyCandidateText(std::string &candidateText) const;
	virtual void         setSourceText(const std::string &newText) const;
	virtual std::string  getSourceText() const;

private:

	static const QPixmap &getPixmap();

private:

	// Disabled.
	MaxTimeListItem();
	MaxTimeListItem(const MaxTimeListItem &);            //lint -esym(754, MaxTimeListItem::MaxTimeListItem) // must disable.
	MaxTimeListItem &operator =(const MaxTimeListItem&); //lint -esym(754, MaxTimeListItem::operator=) // must disable.

private:

	ActionGeneratorListItem &m_generatorParent;
};

// ======================================================================

class ActionGeneratorListItem::ActionWeightListItem: public TextListItem
{
public:

	ActionWeightListItem(ActionListItem &actionParent, FileModel &fileModel, const QString *preTextSortKey = 0);

protected:

	virtual bool         modifyCandidateText(std::string &candidateText) const;
	virtual void         setSourceText(const std::string &newText) const;
	virtual std::string  getSourceText() const;

private:

	static const QPixmap &getPixmap();

private:

	// Disabled.
	ActionWeightListItem();
	ActionWeightListItem(const ActionWeightListItem &);            //lint -esym(754, ActionWeightListItem::ActionWeightListItem) // must disable.
	ActionWeightListItem &operator =(const ActionWeightListItem&); //lint -esym(754, ActionWeightListItem::operator=)            // must disable.

private:

	ActionListItem &m_actionParent;

};

// ======================================================================
// class ActionGeneratorListItem::MinTimeListItem
// ======================================================================

ActionGeneratorListItem::MinTimeListItem::MinTimeListItem(ActionGeneratorListItem &generatorParent, FileModel &fileModel, const QString *preTextSortKey) :
	TextListItem(&generatorParent, "", getPixmap(), &fileModel, preTextSortKey),
	m_generatorParent(generatorParent)
{
	m_type = LITYPE_ActionGeneratorListItem_MinTimeListItem;
	//-- Set text item.
	MinTimeListItem::setText(0, MinTimeListItem::getSourceText().c_str());
}

// ======================================================================

bool ActionGeneratorListItem::MinTimeListItem::modifyCandidateText(std::string &candidateText) const
{
	//-- Convert value to float.
	float newValue = 0.0f;
	const int convertedFieldCount = sscanf(candidateText.c_str(), "%f", &newValue);

	//-- Use string if value could be converted to float.
	return convertedFieldCount == 1;
} //lint !e1764 // candidateText could be declared const ref // this is a virtual interface.  other classes need this.

// ----------------------------------------------------------------------

void ActionGeneratorListItem::MinTimeListItem::setSourceText(const std::string &newText) const
{
	float newValue = 0.0f;

	sscanf(newText.c_str(), "%g", &newValue);
	m_generatorParent.getGeneratorTemplate().setMinActionElapsedTime(newValue);
}

// ----------------------------------------------------------------------

std::string ActionGeneratorListItem::MinTimeListItem::getSourceText() const
{
	char buffer[32];
	sprintf(buffer, "%g", m_generatorParent.getGeneratorTemplate().getMinActionElapsedTime());

	return buffer;
}

// ======================================================================

const QPixmap &ActionGeneratorListItem::MinTimeListItem::getPixmap()
{
	static const QPixmap cs_pixmap = EmbeddedImageLoader::getPixmap("min_time.png");
	return cs_pixmap;
}

// ======================================================================
// class ActionGeneratorListItem::MaxTimeListItem
// ======================================================================

ActionGeneratorListItem::MaxTimeListItem::MaxTimeListItem(ActionGeneratorListItem &generatorParent, FileModel &fileModel, const QString *preTextSortKey) :
	TextListItem(&generatorParent, "", getPixmap(), &fileModel, preTextSortKey),
	m_generatorParent(generatorParent)
{
	m_type = LITYPE_ActionGeneratorListItem_MaxTimeListItem;
	//-- Set text item.
	MaxTimeListItem::setText(0, MaxTimeListItem::getSourceText().c_str());
}

// ======================================================================

bool ActionGeneratorListItem::MaxTimeListItem::modifyCandidateText(std::string &candidateText) const
{
	//-- Convert value to float.
	float newValue = 0.0f;
	const int convertedFieldCount = sscanf(candidateText.c_str(), "%f", &newValue);

	//-- Use string if value could be converted to float.
	return convertedFieldCount == 1;
} //lint !e1764 // candidateText could be declared const ref // this is a virtual interface.  other classes need this.

// ----------------------------------------------------------------------

void ActionGeneratorListItem::MaxTimeListItem::setSourceText(const std::string &newText) const
{
	float newValue = 0.0f;

	sscanf(newText.c_str(), "%g", &newValue);
	m_generatorParent.getGeneratorTemplate().setMaxActionElapsedTime(newValue);
}

// ----------------------------------------------------------------------

std::string ActionGeneratorListItem::MaxTimeListItem::getSourceText() const
{
	char buffer[32];
	sprintf(buffer, "%g", m_generatorParent.getGeneratorTemplate().getMaxActionElapsedTime());

	return buffer;
}

// ======================================================================

const QPixmap &ActionGeneratorListItem::MaxTimeListItem::getPixmap()
{
	static const QPixmap cs_pixmap = EmbeddedImageLoader::getPixmap("max_time.png");
	return cs_pixmap;
}

// ======================================================================
// class ActionGeneratorListItem::ActionWeightListItem
// ======================================================================

ActionGeneratorListItem::ActionWeightListItem::ActionWeightListItem(ActionListItem &actionParent, FileModel &fileModel, const QString *preTextSortKey) :
	TextListItem(&actionParent, "", getPixmap(), &fileModel, preTextSortKey),
	m_actionParent(actionParent)
{
	m_type = LITYPE_ActionGeneratorListItem_ActionWeightListItem;
	//-- Set text item.
	ActionWeightListItem::setText(0, ActionWeightListItem::getSourceText().c_str());
}

// ======================================================================

bool ActionGeneratorListItem::ActionWeightListItem::modifyCandidateText(std::string &candidateText) const
{
	//-- Convert value to float.
	float newValue = 0.0f;
	const int convertedFieldCount = sscanf(candidateText.c_str(), "%f", &newValue);

	//-- Use string if value could be converted to float.
	return convertedFieldCount == 1;
} //lint !e1764 // candidateText could be declared const ref // this is a virtual interface.  other classes need this.

// ----------------------------------------------------------------------

void ActionGeneratorListItem::ActionWeightListItem::setSourceText(const std::string &newText) const
{
	float newValue = 0.0f;

	sscanf(newText.c_str(), "%g", &newValue);
	m_actionParent.getGeneratorTemplate().setRelativeWeight(CrcLowerString(m_actionParent.getActionName().c_str()), newValue);
}

// ----------------------------------------------------------------------

std::string ActionGeneratorListItem::ActionWeightListItem::getSourceText() const
{
	char buffer[32];
	sprintf(buffer, "%g", m_actionParent.getGeneratorTemplate().getRelativeWeight(CrcLowerString(m_actionParent.getActionName().c_str())));

	return buffer;
}

// ======================================================================

const QPixmap &ActionGeneratorListItem::ActionWeightListItem::getPixmap()
{
	static const QPixmap cs_pixmap = EmbeddedImageLoader::getPixmap("action_weight.png");
	return cs_pixmap;
}

// ======================================================================
// class ActionGeneratorListItem::AnimationListItem
// ======================================================================

ActionGeneratorListItem::AnimationListItem::AnimationListItem(ActionGeneratorListItem &generatorParent, const QString *preTextSortKey) :
	ListItem(&generatorParent, preTextSortKey),
	m_generatorParent(generatorParent)
{
	m_type = LITYPE_ActionGeneratorListItem_AnimationListItem;
	//-- Set text.
	AnimationListItem::setText(0, "Looping Animation");

	//-- Set pixmap.
	AnimationListItem::setPixmap(0, getPixmap());

	buildUi();
}

// ----------------------------------------------------------------------

bool ActionGeneratorListItem::AnimationListItem::supportsPopupMenu() const
{
	return true;
}

// ----------------------------------------------------------------------

QPopupMenu *ActionGeneratorListItem::AnimationListItem::createPopupMenu() const
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

bool ActionGeneratorListItem::AnimationListItem::handleKeyPress(QKeyEvent *keyEvent)
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

void ActionGeneratorListItem::AnimationListItem::buildUi()
{
	//-- Clear the UI under this child.
	deleteListItemChildren();

	SkeletalAnimationTemplate *animationTemplate = const_cast<SkeletalAnimationTemplate*>(m_generatorParent.getGeneratorTemplate().fetchLoopingAnimationTemplate());
	if (animationTemplate)
	{
		//-- Retrieve the UiFactory for the SkeletalAnimationTemplate.
		const QtSkeletalAnimationTemplateUiFactory *const uiFactory = dynamic_cast<const QtSkeletalAnimationTemplateUiFactory*>(animationTemplate->getConstUiFactory());

		if (!uiFactory)
			WARNING_STRICT_FATAL(true, ("An animation template did not have a ui factory associated with it."));
		else
		{
			//-- Create the list item for the SkeletalAnimationTemplate.
			ListItem *const listItem = dynamic_cast<ListItem*>(uiFactory->createListItem(this, animationTemplate, m_generatorParent.getFileModel()));
			UNREF(listItem);

			//-- Expand the new list item.
			// if (listItem)
			//	listItem->expandItemAndParents();

			//-- Release local reference.
			animationTemplate->release();
		}
	}
}

// ======================================================================

void ActionGeneratorListItem::AnimationListItem::createSkeletalAnimationTemplate(int templateIndex)
{
	//-- Delete Ui children.
	deleteListItemChildren();

	//-- Create the new SkeletalAnimationTemplate instance.
	SkeletalAnimationTemplate *const newAnimationTemplate = SkeletalAnimationTemplateFactory::createSkeletalAnimationTemplate(templateIndex);
	NOT_NULL(newAnimationTemplate);

	//-- Replace the animation.
	SkeletalAnimationTemplate const *const oldAnimationTemplate = m_generatorParent.getGeneratorTemplate().fetchLoopingAnimationTemplate();
	m_generatorParent.getGeneratorTemplate().setLoopingSkeletalAnimationTemplate(newAnimationTemplate);

	//-- Create Ui under this item.
	const QtSkeletalAnimationTemplateUiFactory *uiFactory = dynamic_cast<const QtSkeletalAnimationTemplateUiFactory*>(newAnimationTemplate->getConstUiFactory());
	if (!uiFactory)
	{
		WARNING_STRICT_FATAL(true, ("new SkeletalAnimationTemplate-derived class did not have a UI factory registered.  Ui out of sync."));
		return;
	}

	ListItem *const listItem = dynamic_cast<ListItem*>(uiFactory->createListItem(this, newAnimationTemplate, m_generatorParent.getFileModel()));
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
	m_generatorParent.getFileModel().setModifiedState(true);
}

// ----------------------------------------------------------------------

void ActionGeneratorListItem::AnimationListItem::clearLogicalAnimation()
{
	//-- Delete Ui children.
	deleteListItemChildren();

	//-- Reset the looping animation.
	m_generatorParent.getGeneratorTemplate().setLoopingSkeletalAnimationTemplate(0);

	//-- Set FileModel as modified.
	m_generatorParent.getFileModel().setModifiedState(true);
}

// ======================================================================

const QPixmap &ActionGeneratorListItem::AnimationListItem::getPixmap()
{
	static const QPixmap cs_pixmap = EmbeddedImageLoader::getPixmap("logical_animation_name.png");
	return cs_pixmap;
}

// ======================================================================
// class ActionGeneratorListItem::ActionListItem
// ======================================================================

ActionGeneratorListItem::ActionListItem::ActionListItem(QListViewItem *newParent, ActionGeneratorListItem &generatorParent, const std::string &actionName) :
	TextListItem(newParent, actionName, getPixmap(), &generatorParent.getFileModel()),
	m_generatorParent(generatorParent),
	m_actionName(actionName)
{

	m_type = LITYPE_ActionGeneratorListItem_ActionListItem;

	ListItem *const newListItem = new ActionWeightListItem(*this, m_generatorParent.getFileModel()); //lint !e1524 // new in constructor with no explicit destructor // Qt cleans this up for us.
	UNREF(newListItem);
	// newListItem->expandItemAndParents();
} //lint !e429 // newListItem not freed or returned // It's okay, Qt owns.

// ----------------------------------------------------------------------

bool ActionGeneratorListItem::ActionListItem::supportsPopupMenu() const
{
	return true;
}

// ----------------------------------------------------------------------

QPopupMenu *ActionGeneratorListItem::ActionListItem::createPopupMenu() const
{
	//-- Create the menu.
	QPopupMenu *menu = new QPopupMenu();

	//-- Add menu items.
	IGNORE_RETURN(menu->insertItem("Remove Action", this, SLOT(removeAction()), static_cast<int>(QListViewItem::CTRL) + static_cast<int>(QListViewItem::Key_R))); //lint !e56 // bad type // ?

	//-- Return menu to caller.
	return menu;
}

// ----------------------------------------------------------------------

const std::string &ActionGeneratorListItem::ActionListItem::getActionName() const
{
	return m_actionName;
}

// ----------------------------------------------------------------------

ActionGeneratorSkeletalAnimationTemplate &ActionGeneratorListItem::ActionListItem::getGeneratorTemplate()
{
	return m_generatorParent.getGeneratorTemplate();
}

// ======================================================================

void ActionGeneratorListItem::ActionListItem::removeAction()
{
	m_generatorParent.getGeneratorTemplate().removeAction(CrcLowerString(m_actionName.c_str()));
	m_generatorParent.getFileModel().setModifiedState(true);
	delete this;
}

// ======================================================================

bool ActionGeneratorListItem::ActionListItem::modifyCandidateText(std::string &candidateText) const
{
	//-- Don't accept empty names.
	if (candidateText.empty())
	{
		return false;
	}

	//-- Make lower case.
	std::transform(candidateText.begin(), candidateText.end(), candidateText.begin(), tolower);

	//-- Accept.
	return true;
}

// ----------------------------------------------------------------------

void ActionGeneratorListItem::ActionListItem::setSourceText(const std::string &newText) const
{
	//-- Rename generated action entry.
	m_generatorParent.getGeneratorTemplate().changeActionName(CrcLowerString(m_actionName.c_str()), CrcLowerString(newText.c_str()));

	//-- Keep track of new action name.
	m_actionName = newText;
}

// ----------------------------------------------------------------------

std::string ActionGeneratorListItem::ActionListItem::getSourceText() const
{
	return m_actionName;
}

// ======================================================================

const QPixmap &ActionGeneratorListItem::ActionListItem::getPixmap()
{
	static const QPixmap cs_pixmap = EmbeddedImageLoader::getPixmap("animation_action.png");
	return cs_pixmap;
}

// ======================================================================
// class ActionGeneratorListItem: public member functions
// ======================================================================

ActionGeneratorListItem::ActionGeneratorListItem(QListViewItem *newParent, ActionGeneratorSkeletalAnimationTemplate &generatorTemplate, FileModel &fileModel) :
	ListItem(newParent),
	m_generatorTemplate(generatorTemplate),
	m_fileModel(fileModel),
	m_actionFolder(0)
{

	m_type = LITYPE_ActionGeneratorListItem;

	//-- Set text.
	ActionGeneratorListItem::setText(0, "Action Generator");

	//-- Set pixmap.
	ActionGeneratorListItem::setPixmap(0, getPixmap());

	//-- Create min/max time child ui.
	IGNORE_RETURN(new MinTimeListItem(*this, m_fileModel, &cms_minTimeSortKey)); //lint !e1524 // new in constructor for class with no explicit destructor // It's okay, Qt owns.
	IGNORE_RETURN(new MaxTimeListItem(*this, m_fileModel, &cms_maxTimeSortKey));

	//-- Create action entries.
	m_actionFolder = new FolderListItem(this, "Actions", FolderListItem::FC_red, &cms_actionFolderSortKey);

	const int actionCount = m_generatorTemplate.getActionCount();
	for (int i = 0; i < actionCount; ++i)
	{
		ListItem *const listItem = new ActionListItem(m_actionFolder, *this, std::string(m_generatorTemplate.getActionName(i).getString()));
		UNREF(listItem);
		// listItem->expandItemAndParents();
	} //lint !e429 // custodial pointer not freed or returned // Qt owns.

	//-- Create animation entry.
	IGNORE_RETURN(new AnimationListItem(*this, &cms_loopingAnimationSortKey));
}

// ----------------------------------------------------------------------

bool ActionGeneratorListItem::supportsPopupMenu() const
{
	return true;
}

// ----------------------------------------------------------------------

QPopupMenu *ActionGeneratorListItem::createPopupMenu() const
{
	//-- Create the menu.
	QPopupMenu *menu = new QPopupMenu();

	//-- Add menu items.
	IGNORE_RETURN(menu->insertItem("Add New Action", this, SLOT(addNewAction()), static_cast<int>(QListViewItem::CTRL) + static_cast<int>(QListViewItem::Key_N))); //lint !e56 // bad type // ?

	//-- Return menu to caller.
	return menu;
}

// ----------------------------------------------------------------------

bool ActionGeneratorListItem::supportsAttachingDownStreamAnimationTemplate() const
{
	return true;
}

// ----------------------------------------------------------------------

void ActionGeneratorListItem::attachDownStreamAnimationTemplate(SkeletalAnimationTemplate const *animationTemplate)
{
	//-- Set new looping skeletal animation template, mark as modified.
	m_generatorTemplate.setLoopingSkeletalAnimationTemplate(animationTemplate);
	m_fileModel.setModifiedState(true);

	//-- Find AnimationListItem child and rebuild UI.
	for (QListViewItem *item = firstChild(); item != NULL; item = item->nextSibling())
	{
		AnimationListItem *const animationListItem = dynamic_cast<AnimationListItem*>(item);
		if (animationListItem)
		{
			// Found the looping animation list item, fix the UI for it.
			animationListItem->buildUi();
			break;
		}
	}
}

// ======================================================================
// class ActionGeneratorListItem: public slot functions
// ======================================================================

void ActionGeneratorListItem::addNewAction()
{
	//-- Create the action.
	const std::string  newActionName = getNewActionName();
	m_generatorTemplate.addNewAction(CrcLowerString(newActionName.c_str()), 100.0f);

	//-- Create the UI.
	ListItem *const listItem = new ActionListItem(m_actionFolder, *this, newActionName);
	listItem->expandItemAndParents();

	m_fileModel.setModifiedState(true);
} //lint !e429 // custodial pointer not freed or returned // Qt owns.

// ======================================================================
// class ActionGeneratorListItem: private static member functions
// ======================================================================

const QPixmap &ActionGeneratorListItem::getPixmap()
{
	static const QPixmap cs_pixmap = EmbeddedImageLoader::getPixmap("action_generator_animation.png");
	return cs_pixmap;
}

// ----------------------------------------------------------------------

std::string ActionGeneratorListItem::getNewActionName()
{
	char  buffer[32];

	++ms_newActionEntryCount;
	sprintf(buffer, "action%d", ms_newActionEntryCount);

	return buffer;
}

// ======================================================================

