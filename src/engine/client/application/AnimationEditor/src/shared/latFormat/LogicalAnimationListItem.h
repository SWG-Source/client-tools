// ======================================================================
//
// LogicalAnimationListItem.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDE_LogicalAnimationListItem_H
#define INCLUDE_LogicalAnimationListItem_H

// ======================================================================

#include "AnimationEditor/ListItem.h"

class CrcLowerString;
class FileModel;
class LogicalAnimationTableTemplate;
class QPixmap;

// ======================================================================

class LogicalAnimationListItem: public ListItem
{
	Q_OBJECT

public:

	LogicalAnimationListItem(QListViewItem *newParent, LogicalAnimationTableTemplate &latTemplate, const CrcLowerString &logicalAnimationName, FileModel &fileModel);
	virtual ~LogicalAnimationListItem();

	virtual bool        supportsPopupMenu() const;
	virtual QPopupMenu *createPopupMenu() const;

	virtual bool        handleKeyPress(QKeyEvent *keyEvent);

public slots:

	void  createSkeletalAnimationTemplate(int templateIndex);
	void  clearLogicalAnimation();

private:

	static const QPixmap &getPixmap();

private:

	// disabled
	LogicalAnimationListItem();
	LogicalAnimationListItem(const LogicalAnimationListItem&);
	LogicalAnimationListItem &operator =(const LogicalAnimationListItem&);

private:

	LogicalAnimationTableTemplate &m_latTemplate;
	CrcLowerString          *const m_logicalAnimationName;
	FileModel                     &m_fileModel;

};

// ======================================================================

#endif
