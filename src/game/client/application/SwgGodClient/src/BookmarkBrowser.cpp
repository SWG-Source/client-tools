// ======================================================================
//
// BookmarkBrowser.cpp
// copyright(c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgGodClient/FirstSwgGodClient.h"
#include "BookmarkBrowser.h"
#include "BookmarkBrowser.moc"

#include "ActionsView.h"
#include "BookmarkData.h"
#include "IconLoader.h"

#include <qlistview.h>
#include <qtoolbutton.h>

// ======================================================================
namespace
{
	struct MyCameraItem : public QListViewItem
	{
	public:
		MyCameraItem(QListViewItem* theParent, size_t theIndex, const BookmarkData::CameraBookmark & cam)
		: QListViewItem(theParent, cam.name.c_str()),
		  index(theIndex)
		{
			char buf[64];
			IGNORE_RETURN(_snprintf(buf, 64, "%4.0f,%4.0f,%4.0f", cam.info.translate.x, cam.info.translate.y, cam.info.translate.z));
			QListViewItem::setText(1, buf);
			QListViewItem::setRenameEnabled(0, true);
			QListViewItem::setPixmap(0, IL_PIXMAP(hi16_action_bookmark));
		}
		size_t  index;
	private:
		//disabled
		MyCameraItem();
	};

	struct MyObjectItem : public QListViewItem
	{
	public:
		MyObjectItem(QListViewItem* theParent, size_t theIndex, const BookmarkData::ObjectBookmark & obj)
		: QListViewItem(theParent, obj.cameraBookmark.name.c_str()),
		  index(theIndex)
		{
			QListViewItem::setRenameEnabled(0, true);
			QListViewItem::setPixmap(0, IL_PIXMAP(hi16_action_bookmark_red));
		}
		size_t  index;
	private:
		//disabled
		MyObjectItem();
	};

	void clearSubItems(QListViewItem* p)
	{
		while(p->firstChild())
		{
			QListViewItem* const item = p->firstChild();
			p->takeItem(item);
			delete item;
		}
	}
}

//-----------------------------------------------------------------

BookmarkBrowser::BookmarkBrowser(QWidget* theParent, const char* theName)
: BaseBookmarkBrowser(theParent, theName),
  MessageDispatch::Receiver(),
  m_cameraParentItem(0),
  m_objectParentItem(0)
{
	m_bookmarkList->clear();

	m_cameraParentItem = new QListViewItem(m_bookmarkList, "Camera Bookmarks");
	m_cameraParentItem->setPixmap(0, IL_PIXMAP(hi16_action_bookmark_folder));
	m_cameraParentItem->setSelectable(false);

	m_objectParentItem = new QListViewItem(m_bookmarkList, "Object Bookmarks");
	m_objectParentItem->setPixmap(0, IL_PIXMAP(hi16_action_bookmark_red_folder));
	m_objectParentItem->setSelectable(false);

	IGNORE_RETURN(connect(m_bookmarkList,    SIGNAL(doubleClicked(QListViewItem*)), this, SLOT(onDoubleClicked(QListViewItem*)))); 
	IGNORE_RETURN(connect(m_deleteButton,    SIGNAL(clicked()),                      this, SLOT(onDeleteClicked())));

	IGNORE_RETURN(connect(this, SIGNAL(cameraBookmarkSelected(int)), &ActionsView::getInstance(), SLOT(gotoCameraBookmark(int))));
	IGNORE_RETURN(connect(this, SIGNAL(objectBookmarkSelected(int)), &ActionsView::getInstance(), SLOT(gotoObjectBookmark(int))));

	connectToMessage(BookmarkData::Messages::CAMERA_BOOKMARKS_CHANGED);
	connectToMessage(BookmarkData::Messages::OBJECT_BOOKMARKS_CHANGED);
}
//----------------------------------------------------------------------
BookmarkBrowser::~BookmarkBrowser()
{
	clearSubItems(m_cameraParentItem);
	clearSubItems(m_objectParentItem);
	delete m_cameraParentItem;
	delete m_objectParentItem;
	m_cameraParentItem = 0;
	m_objectParentItem = 0;
}
//-----------------------------------------------------------------
void BookmarkBrowser::receiveMessage(const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message)
{
	UNREF(source);
	UNREF(message);

	const BookmarkData & bd = BookmarkData::getInstance();
		
	//-----------------------------------------------------------------

	if(message.isType(BookmarkData::Messages::CAMERA_BOOKMARKS_CHANGED))
	{
		clearSubItems(m_cameraParentItem);
		
		const size_t numCam = bd.getNumCameraBookmarks();
		
		for(size_t i = 0; i < numCam; ++i)
		{
			const BookmarkData::CameraBookmark & cam = bd.getCameraBookmark((numCam - i) - 1);
			NOT_NULL(new MyCameraItem(m_cameraParentItem, i, cam));
		}
	}

	//-----------------------------------------------------------------

	else if(message.isType(BookmarkData::Messages::OBJECT_BOOKMARKS_CHANGED))
	{
		clearSubItems(m_objectParentItem);
		
		const size_t numObj = bd.getNumObjectBookmarks();
		
		for(size_t i = 0; i < numObj; ++i)
		{
			const BookmarkData::ObjectBookmark & obj = bd.getObjectBookmark((numObj - i) - 1);
			NOT_NULL(new MyObjectItem(m_objectParentItem, i, obj));
		}		
	}
}
//-----------------------------------------------------------------

void BookmarkBrowser::onDoubleClicked(QListViewItem* item)
{
	MyCameraItem* const camBookmark = dynamic_cast<MyCameraItem*>(item); //use dynamic_cast because failure to cast here is acceptable
	if(!camBookmark)
	{
		MyObjectItem* const objBookmark = dynamic_cast<MyObjectItem*>(item); //use dynamic_cast because failure to cast here is acceptable

		if(objBookmark)
			emit objectBookmarkSelected(static_cast<int>(objBookmark->index));
	}
	else
	{
		emit cameraBookmarkSelected(static_cast<int>(camBookmark->index));
	}
}
//-----------------------------------------------------------------

void BookmarkBrowser::onDeleteClicked() const
{
	QListViewItem* const item = m_bookmarkList->selectedItem();

	if(item == 0)
		return;

	MyCameraItem* const camBookmark = dynamic_cast<MyCameraItem*>(item); //use dynamic_cast because failure to cast here is acceptable
	if(!camBookmark)
	{
		MyObjectItem* const objBookmark = dynamic_cast<MyObjectItem*>(item); //use dynamic_cast because failure to cast here is acceptable
		if(objBookmark)
			BookmarkData::getInstance().removeObjectBookmark(objBookmark->index);
	}
	else
	{
		BookmarkData::getInstance().removeCameraBookmark(camBookmark->index);
	}
}
// ======================================================================
