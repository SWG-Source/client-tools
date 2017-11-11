// ======================================================================
//
// TreeListView.h
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TreeListView_H
#define INCLUDED_TreeListView_H

// ----------------------------------------------------------------------

#include <qlistview.h>

// ----------------------------------------------------------------------

class TreeListView : public QListView
{
	Q_OBJECT //lint !e830 !e1511 !e1516 !e1924

	class TreeListViewWhatsThis;

public:
	explicit TreeListView(QWidget *parentTreeListView, const char *nameTreeListView = 0);
    ~TreeListView();

	QString whatsThisAt(const QPoint &p);
	QString whatsThisText(QListViewItem *i);

protected:
	void dragEnterEvent(QDragEnterEvent *dragEvent);
	void dropEvent(QDropEvent *dropEvent);
	void contentsMousePressEvent(QMouseEvent *mouseEvent);
	void contentsMouseMoveEvent(QMouseEvent *mouseEvent);
	void contentsMouseReleaseEvent(QMouseEvent *mouseEvent);

signals:
	void nodeDropped(const QString&, const QString&);

private slots:

private:
	bool m_dragging;
	QPoint m_pressPos;
	TreeListViewWhatsThis * m_itemWhatsThis;

private: //-- disabled
	TreeListView(TreeListView const &);
	TreeListView &operator=(TreeListView const &);
	TreeListView();
};

// ======================================================================

#endif // INCLUDED_TreeListView_H
