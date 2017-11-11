// ======================================================================
//
// BrushListView.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_BrushListView_H
#define INCLUDED_BrushListView_H

// ======================================================================
#include <qlistview.h>

class QDragObject;

//-----------------------------------------------------------------

/**
*/

class BrushListView : public QListView
{
	Q_OBJECT; //lint !e1516 !e19 !e1924 !e1762 various deficiencies in the Qt macro

public:
	BrushListView(QWidget* parent, const char* name);

public slots:
	void onRefreshList() const;

protected:
	QDragObject* dragObject();

protected slots:
	void onContextMenuRequested(QListViewItem* item, const QPoint& pos, int col);

private slots:
	void onSelectionChanged() const;

private:
	//disabled
	BrushListView();
	BrushListView(const BrushListView& rhs);
	BrushListView& operator= (const BrushListView& rhs);
};

// ======================================================================

#endif
