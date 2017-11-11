// ======================================================================
//
// StringSelectorVarNameListItem.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_StringSelectorVarNameListItem_H
#define INCLUDED_StringSelectorVarNameListItem_H

// ======================================================================

#include "AnimationEditor/TextListItem.h"

class FileModel;
class StringSelectorSkeletalAnimationTemplate;

// ======================================================================

class StringSelectorVarNameListItem: public TextListItem
{
public:

	StringSelectorVarNameListItem(QListViewItem *newParent, StringSelectorSkeletalAnimationTemplate &selectorTemplate, FileModel &fileModel, const QString *preTextSortKey = 0);

	virtual bool         modifyCandidateText(std::string &candidateText) const;
	virtual void         setSourceText(const std::string &newText) const;
	virtual std::string  getSourceText() const;

private:

	static const QPixmap &getPixmap();

private:

	// Disabled.
	StringSelectorVarNameListItem();
	StringSelectorVarNameListItem(const StringSelectorVarNameListItem&);
	StringSelectorVarNameListItem &operator =(const StringSelectorVarNameListItem&);

private:

	StringSelectorSkeletalAnimationTemplate &m_selectorTemplate;

};

// ======================================================================

#endif
