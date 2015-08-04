// ======================================================================
//
// FavoritesListView.h
// copyright (c) 2005 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_FavoritesListView_H
#define INCLUDED_FavoritesListView_H

// ======================================================================

#include <qlistview.h>
#include <string>

//-----------------------------------------------------------------

class QPixmap;
class QListView;
class AbstractFilesystemTree;

class FavoritesListViewItem : public QListViewItem
{
public:
	enum Type
	{
		ROOT = 0,
		FOLDER,
		SERVER_TEMPLATE,
		CLIENT_TEMPLATE
	};

private:
	Type m_type;

public:
	FavoritesListViewItem ( Type type, QListView * parent, QString label1, QString label2 = QString::null, QString label3 = QString::null, QString label4 = QString::null, QString label5 = QString::null, QString label6 = QString::null, QString label7 = QString::null, QString label8 = QString::null );
	FavoritesListViewItem ( Type type, QListViewItem * parent, QString label1, QString label2 = QString::null, QString label3 = QString::null, QString label4 = QString::null, QString label5 = QString::null, QString label6 = QString::null, QString label7 = QString::null, QString label8 = QString::null );
	Type getType();
};

//-----------------------------------------------------------------

class FavoritesListView : public QListView
{
	Q_OBJECT; 

public:

	struct DragMessages
	{
		static const char* const FAVORITES_SERVER_TEMPLATE_DRAGGED;
		static const char* const FAVORITES_CLIENT_TEMPLATE_DRAGGED;
	};

	FavoritesListView(QWidget* parent, const char* name);
		
public slots:

protected:
	QDragObject* dragObject();
	void dragEnterEvent(QDragEnterEvent* evt);
	void dropEvent(QDropEvent *dropEvent);
	void  dragMoveEvent(QDragMoveEvent* evt);

protected slots:
	void onContextMenuRequested(QListViewItem* item, const QPoint& pos, int col);

private slots:

private:
	//disabled
	FavoritesListView();
	FavoritesListView(const FavoritesListView & rhs);
	FavoritesListView& operator= (const FavoritesListView& rhs);	
};

// ======================================================================

#endif
