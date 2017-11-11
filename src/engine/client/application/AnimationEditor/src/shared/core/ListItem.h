// ======================================================================
//
// ListItem.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ListItem_H
#define INCLUDED_ListItem_H

// ======================================================================

#include <qlistview.h>
#include <qobject.h>

class QKeyEvent;
class QListView;
class QPopupMenu;
class QString;
class SkeletalAnimationTemplate;

// ======================================================================

class ListItem: public QListViewItem, public QObject
{
public:

	enum Type
	{
		LITYPE_ListItem = 0,
		LITYPE_TextListItem, // 1

		LITYPE_ActionAnimationListItem,//2
		LITYPE_ActionGeneratorListItem,
		LITYPE_ActionGeneratorListItem_ActionListItem,
		LITYPE_ActionGeneratorListItem_ActionWeightListItem,
		LITYPE_ActionGeneratorListItem_AnimationListItem,
		LITYPE_ActionGeneratorListItem_MinTimeListItem,
		LITYPE_ActionGeneratorListItem_MaxTimeListItem,
		LITYPE_ActionGroupListItem,//9

		LITYPE_ActionGroupRootListItem, //10
		LITYPE_ActionListItem,
		LITYPE_AnimationPriorityListItem,
		LITYPE_AnimationPriorityListItem_MaskedListItem,
		LITYPE_AnimationPriorityListItem_PathListItem,
		LITYPE_AnimationPriorityListItem_UniformListItem,
		LITYPE_DetailStateListItem,
		LITYPE_DirectionAnimationChoiceListItem,
		LITYPE_DirectionAnimationListItem,
		LITYPE_FolderListItem,//19

		LITYPE_IdleTextListItem, //20
		LITYPE_LinkAnimationTextListItem,
		LITYPE_LogicalAnimationListItem,
		LITYPE_MovementActionListItem,
		LITYPE_PriorityBlendAnimationListItem,
		LITYPE_PriorityBlendAnimationListItem_ChoiceListItem,
		LITYPE_PriorityGroupListItem,
		LITYPE_ProxyAnimationListItem,
		LITYPE_SpeedAnimationChoiceListItem,
		LITYPE_SpeedAnimationListItem,//29

		LITYPE_StateLinkListItem,//30
		LITYPE_StringSelectorAnimationListItem,
		LITYPE_StringSelectorChoiceListItem,
		LITYPE_StringSelectorChoiceListItem_AnimationListItem,
		LITYPE_StringSelectorChoiceListItem_ValueListItem,
		LITYPE_TimeScaleAnimationListItem,
		LITYPE_TimeScaleAnimationListItem_ScaledAnimationListItem,
		LITYPE_StringSelectorVarNameListItem,
		LITYPE_YawAnimationListItem,
		LITYPE_YawAnimationListItem_ChoiceListItem, //39
		
		numTypes
	};

	explicit ListItem(QListView *newParent, const QString *preTextSortKey = 0);
	explicit ListItem(QListViewItem *newParent, const QString *preTextSortKey = 0);
	virtual ~ListItem();

	void                expandItemAndChildren();
	void                collapseItemAndChildren();

	void                expandItemAndParents();
	void                deleteListItemChildren();

	void                makeOnlySelection();

	virtual bool        supportsPopupMenu() const;
	virtual QPopupMenu *createPopupMenu() const;
	virtual void        updateVisuals();

	virtual void        doDoubleClick();

	virtual QString     key(int column, bool ascending) const;

	virtual bool        supportsAttachingDownStreamAnimationTemplate() const;
	virtual void        attachDownStreamAnimationTemplate(SkeletalAnimationTemplate const *animationTemplate);

	virtual bool        handleKeyPress(QKeyEvent *keyEvent);

	ListItem*			findItemMatch(const char *subString, ListItem* selectedItem, bool &ignore, bool onlyTextReplaceableItems);

	ListItem::Type		getType();
	bool				canReplaceText();
	virtual void		replaceText(const char *subStringToReplace, const char* replacementText);
private:

	 // disabled
	ListItem();
	ListItem(const ListItem&);
	ListItem &operator =(const ListItem&);

private:

	QString *m_preTextSortKey;

protected:
	Type m_type;

};

// ======================================================================

#endif
