// ======================================================================
//
// BasicFileModel.h
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_BasicFileModel_H
#define INCLUDED_BasicFileModel_H

// ======================================================================

#include "AnimationEditor/FileModel.h"

// ======================================================================

class BasicFileModel: public FileModel
{
public:

	BasicFileModel();
	virtual ~BasicFileModel();

	virtual bool                  isModified() const;
	virtual void                  setModifiedState(bool newModifiedState);

	virtual void                  addModificationListener(ModificationListener listener, const void *context);
	virtual void                  removeModificationListener(ModificationListener listener, const void *context);

	virtual bool                  save() = 0;

	virtual const std::string    &getFileTypeShortName() const = 0;

	virtual CrcString const      &getFilePath() const = 0;

	virtual void                  setFileTitle(const std::string &shortName) = 0;
	virtual const std::string     getFileTitle() const = 0;

	virtual QWidget              *getWidget() = 0;

	virtual void                  expandAllChildrenForSelectedItem() = 0;
	virtual void                  collapseAllChildrenForSelectedItem() = 0;

	virtual void                  selectMatchingItem(const char* string, bool onlyReplaceableTextItems) = 0;
	virtual void                  replaceSelectedText(const char* substringToReplace,const char* newText) = 0;
	virtual void                  replaceAllText(const char* substringToReplace, const char* newText) = 0;

private:

	class ListenerData;
	typedef stdvector<ListenerData*>::fwd  ListenerDataVector;

private:

	// disabled
	BasicFileModel(const BasicFileModel&);
	BasicFileModel &operator =(const BasicFileModel&);

private:

	bool                      m_isModified;
	ListenerDataVector *const m_listenerDataVector;

};

// ======================================================================

#endif
