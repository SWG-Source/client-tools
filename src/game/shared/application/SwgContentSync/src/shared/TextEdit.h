// ======================================================================
//
// TextEdit.h
// Copyright 2000-01, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_TextEdit_H
#define INCLUDED_TextEdit_H

// ======================================================================

#include <qtextedit.h>

// ======================================================================

class TextEdit : public QTextEdit
{
public:
	TextEdit(QWidget *parent);

	void setAppending(bool appending);

public slots:

	virtual void setContentsPos(int x, int y);

private:

	bool m_appending;
};

// ======================================================================

inline void TextEdit::setAppending(bool appending)
{
	m_appending = appending;
}

// ======================================================================

#endif
