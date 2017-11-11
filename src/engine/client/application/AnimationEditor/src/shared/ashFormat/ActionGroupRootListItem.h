// ======================================================================
//
// ActionGroupRootListItem.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ActionGroupRootListItem_H
#define INCLUDED_ActionGroupRootListItem_H

// ======================================================================

#include "AnimationEditor/ListItem.h"

class EditableAnimationStateHierarchyTemplate;
class FileModel;
class QListViewItem;

// ======================================================================

class ActionGroupRootListItem: public ListItem
{
	Q_OBJECT

public:

	ActionGroupRootListItem(QListView *newParent, EditableAnimationStateHierarchyTemplate &hierarchyTemplate, FileModel &fileModel);

	virtual bool        supportsPopupMenu() const;
	virtual QPopupMenu *createPopupMenu() const;

public slots:

	void  createNewActionGroup();

private:

	// disabled
	ActionGroupRootListItem();
	ActionGroupRootListItem(const ActionGroupRootListItem&);
	ActionGroupRootListItem &operator =(const ActionGroupRootListItem&);

private:

	static const std::string  cms_actionGroupRootItemText;

private:

	EditableAnimationStateHierarchyTemplate &m_hierarchyTemplate;
	FileModel                               &m_fileModel;

};

// ======================================================================

#endif
