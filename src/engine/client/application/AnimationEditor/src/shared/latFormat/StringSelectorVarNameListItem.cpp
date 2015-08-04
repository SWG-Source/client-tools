// ======================================================================
//
// StringSelectorVarNameListItem.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"
#include "AnimationEditor/StringSelectorVarNameListItem.h"

#include "AnimationEditor/EmbeddedImageLoader.h"
#include "AnimationEditor/FileModel.h"
#include "clientSkeletalAnimation/StringSelectorSkeletalAnimationTemplate.h"
#include "sharedFoundation/CrcLowerString.h"

#include <algorithm>
#include <qpixmap.h>
#include <string>

// ======================================================================

StringSelectorVarNameListItem::StringSelectorVarNameListItem(QListViewItem *newParent, StringSelectorSkeletalAnimationTemplate &selectorTemplate, FileModel &fileModel, const QString *preTextSortKey) :
	TextListItem(newParent, selectorTemplate.getEnvironmentVariableName().getString(), getPixmap(), &fileModel, preTextSortKey),
	m_selectorTemplate(selectorTemplate)
{
	m_type = LITYPE_StringSelectorVarNameListItem;
}

// ----------------------------------------------------------------------

bool StringSelectorVarNameListItem::modifyCandidateText(std::string &candidateText) const
{
	//-- Don't allow an empty name.
	if (candidateText.empty())
		return false;

	//-- Convert to lower case.
	std::transform(candidateText.begin(), candidateText.end(), candidateText.begin(), tolower);

	//-- Allow for setting.
	return true;
}

// ----------------------------------------------------------------------

void StringSelectorVarNameListItem::setSourceText(const std::string &newText) const
{
	//-- Set variable name.
	m_selectorTemplate.setEnvironmentVariableName(CrcLowerString(newText.c_str()));
}

// ----------------------------------------------------------------------

std::string StringSelectorVarNameListItem::getSourceText() const
{
	return m_selectorTemplate.getEnvironmentVariableName().getString();
}

// ======================================================================

const QPixmap &StringSelectorVarNameListItem::getPixmap()
{
	static const QPixmap cs_pixmap = EmbeddedImageLoader::getPixmap("variable_name.png");
	return cs_pixmap;
}

// ======================================================================
