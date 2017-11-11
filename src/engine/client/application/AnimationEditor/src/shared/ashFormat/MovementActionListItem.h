// ======================================================================
//
// MovementActionListItem.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_MovementActionListItem_H
#define INCLUDED_MovementActionListItem_H

// ======================================================================

#include "AnimationEditor/TextListItem.h"

class EditableBasicAnimationAction;
class EditableMovementAnimationAction;
class FileModel;
class QListViewItem;

// ======================================================================

class MovementActionListItem: public TextListItem
{
	Q_OBJECT

public:

	class ApplyAddListItem;

public:

	MovementActionListItem(QListViewItem *newParent, EditableMovementAnimationAction &action, FileModel *fileModel);

	virtual bool                     supportsPopupMenu() const;
	virtual QPopupMenu              *createPopupMenu() const;

	virtual void                     doDoubleClick();

	EditableMovementAnimationAction &getAction();

public slots:

	void  deleteAction();

private:

	// disabled
	MovementActionListItem();
	MovementActionListItem(const MovementActionListItem&);
	MovementActionListItem &operator =(const MovementActionListItem&);

private:

	static const QPixmap &getPixmap();

private:

	void                 createChildAction(const QString &childName, const QString &childSortKey, FileModel *fileModel, EditableBasicAnimationAction &childAction);

	virtual bool         modifyCandidateText(std::string &candidateText) const;
	virtual void         setSourceText(const std::string &newText) const;
	virtual std::string  getSourceText() const;

private:

	static const QString  cms_animationSortKey;
	static const QString  cms_speedChildName;
	static const QString  cms_speedChildSortKey;
	static const QString  cms_trackSelectorSortKey;
	static const QString  cms_zeroSpeedChildName;
	static const QString  cms_zeroSpeedChildSortKey;

private:

	EditableMovementAnimationAction &m_action;

};

// ======================================================================

class MovementActionListItem::ApplyAddListItem: public ListItem
{
public:

	ApplyAddListItem(QListViewItem &newParent, EditableBasicAnimationAction &action, FileModel *fileModel, const QString *preTextSortKey = 0);

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

	EditableBasicAnimationAction &m_action;
	FileModel                    *m_fileModel;

};

// ======================================================================

inline EditableMovementAnimationAction &MovementActionListItem::getAction()
{
	return m_action;
}

// ======================================================================

#endif
