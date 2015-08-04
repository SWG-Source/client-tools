// ======================================================================
//
// ProxyAnimationListItem.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ProxyAnimationListItem_H
#define INCLUDED_ProxyAnimationListItem_H

// ======================================================================

#include "AnimationEditor/ListItem.h"

class AnimationPriorityListItem;
class ConstCharCrcLowerString;
class CrcLowerString;
class FileModel;
class ProxySkeletalAnimationTemplate;
class QListViewItem;

// ======================================================================

class ProxyAnimationListItem: public ListItem
{
	Q_OBJECT

public:

	ProxyAnimationListItem(QListViewItem *newParent, FileModel &fileModel, ProxySkeletalAnimationTemplate &animationTemplate);
	virtual ~ProxyAnimationListItem();

	virtual bool        supportsPopupMenu() const;
	virtual QPopupMenu *createPopupMenu() const;
	virtual void		replaceText(const char *subStringToReplace, const char* replacementText);

public slots:

	void setTargetAnimationTemplatePath();

	void addMaskedPriorityData();
	void addUniformPriorityData();
	void deletePriorityData();

private:

	static const QPixmap &getPixmap();

private:

	// disabled
	ProxyAnimationListItem();
	ProxyAnimationListItem(const ProxyAnimationListItem&);
	ProxyAnimationListItem &operator =(const ProxyAnimationListItem&);

private:

	static const ConstCharCrcLowerString  cms_defaultMaskName;
	static const ConstCharCrcLowerString  cms_defaultPriorityGroupName;

private:

	FileModel                      &m_fileModel;
	ProxySkeletalAnimationTemplate &m_animationTemplate;
	AnimationPriorityListItem      *m_priorityListItem;

};

// ======================================================================

#endif
