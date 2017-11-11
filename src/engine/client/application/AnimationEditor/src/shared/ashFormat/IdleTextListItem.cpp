// ======================================================================
//
// IdleTextListItem.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"
#include "AnimationEditor/IdleTextListItem.h"

#include "AnimationEditor/EmbeddedImageLoader.h"
#include "clientSkeletalAnimation/EditableAnimationState.h"
#include "sharedFoundation/CrcLowerString.h"

#include <algorithm>
#include <qpixmap.h>
#include <string>

// ======================================================================

IdleTextListItem::IdleTextListItem(QListViewItem *newParent, EditableAnimationState &state, FileModel *fileModel, const QString *preTextSortKey) :
	TextListItem(newParent, state.getLogicalAnimationName().getString(), getPixmap(), fileModel, preTextSortKey),
	m_state(state)
{
	m_type = LITYPE_IdleTextListItem;
}

// ======================================================================

bool IdleTextListItem::modifyCandidateText(std::string &candidateText) const
{
	//-- Make there's at least some characters.
	if (candidateText.empty())
		return false;

	//-- Convert string to lower case (we only support lower case).
	std::transform(candidateText.begin(), candidateText.end(), candidateText.begin(), tolower);

	//-- Allow it to pass.
	return true;
}

// ----------------------------------------------------------------------

void IdleTextListItem::setSourceText(const std::string &newText) const
{
	m_state.setLogicalAnimationName(CrcLowerString(newText.c_str()));
}

// ----------------------------------------------------------------------

std::string IdleTextListItem::getSourceText() const
{
	return m_state.getLogicalAnimationName().getString();
}

// ======================================================================

const QPixmap &IdleTextListItem::getPixmap()
{
	static const QPixmap cs_pixmap(EmbeddedImageLoader::getPixmap("logical_animation_name.png"));
	return cs_pixmap;
}

// ======================================================================
