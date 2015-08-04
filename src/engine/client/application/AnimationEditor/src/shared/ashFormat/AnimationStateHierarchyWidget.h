// ======================================================================
//
// AnimationStateHierarchyWidget.h
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_AnimationStateHierarchyWidget_H
#define INCLUDED_AnimationStateHierarchyWidget_H

// ======================================================================

#include "AnimationEditor/BasicFileModel.h"
#include "BaseAnimationStateHierarchyWidget.h"

class ActionGroupRootListItem;
class EditableAnimationStateHierarchyTemplate;
class ListItemAction;

// ======================================================================

class AnimationStateHierarchyWidget: public BaseAnimationStateHierarchyWidget, public BasicFileModel
{
public:

	AnimationStateHierarchyWidget(QWidget* newParent = 0, const char* newName = 0, WFlags fl = 0);
	AnimationStateHierarchyWidget(EditableAnimationStateHierarchyTemplate *hierarchy, const std::string &pathName, QWidget* newParent = 0, const char* newName = 0, WFlags fl = 0);
	virtual ~AnimationStateHierarchyWidget();

	void  setListItemAction(ListItemAction *action);
	// Event handling.
	virtual void                  keyPressEvent(QKeyEvent *event);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// interface from BasicFileModel
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	virtual bool                                   save();

	virtual const std::string                     &getFileTypeShortName() const;

	virtual CrcString const                       &getFilePath() const;

	virtual void                                   setFileTitle(const std::string &shortName);
	virtual const std::string                      getFileTitle() const;

	virtual QWidget                               *getWidget();

	virtual void                                   expandAllChildrenForSelectedItem();
	virtual void                                   collapseAllChildrenForSelectedItem();
	
	virtual void                                   selectMatchingItem(const char* string, bool onlyReplaceableTextItems);
	virtual void                                   replaceSelectedText(const char* substringToReplace, const char* newText);
	virtual void                                   replaceAllText(const char* substringToReplace, const char* newText);

	const EditableAnimationStateHierarchyTemplate &getConstHierarchyTemplate() const;
	EditableAnimationStateHierarchyTemplate       &getHierarchyTemplate();

public slots:

	virtual void doListItemPopupMenu(QListViewItem *item, const QPoint &location, int column);
	virtual void doUpdateListItemVisuals(QListViewItem *item);
	virtual void handleListItemSelectionChange(QListViewItem *item);
	virtual void doDoubleClick(QListViewItem *item);

private:

	void  doCommonConstructionTasks();

	// disabled
	AnimationStateHierarchyWidget(const AnimationStateHierarchyWidget&);
	AnimationStateHierarchyWidget &operator =(const AnimationStateHierarchyWidget&);

private:

	static const std::string  cms_fileTypeShortName;
	static const std::string  cms_directorySeparatorCharacters;

private:

	EditableAnimationStateHierarchyTemplate *const m_hierarchy;

	std::string                             *const m_fileTitle;
	std::string                             *const m_pathName;

	ListItemAction                                *m_listItemAction;
	ActionGroupRootListItem                       *m_actionGroupRootListItem;

};

// ======================================================================

inline const EditableAnimationStateHierarchyTemplate &AnimationStateHierarchyWidget::getConstHierarchyTemplate() const
{
	return *m_hierarchy;
}

// ----------------------------------------------------------------------

inline EditableAnimationStateHierarchyTemplate &AnimationStateHierarchyWidget::getHierarchyTemplate()
{
	return *m_hierarchy;
}

// ======================================================================

#endif
