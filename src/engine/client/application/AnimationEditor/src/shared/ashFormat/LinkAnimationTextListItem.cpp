// ======================================================================
//
// LinkAnimationTextListItem.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"
#include "AnimationEditor/LinkAnimationTextListItem.h"

#include "AnimationEditor/EmbeddedImageLoader.h"
#include "AnimationEditor/FileModel.h"
#include "clientSkeletalAnimation/EditableAnimationStateLink.h"
#include "sharedFoundation/CrcLowerString.h"

#include <algorithm>
#include <qpixmap.h>
#include <string>

// ======================================================================
// class LinkAnimationTextListItem: public member functions
// ======================================================================

LinkAnimationTextListItem::LinkAnimationTextListItem(QListViewItem *newParent, EditableAnimationStateLink &link, FileModel *fileModel) :
	TextListItem(newParent, link.getTransitionLogicalAnimationName().getString(), getPixmap(), fileModel),
	m_link(link)
{
	m_type = LITYPE_LinkAnimationTextListItem;
}

// ======================================================================
// class LinkAnimationTextListItem: protected member functions
// ======================================================================

bool LinkAnimationTextListItem::modifyCandidateText(std::string & candidateText) const
{
	//-- Convert text to lower case.
	std::transform(candidateText.begin(), candidateText.end(), candidateText.begin(), tolower);

	//-- Accept all text changes.
	return true;
}

// ----------------------------------------------------------------------

void LinkAnimationTextListItem::setSourceText(const std::string &newText) const
{
	m_link.setTransitionLogicalAnimationName(CrcLowerString(newText.c_str()));
}

// ----------------------------------------------------------------------

std::string LinkAnimationTextListItem::getSourceText() const
{
	return m_link.getTransitionLogicalAnimationName().getString();
}

// ======================================================================
// class LinkAnimationTextListItem: private static member functions
// ======================================================================

const QPixmap &LinkAnimationTextListItem::getPixmap()
{
	static const QPixmap cs_pixmap(EmbeddedImageLoader::getPixmap("logical_animation_name.png"));
	return cs_pixmap;
}

// ======================================================================
