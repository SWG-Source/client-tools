// ======================================================================
//
// ActionGroupListItem.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ActionGroupListItem_H
#define INCLUDED_ActionGroupListItem_H

// ======================================================================

#include "AnimationEditor/TextListItem.h"

class EditableAnimationActionGroup;
class FileModel;
class QPixmap;

// ======================================================================

class ActionGroupListItem: public TextListItem
{
	Q_OBJECT

public:

	ActionGroupListItem(QListViewItem *newParent, EditableAnimationActionGroup &actionGroup, FileModel &fileModel);

	EditableAnimationActionGroup &getActionGroup();

	virtual bool         supportsPopupMenu() const;
	virtual QPopupMenu  *createPopupMenu() const;

public slots:

	void  createNewAction();

private:

	static const QPixmap &getPixmap();

private:

	virtual bool         modifyCandidateText(std::string &candidateText) const;
	virtual void         setSourceText(const std::string &newText) const;
	virtual std::string  getSourceText() const;

	// disabled
	ActionGroupListItem();
	ActionGroupListItem(const ActionGroupListItem&);
	ActionGroupListItem &operator =(const ActionGroupListItem&);

private:

	EditableAnimationActionGroup &m_actionGroup;
	FileModel                    &m_fileModel;

};

// ======================================================================

inline EditableAnimationActionGroup &ActionGroupListItem::getActionGroup()
{
	return m_actionGroup;
}

// ======================================================================

#endif
