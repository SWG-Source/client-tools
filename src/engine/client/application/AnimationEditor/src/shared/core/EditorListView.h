// ======================================================================
//
// EditorListView.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_EditorListView_H
#define INCLUDED_EditorListView_H

// ======================================================================

#include <qlistview.h>

// ======================================================================

class EditorListView: public QListView
{
public:

	EditorListView(QWidget *newParent = 0, const char *newName = 0, WFlags f = 0);

	virtual void keyPressEvent(QKeyEvent *keyEvent);

};

// ======================================================================

#endif
