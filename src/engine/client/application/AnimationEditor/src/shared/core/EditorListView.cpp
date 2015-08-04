// ======================================================================
//
// EditorListView.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"
#include "AnimationEditor/EditorListView.h"

#include "AnimationEditor/ListItem.h"

// ======================================================================

EditorListView::EditorListView(QWidget *newParent, const char *newName, WFlags f) :
	QListView(newParent, newName, f)
{
}

// ----------------------------------------------------------------------

void EditorListView::keyPressEvent(QKeyEvent *keyEvent)
{
	DEBUG_REPORT_LOG(true, ("EditorListView: keyPressEvent() received.\n"));
	
	//-- Check if selected item handles key press.
	bool handled = false;

	ListItem *const listItem = dynamic_cast<ListItem*>(selectedItem());
	if (listItem)
		handled = listItem->handleKeyPress(keyEvent);

	//-- Pass on if not handled.
	if (!handled)
		QListView::keyPressEvent(keyEvent);
	else
	{
		NOT_NULL(keyEvent);
		keyEvent->accept();
	}
}

// ======================================================================
