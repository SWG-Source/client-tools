// ======================================================================
//
// ActionGeneratorListItem.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ActionGeneratorListItem_H
#define INCLUDED_ActionGeneratorListItem_H

// ======================================================================

#include "AnimationEditor/ListItem.h"
#include "AnimationEditor/TextListItem.h"

#include <string>

class ActionGeneratorSkeletalAnimationTemplate;
class FileModel;
class FolderListItem;

// ======================================================================

class ActionGeneratorListItem: public ListItem
{
	Q_OBJECT

public:

	class ActionListItem;
	class ActionWeightListItem;
	class AnimationListItem;
	class MinTimeListItem;
	class MaxTimeListItem;

public:

	ActionGeneratorListItem(QListViewItem *newParent, ActionGeneratorSkeletalAnimationTemplate &generatorTemplate, FileModel &fileModel);

	virtual bool        supportsPopupMenu() const;
	virtual QPopupMenu *createPopupMenu() const;

	virtual bool        supportsAttachingDownStreamAnimationTemplate() const;
	virtual void        attachDownStreamAnimationTemplate(SkeletalAnimationTemplate const *animationTemplate);

	const ActionGeneratorSkeletalAnimationTemplate &getGeneratorTemplate() const;
	ActionGeneratorSkeletalAnimationTemplate       &getGeneratorTemplate();

	FileModel                                      &getFileModel();

public slots:

	void  addNewAction();

private:

	static const QPixmap &getPixmap();
	static std::string    getNewActionName();

private:

	// Disabled.
	ActionGeneratorListItem();
	ActionGeneratorListItem(const ActionGeneratorListItem&);
	ActionGeneratorListItem &operator =(const ActionGeneratorListItem&);

private:

	static const QString  cms_minTimeSortKey;
	static const QString  cms_maxTimeSortKey;
	static const QString  cms_actionFolderSortKey;
	static const QString  cms_loopingAnimationSortKey;

	static int            ms_newActionEntryCount;

private:

	ActionGeneratorSkeletalAnimationTemplate &m_generatorTemplate;
	FileModel                                &m_fileModel;
	FolderListItem                           *m_actionFolder;

};

// ======================================================================

class ActionGeneratorListItem::AnimationListItem: public ListItem
{
	Q_OBJECT

public:

	AnimationListItem(ActionGeneratorListItem &generatorParent, const QString *preTextSortKey = 0);

	virtual bool        supportsPopupMenu() const;
	virtual QPopupMenu *createPopupMenu() const;

	virtual bool        handleKeyPress(QKeyEvent *keyEvent);

	void                buildUi();

public slots:

	void  createSkeletalAnimationTemplate(int templateIndex);
	void  clearLogicalAnimation();

private:

	static const QPixmap &getPixmap();

private:

	// Disabled.
	AnimationListItem();
	AnimationListItem(const AnimationListItem&);
	AnimationListItem &operator =(const AnimationListItem&);

private:

	ActionGeneratorListItem &m_generatorParent;

};

// ======================================================================

class ActionGeneratorListItem::ActionListItem: public TextListItem
{
	Q_OBJECT

public:

	ActionListItem(QListViewItem *newParent, ActionGeneratorListItem &generatorParent, const std::string &actionName);

	virtual bool        supportsPopupMenu() const;
	virtual QPopupMenu *createPopupMenu() const;

	const std::string                        &getActionName() const;
	ActionGeneratorSkeletalAnimationTemplate &getGeneratorTemplate();

public slots:

	void  removeAction();

protected:

	virtual bool         modifyCandidateText(std::string &candidateText) const;
	virtual void         setSourceText(const std::string &newText) const;
	virtual std::string  getSourceText() const;

private:

	static const QPixmap &getPixmap();

private:

	// Disabled.
	ActionListItem();
	ActionListItem(const ActionListItem&);
	ActionListItem &operator =(const ActionListItem&);

private:

	ActionGeneratorListItem &m_generatorParent;
	mutable std::string      m_actionName;

};

// ======================================================================

inline const ActionGeneratorSkeletalAnimationTemplate &ActionGeneratorListItem::getGeneratorTemplate() const
{
	return m_generatorTemplate;
}

// ----------------------------------------------------------------------

inline ActionGeneratorSkeletalAnimationTemplate &ActionGeneratorListItem::getGeneratorTemplate()
{
	return m_generatorTemplate;
}

// ----------------------------------------------------------------------

inline FileModel &ActionGeneratorListItem::getFileModel()
{
	return m_fileModel;
}

// ======================================================================

#endif
