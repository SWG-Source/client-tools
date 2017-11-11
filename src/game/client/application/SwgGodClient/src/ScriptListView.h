// ======================================================================
//
// ScriptListView.h
// copyright(c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ScriptListView_H
#define INCLUDED_ScriptListView_H

// ======================================================================
#include <qlistview.h>

class QDragObject;

//-----------------------------------------------------------------

/**
 * The ListView that allows the user to view and modify scripts.
 * Most of the functionality for this is implemented in ActionsScript.
 *
 * @todo: make perforce operations run asynchronously, in a separate thread.
 * @todo: support dragging scripts from the list and onto objects in the GameWidget, as well as objects in the ObjectEditor
 */

class ScriptListView : public QListView
{
	Q_OBJECT; //lint !e1516 !e19 !e1924 !e1762 various deficiencies in the Qt macro

public:
	ScriptListView(QWidget* parent, const char* name);

public slots:
	void onRefreshList();

protected:
	QDragObject* dragObject();

protected slots:
	void onContextMenuRequested(QListViewItem* item, const QPoint & pos, int col);

private slots:
	void onSelectionChanged() const;

private:
	//disabled
	ScriptListView();
	ScriptListView(const ScriptListView& rhs);
	ScriptListView& operator=(const ScriptListView& rhs);
};

// ======================================================================

#endif
