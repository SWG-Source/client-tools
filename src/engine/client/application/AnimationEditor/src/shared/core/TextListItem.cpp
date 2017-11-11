// ======================================================================
//
// TextListItem.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"
#include "AnimationEditor/TextListItem.h"

#include "AnimationEditor/FileModel.h"

// ======================================================================

TextListItem::TextListItem(QListViewItem *newParent, const std::string &initialText, const QPixmap &newPixmap, FileModel *fileModel, const QString *preTextSortKey) :
	ListItem(newParent, preTextSortKey),
	m_fileModel(fileModel)
{
	m_type = LITYPE_TextListItem;
	//-- Setup item text.
	TextListItem::setText(0, initialText.c_str());

	//-- Setup item pixmap.
	TextListItem::setPixmap(0, newPixmap);

	//-- Enable in-place renaming on item.
	TextListItem::setRenameEnabled(0, true);
}

// ----------------------------------------------------------------------

TextListItem::~TextListItem()
{
	//lint -esym(1540, TextListItem::m_fileModel) // neither freed nor zero'ed // we don't own, and can't zero because const.
}

// ----------------------------------------------------------------------
/**
 * Give the TextListItem instance a chance to validate and modify the text 
 * that was entered by the user.
 * 
 * It is intended that the TextListItem-derived class implementation will
 * override this function.  By default, this function will not modify
 * the text and will return true, indicating that the text change should
 * go through.
 * 
 * If the function returns true, the TextListItem will use the
 * (potentially modified) candidateText arg's value upon return for
 * the text of the QListViewItem.  A call to setText() will follow.
 *
 * If the function returns false, the TextListItem will call getText() 
 * and reset the QListViewItem.
 *
 * @param candidateText  On input this contains the in-place edited
 *                       text from the user.  On output, this argument
 *                       contains the text that will replace the
 *                       user-entered text.
 *
 * @return  true if the name change is valid and should go through.
 *          false if the name change should not go through.
 */

bool TextListItem::modifyCandidateText(std::string &candidateText) const
{
	// Don't modify text, return true (accept the change).
	UNREF(candidateText);
	return true;
} //lint !e1764 // candidateText could be declared const ref // not for intent of API.

// ======================================================================

void TextListItem::okRename(int col)
{
	//-- Call handling performed by Qt.
	ListItem::okRename(col);

	//-- I only care about renaming of the name column.
	if (col != 0)
	{
		WARNING(true, ("okRename(): received rename completion for unsupported column [%d].", col));
		return;
	}

	//-- Get name.
	std::string  candidateText(text(0));

	//-- Let instance modify name if necessary.
	const bool keepTextChange = modifyCandidateText(candidateText);
	if (!keepTextChange)
	{
		// Instance determined new text is no good, revert to original text.
		setText(0, getSourceText().c_str());
	}
	else
	{
		// Instance validated text, might have modified it so set text to candidateText.
		setText(0, candidateText.c_str());

		// Set modified text to instance.
		setSourceText(candidateText);

		// Tell optional FileModel it has been modified.
		if (m_fileModel)
			m_fileModel->setModifiedState(true);
	}
}

// ======================================================================
