// ======================================================================
//
// ActionAnimationListItem.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"
#include "AnimationEditor/ActionAnimationListItem.h"

#include "AnimationEditor/AnimationEditorGameWorld.h"
#include "AnimationEditor/EmbeddedImageLoader.h"
#include "clientSkeletalAnimation/EditableBasicAnimationAction.h"
#include "sharedFoundation/CrcLowerString.h"

#include <algorithm>
#include <qpixmap.h>
#include <string>

// ======================================================================
// ActionAnimationListItem: public member functions
// ======================================================================

ActionAnimationListItem::ActionAnimationListItem(QListViewItem *newParent, EditableBasicAnimationAction &action, FileModel *fileModel, const QString *preTextSortKey) :
	TextListItem(newParent, action.getLogicalAnimationName(AnimationEditorGameWorld::getFocusObjectAnimationEnvironment()).getString(), getPixmap(), fileModel, preTextSortKey),
	m_action(action)
{
	m_type = LITYPE_ActionAnimationListItem;
}

// ======================================================================
// ActionAnimationListItem: private static member functions
// ======================================================================

const QPixmap &ActionAnimationListItem::getPixmap()
{
	static const QPixmap cs_pixmap(EmbeddedImageLoader::getPixmap("logical_animation_name.png"));
	return cs_pixmap;
}

// ======================================================================
// ActionAnimationListItem: private member functions
// ======================================================================

bool ActionAnimationListItem::modifyCandidateText(std::string &candidateText) const
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

void ActionAnimationListItem::setSourceText(const std::string &newText) const
{
	m_action.setLogicalAnimationName(CrcLowerString(newText.c_str()));
}

// ----------------------------------------------------------------------

std::string ActionAnimationListItem::getSourceText() const
{
	return m_action.getLogicalAnimationName(AnimationEditorGameWorld::getFocusObjectAnimationEnvironment()).getString();
}

// ======================================================================
