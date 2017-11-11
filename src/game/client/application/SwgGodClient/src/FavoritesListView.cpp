// ======================================================================
//
// FavoritesListView.cpp
// copyright(c) 2005 Sony Online Entertainment
//
// ======================================================================

#include "SwgGodClient/FirstSwgGodClient.h"
#include "FavoritesListView.h"
#include "FavoritesListView.moc"

#include "AbstractFilesystemTree.h"
#include "ActionHack.h"
#include "ActionsEdit.h"
#include "ActionsObjectTemplate.h"
#include "ConfigGodClient.h"
#include "FileSystemTree.h"
#include "IconLoader.h"
#include "ObjectTemplateData.h"

#include <qinputdialog.h>
#include <qmessagebox.h>
#include <qdragobject.h>
#include <qpopupmenu.h>


//----------------------------------------------------------------------

const char * const FavoritesListView::DragMessages::FAVORITES_SERVER_TEMPLATE_DRAGGED = "FavoritesListView::DragMessages::FAVORITES_SERVER_TEMPLATE_DRAGGED";
const char * const FavoritesListView::DragMessages::FAVORITES_CLIENT_TEMPLATE_DRAGGED = "FavoritesListView::DragMessages::FAVORITES_CLIENT_TEMPLATE_DRAGGED";


FavoritesListViewItem::FavoritesListViewItem ( Type type, QListView * parent, QString label1, QString label2, QString label3, QString label4, QString label5, QString label6, QString label7, QString label8 )
: QListViewItem(parent,label1,label2,label3,label4,label5,label6,label7,label8),
m_type(type)
{
	if(type == FavoritesListViewItem::FOLDER)
	{
		setPixmap(0,IL_PIXMAP(hi16_action_bookmark_red_folder));
		setSelectable(true);
	}
	else if(type == FavoritesListViewItem::CLIENT_TEMPLATE)
	{
		setPixmap(0,IL_PIXMAP(hi16_action_bookmark));
		setSelectable(true);
		setDragEnabled(true);
	}
	else if(type == FavoritesListViewItem::SERVER_TEMPLATE)
	{
		setPixmap(0,IL_PIXMAP(hi16_action_bookmark_red));
		setSelectable(true);
		setDragEnabled(true);
	}
}

//----------------------------------------------------------------------

FavoritesListViewItem::FavoritesListViewItem ( Type type, QListViewItem * parent, QString label1, QString label2, QString label3, QString label4, QString label5, QString label6, QString label7, QString label8 )
: QListViewItem(parent,label1,label2,label3,label4,label5,label6,label7,label8),
m_type(type)
{

	if(type == FavoritesListViewItem::FOLDER)
	{
		setPixmap(0,IL_PIXMAP(hi16_action_bookmark_red_folder));
		setSelectable(true);
		setDragEnabled(true);
	}
	else if(type == FavoritesListViewItem::CLIENT_TEMPLATE)
	{
		setPixmap(0,IL_PIXMAP(hi16_action_bookmark));
		setSelectable(true);
		setDragEnabled(true);
	}
	else if(type == FavoritesListViewItem::SERVER_TEMPLATE)
	{
		setPixmap(0,IL_PIXMAP(hi16_action_bookmark_red));
		setSelectable(true);	
		setDragEnabled(true);
	}
}

//----------------------------------------------------------------------

FavoritesListViewItem::Type FavoritesListViewItem::getType()
{
	return m_type;
}

//======================================================================

//----------------------------------------------------------------------

FavoritesListView::FavoritesListView(QWidget* theParent, const char* theName)
: QListView(theParent, theName)
{
	IGNORE_RETURN(QListView::addColumn("Name"));
	IGNORE_RETURN(QListView::addColumn("Value"));
	QListView::setResizeMode(QListView::NoColumn);
	QListView::setRootIsDecorated(true);
	QListView::setColumnWidthMode(0, QListView::Maximum);
	QListView::setColumnWidthMode(1, QListView::Maximum);
	QListView::setAcceptDrops(true);

	IGNORE_RETURN(connect(this, SIGNAL(contextMenuRequested(QListViewItem*, const QPoint&, int)), this, SLOT(onContextMenuRequested(QListViewItem*, const QPoint&, int))));
	IGNORE_RETURN(connect(this, SIGNAL(selectionChanged()), this, SLOT(onSelectionChanged())));
}

