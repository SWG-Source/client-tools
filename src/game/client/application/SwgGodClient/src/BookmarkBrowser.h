// ======================================================================
//
// BookmarkBrowser.h
// copyright(c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_BookmarkBrowser_H
#define INCLUDED_BookmarkBrowser_H

// ======================================================================
#include "BaseBookmarkBrowser.h"
#include <qlistview.h>

//-----------------------------------------------------------------

class QListViewItem;

//-----------------------------------------------------------------

/**
* BookmarkBrowser is the implementation of the designer-generated BaseBookmarkBrowser
* It contains a QListView and other widgets for displaying & manipulating object
* and camera bookmarks.
*
* @todo: make Camera bookmarks visible for current scene only
* @todo: make Object bookmarks disabled/enabled based on whether the associated object is currently in the scene
*/
class BookmarkBrowser : public BaseBookmarkBrowser, public MessageDispatch::Receiver
{
	Q_OBJECT; //lint !e1516 !e19 !e1924 !e1762 various deficiencies in the Qt macro

public:
	BookmarkBrowser(QWidget* parent, const char* name);
	~BookmarkBrowser();

	void receiveMessage(const MessageDispatch::Emitter& source, const MessageDispatch::MessageBase& message);

private slots:
	void onDeleteClicked() const;
	void onDoubleClicked(QListViewItem* item);

signals:

	void cameraBookmarkSelected(int);
	void objectBookmarkSelected(int);

private:
	//disabled
	BookmarkBrowser();
	BookmarkBrowser(const BookmarkBrowser& rhs);
	BookmarkBrowser & operator=(const BookmarkBrowser& rhs);

	QListViewItem*      m_cameraParentItem;
	QListViewItem*      m_objectParentItem;

};

// ======================================================================

#endif
