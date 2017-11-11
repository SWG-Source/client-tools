// ======================================================================
//
// SpeedAnimationChoiceListItem.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_SpeedAnimationChoiceListItem_H
#define INCLUDED_SpeedAnimationChoiceListItem_H

// ======================================================================

#include "AnimationEditor/ListItem.h"
#include "clientSkeletalAnimation/SpeedSkeletalAnimationTemplate.h"

class FileModel;
class QPixmap;

// ======================================================================

class SpeedAnimationChoiceListItem: public ListItem
{
	Q_OBJECT

public:

	SpeedAnimationChoiceListItem(QListViewItem *newParent, SpeedSkeletalAnimationTemplate &animationTemplate, SkeletalAnimationTemplate *choiceAnimationTemplate, FileModel &fileModel);
	
	virtual bool        supportsPopupMenu() const;
	virtual QPopupMenu *createPopupMenu() const;

	virtual bool        handleKeyPress(QKeyEvent *keyEvent);

public slots:

	void                createSkeletalAnimationTemplate(int templateIndex);
	void                clearLogicalAnimation();
	void                deleteChoiceAnimation();

private:

	static const QPixmap &getPixmap();

private:

	// disabled
	SpeedAnimationChoiceListItem();
	SpeedAnimationChoiceListItem(const SpeedAnimationChoiceListItem&);
	SpeedAnimationChoiceListItem &operator =(const SpeedAnimationChoiceListItem&);

private:

	SpeedSkeletalAnimationTemplate            &m_speedAnimationTemplate;
	FileModel                                 &m_fileModel;
	SkeletalAnimationTemplate                 *m_choiceAnimationTemplate;

};

// ======================================================================

#endif
