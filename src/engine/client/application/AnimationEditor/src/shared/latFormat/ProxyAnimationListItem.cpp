// ======================================================================
//
// ProxyAnimationListItem.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"
#include "AnimationEditor/ProxyAnimationListItem.h"
#include "ProxyAnimationListItem.moc"

#include "AnimationEditor/AnimationPriorityListItem.h"
#include "AnimationEditor/EmbeddedImageLoader.h"
#include "AnimationEditor/FileModel.h"
#include "clientSkeletalAnimation/ProxySkeletalAnimationTemplate.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"

#include <qfiledialog.h>
#include <qpopupmenu.h>

// ======================================================================

const ConstCharCrcLowerString ProxyAnimationListItem::cms_defaultMaskName("mask_unset.iff");
const ConstCharCrcLowerString ProxyAnimationListItem::cms_defaultPriorityGroupName("zero_speed");

// ======================================================================

ProxyAnimationListItem::ProxyAnimationListItem(QListViewItem *newParent, FileModel &fileModel, ProxySkeletalAnimationTemplate &animationTemplate) :
	ListItem(newParent),
	m_fileModel(fileModel),
	m_animationTemplate(animationTemplate),
	m_priorityListItem(0)
{
	m_type = LITYPE_ProxyAnimationListItem;

	//-- Fetch a local reference to the animation template.
	m_animationTemplate.fetch();

	//-- Set item text.
	ProxyAnimationListItem::setText(0, m_animationTemplate.getTargetAnimationTemplateName().getString());

	//-- Set the pixmap.
	ProxyAnimationListItem::setPixmap(0, getPixmap());

	//-- Create the animation priority list node.
	m_priorityListItem = AnimationPriorityListItem::createFromData(this, m_fileModel, m_animationTemplate);
}

// ----------------------------------------------------------------------

ProxyAnimationListItem::~ProxyAnimationListItem()
{
	//-- Release local reference.
	m_animationTemplate.release();

	//-- Clear nodes owned by Qt.
	m_priorityListItem = 0;
}

// ----------------------------------------------------------------------

bool ProxyAnimationListItem::supportsPopupMenu() const
{
	return true;
}

// ----------------------------------------------------------------------

QPopupMenu *ProxyAnimationListItem::createPopupMenu() const
{
	//-- Create the menu.
	QPopupMenu *menu = new QPopupMenu();

	//-- Add menu items.
	const int setPathMenuId = menu->insertItem("Set Path...", this, SLOT(setTargetAnimationTemplatePath()), QListViewItem::CTRL + QListViewItem::Key_S); //lint !e56 // bad type // ?

	// @todo don't allow changes if the animation is for a logical animation name no longer exported by the ash.
	menu->setItemEnabled(setPathMenuId, true);

	const int ampMenuId = menu->insertItem("Add masked priority data", this, SLOT(addMaskedPriorityData()));
	menu->setItemEnabled(ampMenuId, m_priorityListItem == 0);

	const int aupMenuId = menu->insertItem("Add uniform priority data", this, SLOT(addUniformPriorityData()));
	menu->setItemEnabled(aupMenuId, m_priorityListItem == 0);

	const int dpMenuId = menu->insertItem("Delete priority data", this, SLOT(deletePriorityData()));
	menu->setItemEnabled(dpMenuId, m_priorityListItem != 0);

	//-- Return menu to caller.
	return menu;
}

// ======================================================================
// class ProxyAnimationListItem: public slot member functions
// ======================================================================