//----------------------------------------------------------------------

/**
 * If the user drags an object template item off this window, generate a message that lets other windows process this event
 *
 */
QDragObject* FavoritesListView::dragObject()
{
	QListViewItem * item = selectedItem();
	FavoritesListViewItem *selectedItem = dynamic_cast<FavoritesListViewItem *> (item);
	if(selectedItem)
	{
		if(selectedItem->getType() == FavoritesListViewItem::SERVER_TEMPLATE)
		{
			return new QTextDrag(FavoritesListView::DragMessages::FAVORITES_SERVER_TEMPLATE_DRAGGED, this, "menu");
		}
		else if(selectedItem->getType() == FavoritesListViewItem::CLIENT_TEMPLATE)
		{
			return new QTextDrag(FavoritesListView::DragMessages::FAVORITES_CLIENT_TEMPLATE_DRAGGED, this, "menu");
		}
	}
	return NULL;
}

//----------------------------------------------------------------------

void FavoritesListView::dragEnterEvent(QDragEnterEvent* evt)
{
	QString text;
	if(QTextDrag::decode(evt, text))
	{
		if(text == ObjectTemplateData::DragMessages::SERVER_TEMPLATE_DRAGGED
			|| text == ObjectTemplateData::DragMessages::CLIENT_TEMPLATE_DRAGGED
			|| text == FavoritesListView::DragMessages::FAVORITES_SERVER_TEMPLATE_DRAGGED
			|| text == FavoritesListView::DragMessages::FAVORITES_CLIENT_TEMPLATE_DRAGGED
			)
		{
			evt->accept();
		}
	}
}

void FavoritesListView::dragMoveEvent(QDragMoveEvent* evt)
{
	if(evt)
	{
		FavoritesListViewItem *targetDraggedOver = dynamic_cast<FavoritesListViewItem *>(itemAt(viewport()->mapFromParent(evt->pos())));
		if(targetDraggedOver && (targetDraggedOver->getType() == FavoritesListViewItem::FOLDER || targetDraggedOver->getType() == FavoritesListViewItem::ROOT))
		{
			// open folders as they are dragged over
			targetDraggedOver->setOpen(true);
		}
	}
}

void FavoritesListView::dropEvent(QDropEvent *dropEvent)
{
	QString text;

	if (QTextDrag::decode(dropEvent, text))
	{	
		FavoritesListViewItem *targetDroppedOnto = dynamic_cast<FavoritesListViewItem *>(itemAt(viewport()->mapFromParent(dropEvent->pos())));
		
		if(!targetDroppedOnto)
		{
			targetDroppedOnto = dynamic_cast<FavoritesListViewItem *>(firstChild()); // the root
			DEBUG_FATAL(!targetDroppedOnto,("targetDroppedOnto is NULL - tried to set to root"));
		}

		while(targetDroppedOnto->getType() != FavoritesListViewItem::ROOT && targetDroppedOnto->getType() != FavoritesListViewItem::FOLDER)
		{
			targetDroppedOnto = dynamic_cast<FavoritesListViewItem *>(targetDroppedOnto->parent());
			DEBUG_FATAL(!targetDroppedOnto,("targetDroppedOnto is NULL - tried to set to folder||root"));
		}

		// handle moving internal items
		bool localServerTemplate = (text == FavoritesListView::DragMessages::FAVORITES_SERVER_TEMPLATE_DRAGGED);
		bool localClientTemplate = (text == FavoritesListView::DragMessages::FAVORITES_CLIENT_TEMPLATE_DRAGGED);
		if(localServerTemplate || localClientTemplate)
		{
			FavoritesListViewItem* itemSelected = dynamic_cast<FavoritesListViewItem *> (currentItem());
			if(itemSelected && targetDroppedOnto && itemSelected != targetDroppedOnto)
			{
				clearSelection();
				// insertitem() isn't working.  workaround - create a new one and delete the old one
				FavoritesListViewItem::Type type = (localServerTemplate) ? FavoritesListViewItem::SERVER_TEMPLATE : FavoritesListViewItem::CLIENT_TEMPLATE;
				new FavoritesListViewItem(type,targetDroppedOnto,itemSelected->text(0),itemSelected->text(1));
				delete itemSelected;
			}
		}

		// handle bringing in new items from the template trees
		bool externalServerTemplate = (text == ObjectTemplateData::DragMessages::SERVER_TEMPLATE_DRAGGED);
		bool externalClientTemplate = (text == ObjectTemplateData::DragMessages::CLIENT_TEMPLATE_DRAGGED);
		if(externalServerTemplate || externalClientTemplate)
		{
			std::string name;
			std::string templateName;
			FavoritesListViewItem::Type type;

			if(externalServerTemplate)
			{
				name = "New Server Template";
				templateName = ActionsEdit::getInstance().getSelectedServerTemplate();
				type = FavoritesListViewItem::SERVER_TEMPLATE;
			}
			else
			{
				name = "New Client Template";
				templateName = ActionsEdit::getInstance().getSelectedClientTemplate();
				type = FavoritesListViewItem::CLIENT_TEMPLATE;
			}
			new FavoritesListViewItem(type,targetDroppedOnto,name.c_str(),templateName.c_str());
		}
	}
}


