// ======================================================================
//
// ActionListItem.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ActionListItem_H
#define INCLUDED_ActionListItem_H

// ======================================================================

#include "AnimationEditor/TextListItem.h"

class EditableBasicAnimationAction;
class FileModel;
class QListViewItem;

// ======================================================================

class ActionListItem: public TextListItem
{
	Q_OBJECT

public:

	class ApplyAddListItem;

public:

	ActionListItem(QListViewItem *newParent, EditableBasicAnimationAction &action, FileModel *fileModel);

	virtual bool                  supportsPopupMenu() const;
	virtual QPopupMenu           *createPopupMenu() const;

	virtual void                  doDoubleClick();

	EditableBasicAnimationAction &getAction();

public slots:

	void  deleteAction();
	void  upgradeToMovementAction();

private:

	// disabled
	ActionListItem();
	ActionListItem(const ActionListItem&);
	ActionListItem &operator =(const ActionListItem&);

private:

	static const QPixmap &getPixmap();

private:

	virtual bool         modifyCandidateText(std::string &candidateText) const;
	virtual void         setSourceText(const std::string &newText) const;
	virtual std::string  getSourceText() const;

private:

	static const QString  cms_animationSortKey;
	static const QString  cms_trackSelectorSortKey;

private:

	EditableBasicAnimationAction &m_action;

};

// ======================================================================

class ActionListItem::ApplyAddListItem: public ListItem
{
public:

	ApplyAddListItem(ActionListItem &actionParent, FileModel *fileModel, const QString *preTextSortKey = 0);

	virtual void  doDoubleClick();

private:

	static const QPixmap &getPixmap();

private:

	void  updateUiText();

	// Disabled.
	ApplyAddListItem();
	ApplyAddListItem(const ApplyAddListItem&);
	ApplyAddListItem &operator =(const ApplyAddListItem&);

private:

	ActionListItem &m_actionParent;
	FileModel      *m_fileModel;

};

// ======================================================================

inline EditableBasicAnimationAction &ActionListItem::getAction()
{
	return m_action;
}

// ======================================================================

#endif