void ProxyAnimationListItem::setTargetAnimationTemplatePath()
{
	QString startupPathName(QString::null);

	//-- Convert template TreeFile reference name to a full path name, used for dialog's startup filename.
	char buffer[2048];
	buffer[0] = 0;

	const CrcLowerString &crcReferenceName = m_animationTemplate.getTargetAnimationTemplateName();
	const char *const     cReferenceName   = crcReferenceName.getString();

	if (cReferenceName && *cReferenceName)
	{
		if (TreeFile::getPathName(cReferenceName, buffer, sizeof(buffer) - 1))
		{
			//-- Mapped reference name to full path name.
			startupPathName = buffer;
		}
	}

	const QString qPathName = QFileDialog::getOpenFileName(startupPathName, "ANS Files (*.ans)", 0, "open file dialog", "Specify an animation file");
	if (qPathName == QString::null)
	{
		// user aborted
		return;
	}

	//-- Convert qt forward slash to back slash
	std::string  fullPathName(qPathName);
	

	//-- Find the TreeFile-relative path name given the on-disk path name.
	std::string  newRelativePathName;

	if (!TreeFile::stripTreeFileSearchPathFromFile(fullPathName, newRelativePathName))
	{
		WARNING(true, ("couldn't determine TreeFile-relative pathname for [%s].", static_cast<const char*>(qPathName)));
		return;
	}

	//-- Set the new path to .ans file.
	m_animationTemplate.setTargetAnimationTemplateName(CrcLowerString(newRelativePathName.c_str()));

	//-- Reset list item name.
	setText(0, newRelativePathName.c_str());

	//-- Mark the file as modified.
	m_fileModel.setModifiedState(true);
}

// ----------------------------------------------------------------------

void ProxyAnimationListItem::replaceText(const char *subStringToReplace, const char* replacementText)
{
	QString newString = text(0);
	if(subStringToReplace && *subStringToReplace != '\0')
	{
		newString.replace(subStringToReplace,replacementText,false);
	}
	else
	{
		newString = replacementText;
	}

	//-- Set the new path to .ans file.
	m_animationTemplate.setTargetAnimationTemplateName(CrcLowerString(newString.ascii()));
	
	//-- Reset list item name.
	setText(0, newString);

	//-- Mark the file as modified.
	m_fileModel.setModifiedState(true);
}

// ----------------------------------------------------------------------

void ProxyAnimationListItem::addMaskedPriorityData()
{
	//-- Delete existing data.
	if (m_priorityListItem)
		delete m_priorityListItem;

	//-- Create new data.
	m_animationTemplate.setMaskedPriority(cms_defaultMaskName, cms_defaultPriorityGroupName, cms_defaultPriorityGroupName, cms_defaultPriorityGroupName);

	//-- Create UI for data.
	m_priorityListItem = AnimationPriorityListItem::createFromData(this, m_fileModel, m_animationTemplate);

	//-- Ensure change gets reflected to the data model.
	m_priorityListItem->handleUpdate();

	//-- Mark the file as modified.
	m_fileModel.setModifiedState(true);
}

// ----------------------------------------------------------------------

void ProxyAnimationListItem::addUniformPriorityData()
{
	//-- Delete existing data.
	if (m_priorityListItem)
		delete m_priorityListItem;

	//-- Create new data.
	m_animationTemplate.setUniformPriority(cms_defaultPriorityGroupName, cms_defaultPriorityGroupName);

	//-- Create UI for data.
	m_priorityListItem = AnimationPriorityListItem::createFromData(this, m_fileModel, m_animationTemplate);

	//-- Ensure change gets reflected to the data model.
	m_priorityListItem->handleUpdate();

	//-- Mark the file as modified.
	m_fileModel.setModifiedState(true);
}

// ----------------------------------------------------------------------

void ProxyAnimationListItem::deletePriorityData()
{
	//-- Delete existing data.
	if (m_priorityListItem)
	{
		delete m_priorityListItem;
		m_priorityListItem = 0;
	}

	//-- Ensure change gets reflected to data model.
	m_animationTemplate.setHasNoAssignedPriority();

	//-- Mark the file as modified.
	m_fileModel.setModifiedState(true);
}

// ======================================================================
// class ProxyAnimationListItem: private static member functions
// ======================================================================

const QPixmap &ProxyAnimationListItem::getPixmap()
{
	static const QPixmap cs_pixmap = EmbeddedImageLoader::getPixmap("proxy_animation.png");
	return cs_pixmap;
}

// ======================================================================
