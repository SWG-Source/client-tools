// ======================================================================
//
// StringSelectorChoiceListItem.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_StringSelectorChoiceListItem_H
#define INCLUDED_StringSelectorChoiceListItem_H

// ======================================================================

#include "AnimationEditor/ListItem.h"
#include "AnimationEditor/TextListItem.h"

class FileModel;
class FolderListItem;
class SkeletalAnimationTemplate;
class StringSelectorAnimationListItem;
class StringSelectorSkeletalAnimationTemplate;

// ======================================================================

class StringSelectorChoiceListItem: public ListItem
{
	Q_OBJECT

public:

	class AnimationListItem;
	class ValueListItem;

public:

	StringSelectorChoiceListItem(QListViewItem *newParent, const SkeletalAnimationTemplate *choiceTemplate, StringSelectorAnimationListItem &selectorListItem, StringSelectorSkeletalAnimationTemplate &selectorTemplate, FileModel &fileModel);
	virtual ~StringSelectorChoiceListItem();

	virtual bool                             supportsPopupMenu() const;
	virtual QPopupMenu                      *createPopupMenu() const;

	FileModel                               &getFileModel();
	StringSelectorSkeletalAnimationTemplate &getSelectorTemplate();
	const SkeletalAnimationTemplate         *getChoiceTemplate() const;
	void                                     setChoiceTemplate(const SkeletalAnimationTemplate *choiceTemplate);

public slots:

	void  createNewValue();

private:

	static const QPixmap &getPixmap();
	static std::string    getNewValueText();

private:

	// Disabled.
	StringSelectorChoiceListItem();
	StringSelectorChoiceListItem(const StringSelectorChoiceListItem&);
	StringSelectorChoiceListItem &operator =(const StringSelectorChoiceListItem&);

private:

	static const QString cms_animationSortKey;
	static const QString cms_valueFolderSortKey;

	static int           ms_newValueCounter;

private:

	const SkeletalAnimationTemplate         *m_choiceTemplate;
	StringSelectorAnimationListItem         &m_selectorListItem;
	StringSelectorSkeletalAnimationTemplate &m_selectorTemplate;
	FileModel                               &m_fileModel;
	FolderListItem                          *m_valueFolder;

};

// ======================================================================

class StringSelectorChoiceListItem::AnimationListItem: public ListItem
{
	Q_OBJECT

public:

	AnimationListItem(StringSelectorChoiceListItem &choiceParent, const QString *preTextSortKey = 0);

	virtual bool        supportsPopupMenu() const;
	virtual QPopupMenu *createPopupMenu() const;

	virtual bool        handleKeyPress(QKeyEvent *keyEvent);

public slots:

	void createSkeletalAnimationTemplate(int templateIndex);
	void clearLogicalAnimation();

private:

	static const QPixmap     &getPixmap();

private:

	// disabled
	AnimationListItem();
	AnimationListItem(const AnimationListItem&);
	AnimationListItem &operator =(const AnimationListItem&);

private:

	StringSelectorChoiceListItem    &m_choiceParent;

};

// ======================================================================

class StringSelectorChoiceListItem::ValueListItem: public TextListItem
{
	Q_OBJECT

public:

	ValueListItem(QListViewItem *newParent, StringSelectorChoiceListItem &choiceParent, const std::string &value, FileModel &fileModel, bool createMapping);

	virtual bool         modifyCandidateText(std::string &candidateText) const;
	virtual void         setSourceText(const std::string &newText) const;
	virtual std::string  getSourceText() const;

	virtual bool         supportsPopupMenu() const;
	virtual QPopupMenu  *createPopupMenu() const;

	virtual void         doDoubleClick();

public slots:

	void  deleteValue();

private:

	static const QPixmap &getPixmap();

private:

	// Disabled.
	ValueListItem();
	ValueListItem(const ValueListItem&);
	ValueListItem &operator =(const ValueListItem&);

private:

	StringSelectorChoiceListItem &m_choiceParent;
	mutable std::string           m_value;

};

// ======================================================================

inline FileModel &StringSelectorChoiceListItem::getFileModel()
{
	return m_fileModel;
}

// ----------------------------------------------------------------------

inline StringSelectorSkeletalAnimationTemplate &StringSelectorChoiceListItem::getSelectorTemplate()
{
	return m_selectorTemplate;
}

// ----------------------------------------------------------------------

inline const SkeletalAnimationTemplate *StringSelectorChoiceListItem::getChoiceTemplate() const
{
	return m_choiceTemplate;
}

// ----------------------------------------------------------------------

inline void StringSelectorChoiceListItem::setChoiceTemplate(const SkeletalAnimationTemplate *choiceTemplate)
{
	m_choiceTemplate = choiceTemplate;
}

// ======================================================================

#endif
