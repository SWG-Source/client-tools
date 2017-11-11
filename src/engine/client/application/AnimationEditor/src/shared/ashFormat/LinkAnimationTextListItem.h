// ======================================================================
//
// LinkAnimationTextListItem.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_LinkAnimationTextListItem_H
#define INCLUDED_LinkAnimationTextListItem_H

// ======================================================================

#include "AnimationEditor/TextListItem.h"

class FileModel;
class EditableAnimationStateLink;
class QPixmap;

// ======================================================================
/**
 * UI for modifying Animation State Link's optional logical animation name.
 */

class LinkAnimationTextListItem: public TextListItem
{
public:

	LinkAnimationTextListItem(QListViewItem *newParent, EditableAnimationStateLink &link, FileModel *fileModel);

protected:

	virtual bool         modifyCandidateText(std::string &candidateText) const;
	virtual void         setSourceText(const std::string &newText) const;
	virtual std::string  getSourceText() const;

private:

	// disabled
	LinkAnimationTextListItem();
	LinkAnimationTextListItem(const LinkAnimationTextListItem&);
	LinkAnimationTextListItem &operator =(const LinkAnimationTextListItem&);

private:

	static const QPixmap &getPixmap();

private:

	EditableAnimationStateLink &m_link;

};

// ======================================================================

#endif
