// ======================================================================
//
// MyMultiLineEdit.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_MyMultiLineEdit_H
#define INCLUDED_MyMultiLineEdit_H

#include <qtextedit.h>

// ======================================================================
class MyMultiLineEdit : public QTextEdit
{
public:
	explicit MyMultiLineEdit ( QWidget * theParent=0, const char * theName=0 ) : QTextEdit (theParent, theName) {}
	inline bool focusNextPrevChild ( bool next ) { return QWidget::focusNextPrevChild (next); }

private:
	MyMultiLineEdit (const MyMultiLineEdit & rhs);
	MyMultiLineEdit & operator= (const MyMultiLineEdit & rhs);
};

// ======================================================================

#endif
