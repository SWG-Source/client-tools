// ======================================================================
//
// DirectionAnimationChoiceListItem.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_DirectionAnimationChoiceListItem_H
#define INCLUDED_DirectionAnimationChoiceListItem_H

// ======================================================================

#include "AnimationEditor/ListItem.h"
#include "clientSkeletalAnimation/DirectionSkeletalAnimationTemplate.h"

class FileModel;
class QPixmap;

// ======================================================================

class DirectionAnimationChoiceListItem: public ListItem
{
	Q_OBJECT

public:

	DirectionAnimationChoiceListItem(QListViewItem *newParent, DirectionSkeletalAnimationTemplate &animationTemplate, DirectionSkeletalAnimationTemplate::Magnitude xMagnitude, DirectionSkeletalAnimationTemplate::Magnitude zMagnitude, FileModel &fileModel);
	
	virtual bool        supportsPopupMenu() const;
	virtual QPopupMenu *createPopupMenu() const;

	virtual bool        handleKeyPress(QKeyEvent *keyEvent);

	DirectionSkeletalAnimationTemplate::Magnitude  getXMagnitude() const;
	DirectionSkeletalAnimationTemplate::Magnitude  getZMagnitude() const;

	void                                           buildUi();

public slots:

	void                createSkeletalAnimationTemplate(int templateIndex);
	void                clearLogicalAnimation();

private:

	static const QPixmap &getPixmap();

private:

	// disabled
	DirectionAnimationChoiceListItem();
	DirectionAnimationChoiceListItem(const DirectionAnimationChoiceListItem&);
	DirectionAnimationChoiceListItem &operator =(const DirectionAnimationChoiceListItem&);

private:

	static const char *const cms_xMagnitudeString[];
	static const char *const cms_zMagnitudeString[];

private:

	DirectionSkeletalAnimationTemplate            &m_directionAnimationTemplate;
	FileModel                                     &m_fileModel;

	DirectionSkeletalAnimationTemplate::Magnitude  m_xMagnitude;
	DirectionSkeletalAnimationTemplate::Magnitude  m_zMagnitude;

};

// ======================================================================

#endif
