// ======================================================================
//
// TextListItem.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_TextListItem_H
#define INCLUDED_TextListItem_H

// ======================================================================

#include "AnimationEditor/ListItem.h"

class FileModel;
class QPixmap;

// ======================================================================

class TextListItem: public ListItem
{
protected:

	TextListItem(QListViewItem *newParent, const std::string &initialText, const QPixmap &newPixmap, FileModel *fileModel, const QString *preTextSortKey = 0);
	virtual ~TextListItem();

	virtual bool         modifyCandidateText(std::string &candidateText) const;
	virtual void         setSourceText(const std::string &newText) const = 0;
	virtual std::string  getSourceText() const = 0;

private:

	virtual void         okRename(int col);

	// disabled
	TextListItem();
	TextListItem(const TextListItem&);
	TextListItem &operator =(const TextListItem&);

private:

	FileModel *const m_fileModel;

};

// ======================================================================

#endif
