// ======================================================================
//
// IdleTextListItem.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_IdleTextListItem_H
#define INCLUDED_IdleTextListItem_H

// ======================================================================

#include "AnimationEditor/TextListItem.h"

class FileModel;
class EditableAnimationState;
class QPixmap;

// ======================================================================
/**
 * UI for modifying Animation State's idle/looping animation.
 */

class IdleTextListItem: public TextListItem
{
public:

	IdleTextListItem(QListViewItem *newParent, EditableAnimationState &state, FileModel *fileModel, const QString *preTextSortKey = 0);

protected:

	virtual bool         modifyCandidateText(std::string &candidateText) const;
	virtual void         setSourceText(const std::string &newText) const;
	virtual std::string  getSourceText() const;

private:

	// disabled
	IdleTextListItem();
	IdleTextListItem(const IdleTextListItem&);
	IdleTextListItem &operator =(const IdleTextListItem&);

private:

	static const QPixmap &getPixmap();

private:

	EditableAnimationState &m_state;

};

// ======================================================================

#endif
