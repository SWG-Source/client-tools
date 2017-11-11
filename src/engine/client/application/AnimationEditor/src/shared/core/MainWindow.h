// ======================================================================
//
// MainWindow.h
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_MainWindow_H
#define INCLUDED_MainWindow_H

// ======================================================================

#include "BaseMainWindow.h"

class AnimationStateHierarchyTemplate;
class BaseAnimationStateHierarchyWidget;
class FileModel;
class FindDialog;
class FindReplaceDialog;

// ======================================================================

class MainWindow: public BaseMainWindow
{
public:

	static MainWindow &getInstance();

public:

	MainWindow(QWidget* newParent = 0, const char* newName = 0, WFlags fl = WType_TopLevel);
	~MainWindow();

	void selectMatchingItem(const char* string, bool onlyReplaceableTextItems);
	void replaceSelectedText(const char* substringToReplace, const char* newText);
	void replaceAllText(const char* substringToReplace, const char* newText);
public:

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// virtual public slots
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	virtual void  newAnimationStateHierarchy();
	virtual void  newLogicalAnimationTable();
	virtual void  closeSelectedTab();
	virtual void  markSelectedTabModified();
	virtual void  syncFocusController();
	virtual void  openFile();
	virtual void  openTargetLatFiles();
	virtual void  openTargetAshFiles();
	virtual void  saveSelectedTab();
	virtual void  expandAllChildren();
	virtual void  collapseAllChildren();
	virtual void  openSharedCreatureTemplate();
	virtual void openFindDialog();
    virtual void openFindReplaceDialog();

private:

	struct  FileModelData;
	typedef stdvector<FileModel*>::fwd  FileModelVector;

private:

	static std::string  getNextNewTitle();
	static std::string  formatTabName(const std::string &fileTypeName, const std::string &fileTitle, bool isModified);

	static void staticModificationListener(FileModel &fileModel, bool oldModifiedState, bool newModifiedState, const void *context);

private:

	void       addFileModel(FileModel *fileModel);
	bool       getSelectedFileModel(FileModel *&fileModel, int &index);

	void       modificationListener(FileModel &fileModel,bool oldModifiedState, bool newModifiedState);

	void       doFileOpen(const std::string &treeFileRelativePath);

	// disabled
	MainWindow(const MainWindow&);
	MainWindow &operator =(const MainWindow&);

private:

	static const std::string  ms_ashTabNamePrefix;
	static const std::string  ms_latTabNamePrefix;
	static const std::string  ms_unnamedTabNamePrefix;

	static int         ms_newTabCounter;
	static MainWindow *ms_theMainWindow;

private:

	FileModelVector *const m_fileModelVector;

	FindDialog* m_findDialog;
	FindReplaceDialog* m_findReplaceDialog;

};

// ======================================================================

#endif
