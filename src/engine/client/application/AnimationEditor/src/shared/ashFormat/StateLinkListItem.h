// ======================================================================
//
// StateLinkListItem.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_StateLinkListItem_H
#define INCLUDED_StateLinkListItem_H

// ======================================================================

#include "AnimationEditor/ListItem.h"

class EditableAnimationState;
class EditableAnimationStateLink;
class FileModel;
class QPixmap;

// ======================================================================

class StateLinkListItem: public ListItem
{
	Q_OBJECT

public:

	StateLinkListItem(ListItem *newParent, EditableAnimationState &sourceState, EditableAnimationStateLink &link, FileModel *fileModel);

	virtual bool        supportsPopupMenu() const;
	virtual QPopupMenu *createPopupMenu() const;

	EditableAnimationStateLink &getStateLink();

public slots:

	void  deleteStateLink();

private:

	static const QPixmap &getPixmap();

private:

	// disabled
	StateLinkListItem();
	StateLinkListItem(const StateLinkListItem&);
	StateLinkListItem &operator =(const StateLinkListItem&);
	
private:

	EditableAnimationState     &m_sourceState;
	EditableAnimationStateLink &m_link;

};

// ======================================================================

inline EditableAnimationStateLink &StateLinkListItem::getStateLink()
{
	return m_link;
}

// ======================================================================

#endif