//----------------------------------------------------------------------

/**
 * Show a right click menu
 */
void FavoritesListView::onContextMenuRequested(QListViewItem* item, const QPoint& p, int)
{
	FavoritesListViewItem *favoritesListViewItem = dynamic_cast<FavoritesListViewItem *>(item);
	if(favoritesListViewItem)
	{
		if(favoritesListViewItem->getType() == FavoritesListViewItem::FOLDER)
		{
			QPopupMenu* const m_pop = new QPopupMenu(this, "menu");
			const int id_newFolder = m_pop->insertItem("new folder");
			const int id_rename = m_pop->insertItem("rename");
			const int id_delete = m_pop->insertItem("delete");
			int id = m_pop->exec(p);
			
			if(id == id_newFolder)
			{
				new FavoritesListViewItem(FavoritesListViewItem::FOLDER,favoritesListViewItem,"New Folder");		
			}
			else if(id == id_rename)
			{
				bool ok = false;
				QString originalName = favoritesListViewItem->text(0);
				QString qName = (QInputDialog::getText(tr("Rename"), tr("Enter the new name"), QLineEdit::Normal, originalName, &ok));
				if(!qName.isNull())
				{
					std::string name = qName.latin1();
					if(ok && !name.empty())
					{
						favoritesListViewItem->setText(0,name.c_str());
					}
				}
			}
			else if(id == id_delete)
			{
				int messageBoxResult = QMessageBox::question(0,"Delete","Are you sure?", QMessageBox::Yes , QMessageBox::No );
				if ( messageBoxResult == QMessageBox::Yes )
				{
					delete favoritesListViewItem;
					favoritesListViewItem = NULL;
				}
			}
		}
		else if(favoritesListViewItem->getType() == FavoritesListViewItem::SERVER_TEMPLATE
			|| favoritesListViewItem->getType() == FavoritesListViewItem::CLIENT_TEMPLATE)
		{
			QPopupMenu* const m_pop = new QPopupMenu(this, "menu");
			const int id_rename = m_pop->insertItem("rename");
			const int id_delete = m_pop->insertItem("delete");
			int id = m_pop->exec(p);
			
			if(id == id_rename)
			{
				bool ok = false;
				QString originalName = favoritesListViewItem->text(0);
				QString qName = (QInputDialog::getText(tr("Rename"), tr("Enter the new name"), QLineEdit::Normal, originalName, &ok));
				if(!qName.isNull())
				{
					std::string name = qName.latin1();
					if(ok && !name.empty())
					{
						favoritesListViewItem->setText(0,name.c_str());
					}
				}
			}
			else if(id == id_delete)
			{
				int messageBoxResult = QMessageBox::question(0,"Delete","Are you sure?", QMessageBox::Yes , QMessageBox::No );
				if ( messageBoxResult == QMessageBox::Yes )
				{
					delete favoritesListViewItem;
					favoritesListViewItem = NULL;
				}
			}
		}
		else if(favoritesListViewItem->getType() == FavoritesListViewItem::ROOT)
		{
			QPopupMenu* const m_pop = new QPopupMenu(this, "menu");
			const int id_newFolder = m_pop->insertItem("new folder");
			int id = m_pop->exec(p);
			
			if(id == id_newFolder)
			{
				new FavoritesListViewItem(FavoritesListViewItem::FOLDER,favoritesListViewItem,"New Folder");
			}	
		}
	}
} 

// ======================================================================
