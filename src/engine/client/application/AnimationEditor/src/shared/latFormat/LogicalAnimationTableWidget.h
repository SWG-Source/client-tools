// ======================================================================
//
// LogicalAnimationTableWidget.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_LogicalAnimationTableWidget_H
#define INCLUDED_LogicalAnimationTableWidget_H

// ======================================================================

#include "AnimationEditor/BasicFileModel.h"
#include "BaseLogicalAnimationTableWidget.h"

class AnimationStateHierarchyTemplate;
class CrcLowerString;
class FolderListItem;
class LogicalAnimationListItem;
class LogicalAnimationTableTemplate;
class QListViewItem;
class QString;

// ======================================================================

class LogicalAnimationTableWidget: public BaseLogicalAnimationTableWidget, public BasicFileModel
{
	Q_OBJECT

public:

	LogicalAnimationTableWidget(QWidget *newParent, LogicalAnimationTableTemplate &latTemplate, const std::string &pathName);
	virtual ~LogicalAnimationTableWidget();

	// Event handling.
	virtual void                  keyPressEvent(QKeyEvent *event);

	// BasicFileModel inherited interface
	virtual bool                  save();
	virtual const std::string    &getFileTypeShortName() const;

	virtual CrcString const      &getFilePath() const;

	virtual void                  setFileTitle(const std::string &shortName);
	virtual const std::string     getFileTitle() const;

	virtual QWidget              *getWidget();

	virtual void                  expandAllChildrenForSelectedItem();
	virtual void                  collapseAllChildrenForSelectedItem();

	virtual void                  selectMatchingItem(const char* string, bool onlyReplaceableTextItems);
	virtual void                  replaceSelectedText(const char* substringToReplace, const char* newText);
	virtual void                  replaceAllText(const char* substringToReplace, const char* newText);

	virtual void                  setModifiedState(bool newModifiedState);


	LogicalAnimationListItem     *addLogicalAnimationNameUi(const CrcLowerString &logicalAnimationName);
	void                          clearLogicalAnimationUi();

public slots:

	void                       doListItemPopupMenu(QListViewItem *item, const QPoint &location, int column);
	void                       doDoubleClick(QListViewItem *item);
	void                       addNewLatEntries();
	void                       deleteStaleLatEntries();

private:

	static std::string         getNewFileTitleName();

private:

	std::string  getTemplateNameItemText() const;

	// disabled
	LogicalAnimationTableWidget();
	LogicalAnimationTableWidget(const LogicalAnimationTableWidget&);
	LogicalAnimationTableWidget &operator =(const LogicalAnimationTableWidget&);

private:

	static const std::string cms_directorySeparatorCharacters;
	static const std::string cms_fileTypeShortName;
	static const std::string cms_newFileTitleBase;
	static const QString     cms_templateNameSortKey;
	static const QString     cms_logicalAnimationFolderSortKey;

	static int               ms_newFileCount;

private:

	LogicalAnimationTableTemplate &m_latTemplate;

	std::string             *const m_pathName;
	std::string             *const m_fileTitle;

	QListViewItem                 *m_templateNameListItem;
	FolderListItem                *m_logicalAnimationFolder;
};

// ======================================================================

#endif
