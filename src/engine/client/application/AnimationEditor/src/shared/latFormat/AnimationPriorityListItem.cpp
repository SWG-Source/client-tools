// ======================================================================
//
// AnimationPriorityListItem.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"
#include "AnimationEditor/AnimationPriorityListItem.h"

#include "AnimationEditor/FileModel.h"
#include "AnimationEditor/PriorityGroupListItem.h"
#include "clientSkeletalAnimation/ProxySkeletalAnimationTemplate.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/CrcLowerString.h"

#include <qfiledialog.h>

// ======================================================================

class AnimationPriorityListItem::PathListItem: public ListItem
{
public:

	explicit PathListItem(QListViewItem *newParent, AnimationPriorityListItem &priorityListItem, const CrcLowerString &path, const QString *preTextSortKey = 0);

	const CrcLowerString &getPath() const;

	virtual void          doDoubleClick();

private:

	void                  updateText();

	// Disabled.
	PathListItem();
	PathListItem(const PathListItem&);             //lint -esym(754, PathListItem::PathListItem) // unreferenced // defensive hiding.
	PathListItem &operator =(const PathListItem&); //lint -esym(754, PathListItem::operator=)    // unreferenced // defensive hiding.

private:

	AnimationPriorityListItem &m_priorityListItem;
	CrcLowerString             m_path;

}; //lint !e1748 // (Info -- non-virtual base class 'Qt' included twice in class 'MaskedListItem')  // Yes, necessary for ListItem slot handling.

// ======================================================================

class AnimationPriorityListItem::MaskedListItem: public AnimationPriorityListItem
{
public:

	MaskedListItem(QListViewItem *newParent, FileModel &fileModel, ProxySkeletalAnimationTemplate &animationTemplate, const QString *preTextSortKey);
	virtual ~MaskedListItem();

	virtual void handleUpdate();

private:

	// Disabled.
	MaskedListItem();
	MaskedListItem(const MaskedListItem&);             //lint -esym(754, MaskedListItem::MaskedListItem) // not referenced // defensive hiding.
	MaskedListItem &operator =(const MaskedListItem&); //lint -esym(754, MaskedListItem::operator=)      // not referenced // defensive hiding.

private:

	static const QString cms_transformMaskPathSortKey;
	static const QString cms_inGroupTransformPrioritySortKey;
	static const QString cms_outGroupTransformPrioritySortKey;
	static const QString cms_locomotionPrioritySortKey;

private:

	FileModel             &m_fileModel;
	PathListItem          *m_transformMaskPath;
	PriorityGroupListItem *m_inGroupTransformPriorityGroup;
	PriorityGroupListItem *m_outGroupTransformPriorityGroup;
	PriorityGroupListItem *m_locomotionPriorityGroup;
	
}; //lint !e1748 // (Info -- non-virtual base class 'Qt' included twice in class 'MaskedListItem')  // Yes, necessary for ListItem slot handling.

// ======================================================================

class AnimationPriorityListItem::UniformListItem: public AnimationPriorityListItem
{
public:

	UniformListItem(QListViewItem *newParent, FileModel &fileModel, ProxySkeletalAnimationTemplate &animationTemplate, const QString *preTextSortKey);
	virtual ~UniformListItem();

	virtual void handleUpdate();

private:

	// Disabled.
	UniformListItem();
	UniformListItem(const UniformListItem&);             //lint -esym(754, UniformListItem::UniformListItem) // not referenced // defensive hiding.
	UniformListItem &operator =(const UniformListItem&); //lint -esym(754, UniformListItem::operator=)       // not referenced // defensive hiding.

private:

	FileModel             &m_fileModel;

	/// Animation priority group used both for transform priority and locomotion priority.
	PriorityGroupListItem *m_priorityGroup;

}; //lint !e1748 // (Info -- non-virtual base class 'Qt' included twice in class 'MaskedListItem')  // Yes, necessary for ListItem slot handling.

// ======================================================================
// class AnimationPriorityListItem::PathListItem
// ======================================================================

AnimationPriorityListItem::PathListItem::PathListItem(QListViewItem *newParent, AnimationPriorityListItem &priorityListItem, const CrcLowerString &path, const QString *preTextSortKey) :
	ListItem(newParent, preTextSortKey),
	m_priorityListItem(priorityListItem),
	m_path(path)
{
	m_type = LITYPE_AnimationPriorityListItem_PathListItem;
	updateText();
}

// ----------------------------------------------------------------------

inline const CrcLowerString &AnimationPriorityListItem::PathListItem::getPath() const
{
	return m_path;
}

// ----------------------------------------------------------------------

