// ======================================================================
//
// TextEdit.cpp
// Copyright 2000-01, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "FirstSwgContentSync.h"
#include "TextEdit.h"

// ======================================================================

TextEdit::TextEdit(QWidget *parent)
: QTextEdit(parent),
	m_appending(false)
{
}

// ----------------------------------------------------------------------

void TextEdit::setContentsPos(int x, int y)
{
	if (m_appending)
		x = contentsX();

	QTextEdit::setContentsPos(x, y);
}

// ======================================================================
