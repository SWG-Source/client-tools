// ======================================================================
//
// ActionAnimationListItem.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ActionAnimationListItem_H
#define INCLUDED_ActionAnimationListItem_H

// ======================================================================

#include "AnimationEditor/TextListItem.h"

class EditableBasicAnimationAction;

// ======================================================================
/**
 * A UI element for displaying and editing the LogicalAnimation name defined for
 * an animation action.
 */

class ActionAnimationListItem: public TextListItem
{
public:

	ActionAnimationListItem(QListViewItem *newParent, EditableBasicAnimationAction &action, FileModel *fileModel, const QString *preTextSortKey = 0);

private:

	static const QPixmap &getPixmap();

private:

	virtual bool         modifyCandidateText(std::string &candidateText) const;
	virtual void         setSourceText(const std::string &newText) const;
	virtual std::string  getSourceText() const;

	// disabled
	ActionAnimationListItem();
	ActionAnimationListItem(const ActionAnimationListItem&);
	ActionAnimationListItem &operator =(const ActionAnimationListItem&);

private:

	EditableBasicAnimationAction &m_action;

};

// ======================================================================

#endif