void AnimationPriorityListItem::PathListItem::doDoubleClick()
{
	//-- Convert path's TreeFile reference name to a full path name, used for dialog's startup filename.
	char buffer[2048];
	buffer[0] = 0;

	const char *const cReferenceName = m_path.getString();
	QString startupPathName(QString::null);

	if (TreeFile::getPathName(cReferenceName, buffer, sizeof(buffer) - 1))
	{
		//-- Mapped reference name to full path name.
		startupPathName = buffer;
	}

	//-- Retrieve full pathname from user via dialog.
	const QString qPathName = QFileDialog::getOpenFileName(startupPathName, "Transform Mask Files (*.iff)", 0, "open file dialog", "Specify a Transform Mask File");
	if (qPathName == QString::null)
	{
		// User aborted.
		return;
	}

	//-- Convert to TreeFile-relative.
	std::string  fullPathName(qPathName);

	// Find the TreeFile-relative path name given the on-disk path name.
	std::string  newRelativePathName;

	if (!TreeFile::stripTreeFileSearchPathFromFile(fullPathName, newRelativePathName))
	{
		WARNING(true, ("couldn't determine TreeFile-relative pathname for [%s].", static_cast<const char*>(qPathName)));
		return;
	}

	//-- Save path.
	m_path.setString(newRelativePathName.c_str());

	//-- Update visuals.
	updateText();

	//-- Notify parent of change.
	m_priorityListItem.handleUpdate();
}

// ======================================================================

void AnimationPriorityListItem::PathListItem::updateText()
{
	//-- Build item text.
	std::string  updatedText("Mask file: ");

	updatedText += m_path.getString();

	//-- Set item text.
	setText(0, updatedText.c_str());
}

// ======================================================================
// class AnimationPriorityListItem::MaskedListItem
// ======================================================================

const QString AnimationPriorityListItem::MaskedListItem::cms_transformMaskPathSortKey("1:");
const QString AnimationPriorityListItem::MaskedListItem::cms_inGroupTransformPrioritySortKey("2:");
const QString AnimationPriorityListItem::MaskedListItem::cms_outGroupTransformPrioritySortKey("3:");
const QString AnimationPriorityListItem::MaskedListItem::cms_locomotionPrioritySortKey("4:");

// ======================================================================

AnimationPriorityListItem::MaskedListItem::MaskedListItem(QListViewItem *newParent, FileModel &fileModel, ProxySkeletalAnimationTemplate &animationTemplate, const QString *preTextSortKey) :
	AnimationPriorityListItem(newParent, animationTemplate, preTextSortKey),
	m_fileModel(fileModel),
	m_transformMaskPath(0),
	m_inGroupTransformPriorityGroup(0),
	m_outGroupTransformPriorityGroup(0),
	m_locomotionPriorityGroup(0)
{
	m_type = LITYPE_AnimationPriorityListItem_MaskedListItem;
	//-- Set Text.
	MaskedListItem::setText(0, "Masked Priority");

	//-- Retrieve initial state from data model.
	CrcLowerString  transformMaskName;
	CrcLowerString  inGroupTransformPriorityGroupName;
	CrcLowerString  outGroupTransformPriorityGroupName;
	CrcLowerString  locomotionPriorityGroupName;

	DEBUG_FATAL(!animationTemplate.hasMaskedPriority(), ("expecting masked priority."));
	animationTemplate.getMaskedPriority(transformMaskName, inGroupTransformPriorityGroupName, outGroupTransformPriorityGroupName, locomotionPriorityGroupName);

	//-- Create PriorityGroup items with correct data.
	m_transformMaskPath              = new PathListItem(this, *this, transformMaskName, &cms_transformMaskPathSortKey);
	m_inGroupTransformPriorityGroup  = new PriorityGroupListItem(this, *this, "In-group Transform Priority: ", inGroupTransformPriorityGroupName, &cms_inGroupTransformPrioritySortKey);
	m_outGroupTransformPriorityGroup = new PriorityGroupListItem(this, *this, "Out-group Transform Priority: ", outGroupTransformPriorityGroupName, &cms_outGroupTransformPrioritySortKey);
	m_locomotionPriorityGroup        = new PriorityGroupListItem(this, *this, "Locomotion Priority: ", locomotionPriorityGroupName, &cms_locomotionPrioritySortKey);
}

// ----------------------------------------------------------------------
	
AnimationPriorityListItem::MaskedListItem::~MaskedListItem()
{
	m_transformMaskPath              = 0; //lint !e423 // creation of memory leak. // No, Qt owns it.
	m_locomotionPriorityGroup        = 0; //lint !e423 // creation of memory leak. // No, Qt owns it.
	m_outGroupTransformPriorityGroup = 0; //lint !e423 // creation of memory leak. // No, Qt owns it.
	m_inGroupTransformPriorityGroup  = 0; //lint !e423 // creation of memory leak. // No, Qt owns it.
}

