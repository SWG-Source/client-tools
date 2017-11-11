// ======================================================================
//
// BuildoutAreaListView.h
//
// Copyright 2005 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_BuildoutAreaListView_H
#define INCLUDED_BuildoutAreaListView_H

// ======================================================================

#include <qlistview.h>

// ======================================================================

/**
 * The ListView that allows the user to view and modify buildout areas.
 * Most of the functionality for this is implemented in ActionsBuildoutArea.
 */

class BuildoutAreaListView: public QListView
{
	Q_OBJECT; //lint !e1516 !e19 !e1924 !e1762 various deficiencies in the Qt macro

public:
	BuildoutAreaListView(QWidget *parent, char const *name);

public slots:
	void onRefresh();
	void onEdit();
	void onSave();

protected slots:
	void onContextMenuRequested(QListViewItem* item, const QPoint & pos, int col);

private slots:
	void onSelectionChanged() const;

private:
	//disabled
	BuildoutAreaListView();
	BuildoutAreaListView(BuildoutAreaListView const &);
	BuildoutAreaListView &operator=(BuildoutAreaListView const &);
};

// ======================================================================

#endif // INCLUDED_BuildoutAreaListView_H
