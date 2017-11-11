// ======================================================================
//
// FileModel.h
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_FileModel_H
#define INCLUDED_FileModel_H

// ======================================================================

class CrcString;
class QWidget;

// ======================================================================
/**
 * An interface to a model for a file containing some sort of data.
 *
 * This interface class exposes members for checking the modification
 * state and for saving the contents of a file.
 */

class FileModel
{
public:

	typedef void (*ModificationListener)(FileModel &fileModel, bool oldModifiedState, bool newModifiedState, const void *context);

public:

	virtual                      ~FileModel() = 0;

	virtual bool                  isModified() const = 0;
	virtual void                  setModifiedState(bool newModifiedState) = 0;

	virtual void                  addModificationListener(ModificationListener listener, const void *context) = 0;
	virtual void                  removeModificationListener(ModificationListener listener, const void *context) = 0;

	virtual bool                  save() = 0;

	virtual const std::string    &getFileTypeShortName() const = 0;

	virtual CrcString const      &getFilePath() const = 0;

	virtual void                  setFileTitle(const std::string &shortName) = 0;
	virtual const std::string     getFileTitle() const = 0;

	virtual QWidget              *getWidget() = 0;

	virtual void                  expandAllChildrenForSelectedItem() = 0;
	virtual void                  collapseAllChildrenForSelectedItem() = 0;

	virtual void                  selectMatchingItem(const char* string, bool onlyReplaceableTextItems) = 0;
	virtual void                  replaceSelectedText(const char* substringToReplace, const char* newText) = 0;
	virtual void                  replaceAllText(const char* substringToReplace, const char* newText) = 0;

};

// ======================================================================

#endif