// ----------------------------------------------------------------------

void AnimationPriorityListItem::MaskedListItem::handleUpdate()
{
	//-- Retrieve UI state.
	const CrcLowerString &transformMaskName                  = m_transformMaskPath->getPath();
	const CrcLowerString &inGroupTransformPriorityGroupName  = m_inGroupTransformPriorityGroup->getPriorityGroupName();
	const CrcLowerString &outGroupTransformPriorityGroupName = m_outGroupTransformPriorityGroup->getPriorityGroupName();
	const CrcLowerString &locomotionPriorityGroupName        = m_locomotionPriorityGroup->getPriorityGroupName();

	//-- Set data model.
	getAnimationTemplate().setMaskedPriority(transformMaskName, inGroupTransformPriorityGroupName, outGroupTransformPriorityGroupName, locomotionPriorityGroupName);

	//-- Update the file model to modified.
	m_fileModel.setModifiedState(true);
}

// ======================================================================
// class AnimationPriorityListItem::UniformListItem
// ======================================================================

AnimationPriorityListItem::UniformListItem::UniformListItem(QListViewItem *newParent, FileModel &fileModel, ProxySkeletalAnimationTemplate &animationTemplate, const QString *preTextSortKey) :
	AnimationPriorityListItem(newParent, animationTemplate, preTextSortKey),
	m_fileModel(fileModel),
	m_priorityGroup(0)
{
	m_type = LITYPE_AnimationPriorityListItem_UniformListItem;
	//-- Set Text.
	UniformListItem::setText(0, "Uniform Priority");

	//-- Retrieve initial state from data model.
	CrcLowerString  transformPriorityGroupName;
	CrcLowerString  locomotionPriorityGroupName;

	DEBUG_FATAL(!animationTemplate.hasUniformPriority(), ("expecting uniform priority."));
	animationTemplate.getUniformPriority(transformPriorityGroupName, locomotionPriorityGroupName);

	WARNING(transformPriorityGroupName != locomotionPriorityGroupName, ("proxy for [%s] has uniform priority that differs for locomotion and transforms, ignoring locomotion priority.", animationTemplate.getTargetAnimationTemplateName().getString()));

	//-- Create PriorityGroup items with correct data.
	m_priorityGroup = new PriorityGroupListItem(this, *this, "Uniform Priority: ", transformPriorityGroupName);
}

// ----------------------------------------------------------------------

AnimationPriorityListItem::UniformListItem::~UniformListItem()
{
	m_priorityGroup = 0; //lint !e423 // creation of memory leak. // No, Qt owns it.
}

// ----------------------------------------------------------------------
/**
 * Commit UI changes to the data model.
 */

void AnimationPriorityListItem::UniformListItem::handleUpdate()
{
	//-- Retrieve UI state.
	const CrcLowerString  &priorityGroupName = m_priorityGroup->getPriorityGroupName();

	//-- Set data model.
	getAnimationTemplate().setUniformPriority(priorityGroupName, priorityGroupName);

	//-- Update the file model to modified.
	m_fileModel.setModifiedState(true);
}

// ======================================================================
// class AnimationPriorityListItem: public static member functions
// ======================================================================

AnimationPriorityListItem *AnimationPriorityListItem::createFromData(QListViewItem *newParent, FileModel &fileModel, ProxySkeletalAnimationTemplate &animationTemplate, const QString *preTextSortKey)
{
	if (animationTemplate.hasNoAssignedPriority())
	{
		//-- Don't create anything in this case.
		return 0;
	}
	else if (animationTemplate.hasUniformPriority())
	{
		return new UniformListItem(newParent, fileModel, animationTemplate, preTextSortKey);
	}
	else if (animationTemplate.hasMaskedPriority())
	{
		return new MaskedListItem(newParent, fileModel, animationTemplate, preTextSortKey);
	}
	else
	{
		WARNING(true, ("AnimationTemplate has unknown animation priority associated with it."));
		return 0;
	}
}

// ======================================================================
// class AnimationPriorityListItem: protected member functions
// ======================================================================

AnimationPriorityListItem::AnimationPriorityListItem(QListViewItem *newParent, ProxySkeletalAnimationTemplate &animationTemplate, const QString *preTextSortKey) :
	ListItem(newParent, preTextSortKey),
	m_animationTemplate(animationTemplate)
{
	m_type = LITYPE_AnimationPriorityListItem;
}

// ----------------------------------------------------------------------

AnimationPriorityListItem::~AnimationPriorityListItem()
{
}

// ======================================================================
